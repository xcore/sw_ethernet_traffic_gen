#include <stddef.h>
#include <stdio.h>
#include <xccompat.h>
#include <stdint.h>
#include <string.h>

#include "xassert.h"
#include "random.h"
#include "common.h"
#include "xc_utils.h"
#include "debug_print.h"
#include "packet_controller.h"
#include "packet_generator.h"

volatile int g_directed_read_index = 0;

unsigned char g_unicast_mac[2][MAC_ADDRESS_BYTES] = {
  { 0x00, 0x22, 0x97, 0x00, 0x42, 0xa2 },
  { 0x00, 0x22, 0x97, 0x00, 0x42, 0xa2 },
};
unsigned char g_multicast_mac[2][MAC_ADDRESS_BYTES] = {
  { 0x01, 0x22, 0x97, 0x00, 0x42, 0xa6 },
  { 0x01, 0x22, 0x97, 0x00, 0x42, 0xa6 },
};
unsigned char g_src_mac[MAC_ADDRESS_BYTES] = { 0, 0, 0, 0, 0, 0 };
unsigned char g_broadcast_addr[MAC_ADDRESS_BYTES] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };

static void fill_pkt_hdr(packet_data_t *pkt_dptr)
{
  static unsigned seq_num = 1;
  pkt_dptr->seq_num[3] = seq_num & 0xFF;
  pkt_dptr->seq_num[2] = (seq_num >> 8) & 0xFF;
  pkt_dptr->seq_num[1] = (seq_num >> 16) & 0xFF;
  pkt_dptr->seq_num[0] = (seq_num >> 24) & 0xFF;
  seq_num++;
}

void gen_unicast_frame(uintptr_t pkt_dptr)
{
  packet_data_t *ptr = (packet_data_t *)pkt_dptr;
  memcpy(ptr->dest_mac, g_unicast_mac[g_directed_read_index], MAC_ADDRESS_BYTES);
  memcpy(ptr->src_mac, g_src_mac, MAC_ADDRESS_BYTES);
  ptr->frame_type[0] = 0x89;
  ptr->frame_type[1] = 0x32;
  fill_pkt_hdr(ptr);
}

void gen_multicast_frame(uintptr_t pkt_dptr)
{
  packet_data_t *ptr = (packet_data_t *)pkt_dptr;
  memcpy(ptr->dest_mac, g_multicast_mac[g_directed_read_index], MAC_ADDRESS_BYTES);
  memcpy(ptr->src_mac, g_src_mac, MAC_ADDRESS_BYTES);
  ptr->frame_type[0] = 0x89;
  ptr->frame_type[1] = 0x33;
  fill_pkt_hdr(ptr);
}

void gen_broadcast_frame(uintptr_t pkt_dptr)
{
  packet_data_t *ptr = (packet_data_t *)pkt_dptr;
  memcpy(ptr->dest_mac, g_broadcast_addr, MAC_ADDRESS_BYTES);
  memcpy(ptr->src_mac, g_broadcast_addr, MAC_ADDRESS_BYTES);
  ptr->frame_type[0] = 0x89;
  ptr->frame_type[1] = 0x34;
  fill_pkt_hdr(ptr);
}

pkt_ctrl_t unicast =   { TYPE_UNICAST,   64, 1500, 20 };
pkt_ctrl_t multicast = { TYPE_MULTICAST, 64, 1500, 50 };
pkt_ctrl_t broadcast = { TYPE_BROADCAST, 64, 1500, 30 };

pkt_ctrl_t *packet_type_none[] = { NULL };
pkt_ctrl_t *packet_type_all[] = { &unicast, &multicast, &broadcast, NULL };

pkt_ctrl_t *packet_type_unicast[] = { &unicast, NULL };
pkt_ctrl_t *packet_type_multicast[] = { &multicast, NULL };
pkt_ctrl_t *packet_type_broadcast[] = { &broadcast, NULL };

pkt_gen_ctrl_t unicast_only;
pkt_gen_ctrl_t multicast_only;
pkt_gen_ctrl_t broadcast_only;

pkt_gen_ctrl_t *choice[] = { &unicast_only, &multicast_only, &broadcast_only, NULL };

pkt_gen_ctrl_t initial = {
  packet_type_none, 1, choice
};

pkt_gen_ctrl_t *choice_initial[] = { &initial, NULL };

pkt_gen_ctrl_t unicast_only = {
  packet_type_unicast, 1, choice_initial
};

pkt_gen_ctrl_t multicast_only = {
  packet_type_multicast, 1, choice_initial
};

pkt_gen_ctrl_t broadcast_only = {
  packet_type_broadcast, 1, choice_initial
};

/* Begin - Directed mode configuration */
pkt_ctrl_t unicast_directed[2] = {
  { TYPE_UNICAST, 64, 64, 35 },
  { TYPE_UNICAST, 64, 1500, 35 }
};
pkt_ctrl_t multicast_directed[2] = {
  { TYPE_MULTICAST, 64, 1500, 0 },
  { TYPE_MULTICAST, 64, 1500, 30 }
};
pkt_ctrl_t broadcast_directed[2] = {
  { TYPE_BROADCAST, 64, 1500, 0 },
  { TYPE_BROADCAST, 64, 1500, 40 }
};

pkt_ctrl_t *packet_type_directed0[] = { &unicast_directed[0], &multicast_directed[0], &broadcast_directed[0], NULL };
pkt_ctrl_t *packet_type_directed1[] = { &unicast_directed[1], &multicast_directed[1], &broadcast_directed[1], NULL };

pkt_gen_ctrl_t directed[2];

pkt_gen_ctrl_t *directed_choice0[] = { &directed[0], NULL };
pkt_gen_ctrl_t *directed_choice1[] = { &directed[1], NULL };

pkt_gen_ctrl_t directed[2] = {
  { packet_type_directed0, 1, directed_choice0 },
  { packet_type_directed1, 1, directed_choice1 }
};

pkt_ctrl_t *get_packet_control(pkt_type_t pkt_type, int index)
{
  switch (pkt_type) {
    case TYPE_UNICAST:
      return &unicast_directed[index];
    case TYPE_MULTICAST:
      return &multicast_directed[index];
    case TYPE_BROADCAST:
      return &broadcast_directed[index];
  }
  assert(0);
  return NULL;
}
/* End - Directed mode configuration */

void set_directed_read_index(int read_index)
{
  g_directed_read_index = read_index;
}

void set_unicast_mac_address(int write_index, unsigned char mac_address[])
{
  memcpy(g_unicast_mac[write_index], mac_address, MAC_ADDRESS_BYTES);
}

void get_unicast_mac_address(int read_index, unsigned char mac_address[])
{
  memcpy(mac_address, g_unicast_mac[read_index], MAC_ADDRESS_BYTES);
}

void set_multicast_mac_address(int write_index, unsigned char mac_address[])
{
  memcpy(g_multicast_mac[write_index], mac_address, MAC_ADDRESS_BYTES);
}

void get_multicast_mac_address(int read_index, unsigned char mac_address[])
{
  memcpy(mac_address, g_multicast_mac[read_index], MAC_ADDRESS_BYTES);
}

pkt_ctrl_t *choose_packet_type(random_generator_t *r, uintptr_t ctrl_ptr, unsigned int *len)
{
  pkt_ctrl_t **choices = ((pkt_gen_ctrl_t *)ctrl_ptr)->packet_types;
  int total_weight = 0;
  pkt_ctrl_t **ptr = choices;
  while (*ptr) {
    total_weight += (*ptr)->weight;
    ptr++;
  }
  if (total_weight == 0)
    return NULL;

  unsigned choice_weight = random_get_random_number(r) % total_weight;
  ptr = choices;
  int cum_weight = 0;
  while (*ptr) {
    cum_weight += (*ptr)->weight;
    if (choice_weight < cum_weight) {
      /* Choose packet length */
      *len = random_get_random_number(r);
      if ((*ptr)->size_max == (*ptr)->size_min)
        *len = (*ptr)->size_min;
      else
        *len = (*len % ((*ptr)->size_max - (*ptr)->size_min)) + (*ptr)->size_min ;
      return *ptr;
    }
    ptr++;
  }
  *len = 0;
  return NULL;
}

uintptr_t choose_next(random_generator_t *r, uintptr_t ctrl_ptr)
{
  pkt_gen_ctrl_t **choices = ((pkt_gen_ctrl_t *)ctrl_ptr)->next;
  int total_weight = 0;
  pkt_gen_ctrl_t **ptr = choices;
  while (*ptr) {
    total_weight += (*ptr)->weight;
    ptr++;
  }
  if (total_weight == 0)
    return (uintptr_t)NULL;

  unsigned choice_weight = random_get_random_number(r) % total_weight;
  ptr = choices;
  int cum_weight = 0;
  while (*ptr) {
    cum_weight += (*ptr)->weight;
    if (choice_weight < cum_weight)
      return (uintptr_t)*ptr;
    ptr++;
  }
  return (uintptr_t)NULL;
}

