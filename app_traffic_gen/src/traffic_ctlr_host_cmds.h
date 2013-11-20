#ifndef TRAFFIC_GEN_HOST_CMDS_H_
#define TRAFFIC_GEN_HOST_CMDS_H_

typedef char tester_command_t;

enum {
  CMD_SET_MAC_ADDRESS          = 'a',
  CMD_PKT_CONTROL              = 'c',
  CMD_SET_GENERATOR_MODE       = 'm',
  CMD_PRINT_PKT_CONFIGURATION  = 'p',
  CMD_APPLY_CFG                = 'e',
  CMD_SWAP_CFG                 = 's',
  CMD_LINE_RATE                = 'r',
  CMD_QUIT                     = 'q'
};

#endif /* TRAFFIC_GEN_HOST_CMDS_H_ */
