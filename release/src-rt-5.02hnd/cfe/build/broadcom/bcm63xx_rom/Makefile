ifneq ($(wildcard ../../../../.config),)
include ../../../../.config
endif

DFT_NVRAM_NAME := $(shell echo $(BUILD_NAME)|tr A-Z a-z)

CFG_64BIT    ?= 0       # cpu in 64 bit mode
CFG_MLONG64  ?= 0       # 64 bit long integer
CFG_LITTLE   ?= 0       # Mindbender
CFG_RELOC    ?= 0
CFG_UNCACHED ?= 0       # Mindbender
CFG_NEWRELOC ?= 0
CFG_VAPI     ?= 0
CFG_BOOTRAM  ?= 0
CFG_RAMAPP   ?= 0       # EPI
CFG_BOARDNAME = "BCM9$(BRCM_CHIP)"
CFG_PCI      ?= 0
CFG_ZLIB     ?= 0       # enable zlib
CFG_SBP	     ?= 0       # use Single Board Parameter if defined to 1
CFG_ROM_PRINTF ?= 0     # use this only on the chip that cfe rom runs from internal memory with enough space
SKIP_FLASH ?=0          # use this to disable flash access for nvram, flash controller still init'ed though
NONETWORK ?= 0
BPCM_CFE_CMD ?= 0	#  this is to enable read/write BPCM register CFE command
BOOT_PRE_CFE ?= 0

#sim and ikos related the options
BRCM_IKOS ?= 0          # use this to compile cfe for ikos or simulation environment.
ifeq ($(BRCM_IKOS),1)
IKOS_NO_DDRINIT ?= 1	# a specific mode for IKOS/sim where DDR does not to be initialized
IKOS_SMPL_DDRINIT ?= 1	# if ddr init is enabled in IKOS/sim, use simple or shmoo version of ddr_init routine
IKOS_BD_CFERAM ?= 0     # CFE RAM loaded through back door
IKOS_BD_LINUX ?= 0      # Linux loaded through back door in cfe ram
IKOS_BD_LINUX_ROM ?= 0  # Linux loaded through back door in cfe rom
IKOS_FAST_UART ?= 1     # Run uart at 1.5625mbps
IKOS_FAST_CACHE_INIT ?= 0 # Fast cache init, for MIPS only
SKIP_FLASH = 1          # Flash model in sim/ikos does not support write function 
NONETWORK = 1           # no network support in sim/ikos
endif

BUILD_DIR = $(shell pwd)

TOP   = $(BUILD_DIR)/../../../cfe
ARCH  ?= mips
CPU   = bcmcore
BOARD = bcm63xx

ifneq ($(findstring _$(strip $(BRCM_CHIP))_,_6838_6848_),)
CFG_ROM_PRINTF = 1
endif

ifneq ($(findstring _$(strip $(BRCM_CHIP))_,_63138_63148_47189_),)
ARCH = arm
CPU = armv7
CFG_LITTLE = 1
CFG_ROM_PRINTF = 1
endif

ifeq ($(BRCM_CHIP), 60333)
CFG_LITTLE = 1
endif

ifneq ($(findstring _$(strip $(BRCM_CHIP))_,_4908_6858_68360_),)
CFG_MLONG64 = 1
CFG_ARMV8_AARCH64 = 0
CFG_ARMV8_AARCH32 = 0
CFG_64BIT = 1     # in ARM AArch64 mode
ARCH = arm
ifeq ($(strip $(CFG_64BIT)),1)
CPU = armv8
CFG_ARMV8_AARCH64 = 1
else
CPU = armv7       # ARM AArch32 mode use armv7 code.   
CFG_ARMV8_AARCH32 = 1
endif
CFG_LITTLE = 1
CFG_ROM_PRINTF = 1

ifeq ($(strip $(BRCM_CHIP)),6858)
IKOS_NO_DDRINIT = 0
IKOS_SMPL_DDRINIT = 0
#temporary for BU since PSRAM is broken.
BOOT_MEMC_PSRAM = 1
endif

endif

LINUX_KERNEL_DIR=$(BUILD_DIR)/../../../../kernel/linux
HOSTTOOLS = $(BUILD_DIR)/../../../../hostTools
TARGETS_DIR = $(BUILD_DIR)/../../../../targets
SHARED_DIR = $(BUILD_DIR)/../../../../shared
INC_SHARED_PUB_PATH = $(SHARED_DIR)/opensource/include/bcm963xx
INC_SHARED_PRIV_PATH = $(SHARED_DIR)/broadcom/include/bcm963xx
FLASH_DIR = $(SHARED_DIR)/opensource/flash
BCMSPI_DIR = $(SHARED_DIR)/opensource/spi
DRIVERS_DIR = $(SHARED_DIR)/opensource/drivers
SHA_DIR = ../../../cfe/sha256
AES_DIR = ../../../cfe/aes128

ASM_FILE = flashimg.S
RAM_BUILD = ../$(BOARD)_ram

ifeq ($(strip ${CFG_LITTLE}),1)
ENDIAN = little
else
ENDIAN = big
endif


include $(TOP)/board/$(BOARD)_rom/src/rom_cfe.mk

# Use VPATH for all source dirs within cfe/, vpath %.filetype for all external source dirs
VPATH += ${SHA_DIR} ${AES_DIR}

vpath %.c ${FLASH_DIR} $(BCMSPI_DIR)
INCDIRS += -I$(TOP)/board/$(BOARD)_ram/include -I${INC_SHARED_PUB_PATH} -I${INC_SHARED_PRIV_PATH} -I${SHA_DIR} -I${AES_DIR}

ifeq ($(strip $(INC_PMC_DRIVER)),1)
INCDIRS += -I${INC_SHARED_PUB_PATH}/../pmc -I${INC_SHARED_PUB_PATH}/../rdp
ifeq ($(strip $(BRCM_CHIP)),6838)
vpath %.c $(SHARED_DIR)/opensource/pmc/impl2
else
vpath %.c $(SHARED_DIR)/opensource/pmc/impl1
endif
endif

ifneq ($(findstring _$(strip $(BRCM_CHIP))_,_63381_63268_6838_63138_63148_4908_6858_68360_),)
vpath %.c $(DRIVERS_DIR)
endif

ifneq ($(strip $(INC_NAND_FLASH_DRIVER)),1)

# default BTLDR_MAX_SIZE values for SPI NOR builds
BTLDR_MAX_SIZE=196608
ifeq ($(strip $(BLD_SPI_NAND)),1)
BTLDR_MAX_SIZE=196608
endif
ifeq ($(strip $(BLD_DDR_DIAGS)),1)
BTLDR_MAX_SIZE=262144
endif

ifeq ($(strip $(BRCM_CHIP)),6858)
BTLDR_MAX_SIZE=327680
endif

# adjust BTLDR_MAX_SIZE values for SPI NOR builds that support secure boot
ifeq ($(strip $(INC_BTRM_BOOT)),1)
ifeq ($(strip $(BRCM_CHIP)),63268)
BTLDR_MAX_SIZE=131072
endif
ifneq ($(findstring _$(strip $(BRCM_CHIP))_,_63381_68360_),)
BTLDR_MAX_SIZE=196608
endif
ifneq ($(findstring _$(strip $(BRCM_CHIP))_,_63138_63148_),)
BTLDR_MAX_SIZE=393216
endif
endif

else
# default BTLDR_MAX_SIZE values for NAND builds
BTLDR_MAX_SIZE=131072

ifneq ($(findstring _$(strip $(BRCM_CHIP))_,_63138_63148_),)
BTLDR_MAX_SIZE=65536
endif

ifeq ($(strip $(INC_BTRM_BOOT)),1)
BTLDR_MAX_SIZE=131072
endif

endif

# Add SHA,AES,LZMA libs into appropriate secure boot builds
# 6838, 4908, 6858, 68360 cfe rom build supports both unsecure and secure boot within same nand build (SPI NOR built but not available)
ifneq ($(findstring _$(strip $(BRCM_CHIP))_,_6838_4908_6858_68360_),)
ifeq ($(strip $(INC_NAND_FLASH_DRIVER)),1)
BSPOBJS += $(AESOBJS) $(LZMAOBJS)
else
BSPOBJS += bcm63xx_encr_if.o
endif
BSPOBJS += $(SHAOBJS)
endif
# cfe rom build is secure boot build when the INC_BTRM_BOOT=1 option is included
# 63268 because of 3k nvram; 63381 because secure build is greater than 16k and cfe rom has to fit into 16k block 0;
# 63138 / 63148 because secure build is greater than 64k, but PMC mips (64k) and cfe rom have to fit into 128k block 0. 
ifneq ($(findstring _$(strip $(BRCM_CHIP))_,_63268_63381_63138_63148_),)
ifeq ($(strip $(INC_BTRM_BOOT)),1)
ifeq ($(strip $(INC_NAND_FLASH_DRIVER)),1)
BSPOBJS += $(SHAOBJS) $(AESOBJS) $(LZMAOBJS)
else
BSPOBJS += $(SHAOBJS) $(AESOBJS)
endif
endif
endif

BSPOBJS_ROM := $(BSPOBJS)

ifeq ($(strip $(CFG_BOOT_PSRAM)),1)
BSPOBJS += $(LZMAOBJS)
else
BSPOBJS += flashimg.o $(LZMAOBJS)
endif

export INC_CFI_FLASH_DRIVER INC_SPI_FLASH_DRIVER INC_NAND_FLASH_DRIVER INC_SPI_PROG_NAND INC_SPI_NAND_DRIVER INC_BTRM_BOOT INC_BTRM_BUILD SHARED_DIR TOOLS TOOLPREFIX TOP BLD_NAND \
	INC_PMC_DRIVER CFG_COPY_PSRAM CFG_BOOT_PSRAM NONETWORK SKIP_FLASH IKOS_NO_DDRINIT IKOS_SMPL_DDRINIT BPCM_CFE_CMD IKOS_BD_CFERAM IKOS_BD_LINUX IKOS_BD_LINUX_ROM IKOS_FAST_UART \
	IKOS_FAST_CACHE_INIT BOOT_PRE_CFE

ALL : sanity_check cfe$(BRCM_CHIP)
ifneq ($(strip $(INC_NAND_FLASH_DRIVER)),1)
	gcc  -o ${HOSTTOOLS}/addvtoken -I ${INC_SHARED_PUB_PATH} ${HOSTTOOLS}/addvtoken.c
ifeq ($(strip $(INC_BTRM_BOOT)),1)
		${HOSTTOOLS}/SecureBootUtils/makeSecureBootCfeOnly $(BRCM_CHIP) $(BUILD_DIR) $(BTLDR_MAX_SIZE);
		${HOSTTOOLS}/addvtoken --endian $(ENDIAN) --chip $(BRCM_CHIP) --flashtype NOR --btrm 1 cfe$(BRCM_CHIP).tmp bcm9$(BRCM_CHIP)_cfe.w;
		$(OBJCOPY) --output-target=srec --input-target=binary --srec-forceS3 --change-addresses=0xFFD10000 cfe$(BRCM_CHIP).tmp cfe$(BRCM_CHIP).srec;
		rm -f cfe$(BRCM_CHIP).tmp
else
	@if [ $(shell find . -name cfe$(BRCM_CHIP).bin -printf '%s\n') -le $(BTLDR_MAX_SIZE) ] ; then \
		${HOSTTOOLS}/addvtoken --endian $(ENDIAN) --chip $(BRCM_CHIP) --flashtype NOR cfe$(BRCM_CHIP).bin bcm9$(BRCM_CHIP)_cfe.w; \
	else \
		find . -name cfe$(BRCM_CHIP).bin -printf '\nERROR: cfe$(BRCM_CHIP).bin size, %s, is greater than $(BTLDR_MAX_SIZE) maximum size.\n'; \
		rm cfe$(BRCM_CHIP) cfe$(BRCM_CHIP).bin; \
	fi
endif
else
	@if [ $(shell find . -name cfe$(BRCM_CHIP)rom.bin -printf '%s\n') -gt $(BTLDR_MAX_SIZE) ] ; then \
		find . -name cfe$(BRCM_CHIP)rom.bin -printf '\nERROR: cfe$(BRCM_CHIP)rom.bin size, %s, is greater than $(BTLDR_MAX_SIZE) maximum size.\n'; \
		rm cfe$(BRCM_CHIP)rom cfe$(BRCM_CHIP)rom.bin; \
	fi
endif
	@echo done

untar_check:
	@if [ ! -e $(TOP)/api ]; then \
		echo Untarring original CFE source...; \
		(cd ../../.. && tar xfzk cfe*.tar.gz 2> /dev/null || true); \
	fi

sanity_check:
	@if [ "$(BRCM_CHIP)" = "" ]; then \
		echo You must specify chip ID using 'make BRCM_CHIP=[63268|6838|63138|60333|63381|63148|6848|4908|6858|47189|68360]'; exit 1; \
	fi
	@if [ ! -e $(TOP)/api ]; then \
		echo Untarring original CFE source...; \
		(cd ../../.. && tar xfzk cfe*.tar.gz 2> /dev/null || true); \
		$(MAKE) BRCM_CHIP=$(BRCM_CHIP) BLD_NAND=$(BLD_NAND); \
	fi

mksbp: ALL
	@echo done

sbp:
	${MAKE} clean
	${MAKE} CFG_SBP?=1 mksbp

ifeq ($(strip $(INC_NAND_FLASH_DRIVER)),1)
ifneq ($(strip $(JTAG_DOWNLOAD)),1)
# Build NAND flash boot loader
cfe$(BRCM_CHIP) cfe$(BRCM_CHIP).bin : $(CRT0OBJS) $(BSPOBJS_ROM) $(LIBCFE) noflashimg
	$(MAKE) -C $(RAM_BUILD)
	$(GLD) -o cfe$(BRCM_CHIP)rom -Map cfe$(BRCM_CHIP)rom.map $(LDFLAGS) $(CRT0OBJS) $(BSPOBJS_ROM) noflashimg.o -L. -lcfe $(LDLIBS)
	$(OBJDUMP) -d cfe$(BRCM_CHIP)rom > cfe$(BRCM_CHIP).dis
	$(OBJCOPY) --output-target=binary cfe$(BRCM_CHIP)rom cfe$(BRCM_CHIP)rom.bin
	$(OBJCOPY) --output-target=srec -R .reginfo -R .note -R .comment -R .mdebug -S cfe$(BRCM_CHIP)rom cfe$(BRCM_CHIP).srec
# build srec for ikos/sim test
ifeq ($(strip $(BRCM_IKOS)),1)
ifneq ($(findstring _$(strip $(BRCM_CHIP))_,_63138_63148_4908_6858_68360_),)
	$(OBJCOPY) --output-target=srec --input-target=binary --srec-forceS3 --change-addresses=0xFFE10000 cfe$(BRCM_CHIP)rom.bin cfe$(BRCM_CHIP)-ikos.srec;
else
	$(OBJCOPY) --output-target=srec --input-target=binary --srec-forceS3 --change-addresses=0xB8000000 cfe$(BRCM_CHIP)rom.bin cfe$(BRCM_CHIP)-ikos.srec;
endif
endif
	rm noflashimg.*

noflashimg:
ifeq ($(strip $(INC_BTRM_BOOT)),1)
	echo "char *_binArrayStart = (char *) 0, *_binArrayEnd = (char *) 0;" > $@.c;
else
ifneq ($(findstring _$(strip $(BRCM_CHIP))_,_6838_4908_6858_68360_),)
	echo "char *_binArrayStart = (char *) 0, *_binArrayEnd = (char *) 0;" > $@.c;
else
	echo "char *_binArrayStart = (char *) 0, *_binArrayEnd = (char *) 0;" > $@.c; \
	echo "int decompressLZMA(unsigned char *s, unsigned int sl, unsigned char *d, unsigned int dl);" >> $@.c; \
	echo "int decompressLZMA(unsigned char *s, unsigned int sl, unsigned char *d, unsigned int dl){return 0;}" >> $@.c
endif
endif
	$(GCC) $(CFLAGS) -o $@.o $@.c
endif

else
# Build NOR flash boot loader
cfe$(BRCM_CHIP) cfe$(BRCM_CHIP).bin : $(CRT0OBJS) $(BSPOBJS) $(LIBCFE) $(ASM_FILE)
	$(GLD) -o cfe$(BRCM_CHIP) -Map cfe$(BRCM_CHIP).map $(LDFLAGS) $(CRT0OBJS) $(BSPOBJS) -L. -lcfe $(LDLIBS)
	$(OBJDUMP) -d cfe$(BRCM_CHIP) > cfe$(BRCM_CHIP).dis
	$(OBJCOPY) --output-target=binary -R .reginfo -R .note -R .comment -R .mdebug -S cfe$(BRCM_CHIP) cfe$(BRCM_CHIP).bin
	$(OBJCOPY) --output-target=srec   -R .reginfo -R .note -R .comment -R .mdebug -S cfe$(BRCM_CHIP) cfe$(BRCM_CHIP).srec
# build srec for ikos/sim test
ifeq ($(strip $(BRCM_IKOS)),1)
ifneq ($(findstring _$(strip $(BRCM_CHIP))_,_63138_63148_4908_6858_68360_),)
	$(OBJCOPY) --output-target=srec --input-target=binary --srec-forceS3 --change-addresses=0xFFD10000 cfe$(BRCM_CHIP).bin cfe$(BRCM_CHIP)-ikos.srec;
else
	$(OBJCOPY) --output-target=srec --input-target=binary --srec-forceS3 --change-addresses=0xB8000000 cfe$(BRCM_CHIP).bin cfe$(BRCM_CHIP)-ikos.srec;
endif
endif

$(ASM_FILE) : $(TOP)/board/$(BOARD)_ram/src 
	rm -f $(ASM_FILE)
	$(MAKE) -C $(RAM_BUILD)
endif

ifeq ($(strip $(JTAG_DOWNLOAD)),1)
ifeq ($(strip $(INC_NAND_FLASH_DRIVER)),1)
# Build NOR like NAND flash boot loader for Fast JTAG only.
cfe$(BRCM_CHIP) cfe$(BRCM_CHIP).bin : $(CRT0OBJS) $(BSPOBJS) $(LIBCFE) $(ASM_FILE)
	$(GLD) -o cfe$(BRCM_CHIP) -Map cfe$(BRCM_CHIP).map $(LDFLAGS) $(CRT0OBJS) $(BSPOBJS) -L. -lcfe $(LDLIBS)
	$(OBJDUMP) -d cfe$(BRCM_CHIP) > cfe$(BRCM_CHIP).dis
	$(OBJCOPY) --output-target=binary cfe$(BRCM_CHIP) cfe$(BRCM_CHIP).bin

$(ASM_FILE) : $(TOP)/arch/mips/board/$(BOARD)_ram/src 
	rm -f $(ASM_FILE)
	$(MAKE) -C $(RAM_BUILD)
endif
endif

clean : untar_check
	rm -f *.o *~ cfe* *.w *.srec
	rm -f build_date.c
	rm -f noflashimg.c >& /dev/null
	rm -f libcfe.a
	$(MAKE) -C $(RAM_BUILD) clean

distclean : clean

release_mips :
	$(MAKE) sanity_toolchain BRCM_CHIP=63268
	make clean
	$(MAKE) BRCM_CHIP=63268
	cp -f cfe*.bin $(TARGETS_DIR)/cfe
	$(MAKE) clean
	$(MAKE) BRCM_CHIP=63268 BLD_NAND=1
	cp -f cfe*.bin $(TARGETS_DIR)/cfe
	cp -f $(RAM_BUILD)/cfe*ram.bin $(TARGETS_DIR)/cfe
	$(MAKE) clean
	$(MAKE) BRCM_CHIP=63268 BLD_BTRM_BOOT=1
	cp -f cfe63268.bin $(TARGETS_DIR)/cfe/cfesec63268.bin
	$(MAKE) clean
	$(MAKE) BRCM_CHIP=63268 BLD_NAND=1 BLD_BTRM_BOOT=1
	cp -f cfe63268rom.bin $(TARGETS_DIR)/cfe/cfesec63268rom.bin
	cp -f $(RAM_BUILD)/cfe63268ram.bin $(TARGETS_DIR)/cfe/cfesec63268ram.bin
	$(MAKE) clean
	$(MAKE) BRCM_CHIP=6838
	cp -f cfe*.bin $(TARGETS_DIR)/cfe
	$(MAKE) clean
	$(MAKE) BRCM_CHIP=6838 BLD_NAND=1
	cp -f cfe*.bin $(TARGETS_DIR)/cfe
	cp -f $(RAM_BUILD)/cfe*ram.bin $(TARGETS_DIR)/cfe 
	$(MAKE) clean
	$(MAKE) BRCM_CHIP=63381
	cp -f cfe*.bin $(TARGETS_DIR)/cfe
	$(MAKE) clean
	$(MAKE) BRCM_CHIP=63381 BLD_NAND=1
	cp -f cfe*.bin $(TARGETS_DIR)/cfe
	cp -f $(RAM_BUILD)/cfe*ram.bin $(TARGETS_DIR)/cfe 
	$(MAKE) clean
	$(MAKE) BRCM_CHIP=63381 BLD_BTRM_BOOT=1
	cp -f cfe63381.bin $(TARGETS_DIR)/cfe/cfesec63381.bin
	$(MAKE) clean
	$(MAKE) BRCM_CHIP=63381 BLD_NAND=1 BLD_BTRM_BOOT=1
	cp -f cfe63381rom.bin $(TARGETS_DIR)/cfe/cfesec63381rom.bin
	cp -f $(RAM_BUILD)/cfe63381ram.bin $(TARGETS_DIR)/cfe/cfesec63381ram.bin
	$(MAKE) clean
	$(MAKE) BRCM_CHIP=6848
	cp -f cfe*.bin $(TARGETS_DIR)/cfe
	$(MAKE) clean
	$(MAKE) BRCM_CHIP=6848 BLD_NAND=1
	cp -f cfe*.bin $(TARGETS_DIR)/cfe
	cp -f $(RAM_BUILD)/cfe*ram.bin $(TARGETS_DIR)/cfe
	$(MAKE) clean


release_arm :
	$(MAKE) clean
	$(MAKE) BRCM_CHIP=4908
	cp -f cfe*.bin $(TARGETS_DIR)/cfe
	$(MAKE) clean
	$(MAKE) BRCM_CHIP=4908 BLD_NAND=1
	cp -f cfe*.bin $(TARGETS_DIR)/cfe
	cp -f $(RAM_BUILD)/cfe*ram.bin $(TARGETS_DIR)/cfe

release_arm_all :
	$(MAKE) sanity_toolchain BRCM_CHIP=63138
	$(MAKE) clean
	$(MAKE) BRCM_CHIP=63138
	cp -f cfe*.bin $(TARGETS_DIR)/cfe
	$(MAKE) clean
	$(MAKE) BRCM_CHIP=63138 BLD_NAND=1
	cp -f cfe*.bin $(TARGETS_DIR)/cfe
	cp -f $(RAM_BUILD)/cfe*ram.bin $(TARGETS_DIR)/cfe
	$(MAKE) clean
	$(MAKE)  BRCM_CHIP=63138 BLD_SPI_NAND=1
	cp -f *cfe.w $(TARGETS_DIR)/cfe/bcm63138_spi_prog_nand_cfe.w
	$(MAKE) clean
	$(MAKE) BRCM_CHIP=63138 BLD_BTRM_BOOT=1
	cp -f cfe63138.bin $(TARGETS_DIR)/cfe/cfesec63138.bin
	$(MAKE) clean
	$(MAKE) BRCM_CHIP=63138 BLD_NAND=1 BLD_BTRM_BOOT=1
	cp -f cfe63138rom.bin $(TARGETS_DIR)/cfe/cfesec63138rom.bin
	cp -f $(RAM_BUILD)/cfe63138ram.bin $(TARGETS_DIR)/cfe/cfesec63138ram.bin
	$(MAKE) clean
	$(MAKE) BRCM_CHIP=63148
	cp -f cfe*.bin $(TARGETS_DIR)/cfe
	$(MAKE) clean
	$(MAKE) BRCM_CHIP=63148 BLD_NAND=1
	cp -f cfe*.bin $(TARGETS_DIR)/cfe
	cp -f $(RAM_BUILD)/cfe*ram.bin $(TARGETS_DIR)/cfe
	$(MAKE) clean
	$(MAKE) BRCM_CHIP=63148 BLD_BTRM_BOOT=1
	cp -f cfe63148.bin $(TARGETS_DIR)/cfe/cfesec63148.bin
	$(MAKE) clean
	$(MAKE) BRCM_CHIP=63148 BLD_NAND=1 BLD_BTRM_BOOT=1
	cp -f cfe63148rom.bin $(TARGETS_DIR)/cfe/cfesec63148rom.bin
	cp -f $(RAM_BUILD)/cfe63148ram.bin $(TARGETS_DIR)/cfe/cfesec63148ram.bin
	$(MAKE) clean
	$(MAKE) BRCM_CHIP=4908
	cp -f cfe*.bin $(TARGETS_DIR)/cfe
	$(MAKE) clean
	$(MAKE) BRCM_CHIP=4908 BLD_NAND=1
	cp -f cfe*.bin $(TARGETS_DIR)/cfe
	cp -f $(RAM_BUILD)/cfe*ram.bin $(TARGETS_DIR)/cfe
	$(MAKE) clean
	$(MAKE) BRCM_CHIP=6858
	cp -f cfe*.bin $(TARGETS_DIR)/cfe
	$(MAKE) clean
	$(MAKE) BRCM_CHIP=6858 BLD_NAND=1
	cp -f cfe*.bin $(TARGETS_DIR)/cfe
	cp -f $(RAM_BUILD)/cfe*ram.bin $(TARGETS_DIR)/cfe
	$(MAKE) clean
	$(MAKE) BRCM_CHIP=47189
	cp -f cfe*.bin $(TARGETS_DIR)/cfe
	$(MAKE) clean
	$(MAKE) BRCM_CHIP=47189 BLD_NAND=1 
	cp -f cfe47189rom.bin $(TARGETS_DIR)/cfe/cfe47189rom.bin
	cp -f $(RAM_BUILD)/cfe47189ram.bin $(TARGETS_DIR)/cfe/cfe47189ram.bin
	$(MAKE) clean
	$(MAKE) BRCM_CHIP=68360 NONETWORK=1
	cp -f cfe*.bin $(TARGETS_DIR)/cfe
	$(MAKE) clean
	$(MAKE) BRCM_CHIP=68360 BLD_NAND=1 NONETWORK=1
	cp -f cfe*.bin $(TARGETS_DIR)/cfe
	cp -f $(RAM_BUILD)/cfe*ram.bin $(TARGETS_DIR)/cfe
	$(MAKE) clean
release_mipsel :
	$(MAKE) sanity_toolchain BRCM_CHIP=60333
	$(MAKE) clean
	$(MAKE) BRCM_CHIP=60333
	cp -f cfe*.bin $(TARGETS_DIR)/cfe
	$(MAKE) clean

sanity_toolchain :
ifeq ($(wildcard $(TOOLS)),)
	@echo Missing $(TOOLPREFIX) toolchain. Please install it to build for $(BRCM_CHIP).; exit 1;
endif

release : untar_check
ifneq ($(wildcard $(TOP)/../../release/INTERNAL_BUILD),)
	$(MAKE) release_mipsel
	$(MAKE) release_arm
	$(MAKE) release_mips
else
	-$(MAKE) release_mipsel
	-$(MAKE) release_arm
	-$(MAKE) release_mips
endif

JTAG_FAST_DIR = jtag_fast
NORMAL_DIR = normal
SPI_PROG_NAND_DIR = spi_prog_nand

mk_dir:
	rm -fr $(JTAG_FAST_DIR)
	rm -fr $(NORMAL_DIR)
	rm -fr $(SPI_PROG_NAND_DIR)
	mkdir $(JTAG_FAST_DIR)
	mkdir $(NORMAL_DIR)
	mkdir $(SPI_PROG_NAND_DIR)

# Makes cfe binaries and jtag files for all chips.
jtag : untar_check mk_dir
	$(MAKE) clean
	$(MAKE) BRCM_CHIP=63268
	cp -f $(RAM_BUILD)/cfe63268 $(NORMAL_DIR)/cfe63268_NOR.elf
	cp -f bcm963268_cfe.w $(NORMAL_DIR)/bcm963268_cfe_NOR.w	
	$(MAKE) clean
	$(MAKE) BRCM_CHIP=63268 BLD_NAND=1
	cp -f $(RAM_BUILD)/cfe63268  $(NORMAL_DIR)/cfe63268_NAND.elf
	$(MAKE) clean
	$(MAKE) BRCM_CHIP=63268 BLD_SPI_NAND=1
	cp -f bcm963268_cfe.w  $(NORMAL_DIR)/bcm963268_SPI_PROG_NAND.w
	cp -f cfe63268.bin  $(NORMAL_DIR)/bcm963268_SPI_PROG_NAND.bin
	$(MAKE) clean
	$(MAKE) BRCM_CHIP=63268 JTAG_DOWNLOAD=1
	cp -f cfe63268 $(JTAG_FAST_DIR)/cfe63268_NOR_Fast.elf
	$(MAKE) clean	
	$(MAKE) BRCM_CHIP=63268 JTAG_DOWNLOAD=1 BLD_NAND=1
	cp -f cfe63268 $(JTAG_FAST_DIR)/cfe63268_NAND_Fast.elf
	$(MAKE) clean	
	$(MAKE) BRCM_CHIP=6838
	cp -f $(RAM_BUILD)/cfe6838 $(NORMAL_DIR)/cfe6838_NOR.elf
	cp -f bcm96838_cfe.w $(NORMAL_DIR)/bcm96838_cfe_NOR.w
	$(MAKE) clean
	$(MAKE) BRCM_CHIP=6838 BLD_NAND=1
	cp -f $(RAM_BUILD)/cfe6838 $(NORMAL_DIR)/cfe6838_NAND.elf
	$(MAKE) clean
	$(MAKE) BRCM_CHIP=6838 CFG_BOOT_PSRAM=1
	cp -f cfe6838 $(JTAG_FAST_DIR)/cfe6838_NOR_Fast.elf
	$(MAKE) clean	
	$(MAKE) BRCM_CHIP=6838 CFG_BOOT_PSRAM=1 BLD_NAND=1
	cp -f cfe6838rom $(JTAG_FAST_DIR)/cfe6838_NAND_Fast.elf
	$(MAKE) clean
	$(MAKE) BRCM_CHIP=63138
	cp -f $(RAM_BUILD)/cfe63138 $(NORMAL_DIR)/cfe63138_NOR.elf
	cp -f bcm963138_cfe.w $(NORMAL_DIR)/bcm963138_cfe_NOR.w	
	$(MAKE) clean
	$(MAKE) BRCM_CHIP=63138 BLD_NAND=1
	cp -f $(RAM_BUILD)/cfe63138  $(NORMAL_DIR)/cfe63138_NAND.elf
	$(MAKE) clean
	$(MAKE) BRCM_CHIP=63138 BLD_SPI_NAND=1
	cp -f bcm963138_cfe.w  $(NORMAL_DIR)/bcm963138_SPI_PROG_NAND.w
	cp -f cfe63138.bin  $(NORMAL_DIR)/bcm963138_SPI_PROG_NAND.bin
	$(MAKE) clean
	$(MAKE) BRCM_CHIP=63381
	cp -f $(RAM_BUILD)/cfe63381 $(NORMAL_DIR)/cfe63381_NOR.elf
	cp -f bcm963381_cfe.w $(NORMAL_DIR)/bcm963381_cfe_NOR.w	
	$(MAKE) clean
	$(MAKE) BRCM_CHIP=63381 BLD_NAND=1
	cp -f $(RAM_BUILD)/cfe63381 $(NORMAL_DIR)/cfe63381_NAND.elf
	$(MAKE) clean
	$(MAKE) BRCM_CHIP=63381 BLD_SPI_NAND=1
	cp -f bcm963381_cfe.w  $(NORMAL_DIR)/bcm963381_SPI_PROG_NAND.w
	cp -f cfe63381.bin  $(NORMAL_DIR)/bcm963381_SPI_PROG_NAND.bin
	$(MAKE) clean
	$(MAKE) BRCM_CHIP=63148
	cp -f $(RAM_BUILD)/cfe63148 $(NORMAL_DIR)/cfe63148_NOR.elf
	cp -f bcm963148_cfe.w $(NORMAL_DIR)/bcm963148_cfe_NOR.w	
	$(MAKE) clean
	$(MAKE) BRCM_CHIP=63148 BLD_NAND=1
	cp -f $(RAM_BUILD)/cfe63148  $(NORMAL_DIR)/cfe63148_NAND.elf
	$(MAKE) clean
	$(MAKE) BRCM_CHIP=63148 BLD_SPI_NAND=1
	cp -f bcm963148_cfe.w  $(NORMAL_DIR)/bcm963148_SPI_PROG_NAND.w
	cp -f cfe63148.bin  $(NORMAL_DIR)/bcm963148_SPI_PROG_NAND.bin
	$(MAKE) clean
	$(MAKE) BRCM_CHIP=6848 CFG_BOOT_PSRAM=1
	cp -f cfe6848 $(JTAG_FAST_DIR)/cfe6848_NOR_Fast.elf
	$(MAKE) clean
	$(MAKE) BRCM_CHIP=6848 CFG_BOOT_PSRAM=1 BLD_NAND=1
	cp -f cfe6848rom $(JTAG_FAST_DIR)/cfe6848_NAND_Fast.elf
	$(MAKE) clean
	$(MAKE) BRCM_CHIP=6858  CFE_ROM_STOP=1
	cp -f cfe6858 $(JTAG_FAST_DIR)/cfe6858_Fast.elf
	$(MAKE) clean
	$(MAKE)  BRCM_CHIP=63138 BLD_SPI_NAND=1
	cp -f *cfe.w $(SPI_PROG_NAND_DIR)/bcm63138_spi_prog_nand_cfe.w
	$(MAKE) clean
	$(MAKE)  BRCM_CHIP=63148 BLD_SPI_NAND=1
	cp -f *cfe.w $(SPI_PROG_NAND_DIR)/bcm63148_spi_prog_nand_cfe.w
	$(MAKE) clean
	$(MAKE)  BRCM_CHIP=63268 BLD_SPI_NAND=1
	cp -f *cfe.w $(SPI_PROG_NAND_DIR)/bcm63268_spi_prog_nand_cfe.w
	$(MAKE) clean

CFE_RAM_START_ADDR := $(OBJDUMP) -f $(RAM_BUILD)/cfe$(BRCM_CHIP) | grep start | cut -d'x' -f2
VMLINUX_START_ADDR := $(OBJDUMP) -f $(LINUX_KERNEL_DIR)/vmlinux | grep start | cut -d'x' -f2

# Must be called from base Linux build directory.
ikos_finish:
	@xxd -g4 cfe$(BRCM_CHIP) | sed -e "s/694b6f31/$(shell $(CFE_RAM_START_ADDR))/" | xxd -r > cfe$(BRCM_CHIP).tmp
	@mv cfe$(BRCM_CHIP).tmp cfe$(BRCM_CHIP)
	@xxd -g4 $(RAM_BUILD)/cfe$(BRCM_CHIP) | sed -e "s/694b6f32/$(shell $(VMLINUX_START_ADDR))/" | xxd -r > $(RAM_BUILD)/cfe$(BRCM_CHIP).tmp
	@mv $(RAM_BUILD)/cfe$(BRCM_CHIP).tmp $(RAM_BUILD)/cfe$(BRCM_CHIP)
	$(OBJCOPY) --output-target=binary cfe$(BRCM_CHIP) cfe$(BRCM_CHIP).bin
	$(OBJCOPY) --output-target=srec $(RAM_BUILD)/cfe$(BRCM_CHIP) cferam$(BRCM_CHIP).srec

default_nvm:
ifneq ($(BUILD_NAME),)
	@nvserial -a -s 100 -o $(DFT_NVRAM_NAME)_nvram.nvm $(DFT_NVRAM_NAME)_nvram.txt
endif

default_nvm-clean:
	@rm -f *.nvm
