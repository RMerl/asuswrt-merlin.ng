#************************************
#
#  Copyright (c) 2014  Broadcom
#  All Rights Reserved
#
#***********************************/

all: firmware drivers

include make.proj_flags
include $(TOP_DIR)/make.common
include $(TOP_DIR)/make.dyn_targets

ut: sim coverage firmware drivers
	$(MAKE) -C $(PROJ_UT_TARGET) $@ GEN=$(GEN)

