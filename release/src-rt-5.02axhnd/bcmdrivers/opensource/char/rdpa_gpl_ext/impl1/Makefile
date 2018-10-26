# File: 
#
# Makefile for RDPA GPL EXT driver
#

obj-$(CONFIG_BCM_RDPA_GPL_EXT)	 := rdpa_gpl_ext.o
SRCS     := $(subst rdpa_gpl_ext.mod.c,,$(notdir $(shell ls $(src)/*.c)))
ifeq ($(PREBUILT_EXTRAMOD),1)
OBJS     := rdpa_gpl_ext_preb.o
else
OBJS 	 := $(SRCS:.c=.o)
endif
rdpa_gpl_ext-objs = $(OBJS)

ifneq (,$(filter "y","$(CONFIG_BCM96838)" "$(CONFIG_BCM96848)"))
	PLATFORM=oren
	#EXTRA_INCLUDES += -I$(INC_BRCMSHARED_PUB_PATH)/rdp -I$(INC_BRCMSHARED_PUB_PATH)/rdp/base
else ifeq ("$(CONFIG_BCM963138)" ,"y")
	PLATFORM=dsl
	#EXTRA_INCLUDES += -I$(INC_BRCMSHARED_PUB_PATH)/rdp -I$(INC_BRCMSHARED_PUB_PATH)/rdp/base -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD) -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD)
else ifeq ("$(CONFIG_BCM963148)" ,"y")
	PLATFORM=dsl
	#EXTRA_INCLUDES += -I$(INC_BRCMSHARED_PUB_PATH)/rdp -I$(INC_BRCMSHARED_PUB_PATH)/rdp/base -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD) -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD)
else ifeq ("$(CONFIG_BCM94908)" ,"y")
	PLATFORM=dsl
	#EXTRA_INCLUDES += -I$(INC_BRCMSHARED_PUB_PATH)/rdp -I$(INC_BRCMSHARED_PUB_PATH)/rdp/base -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD) -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD)
else ifeq ("$(CONFIG_BCM96858)" ,"y")
	PLATFORM=BCM6858
else ifeq ("$(CONFIG_BCM96836)" ,"y")
	PLATFORM=BCM6836
else ifeq ("$(CONFIG_BCM96846)" ,"y")
	PLATFORM=BCM6846
else ifeq ("$(CONFIG_BCM96856)" ,"y")
	PLATFORM=BCM6856
else
	PLATFORM=sim
endif

ifneq ($(strip $(CONFIG_BCM_DHD_RUNNER)),)
EXTRA_CFLAGS += -DCONFIG_DHD_RUNNER
endif

EXTRA_CFLAGS += $(INC_RDP_FLAGS)

#EXTRA_INCLUDES += -I$(INC_BDMF_PATH)/framework -I$(INC_BDMF_PATH)/system -I$(INC_BDMF_PATH)/system/linux -I$(INC_BDMF_PATH)/system/linux/ce
#EXTRA_INCLUDES += -I$(INC_BDMF_PATH)/platform/$(PLATFORM)
#EXTRA_INCLUDES += -I$(src)/include -I$(src)/include/$(PLATFORM)
#EXTRA_INCLUDES += -I$(INC_BRCMSHARED_PUB_PATH)/bcm963xx
EXTRA_CFLAGS += -Werror $(EXTRA_INCLUDES) $(EXTRA_DEFINES)
EXTRA_CFLAGS += -include $(RDPSDK_DIR)/drivers/rdpa_gpl/include/rdpa_api.h

-include $(TOPDIR)/Rules.make

clean:
	rm -rf `find . -name '*.[o,d]'`
	rm -rf `find . -name '*~'`
	rm -rf `find . -name '*.ko'`
	rm -rf rdpa_gpl_ext.mod.c modules.order Module.symvers .tmp_versions
	rm -rf `find . -name '.*.cmd'`
	rm -rf rdpa
