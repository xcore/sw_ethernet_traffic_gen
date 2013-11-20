#include <platform.h>
#include <xscope.h>
#include <stdint.h>
#include "debug_print.h"
#include <xclib.h>
#include "xassert.h"
#include "buffers.h"

static inline void process_received(streaming chanend c, int &work_pending,
    buffers_used_t &used_buffers, buffers_free_t &free_buffers, uintptr_t buffer, int &all_buffers_used)
{
  unsigned length_in_bytes;
  c :> length_in_bytes;

   buffers_used_add(used_buffers, buffer, length_in_bytes);
   work_pending++;
   if (buffers_used_full(used_buffers) || free_buffers.top_index == 0) {
     all_buffers_used = 1;
   } else {
     c <: buffers_free_acquire(free_buffers);
   }
}

void buffer_manager(streaming chanend c_prod, streaming chanend c_con)
{
  buffers_used_t used_buffers;
  buffers_used_initialise(used_buffers);

  buffers_free_t free_buffers;
  buffers_free_initialise(free_buffers);

  // Pass two buffers so there is no delay in the manager
  c_prod <: buffers_free_acquire(free_buffers);
  c_prod <: buffers_free_acquire(free_buffers);

  int sender_active = 0;
  int work_pending = 0;
  int all_buffers_used = 0;

  while (1) {
    select {
      case c_prod :> uintptr_t buffer: {
        process_received(c_prod, work_pending, used_buffers, free_buffers, buffer, all_buffers_used);
        break;
      }
      case c_con :> uintptr_t sent_buffer : {
        sender_active--;
        if (all_buffers_used) {
          c_prod <: sent_buffer;
          all_buffers_used = 0;
        } else {
          buffers_free_release(free_buffers, sent_buffer);
        }
        break;
      }
      work_pending && (sender_active < 2) => default : {
        // Send a pointer out to the outputter
        uintptr_t buffer;
        unsigned length_in_bytes;
        {buffer, length_in_bytes} = buffers_used_take(used_buffers);
        c_con <: buffer;
        c_con <: length_in_bytes;
        work_pending--;
        sender_active++;
        break;
      }
    }  //select
  }  //while (1)
}

