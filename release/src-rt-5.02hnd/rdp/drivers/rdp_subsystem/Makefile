#*********************************************
#
#  Copyright (c) 2014  Broadcom
#  All Rights Reserved
#
#*********************************************/
include $(PROJECT_DIR)/make.proj_flags
include $(TOP_DIR)/make.common

SRCS := $(notdir $(shell ls *.c))
OBJS := $(SRCS:.c=.o)

CFLAGS = $(COMMON_CFLAGS) $(PROJ_CFLAGS)
LIB = $(PROJ_RDP_SUBSYS_TARGET)/librdpsubsys.a

ifeq ("$(CONFIG_SYSTEM)", "sim")
	CFLAGS += -include stdint.h -g -std=gnu99 $(HOST_PLATFORM_FLAGS) -DRDP_SIM
	LFLAGS = $(HOST_PLATFORM_FLAGS) -g 
	ARLAGS = -s 
endif

all: lib
	echo "Done building RDP Subsystem lib"

lib: $(OBJS)
	$(AR) $(LIB) $(OBJS)

$(OBJS): %.o : %.c
	$(CC) $(CFLAGS) $< -o $@ $(LFLAGS)

clean:
	rm -rf $(LIB) $(OBJS)

