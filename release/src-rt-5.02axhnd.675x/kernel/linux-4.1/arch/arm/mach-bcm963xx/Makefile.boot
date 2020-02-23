ifdef BCM_KF # defined(CONFIG_BCM_KF_ARM_BCM963XX)
#
# SDRAM location for decompressor
#
zreladdr-y      := $(CONFIG_BOARD_ZRELADDR)
#
# Where boot monitor is expected to leave parameters
#
params_phys-y   := $(CONFIG_BOARD_PARAMS_PHYS)
endif # BCM_KF # defined(CONFIG_BCM_KF_ARM_BCM963XX)