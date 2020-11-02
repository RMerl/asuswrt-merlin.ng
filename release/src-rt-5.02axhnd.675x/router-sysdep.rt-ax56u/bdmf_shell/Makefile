# This Makefile builds bdmf_shell application that enables
# CLI capabilities in BDMF-based management system (ie, RDPA)
#
EXE=bdmf_shell
OBJS=bdmf_shell_client.o
LIBS = -lpthread

all dynamic install: build

CURR_DIR := $(shell pwd)
BUILD_DIR:=$(HND_SRC)
BDMF_DIR=$(BRCMDRIVERS_DIR)/opensource/char/bdmf/bcm9$(BRCM_CHIP)

include $(BUILD_DIR)/make.common

ALLOWED_INCLUDE_PATHS := -I.
ALLOWED_LIB_DIRS := /lib:/lib/private:/lib/public
CUSTOM_CFLAGS += -Werror -Wfatal-errors -DBDMF_SYSTEM_LINUX $(INC_RDP_FLAGS)

ifneq ($(strip $(BUILD_BDMF_SHELL)),)
build: $(EXE) generic_exe_install
	install scripts/rdpa_init.sh $(INSTALL_DIR)/rom/rom/etc
	install scripts/bs $(INSTALL_DIR)/bin
	install scripts/bsi $(INSTALL_DIR)/bin
	install scripts/runner $(INSTALL_DIR)/bin	
else
build:
	@echo "skipping $@ (not configured)"
endif

$(EXE): $(OBJS)
	$(CC) $(BCM_LD_FLAGS) -o $@ $(OBJS) $(LIBS)

clean: generic_clean
	rm -f $(INSTALL_DIR)/bin/$(EXE)

