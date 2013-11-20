#include <xs1.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <xccompat.h>

#include "c_utils.h"
#include "ethernet.h"
#include "packet_generator.h"

void send_ether_frame(chanend c_tx, uintptr_t dptr, unsigned int nbytes)
{
  mac_tx(c_tx, (unsigned int *)dptr, nbytes, ETH_BROADCAST);
}

char get_next_char(const unsigned char **buffer)
{
  const unsigned char *ptr = *buffer;
  while (*ptr && isspace(*ptr))
    ptr++;

  *buffer = ptr + 1;
  return *ptr;
}

int convert_atoi_substr(const unsigned char **buffer)
{
  const unsigned char *ptr = *buffer;
  unsigned int value = 0;
  while (*ptr && isspace(*ptr))
    ptr++;

  if (*ptr == '\0')
    return 0;

  value = atoi((char*)ptr);

  while (*ptr && !isspace(*ptr))
    ptr++;

  *buffer = ptr;
  return value;
}

/* Parse a MAC address of the form aa:bb:cc:dd:ee:ff
 * Returns 0 on successful parsing, 1 otherwise */
int parse_mac_address(const unsigned char *ptr, unsigned char mac_address[])
{
  int index = 0;

  while (*ptr && index < MAC_ADDRESS_BYTES) {
    char *end = NULL;
    while (isspace(*ptr))
      ptr++;

    // Assume the bytes are hex
    int byte = strtol((const char *)ptr, &end, 16);

    // If the pointer hasn't moved then there is an error
    if ((unsigned char *)end == ptr)
      return 1;

    mac_address[index] = byte;

    ptr = (unsigned char *)end;
    while (isspace(*ptr))
      ptr++;

    if (*ptr) {
      if (*ptr == ':') {
        index++;
        ptr++;
      } else {
        return 1;
      }
    }
  }
  return 0;
}
