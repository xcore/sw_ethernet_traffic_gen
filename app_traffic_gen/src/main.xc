// Copyright (c) 2011, XMOS Ltd, All rights reserved
// This software is freely distributable under a derivative of the
// University of Illinois/NCSA Open Source License posted in
// LICENSE.txt and at <http://github.xcore.com/>

/*************************************************************************
 *
 * Ethernet MAC Layer Client Test Code
 * IEEE 802.3 MAC Client
 *
 *************************************************************************
 *
 * Layer 2 MII demo
 * Note: Only supports unfragmented IP packets
 *
 *************************************************************************/

#include <xs1.h>
#include <print.h>
#include <stdio.h>
#include <platform.h>
#include "otp_board_info.h"
#include "ethernet.h"
#include "ethernet_board_support.h"
#include "packet_generator.h"
#include "xscope.h"
#include "buffer_manager.h"
#include "packet_transmitter.h"
#include "packet_controller.h"
#include "debug_print.h"

extern unsigned char g_src_mac[];

void xscope_user_init(void) {
  xscope_register(0);
  xscope_config_io(XSCOPE_IO_BASIC);
}

on ETHERNET_DEFAULT_TILE: otp_ports_t otp_ports = OTP_PORTS_INITIALIZER;

smi_interface_t smi = ETHERNET_DEFAULT_SMI_INIT;
mii_interface_t mii = ETHERNET_DEFAULT_MII_INIT;
ethernet_reset_interface_t eth_rst = ETHERNET_DEFAULT_RESET_INTERFACE_INIT;

void eth_client(chanend tx, chanend rx)
{
  unsigned int rxbuf[1600/4];
  unsigned int src_port;
  unsigned int nbytes;

  //::setup-filter
  mac_set_custom_filter(rx, 0x1);
  //::

  //::mainloop
  while (1)
  {
    select {
      case mac_rx(rx, (rxbuf,char[]), nbytes, src_port):
        mac_tx(tx, rxbuf, nbytes, ETH_BROADCAST);
      break;
    }
  }
}

// The maximum read size is 256 bytes
#define MAX_BYTES_READ 256
#define MAX_WORDS_READ (MAX_BYTES_READ / 4)

// Rate calculation is done using a fixed-point number to save using a divide in the critical loop
#define POINT_POS 8

extern pkt_gen_ctrl_t directed[];

void listener_and_generator(chanend c_host_data, chanend c_mac_address, streaming chanend c_prod)
{
  // Receive the mac address from the ethernet tile
  slave {
    for (int i = 0; i < MAC_ADDRESS_BYTES; i++)
      c_mac_address :> g_src_mac[i];
  }

  // Setup xscope
  unsigned int xscope_buffer[MAX_WORDS_READ];
  for (int i = 0; i < MAX_WORDS_READ; i++)
    xscope_buffer[i] = 0;
  xscope_connect_data_from_host(c_host_data);

  // State needed by the packet generator
  random_generator_t r = random_create_generator_from_seed(0);
  uintptr_t ctrl_ptr = (uintptr_t)&directed[0];
  unsigned rate_factor = 0;
  generator_mode_t generator_mode = GENERATOR_DIRECTED;//GENERATOR_SILENT;

  unsigned len = 0;
  pkt_ctrl_t * unsafe packet = NULL;

  while (1) {
    if ((generator_mode != GENERATOR_SILENT) && ctrl_ptr) {
      int buffers = 1;
      while (buffers) {
        if (!packet) {
          unsafe {
            packet = choose_packet_type(&r, ctrl_ptr, &len);
          }
        }
        if (packet) {
          select {
            case c_prod :> uintptr_t dptr: {
              unsigned delay = 0;

              // The value can overflow if multiplying large packet lengths by maximum delay
              const int ifg_bytes = 96/8;
              const int preamble_bytes = 8;
              const int crc_bytes = 4;
              int bits_on_wire = (len + ifg_bytes + preamble_bytes + crc_bytes) * 8;
              if (rate_factor >= (1 << POINT_POS))
                delay = bits_on_wire * (rate_factor >> POINT_POS);
              else
                delay = (bits_on_wire * rate_factor) >> POINT_POS;

              unsafe {
                ((packet_data_t *)dptr)->delay = delay;

                switch (packet->type) {
                  case TYPE_UNICAST:   gen_unicast_frame(dptr);   break;
                  case TYPE_MULTICAST: gen_multicast_frame(dptr); break;
                  case TYPE_BROADCAST: gen_broadcast_frame(dptr); break;
                }

                // Send pointer and length to transmitter
                c_prod <: dptr;
                c_prod <: (len + sizeof(((packet_data_t *)dptr)->delay));
              }

              // Choose the next packet type
              ctrl_ptr = choose_next(&r, ctrl_ptr);
              packet = NULL;
              break;
            }
            default:
              buffers = 0;
              break;
          }
        }
      }
    }

    // Check for xscope data
    int bytes_read = 0;
    select {
      case xscope_data_from_host(c_host_data, (unsigned char *)xscope_buffer, bytes_read):
        if (bytes_read) {
          handle_host_data((unsigned char *)xscope_buffer, bytes_read,
              &generator_mode, &ctrl_ptr, &rate_factor);

          // Clear buffer after use
          for (int i = 0; i < (bytes_read + 3)/4; i++)
            xscope_buffer[i] = 0;
        }
        break;

      default:
        break;
    }
  }
}

int main()
{
  chan c_rx[1], c_tx[2];
  streaming chan c_prod;
  streaming chan c_con;
  chan c_host_data;

  // Need a channel to send the mac address over to the generation tile
  chan c_mac_address;

  par
  {
    xscope_host_data(c_host_data);

    on ETHERNET_DEFAULT_TILE:
    {
      char mac_address[MAC_ADDRESS_BYTES];
      otp_board_info_get_mac(otp_ports, 0, mac_address);

      master {
        // Send the mac address to the generation tile
        for (int i = 0; i < MAC_ADDRESS_BYTES; i++)
          c_mac_address <: mac_address[i];
      }
      
      eth_phy_reset(eth_rst);
      smi_init(smi);
      eth_phy_config(1, smi);
      ethernet_server(mii,
          null,
          mac_address,
          c_rx, 1,
          c_tx, 2);
    }
    on tile[0] : eth_client(c_tx[0], c_rx[0]);
    on tile[0] : buffer_manager(c_prod, c_con);
    on tile[0] : packet_transmitter(c_tx[1], c_con);
    on tile[0] : listener_and_generator(c_host_data, c_mac_address, c_prod);
  }

  return 0;
}
