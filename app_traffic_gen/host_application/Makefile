ifeq ($(OS),WINDOWS)
WINFLAG=1
else
ifeq ($(findstring Windows, $(OS)),Windows)
WINFLAG=1
else
WINFLAG=0
endif
endif


APP_NAME = traffic_gen_controller

ROOT = ../../..
XSCOPE_HOST = $(ROOT)/sc_xscope_support/host_library
MODULE_PCAP = $(ROOT)/sw_ethernet_tap/module_pcapng

INCLUDES  = -I$(MODULE_PCAP)/src -I$(XSCOPE_HOST) -I../src
FLAGS = -O2 $(INCLUDES) -DHOST_APP_PROMPT

SOURCES = $(XSCOPE_HOST)/xscope_host_shared.c $(APP_NAME).c
ifeq ($(WINFLAG),1)
SOURCES += $(XSCOPE_HOST)/getopt.c $(XSCOPE_HOST)/inet_pton.c
endif


ifeq ($(WINFLAG),1)
all:
	cl $(FLAGS) $(SOURCES)
else
all:
	gcc $(FLAGS) $(SOURCES) -o $(APP_NAME)
endif
