#include <platform.h>
#include <xscope.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <xccompat.h>
#include <ctype.h>

#include "xassert.h"

#include "debug_print.h"
#include "traffic_ctlr_host_cmds.h"
#include "packet_controller.h"
#include "common.h"
#include "packet_generator.h"
#include "c_utils.h"

extern unsigned char g_src_mac[];
extern pkt_gen_ctrl_t initial;
extern pkt_gen_ctrl_t directed[];

/* The multiplier used to control the line rate */
volatile unsigned g_rate_factor[2] = {0, 0};

/* The index into the table of configurations - start on second entry */
unsigned int g_directed_write_index = 1;

static void copy_over_config(int read_index)
{
  int write_index = read_index ? 0 : 1;
  pkt_ctrl_t *read = get_packet_control(TYPE_UNICAST, read_index);
  pkt_ctrl_t *write = get_packet_control(TYPE_UNICAST, write_index);
  *write = *read;

  read = get_packet_control(TYPE_MULTICAST, read_index);
  write = get_packet_control(TYPE_MULTICAST, write_index);
  *write = *read;

  read = get_packet_control(TYPE_BROADCAST, read_index);
  write = get_packet_control(TYPE_BROADCAST, write_index);
  *write = *read;

  g_rate_factor[write_index] = g_rate_factor[read_index];

  unsigned char mac_address[6];
  get_unicast_mac_address(read_index, mac_address);
  set_unicast_mac_address(write_index, mac_address);

  get_multicast_mac_address(read_index, mac_address);
  set_multicast_mac_address(write_index, mac_address);
}

/**
 * \brief   A function that processes data being sent from the host and
 *          informs the analysis engine of any changes
 */
void handle_host_data(unsigned char buffer[], int bytes_read, generator_mode_t *generator_mode,
    uintptr_t *ctrl_ptr, unsigned *rate_factor)
{
  tester_command_t cmd = buffer[0];
  const unsigned char *ptr = &buffer[1]; // Skip command
  switch (cmd) {
    case CMD_SET_GENERATOR_MODE:
      {
        unsigned char c = get_next_char(&ptr);
        switch (c) {
          case 's': *generator_mode = GENERATOR_SILENT;   break;
          case 'r': *generator_mode = GENERATOR_RANDOM;   break;
          case 'd': *generator_mode = GENERATOR_DIRECTED; break;
          default : break;
        }
      }
      break;

    case CMD_PKT_CONTROL:
      {
        unsigned char c = get_next_char(&ptr);
        pkt_ctrl_t *pkt_ctrl = NULL;
        pkt_type_t pkt_type;

        switch (c) {
          case 'u': pkt_type = TYPE_UNICAST; break;
          case 'm': pkt_type = TYPE_MULTICAST; break;
          case 'b': pkt_type = TYPE_BROADCAST; break;
          default : break;
        }

        pkt_ctrl = get_packet_control(pkt_type, g_directed_write_index);
        pkt_ctrl->weight = convert_atoi_substr(&ptr);
        if (pkt_ctrl->weight) {
          pkt_ctrl->size_min = convert_atoi_substr(&ptr);
          pkt_ctrl->size_max = convert_atoi_substr(&ptr);
        }
      }
      break;

    case CMD_LINE_RATE:
      g_rate_factor[g_directed_write_index] = convert_atoi_substr(&ptr);
      break;

    case CMD_SET_MAC_ADDRESS:
      {
        unsigned char pkt_type = get_next_char(&ptr);
        unsigned char mac_address[MAC_ADDRESS_BYTES] = { 0, 0, 0, 0, 0, 0 };
        int error = parse_mac_address(ptr, mac_address);
        if (error)
          break;

        switch (pkt_type) {
          case 'u': set_unicast_mac_address(g_directed_write_index, mac_address); break;
          case 'm': set_multicast_mac_address(g_directed_write_index, mac_address); break;
          default : break;
        }
      }
      break;

    case CMD_PRINT_PKT_CONFIGURATION:
      {
        pkt_ctrl_t *pkt_ctrl = NULL;
        unsigned char mac_address[6];

        debug_printf("Packet generator is running in %s mode on %x:%x:%x:%x:%x:%x\n",
            (*generator_mode == 0) ? "silent" : (*generator_mode == 1) ? "random" : "directed",
            g_src_mac[0], g_src_mac[1], g_src_mac[2], g_src_mac[3], g_src_mac[4], g_src_mac[5]);

        int directed_read_index = g_directed_write_index ? 0 : 1;

        get_unicast_mac_address(directed_read_index, mac_address);

        debug_printf("Current configuration (%d rate factor)\n", g_rate_factor[directed_read_index]);
        pkt_ctrl = get_packet_control(TYPE_UNICAST, directed_read_index);
        debug_printf("Unicast   weight %d, packet bytes %d-%d [%x:%x:%x:%x:%x:%x]\n",
            pkt_ctrl->weight, pkt_ctrl->size_min, pkt_ctrl->size_max,
            mac_address[0], mac_address[1], mac_address[2], mac_address[3], mac_address[4], mac_address[5]);

        get_multicast_mac_address(g_directed_write_index, mac_address);
        pkt_ctrl = get_packet_control(TYPE_MULTICAST, directed_read_index);
        debug_printf("Multicast weight %d, packet bytes %d-%d [%x:%x:%x:%x:%x:%x]\n",
            pkt_ctrl->weight, pkt_ctrl->size_min, pkt_ctrl->size_max,
            mac_address[0], mac_address[1], mac_address[2], mac_address[3], mac_address[4], mac_address[5]);

        pkt_ctrl = get_packet_control(TYPE_BROADCAST, directed_read_index);
        debug_printf("Broadcast weight %d, packet bytes %d-%d\n", pkt_ctrl->weight, pkt_ctrl->size_min, pkt_ctrl->size_max);
        

        debug_printf("Next configuration (%d rate factor):\n", g_rate_factor[g_directed_write_index]);

        get_unicast_mac_address(g_directed_write_index, mac_address);
        pkt_ctrl = get_packet_control(TYPE_UNICAST, g_directed_write_index);
        debug_printf("Unicast   weight %d, packet bytes %d-%d [%x:%x:%x:%x:%x:%x]\n",
            pkt_ctrl->weight, pkt_ctrl->size_min, pkt_ctrl->size_max,
            mac_address[0], mac_address[1], mac_address[2], mac_address[3], mac_address[4], mac_address[5]);

        get_multicast_mac_address(g_directed_write_index, mac_address);
        pkt_ctrl = get_packet_control(TYPE_MULTICAST, g_directed_write_index);
        debug_printf("Multicast weight %d, packet bytes %d-%d [%x:%x:%x:%x:%x:%x]\n",
            pkt_ctrl->weight, pkt_ctrl->size_min, pkt_ctrl->size_max,
            mac_address[0], mac_address[1], mac_address[2], mac_address[3], mac_address[4], mac_address[5]);
        
        pkt_ctrl = get_packet_control(TYPE_BROADCAST, g_directed_write_index);
        debug_printf("Broadcast weight %d, packet bytes %d-%d\n", pkt_ctrl->weight, pkt_ctrl->size_min, pkt_ctrl->size_max);

        debug_printf("Press 's' to swap, press 'e' to update and copy\n");
      }
      break;

    case CMD_APPLY_CFG:
      // Pass the write index as the read index as it is just about to be swapped
      copy_over_config(g_directed_write_index);
      // fall through

    case CMD_SWAP_CFG: {
      // The read index is the write index as it will be swapped below
      int directed_read_index = g_directed_write_index;
      if (*generator_mode == GENERATOR_RANDOM)
        *ctrl_ptr = (uintptr_t)&initial;
      else if (*generator_mode == GENERATOR_DIRECTED)
        *ctrl_ptr = (uintptr_t)&directed[directed_read_index];

      *rate_factor = g_rate_factor[directed_read_index];

      set_directed_read_index(g_directed_write_index);
      g_directed_write_index = g_directed_write_index ? 0 : 1;
      break;
    }

    default:
      debug_printf("Unrecognised command '%s' with bytes: %d received from host\n", buffer, bytes_read);
      break;
  } //switch (cmd)
}
