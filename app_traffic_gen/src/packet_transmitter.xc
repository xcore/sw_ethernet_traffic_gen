#include <stddef.h>
#include <stdio.h>
#include <xccompat.h>
#include <stdint.h>
#include "debug_print.h"
#include "common.h"
#include "random.h"
#include "xc_utils.h"
#include "c_utils.h"

void packet_transmitter(chanend c_tx, streaming chanend c_con)
{
    while (1) {
      uintptr_t dptr;
      unsigned length_in_bytes;
      unsigned delay;

      c_con :> dptr;
      c_con :> length_in_bytes;

      asm volatile("ldw %0, %1[0]":"=r"(delay):"r"(dptr));
      wait(delay);

      /* Increment dptr to point to actual pkt data */
      send_ether_frame(c_tx, dptr + sizeof(delay), length_in_bytes - sizeof(delay));

      /* Release the buffer */
      c_con <: dptr;
    }
}

