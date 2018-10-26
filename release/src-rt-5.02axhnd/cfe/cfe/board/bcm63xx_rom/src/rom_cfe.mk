
#
# CFE's version number
#

include ${TOP}/main/cfe_version.mk

#
# Default values for certain parameters
#

CFG_MLONG64 ?= 0
CFG_LITTLE  ?= 0
CFG_RELOC ?= 0
CFG_UNCACHED ?= 0
CFG_NEWRELOC ?= 0
CFG_BOOTRAM ?= 0
CFG_VGACONSOLE ?= 0
CFG_PCI ?= 1
CFG_LDT_REV_017 ?= 0
CFG_ZLIB ?= 0
CFG_BIENDIAN ?= 0
CFG_DOWNLOAD ?= 0
CFG_RAMAPP ?= 0
CFG_USB ?= 0
CFG_COPY_PSRAM ?=0		# copy cfe-rom into PSRAM and execute.
CFG_BOOT_PSRAM ?=0		# boot cfe-rom from PSRAM

#
# Paths to other parts of the firmware.  Everything's relative to ${TOP}
# so that you can actually do a build anywhere you want.
#

ARCH_TOP   = ${TOP}/arch/${ARCH}
ARCH_SRC   = ${ARCH_TOP}/common/src
ARCH_INC   = ${ARCH_TOP}/common/include
CPU_SRC    = ${ARCH_TOP}/cpu/${CPU}/src
CPU_INC    = ${ARCH_TOP}/cpu/${CPU}/include
#
# It's actually optional to have a 'board'
# directory.  If you don't specify BOARD,
# don't include the files.
#

ifneq ("$(strip ${BOARD})","")
BOARD_SRC       = ${TOP}/board/${BOARD}_rom/src
BOARD_INC       = ${TOP}/board/${BOARD}_rom/include
BOARD_INC       += ${TOP}/board/common/include
BOARD_ARCH_SRC  = ${ARCH_TOP}/board/${BOARD}_rom/src
BOARD_ARCH_INC  = ${ARCH_TOP}/board/${BOARD}_rom/include
BOARD_ARCH_SHR_SRC  = ${ARCH_TOP}/board/${BOARD}_shared/src
BOARD_ARCH_SHR_INC  = ${ARCH_TOP}/board/${BOARD}_shared/include
endif

#
# Preprocessor defines for CFE's version number
#

VDEF = -DCFE_VER_MAJ=${CFE_VER_MAJ} -DCFE_VER_MIN=${CFE_VER_MIN} -DCFE_VER_ECO=${CFE_VER_ECO}

#
# Construct the list of paths that will eventually become the include
# paths and VPATH
#
SRCDIRS = ${ARCH_SRC} ${CPU_SRC} ${BOARD_SRC} $(BOARD_ARCH_SRC) $(BOARD_ARCH_SHR_SRC) ${TOP}/main ${TOP}/vendor ${TOP}/include ${TOP}/net ${TOP}/dev ${TOP}/pci ${TOP}/ui ${TOP}/lib ${TOP}/common ${TOP}/verif ${TOP}/lzma ${TOP}/board/common/src
CFE_INC = ${TOP}/include ${TOP}/pci ${TOP}/net 

ifeq ($(strip ${CFG_VGACONSOLE}),1)
SRCDIRS += ${TOP}/x86emu ${TOP}/pccons
CFE_INC += ${TOP}/x86emu ${TOP}/pccons
endif

ifeq ($(strip ${CFG_VAPI}),1)
SRCDIRS += ${TOP}/verif
CFE_INC += ${TOP}/verif
endif

ifeq ($(strip ${CFG_ZLIB}),1)
SRCDIRS += ${TOP}/zlib
CFE_INC += ${TOP}/zlib
endif


INCDIRS = $(patsubst %,-I%,$(subst :, ,$(ARCH_INC) $(CPU_INC) $(BOARD_INC) $(BOARD_ARCH_INC) $(BOARD_ARCH_SHR_INC) $(CFE_INC)))

VPATH = $(SRCDIRS)

#
# Bi-endian support: If we're building the little-endian
# version, use a different linker script so we can locate the
# ROM at a higher address.  You'd think we could do this with
# normal linker command line switches, but there appears to be no
# command-line way to override the 'AT' qualifier in the linker script.
#

CFG_TEXTAT1MB=0
ifeq ($(strip ${CFG_BIENDIAN}),1)
  ifeq ($(strip ${CFG_LITTLE}),1)
    CFG_TEXTAT1MB=1
  endif
endif

INC_PMC_DRIVER=0

ifneq ($(findstring _$(strip $(BRCM_CHIP))_,_6838_6848_),)
INC_PMC_DRIVER=1
ifneq ($(strip $(CFG_BOOT_PSRAM)),1)
CFG_COPY_PSRAM=1
endif
endif

# meaning if ((BRCM_CHIP != 60333) && (BRCM_CHIP != 63268) && (BRCM_CHIP != 6838) && (BRCM_CHIP != 6848) && (BRCM_CHIP == 47189))
ifeq ($(findstring _$(strip $(BRCM_CHIP))_,_60333_63268_6838_6848_47189_),) 
INC_PMC_DRIVER=1
endif

#
# Add some common flags that are used on any architecture.
#

CFLAGS += -I. $(INCDIRS)
CFLAGS += -D_CFE_ ${VDEF} -DCFG_BOARDNAME=\"${CFG_BOARDNAME}\"

include ../../../../version.make
CFLAGS += -DBRCM_VERSION="\"$(BRCM_VERSION)\"" -DBRCM_RELEASE="\"$(BRCM_RELEASE)\"" -DBRCM_EXTRAVERSION="\"$(BRCM_EXTRAVERSION)\""
CFLAGS += -DCFE_VER_MAJ_STR="\"$(CFE_VER_MAJ)\"" -DCFE_VER_MIN_STR="\"$(CFE_VER_MIN)\"" -DCFE_VER_ECO_STR="\"$(CFE_VER_ECO)\""
CFE_MAJOR=$(shell cat ../../../cfe/include/cfe.h| \grep BCM63XX_MAJOR | awk '{print $$3 }')
CFE_MINOR=$(shell cat ../../../cfe/include/cfe.h| \grep BCM63XX_MINOR | awk '{print $$3 }')
CFLAGS += -DCFE_MAJOR_STR="\"$(CFE_MAJOR)\"" -DCFE_MINOR_STR="\"$(CFE_MINOR)\""

#
# Gross - allow more options to be supplied from command line
#

ifdef CFG_OPTIONS
OPTFLAGS = $(patsubst %,-D%,$(subst :, ,$(CFG_OPTIONS)))
CFLAGS += ${OPTFLAGS}
endif

CFLAGS += -fdata-sections -ffunction-sections
LDFLAGS += --gc-sections

#
# Add flash driver support.
#

INC_CFI_FLASH_DRIVER=0
INC_SPI_FLASH_DRIVER=0
INC_NAND_FLASH_DRIVER=0
INC_EMMC_FLASH_DRIVER=0
INC_SPI_PROG_NAND=0
INC_SPI_NAND_DRIVER=0
INC_EMMC_FLASH_DRIVER=0
INC_MEMSYS_INIT=0
INC_NVRAM_MIRRORING=0
INC_SPLIT_NOR_SECONDARY_STAGE=0

ifneq ($(findstring _$(strip $(BRCM_CHIP))_,_4908_6858_63158_6846_6856_),)
INC_NVRAM_MIRRORING=1
endif

ifeq ($(strip $(BLD_EMMC)),1)
# BUILD EMMC flash boot loader
# supported chips are 63138, 4908, 6858, 63158, 6856
ifneq ($(findstring _$(strip $(BRCM_CHIP))_,_63138_4908_6858_63158_6856_),)
INC_EMMC_FLASH_DRIVER=1
endif
endif

ifeq ($(strip $(BLD_NAND)),1)
# BUILD NAND flash boot loader
# all chip supports nand except 60333
ifeq ($(findstring _$(strip $(BRCM_CHIP))_,_60333_),)
INC_NAND_FLASH_DRIVER=1
ifeq ($(findstring _$(strip $(BRCM_CHIP))_,_63268_63148_47189_),)
INC_SPI_NAND_DRIVER=1
endif
endif
endif

ifeq ($(strip $(BLD_EMMC)),)
ifeq ($(strip $(BLD_NAND)),)
# BUILD NOR flash boot loader
INC_SPI_FLASH_DRIVER=1
# By default, NOR images contain cferam as a binary blob denoted with
# _binArrayStart. But this image needs to fit in 128KB on 4908, which is not
# enough for the ever expanding cferam (and u-boot, in some cases). So we now
# introduce another option to build cferom and cferam as two separate binary
# images.
ifeq ($(strip $(BLD_SPLIT_NOR_SECONDARY_STAGE)),1)
INC_SPLIT_NOR_SECONDARY_STAGE=1
endif
endif
endif

ifeq ($(strip $(BLD_SPI_NAND)),1)
INC_SPI_FLASH_DRIVER=1
INC_SPI_PROG_NAND=1
endif

ifneq ($(findstring _$(strip $(BRCM_CHIP))_,_63138_63148_4908_6858_63158_6846_6856_),)
INC_MEMSYS_INIT=1
endif

ifeq ($(strip $(BRCM_IKOS)),1)
INC_MEMSYS_INIT=0
endif

# If the BLD_BTRM_BOOT arg is defined and set to 1,
# the 63268 CFE ROM and CFE RAM should be built such that
# they are the 2nd and 3rd level bootloaders, respectively.
# If it is not defined, or set to 0, the CFE ROM and CFE RAM
# are the 1st and 2nd level bootloaders, respectively
# This flag is only relavent for the 63268!!
ifeq ($(strip $(BLD_BTRM_BOOT)),1)
INC_BTRM_BOOT=1
ifneq ($(strip ${INC_BTRM_DEVEL}),)
CFLAGS+=-D_BTRM_DEVEL_=$(INC_BTRM_DEVEL)
endif
else
INC_BTRM_BOOT=0
endif

ifeq ($(strip $(BLD_SEC_TK)),1)
include ${TOP}/otp/Makefile
CFE_INC += ${TOP}/otp
SRCDIRS += ${TOP}/otp
CFLAGS += -DSEC_TK=$(BLD_SEC_TK)
endif
# This mk file is used only for building the CFE ROM, therefore any
# BTRM-specific code within shared files (such as init_mips.S) should not 
# be included within the build 
INC_BTRM_BUILD=0

# Add a 2 second delay during initial boot for jtag to break in.
ifeq ($(strip $(JTAG_DELAY)),1)
CFLAGS += -DJTAG_DELAY

# Make checks for a GPIO based infinite loop.
ifneq ("$(strip ${GPIO_PIN})","")
CFLAGS += -DGPIO_PIN=$(GPIO_PIN)
endif
ifneq ("$(strip ${GPIO_VALUE})","")
CFLAGS += -DGPIO_VALUE=$(GPIO_VALUE)
endif
endif

# Make the downloads from JTAG CFE faster(closed to flashed cfe).
ifneq ("$(strip ${JTAG_DOWNLOAD})","")
CFLAGS += -DJTAG_DOWNLOAD=1
endif

# Enable cfe abort key for all devices except 47189
ifeq ($(findstring _$(strip $(BRCM_CHIP))_,_47189_),)
ifneq ($(strip $(BOOT_PRE_CFE)),1)
CFLAGS += -DCFE_ABORT_KEY=1
endif
endif

ifneq ("$(strip ${CFE_ROM_STOP})","")
CFLAGS += -DCFE_ROM_STOP
endif

ifeq ($(strip $(BLD_DDR_DIAGS)),1)
CFLAGS += -DINC_DDR_DIAGS=1
endif

CFLAGS += -DINC_BTRM_BOOT=$(INC_BTRM_BOOT)
CFLAGS += -DINC_BTRM_BUILD=$(INC_BTRM_BUILD)
CFLAGS += -DINC_CFI_FLASH_DRIVER=$(INC_CFI_FLASH_DRIVER)
CFLAGS += -DINC_SPI_FLASH_DRIVER=$(INC_SPI_FLASH_DRIVER)
CFLAGS += -DINC_NAND_FLASH_DRIVER=$(INC_NAND_FLASH_DRIVER)
CFLAGS += -DINC_EMMC_FLASH_DRIVER=$(INC_EMMC_FLASH_DRIVER)
CFLAGS += -DINC_SPI_PROG_NAND=$(INC_SPI_PROG_NAND)
CFLAGS += -DINC_SPI_NAND_DRIVER=$(INC_SPI_NAND_DRIVER)
CFLAGS += -DINC_PMC_DRIVER=$(INC_PMC_DRIVER)
CFLAGS += -DINC_MEMSYS_INIT=$(INC_MEMSYS_INIT)
CFLAGS += -DCFG_COPY_PSRAM=$(CFG_COPY_PSRAM)
CFLAGS += -DCFG_BOOT_PSRAM=$(CFG_BOOT_PSRAM)
CFLAGS += -DCFG_ROM_PRINTF=$(CFG_ROM_PRINTF)
CFLAGS += -DBOOT_PRE_CFE=$(BOOT_PRE_CFE)
CFLAGS += -DINC_NVRAM_MIRRORING=$(INC_NVRAM_MIRRORING)
CFLAGS += -DINC_SPLIT_NOR_SECONDARY_STAGE=$(INC_SPLIT_NOR_SECONDARY_STAGE)

#
# Set IKOS/Sim options
#
ifeq ($(strip $(BRCM_IKOS)),1)
CFLAGS += -DCONFIG_BRCM_IKOS
endif
ifeq ($(strip $(IKOS_NO_DDRINIT)),1)
CFLAGS += -DIKOS_NO_DDRINIT
endif
ifeq ($(strip $(IKOS_SMPL_DDRINIT)),1)
CFLAGS += -DIKOS_SMPL_DDRINIT
endif
ifeq ($(strip $(IKOS_BD_CFERAM)),1)
CFLAGS += -DIKOS_BD_CFERAM
endif
ifeq ($(strip $(IKOS_BD_LINUX)),1)
CFLAGS += -DIKOS_BD_LINUX
endif
ifeq ($(strip $(IKOS_BD_LINUX_ROM)),1)
CFLAGS += -DIKOS_BD_LINUX_ROM
endif
ifeq ($(strip $(IKOS_FAST_UART)),1)
CFLAGS += -DIKOS_FAST_UART
endif
ifeq ($(strip $(IKOS_FAST_CACHE_INIT)),1)
CFLAGS += -DIKOS_FAST_CACHE_INIT
endif
#
# This is the makefile's main target.  Note that we actually
# do most of the work in 'ALL' not 'all', since we include
# other makefiles after this point.
#

all : ALL

#
# Macros that expand to the list of arch-independent files
#

SHAOBJS = bcm63xx_rsa.o bcm63xx_sha.o bcm63xx_auth_if.o
AESOBJS = bcm63xx_aes.o bcm63xx_encr_if.o
LZMAOBJS = LzmaDecode.o dcapi.o
DEVOBJS =
## dev_newflash.o dev_null.o dev_promice.o dev_ide_common.o dev_ns16550.o dev_ds17887clock.o dev_flash.o
LIBOBJS = lib_string.o lib_crc.o
ifeq ($(strip $(CFG_ROM_PRINTF)),1)
LIBOBJS += lib_printf.o
endif
##lib_hssubr.o lib_physio.o lib_printf.o lib_misc.o lib_arena.o lib_queue.o 
##		  lib_qsort.o  lib_string2.o

NETOBJS = 
##	    net_tcp.o net_tcpbuf.o dev_tcpconsole.o  net_dhcp.o net_icmp.o net_ether.o net_tftp.o net_ip.o net_udp.o  net_dns.o net_arp.o net_api.o 

CFEOBJS = 
## cfe_iocb_dispatch.o cfe_devfuncs.o \ cfe_console.o cfe_timer.o cfe_attach.o cfe_background.o cfe_zlibfs.o 
##            cfe_mem.o  
##           cfe_error.o build_date.o \
##	  cfe_rawfs.o cfe_xreq.o cfe_filesys.o 
##	  cfe_fatfs.o cfe_httpfs.o  cfe_ldr_srec.o  cfe_autoboot.o cfe_boot.o cfe_ldr_elf.o cfe_ldr_raw.o  cfe_loader.o
##    cfe_main.o nvram_subr.o url.o cfe_savedata.o env_subr.o
UIOBJS  = 
##ui_command.o ui_cmddisp.o 
##	  ui_pcicmds.o \ui_tcpcmds.o ui_memcmds.o ui_loadcmds.o ui_flash.o ui_netcmds.o ui_envcmds.o ui_devcmds.o
##	  ui_examcmds.o ui_misccmds.o \
## ui_test_disk.o ui_test_ether.o ui_test_flash.o ui_test_uart.o

#
# Add more object files if we're supporting PCI
#

ifeq ($(strip ${CFG_PCI}),1)
PCIOBJS  = pciconf.o ldtinit.o pci_subr.o
PCIOBJS += pci_devs.o
DEVOBJS += dev_sp1011.o dev_ht7520.o
DEVOBJS += dev_ide_pci.o dev_ns16550_pci.o
DEVOBJS += dev_tulip.o dev_dp83815.o
CFLAGS  += -DCFG_PCI=1
ifeq ($(strip ${CFG_LDT_REV_017}),1)
CFLAGS  += -DCFG_LDT_REV_017=1
endif
ifeq ($(strip ${CFG_DOWNLOAD}),1)
DEVOBJS += dev_bcm1250.o download.data
CFLAGS += -DCFG_DOWNLOAD=1
endif
endif

#
# If doing bi-endian, add the compiler switch to change
# the way the vectors are generated.  These switches are
# only added to the big-endian portion of the ROM,
# which is located at the real boot vector.
#

ifeq ($(strip ${CFG_BIENDIAN}),1)
  ifeq ($(strip ${CFG_LITTLE}),0)
    CFLAGS += -DCFG_BIENDIAN=1
  endif
endif

#
# Include the makefiles for the architecture-common, cpu-specific,
# and board-specific directories.  Each of these will supply
# some files to "ALLOBJS".  The BOARD directory is optional
# as some ports are so simple they don't need boad-specific stuff.
#

include ${ARCH_SRC}/Makefile
include ${CPU_SRC}/Makefile

include ${TOP}/board/common/src/Makefile

ifneq ("$(strip ${BOARD})","")
include ${BOARD_SRC}/Makefile
include ${BOARD_ARCH_SRC}/Makefile
include ${BOARD_ARCH_SHR_SRC}/Makefile
endif

#
# Add the common object files here.
#

ALLOBJS += $(LIBOBJS) $(DEVOBJS) $(CFEOBJS) $(VENOBJS) $(UIOBJS) $(NETOBJS)
#$(PCIOBJS)

#
# VAPI continues to be a special case.
#

ifeq ($(strip ${CFG_VAPI}),1)
include ${TOP}/verif/Makefile
endif

#
# USB support
#

ifeq ($(strip ${CFG_USB}),1)
SRCDIRS += ${TOP}/usb
CFE_INC += ${TOP}/usb
include ${TOP}/usb/Makefile
endif

#
# If we're doing the VGA console thing, pull in the x86 emulator
# and the pcconsole subsystem
#

ifeq ($(strip ${CFG_VGACONSOLE}),1)
include ${TOP}/x86emu/Makefile
include ${TOP}/pccons/Makefile
endif

#
# If we're including ZLIB, then add its makefile.
#

##ifeq ($(strip ${CFG_ZLIB}),1)
##include ${TOP}/zlib/Makefile
CFLAGS += -DCFG_ZLIB=1 -DMY_ZCALLOC -DNO_MEMCPY
##endif

#
# Configure tools and basic tools flags.  This include sets up
# macros for calling the C compiler, basic flags,
# and linker scripts.
#

include ${ARCH_SRC}/tools.mk


.PHONY : all 
.PHONY : ALL

#
# Build the local tools that we use to construct other source files
#

mkpcidb : ${TOP}/hosttools/mkpcidb.c
	gcc -o mkpcidb ${TOP}/hosttools/mkpcidb.c

memconfig : ${TOP}/hosttools/memconfig.c
	gcc -o memconfig -D_MCSTANDALONE_ -D_MCSTANDALONE_NOISY_ -I${TOP}/arch/mips/cpu/sb1250/include ${TOP}/hosttools/memconfig.c ${TOP}/arch/${ARCH}/cpu/${CPU}/src/sb1250_draminit.c

pcidevs_data2.h : mkpcidb ${TOP}/pci/pcidevs_data.h
	./mkpcidb > pcidevs_data2.h

mkflashimage : ${TOP}/hosttools/mkflashimage.c
	gcc -o mkflashimage -I${TOP}/include ${TOP}/hosttools/mkflashimage.c

pci_subr.o : ${TOP}/pci/pci_subr.c pcidevs_data2.h

##build_date.c :
##	echo "const char *builddate = \"`date`\";" > build_date.c
##	echo "const char *builduser = \"`whoami`@`hostname`\";" >> build_date.c

#
# Make a define for the board name
#

CFLAGS += -D_$(patsubst "%",%,${CFG_BOARDNAME})_

LIBCFE = libcfe.a

%.o : %.c
	$(GCC) $(CFLAGS) -o $@ $<

%.o : %.S
	$(GCC) $(CFLAGS) -D_LANGUAGE_ASSEMBLY -o $@ $<
#
# This rule constructs "libcfe.a" which contains most of the object
# files.
#

$(LIBCFE) : $(ALLOBJS)
	rm -f $(LIBCFE)
	$(GAR) cr $(LIBCFE) $(ALLOBJS)
	$(GRANLIB) $(LIBCFE)

