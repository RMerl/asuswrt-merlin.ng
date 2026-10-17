# SPDX-License-Identifier: GPL-2.0+

head-y := arch/sandbox/cpu/start.o arch/sandbox/cpu/os.o
head-$(CONFIG_SANDBOX_SDL) += arch/sandbox/cpu/sdl.o
libs-y += arch/sandbox/cpu/
libs-y += arch/sandbox/lib/

# sdl.c fails to compile with -fshort-wchar using musl.
cmd_cc_sdl.o = $(CC) $(filter-out -nostdinc -fshort-wchar, \
	$(patsubst -I%,-idirafter%,$(c_flags))) -fno-lto -c -o $@ $<

$(obj)/sdl.o: $(src)/sdl.c FORCE
	$(call if_changed_dep,cc_sdl.o)
