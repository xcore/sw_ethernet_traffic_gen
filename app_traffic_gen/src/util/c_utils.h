#ifndef __C_UTILS_H__
#define __C_UTILS_H__

#include <xccompat.h>

void send_ether_frame(chanend c_tx, uintptr_t dptr, unsigned int nbytes);
char get_next_char(const unsigned char **buffer);
int convert_atoi_substr(const unsigned char **buffer);
int parse_mac_address(const unsigned char *buffer, unsigned char mac[]);

#endif // __C_UTILS_H__
