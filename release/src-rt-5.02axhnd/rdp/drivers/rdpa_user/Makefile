#***********************************************************************
#
#  Copyright (c) 2017  Broadcom Corporation
#  All Rights Reserved
#
#
# Makefile for Runner Data Path API USER (RDPA USR) kernel module 
#
#***********************************************************************
include $(PROJECT_DIR)/make.proj_flags

obj-$(CONFIG_BCM_RDPA_USR)	 := rdpa_usr.o
SRCS     := $(subst rdpa_usr.mod.c,,$(notdir $(shell ls $(src)/*.c)))
OBJS 	 := $(SRCS:.c=.o)
rdpa_usr-objs = $(OBJS)

ifeq ("$(CONFIG_64BIT)" ,"y")
EXTRA_DEFINES += -DPHYS_ADDR_64BIT
endif

EXTRA_INCLUDES += $(INC_RDP_FLAGS)
EXTRA_INCLUDES += -I$(src)/include -I$(src)/include/$(PLATFORM) -I$(DRIVERS_DIR)/rdpa_user/


EXTRA_CFLAGS += -Werror $(EXTRA_INCLUDES) $(EXTRA_DEFINES)
