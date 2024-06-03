UB := u-boot-2019.07
OPTEE_DFLT_BIN := secureos/optee/tee.bin
INC_OPTEE :=
ATF_DFLT_BIN := bootloaders/armtf/armtf.bin
INC_ATF :=
PATH:=$(PATH):$(CURDIR)/obj/uboot/scripts/dtc

# Specify default tee and armtf bins
bcm_secure_os_bin ?= $(OPTEE_DFLT_BIN)
bcm_armtf_bin ?= $(ATF_DFLT_BIN)

#  This section sets HOSTCC to a version of gcc >= 6
ifeq ($(HOSTCC),)
HOSTCC=cc
hostccversion=$(shell expr `$(HOSTCC) -dumpversion | cut -f1 -d.` \< 6)
ifneq ($(GCC6),)
HOSTCC := $(GCC6)
endif
bcmitgcc6 := /tools/oss/packages/x86_64-rhel6/gcc/6.3.0/bin/gcc
ifneq ($(wildcard $(bcmitgcc6)),)
ifeq ($(strip $(hostccversion)),1)
HOSTCC := "$(bcmitgcc6)"
endif
endif
CCVERSIONGE6 := $(shell expr `$(HOSTCC) -dumpversion | cut -f1 -d.` \< 6)
ifeq ($(strip $(CCVERSIONGE6)),1)
HOSTCC := cc
$(error Uboot compile requires a host compiler version >=6.  If not the default, ser the HOSTCC or GCC6 variable to point at a newer gcc)
endif # CC < 6
endif # HOSTCC

FITPAD ?= 4096
XZ=xz

mdir := $(realpath $(dir $(lastword $(MAKEFILE_LIST))))
thismakefile := $(lastword $(MAKEFILE_LIST))
HOSTTOOLS_DIR ?= $(mdir)/../hostTools
objdir := $(mdir)/obj
atfobjdir := $(mdir)/obj/atf
opteeobjdir := $(mdir)/obj/optee
topbuilddir := $(mdir)/..

LAST_OPTIONS_COOKIE := $(objdir)/.last_options
LAST_OPTIONS        := $(strip $(shell cut -d: -f1 $(LAST_OPTIONS_COOKIE) 2>/dev/null))
LAST_CHIP           := $(strip $(shell cut -d: -f2 $(LAST_OPTIONS_COOKIE) 2>/dev/null))

ifneq ($(strip $(BRCM_CHIP)),)
OPTIONS ?= options_$(BRCM_CHIP)_nand
else ifneq ($(strip $(LAST_OPTIONS)),)
OPTIONS ?= $(LAST_OPTIONS)
endif


ifeq ($(strip $(UBOOT_BUILD_TAG)),)
include build/scripts/bootloader_version.mk
UBOOT_BUILD_TAG="$(BRANCH_STR)@$(word 2,$(VERSION_STR))"
else
UBOOT_BUILD_TAG="$(UBOOT_BUILD_TAG)"
endif

ifeq ($(strip $(LOADER_BUILD_TAG)),)
LOADER_BUILD_TAG=$(UBOOT_BUILD_TAG)
else
LOADER_BUILD_TAG="$(LOADER_BUILD_TAG)"
endif

-include build/configs/$(OPTIONS).conf

KCFL="KCFLAGS=-Werror"

ifeq ($(strip $(findstring smc,$(bcm_uboot_spl_prefix))),smc)
FSBL_FINAL_BASE	= $(objdir)/binaries/u-boot-tpl
FSBL_INTERIM	= $(ubootobjdir)/tpl/u-boot-tpl.bin
BL = tpl
else
FSBL_FINAL_BASE	= $(objdir)/binaries/$(bcm_uboot_spl_prefix)u-boot-spl
FSBL_INTERIM	= $(splobjdir)/spl/u-boot-spl.bin
BL = spl
endif


ubootobjdir := $(mdir)/obj/$(bcm_uboot_uboot_prefix)uboot

ifneq ($(strip $(bcm_cred_dir)),)
bcm_cred_dir:=$(mdir)/../$(bcm_cred_dir)
endif

ifneq ($(strip $(bcm_uboot_rootfs_encrypt)),)
uboot_squash_fname := rootfs.enc
else
uboot_squash_fname := rootfs.squashfs
endif

ifneq ($(strip $(bcm_uboot_encode_keys)),)
gen_keyring_opt := --keyring obj/binaries/image_key.keyring
endif

ifneq ($(strip $(bcm_uboot_delg_fit_sign)),)
bcm_uboot_fit_sign:=$(mdir)/../$(bcm_uboot_delg_fit_sign)
else
ifneq ($(strip $(bcm_uboot_fit_sign)),)
bcm_uboot_fit_sign:=$(mdir)/../$(bcm_uboot_fit_sign)
endif
endif

ifneq ($(strip $(bcm_uboot_delg_sec_sdr)),)
ifneq ($(strip $(bcm_uboot_delg_sec_sdr_sig)),)
bcm_uboot_fit_delegations:=$(addprefix --append=$(mdir)/../,$(bcm_uboot_delg_sec_sdr) $(bcm_uboot_delg_sec_sdr_sig))
endif
endif

ifneq ($(strip $(bcm_uboot_spl_sign)),)
bcm_uboot_spl_sign:=$(mdir)/../$(bcm_uboot_spl_sign)
endif

ifneq ($(strip $(bcm_uboot_rootfs_hash)),)
rootfs_hash_opt := --sha=obj/binaries/rootfs.${bcm_uboot_rootfs_hash}
endif

ifeq ($(strip $(SEC_ARCH)),GEN3)

include build/configs/sec_gen3.conf
ifneq ($(and $(strip $(bcm_uboot_$(BL)_encrypt)),$(strip $(bcm_uboot_spl_encryptor))),)
FLD_CRED_LIST:=$(FLD_CRED_LIST)'opt<enc>'
endif
endif

ifneq ($(strip $(bcm_emmc_raw_linux_bootfs_part_size)),)
opts_gen_emmc_linux_raw += --boot=$(strip $(bcm_emmc_raw_linux_bootfs_part_size))
endif

ifneq ($(strip $(bcm_emmc_raw_linux_rootfs_part_size)),)
opts_gen_emmc_linux_raw += --root=$(strip $(bcm_emmc_raw_linux_rootfs_part_size))
endif

ifeq ($(STANDALONE_TYPE),)
STANDALONE_TYPE := ddr
endif
DDRMOD := arch/arm/mach-bcmbca/bcmbca_$(STANDALONE_TYPE)/bcm_ddr.bin

# Specifies how and if the loader binary is built
BLD_LOADER ?= source

# Specifies whether we want to build binaries that are common between 
# two flash configuration options namely uboot, tpl, DPFE, MCB, DDR binary
BLD_COMMON ?=y

ifneq ($(TOOLCHAIN_BASE),)
UBOOT_CROSS_COMPILE := $(subst /opt/toolchains/,$(TOOLCHAIN_BASE)/,$(UBOOT_CROSS_COMPILE))
endif

ifeq ($(BRCM_CHIP),)
BRCM_CHIP=$(strip $(shell build/work/get_from_last_profile BRCM_CHIP))
endif

splobjdir := $(mdir)/obj/$(bcm_uboot_spl_prefix)spl

# generate configs for SPL
splconfig := $(UBOOT_CONFIG)
ifneq ($strip $(bcm_uboot_spl_arch)),)

splconfig := tmp_$(bcm_uboot_spl_prefix)_$(UBOOT_CONFIG)

tmpconfig: $(UB)/configs/$(splconfig) 

.PHONY: tmpconfig

$(UB)/configs/$(splconfig) : $(UB)/configs/$(UBOOT_CONFIG)
	cat $< > $@
	for i in $(bcm_uboot_spl_arch) ; do perl ./build/configs/$$i.arch $@; done 
	for i in $(STANDALONE_CONFIGS) ; do perl ./build/configs/$$i.arch $@; done

endif

ubootconfig := $(UBOOT_CONFIG)

ifneq ($(strip $(bcm_uboot_arch)),)

ubootconfig := tmp_$(bcm_uboot_uboot_prefix)$(UBOOT_CONFIG)

tmpconfig: $(UB)/configs/$(ubootconfig) 

.PHONY: tmpconfig

$(UB)/configs/$(ubootconfig) : $(UB)/configs/$(UBOOT_CONFIG)
	cat $< > $@
	for i in $(bcm_uboot_arch) ; do perl ./build/configs/$$i.arch $@; done 

endif

ifneq ($(strip $(SEC_ARCH)),XIP)
SEC_MODE := --nonsec
endif

ifeq ($(STANDALONE_TYPE), dpfe)
GEN_HASH_DPFE_OPTION := $(addprefix --dpfeout=$(objdir)/binaries/,$(addsuffix _dpfes.bin_headered,$(STANDALONE_CONFIGS)))
endif

ifneq ($(DPFE_CONFIGS), )
GEN_HASH_DPFE_OPTION := $(addprefix --dpfeout=$(objdir)/binaries/,$(addsuffix _dpfes.bin_headered,$(DPFE_CONFIGS)))
endif


OBJDUMP   = $(UBOOT_CROSS_COMPILE)objdump
OBJCOPY   = $(UBOOT_CROSS_COMPILE)objcopy

defaultrule: spl

COMM_TARGETS := tpl
COMM_TARGETS += $(ubootobjdir)/include/generated/hashtable.h
common: $(COMM_TARGETS)

changed_check: 
	mkdir -p $(objdir)/binaries
	mkdir -p $(ubootobjdir)
	mkdir -p $(splobjdir)
	if [ ! -L "$(splobjdir)/tools" ]; then ln -sn $(ubootobjdir)/tools $(splobjdir)/tools; fi
	mkdir -p $(atfobjdir)
	mkdir -p $(opteeobjdir)
	@if [ "$(LAST_OPTIONS)" != "" ]; then \
                if [ "$(LAST_CHIP)" != "$(BRCM_CHIP)" ]; then \
                        echo "You must do a make clean before rebuilding."; \
                        echo; \
                        exit 1; \
                fi \
        fi
ifneq ($(strip $(BLD_COMMON)),n)
	@echo "$(OPTIONS):$(BRCM_CHIP)" > $(LAST_OPTIONS_COOKIE); 
endif


# unpack target forces uboot symlinks to be created if absent

ifneq ($(strip $(BLD_COMMON)),n)
unpack: $(UB)/tools/dtoc/dtoc 
else
unpack:
endif

$(UB)/tools/dtoc/dtoc : 
	tar xf uboot-2019.07_symlinks.tgz

gen_html_headers:
	mkdir -p $(ubootobjdir)/include/generated
	mkdir -p $(splobjdir)/include/generated
	cd $(UB)/board/broadcom/bcmbca/httpd/html; xxd -i index.html > $(ubootobjdir)/include/generated/index.h
	cd $(UB)/board/broadcom/bcmbca/httpd/html; xxd -i flashing.html > $(ubootobjdir)/include/generated/flashing.h
	cd $(UB)/board/broadcom/bcmbca/httpd/html; xxd -i fail.html > $(ubootobjdir)/include/generated/fail.h
	cd $(UB)/board/broadcom/bcmbca/httpd/html; xxd -i 404.html > $(ubootobjdir)/include/generated/404.h
	cp $(ubootobjdir)/include/generated/index.h $(splobjdir)/include/generated/index.h
	cp $(ubootobjdir)/include/generated/flashing.h $(splobjdir)/include/generated/flashing.h
	cp $(ubootobjdir)/include/generated/fail.h $(splobjdir)/include/generated/fail.h
	cp $(ubootobjdir)/include/generated/404.h $(splobjdir)/include/generated/404.h

rt_ver:
	cp $(mdir)/../router/shared/version.h $(mdir)/$(UB)/include/rt_version.h

tools: configure
	$(MAKE) -C $(UB) O=$(ubootobjdir) CROSS_COMPILE=$(UBOOT_CROSS_COMPILE) HOSTCC="$(HOSTCC)" tools

configure: $(ubootobjdir)/.config gen_html_headers rt_ver
ifeq ($(strip $(BLD_COMMON)),y)
	-rm -f $(ubootobjdir)/include/generated/hashtable.h
endif
	@cp -ar $(objdir).$(shell echo $(BUILD_NAME) | tr A-Z a-z)/* $(objdir)/

configure_spl: $(splobjdir)/.config
	-rm -f $(splobjdir)/include/generated/hashtable.h

$(splobjdir)/.config : $(UB)/configs/$(splconfig) | unpack changed_check 
	$(MAKE) -C $(UB) O=$(splobjdir) $(splconfig)  HOSTCC="$(HOSTCC)"

ifneq ($(strip $(BLD_COMMON)),y)
$(splobjdir)/include/generated/hashtable.h : configure_spl 
else
$(splobjdir)/include/generated/hashtable.h : configure_spl $(ubootobjdir)/include/generated/hashtable.h
endif	
	mkdir -p $(splobjdir)/include/generated/
	cp $(ubootobjdir)/include/generated/hashtable.h $(splobjdir)/include/generated/hashtable.h 

$(ubootobjdir)/.config : $(UB)/configs/$(ubootconfig) | unpack changed_check 
	$(MAKE) -C $(UB) O=$(ubootobjdir) $(ubootconfig)  HOSTCC="$(HOSTCC)"

xconfig savedefconfig menuconfig : 
	$(MAKE) -C $(UB) O=$(ubootobjdir) $@  HOSTCC="$(HOSTCC)"

fixdefconfig:  configure
	$(MAKE) -C $(UB) O=$(ubootobjdir) savedefconfig HOSTCC="$(HOSTCC)"
	chmod u+w $(UB)/configs/$(UBOOT_CONFIG)
	cp $(UB)/configs/$(UBOOT_CONFIG) $(UB)/configs/$(UBOOT_CONFIG).old
	cp $(ubootobjdir)/defconfig  $(UB)/configs/$(UBOOT_CONFIG)
	rm -rf $(objdir)

fix_all_defconfig:
	@if [ "$(LAST_OPTIONS)" ]; then \
	   echo "must run from a clean directory" ; exit 1 ; fi 
	for opfile in `grep -lL bcm_uboot_arch build/configs/options_*.conf`; do op=`basename $$opfile .conf`; \
	   echo $$op; \
	   $(MAKE) OPTIONS=$$op fixdefconfig ; $(MAKE) clean ; done;

spl: configure_spl $(splobjdir)/include/generated/hashtable.h
	$(MAKE) -C $(UB) O=$(splobjdir) spl/u-boot-spl.bin CROSS_COMPILE=$(UBOOT_CROSS_COMPILE) HOSTCC="$(HOSTCC)" BUILD_TAG=$(LOADER_BUILD_TAG) $(KCFL) BLD_COMMON=n
	$(OBJDUMP) -d $(splobjdir)/spl/u-boot-spl > $(splobjdir)/spl/u-boot-spl.dis

ifneq ($(strip $(filter cmm,$(MAKECMDGOALS))),)
uboot: tpl
endif

cmm: spl | uboot
	$(MAKE) -C build/support/jtag/misc $(BRCM_CHIP) CROSS_COMPILE=$(UBOOT_CROSS_COMPILE)
	cp ../kernel/dts/$(BRCM_CHIP)/*.dtb $(objdir)/binaries/linux/
	build/work/generate_cmm.sh $(objdir) $(BRCM_CHIP) $(UBOOT_CROSS_COMPILE) $(splobjdir)/spl/
	build/work/generate_cmm_legacy.py $(objdir) $(BRCM_CHIP) $(UBOOT_CROSS_COMPILE) $(splobjdir)/spl/

tpl: configure
	$(MAKE) -C $(UB) O=$(ubootobjdir) tpl/u-boot-tpl.bin CROSS_COMPILE=$(UBOOT_CROSS_COMPILE) HOSTCC="$(HOSTCC)" BUILD_TAG=$(LOADER_BUILD_TAG) $(KCFL)
	$(OBJDUMP) -d $(ubootobjdir)/tpl/u-boot-tpl > $(ubootobjdir)/tpl/u-boot-tpl.dis

uboot: configure 
	$(MAKE) -C $(UB) O=$(ubootobjdir) u-boot.bin CROSS_COMPILE=$(UBOOT_CROSS_COMPILE) HOSTCC="$(HOSTCC)" BUILD_TAG=$(UBOOT_BUILD_TAG) $(KCFL)
	$(OBJDUMP) -d $(ubootobjdir)/u-boot > $(ubootobjdir)/u-boot.dis
	mkdir -p $(objdir)/binaries/linux/

uboot_ikos: tools spl image_bootstrap
ifeq ($(strip $(findstring spinor,$(bcm_uboot_spl_prefix))),spinor)
	# NOR XIP still runs from 64KB offset. 
ifeq ($(strip $(SEC_ARCH)),XIP)
	$(call gen_ikos_srec,0xFFD10000,$(objdir)/binaries/$(bcm_uboot_spl_prefix)u-boot-spl.bin_headered,bootimg.srec,"u-boot")
else
	$(call gen_ikos_srec,0xFFD00000,$(objdir)/binaries/$(bcm_uboot_spl_prefix)u-boot-spl.bin_headered,bootimg.srec,"u-boot")
endif
else
ifeq ($(strip $(BRCM_CHIP_TYPE)),SMC)
	# NAND XIP runs from 0KB offset in the new chips
	$(call gen_ikos_srec,0x84400000,$(objdir)/binaries/$(bcm_uboot_spl_prefix)u-boot-spl.bin_headered,bootimg.srec,"u-boot")
else
	$(call gen_ikos_srec,0xFFE00000,$(objdir)/binaries/$(bcm_uboot_spl_prefix)u-boot-spl.bin_headered,bootimg.srec,"u-boot")
endif
endif

-include $(UB)/configs/bcm9$(BRCM_CHIP)_defconfig

ifeq ($(strip $(INCLUDE_OPTEE)),y)
INC_OPTEE := _optee
endif

ifeq ($(strip $(INCLUDE_ATF)),y)
INC_ATF := _atf
endif

clean:
	-$(MAKE) -C $(UB)/drivers/net/bcmbca/xrdp_full/ clean
	rm -rf $(objdir)
	rm -f $(UB)/configs/*DDR*_$(UBOOT_CONFIG)
	rm -f $(UB)/configs/tmp_*defconfig

##############################################################################
# HASHTABLE GENERATION                                                       #
# NOTE: Hashtable contains hashes of ddr binary, mcbs, dpfe overlays and TPL #
#       ,will also generate headered verions of above binaries               #
##############################################################################
$(ubootobjdir)/include/generated/hashtable.h: standalone tpl changed_check|encrypt_tpl gen4_sec_switch gen4_sec_tpl
	mkdir -p $(dir $@)
ifeq ($(strip $(findstring smc,$(bcm_uboot_spl_prefix))),smc)
ifeq ($(strip $(bcm_unscrambled_addr)),)
	./build/work/generate_hashes --hashout=$@ --mcbout=$(objdir)/binaries/mcbs.bin_headered $(GEN_HASH_DPFE_OPTION) \
		$(BRCM_CHIP) --bin=$(objdir)/binaries/hashtable.bin_headered --unscram_addr=0x0 --unscram_size=0x0
else
	./build/work/generate_hashes --hashout=$@ --mcbout=$(objdir)/binaries/mcbs.bin_headered $(GEN_HASH_DPFE_OPTION) \
		$(BRCM_CHIP) --bin=$(objdir)/binaries/hashtable.bin_headered --unscram_addr=$(bcm_unscrambled_addr) --unscram_size=$(bcm_unscrambled_size)
endif
else
	./build/work/generate_hashes --hashout=$@ --mcbout=$(objdir)/binaries/mcbs.bin_headered $(GEN_HASH_DPFE_OPTION) \
		$(addprefix --ddr=obj/,$(addsuffix /$(DDRMOD),$(STANDALONE_CONFIGS))) $(BRCM_CHIP) --tpl=$(ubootobjdir)/tpl/u-boot-tpl.bin
	ls $(objdir)/binaries
	for i in $(STANDALONE_CONFIGS) ; do cp $(objdir)/$$i/$(DDRMOD)_headered $(objdir)/binaries/$${i}_$(notdir $(DDRMOD))_headered ; done 
	cp $(ubootobjdir)/tpl/u-boot-tpl.bin_headered $(objdir)/binaries/
endif

##############################################################################
# STANDALONE GENERATION                                                      #
# NOTE: Standalones comprise of ddr binary                                   #
##############################################################################
.SECONDARY : $(patsubst %,$(UB)/configs/%_$(UBOOT_CONFIG),$(STANDALONE_CONFIGS)) $(objdir)/binaries/mcbs.bin_headered

standalone: $(patsubst %,$(objdir)/%/spl/$(DDRMOD),$(STANDALONE_CONFIGS))

$(objdir)/%/spl/$(DDRMOD) : $(UB)/$(dir $(DDRMOD))/*.[ch] | unpack changed_check
	echo "ddr_bin rule called with " $@
	echo $(UB)/configs/$(subst $(objdir)/,,$(basename $(subst /spl/$(DDRMOD),,$@))_$(UBOOT_CONFIG)) 
	$(MAKE) -f $(thismakefile) $(UB)/configs/$(subst $(objdir)/,,$(basename $(subst /spl/$(DDRMOD),,$@))_$(UBOOT_CONFIG)) BLD_COMMON=n
	$(MAKE) -f $(thismakefile) $(subst /spl/$(DDRMOD),,$@)/.config BLD_COMMON=n
	work=$(subst /spl/$(DDRMOD),,$@) ;\
	   $(MAKE) -C $(UB) O=$$work  $(dir $(DDRMOD)) CROSS_COMPILE=$(UBOOT_CROSS_COMPILE) HOSTCC="$(HOSTCC)" V=1 BUILD_TAG=$(LOADER_BUILD_TAG)

$(objdir)/%/.config : $(UB)/configs/%_$(UBOOT_CONFIG) | unpack changed_check
	$(MAKE) -C $(UB) O=$(dir $@) $(notdir $<)  HOSTCC="$(HOSTCC)"

$(UB)/configs/%_$(UBOOT_CONFIG): | unpack changed_check
	cat $(UB)/configs/$(UBOOT_CONFIG) > $@
ifneq ($strip $(bcm_uboot_spl_arch)),)
	for i in $(bcm_uboot_spl_arch) ; do perl ./build/configs/$$i.arch $@; done
endif
# apply the static DDR3/4 configuration
	DDR=$(subst _$(UBOOT_CONFIG),,$(notdir $@)) ; \
	  perl ./build/configs/$$DDR.arch $@

##############################################################################
# UBOOT DEFAULT ENVIRONMENT GENERATION                                       #
##############################################################################
$(objdir)/binaries/$(bcm_uboot_spl_prefix)env.bin_headered:
	-rm -f $(objdir)/binaries/$(bcm_uboot_spl_prefix)env.bin_headered
	@if [ ! -f "build/configs/${bcm_uboot_env}" ]; then \
		echo "Error! Cannot find environment file build/configs/${bcm_uboot_env}!"; \
		exit 1; \
	fi
	echo "Generating UBOOT environment from file build/configs/$(bcm_uboot_env)"; \
	cat build/configs/$(bcm_uboot_env) | $(ubootobjdir)/tools/mkenvimage --bootmagic -s $(bcm_uboot_envsize) -o $(objdir)/binaries/$(bcm_uboot_spl_prefix)env.bin_headered;

##############################################################################
# IMAGE GENERATION                                                           #
# NOTE: Requires generation of loader and FIT sections                       #                                                      
##############################################################################

LINUXFIT = brcm_full_linux.itb
BOOTSTRAPFIT = brcm_simple.itb

# Determine if loader is being built from source
ifeq ($(strip $(BLD_LOADER)),)
LOADERBIN = 
LOADERIMG = 
else # BLD_LOADER
ifneq ($(strip $(BLD_LOADER)),source)
IS_ABS  := $(filter /%,$(BLD_LOADER))
ifneq ($(strip $(IS_ABS)),)
SPECLOADERIMG = $(BLD_LOADER)
else
SPECLOADERIMG = $(topbuilddir)/$(BLD_LOADER)
endif
LOADERBIN = $(notdir $(SPECLOADERIMG))
else # from source
LOADERBIN = loader_test_$(bcm_uboot_spl_prefix)$(BRCM_CHIP).bin
endif
LOADERIMG = $(objdir)/binaries/$(LOADERBIN)
endif

# Setup build configurations
BINARIES = $(wildcard $(objdir)/binaries/*.bin_headered) 
PROFILE  = $(shell cat ../.last_profile)
bcm_uboot_env ?= env_placeholder.conf
bcm_uboot_envsize ?= 16384
bcm_uboot_image_offset ?= 1M

# Configure dynamic build targets
ifeq ($(strip $(BLD_LOADER)),)
bootstrap_deps = image_bootstrap_fit
else
bootstrap_deps = loaderimage image_bootstrap_fit
endif

ifeq ($(strip $(findstring field,$(bcm_uboot_spl_secmode))),field)
ifneq ($(strip $(bcm_uboot_spl_sign)),)
bootstrap_deps += sign_loaderimage 
endif
endif

ifeq ($(strip $(findstring nand,$(bcm_uboot_spl_prefix))),nand)

image_linux_deps = image_linux_nand
image_bootstrap_deps = image_bootstrap_nand
image_linux_nand: | image_linux_fit
ifneq ($(strip $(OUTBAND)),y)
sign_image_linux_nand: image_linux_fit 
endif
image_bootstrap_nand: $(bootstrap_deps)
ifeq ($(strip $(BLD_IKOS)),y)
image_linux_deps += image_ikos_nand
endif

endif

ifeq ($(strip $(findstring emmc,$(bcm_uboot_spl_prefix))),emmc)

image_linux_deps = image_linux_emmc
image_bootstrap_deps = image_bootstrap_emmc
image_linux_emmc: | image_linux_fit
ifneq ($(strip $(OUTBAND)),y)
sign_image_linux_emmc: image_linux_fit
endif
image_bootstrap_emmc: $(bootstrap_deps)

endif

ifeq ($(strip $(findstring spinor,$(bcm_uboot_spl_prefix))),spinor)
image_linux_deps = image_linux_spinor
image_bootstrap_deps = image_bootstrap_spinor
image_linux_spinor: | image_linux_fit
image_bootstrap_spinor: $(bootstrap_deps)
ifeq ($(strip $(BLD_IKOS)),y)
image_linux_deps += image_ikos_spinor
endif
endif

ifeq ($(strip $(findstring smc,$(bcm_uboot_spl_prefix))),smc)
image_linux_deps = image_linux_smc
image_bootstrap_deps = image_bootstrap_smc
image_linux_smc: | image_linux_fit
image_bootstrap_smc: $(bootstrap_deps)
endif

# 1 - output binary 
# 2 - objdir
# 3 - defconfig name
define gen_extra_spl
	$(MAKE) -C $(UB) O=$2 $3  HOSTCC="$(HOSTCC)"
	if [ ! -L "$2/tools" ]; then ln -sn $(ubootobjdir)/tools $2/tools; fi
	$(MAKE) -C $(UB) O=$2 spl/u-boot-spl.bin CROSS_COMPILE=$(UBOOT_CROSS_COMPILE) HOSTCC="$(HOSTCC)"
	cp $2/spl/u-boot-spl.bin $1
	$(OBJDUMP) -d $2/spl/u-boot-spl > $2/spl/u-boot-spl.dis
endef

define gen_spl_nonsec_headered
	-rm -f $2
	echo > $2
	PERL5LIB=$(HOSTTOOLS_DIR)/PerlLib/ $(HOSTTOOLS_DIR)/imagetools/insertboot --cfe $1 \
		--arch $(SEC_ARCH) $(SEC_MODE) --chip $(BRCM_CHIP) $2
endef



ifneq ($(strip $(prebuilt_bin)),)
loader_extra_deps += obj/binaries/$(prebuilt_bin)

obj/binaries/$(prebuilt_bin):prebuilt/${BRCM_CHIP}/$(prebuilt_bin)
		@cd prebuilt/${BRCM_CHIP}/ && cp $(prebuilt_bin) $(objdir)/binaries/.

endif

ifneq ($(strip $(bcm_uboot_tkmfg_mid)),)

loader_extra_deps += $(objdir)/binaries/tkprog-spl.mfg

bcm_uboot_tkmfg_keystore := $(mdir)/../$(strip $(bcm_uboot_tkmfg_keystore))

tkobjdir := $(mdir)/obj/tkprog_out
tkconfig := tmp_tk_$(UBOOT_CONFIG)
tkprog := $(objdir)/binaries/tkprog-spl.bin
tkprog_nontk := $(tkobjdir)/tkprog-spl.nontk

$(UB)/configs/$(tkconfig) : $(UB)/configs/$(UBOOT_CONFIG)
	cat $< > $@
	perl ./build/configs/tkprog.arch $@ $(bcm_sech_arch_ver) 
	# for i in $(bcm_uboot_spl_arch) ; do perl ./build/configs/$$i.arch $@; done 

$(objdir)/binaries/tkprog-spl.bin_headered: build_tkprog 
	-rm -f $@
	echo > $@
	PERL5LIB=$(HOSTTOOLS_DIR)/PerlLib/ $(HOSTTOOLS_DIR)/imagetools/insertboot --cfe $(tkprog),$(BOOTLOADER_EXTRA) \
		--arch $(SEC_ARCH) $(SEC_MODE) --chip $(BRCM_CHIP) $@

$(objdir)/binaries/tkprog-spl.mfg: build_tkprog
	-rm -f $@
	echo > $@
	-rm -f $@.auth_header
	echo > $@.auth_header
	PERL5LIB=$(HOSTTOOLS_DIR)/PerlLib/ $(HOSTTOOLS_DIR)/imagetools/insertboot       \
		--cfe $(tkprog) --arch $(SEC_ARCH)  --chip $(BRCM_CHIP) --mfg           \
		--cred=$(MFG_CRED_LIST) --out $@.auth_header
	cat $@.auth_header $(tkprog) > $@.signable
ifneq ($(and $(strip $(bcm_uboot_tkmfg_sign)),$(strip $(bcm_uboot_spl_signer))),)
	build/scripts/$(bcm_uboot_spl_signer) $@.signable $@.sig $(topbuilddir)/$(bcm_uboot_tkmfg_sign)
endif
	PERL5LIB=$(HOSTTOOLS_DIR)/PerlLib/ $(HOSTTOOLS_DIR)/imagetools/insertboot       \
		--cfe $(tkprog),$(BOOTLOADER_EXTRA)                                     \
		--arch $(SEC_ARCH)  --chip $(BRCM_CHIP) --mfg=$@.sig                    \
		--cred=$(MFG_CRED_LIST) --offset 0 $@

##encrypt Kroe-fld with Kaes-mfg
$(objdir)/binaries/keyinfo.encrypted: 
	PERL5LIB=$(HOSTTOOLS_DIR)/PerlLib/ build/scripts/gen_tk_util                           \
		--encrypt $(patsubst %,$(topbuilddir)/%,"$(strip $(bcm_uboot_tkmfg_aes))")     \
		--fld_roe $(patsubst %,$(topbuilddir)/%,"$(strip $(bcm_uboot_tkmfg_fld_roe))") \
		--key_len $(bcm_uboot_aes_len) --out $@
##sign keyinfo

##generate signable from encrypted 
$(objdir)/binaries/keyinfo.signable: $(objdir)/binaries/keyinfo.encrypted
	cat $<|head -c 32 > $(objdir)/binaries/fld_roe_ek.enc
	cat $<|tail -c 32 > $(objdir)/binaries/fld_roe_iv.enc
	PERL5LIB=$(HOSTTOOLS_DIR)/PerlLib/ $(HOSTTOOLS_DIR)/SecureBootUtils/genkeyst \
		$(strip $(bcm_uboot_tkmfg_args))                                     \
		--args ek=$(objdir)/binaries/fld_roe_ek.enc                          \
		--args iv=$(objdir)/binaries/fld_roe_iv.enc                          \
		--args hash=$(topbuilddir)/$(bcm_uboot_tkmfg_fld_hmid)               \
		--args mid=$(topbuilddir)/$(bcm_uboot_tkmfg_mid)                                    \
		--args keyinfo=$@
	 
$(objdir)/binaries/keyinfo.sig: $(objdir)/binaries/keyinfo.signable
	PERL5LIB=$(HOSTTOOLS_DIR)/PerlLib/ build/scripts/gen_tk_util 		     \
		--sign $(topbuilddir)/$(bcm_uboot_tkmfg_sign),$<                     \
		--out $@
	cat $@ $< > $(objdir)/binaries/keyinfo.signed

$(bcm_uboot_tkmfg_keystore): $(objdir)/binaries/keyinfo.sig
	PERL5LIB=$(HOSTTOOLS_DIR)/PerlLib/ $(HOSTTOOLS_DIR)/SecureBootUtils/genkeyst \
		$(bcm_uboot_tkmfg_args)                                              \
		--args keystore=$(objdir)/binaries/keyinfo.signed                    \
		--args out=$(bcm_uboot_tkmfg_keystore)

$(tkprog_nontk):$(UB)/configs/$(tkconfig)|$(bcm_uboot_tkmfg_keystore)
	$(call gen_extra_spl,$@,$(tkobjdir),$(tkconfig))

build_tkprog: $(tkprog_nontk)
	rm  -rvf $(tkprog)
	cat $< $(bcm_uboot_tkmfg_keystore) > $(tkprog)


.PHONY: build_tkprog

endif

encrypt_tpl:
ifneq ($(strip $(findstring smc,$(bcm_uboot_spl_prefix))),smc)
ifneq ($(and $(strip $(bcm_uboot_tpl_encrypt)),$(strip $(bcm_uboot_spl_encryptor))),)
	mv -vf $(ubootobjdir)/tpl/u-boot-tpl.bin  $(ubootobjdir)/tpl/u-boot-tpl.bin.unenc
	build/scripts/$(bcm_uboot_spl_encryptor) $(bcm_uboot_spl_encryptor_args) $(ubootobjdir)/tpl/u-boot-tpl.bin.unenc $(patsubst %,$(topbuilddir)/%,"$(strip $(bcm_uboot_tpl_encrypt))") > $(ubootobjdir)/tpl/u-boot-tpl.bin
endif
endif

.PHONY: encrypt_tpl

# Loader image
ifeq ($(strip $(BLD_LOADER)),source)
loaderimage: $(objdir)/binaries/$(bcm_uboot_spl_prefix)env.bin_headered $(loader_extra_deps)
	# build loader from layout file
	-rm -f $(FSBL_FINAL_BASE).bin_headered
	echo > $(FSBL_FINAL_BASE).bin_headered
	PERL5LIB=$(HOSTTOOLS_DIR)/PerlLib/ $(HOSTTOOLS_DIR)/imagetools/insertboot --cfe $(FSBL_INTERIM),$(BOOTLOADER_EXTRA) \
		--arch $(SEC_ARCH) $(SEC_MODE) --chip $(BRCM_CHIP) $(FSBL_FINAL_BASE).bin_headered
ifeq ($(strip $(findstring field,$(bcm_uboot_spl_secmode))),field)
	# Prepare headered spl for signature
	-rm -f $(FSBL_FINAL_BASE).auth_header
	echo > $(FSBL_FINAL_BASE).auth_header
	PERL5LIB=$(HOSTTOOLS_DIR)/PerlLib/ $(HOSTTOOLS_DIR)/imagetools/insertboot --cfe $(FSBL_INTERIM),$(BOOTLOADER_EXTRA) \
		--arch $(SEC_ARCH)  --chip $(BRCM_CHIP) --field --cred=$(FLD_CRED_LIST) --out $(FSBL_FINAL_BASE).auth_header
	## FIXME -- does signable need to include BOOTLOADER_EXTRA??
ifneq ($(and $(strip $(bcm_uboot_$(BL)_encrypt)),$(strip $(bcm_uboot_spl_encryptor))),)
	build/scripts/$(bcm_uboot_spl_encryptor) $(bcm_uboot_spl_encryptor_args) $(FSBL_INTERIM) "$(patsubst %,$(topbuilddir)/%,$(bcm_uboot_$(BL)_encrypt))" $ > $(FSBL_FINAL_BASE).enc
	cat $(FSBL_FINAL_BASE).auth_header $(FSBL_FINAL_BASE).enc > $(FSBL_FINAL_BASE).signable
else
	cat $(FSBL_FINAL_BASE).auth_header $(FSBL_INTERIM) > $(FSBL_FINAL_BASE).signable
endif
ifneq ($(and $(strip $(bcm_uboot_spl_sign)),$(strip $(bcm_uboot_spl_signer))),)
	build/scripts/$(bcm_uboot_spl_signer) $(FSBL_FINAL_BASE).signable $(FSBL_FINAL_BASE).sig $(bcm_uboot_spl_sign)
endif
else
	build/work/generate_loader --layout=build/configs/$(bcm_uboot_layout) --splprefix=$(bcm_uboot_spl_prefix) \
		--out=$(LOADERIMG) $(objdir)/binaries/*.bin_headered 
endif	
else
loaderimage:
	mkdir -p $(objdir)/binaries
	cp $(SPECLOADERIMG) $(LOADERIMG)
endif		

sign_loaderimage: | loaderimage
	-rm -f $(FSBL_FINAL_BASE).field
	echo > $(FSBL_FINAL_BASE).field
ifneq ($(and $(strip $(bcm_uboot_$(BL)_encrypt)),$(strip $(bcm_uboot_spl_encryptor))),)
	PERL5LIB=$(HOSTTOOLS_DIR)/PerlLib/ $(HOSTTOOLS_DIR)/imagetools/insertboot --cfe  $(FSBL_FINAL_BASE).enc,$(BOOTLOADER_EXTRA) \
		--arch $(SEC_ARCH)  --chip $(BRCM_CHIP) --field=$(FSBL_FINAL_BASE).sig \
		--cred=$(FLD_CRED_LIST) --offset 0  $(FSBL_FINAL_BASE).field
else
	PERL5LIB=$(HOSTTOOLS_DIR)/PerlLib/ $(HOSTTOOLS_DIR)/imagetools/insertboot --cfe $(FSBL_INTERIM),$(BOOTLOADER_EXTRA) \
		--arch $(SEC_ARCH)  --chip $(BRCM_CHIP) --field=$(FSBL_FINAL_BASE).sig \
		--cred=$(FLD_CRED_LIST) --offset 0  $(FSBL_FINAL_BASE).field
endif
	build/work/generate_loader --layout=build/configs/$(bcm_uboot_layout) --splprefix=$(bcm_uboot_spl_prefix) \
		--out=$(LOADERIMG) $(objdir)/binaries/*.bin_headered $(objdir)/binaries/*.field $(objdir)/binaries/*.mfg


.PHONY: sign_loaderimage

ifeq ($(strip $(BLD_COMMON)),y)
	echo "BLD_COMMON is set $(BLD_COMMON)"
else
	echo "BLD_COMMON is not set $(BLD_COMMON)"
endif

# 1 - .itb binary name
define add_fit_sig
	if [ -f obj/binaries/$1.sig ]; then \
		echo "INSERTING signature $1.sig in $1"; \
		./build/work/fit_header_tool \
		$(bcm_uboot_fit_delegations) \
		--sig obj/binaries/$1.sig obj/binaries/$1; \
		echo "" ;\
	else \
		echo "No signature file found --> NOT Inserting signatures"; \
	fi
endef

# Note: This target is called from within /bootloaders dir while bcm_secure_os_bin
#       and bcm_armtf_bin are specified relative to toplevel build dir
copy_trusted_bins:
ifeq ($(strip $(INCLUDE_OPTEE)),y)
	cp -f ../$(bcm_secure_os_bin) $(opteeobjdir)/tee.bin
endif
ifeq ($(strip $(INCLUDE_ATF)),y)
	cp -f ../$(bcm_armtf_bin) $(atfobjdir)/armtf.bin
endif

.PHONY: copy_trusted_bins

# Bootstrap FIT image
ifneq ($(strip $(BLD_COMMON)),y)
image_bootstrap_fit: copy_trusted_bins
else
image_bootstrap_fit: uboot copy_trusted_bins
endif
	# workaround rm -f ~/bin/dtc
	mkdir -p $(objdir)/binaries/linux/
	-cp ../targets/$(PROFILE)/image_ident $(objdir)/binaries/linux/
	-cp ../kernel/dts/$(BRCM_CHIP)/*.dtb $(objdir)/binaries/linux/
	./build/work/generate_linux_its --simple --dir=obj/binaries/linux/ --chip=$(BRCM_CHIP) \
		$(if $(bcm_uboot_delg_sec_pol),--security_policy=../$(bcm_uboot_delg_sec_pol)) \
		$(if $(bcm_uboot_fit_security),--securitydefs=$(bcm_uboot_fit_security)) \
		$(if $(strip $(findstring smc,$(bcm_uboot_spl_prefix))),--smc) \
		--arch=$(ARCH) $(if $(INC_ATF),--armtf) $(if $(INC_OPTEE),--optee) $(if $(BLD_IKOS),--nohash) > obj/binaries/brcm_simple.its
	./obj/uboot/tools/mkimage -f obj/binaries/brcm_simple.its -E obj/binaries/tmp_$(BOOTSTRAPFIT);\
	fitpad2len=`build/work/fit_header_tool --hex --pad $(FITPAD)  obj/binaries/tmp_$(BOOTSTRAPFIT)` ; \
		./obj/uboot/tools/mkimage -p $$fitpad2len -f obj/binaries/brcm_simple.its -E obj/binaries/$(BOOTSTRAPFIT) 
	fithdrlen=`build/work/fit_header_tool  obj/binaries/$(BOOTSTRAPFIT)` ; \
		dd if=obj/binaries/$(BOOTSTRAPFIT) of=obj/binaries/$(BOOTSTRAPFIT)_signable bs=$$fithdrlen count=1
ifneq ($(and $(strip $(bcm_uboot_fit_sign)),$(strip $(bcm_uboot_fit_signer))),)
	build/scripts/$(bcm_uboot_fit_signer) obj/binaries/$(BOOTSTRAPFIT)_signable obj/binaries/$(BOOTSTRAPFIT).sig $(bcm_uboot_fit_sign)
endif

# Bootstrap images for emmc		
define gen_emmc_bootstrap_bins
	# FIXME ... these sizes should not be hardcoded
	dd if=/dev/zero of=obj/binaries/bootstrap_image_emmc_userdata_part.bin bs=1M count=3; \
	sgdisk -n 3:1MiB:+1MiB -c 3:fit1 obj/binaries/bootstrap_image_emmc_userdata_part.bin; \
	dd if=obj/binaries/$(BOOTSTRAPFIT) of=obj/binaries/bootstrap_image_emmc_userdata_part.bin bs=512 seek=2048 conv=notrunc; \
	cp $(LOADERIMG) $(objdir)/binaries/bootstrap_image_emmc_boot_part.bin; 
endef

image_bootstrap_emmc sign_image_bootstrap_emmc:
	# Add signatures if required
	@$(call add_fit_sig,$(BOOTSTRAPFIT))
	@$(call gen_emmc_bootstrap_bins)

image_bootstrap_smc sign_image_bootstrap_smc:
	# Add signatures if required
	@$(call add_fit_sig,$(BOOTSTRAPFIT))


# Bootstrap images for nand		
# 1 - Erase block size
# 2 - Min i/o unit size
define gen_nand_bootstrap_bins
	# $(BOOTSTRAPFIT) already exists 
	@echo "We are assuming (unwisely) that ubinize is already built"; 
	../hostTools/mtd-utils/ubinize -o obj/binaries/fit$1.ubi -v -m $2 -p $1KiB build/work/fit_simple.ubinize; \
	cp -rvf obj/binaries/$(BOOTSTRAPFIT)  obj/binaries/brcm_simple.dup.itb; \
	cp $(LOADERIMG) $(objdir)/binaries/bootstrap_image_$1.bin; \
	dd if=obj/binaries/fit$1.ubi of=$(objdir)/binaries/bootstrap_image_$1.bin  bs=$(bcm_uboot_image_offset) seek=1 conv=notrunc; 
endef

image_bootstrap_nand sign_image_bootstrap_nand:
	# Add signatures if required
	@$(call add_fit_sig,$(BOOTSTRAPFIT))
	@$(call gen_nand_bootstrap_bins,"128","2048")
	@$(call gen_nand_bootstrap_bins,"256","4096")

# Bootstrap images for spinor		
define gen_spinor_bootstrap_bins
	cp $(LOADERIMG) $(objdir)/binaries/bootstrap_image_spinor.bin;
	dd if=obj/binaries/$(BOOTSTRAPFIT) of=$(objdir)/binaries/bootstrap_image_spinor.bin  bs=$(bcm_uboot_image_offset) seek=1 conv=notrunc;
endef

image_bootstrap_spinor sign_image_bootstrap_spinor:
	# Add signatures if required
	@$(call add_fit_sig,$(BOOTSTRAPFIT))
	@$(call gen_spinor_bootstrap_bins)

ifeq ($(strip $(IMAGE_GOAL)),)
image_bootstrap: $(image_bootstrap_deps)
else
image_bootstrap: 
endif

prepare_linux_image:
ifeq ($(strip $(bcm_uboot_kernel_compression)),lzma)
	$(XZ) --format=lzma -f --stdout -e  ../targets/$(PROFILE)/vmlinux.bin > ../targets/$(PROFILE)/vmlinux.bin.lzma
endif
ifeq ($(strip $(bcm_uboot_kernel_compression)),lzo)
	../hostTools/local_install/lzop  < ../targets/$(PROFILE)/vmlinux.bin > ../targets/$(PROFILE)/vmlinux.bin.lzo
endif

image_linux_fit: image_bootstrap copy_trusted_bins $(bootstrap_deps)
	echo "RUNNING image_linux_fit"
	cp ../targets/$(PROFILE)/vmlinux* $(objdir)/uboot/
	cp ../targets/$(PROFILE)/vmlinux* $(objdir)/binaries/linux/
	-cp ../targets/$(PROFILE)/rootfs_* $(objdir)/binaries
	-cp ../targets/$(PROFILE)/rootfs.img $(objdir)/binaries/rootfs.squashfs
	-cp ../targets/$(PROFILE)/rootfs.ext4 $(objdir)/binaries
ifneq ($(strip $(bcm_uboot_rootfs_encrypt)),)
	build/scripts/${bcm_uboot_rootfs_encrypt}
endif
	# cp uboot_squash_fname needs to be after rootfs encrypt
	-cp $(objdir)/binaries/${uboot_squash_fname} $(objdir)/binaries/rootfs_nonubifs
ifneq ($(strip $(bcm_uboot_encode_keys)),)
	>obj/binaries/image_key.keyring
	build/work/embed_image_key --delegated=${bcm_uboot_delg_aes_base} ${bcm_uboot_encode_keys}
endif
ifneq ($(strip $(bcm_uboot_pmcfw_code_addr)$(bcm_uboot_pmcfw_data_addr)),)
	cp -f $(UB)/arch/arm/mach-bcmbca/pmc/pmc_firmware_$(BRCM_CHIP)* $(objdir)/binaries
endif
	./build/work/generate_linux_its --dir=obj/binaries/linux/ --chip=$(BRCM_CHIP) \
		${rootfs_hash_opt} ${gen_keyring_opt} \
		$(if $(bcm_uboot_rootfs_encrypt),--mapper=obj/binaries/dm.txt) \
		$(if $(bcm_uboot_delg_sec_pol),--security_policy=../$(bcm_uboot_delg_sec_pol)) \
		$(if $(bcm_support_fallback),--support_fallback=$(bcm_support_fallback)) \
		$(if $(bcm_uboot_fit_security),--securitydefs=$(bcm_uboot_fit_security)) \
		$(if $(bcm_uboot_kernel_compression),--kernel_compression=$(bcm_uboot_kernel_compression)) \
		$(if $(bcm_uboot_pmcfw_code_addr),--pmcfw_code_addr=$(bcm_uboot_pmcfw_code_addr)) \
		$(if $(bcm_uboot_pmcfw_data_addr),--pmcfw_data_addr=$(bcm_uboot_pmcfw_data_addr)) \
		$(if $(strip $(findstring smc,$(bcm_uboot_spl_prefix))),--smc) \
		--arch=$(ARCH) $(if $(INC_ATF),--armtf) $(if $(INC_OPTEE),--optee) $(if $(BLD_IKOS),--nohash) \
		> obj/binaries/brcm_full_linux.its
	./obj/uboot/tools/mkimage -f obj/binaries/brcm_full_linux.its -E obj/binaries/tmp_fit.itb 
	fitpad2len=`build/work/fit_header_tool --hex --pad 1280  obj/binaries/tmp_fit.itb` ; \
		./obj/uboot/tools/mkimage -p $$fitpad2len -f obj/binaries/brcm_full_linux.its -E obj/binaries/$(LINUXFIT)
	fithdrlen=`build/work/fit_header_tool  obj/binaries/$(LINUXFIT)` ; \
		dd if=obj/binaries/$(LINUXFIT) of=obj/binaries/$(LINUXFIT)_signable bs=$$fithdrlen count=1
ifneq ($(and $(strip $(bcm_uboot_fit_sign)),$(strip $(bcm_uboot_fit_signer))),)
	build/scripts/$(bcm_uboot_fit_signer) obj/binaries/$(LINUXFIT)_signable  obj/binaries/$(LINUXFIT).sig  $(bcm_uboot_fit_sign)
endif
	dd if=/dev/zero of=obj/binaries/test_metadata.bin_headered bs=1M count=1 ; \
	echo "COMMITTED=1\nVALID=1\n" | $(ubootobjdir)/tools/mkenvimage --metadata -s 1024 -o $(objdir)/binaries/test_metadata.bin_headered; 
	echo "RUNNING image_linux_fit END"

# Generate SREC IKOS image
# 1 - loader address
# 2 - loader binary name
# 3 - output srec name
# 4 - full image or u-boot image

define gen_ikos_srec
	echo "Generate IKOS $4 IMG SREC $3 loader $2 @ $1"
	$(OBJCOPY) --output-target=srec --input-target=binary --srec-forceS3 --change-addresses=$1  $2  $(objdir)/binaries/loader.srec ;
	$(OBJCOPY) --output-target=srec --input-target=binary --srec-forceS3 --change-addresses=0x05000000 $(ubootobjdir)/tpl/u-boot-tpl.bin  $(ubootobjdir)/tpl/u-boot-tpl.srec
	$(OBJCOPY) --output-target=srec --input-target=binary --srec-forceS3 --change-addresses=0x01000000 $(ubootobjdir)/u-boot.bin  $(ubootobjdir)/u-boot.srec
	if [ $4 = "full" ]; then \
		cp ../targets/$(PROFILE)/ramdisk $(objdir)/binaries/linux/; \
		$(OBJCOPY) --output-target=srec --input-target=binary --srec-forceS3 --change-addresses=0x03000000 $(objdir)/binaries/brcm_full_linux.itb  $(objdir)/binaries/fit.srec; \
		$(OBJCOPY) --output-target=srec --input-target=binary --srec-forceS3 --change-addresses=0x04000000 $(objdir)/binaries/linux/ramdisk $(objdir)/binaries/ramdisk.srec; \
	else \
	    if [ "$(BRCM_CHIP_TYPE)" = "SMC" ]; then \
		    $(OBJCOPY) --output-target=srec --input-target=binary --srec-forceS3 --change-addresses=0x03000000 $(objdir)/nand_spl/arch/arm/dts/bcm9$(BRCM_CHIP).dtb  $(objdir)/binaries/dtb.srec; \
	    else \
		    $(OBJCOPY) --output-target=srec --input-target=binary --srec-forceS3 --change-addresses=0x03000000 ../kernel/dts/$(BRCM_CHIP)/9$(BRCM_CHIP)ikos.dtb  $(objdir)/binaries/dtb.srec; \
	    fi \
	fi ;

	cp -f $(objdir)/binaries/loader.srec $(objdir)/binaries/$3
	cat $(ubootobjdir)/tpl/u-boot-tpl.srec >> $(objdir)/binaries/$3
	cat $(ubootobjdir)/u-boot.srec >> $(objdir)/binaries/$3
	if [ $4 = "full" ]; then \
		cat $(objdir)/binaries/fit.srec >> $(objdir)/binaries/$3; \
		cat $(objdir)/binaries/ramdisk.srec >> $(objdir)/binaries/$3; \
	else \
		cat $(objdir)/binaries/dtb.srec >> $(objdir)/binaries/$3; \
	fi ;
endef

# 1 - Loader binary
# 2 - Flash type
# 3 - Rootfs binary
define gen_pkgtb_bundles
	if [[ $2 =~ "nand" ]] && [ -z "$(BLD_NAND_PKGTB)" ]; then \
		echo "NOT Building NAND PKTGB Images!"; \
	elif [[ $2 =~ "nor" ]] && [ -z "$(BLD_NOR_PKGTB)" ]; then \
		echo "NOT Building NOR PKTGB Images!"; \
	elif [[ $2 =~ "emmc" ]] && [ -z "$(BLD_EMMC_PKGTB)" ]; then \
		echo "NOT Building EMMC PKTGB Images!"; \
	elif [ $3 = "" ]; then \
		./build/work/generate_bundle_itb --chip=$(BRCM_CHIP) \
		--profile=$(PROFILE) --compatstr=${bcm_image_compatstr}  \
		$(if $(bcm_uboot_pkgtb_signer), --padding=1024 ) --loader=$1 \
		--bootfs=$(BOOTSTRAPFIT); \
	elif [ -f obj/binaries/$3 ]; then \
		./build/work/generate_bundle_itb --chip=$(BRCM_CHIP) \
		--profile=$(PROFILE) --compatstr=${bcm_image_compatstr}  \
		$(if $(bcm_uboot_pkgtb_signer), --padding=1024 ) --loader=$1 \
		--bootfs=$(LINUXFIT) --rootfsspec=$2,$3; \
	else \
		echo "$3 not found --> Skipping its .pkgtb generation"; \
	fi
endef

# Image bundles for nand

# 1 - Erase block size
# 2 - Min i/o unit size
define gen_nand_raw_imgs
	if [ -z "$(BLD_NAND_PKGTB)" ]; then \
		echo "NOT Building NAND Raw Images!"; \
	elif [ -f obj/binaries/rootfs_$1.ubifs ]; then \
		echo "We are assuming (unwisely) that ubinize is already built"; \
		../hostTools/mtd-utils/ubinize -o obj/binaries/linux$1.ubi -v -m $2 -p $1KiB build/work/linux_simple$1.ubinize; \
		cp $(LOADERIMG) $(objdir)/binaries/linux_raw_image_$1.bin; \
		dd if=obj/binaries/linux$1.ubi of=$(objdir)/binaries/linux_raw_image_$1.bin  bs=$(bcm_uboot_image_offset) seek=1 conv=notrunc; \
	else \
		echo "rootfs_$1.ubifs not found --> Skipping its raw image build"; \
	fi ;  \
	if [ -f obj/binaries/${uboot_squash_fname} ]; then \
		echo "We are assuming (unwisely) that ubinize is already built"; \
		../hostTools/mtd-utils/ubinize -o obj/binaries/linux_squash_$1.ubi -v -m $2 -p $1KiB build/work/linux_simple_nonubifs.ubinize; \
		cp $(LOADERIMG) $(objdir)/binaries/linux_raw_squash_image_$1.bin; \
		dd if=obj/binaries/linux_squash_$1.ubi of=$(objdir)/binaries/linux_raw_squash_image_$1.bin  bs=$(bcm_uboot_image_offset) seek=1 conv=notrunc; \
	else \
		echo "rootfs_$1.ubifs not found --> Skipping its raw image build"; \
	fi  
endef

copy_completed_bins:
	cp ../targets/$(PROFILE)/bcm$(PROFILE)_uboot_linux.itb $(objdir)/binaries/brcm_full_linux.itb
	for i in ../targets/$(PROFILE)/bcm$(PROFILE)_uboot_rootfs* ../targets/$(PROFILE)/bcm$(PROFILE)_uboot_bootstrap_image*bin; do \
	   j=`basename $$i | awk -F "bcm$(PROFILE)_uboot_" '{print $$2}'`; if [ -r "$$i" ]; then \
	     cp $$i $(objdir)/binaries/$$j ; \
	   fi; \
	done

# Add signatures if required
ifneq ($(strip $(OUTBAND)),y)
add_fit_sig_linuxfit: | image_linux_fit
else
add_fit_sig_linuxfit: copy_completed_bins
endif
	@echo "SIGN LINUXFIT: $(LINUXFIT)"
	@$(call add_fit_sig,$(LINUXFIT))

IMAGE_BUNDLES_NAND := gen_pkgtb_loader_linux_nand_squash gen_pkgtb_linux_nand_squash
ifeq ($(strip $(findstring field,$(bcm_uboot_spl_secmode))),)
IMAGE_BUNDLES_NAND += gen_pkgtb_loader_linux_nand128_ubifs
IMAGE_BUNDLES_NAND += gen_pkgtb_loader_linux_nand256_ubifs
IMAGE_BUNDLES_NAND += gen_pkgtb_linux_nand128_ubifs
IMAGE_BUNDLES_NAND += gen_pkgtb_linux_nand256_ubifs
endif
ifeq ($(strip $(IMAGE_GOAL)),)
IMAGE_BUNDLES_NAND += gen_nand_raw_128_2048
IMAGE_BUNDLES_NAND += gen_nand_raw_256_4096
endif

$(IMAGE_BUNDLES_NAND): add_fit_sig_linuxfit

sign_image_linux_nand: $(IMAGE_BUNDLES_NAND)
	@echo "RUNNING sign_image_linux_nand END"
	cp obj/binaries/bcm$(PROFILE)_nand_squashfs_loader_update.pkgtb obj/binaries/$(IMGNAME)_nand_squashfs_loader.pkgtb
	cp obj/binaries/bcm$(PROFILE)_nand_squashfs_update.pkgtb obj/binaries/$(IMGNAME)_nand_squashfs.pkgtb

# Generate loader + linux pkgtb images
gen_pkgtb_loader_linux_nand_squash:
	@$(call gen_pkgtb_bundles,$(LOADERBIN),"nand",${uboot_squash_fname})

ifeq ($(strip $(findstring field,$(bcm_uboot_spl_secmode))),)
gen_pkgtb_loader_linux_nand128_ubifs:
	@$(call gen_pkgtb_bundles,$(LOADERBIN),"nand128","rootfs_128.ubifs")

gen_pkgtb_loader_linux_nand256_ubifs:
	@$(call gen_pkgtb_bundles,$(LOADERBIN),"nand256","rootfs_256.ubifs")
endif

# Generate linux pkgtb images
gen_pkgtb_linux_nand_squash:
	@$(call gen_pkgtb_bundles,"","nand",${uboot_squash_fname})

ifeq ($(strip $(findstring field,$(bcm_uboot_spl_secmode))),)
gen_pkgtb_linux_nand128_ubifs:
	@$(call gen_pkgtb_bundles,"","nand128","rootfs_128.ubifs")

gen_pkgtb_linux_nand256_ubifs:
	@$(call gen_pkgtb_bundles,"","nand256","rootfs_256.ubifs")
endif

ifeq ($(strip $(IMAGE_GOAL)),)
# Generate RAW images for nand flash programmers
gen_nand_raw_128_2048:
	@$(call gen_nand_raw_imgs,"128","2048")

gen_nand_raw_256_4096:
	@$(call gen_nand_raw_imgs,"256","4096")
endif

image_linux_nand: sign_image_linux_nand 
ifneq ($(and $(strip $(bcm_uboot_pkgtb_sign)),$(strip $(bcm_uboot_pkgtb_signer))),)
	for pkg in obj/binaries/*.pkgtb.signable; do \
	  sig=obj/binaries/`basename $$pkg nable` ;\
	  echo $$pkg $$sig ;\
	  build/scripts/$(bcm_uboot_pkgtb_signer) $$pkg $$sig $(topbuilddir)/$(bcm_uboot_pkgtb_sign); \
	done
	for pkghead in obj/binaries/*.pkgtb.signable; do \
	  sig=obj/binaries/`basename $$pkghead nable` ;\
	  pkg=obj/binaries/`basename $$pkghead .signable` ;\
	  dd if=$$sig of=$$pkg bs=1 conv=notrunc seek=`stat -c %s $$pkghead` ;\
	done
endif

.PHONY: image_linux_nand sign_image_linux_nand 

image_ikos_nand: | image_linux_fit
	$(call gen_ikos_srec,0xFFE00000,$(LOADERIMG),fullimg.srec,"full")

image_ikos_spinor: | image_linux_fit
	$(call gen_ikos_srec,0xFFD00000,$(LOADERIMG),fullimg.srec,"full")

# Image bundles for emmc
 
# 1 - rootfs binary
define gen_emmc_raw_imgs
	if [ -z "$(BLD_EMMC_PKGTB)" ]; then \
		echo "NOT Building EMMC Raw Images!"; \
	else \
		./build/work/gen_emmc_linux_raw_img.sh $(opts_gen_emmc_linux_raw) obj/binaries/$(LINUXFIT) obj/binaries/$1 $(objdir)/binaries/test_metadata.bin_headered obj/binaries/linux_raw_emmc_userdata_part.bin  ; \
	fi
endef

IMAGE_BUNDLES_EMMC := gen_pkgtb_loader_linux_emmc_squash gen_pkgtb_linux_emmc_squash
ifeq ($(strip $(findstring field,$(bcm_uboot_spl_secmode))),)
IMAGE_BUNDLES_EMMC += gen_pkgtb_loader_linux_emmc_ext4
IMAGE_BUNDLES_EMMC += gen_pkgtb_linux_emmc_ext4
endif
ifeq ($(strip $(IMAGE_GOAL)),)
IMAGE_BUNDLES_EMMC += gen_emmc_raw_squash
endif

$(IMAGE_BUNDLES_EMMC): add_fit_sig_linuxfit

sign_image_linux_emmc: $(IMAGE_BUNDLES_EMMC)
	echo "RUNNING sign_image_linux_emmc END"

# Generate loader + linux pkgtb images
gen_pkgtb_loader_linux_emmc_squash:
	@$(call gen_pkgtb_bundles,$(LOADERBIN),"emmc",${uboot_squash_fname})

ifeq ($(strip $(findstring field,$(bcm_uboot_spl_secmode))),)
gen_pkgtb_loader_linux_emmc_ext4:
	@$(call gen_pkgtb_bundles,$(LOADERBIN),"emmc","rootfs.ext4")
endif

# Generate linux pkgtb images
gen_pkgtb_linux_emmc_squash:
	@$(call gen_pkgtb_bundles,"","emmc",${uboot_squash_fname})

ifeq ($(strip $(findstring field,$(bcm_uboot_spl_secmode))),)
gen_pkgtb_linux_emmc_ext4:
	@$(call gen_pkgtb_bundles,"","emmc","rootfs.ext4")
endif

ifeq ($(strip $(IMAGE_GOAL)),)
# Generate RAW images for emmc flash programmers
gen_emmc_raw_squash:
	@$(call gen_emmc_raw_imgs,${uboot_squash_fname})
endif

image_linux_emmc: sign_image_linux_emmc 
ifneq ($(and $(strip $(bcm_uboot_pkgtb_sign)),$(strip $(bcm_uboot_pkgtb_signer))),)
	for pkg in obj/binaries/*.pkgtb.signable; do \
	  sig=obj/binaries/`basename $$pkg nable` ;\
	  echo $$pkg $$sig ;\
	  build/scripts/$(bcm_uboot_pkgtb_signer) $$pkg $$sig $(topbuilddir)/$(bcm_uboot_pkgtb_sign); \
	done
	for pkghead in obj/binaries/*.pkgtb.signable; do \
	  sig=obj/binaries/`basename $$pkghead nable` ;\
	  pkg=obj/binaries/`basename $$pkghead .signable` ;\
	  dd if=$$sig of=$$pkg bs=1 conv=notrunc seek=`stat -c %s $$pkghead` ;\
	done
endif

IMAGE_BUNDLES_SMC := gen_pkgtb_smc_ext4 gen_pkgtb_smc_bl_ext4 gen_pkgtb_smc_squash  gen_pkgtb_smc_bl_squash gen_pkgtb_smc_bstrap gen_pkgtb_smc_burner_ext4 gen_pkgtb_smc_burner_squash
LICENSE_PAK := $(shell ls -1 ../targets/pakl | head -1)
ifneq ($(strip $(LICENSE_PAK)),)
	LICENSE_IMAGE := --image=pakl,pakl.bin
endif

smc_prepare_images: add_fit_sig_linuxfit
	-rm -f $(objdir)/binaries/smc_os-$(BRCM_CHIP)$(BRCM_CHIP_REV).bin.prodkey.hsm_signed.flash
	-rm -f $(objdir)/binaries/smc_bootl-$(BRCM_CHIP)$(BRCM_CHIP_REV).prodkey.hsm_signed.flash
	cp ../targets/smc/smc_os-$(BRCM_CHIP)$(BRCM_CHIP_REV).bin.prodkey.hsm_signed.flash $(objdir)/binaries/
	cp ../targets/smc/smc_bootl-$(BRCM_CHIP)$(BRCM_CHIP_REV).prodkey.hsm_signed.flash $(objdir)/binaries/
	build/work/generate_loader --layout=build/configs/$(bcm_meminit_layout) --splprefix=$(bcm_uboot_spl_prefix) \
		--out=$(objdir)/binaries/bcm$(PROFILE)_meminit_lun.bin $(objdir)/binaries/*.bin_headered 
	cp $(objdir)/binaries/bcm$(PROFILE)_meminit_lun.bin ../targets/$(PROFILE)/
	cp  $(LOADERIMG) ../targets/$(PROFILE)/bcm$(PROFILE)_armbl_lun.bin
ifneq ($(strip $(LICENSE_PAK)),)
	cp -f "../targets/pakl/$(LICENSE_PAK)" $(objdir)/binaries/pakl.bin
endif

$(IMAGE_BUNDLES_SMC): smc_prepare_images

image_linux_smc sign_image_linux_smc: $(IMAGE_BUNDLES_SMC)
	cp --no-preserve=mode build/configs/$(bcm_uboot_env) $(objdir)/binaries/tmp_env
	echo vf_1_boot=`stat -c %s $(objdir)/binaries/bcm$(PROFILE)_bstrap.pkgtb` >> $(objdir)/binaries/tmp_env
	cat $(objdir)/binaries/tmp_env | $(ubootobjdir)/tools/mkenvimage --bootmagic -s $(bcm_uboot_envsize) -o ../targets/$(PROFILE)/bcm$(PROFILE)_ubootenv_lun.bin
	chmod a+r ../targets/$(PROFILE)/bcm$(PROFILE)_ubootenv_lun.bin
	echo "RUNNING image_linux_smc END"

# Generate linux pkgtb images
gen_pkgtb_smc_ext4:
	./build/work/generate_bundle_itb --chip=$(BRCM_CHIP) --profile=$(PROFILE) --compatstr=${bcm_image_compatstr}  \
		$(if $(bcm_uboot_pkgtb_signer), --padding=1024 ) --bootfs=$(LINUXFIT) --rootfsspec="","rootfs.ext4" \
		--image=armbl,$(LOADERBIN) --image=meminit,bcm$(PROFILE)_meminit_lun.bin  --image=smcos,smc_os-$(BRCM_CHIP)$(BRCM_CHIP_REV).bin.prodkey.hsm_signed.flash \
		$(LICENSE_IMAGE)

gen_pkgtb_smc_bl_ext4:
	./build/work/generate_bundle_itb --chip=$(BRCM_CHIP) --profile=$(PROFILE) --compatstr=${bcm_image_compatstr}  \
		$(if $(bcm_uboot_pkgtb_signer), --padding=1024 ) --bootfs=$(LINUXFIT) --rootfsspec="","rootfs.ext4" \
		--image=armbl,$(LOADERBIN) --image=meminit,bcm$(PROFILE)_meminit_lun.bin --image=smcos,smc_os-$(BRCM_CHIP)$(BRCM_CHIP_REV).bin.prodkey.hsm_signed.flash \
		--image=smcbl,smc_bootl-$(BRCM_CHIP)$(BRCM_CHIP_REV).prodkey.hsm_signed.flash \
		$(LICENSE_IMAGE)

gen_pkgtb_smc_squash:
	./build/work/generate_bundle_itb --chip=$(BRCM_CHIP) --profile=$(PROFILE) --compatstr=${bcm_image_compatstr}  \
		$(if $(bcm_uboot_pkgtb_signer), --padding=1024 ) --bootfs=$(LINUXFIT) --rootfsspec="",${uboot_squash_fname} \
		--image=armbl,$(LOADERBIN) --image=meminit,bcm$(PROFILE)_meminit_lun.bin  --image=smcos,smc_os-$(BRCM_CHIP)$(BRCM_CHIP_REV).bin.prodkey.hsm_signed.flash \
		$(LICENSE_IMAGE)

gen_pkgtb_smc_bl_squash:
	./build/work/generate_bundle_itb --chip=$(BRCM_CHIP) --profile=$(PROFILE) --compatstr=${bcm_image_compatstr}  \
		$(if $(bcm_uboot_pkgtb_signer), --padding=1024 ) --bootfs=$(LINUXFIT) --rootfsspec="",${uboot_squash_fname} \
		--image=armbl,$(LOADERBIN) --image=meminit,bcm$(PROFILE)_meminit_lun.bin  --image=smcos,smc_os-$(BRCM_CHIP)$(BRCM_CHIP_REV).bin.prodkey.hsm_signed.flash \
		--image=smcbl,smc_bootl-$(BRCM_CHIP)$(BRCM_CHIP_REV).prodkey.hsm_signed.flash \
		$(LICENSE_IMAGE)

gen_pkgtb_smc_bstrap:
	./build/work/generate_bundle_itb --chip=$(BRCM_CHIP) --profile=$(PROFILE) --compatstr=${bcm_image_compatstr}  \
		$(if $(bcm_uboot_pkgtb_signer), --padding=1024 ) --bootfs=$(BOOTSTRAPFIT)

gen_pkgtb_smc_burner_ext4:
	./build/work/generate_bundle_itb --chip=$(BRCM_CHIP) --profile=$(PROFILE) --compatstr=${bcm_image_compatstr}  \
		$(if $(bcm_uboot_pkgtb_signer), --padding=1024 ) --bootfs=$(LINUXFIT) --rootfsspec="","rootfs.ext4" --interim

gen_pkgtb_smc_burner_squash:
	./build/work/generate_bundle_itb --chip=$(BRCM_CHIP) --profile=$(PROFILE) --compatstr=${bcm_image_compatstr}  \
		$(if $(bcm_uboot_pkgtb_signer), --padding=1024 ) --bootfs=$(LINUXFIT) --rootfsspec="",${uboot_squash_fname} --interim

# Image bundles for spinor

define gen_spinor_raw_imgs
	cp $(LOADERIMG) $(objdir)/binaries/linux_raw_spinor_image.bin; \
	dd if=$(objdir)/binaries/$(LINUXFIT) of=$(objdir)/binaries/linux_raw_spinor_image.bin  bs=$(bcm_uboot_image_offset) seek=1 conv=notrunc; \
	dd if=$(objdir)/binaries/rootfs.squashfs of=$(objdir)/binaries/linux_raw_spinor_image.bin  bs=$(bcm_spinor_squashfs_offset) seek=1 conv=notrunc; 
endef

IMAGE_BUNDLES_SPINOR := gen_pkgtb_loader_linux_spinor_squash gen_pkgtb_linux_spinor_squash
ifeq ($(strip $(IMAGE_GOAL)),)
IMAGE_BUNDLES_SPINOR += gen_spinor_raw_squash
endif

$(IMAGE_BUNDLES_SPINOR): add_fit_sig_linuxfit

image_linux_spinor sign_image_linux_spinor: $(IMAGE_BUNDLES_SPINOR)
	echo "RUNNING image_linux_spinor END"

# Generate loader + linux pkgtb images
gen_pkgtb_loader_linux_spinor_squash:
	@$(call gen_pkgtb_bundles,$(LOADERBIN),"spinor",${uboot_squash_fname})

# Generate linux pkgtb images
gen_pkgtb_linux_spinor_squash:
	@$(call gen_pkgtb_bundles,"","spinor",${uboot_squash_fname})

ifeq ($(strip $(IMAGE_GOAL)),)
# Generate RAW images for spinor flash programmers
gen_spinor_raw_squash:
	@$(call gen_spinor_raw_imgs)
endif

$(info  ($(strip $(findstring $(or $(IMAGE_GOAL),unset),$(bcm_uboot_spl_prefix)))))
$(info ($(or $(IMAGE_GOAL),unset)))

ifeq ($(strip $(findstring $(IMAGE_GOAL),$(bcm_uboot_spl_prefix))),$(IMAGE_GOAL))
image_linux: $(image_linux_deps)
else
image_linux: 
endif

binaries: standalone
	mkdir -p prebuilt/$(BRCM_CHIP)
	for i in $(STANDALONE_CONFIGS) ; do cp $(objdir)/$$i/$(DDRMOD) prebuilt/$(BRCM_CHIP)/$${i}_$(notdir $(DDRMOD)) ; done 

ifeq ($(wildcard $(UB)/$(dir $(DDRMOD))/*.c),)

$(objdir)/DDR3/spl/$(DDRMOD) : prebuilt/$(BRCM_CHIP)/DDR3_$(notdir $(DDRMOD))
	mkdir -p $(objdir)/DDR3/$(dir $(DDRMOD))
	cp $<  $(objdir)/DDR3/$(DDRMOD)
	-mkdir -p obj/DDR3/arch/arm/mach-bcmbca/bcmbca_dpfe/
	-echo ddr3 > obj/DDR3/arch/arm/mach-bcmbca/bcmbca_dpfe/bcm_ddr.txt
	-mkdir -p obj/DDR3/arch/arm/mach-bcmbca/bcmbca_ddr/
	-echo ddr3 > obj/DDR3/arch/arm/mach-bcmbca/bcmbca_ddr/bcm_ddr.txt

$(objdir)/DDR4/spl/$(DDRMOD) : prebuilt/$(BRCM_CHIP)/DDR4_$(notdir $(DDRMOD))
	mkdir -p $(objdir)/DDR4/$(dir $(DDRMOD))
	cp $<  $(objdir)/DDR4/$(DDRMOD)
	-mkdir -p obj/DDR4/arch/arm/mach-bcmbca/bcmbca_dpfe/
	-echo ddr4 > obj/DDR4/arch/arm/mach-bcmbca/bcmbca_dpfe/bcm_ddr.txt
	-mkdir -p obj/DDR4/arch/arm/mach-bcmbca/bcmbca_ddr/
	-echo ddr4 > obj/DDR4/arch/arm/mach-bcmbca/bcmbca_ddr/bcm_ddr.txt

$(objdir)/LPDDR4/spl/$(DDRMOD) : prebuilt/$(BRCM_CHIP)/LPDDR4_$(notdir $(DDRMOD))
	mkdir -p $(objdir)/LPDDR4/$(dir $(DDRMOD))
	cp $<  $(objdir)/LPDDR4/$(DDRMOD)
	-mkdir -p obj/LPDDR4/arch/arm/mach-bcmbca/bcmbca_dpfe/
	-echo lpddr4 > obj/LPDDR4/arch/arm/mach-bcmbca/bcmbca_dpfe/bcm_ddr.txt
	-mkdir -p obj/LPDDR4/arch/arm/mach-bcmbca/bcmbca_ddr/
	-echo lpddr4 > obj/LPDDR4/arch/arm/mach-bcmbca/bcmbca_ddr/bcm_ddr.txt

$(objdir)/LPDDR5/spl/$(DDRMOD) : prebuilt/$(BRCM_CHIP)/LPDDR5_$(notdir $(DDRMOD))
	mkdir -p $(objdir)/LPDDR5/$(dir $(DDRMOD))
	cp $<  $(objdir)/LPDDR5/$(DDRMOD)
	-mkdir -p obj/LPDDR5/arch/arm/mach-bcmbca/bcmbca_dpfe/
	-echo lpddr5 > obj/LPDDR5/arch/arm/mach-bcmbca/bcmbca_dpfe/bcm_ddr.txt
	-mkdir -p obj/LPDDR5/arch/arm/mach-bcmbca/bcmbca_ddr/
	-echo lpddr5 > obj/LPDDR5/arch/arm/mach-bcmbca/bcmbca_ddr/bcm_ddr.txt

endif

gen4_sec_switch: tpl
ifeq ($(strip $(SEC_ARCH)),GEN4)
ifneq ($(and $(strip $(bcm_sec_switch_certificate)),$(strip $(bcm_sec_switch_rot)),$(strip $(bcm_sec_switch_roe_ek)),$(strip $(bcm_sec_switch_roe_iv)),$(strip $(bcm_sec_switch_ksb)),$(strip $(bcm_sec_encryptor)),$(strip $(bcm_sec_signer))),)
	$(HOSTTOOLS_DIR)/SecureBootUtils/gen4_sbi_composer.py \
		--key_map=$(topbuilddir)/$(bcm_sec_dir)/$(bcm_sec_key_map) \
		--eligibility=MODE_UNSECURE \
		--action=MOVE_TO_FLD \
		--certificate=$(topbuilddir)/$(bcm_sec_dir)/$(bcm_sec_switch_certificate) \
		--mfg_rot=$(bcm_sec_switch_rot) \
		--mfg_encryptor_args="-aes-256-cbc -salt" \
		--mfg_roe_ek=$(bcm_sec_switch_roe_ek) \
		--mfg_roe_iv=$(bcm_sec_switch_roe_iv) \
		--fld_rot="" \
		--fld_encryptor_args="" \
		--fld_roe_ek="" \
		--fld_roe_iv="" \
		--blob=$(topbuilddir)/$(bcm_sec_dir)/$(bcm_sec_switch_ksb) \
		--encryptor=$(topbuilddir)/$(bcm_sec_encryptor) \
		--signer=$(topbuilddir)/$(bcm_sec_signer) \
		--output=$(objdir)/binaries/gen4-sec-switch.bin_headered \
		--build_dir=$(objdir) \
		--crc_calculator=$(topbuilddir)/hostTools/gencrc32
endif
endif
.PHONY: gen4_sec_switch

gen4_sec_tpl: tpl
ifeq ($(strip $(SEC_ARCH)),GEN4)
ifneq ($(and $(strip $(bcm_sec_tpl_rot)),$(strip $(bcm_sec_tpl_roe_options)),$(strip $(bcm_sec_tpl_roe_ek)),$(strip $(bcm_sec_tpl_roe_iv)),$(strip $(bcm_sec_encryptor)),$(strip $(bcm_sec_signer))),)
	$(HOSTTOOLS_DIR)/SecureBootUtils/gen4_sbi_composer.py \
		--key_map=$(topbuilddir)/$(bcm_sec_dir)/$(bcm_sec_key_map) \
		--eligibility=MODE_FLD_SECURE \
		--action=NO_ACTION \
		--certificate="" \
		--mfg_rot="" \
		--mfg_encryptor_args="" \
		--mfg_roe_ek="" \
		--mfg_roe_iv="" \
		--fld_rot=$(bcm_sec_tpl_rot) \
		--fld_encryptor_args=$(bcm_sec_tpl_roe_options) \
		--fld_roe_ek=$(bcm_sec_tpl_roe_ek) \
		--fld_roe_iv=$(bcm_sec_tpl_roe_iv) \
		--blob=$(FSBL_INTERIM) \
		--encryptor=$(topbuilddir)/$(bcm_sec_encryptor) \
		--signer=$(topbuilddir)/$(bcm_sec_signer) \
		--output=$(objdir)/binaries/gen4-sec-u-boot-tpl.bin_headered \
		--build_dir=$(objdir) \
		--crc_calculator=$(topbuilddir)/hostTools/gencrc32
endif
endif
.PHONY: gen4_sec_tpl

ifneq ($(bcm_uboot_mk_include),)

include $(bcm_uboot_mk_include)

endif

