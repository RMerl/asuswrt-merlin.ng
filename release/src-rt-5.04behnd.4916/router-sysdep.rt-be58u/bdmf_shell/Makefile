# This Makefile builds bdmf_shell application that enables
# CLI capabilities in BDMF-based management system (ie, RDPA)
#
EXE=bdmf_shell
OBJS=bdmf_shell_client.o
LIBS = -lpthread

all dynamic install: conditional_build


ifeq ($(BCM_MODULAR_BUILD),)
# Old way: infer location of make.common based on pwd.
CURR_DIR := $(shell pwd)
BUILD_DIR:=$(HND_SRC)
include $(BUILD_DIR)/make.common
else
# New Modular Build way: EXT_BUILD_DIR must be set.
# Also point BUILD_DIR to EXT_BUILD_DIR
BUILD_DIR := $(EXT_BUILD_DIR)
include $(EXT_BUILD_DIR)/make.common
endif


ALLOWED_INCLUDE_PATHS := -I.

# There are lots of include paths and defines in INC_RDP_FLAGS
CUSTOM_CFLAGS += -Werror -Wfatal-errors -DBDMF_SYSTEM_LINUX $(INC_RDP_FLAGS)

SCRIPT_FILE=scripts/xrdp_hw_accel_dump.sh
ifneq ($(findstring _$(strip $(BRCM_CHIP))_,_63138_63148_4908_),)
    SCRIPT_FILE=scripts/rgen30_hw_accel_dump.sh
endif
ifneq ($(findstring _$(strip $(BRCM_CHIP))_,_63146_4912_6855_6813_),)
    SCRIPT_FILE=scripts/rgen61_hw_accel_dump.sh
endif

ifneq ($(strip $(BUILD_BDMF_SHELL)),)
conditional_build: $(EXE)
	mkdir -p $(EXT_DEVICEFS_DIR)/bin
	mkdir -p $(EXT_DEVICEFS_DIR)/rom/rom/etc
	install -m 755 $(EXE) $(EXT_DEVICEFS_DIR)/bin
	install scripts/rdpa_init.sh $(EXT_DEVICEFS_DIR)/rom/rom/etc
	install scripts/bs $(EXT_DEVICEFS_DIR)/bin
	install scripts/bsi $(EXT_DEVICEFS_DIR)/bin
	install scripts/runner $(EXT_DEVICEFS_DIR)/bin
	install scripts/runner_dump.sh $(EXT_DEVICEFS_DIR)/bin
	install $(SCRIPT_FILE) $(EXT_DEVICEFS_DIR)/bin/hw_accel_dump.sh
else
conditional_build:
	@echo "skipping $@ (not configured)"
endif

ifneq ($(wildcard prebuilt/bdmf_shell),)
$(EXE):
	cp prebuilt/bdmf_shell ./bdmf_shell
else
$(EXE): $(OBJS)
	$(CC) $(BCM_LD_FLAGS) -o $@ $(OBJS) $(LIBS)
endif

clean:
	rm -f $(EXT_DEVICEFS_DIR)/bin/$(EXE)
	rm -f $(EXT_DEVICEFS_DIR)/etc/rdpa_init.sh
	rm -f $(EXT_DEVICEFS_DIR)/bin/bs
	rm -f $(EXT_DEVICEFS_DIR)/bin/bsi
	rm -f $(EXT_DEVICEFS_DIR)/bin/runner
	rm -f $(EXT_DEVICEFS_DIR)/bin/runner_dump.sh
	rm -f $(EXT_DEVICEFS_DIR)/bin/hw_accel_dump.sh
	rm -f *.o *.d *.d.* core.* *.a output.txt $(EXE)
