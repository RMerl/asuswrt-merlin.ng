# SPDX-License-Identifier: GPL-2.0+
#
# Copyright (c) 2011 The Chromium OS Authors.
#
# (C) Copyright 2000-2003
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.

obj-y	:= cache.o cpu.o state.o
extra-y	:= start.o os.o
extra-$(CONFIG_SANDBOX_SDL)    += sdl.o
obj-$(CONFIG_SPL_BUILD)	+= spl.o
obj-$(CONFIG_ETH_SANDBOX_RAW)	+= eth-raw-os.o

# os.c is build in the system environment, so needs standard includes
# CFLAGS_REMOVE_os.o cannot be used to drop header include path
quiet_cmd_cc_os.o = CC $(quiet_modtag)  $@
cmd_cc_os.o = $(CC) $(filter-out -nostdinc, \
	$(patsubst -I%,-idirafter%,$(c_flags))) -c -o $@ $<

$(obj)/os.o: $(src)/os.c FORCE
	$(call if_changed_dep,cc_os.o)

# eth-raw-os.c is built in the system env, so needs standard includes
# CFLAGS_REMOVE_eth-raw-os.o cannot be used to drop header include path
quiet_cmd_cc_eth-raw-os.o = CC $(quiet_modtag)  $@
cmd_cc_eth-raw-os.o = $(CC) $(filter-out -nostdinc, \
	$(patsubst -I%,-idirafter%,$(c_flags))) -c -o $@ $<

$(obj)/eth-raw-os.o: $(src)/eth-raw-os.c FORCE
	$(call if_changed_dep,cc_eth-raw-os.o)

# sdl.c fails to build with -fshort-wchar using musl
cmd_cc_sdl.o = $(CC) $(filter-out -nostdinc -fshort-wchar, \
	$(patsubst -I%,-idirafter%,$(c_flags))) -fno-lto -c -o $@ $<

$(obj)/sdl.o: $(src)/sdl.c FORCE
	$(call if_changed_dep,cc_sdl.o)
