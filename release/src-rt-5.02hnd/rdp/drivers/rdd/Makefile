#**********************************************
#
#  Copyright (c) 2014  Broadcom
#  All Rights Reserved
#
#*********************************************/
include $(PROJECT_DIR)/make.proj_flags
include $(TOP_DIR)/make.common

SRCS := $(notdir $(shell ls *.c))
OBJS := $(SRCS:.c=.o)

CFLAGS = $(COMMON_CFLAGS) $(PROJ_CFLAGS) -DRDP_SIM
LIB = $(PROJ_RDD_TARGET)/librdd.a

ifeq ("$(CONFIG_SYSTEM)", "sim")
	CFLAGS += -g -std=gnu99 $(HOST_PLATFORM_FLAGS)
	LFLAGS = $(HOST_PLATFORM_FLAGS) -g
	ARLAGS = -s
endif

all: lib
	echo "Done building RDD lib"

validate:
	$(VALIDATE_FILES) *.[h,c]

lib: validate $(OBJS)
	$(AR) $(LIB) $(OBJS)

$(OBJS): %.o : %.c
	ls $<
	$(CC) $(CFLAGS) $< -o $@ $(LFLAGS)

clean:
	rm -rf $(LIB) $(OBJS)
