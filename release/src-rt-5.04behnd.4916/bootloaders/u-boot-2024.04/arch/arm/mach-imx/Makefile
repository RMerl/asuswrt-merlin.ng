# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2000-2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# (C) Copyright 2011 Freescale Semiconductor, Inc.

ifeq ($(SOC),$(filter $(SOC),mx25 mx35 mx5 mx6 mx7 imx8m imx9 vf610))
obj-y	= iomux-v3.o
endif

ifeq ($(SOC),$(filter $(SOC),imx8m))
ifneq ($(CONFIG_SPL_BUILD),y)
obj-$(CONFIG_IMX_BOOTAUX) += imx_bootaux.o
endif
obj-$(CONFIG_ENV_IS_IN_MMC) += mmc_env.o
obj-$(CONFIG_FEC_MXC) += mac.o
obj-$(CONFIG_DWC_ETH_QOS) += mac.o
obj-$(CONFIG_SYS_I2C_MXC) += i2c-mxv7.o
obj-$(CONFIG_IMX_HAB) += hab.o
obj-y += cpu.o
endif

ifeq ($(SOC),$(filter $(SOC),mx5 mx6))
obj-y	+= cpu.o speed.o
ifneq ($(CONFIG_MX51),y)
obj-y	+= mmdc_size.o
endif
obj-$(CONFIG_GPT_TIMER) += timer.o
obj-$(CONFIG_SYS_I2C_MXC) += i2c-mxv7.o
endif
ifeq ($(SOC),$(filter $(SOC),mx7 mx6 mxs imx8m imx8 imx9 imxrt))
obj-y	+= misc.o
obj-$(CONFIG_CMD_PRIBLOB) += priblob.o
obj-$(CONFIG_SPL_BUILD)	+= spl.o
endif
ifeq ($(SOC),$(filter $(SOC),mx7))
obj-y	+= cpu.o
obj-$(CONFIG_SYS_I2C_MXC) += i2c-mxv7.o
obj-$(CONFIG_ENV_IS_IN_MMC) += mmc_env.o
endif
ifeq ($(SOC),$(filter $(SOC),mx7 imx8m))
ifneq ($(CONFIG_SPL_BUILD),y)
obj-$(CONFIG_FSL_MFGPROT) += cmd_mfgprot.o
endif
endif
ifeq ($(SOC),$(filter $(SOC),mx5 mx6 mx7))
obj-$(CONFIG_IMX_VIDEO_SKIP) += video.o
endif
ifeq ($(SOC),$(filter $(SOC),mx6 mx7))
obj-y	+= cache.o init.o
obj-$(CONFIG_FEC_MXC) += mac.o
obj-$(CONFIG_IMX_RDC) += rdc-sema.o
ifneq ($(CONFIG_SPL_BUILD),y)
obj-$(CONFIG_IMX_BOOTAUX) += imx_bootaux.o
endif
obj-$(CONFIG_$(SPL_)SATA) += sata.o
obj-$(CONFIG_IMX_HAB)    += hab.o
obj-$(CONFIG_SYSCOUNTER_TIMER) += syscounter.o
endif
ifeq ($(SOC),$(filter $(SOC),mx7ulp))
obj-y  += cache.o mmdc_size.o
obj-$(CONFIG_IMX_HAB) += hab.o
endif
ifeq ($(SOC),$(filter $(SOC),vf610))
obj-y += ddrmc-vf610.o
obj-$(CONFIG_DDRMC_VF610_CALIBRATION) += ddrmc-vf610-calibration.o
endif
ifeq ($(SOC),$(filter $(SOC),imx8))
ifneq ($(CONFIG_SPL_BUILD),y)
obj-$(CONFIG_IMX_BOOTAUX) += imx_bootaux.o
endif
endif
ifneq ($(CONFIG_SPL_BUILD),y)
obj-$(CONFIG_CMD_BMODE) += cmd_bmode.o
obj-$(CONFIG_CMD_HDMIDETECT) += cmd_hdmidet.o
obj-$(CONFIG_CMD_DEKBLOB) += cmd_dek.o
obj-$(CONFIG_CMD_NANDBCB) += cmd_nandbcb.o
endif

ifeq ($(CONFIG_SPL_BUILD),y)
obj-$(CONFIG_SPL_LOAD_IMX_CONTAINER) += image-container.o
endif

ifeq ($(SOC),$(filter $(SOC),imx8ulp imx9))
obj-$(CONFIG_AHAB_BOOT) += ele_ahab.o
endif

PLUGIN = board/$(BOARDDIR)/plugin

ifeq ($(CONFIG_USE_IMXIMG_PLUGIN),y)

$(PLUGIN).o: $(PLUGIN).S FORCE
	$(Q)mkdir -p $(dir $@)
	$(call if_changed_dep,as_o_S)

$(PLUGIN).bin: $(PLUGIN).o FORCE
	$(Q)mkdir -p $(dir $@)
	$(OBJCOPY) -O binary --gap-fill 0xff $< $@
else

$(PLUGIN).bin:

endif

quiet_cmd_cpp_cfg = CFGS    $@
      cmd_cpp_cfg = $(CPP) $(cpp_flags) -D__ASSEMBLY__ -x c -o $@ $<

# mkimage source config file
IMX_CONFIG = $(CONFIG_IMX_CONFIG:"%"=%)

# How to create a cpp processed config file, they all use the same source
%.cfgout: $(IMX_CONFIG) FORCE
	$(Q)mkdir -p $(dir $@)
	$(call if_changed_dep,cpp_cfg)

IMX_CONTAINER_CFG = $(CONFIG_IMX_CONTAINER_CFG:"%"=%)
container.cfg: $(IMX_CONTAINER_CFG) FORCE
	$(Q)mkdir -p $(dir $@)
	$(call if_changed_dep,cpp_cfg)

ifeq ($(CONFIG_ARCH_IMX8), y)
CNTR_DEPFILES := $(srctree)/tools/imx_cntr_image.sh
IMAGE_TYPE := imx8image
ifeq ($(CONFIG_SPL_BUILD),y)
SPL_DEPFILE_EXISTS := $(shell $(CPP) $(cpp_flags) -x c -o spl/u-boot-spl.cfgout $(srctree)/$(IMX_CONFIG); if [ -f spl/u-boot-spl.cfgout ]; then $(CNTR_DEPFILES) spl/u-boot-spl.cfgout; echo $$?; fi)
endif
DEPFILE_EXISTS := $(shell $(CPP) $(cpp_flags) -x c -o u-boot-dtb.cfgout $(srctree)/$(IMX_CONFIG); if [ -f u-boot-dtb.cfgout ]; then $(CNTR_DEPFILES) u-boot-dtb.cfgout; echo $$?; fi)
else ifeq ($(CONFIG_ARCH_IMX8M), y)
IMAGE_TYPE := imx8mimage
DEPFILE_EXISTS := 0
else ifeq ($(CONFIG_ARCH_IMX9), y)
IMAGE_TYPE := imx8image
DEPFILE_EXISTS := 0
else
IMAGE_TYPE := imximage
DEPFILE_EXISTS := 0
endif

MKIMAGEFLAGS_u-boot.imx = -n $(filter-out $(PLUGIN).bin $< $(PHONY),$^) \
	-T $(IMAGE_TYPE) -e $(CONFIG_TEXT_BASE)
u-boot.imx: MKIMAGEOUTPUT = u-boot.imx.log

u-boot.imx: u-boot.bin u-boot.cfgout $(PLUGIN).bin FORCE
	$(call if_changed,mkimage)

quiet_cmd_u-boot-nand_imx = GEN     $@
cmd_u-boot-nand_imx = (dd bs=1024 count=1 if=/dev/zero 2>/dev/null) | cat - $< > $@.zero-padded ; \
		      (dd bs=10k count=1 if=/dev/zero 2>/dev/null) | cat $@.zero-padded - > $@ ; \
		      rm -f $@.zero-padded

u-boot-nand.imx: u-boot.imx FORCE
	$(call if_changed,u-boot-nand_imx)

ifeq ($(CONFIG_MULTI_DTB_FIT),y)
MKIMAGEFLAGS_u-boot-dtb.imx = -n $(filter-out $(PLUGIN).bin $< $(PHONY),$^) \
	-T $(IMAGE_TYPE) -e $(CONFIG_TEXT_BASE)
u-boot-dtb.imx: MKIMAGEOUTPUT = u-boot-dtb.imx.log

u-boot-dtb.imx: u-boot-fit-dtb.bin u-boot-dtb.cfgout $(PLUGIN).bin FORCE
ifeq ($(DEPFILE_EXISTS),0)
	$(call if_changed,mkimage)
endif
else ifeq ($(CONFIG_OF_SEPARATE),y)
MKIMAGEFLAGS_u-boot-dtb.imx = -n $(filter-out $(PLUGIN).bin $< $(PHONY),$^) \
	-T $(IMAGE_TYPE) -e $(CONFIG_TEXT_BASE)
u-boot-dtb.imx: MKIMAGEOUTPUT = u-boot-dtb.imx.log

u-boot-dtb.imx: u-boot-dtb.bin u-boot-dtb.cfgout $(PLUGIN).bin FORCE
ifeq ($(DEPFILE_EXISTS),0)
	$(call if_changed,mkimage)
endif
endif

ifdef CONFIG_ARM64
ifeq ($(CONFIG_ARCH_IMX8M), y)

SPL: spl/u-boot-spl.bin spl/u-boot-spl.cfgout FORCE

MKIMAGEFLAGS_flash.bin = -n spl/u-boot-spl.cfgout \
		   -T $(IMAGE_TYPE) -e $(CONFIG_SPL_TEXT_BASE)
flash.bin: MKIMAGEOUTPUT = flash.log

spl/u-boot-spl.cfgout: $(IMX_CONFIG) FORCE
	$(Q)mkdir -p $(dir $@)
	$(call if_changed_dep,cpp_cfg)

spl/u-boot-spl-ddr.bin: spl/u-boot-spl.bin spl/u-boot-spl.cfgout FORCE

flash.bin: spl/u-boot-spl-ddr.bin u-boot.itb FORCE
	$(call if_changed,mkimage)
endif

ifeq ($(CONFIG_ARCH_IMX8), y)
SPL:

MKIMAGEFLAGS_flash.bin = -n spl/u-boot-spl.cfgout -T $(IMAGE_TYPE) -e 0x100000
flash.bin: MKIMAGEOUTPUT = flash.log

MKIMAGEFLAGS_u-boot.cnt = -n container.cfg -T $(IMAGE_TYPE) -e 0x100000
u-boot.cnt: MKIMAGEOUTPUT = u-boot.cnt.log

ifeq ($(CONFIG_SPL_LOAD_IMX_CONTAINER), y)
u-boot.cnt: u-boot.bin container.cfg FORCE
	$(call if_changed,mkimage)
flash.bin: spl/u-boot-spl.bin FORCE
	$(call if_changed,mkimage)
	@flashbin_size=`wc -c flash.bin | awk '{print $$1}'`; \
                   pad_cnt=$$(((flashbin_size + 0x400 - 1) / 0x400)); \
                   echo "append u-boot.cnt at $$pad_cnt KB"; \
                   dd if=u-boot.cnt of=flash.bin bs=1K seek=$$pad_cnt;
else
flash.bin: spl/u-boot-spl.bin FORCE
	$(call if_changed,mkimage)
endif
endif

ifeq ($(CONFIG_ARCH_IMX9), y)

quiet_cmd_imx9_check = CHECK    $@
cmd_imx9_check = $(srctree)/tools/imx9_image.sh $@

SPL: spl/u-boot-spl.bin spl/u-boot-spl.cfgout u-boot-container.cfgout FORCE

MKIMAGEFLAGS_flash.bin = -n spl/u-boot-spl.cfgout -T $(IMAGE_TYPE) -e $(CONFIG_SPL_TEXT_BASE)
flash.bin: MKIMAGEOUTPUT = flash.log

spl/u-boot-spl.cfgout: $(IMX_CONFIG) FORCE
	$(Q)mkdir -p $(dir $@)
	$(call if_changed_dep,cpp_cfg)
	$(call if_changed,imx9_check)

spl/u-boot-spl-ddr.bin: spl/u-boot-spl.bin spl/u-boot-spl.cfgout FORCE

u-boot-container.cfgout: $(IMX_CONTAINER_CFG) FORCE
	$(Q)mkdir -p $(dir $@)
	$(call if_changed_dep,cpp_cfg)
	$(call if_changed,imx9_check)

flash.bin: spl/u-boot-spl-ddr.bin container.cfgout FORCE
	$(call if_changed,mkimage)
endif

else
MKIMAGEFLAGS_SPL = -n $(filter-out $(PLUGIN).bin $< $(PHONY),$^) \
		   -T $(IMAGE_TYPE) -e $(CONFIG_SPL_TEXT_BASE)
SPL: MKIMAGEOUTPUT = SPL.log

SPL: spl/u-boot-spl.bin spl/u-boot-spl.cfgout $(PLUGIN).bin FORCE
	$(call if_changed,mkimage)

MKIMAGEFLAGS_u-boot.uim = -A arm -O U-Boot -a $(CONFIG_TEXT_BASE) \
		-e $(CONFIG_TEXT_BASE) -C none -T firmware

u-boot.uim: u-boot.bin FORCE
	$(call if_changed,mkimage)

OBJCOPYFLAGS += -I binary -O binary --pad-to=$(CONFIG_SPL_PAD_TO)
append = cat $(filter-out $< $(PHONY), $^) >> $@

quiet_cmd_pad_cat = CAT     $@
cmd_pad_cat = $(cmd_objcopy) && $(append) || rm -f $@

u-boot-with-spl.imx: SPL $(if $(CONFIG_OF_SEPARATE),u-boot.img,u-boot.uim) FORCE
	$(call if_changed,pad_cat)

u-boot-with-nand-spl.imx: spl/u-boot-nand-spl.imx $(if $(CONFIG_OF_SEPARATE),u-boot.img,u-boot.uim) FORCE
	$(call if_changed,pad_cat)

quiet_cmd_u-boot-nand-spl_imx = GEN     $@
cmd_u-boot-nand-spl_imx = (printf '\000\000\000\000\106\103\102\040\001' && \
	dd bs=1015 count=1 if=/dev/zero 2>/dev/null) | cat - $< > $@

spl/u-boot-nand-spl.imx: SPL FORCE
	$(call if_changed,u-boot-nand-spl_imx)
endif

targets += $(addprefix ../../../,SPL spl/u-boot-spl.cfgout u-boot-dtb.cfgout u-boot.cfgout u-boot.uim spl/u-boot-nand-spl.imx)

obj-$(CONFIG_MX5) += mx5/
obj-$(CONFIG_MX6) += mx6/
obj-$(CONFIG_MX7) += mx7/
obj-$(CONFIG_ARCH_MX7ULP) += mx7ulp/
obj-$(CONFIG_ARCH_IMX8ULP) += imx8ulp/
obj-$(CONFIG_IMX8M) += imx8m/
obj-$(CONFIG_ARCH_IMX8) += imx8/
obj-$(CONFIG_ARCH_IMX9) += imx9/
obj-$(CONFIG_ARCH_IMXRT) += imxrt/

obj-$(CONFIG_SPL_BOOTROM_SUPPORT) += spl_imx_romapi.o
obj-$(CONFIG_IMX8_ROMAPI) += romapi.o
