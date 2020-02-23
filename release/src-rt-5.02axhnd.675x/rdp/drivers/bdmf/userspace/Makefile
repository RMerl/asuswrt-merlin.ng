# This Makefile builds bdmf_shell application that enables
# CLI capabilities in BDMF-based management system (ie, RDPA)
#

ifeq ($(strip $(TOP_DIR)),)
TOP_DIR := $(shell cd ../../../../../ && pwd)
endif

ifeq ($(strip $(PROJECT)),)
PROJECT = $(shell cd ../../../ && basename `pwd`)
endif

include $(TOP_DIR)/make.common

SRCS := $(notdir $(shell ls *.c))
OBJS := $(SRCS:.c=.o)

CFLAGS = $(COMMON_CFLAGS) -I$(DRIVERS_DIR)/bdmf/system/linux -c -Wall -Werror -ggdb -DFIRMWARE_INIT -DFIRMWARE_LITTLE_ENDIAN -D$(PLATFORM) -DCONFIG_$(CHIP_TYPE) -DUSE_BDMF_SHELL

all: bdmf_shell
	echo "Done building bdmf_shell board application"

bdmf_shell: $(OBJS)
	$(CC) -o $@ $(OBJS) $(LIBS)

$(OBJS): %.o : %.c 
	$(CC) $(CFLAGS) $< -o $@


clean: 
	rm -f $(OBJS) bdmf_shell

