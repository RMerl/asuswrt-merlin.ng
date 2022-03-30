# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2000-2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.

obj-y = cpu_info.o
obj-y += emac.o

obj-$(CONFIG_DISPLAY_BOARDINFO) += board.o
obj-$(CONFIG_GLOBAL_TIMER) += timer.o
obj-$(CONFIG_TMU_TIMER) += ../../sh/lib/time.o
obj-$(CONFIG_SH73A0) += lowlevel_init.o cpu_info-sh73a0.o pfc-sh73a0.o
obj-$(CONFIG_R8A7740) += lowlevel_init.o cpu_info-r8a7740.o pfc-r8a7740.o
obj-$(CONFIG_RCAR_GEN2) += lowlevel_init_ca15.o cpu_info-rcar.o
obj-$(CONFIG_RCAR_GEN3) += lowlevel_init_gen3.o cpu_info-rcar.o memmap-gen3.o

OBJCOPYFLAGS_u-boot-spl.srec := -O srec
quiet_cmd_objcopy = OBJCOPY $@
cmd_objcopy = $(OBJCOPY) --gap-fill=0x00 $(OBJCOPYFLAGS) \
	$(OBJCOPYFLAGS_$(@F)) $< $@

spl/u-boot-spl.srec: spl/u-boot-spl FORCE
	$(call if_changed,objcopy)

ifneq ($(CONFIG_R8A77990)$(CONFIG_R8A77995),)
#
# The first 6 generate statements generate the R-Car Gen3 SCIF loader header.
# The subsequent generate statements represent the following chunk of assembler
# code, which copies the loaded data from 0xe6304030 to 0xe6318000. This is to
# work around a limitation of the D3/E3 BootROM, which does not permit loading
# to 0xe6318000 directly.
#
#	mov	x0,	#0xe6000000
#	orr	x0, x0,	#0x00300000
#	orr	x1, x0,	#0x00004000
#	orr	x1, x1,	#0x00000030
#
#	orr	x2, x0,	#0x00018000
#	mov	x0, x2
#	mov	x3,	#0x7000
#1:	ldp	x4, x5, [x1], #16
#
#	stp	x4, x5, [x2], #16
#	subs	x3, x3, #16
#	b.ge	1b
#	br	x0
#
quiet_cmd_srec_cat = SRECCAT $@
      cmd_srec_cat = srec_cat -output $@ -M 8 $< -M 8 \
			-offset -0x13fd0 \
			-Output_Block_Size 16 \
			-generate 0xe6300400 0xe6300404 -l-e-constant 0x0 4 \
			-generate 0xe630048c 0xe6300490 -l-e-constant 0x0 4 \
			-generate 0xe63005d4 0xe63005d8 -l-e-constant 0xe6304000 4 \
			-generate 0xe63006e4 0xe63006e8 -l-e-constant $2 4 \
			-generate 0xe6301154 0xe6301158 -l-e-constant 0xe6304000 4 \
			-generate 0xe6301264 0xe6301268 -l-e-constant $2 4 \
			-generate 0xe6304000 0xe6304004 -l-e-constant 0xd2bcc000 4 \
			-generate 0xe6304004 0xe6304008 -l-e-constant 0xb26c0400 4 \
			-generate 0xe6304008 0xe630400c -l-e-constant 0xb2720001 4 \
			-generate 0xe630400c 0xe6304010 -l-e-constant 0xb27c0421 4 \
			-generate 0xe6304010 0xe6304014 -l-e-constant 0xb2710402 4 \
			-generate 0xe6304014 0xe6304018 -l-e-constant 0xaa0203e0 4 \
			-generate 0xe6304018 0xe630401c -l-e-constant 0xd28e0003 4 \
			-generate 0xe630401c 0xe6304020 -l-e-constant 0xa8c11424 4 \
			-generate 0xe6304020 0xe6304024 -l-e-constant 0xa8811444 4 \
			-generate 0xe6304024 0xe6304028 -l-e-constant 0xf1004063 4 \
			-generate 0xe6304028 0xe630402c -l-e-constant 0x54ffffaa 4 \
			-generate 0xe630402c 0xe6304030 -l-e-constant 0xd61f0000 4
else
quiet_cmd_srec_cat = SRECCAT $@
      cmd_srec_cat = srec_cat -output $@ -M 8 $< -M 8 \
			-Output_Block_Size 16 \
			-generate 0xe6300400 0xe6300404 -l-e-constant 0x0 4 \
			-generate 0xe630048c 0xe6300490 -l-e-constant 0x0 4 \
			-generate 0xe63005d4 0xe63005d8 -l-e-constant $(CONFIG_SPL_TEXT_BASE) 4 \
			-generate 0xe63006e4 0xe63006e8 -l-e-constant $2 4 \
			-generate 0xe6301154 0xe6301158 -l-e-constant $(CONFIG_SPL_TEXT_BASE) 4 \
			-generate 0xe6301264 0xe6301268 -l-e-constant $2 4
endif

spl/u-boot-spl.scif: spl/u-boot-spl.srec spl/u-boot-spl.bin
	$(call cmd,srec_cat,$(shell wc -c spl/u-boot-spl.bin | awk '{printf("0x%08x\n",$$1)}'))

# if srec_cat is present build u-boot-spl.scif by default
has_srec_cat = $(call try-run,srec_cat -VERSion,y,n)
ALL-$(has_srec_cat) += u-boot-spl.scif
CLEAN_FILES += u-boot-spl.scif
