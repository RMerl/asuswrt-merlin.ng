#***********************************************************************
#
#  Copyright (c) 2014  Broadcom Corporation
#  All Rights Reserved
#
#
#  Makefile for Broadcom Device Management Framework (BDMF), includes build for CPE. CPE build includes BDMF
#  kernel module, BDMF simulator library for x86, and BDMF shell user-space client.
#
#***********************************************************************
include $(PROJECT_DIR)/make.proj_flags
ifeq ("$(CONFIG_SYSTEM)", "sim")
include $(TOP_DIR)/make.common
endif

# Set the following options according to configuration
src 				?= $(shell pwd)
BDMFDIR				?= $(src)
CONFIG_SYSTEM		?= linux
CONFIG_BDMF_SHELL	?= y
CONFIG_REMOTE_SHELL	?= n
CONFIG_DB			?= n
CONFIG_DRIVER_TEST	?= n
CONFIG_BDMF_CODEGEN ?= y
CONFIG_BDMF_HISTORY	?= y
 
REMOTE_SHELL_CC ?= gcc
REMOTE_SHELL_CFLAGS ?= -g -Wall -Werror -I$(BDMFDIR)/system -I$(BDMFDIR)/system/$(CONFIG_SYSTEM)
REMOTE_SHELL_LIBS = -lrt -lpthread 
HOST_PLATFORM_FLAGS ?= -m32 -march=i386 

#
# End of configuration section
#
EXTRA_INCLUDES = $(COMMON_INCLUDE_FLAGS)
EXTRA_INCLUDES += -I$(BDMFDIR) -I$(BDMFDIR)/framework 
EXTRA_INCLUDES += -I$(BDMFDIR)/system -I$(BDMFDIR)/system/$(CONFIG_SYSTEM) 

ifneq (x"$(V)", x"1")
	SILENT_BUILD = @
endif

ifeq ("$(CONFIG_SYSTEM)", "sim")
	CC 	   ?= gcc
	CFLAGS = -g -Wall -Werror -Wno-char-subscripts -std=gnu99 $(HOST_PLATFORM_FLAGS) $(EXTRA_OPTIONS) $(EXTRA_INCLUDES) $(EXTRA_DEFINES) $(PROJ_DEFS)
	LFLAGS = $(HOST_PLATFORM_FLAGS) -g 
	LIBS   = -lrt -lpthread
	EXTRA_DEFINES += -DBDMF_SYSTEM_SIM
	REMOTE_SHELL_CFLAGS += -DBDMF_SYSTEM_SIM
	CONFIG_REMOTE_SHELL = y
	CONFIG_LINENOISE ?= y
	CONFIG_LIBEDIT ?= n
	CC_CMD = $(CC) -c $(CFLAGS) -o
endif

OBJS = 	$(BDMF_OUTDIR)/framework/bdmf_init.o
OBJS += $(SYSTEM_OBJS) $(FRAMEWORK_OBJS) $(DRIVER_OBJS)
OBJS += $(BDMF_OUTDIR)/framework/bdmf_fini.o

ifeq ("$(CONFIG_SYSTEM)", "linux")
	CONFIG_BDMF_CODEGEN := n
	EXTRA_CFLAGS = -Werror $(INC_RDP_FLAGS) $(EXTRA_INCLUDES) $(EXTRA_DEFINES) $(PROJ_DEFS)
	SYSTEM_OBJS += system/linux/bdmf_chrdev.o
	obj-$(CONFIG_BCM_BDMF) = bdmf.o
ifneq ($(PREBUILT_EXTRAMOD),1)	
	bdmf-objs = $(OBJS)
else
	bdmf-objs := bdmf$(PRBM_EXT).o
endif
	EXTRA_DEFINES += -DBDMF_SYSTEM_LINUX
	REMOTE_SHELL_CFLAGS += -DBDMF_SYSTEM_LINUX
	REMOTE_SHELL_CC = $(CROSS)gcc
	REMOTE_SHELL_DEPS += $(BDMFDIR)/system/$(CONFIG_SYSTEM)/bdmf_chrdev.h
	BDMF_OUTDIR = .
ifeq ("$(CONFIG_64BIT)" ,"y")
	PROJ_DEFS += -DPHYS_ADDR_64BIT
endif
endif

FRAMEWORK_OBJS = \
	$(BDMF_OUTDIR)/framework/bdmf_attr.o \
	$(BDMF_OUTDIR)/framework/bdmf_memarea.o \
	$(BDMF_OUTDIR)/framework/bdmf_utils.o \
	$(BDMF_OUTDIR)/framework/bdmf_type.o \
	$(BDMF_OUTDIR)/framework/bdmf_object.o \
	$(BDMF_OUTDIR)/framework/bdmf_trace.o

SYSTEM_OBJS += \
	$(BDMF_OUTDIR)/system/bdmf_session.o \
	$(BDMF_OUTDIR)/system/bdmf_errno.o

SRCS = $(OBJS:.o=.c)
DOC_HDRS = 	$(wildcard framework/*.h)
DOC_HDRS += $(wildcard shell/*.h)
DOC_HDRS += $(wildcard db_engine/*.h)
DOC_HDRS += $(wildcard transport/*.h)
REFMAN_DIR = doc/latex
REFMAN_FILE = bdmf_refman.pdf
REFMAN = $(REFMAN_DIR)/$(REFMAN_FILE)

ifeq ("$(CONFIG_DB)", "y")
	EXTRA_DEFINES += -DBDMF_DB_ENGINE
	EXTRA_INCLUDES += -I$(BDMFDIR)/db_engine
	FRAMEWORK_OBJS += $(BDMF_OUTDIR)/db_engine/db_engine.o
endif

ifeq ("$(CONFIG_BDMF_SHELL)", "y")
	EXTRA_DEFINES += -DBDMF_SHELL
	EXTRA_INCLUDES += -I$(BDMFDIR)/shell
	FRAMEWORK_OBJS += $(BDMF_OUTDIR)/framework/bdmf_mon.o
	SYSTEM_OBJS += $(BDMF_OUTDIR)/shell/bdmf_shell.o 
	ifeq ("$(CONFIG_REMOTE_SHELL)", "y")
		EXTRA_DEFINES += -DBDMF_SHELL_SERVER
		SYSTEM_OBJS += $(BDMF_OUTDIR)/shell/bdmf_shell_server.o 
	endif
endif

ifeq ("$(CONFIG_LINENOISE)", "y")
	EXTRA_DEFINES += -DCONFIG_LINENOISE -DLINENOISE_DISABLE_HIST_SAVE
	SYSTEM_OBJS += $(BDMF_OUTDIR)/shell/linenoise/linenoise.o 
	EXTRA_INCLUDES += -I$(BDMFDIR)/shell/linenoise
	CONFIG_LIBEDIT = n
endif

ifeq ("$(CONFIG_LIBEDIT)", "y")
	EXTRA_DEFINES += -DCONFIG_LIBEDIT
	LIBS +=  -ledit -ltermcap
endif

ifeq ("$(CONFIG_DRIVER_TEST)", "y")
	DRIVER_OBJS += $(BDMF_OUTDIR)/drivers/test1_drv.o $(BDMF_OUTDIR)/drivers/test2_drv.o
endif

ifeq ("$(CONFIG_SYSTEM)", "sim")
	SYSTEM_OBJS += $(BDMF_OUTDIR)/system/$(CONFIG_SYSTEM)/bdmf_buf.o $(BDMF_OUTDIR)/system/$(CONFIG_SYSTEM)/bdmf_system.o
	LFLAGS += -lpthread
else
	SYSTEM_OBJS +=$(BDMF_OUTDIR)/system/$(CONFIG_SYSTEM)/bdmf_system_common.o
	SYSTEM_OBJS +=$(BDMF_OUTDIR)/system/$(CONFIG_SYSTEM)/ce/bdmf_system.o
	EXTRA_INCLUDES +=-I$(BDMFDIR)/system/$(CONFIG_SYSTEM)/ce
endif

ifeq ("$(CONFIG_BDMF_DEBUG)", "y")
	EXTRA_DEFINES += -DBDMF_DEBUG
endif

ifeq ("$(CONFIG_BDMF_CODEGEN)", "y")
	EXTRA_DEFINES += -DBDMF_CODEGEN
	FRAMEWORK_OBJS += $(BDMF_OUTDIR)/framework/bdmf_codegen.o
endif

ifeq ("$(CONFIG_BDMF_HISTORY)", "y")
	EXTRA_DEFINES += -DBDMF_HISTORY
	FRAMEWORK_OBJS += $(BDMF_OUTDIR)/framework/bdmf_history.o
endif

# 
# Simulation build
#
ifeq ("$(CONFIG_SYSTEM)", "sim")

bdmf_sim: $(BDMF_OUTDIR)/libbdmf.a $(BDMF_OUTDIR)/bdmf

$(BDMF_OUTDIR)/.dir_created: 
	mkdir -p $(BDMF_OUTDIR)/
	mkdir -p $(BDMF_OUTDIR)/framework
	mkdir -p $(BDMF_OUTDIR)/transport
	mkdir -p $(BDMF_OUTDIR)/system/sim
	mkdir -p $(BDMF_OUTDIR)/shell
	$(SILENT_BUILD)touch $(BDMF_OUTDIR)/.dir_created

-include $(OBJS:.o=.d)

$(BDMF_OUTDIR)/%.d: %.c Makefile $(BDMF_OUTDIR)/.dir_created
	@echo "Making dependency for $< --> $@"
	$(SILENT_BUILD)$(CC) -MM -MT $(BDMF_OUTDIR)/$(<:.c=.o) $(CFLAGS) $< | sed 's,\($*\.o\)[ :]*,\1 $@ : ,g' > $@

$(BDMF_OUTDIR)/%.o: %.c $(BDMF_OUTDIR)/.dir_created
	@echo "CC $< --> $@"
	$(SILENT_BUILD)$(CC_CMD) $@ $<

endif

$(BDMF_OUTDIR)/bdmf: $(OBJS) main.o
	@echo LD $(BDMF_OUTDIR)bdmf 
	$(SILENT_BUILD)$(CC) -o $@ $(LFLAGS) $(OBJS) main.o $(LIBS)

libbdmf.a: $(BDMF_OUTDIR)/libbdmf.a

bdmf_test: $(BDMF_OUTDIR)/bdmf

$(BDMF_OUTDIR)/libbdmf.a: $(OBJS)
	$(SILENT_BUILD)$(AR) $@ $(OBJS)

docs: $(REFMAN) 

$(REFMAN): $(DOC_HDRS) Doxyfile DoxygenLayout.xml
	$(SILENT_BUILD)doxygen
	$(SILENT_BUILD)make -C $(REFMAN_DIR)
	$(SILENT_BUILD)mv -f $(REFMAN_DIR)/refman.pdf $(REFMAN)

clean:
	$(SILENT_BUILD)rm -fr `find $(BDMF_OUTDIR)/. -name '*.[o,d]'`
	$(SILENT_BUILD)rm -fr `find . -name '*~'`
	$(SILENT_BUILD)rm -fr `find . -name '*.ko'`
	$(SILENT_BUILD)rm -fr `find . -name '.*.cmd'`
	$(SILENT_BUILD)rm -fr bdmf.mod.c modules.order Module.symvers .tmp_versions
	$(SILENT_BUILD)rm -fr $(BDMF_OUTDIR)/bdmf $(BDMF_OUTDIR)/bdmf_shell $(BDMF_OUTDIR)/libbdmf.a

clobber: clean
	$(SILENT_BUILD)rm -fr doc
