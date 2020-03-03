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

ut: hw_dts_gen sim firmware drivers
	$(MAKE) -C $(PROJ_UT_TARGET) $@ GEN=$(GEN)

