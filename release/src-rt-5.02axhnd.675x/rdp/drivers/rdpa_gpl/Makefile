#***********************************************************************
#
#  Copyright (c) 2015  Broadcom Corporation
#  All Rights Reserved
#
#
# Makefile for Runner Data Path API GPL (RDPA GPL) kernel module 
#
#***********************************************************************
include $(PROJECT_DIR)/make.proj_flags

obj-$(CONFIG_BCM_RDPA_GPL)	 := rdpa_gpl.o
SRCS     := $(subst rdpa_gpl.mod.c,,$(notdir $(shell ls $(src)/*.c)))
OBJS 	 := $(SRCS:.c=.o)
rdpa_gpl-objs = $(OBJS)

ifneq ($(strip $(CONFIG_BCM_DHD_RUNNER)),)
EXTRA_CFLAGS += -DCONFIG_DHD_RUNNER
endif

ifeq ("$(CONFIG_64BIT)" ,"y")
EXTRA_DEFINES += -DPHYS_ADDR_64BIT
endif

EXTRA_INCLUDES += $(INC_RDP_FLAGS)
EXTRA_INCLUDES += -I$(src)/include -I$(src)/include/$(PLATFORM)

EXTRA_CFLAGS += -Werror $(EXTRA_INCLUDES) $(EXTRA_DEFINES)
EXTRA_CFLAGS += -include rdpa_api.h

