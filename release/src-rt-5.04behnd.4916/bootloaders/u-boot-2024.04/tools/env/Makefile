# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2002-2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.

# fw_printenv is supposed to run on the target system, which means it should be
# built with cross tools. Although it may look weird, we only replace "HOSTCC"
# with "CC" here for the maximum code reuse of scripts/Makefile.host.
override HOSTCC = $(CC)

# Compile for a hosted environment on the target
HOST_EXTRACFLAGS  = -I$(srctree)/tools \
		$(patsubst -I%,-idirafter%, $(filter -I%, $(UBOOTINCLUDE))) \
		-idirafter $(srctree)/tools/env \
		-DUSE_HOSTCC \
		-DTEXT_BASE=$(TEXT_BASE)

ifeq ($(MTD_VERSION),old)
HOST_EXTRACFLAGS += -DMTD_OLD
endif

always := fw_printenv
hostprogs-y := fw_printenv

lib-y += fw_env.o \
	crc32.o ctype.o linux_string.o \
	env_attr.o env_flags.o

fw_printenv-objs := fw_env_main.o $(lib-y)

quiet_cmd_crosstools_strip = STRIP   $^
      cmd_crosstools_strip = $(STRIP) $^; touch $@

$(obj)/.strip: $(obj)/fw_printenv
	$(call cmd,crosstools_strip)

always += .strip
