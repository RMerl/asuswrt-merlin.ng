# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2000-2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.

extra-$(CONFIG_SPL_BUILD) := start.o

obj-y	= clock.o mxs.o iomux.o timer.o

ifdef	CONFIG_SPL_BUILD
obj-y	+= spl_boot.o spl_lradc_init.o spl_mem_init.o spl_power_init.o
endif

# Specify the target for use in elftosb call
MKIMAGE_TARGET-$(CONFIG_MX23) = mxsimage$(CONFIG_SPL_FRAMEWORK:%=-spl).mx23.cfg
MKIMAGE_TARGET-$(CONFIG_MX28) = mxsimage$(CONFIG_SPL_FRAMEWORK:%=-spl).mx28.cfg

# Generate HAB-capable IVT
#
# Note on computing the post-IVT size field value for the U-Boot binary.
# The value is the result of adding the following:
#  -> The size of U-Boot binary aligned to 64B (u-boot.bin)
#  -> The size of IVT block aligned to 64B (u-boot.ivt)
#  -> The size of U-Boot signature (u-boot.sig), 3904 B
#  -> The 64B hole in front of U-Boot binary for 'struct mxs_spl_data' passing
#
quiet_cmd_mkivt_mxs = MXSIVT  $@
cmd_mkivt_mxs =								\
	sz=`expr \`stat -c "%s" $^\` + 64 + 3904 + 128` ;		\
	echo -n "0x402000d1 $2 0 0 0 $3 $4 0 $$sz 0 0 0 0 0 0 0" |	\
	tr -s " " | xargs -d " " -i printf "%08x\n" "{}" | rev |	\
	sed "s/\(.\)\(.\)/\\\\\\\\x\2\1\n/g" | xargs -i printf "{}" >$@

# Align binary to 64B
quiet_cmd_mkalign_mxs = MXSALGN $@
cmd_mkalign_mxs =							\
	dd if=$^ of=$@ ibs=64 conv=sync 2>/dev/null &&			\
	mv $@ $^

# Assemble the CSF file
quiet_cmd_mkcsfreq_mxs = MXSCSFR $@
cmd_mkcsfreq_mxs =							\
	ivt=$(word 1,$^) ;						\
	bin=$(word 2,$^) ;						\
	csf=$(word 3,$^) ;						\
	sed "s@VENDOR@$(VENDOR)@g;s@BOARD@$(BOARD)@g" "$$csf" |		\
		sed '/^\#\#Blocks/ d' > $@ ;				\
	echo "  Blocks = $2 0x0 `stat -c '%s' $$bin` \"$$bin\" , \\" >> $@ ; \
	echo "           $3 0x0 0x40 \"$$ivt\"" >> $@

# Sign files
quiet_cmd_mkcst_mxs = MXSCST  $@
cmd_mkcst_mxs = cst -o $@ < $^						\
	$(if $(KBUILD_VERBOSE:1=), >/dev/null)

spl/u-boot-spl.ivt: spl/u-boot-spl.bin
	$(call if_changed,mkalign_mxs)
	$(call if_changed,mkivt_mxs,$(CONFIG_SPL_TEXT_BASE),\
		0x00008000,0x00008040)

u-boot.ivt: u-boot.bin
	$(call if_changed,mkalign_mxs)
	$(call if_changed,mkivt_mxs,$(CONFIG_SYS_TEXT_BASE),\
		0x40001000,0x40001040)

spl/u-boot-spl.csf: spl/u-boot-spl.ivt spl/u-boot-spl.bin board/$(VENDOR)/$(BOARD)/sign/u-boot-spl.csf
	$(call if_changed,mkcsfreq_mxs,$(CONFIG_SPL_TEXT_BASE),0x8000)

u-boot.csf: u-boot.ivt u-boot.bin board/$(VENDOR)/$(BOARD)/sign/u-boot.csf
	$(call if_changed,mkcsfreq_mxs,$(CONFIG_SYS_TEXT_BASE),0x40001000)

%.sig: %.csf
	$(call if_changed,mkcst_mxs)

MKIMAGEFLAGS_u-boot.sb = -n $< -T mxsimage
u-boot.sb: $(src)/$(MKIMAGE_TARGET-y) u-boot.bin spl/u-boot-spl.bin FORCE
	$(call if_changed,mkimage)

MKIMAGEFLAGS_u-boot-signed.sb = -n $< -T mxsimage
u-boot-signed.sb: $(src)/mxsimage-signed.cfg u-boot.ivt u-boot.sig spl/u-boot-spl.ivt spl/u-boot-spl.sig FORCE
	$(call if_changed,mkimage)
