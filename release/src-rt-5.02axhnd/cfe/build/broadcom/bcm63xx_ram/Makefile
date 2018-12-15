ifneq ($(wildcard ../../../../.config),)
include ../../../../.config
endif

ifeq ($(SWITCH2),BCM53134)
CFLAGS += -DEXT_BCM53134
export HND_EXT_53134 := y
endif
ifeq ($(EXT_PHY), BCM84880)
CFLAGS += -DEXT_BCM84880
endif
.NOTPARALLEL :

CFG_64BIT    ?= 0       # cpu in 64 bit mode
CFG_MLONG64  ?= 0       # 64 bit long integer
CFG_LITTLE   ?= 0       # Mindbender
CFG_RELOC    ?= 0
CFG_UNCACHED ?= 0       # Mindbender
CFG_NEWRELOC ?= 0
CFG_VAPI     ?= 0 
CFG_BOOTRAM  ?= 0
CFG_RAMAPP   ?= 1
CFG_BOARDNAME = "BCM9$(BRCM_CHIP)"
CFG_PCI       = 0
CFG_ZLIB      = 0		# enable zlib
CFG_COPY_PSRAM ?= 0
CFG_BOOT_PSRAM ?= 0
CFG_DT         ?= 1
CFG_ATAG       ?= 0
CFG_NONSEC_BOOT ?= 0

COMPRESS ?= cmplzma
ENCRYPTRAM ?= SecureBootUtils/makeEncryptedCfeRamForNor

TOP   = ../../../cfe
ARCH  ?= mips
CPU   ?= bcmcore
BOARD = bcm63xx

# configuration for chips: 63138, 63148
ifneq ($(findstring _$(strip $(BRCM_CHIP))_,_63138_63148_47189_6846_),)
CFG_LDR_SREC := 0
ARCH = arm
CPU = armv7
CFG_LITTLE = 1
# enable nonsecure boot only for 6846 and 63148.
ifneq ($(findstring _$(strip $(BRCM_CHIP))_,_6846_63148_),)
CFG_NONSEC_BOOT = 1
endif
endif

ifneq ($(findstring _$(strip $(BRCM_CHIP))_,_6838_6848_),)
CFG_KER_LZ4 = 1
else
CFG_KER_LZ4 = 0
endif

ifeq ($(BRCM_CHIP), 60333)
CFG_LITTLE = 1
endif

ifneq ($(findstring _$(strip $(BRCM_CHIP))_,_63138_63148_63381_63268_6838_),)
SEC_ARCH=2
else
ifneq ($(findstring _$(strip $(BRCM_CHIP))_,_6846_6856_6858_4908_63158_),)
SEC_ARCH=3
endif
endif

ifneq ($(findstring _$(strip $(BRCM_CHIP))_,_4908_6858_63158_6856_),)
CFG_MLONG64 = 1
CFG_ARMV8_AARCH64 = 0
CFG_ARMV8_AARCH32 = 0
CFG_64BIT = 1     # in ARM AArch64 mode
ARCH = arm

ifeq ($(strip $(CFG_64BIT)),1)
CPU = armv8
CFG_ARMV8_AARCH64 = 1

# device tree support. Both mips and arm support device tree in linux
# but currently only used in ARMv8 based chip
else
CPU = armv7       # ARM AArch32 mode use armv7 code.   
CFG_ARMV8_AARCH32 = 1
endif
CFG_LITTLE = 1
CFG_ROM_PRINTF = 0
IKOS_NO_DDRINIT ?= 0
CFG_KER_LZ4 = 0
endif

ifeq ($(strip ${CFG_LDR_SREC}),1)
CFLAGS += -DCFG_LDR_SREC=1
endif

# Configurable cfe ram link address based on the chip and total memory on board
# considering both linux kernel space below this address and image staging buffer
# above this address

# default link address for mips, up to 10MB kernel size
RAMAPP_TEXT = 0x80a01000
ifneq ($(findstring _$(strip $(BRCM_CHIP))_,_63138_63148_47189_6846_),)
# ARM based chip with 128MB memory on board, up to 15MB kernel size
RAMAPP_TEXT = 0x00f00000
endif
ifneq ($(findstring _$(strip $(BRCM_CHIP))_,_60333_),)
# 6318, 60333 with 8MB memory on board, up to 6MB kernel size
RAMAPP_TEXT = 0x80601000
endif
ifneq ($(findstring _$(strip $(BRCM_CHIP))_,_4908_6858_63158_6856_),)
RAMAPP_TEXT = 0x01000000
endif

HOSTTOOLS = ../../../../hostTools
SHARED_DIR = ../../../../shared
BCMDRIVERS_DIR = ../../../../bcmdrivers
RDP_DIR = ../../../../rdp
XRDP_DIR_ORG = ../../../cfe/xrdp
BOARDPARMS_DIR = $(SHARED_DIR)/opensource/boardparms/bcm963xx
FLASH_DIR = $(SHARED_DIR)/opensource/flash
UTILS_DIR = $(SHARED_DIR)/opensource/utils
SHARED_DRIVERS_DIR = $(SHARED_DIR)/opensource/drivers
BCMSPI_DIR = $(SHARED_DIR)/opensource/spi
INC_SHARED_PUB_PATH = $(SHARED_DIR)/opensource/include/bcm963xx
INC_SHARED_PRIV_PATH = $(SHARED_DIR)/broadcom/include/bcm963xx
ROM_BUILD = ../$(BOARD)_rom
ASM_FILE = $(ROM_BUILD)/flashimg.S

ifeq ($(strip $(CFG_SBP)),1)
	CFLAGS += -DUSE_SINGLE_BOARD_PARAM 
endif

include $(TOP)/board/$(BOARD)_ram/src/ram_cfe.mk

ifneq ($(HND),)
VERSION_MAKE_FILE := version.hnd
else 
ifneq ($(wildcard ../../../../version.make),)
VERSION_MAKE_FILE := version.make
else
VERSION_MAKE_FILE := version.hnd
endif
endif

CFLAGS += -DCONFIG_BCM9$(BRCM_CHIP)

include ../../../../$(VERSION_MAKE_FILE)
CFLAGS += -DBRCM_VERSION=$(BRCM_VERSION) -DBRCM_RELEASE=$(BRCM_RELEASE) -DBRCM_EXTRAVERSION="\"$(BRCM_EXTRAVERSION)\""

ifeq ($(strip $(CFG_DT)),1)
CFLAGS += -DCFG_DTB_IMAGE="\"9$(BRCM_CHIP).dtb\"" 
endif

# Use VPATH for all source dirs within cfe/, vpath %.filetype for all external source dirs
VPATH += ${TOP}/board/$(BOARD)_ram/src/html 

vpath %.c ${BOARDPARMS_DIR} $(BCMSPI_DIR) ${FLASH_DIR} ${UTILS_DIR} ${SHARED_DRIVERS_DIR} 
INCDIRS += -I${INC_SHARED_PUB_PATH} -I${INC_SHARED_PRIV_PATH}
ifeq ($(strip $(INC_PMC_DRIVER)),1)
INCDIRS += -I$(INC_SHARED_PUB_PATH)/../pmc
ifeq ($(strip $(BRCM_CHIP)),6838)
vpath %.c $(SHARED_DIR)/opensource/pmc/impl2
else
vpath %.c $(SHARED_DIR)/opensource/pmc/impl1
endif
endif

SHA_DIR = ../../../cfe/sha256
AES_DIR = ../../../cfe/aes128
OTP_DIR = ../../../cfe/otp
INCDIRS += -I${SHA_DIR} -I${AES_DIR} -I${OTP_DIR}

ifneq ($(findstring _$(strip $(BRCM_CHIP))_,_63158_),)
CFLAGS += -DCONFIG_BP_PHYS_INTF
endif

#cathy
ifneq ($(findstring _$(strip $(BRCM_CHIP))_,_4908_),)
ifeq ($(strip $(CFG_2P5G_LAN)),1)
INCDIRS += -I$(SHARED_DIR)/opensource/include/drv/phys
INCDIRS += -I$(SHARED_DIR)/opensource/include/drv/mdio
INCDIRS += -I$(BCMDRIVERS_DIR)/opensource/net/enet/shared
INCDIRS += -I$(BCMDRIVERS_DIR)/broadcom/include/bcm963xx
vpath %.c $(SHARED_DIR)/opensource/drv/phys
vpath %.c $(SHARED_DIR)/opensource/drv/mdio
CFLAGS += -D_BYTE_ORDER_LITTLE_ENDIAN_
CFLAGS += -DCFG_2P5G_LAN
#CFLAGS += -DCFG_2P5G_LAN_DEBUG
endif
endif

ifneq ($(findstring _$(strip $(BRCM_CHIP))_,_6838_),)
ifeq ($(strip $(INC_NAND_FLASH_DRIVER)),1)
include ${SHA_DIR}/Makefile
# Single build supports both unsecure and secure boot (no sotp)
VPATH += ${SHA_DIR}
endif
endif

ifneq ($(findstring _$(strip $(BRCM_CHIP))_,_4908_6858_63158_6846_6856_),)
ifeq ($(strip $(INC_NAND_FLASH_DRIVER)),1)
VPATH += ${SHA_DIR} ${OTP_DIR} ${AES_DIR}
include ${OTP_DIR}/Makefile
include ${SHA_DIR}/Makefile
include ${AES_DIR}/Makefile
# Single build supports both unsecure and secure boot (has sotp)
else
ifeq ($(strip $(INC_EMMC_FLASH_DRIVER)),1)
# Single build supports both unsecure and secure boot (has sotp)
VPATH += ${SHA_DIR} ${OTP_DIR} ${AES_DIR}
include ${OTP_DIR}/Makefile
include ${SHA_DIR}/Makefile
include ${AES_DIR}/Makefile
endif
endif
endif

ifneq ($(findstring _$(strip $(BRCM_CHIP))_,_63138_63148_63381_),)
VPATH += ${SHA_DIR}
VPATH += ${OTP_DIR}
ifneq ($(strip $(INC_BTRM_BOOT)),1)
# nonsecure boot build
SHA_AUTH_ONLY=1
endif
include ${OTP_DIR}/Makefile
include ${SHA_DIR}/Makefile
endif

ifneq ($(findstring _$(strip $(BRCM_CHIP))_,_63268_),)
VPATH += ${SHA_DIR}
ifneq ($(strip $(INC_BTRM_BOOT)),1)
# secure boot build
SHA_AUTH_ONLY=1
# nonsecure boot build
endif
include ${SHA_DIR}/Makefile
endif

ifneq ($(findstring _$(strip $(BRCM_CHIP))_,_6838_6848_6858_6846_6856_),)
CFLAGS += -DCONFIG_BCM_PON
export BCM_PON=y
ifneq ($(findstring _$(strip $(BRCM_CHIP))_,_6858_6846_6856_),)
CFLAGS += -DCONFIG_BCM_PON_XRDP
export BCM_XRDP=y
endif
ifneq ($(findstring _$(strip $(BRCM_CHIP))_,_6838_6848_),)
CFLAGS += -DCONFIG_BCM_PON_RDP
export BCM_RDP=y
endif
endif

ifeq ($(strip $(NONETWORK)),0)

ifneq ($(findstring _$(strip $(BRCM_CHIP))_,_6838_6848_),)
CFLAGS += -DRDD_BASIC -DOREN -D__OREN__ -DLEGACY_RDP --include lib_types.h
vpath %.c $(SHARED_DIR)/opensource/drivers
RDP_PROJECT := PON_6838
endif

ifneq ($(findstring _$(strip $(BRCM_CHIP))_,_63138_63148_),)
CFLAGS += -DLEGACY_RDP --include lib_types.h
CFLAGS += -DRDD_BASIC -DDSL_$(BRCM_CHIP)
INCDIRS += -I$(RDP_DIR)/projects/$(RDP_PROJECT)/target/bdmf/system
RDP_PROJECT := DSL_$(BRCM_CHIP)
endif

ifneq ($(findstring _$(strip $(BRCM_CHIP))_,_6858_6846_6856_),)
RDP_PROJECT=XRDP_CFE
endif

INCDIRS += -I$(SHARED_DIR)/opensource/include/drv/phys
INCDIRS += -I$(SHARED_DIR)/opensource/include/rdp
vpath %.c $(SHARED_DIR)/opensource/drv/phys

ifneq ($(strip $(BRCM_CHIP)),6858)
vpath %.c $(SHARED_DIR)/opensource/drv/mdio
vpath %.c $(SHARED_DIR)/opensource/drv/unimac
INCDIRS += -I$(SHARED_DIR)/opensource/include/drv/mdio
INCDIRS += -I$(SHARED_DIR)/opensource/include/drv/unimac
else
vpath %.c $(SHARED_DIR)/opensource/drv/lport
vpath %.c $(SHARED_DIR)/opensource/drv/lport/ag
INCDIRS += -I$(SHARED_DIR)/opensource/drv/lport
INCDIRS += -I$(SHARED_DIR)/opensource/drv/lport/ag
endif

ifneq ($(findstring _$(strip $(BRCM_CHIP))_,_6838_6848_63138_63148_),)
INCDIRS += -I$(RDP_DIR)/projects/$(RDP_PROJECT)/target/rdd
INCDIRS += -I$(RDP_DIR)/projects/$(RDP_PROJECT)/target/bdmf/framework
INCDIRS += -I$(RDP_DIR)/projects/$(RDP_PROJECT)/target/rdp_subsystem
INCDIRS += -I$(RDP_DIR)/projects/$(RDP_PROJECT)/target/rdpa
INCDIRS += -I$(RDP_DIR)/projects/$(RDP_PROJECT)/target/rdpa_gpl
INCDIRS += -I$(SHARED_DIR)/opensource/include/drv/dpi
vpath %.c $(RDP_DIR)/projects/$(RDP_PROJECT)/target/rdp_subsystem
vpath %.c $(RDP_DIR)/projects/$(RDP_PROJECT)/target/rdd
vpath %.c $(SHARED_DIR)/opensource/drv/dpi
endif

ifneq ($(findstring _$(strip $(BRCM_CHIP))_,_6858_6846_6856_),)
ifneq ($(strip $(CONFIG_GPL_RDP_GEN)),1)
# Use GPL RDP by default
CONFIG_GPL_RDP ?= 1
LIBXRDP_CLEAN =libxrdp_clean
else
CFLAGS += -DCONFIG_GPL_RDP_GEN
CONFIG_GPL_RDP := 0
endif
INCDIRS += -I$(RDP_DIR)/projects/$(RDP_PROJECT)/target/bdmf/framework
INCDIRS += -I$(RDP_DIR)/projects/$(RDP_PROJECT)/target/bdmf/system
ifeq ("$(CONFIG_GPL_RDP)", "1")
CFLAGS += -DCONFIG_GPL_RDP
INCDIRS += -I$(RDP_DIR)/projects/$(RDP_PROJECT)/target/rdpa_gpl
XRDP_DIR = xrdp
else
XRDP_DIR = $(XRDP_DIR_ORG)
endif
CFLAGS += -DCPU_TX_RING -DCPU_TX_RING_COPY
CFLAGS += -D_CFE_REDUCED_XRDP_
CFLAGS += -DBCM$(BRCM_CHIP) -DFIRMWARE_LITTLE_ENDIAN -D_BYTE_ORDER_LITTLE_ENDIAN_
ifneq ($(findstring _$(strip $(BRCM_CHIP))_,_6858_6856_),)
CFLAGS += -DPHYS_ADDR_64BIT
endif
INCDIRS += -I$(XRDP_DIR)
XRDP_LINK = -L$(XRDP_DIR) -lxrdp
export RDP_PROJECT
export RDP_SUBSYS_PROJECT
export CONFIG_GPL_RDP
export CONFIG_GPL_RDP_GEN
endif

rdp:
ifeq ("$(CONFIG_GPL_RDP)", "1")
	mkdir -p $(XRDP_DIR)
	cp -f  $(XRDP_DIR_ORG)/*.h  $(XRDP_DIR)
	cp -f  $(XRDP_DIR_ORG)/xrdp.mk $(XRDP_DIR)/Makefile
	$(MAKE) -C $(XRDP_DIR) GAR=$(GAR) cfe
else
ifneq ($(strip $(RDP_PROJECT)),)
	$(MAKE) -C $(RDP_DIR) PROJECT=$(RDP_PROJECT) rdp_link
endif
ifneq ($(findstring _$(strip $(BRCM_CHIP))_,_6858_6846_6856_),)
	$(MAKE) -C $(XRDP_DIR) GAR=$(GAR) cfe
endif	
endif

ifeq ($(findstring _$(strip $(BRCM_CHIP))_,_60333_63268_63381_47189_),)
RDP_BUILD := rdp
endif

rdp_clean:
	$(MAKE) -C $(XRDP_DIR_ORG) clean
ifneq ($(strip $(RDP_PROJECT)),)
	$(MAKE) -C $(RDP_DIR) PROJECT=$(RDP_PROJECT) clean
else
	$(MAKE) -C $(RDP_DIR) ALL=clean
endif

RDP_CLEAN = rdp_clean

endif  #ifeq ($(strip $(NONETWORK)),0)

cmplzma_tool:
	$(MAKE) -C $(HOSTTOOLS) build_cmplzma

secbt_tools:
ifeq ($(strip $(INC_BTRM_BOOT)),1)
	$(MAKE) -C $(HOSTTOOLS) build_secbtutils
endif

# support rdp target except the chips in the list
ALL: sanity_check cmplzma_tool secbt_tools $(RDP_BUILD)
	find $(SHARED_DIR) -name "*.o" -exec rm -f "{}" ";"
	find $(BOARDPARMS_DIR) -name "*.o" -exec rm -f "{}" ";"
	${MAKE} cfe$(BRCM_CHIP)
ifeq ($(strip $(INC_BTRM_BOOT)),1)
	$(MAKE) -C $(TOP)/board/$(BOARD)_btrm/tools/src
	$(HOSTTOOLS)/$(COMPRESS) -c -2 -lzma cfe$(BRCM_CHIP) cfe$(BRCM_CHIP).bin $(ASM_FILE) -e "$(HOSTTOOLS)/$(ENCRYPTRAM) $(BRCM_CHIP) cfe$(BRCM_CHIP).bin.lzma cfe$(BRCM_CHIP).bin.lzma $(TOP)/board/$(BOARD)_btrm"
else
	$(HOSTTOOLS)/$(COMPRESS) -c -2 -lzma cfe$(BRCM_CHIP) cfe$(BRCM_CHIP).bin $(ASM_FILE)
endif

sanity_check:
	@if [ "$(BRCM_CHIP)" = "" ]; then \
		echo You must specify chip ID using 'gmake BRCM_CHIP=[60333|63268|63138|6838|63148|6848|4908|6858|63158|6846|6856]'; exit 1; \
	fi

export GCC
BASEFLAGS := $(CFLAGS)
export BASEFLAGS

$(ASM_FILE): cfe$(BRCM_CHIP) cfe$(BRCM_CHIP).bin

cfe$(BRCM_CHIP) cfe$(BRCM_CHIP).bin: $(CRT0OBJS) $(BSPOBJS) $(LIBCFE) 
	$(GLD) -o cfe$(BRCM_CHIP) -Map cfe$(BRCM_CHIP).map $(LDFLAGS) $(CRT0OBJS) $(BSPOBJS) $(XRDP_LINK) -L. -lcfe $(LDLIBS) 
	$(GOBJDUMP) -d cfe$(BRCM_CHIP) > cfe$(BRCM_CHIP).dis
	$(GOBJCOPY) --output-target=binary cfe$(BRCM_CHIP) cfe$(BRCM_CHIP).bin
	$(GOBJCOPY) --output-target=srec --srec-forceS3 -R .reginfo -R .note -R .comment -R .mdebug -S cfe$(BRCM_CHIP) cfe$(BRCM_CHIP).srec
ifeq ($(strip $(INC_NAND_FLASH_DRIVER)),1)
	LC_ALL=C $(GOBJDUMP) -x cfe$(BRCM_CHIP) | $(HOSTTOOLS)/mkCfeHeader  cferamhdr.bin
	cat cferamhdr.bin cfe$(BRCM_CHIP).bin > cfe$(BRCM_CHIP)ram.bin
	rm cferamhdr.bin
else ifeq ($(strip $(INC_EMMC_FLASH_DRIVER)),1)
	LC_ALL=C $(GOBJDUMP) -x cfe$(BRCM_CHIP) | $(HOSTTOOLS)/mkCfeHeader  cferamhdr.bin
	cat cferamhdr.bin cfe$(BRCM_CHIP).bin > cfe$(BRCM_CHIP)ram.bin
	rm cferamhdr.bin
endif

clean: $(RDP_CLEAN)
	rm -rf xrdp
	rm -f *.[oa] *~ cfe*
	rm -f $(ASM_FILE)
	
distclean: clean
