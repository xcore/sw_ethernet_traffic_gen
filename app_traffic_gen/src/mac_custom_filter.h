// Copyright (c) 2011, XMOS Ltd, All rights reserved
// This software is freely distributable under a derivative of the
// University of Illinois/NCSA Open Source License posted in
// LICENSE.txt and at <http://github.xcore.com/>

/* The traffic generator doesn't need to receive any traffic */
static inline int mac_custom_filter(unsigned int data[])
{
  return 0;
}
