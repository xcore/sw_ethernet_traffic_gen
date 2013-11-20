MII/MAC traffic generator
=========================

:scope: test application
:description: A test application to stress mii and mac layer
:keywords: ethernet, mac, mii

A random traffic generator application which can deliver packets at anything from 1%-100% of
line rate. 

The traffic generator is run using:
  xrun --xscope-realtime --xscope-port 127.0.0.1:12346 bin/test_mii_packetgen.xe

And the host controller is found in host_application
Which is run using from that folder using:

   ./traffic_gen_controller -s 127.0.0.1 -p 12346

or if using these default values then simply:

   ./traffic_gen_controller
