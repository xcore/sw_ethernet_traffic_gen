#ifndef __PACKET_GENERATOR_H__
#define __PACKET_GENERATOR_H__

#include <xccompat.h>
#include <stdint.h>
#include "random.h"

#define MAC_ADDRESS_BYTES 6

typedef enum {
  GENERATOR_SILENT,
  GENERATOR_RANDOM,
  GENERATOR_DIRECTED,
} generator_mode_t;

typedef struct packet_data_t {
  unsigned delay;
  char dest_mac[MAC_ADDRESS_BYTES];
  char src_mac[MAC_ADDRESS_BYTES];
  char frame_type[2];
  char seq_num[4];
} packet_data_t;

typedef struct packet_data_vlan_t {
  unsigned delay;
  char dest_mac[MAC_ADDRESS_BYTES];
  char src_mac[MAC_ADDRESS_BYTES];
  char tpid[2];
  char vlan_prio[2];
  char frame_type[2];
  char seq_num[4];
} packet_data_vlan_t;

typedef enum {
  TYPE_UNICAST,
  TYPE_MULTICAST,
  TYPE_BROADCAST,
} pkt_type_t;

typedef struct pkt_ctrl_t {
    pkt_type_t type;
    unsigned int size_min;
    unsigned int size_max;
    int weight;

    unsigned int vlan_tag_enabled;
    unsigned int vlan;
    unsigned int prio;
} pkt_ctrl_t;

#ifdef __XC__
extern "C" {
#endif

typedef struct pkt_gen_ctrl_t {
    pkt_ctrl_t **packet_types;
    int weight;
    struct pkt_gen_ctrl_t **next;
} pkt_gen_ctrl_t;

pkt_ctrl_t *choose_packet_type(random_generator_t *r, uintptr_t ctrl_ptr, unsigned int *len);
uintptr_t choose_next(random_generator_t *r, uintptr_t ctrl_ptr);
void gen_unicast_frame(uintptr_t pkt_dptr, pkt_ctrl_t *ctrl);
void gen_multicast_frame(uintptr_t pkt_dptr, pkt_ctrl_t *ctrl);
void gen_broadcast_frame(uintptr_t pkt_dptr, pkt_ctrl_t *ctrl);

#ifdef __XC__
}
#endif

void set_directed_read_index(int read_index);
void set_unicast_mac_address(int write_index, unsigned char mac_address[]);
void get_unicast_mac_address(int read_index, unsigned char mac_address[]);
void set_multicast_mac_address(int write_index, unsigned char mac_address[]);
void get_multicast_mac_address(int read_index, unsigned char mac_address[]);
#ifndef __XC__
pkt_ctrl_t *get_packet_control(pkt_type_t pkt_type, int index);
#endif //__XC__

#endif // __PACKET_GENERATOR_H__
