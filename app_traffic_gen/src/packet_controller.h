#ifndef PACKET_CONTROLLER_H_
#define PACKET_CONTROLLER_H_

#include "packet_generator.h"

#ifdef __XC__
extern "C" {
#endif

void handle_host_data(unsigned char buffer[], int bytes_read,
    generator_mode_t *generator_mode, uintptr_t *ctrl_ptr, unsigned *rate_factor);

#ifdef __XC__
}
#endif

#endif /* PACKET_CONTROLLER_H_ */
