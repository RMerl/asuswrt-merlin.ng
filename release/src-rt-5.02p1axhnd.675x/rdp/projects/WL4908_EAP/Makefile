#************************************
#
#  Copyright (c) 2014  Broadcom
#  All Rights Reserved
#
#***********************************/

#This is needed because firmware parallel build is broken.
#This causes this instance of Make to execute -j1.
.NOTPARALLEL:

all: firmware drivers

include make.proj_flags
include $(TOP_DIR)/make.common
include $(TOP_DIR)/make.dyn_targets

ut:
	make -C $(TOP_DIR) PROJECT=$(PROJECT) BUILD_FOR_SIM=y clean all
	cd $(PROJECT_DIR)/firmware/ut/capwap && ./capwap_run_tests input_packets/*
	make -C $(TOP_DIR) PROJECT=$(PROJECT) clean fw

