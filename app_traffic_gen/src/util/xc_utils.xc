#include <xs1.h>
#include "xc_utils.h"

void wait(unsigned delay)
{
    if (delay == 0)
        return;

    timer t;
    int time;

    t :> time;
    t when timerafter(time + delay) :> void;
}

