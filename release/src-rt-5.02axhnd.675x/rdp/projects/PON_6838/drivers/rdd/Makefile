#***************************************************
#
#  Copyright (c) 2014  Broadcom
#  All Rights Reserved
#
#***************************************************/

include $(PROJECT_DIR)/make.proj_flags
include $(TOP_DIR)/make.common

SRCS := $(notdir $(shell ls *.c))
OBJS := $(SRCS:.c=.o)

CFLAGS = $(COMMON_CFLAGS) $(PROJ_CFLAGS)
LIB = $(PROJ_RDD_TARGET)/librdd.a

ifeq ("$(CONFIG_SYSTEM)", "sim")
	CFLAGS += -g -std=gnu99 -m32 -march=i386
	LFLAGS = -m32 -march=i386 -g 
	ARLAGS = -s 
endif

all: lib
	echo "Done building RDD lib"

lib: $(OBJS)
	$(AR) $(LIB) $(OBJS)

$(OBJS): %.o : %.c 
	$(CC) $(CFLAGS) $< -o $@ $(LFLAGS)

clean:
	rm -rf $(LIB) $(OBJS)

