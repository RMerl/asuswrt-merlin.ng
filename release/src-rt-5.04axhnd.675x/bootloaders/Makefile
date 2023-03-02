UB := u-boot-2019.07
ATF := armtf
INC_ATF :=
OPTEE := ../secureos/optee
INC_OPTEE :=
PATH:=$(PATH):$(CURDIR)/obj/uboot/scripts/dtc

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


include build/scripts/bootloader_version.mk
BUILD_TAG="$(BRANCH_STR)@$(word 2,$(VERSION_STR))"
-include build/configs/$(OPTIONS).conf

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

ifneq ($(and $(strip $(bcm_uboot_spl_encrypt)),$(strip $(bcm_uboot_spl_encryptor))),)
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
	for i in $(bcm_uboot_spl_arch) ; do ./build/configs/$$i.arch $@; done 

endif

ubootconfig := $(UBOOT_CONFIG)

ifneq ($(strip $(bcm_uboot_arch)),)

ubootconfig := tmp_$(bcm_uboot_uboot_prefix)$(UBOOT_CONFIG)

tmpconfig: $(UB)/configs/$(ubootconfig) 

.PHONY: tmpconfig

$(UB)/configs/$(ubootconfig) : $(UB)/configs/$(UBOOT_CONFIG)
	cat $< > $@
	for i in $(bcm_uboot_arch) ; do ./build/configs/$$i.arch $@; done 

endif

ifneq ($(strip $(SEC_ARCH)),XIP)
SEC_MODE := --nonsec
endif

ifeq ($(STANDALONE_TYPE), dpfe)
GEN_HASH_DPFE_OPTION := $(addprefix --dpfeout=$(objdir)/binaries/,$(addsuffix _dpfes.bin_headered,$(STANDALONE_CONFIGS)))
endif


OBJDUMP   = $(UBOOT_CROSS_COMPILE)objdump
OBJCOPY   = $(UBOOT_CROSS_COMPILE)objcopy

defaultrule: spl

changed_check: 
	mkdir -p $(objdir)/binaries
	mkdir -p $(ubootobjdir)
	mkdir -p $(splobjdir)
	mkdir -p $(atfobjdir)
	mkdir -p $(opteeobjdir)
	@if [ "$(LAST_OPTIONS)" != "" ]; then \
                if [ "$(LAST_CHIP)" != "$(BRCM_CHIP)" ]; then \
                        echo "You must do a make clean before rebuilding."; \
                        echo; \
                        exit 1; \
                fi \
        fi
	@echo "$(OPTIONS):$(BRCM_CHIP)" > $(LAST_OPTIONS_COOKIE); 


# unpack target forces uboot symlinks to be created if absent

unpack: $(UB)/tools/dtoc/dtoc 

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
	$(MAKE) -j8 -C $(UB) O=$(ubootobjdir) CROSS_COMPILE=$(UBOOT_CROSS_COMPILE) HOSTCC="$(HOSTCC)" tools

configure: $(ubootobjdir)/.config  $(splobjdir)/.config gen_html_headers rt_ver
	-rm -f $(splobjdir)/include/generated/hashtable.h
ifeq ($(strip $(BLD_COMMON)),y)
	-rm -f $(ubootobjdir)/include/generated/hashtable.h
endif
	@cp -ar $(objdir).$(shell echo $(BUILD_NAME) | tr A-Z a-z)/* $(objdir)/

$(splobjdir)/.config : $(UB)/configs/$(splconfig) | unpack changed_check 
	$(MAKE) -C $(UB) O=$(splobjdir) $(splconfig)  HOSTCC="$(HOSTCC)"

ifneq ($(strip $(BLD_COMMON)),y)
$(splobjdir)/include/generated/hashtable.h : 
else
$(splobjdir)/include/generated/hashtable.h : $(ubootobjdir)/include/generated/hashtable.h
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

spl: configure $(splobjdir)/include/generated/hashtable.h
	$(MAKE) -j8 -C $(UB) O=$(splobjdir) spl/u-boot-spl.bin CROSS_COMPILE=$(UBOOT_CROSS_COMPILE) HOSTCC="$(HOSTCC)" BUILD_TAG=$(BUILD_TAG)
	$(OBJDUMP) -d $(splobjdir)/spl/u-boot-spl > $(splobjdir)/spl/u-boot-spl.dis

cmm: spl | uboot
	$(MAKE) -C build/support/jtag/misc $(BRCM_CHIP) CROSS_COMPILE=$(UBOOT_CROSS_COMPILE)
	build/work/generate_cmm.sh $(objdir) $(BRCM_CHIP) $(UBOOT_CROSS_COMPILE) $(splobjdir)/spl/
	build/work/generate_cmm_legacy.py $(objdir) $(BRCM_CHIP) $(UBOOT_CROSS_COMPILE) $(splobjdir)/spl/

tpl: configure
	$(MAKE) -j8 -C $(UB) O=$(ubootobjdir) tpl/u-boot-tpl.bin CROSS_COMPILE=$(UBOOT_CROSS_COMPILE) HOSTCC="$(HOSTCC)" BUILD_TAG=$(BUILD_TAG)
	$(OBJDUMP) -d $(ubootobjdir)/tpl/u-boot-tpl > $(ubootobjdir)/tpl/u-boot-tpl.dis

uboot: configure 
	$(MAKE) -j8 -C $(UB) O=$(ubootobjdir) u-boot.bin CROSS_COMPILE=$(UBOOT_CROSS_COMPILE) HOSTCC="$(HOSTCC)" BUILD_TAG=$(BUILD_TAG)
	$(OBJDUMP) -d $(ubootobjdir)/u-boot > $(ubootobjdir)/u-boot.dis
	mkdir -p $(objdir)/binaries/linux/
	-cp ../targets/$(PROFILE)/image_ident $(objdir)/binaries/linux/
	-cp ../kernel/dts/$(BRCM_CHIP)/*.dtb $(objdir)/binaries/linux/

ikos: spl | uboot
	$(OBJCOPY) --output-target=srec --input-target=binary --srec-forceS3 --change-addresses=0xFFE00000  $(splobjdir)/spl/u-boot-spl.bin  $(splobjdir)/spl/u-boot-spl.srec
	$(OBJCOPY) --output-target=srec --input-target=binary --srec-forceS3 --change-addresses=0x05000000 $(ubootobjdir)/tpl/u-boot-tpl.bin  $(ubootobjdir)/tpl/u-boot-tpl.srec
	$(OBJCOPY) --output-target=srec --srec-forceS3 $(ubootobjdir)/u-boot  $(ubootobjdir)/u-boot.srec
	cp -f $(splobjdir)/spl/u-boot-spl.srec $(objdir)/binaries/bootimg.srec
	cat $(ubootobjdir)/tpl/u-boot-tpl.srec >> $(objdir)/binaries/bootimg.srec
	cat $(ubootobjdir)/u-boot.srec >> $(objdir)/binaries/bootimg.srec

-include $(UB)/configs/bcm9$(BRCM_CHIP)_defconfig

ifeq ($(strip $(INCLUDE_OPTEE)),y)
INC_OPTEE := _optee
endif

ifeq ($(strip $(INCLUDE_ATF)),y)
INC_ATF := _atf
endif

clean:
	rm -rf $(objdir)
	rm -f $(UB)/configs/DDR*_$(UBOOT_CONFIG)
	rm -f $(UB)/configs/tmp_*defconfig
	-$(MAKE) -C $(UB)/drivers/net/bcmbca/xrdp_full/ clean

##############################################################################
# HASHTABLE GENERATION                                                       #
# NOTE: Hashtable contains hashes of ddr binary, mcbs, dpfe overlays and TPL #
#       ,will also generate headered verions of above binaries               #
##############################################################################
$(ubootobjdir)/include/generated/hashtable.h: standalone tpl changed_check|encrypt_tpl
	mkdir -p $(dir $@)
	./build/work/generate_hashes --hashout=$@ --mcbout=$(objdir)/binaries/mcbs.bin_headered $(GEN_HASH_DPFE_OPTION) \
		$(addprefix --ddr=obj/,$(addsuffix /$(DDRMOD),$(STANDALONE_CONFIGS))) $(BRCM_CHIP) --tpl=$(ubootobjdir)/tpl/u-boot-tpl.bin
	ls $(objdir)/binaries
	for i in $(STANDALONE_CONFIGS) ; do cp $(objdir)/$$i/$(DDRMOD)_headered $(objdir)/binaries/$${i}_$(notdir $(DDRMOD))_headered ; done 
	cp $(ubootobjdir)/tpl/u-boot-tpl.bin_headered $(objdir)/binaries/

##############################################################################
# STANDALONE GENERATION                                                      #
# NOTE: Standalones comprise of ddr binary                                   #
##############################################################################
.SECONDARY : $(patsubst %,$(UB)/configs/%_$(UBOOT_CONFIG),$(STANDALONE_CONFIGS)) $(objdir)/binaries/mcbs.bin_headered

standalone: $(patsubst %,$(objdir)/%/spl/$(DDRMOD),$(STANDALONE_CONFIGS))

$(objdir)/%/spl/$(DDRMOD) : $(UB)/$(dir $(DDRMOD))/*.[ch]
	echo "ddr_bin rule called with " $@
	echo $(UB)/configs/$(subst $(objdir)/,,$(basename $(subst /spl/$(DDRMOD),,$@))_$(UBOOT_CONFIG)) 
	$(MAKE) -f $(thismakefile) $(UB)/configs/$(subst $(objdir)/,,$(basename $(subst /spl/$(DDRMOD),,$@))_$(UBOOT_CONFIG)) 
	$(MAKE) -f $(thismakefile) $(subst /spl/$(DDRMOD),,$@)/.config
	work=$(subst /spl/$(DDRMOD),,$@) ;\
	   $(MAKE) -j8 -C $(UB) O=$$work  $(dir $(DDRMOD)) CROSS_COMPILE=$(UBOOT_CROSS_COMPILE) HOSTCC="$(HOSTCC)" V=1 BUILD_TAG=$(BUILD_TAG)

$(objdir)/%/.config : $(UB)/configs/%_$(UBOOT_CONFIG) 
	$(MAKE) -C $(UB) O=$(dir $@) $(notdir $<)  HOSTCC="$(HOSTCC)"

$(UB)/configs/%_$(UBOOT_CONFIG): | unpack changed_check
	cat $(UB)/configs/$(UBOOT_CONFIG) > $@
ifneq ($strip $(bcm_uboot_spl_arch)),)
	for i in $(bcm_uboot_spl_arch) ; do ./build/configs/$$i.arch $@; done
endif
# apply the static DDR3/4 configuration
	DDR=$(subst _$(UBOOT_CONFIG),,$(notdir $@)) ; \
	  ./build/configs/$$DDR.arch $@

##############################################################################
# UBOOT DEFAULT ENVIRONMENT GENERATION                                       #
##############################################################################
$(objdir)/binaries/$(bcm_uboot_spl_prefix)env.bin_headered: | spl
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
else
ifneq ($(strip $(BLD_LOADER)),source)
IS_ABS  := $(filter /%,$(BLD_LOADER))
ifneq ($(strip $(IS_ABS)),)
SPECLOADERIMG = $(BLD_LOADER)
else
SPECLOADERIMG = $(topbuilddir)/$(BLD_LOADER)
endif
LOADERBIN = $(notdir $(SPECLOADERIMG))
else
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
image_bootstrap_nand: $(bootstrap_deps)

endif

ifeq ($(strip $(findstring emmc,$(bcm_uboot_spl_prefix))),emmc)

image_linux_deps = image_linux_emmc
image_bootstrap_deps = image_bootstrap_emmc
image_linux_emmc: | image_linux_fit
image_bootstrap_emmc: $(bootstrap_deps)

endif

ifeq ($(strip $(findstring spinor,$(bcm_uboot_spl_prefix))),spinor)
image_linux_deps = image_linux_spinor
image_bootstrap_deps = image_bootstrap_spinor
image_linux_spinor: | image_linux_fit
image_bootstrap_spinor: $(bootstrap_deps)
endif

# 1 - output binary 
# 2 - objdir
# 3 - defconfig name
define gen_extra_spl
	$(MAKE) -C $(UB) O=$2 $3  HOSTCC="$(HOSTCC)"
	$(MAKE) -j8 -C $(UB) O=$2 spl/u-boot-spl.bin CROSS_COMPILE=$(UBOOT_CROSS_COMPILE) HOSTCC="$(HOSTCC)"
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
	./build/configs/tkprog.arch $@
	# for i in $(bcm_uboot_spl_arch) ; do ./build/configs/$$i.arch $@; done 

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
		--out $@
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
ifneq ($(and $(strip $(bcm_uboot_tpl_encrypt)),$(strip $(bcm_uboot_spl_encryptor))),)
	mv -vf $(ubootobjdir)/tpl/u-boot-tpl.bin  $(ubootobjdir)/tpl/u-boot-tpl.bin.unenc
	build/scripts/$(bcm_uboot_spl_encryptor) $(ubootobjdir)/tpl/u-boot-tpl.bin.unenc $(patsubst %,$(topbuilddir)/%,"$(strip $(bcm_uboot_tpl_encrypt))")  > $(ubootobjdir)/tpl/u-boot-tpl.bin
endif

.PHONY: encrypt_tpl

# Loader image
ifeq ($(strip $(BLD_LOADER)),source)
loaderimage: spl $(objdir)/binaries/$(bcm_uboot_spl_prefix)env.bin_headered $(loader_extra_deps)
	# build loader from layout file
	-rm -f $(objdir)/binaries/$(bcm_uboot_spl_prefix)u-boot-spl.bin_headered
	echo > $(objdir)/binaries/$(bcm_uboot_spl_prefix)u-boot-spl.bin_headered
	PERL5LIB=$(HOSTTOOLS_DIR)/PerlLib/ $(HOSTTOOLS_DIR)/imagetools/insertboot --cfe $(splobjdir)/spl/u-boot-spl.bin,$(BOOTLOADER_EXTRA) \
		--arch $(SEC_ARCH) $(SEC_MODE) --chip $(BRCM_CHIP) $(objdir)/binaries/$(bcm_uboot_spl_prefix)u-boot-spl.bin_headered
ifeq ($(strip $(findstring field,$(bcm_uboot_spl_secmode))),field)
	# Prepare headered spl for signature
	-rm -f $(objdir)/binaries/$(bcm_uboot_spl_prefix)u-boot-spl.auth_header
	echo > $(objdir)/binaries/$(bcm_uboot_spl_prefix)u-boot-spl.auth_header
	PERL5LIB=$(HOSTTOOLS_DIR)/PerlLib/ $(HOSTTOOLS_DIR)/imagetools/insertboot --cfe $(splobjdir)/spl/u-boot-spl.bin,$(BOOTLOADER_EXTRA) \
		--arch $(SEC_ARCH)  --chip $(BRCM_CHIP) --field --cred=$(FLD_CRED_LIST) --out $(objdir)/binaries/$(bcm_uboot_spl_prefix)u-boot-spl.auth_header
	## FIXME -- does signable need to include BOOTLOADER_EXTRA??
ifneq ($(and $(strip $(bcm_uboot_spl_encrypt)),$(strip $(bcm_uboot_spl_encryptor))),)
	build/scripts/$(bcm_uboot_spl_encryptor)  $(splobjdir)/spl/u-boot-spl.bin "$(patsubst %,$(topbuilddir)/%,$(bcm_uboot_spl_encrypt))" > $(objdir)/binaries/$(bcm_uboot_spl_prefix)u-boot-spl.enc
	cat $(objdir)/binaries/$(bcm_uboot_spl_prefix)u-boot-spl.auth_header $(objdir)/binaries/$(bcm_uboot_spl_prefix)u-boot-spl.enc > $(objdir)/binaries/$(bcm_uboot_spl_prefix)u-boot-spl.signable
else
	cat $(objdir)/binaries/$(bcm_uboot_spl_prefix)u-boot-spl.auth_header $(splobjdir)/spl/u-boot-spl.bin > $(objdir)/binaries/$(bcm_uboot_spl_prefix)u-boot-spl.signable
endif
ifneq ($(and $(strip $(bcm_uboot_spl_sign)),$(strip $(bcm_uboot_spl_signer))),)
	build/scripts/$(bcm_uboot_spl_signer) $(objdir)/binaries/$(bcm_uboot_spl_prefix)u-boot-spl.signable $(objdir)/binaries/$(bcm_uboot_spl_prefix)u-boot-spl.sig $(bcm_uboot_spl_sign)
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
	-rm -f $(objdir)/binaries/$(bcm_uboot_spl_prefix)u-boot-spl.field
	echo > $(objdir)/binaries/$(bcm_uboot_spl_prefix)u-boot-spl.field
ifneq ($(and $(strip $(bcm_uboot_spl_encrypt)),$(strip $(bcm_uboot_spl_encryptor))),)
	PERL5LIB=$(HOSTTOOLS_DIR)/PerlLib/ $(HOSTTOOLS_DIR)/imagetools/insertboot --cfe  $(objdir)/binaries/$(bcm_uboot_spl_prefix)u-boot-spl.enc,$(BOOTLOADER_EXTRA) \
		--arch $(SEC_ARCH)  --chip $(BRCM_CHIP) --field=$(objdir)/binaries/$(bcm_uboot_spl_prefix)u-boot-spl.sig \
		--cred=$(FLD_CRED_LIST) --offset 0  $(objdir)/binaries/$(bcm_uboot_spl_prefix)u-boot-spl.field
else
	PERL5LIB=$(HOSTTOOLS_DIR)/PerlLib/ $(HOSTTOOLS_DIR)/imagetools/insertboot --cfe $(splobjdir)/spl/u-boot-spl.bin,$(BOOTLOADER_EXTRA) \
		--arch $(SEC_ARCH)  --chip $(BRCM_CHIP) --field=$(objdir)/binaries/$(bcm_uboot_spl_prefix)u-boot-spl.sig \
		--cred=$(FLD_CRED_LIST) --offset 0  $(objdir)/binaries/$(bcm_uboot_spl_prefix)u-boot-spl.field
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

# Bootstrap FIT image
ifneq ($(strip $(BLD_COMMON)),y)
image_bootstrap_fit: 
else
image_bootstrap_fit: uboot
endif
ifeq ($(strip $(INCLUDE_OPTEE)),y)
	cp -f $(OPTEE)/tee.bin $(opteeobjdir)
endif
ifeq ($(strip $(INCLUDE_ATF)),y)
	cp -f $(ATF)/armtf.bin $(atfobjdir)
endif
	# workaround rm -f ~/bin/dtc
	mkdir -p $(objdir)/binaries/linux/
	-cp ../targets/$(PROFILE)/image_ident $(objdir)/binaries/linux/
	-cp ../kernel/dts/$(BRCM_CHIP)/*.dtb $(objdir)/binaries/linux/
	./build/work/generate_linux_its --simple --dir=obj/binaries/linux/ --chip=$(BRCM_CHIP) \
		$(if $(bcm_uboot_delg_sec_pol),--security_policy=../$(bcm_uboot_delg_sec_pol)) \
		$(if $(bcm_uboot_fit_security),--securitydefs=$(bcm_uboot_fit_security)) \
		--arch=$(ARCH) $(if $(INC_ATF),--armtf) > obj/binaries/brcm_simple.its
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

# Bootstrap images for nand		
# 1 - Erase block size
# 2 - Min i/o unit size
define gen_nand_bootstrap_bins
	# $(BOOTSTRAPFIT) already exists 
	@echo "We are assuming (unwisely) that ubinize is already built"; 
	../hostTools/mtd-utils-2.0.1/ubinize -o obj/binaries/fit$1.ubi -v -m $2 -p $1KiB build/work/fit_simple.ubinize; \
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

image_bootstrap: $(image_bootstrap_deps)

image_linux_fit: image_bootstrap
	echo "RUNNING image_linux_fit"
	cp ../targets/$(PROFILE)/vmlinux* $(objdir)/uboot/
ifeq ($(strip $(bcm_uboot_kernel_compression)),lzma)
	$(XZ) --format=lzma -f --stdout -e  $(objdir)/uboot/vmlinux.bin > $(objdir)/uboot/vmlinux.bin.lzma
endif
ifeq ($(strip $(bcm_uboot_kernel_compression)),lzo)
	../hostTools/local_install/lzop  < $(objdir)/uboot/vmlinux.bin > $(objdir)/uboot/vmlinux.bin.lzo
endif
	cp ../targets/$(PROFILE)/vmlinux* $(objdir)/binaries/linux/
	-cp ../targets/$(PROFILE)/rootfs_* $(objdir)/binaries
	-cp ../targets/$(PROFILE)/rootfs.img $(objdir)/binaries/rootfs.squashfs
	-cp ../targets/$(PROFILE)/rootfs.ext4 $(objdir)/binaries
ifneq ($(strip $(bcm_uboot_rootfs_encrypt)),)
	build/scripts/${bcm_uboot_rootfs_encrypt}
endif
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
		$(if $(bcm_uboot_fit_security),--securitydefs=$(bcm_uboot_fit_security)) \
		$(if $(bcm_uboot_kernel_compression),--kernel_compression=$(bcm_uboot_kernel_compression)) \
		$(if $(bcm_uboot_pmcfw_code_addr),--pmcfw_code_addr=$(bcm_uboot_pmcfw_code_addr)) \
		$(if $(bcm_uboot_pmcfw_data_addr),--pmcfw_data_addr=$(bcm_uboot_pmcfw_data_addr)) \
		--arch=$(ARCH) $(if $(INC_ATF),--armtf) $(if $(INC_OPTEE),--optee) > obj/binaries/brcm_full_linux.its
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

# 1 - Loader binary
# 2 - Flash type
# 3 - Rootfs binary
define gen_pkgtb_bundles
	if [ -f obj/binaries/$3 ]; then \
		./build/work/generate_bundle_itb --chip=$(BRCM_CHIP) --profile=$(PROFILE) --compatstr=${bcm_image_compatstr}  --loader=$1 --bootfs=$(LINUXFIT) --rootfsspec=$2,$3; \
	else \
		echo "$3 not found --> Skipping its .pkgtb generation"; \
	fi
endef

# Image bundles for nand

# 1 - Erase block size
# 2 - Min i/o unit size
define gen_nand_raw_imgs
	if [ -f obj/binaries/rootfs_$1.ubifs ]; then \
		echo "We are assuming (unwisely) that ubinize is already built"; \
		../hostTools/mtd-utils-2.0.1/ubinize -o obj/binaries/linux$1.ubi -v -m $2 -p $1KiB build/work/linux_simple$1.ubinize; \
		cp $(LOADERIMG) $(objdir)/binaries/linux_raw_image_$1.bin; \
		dd if=obj/binaries/linux$1.ubi of=$(objdir)/binaries/linux_raw_image_$1.bin  bs=$(bcm_uboot_image_offset) seek=1 conv=notrunc; \
	else \
		echo "rootfs_$1.ubifs not found --> Skipping its raw image build"; \
	fi ;  \
	if [ -f obj/binaries/${uboot_squash_fname} ]; then \
		echo "We are assuming (unwisely) that ubinize is already built"; \
		cp obj/binaries/${uboot_squash_fname} obj/binaries/rootfs_nonubifs ; \
		../hostTools/mtd-utils-2.0.1/ubinize -o obj/binaries/linux_squash_$1.ubi -v -m $2 -p $1KiB build/work/linux_simple_nonubifs.ubinize; \
		cp $(LOADERIMG) $(objdir)/binaries/linux_raw_squash_image_$1.bin; \
		dd if=obj/binaries/linux_squash_$1.ubi of=$(objdir)/binaries/linux_raw_squash_image_$1.bin  bs=$(bcm_uboot_image_offset) seek=1 conv=notrunc; \
	else \
		echo "rootfs_$1.ubifs not found --> Skipping its raw image build"; \
	fi  
endef

image_linux_nand sign_image_linux_nand: 
	# Add signatures if required
	@$(call add_fit_sig,$(LINUXFIT))
	# Generate loader + linux pkgtb images
	@$(call gen_pkgtb_bundles,$(LOADERBIN),"nand",${uboot_squash_fname})
ifeq ($(strip $(findstring field,$(bcm_uboot_spl_secmode))),)
	@$(call gen_pkgtb_bundles,$(LOADERBIN),"nand128","rootfs_128.ubifs")
	@$(call gen_pkgtb_bundles,$(LOADERBIN),"nand256","rootfs_256.ubifs")
endif
	# Generate linux pkgtb images
	@$(call gen_pkgtb_bundles,"","nand",${uboot_squash_fname})
ifeq ($(strip $(findstring field,$(bcm_uboot_spl_secmode))),)
	@$(call gen_pkgtb_bundles,"","nand128","rootfs_128.ubifs")
	@$(call gen_pkgtb_bundles,"","nand256","rootfs_256.ubifs")
endif
	# Generate RAW images for nand flash programmers
	@$(call gen_nand_raw_imgs,"128","2048")
	@$(call gen_nand_raw_imgs,"256","4096")
ifeq ($(BCM4906_504),y)
	cp obj/binaries/bcm9$(CUR_CHIP_PROFILE)G_nand_squashfs_update.pkgtb obj/binaries/$(IMGNAME)_nand_squashfs.pkgtb
else
	cp obj/binaries/bcm9$(CUR_CHIP_PROFILE)GW_nand_squashfs_update.pkgtb obj/binaries/$(IMGNAME)_nand_squashfs.pkgtb
endif

# Image bundles for emmc
 
# 1 - rootfs binary
define gen_emmc_raw_imgs
	./build/work/gen_emmc_linux_raw_img.sh $(opts_gen_emmc_linux_raw) obj/binaries/$(LINUXFIT) obj/binaries/$1 $(objdir)/binaries/test_metadata.bin_headered obj/binaries/linux_raw_emmc_userdata_part.bin
endef

image_linux_emmc sign_image_linux_emmc: 
	echo "RUNNING image_linux_emmc"
	# Add signatures if required
	@$(call add_fit_sig,$(LINUXFIT))
	# Generate loader + linux pkgtb images
	@$(call gen_pkgtb_bundles,$(LOADERBIN),"emmc",${uboot_squash_fname})
ifeq ($(strip $(findstring field,$(bcm_uboot_spl_secmode))),)
	@$(call gen_pkgtb_bundles,$(LOADERBIN),"emmc","rootfs.ext4")
endif
	# Generate linux pkgtb images
	@$(call gen_pkgtb_bundles,"","emmc",${uboot_squash_fname})
ifeq ($(strip $(findstring field,$(bcm_uboot_spl_secmode))),)
	@$(call gen_pkgtb_bundles,"","emmc","rootfs.ext4")
endif
	# Generate RAW images for emmc flash programmers
	@$(call gen_emmc_raw_imgs,${uboot_squash_fname})

define gen_spinor_raw_imgs
	cp $(LOADERIMG) $(objdir)/binaries/$(PROFILE)_spinor_loader_linux_squashfs.bin; \
	dd if=$(objdir)/binaries/$(LINUXFIT) of=$(objdir)/binaries/$(PROFILE)_spinor_loader_linux_squashfs.bin  bs=$(bcm_uboot_image_offset) seek=1 conv=notrunc; \
	dd if=$(objdir)/binaries/rootfs.squashfs of=$(objdir)/binaries/$(PROFILE)_spinor_loader_linux_squashfs.bin  bs=$(bcm_spinor_squashfs_offset) seek=1 conv=notrunc; 
endef

image_linux_spinor sign_image_linux_spinor: 
	echo "RUNNING image_linux_spinor"
	# Add signatures if required
	@$(call add_fit_sig,$(LINUXFIT))
	# Generate loader + linux pkgtb images
	@$(call gen_pkgtb_bundles,$(LOADERBIN),"spinor",${uboot_squash_fname})
	# Generate linux pkgtb images
	@$(call gen_pkgtb_bundles,"","spinor",${uboot_squash_fname})
	# Generate RAW images for spinor flash programmers
	@$(call gen_spinor_raw_imgs)

image_linux: $(image_linux_deps)

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


endif

ifneq ($(bcm_uboot_mk_include),)

include $(bcm_uboot_mk_include)

endif

