
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
INC_KERMIT ?= 0
CFG_KER_LZ4 ?= 0

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
BOARD_SRC       = ${TOP}/board/${BOARD}_ram/src
BOARD_INC       = ${TOP}/board/${BOARD}_ram/include
BOARD_INC       += ${TOP}/board/common/include
BOARD_ARCH_SRC  = ${ARCH_TOP}/board/${BOARD}_ram/src
BOARD_ARCH_INC  = ${ARCH_TOP}/board/${BOARD}_ram/include
BOARD_ARCH_SHR_SRC  = ${ARCH_TOP}/board/${BOARD}_shared/src
BOARD_ARCH_SHR_INC  = ${ARCH_TOP}/board/${BOARD}_shared/include
BOARD_INC       += ${TOP}/board/${BOARD}_rom/include
BOARD_INC       += ${TOP}/board/${BOARD}_btrm/inc
endif

#
# Preprocessor defines for CFE's version number
#

VDEF = -DCFE_VER_MAJ=${CFE_VER_MAJ} -DCFE_VER_MIN=${CFE_VER_MIN} -DCFE_VER_ECO=${CFE_VER_ECO}

#
# Construct the list of paths that will eventually become the include
# paths and VPATH
#

SRCDIRS = ${ARCH_SRC} ${CPU_SRC} ${LZMZ_SRC} ${BOARD_SRC} ${BOARD_ARCH_SRC} $(BOARD_ARCH_SHR_SRC) ${TOP}/main ${TOP}/vendor ${TOP}/include ${TOP}/net \
	  ${TOP}/dev ${TOP}/pci ${TOP}/ui ${TOP}/lib ${TOP}/common ${TOP}/verif ${TOP}/lzma ${TOP}/lz4hc \
		${TOP}/board/common/src

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

ifeq ($(strip ${CFG_DT}),1)
SRCDIRS += ${TOP}/fdt
CFE_INC += ${TOP}/fdt
endif

INCDIRS = $(patsubst %,-I%,$(subst :, ,$(ARCH_INC) $(CPU_INC) $(BOARD_INC) $(BOARD_ARCH_INC)  $(BOARD_ARCH_SHR_INC) $(CFE_INC)))

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


#
# Configure tools and basic tools flags.  This include sets up
# macros for calling the C compiler, basic flags,
# and linker scripts.
#

include ${ARCH_SRC}/tools.mk

#
# Add some common flags that are used on any architecture.
#

ifneq ($(strip ${INC_BTRM_DEVEL}),)
CFLAGS+=-D_BTRM_DEVEL_=$(INC_BTRM_DEVEL)
endif
CFLAGS += -I. $(INCDIRS)
CFLAGS += -D_CFE_ ${VDEF} -DCFG_BOARDNAME=\"${CFG_BOARDNAME}\" -DCONFIG_MIPS_BRCM

#
# Add flash driver support.
#
# INC_xxx_FLASH_DRIVER is exported from rom_cfe.mk

CFLAGS += -DINC_CFI_FLASH_DRIVER=$(INC_CFI_FLASH_DRIVER)
CFLAGS += -DINC_SPI_FLASH_DRIVER=$(INC_SPI_FLASH_DRIVER)
CFLAGS += -DINC_NAND_FLASH_DRIVER=$(INC_NAND_FLASH_DRIVER)
CFLAGS += -DINC_EMMC_FLASH_DRIVER=$(INC_EMMC_FLASH_DRIVER)
CFLAGS += -DINC_SPI_PROG_NAND=$(INC_SPI_PROG_NAND)
CFLAGS += -DINC_SPI_NAND_DRIVER=$(INC_SPI_NAND_DRIVER)
CFLAGS += -DINC_BTRM_BOOT=$(INC_BTRM_BOOT)
CFLAGS += -DINC_PMC_DRIVER=$(INC_PMC_DRIVER)
CFLAGS += -DCFG_COPY_PSRAM=$(CFG_COPY_PSRAM)
CFLAGS += -DINC_KERMIT=$(INC_KERMIT)
CFLAGS += -DCFG_BOOT_PSRAM=$(CFG_BOOT_PSRAM)
CFLAGS += -DNONETWORK=$(NONETWORK)
CFLAGS += -DSKIP_FLASH=$(SKIP_FLASH)
CFLAGS += -DBPCM_CFE_CMD=$(BPCM_CFE_CMD)
ifeq ($(strip $(BCM_OPTEE)),y)
CFLAGS += -DBCM_OPTEE
endif

ifneq ($(findstring _$(strip $(BRCM_CHIP))_,_4908_6858_63158_6846_6856_),)
CFLAGS += -DINC_NVRAM_MIRRORING=$(INC_NVRAM_MIRRORING)
endif

#
# Set IKOS/Sim options
#
ifeq ($(strip $(BRCM_IKOS)),1)
CFLAGS += -DCONFIG_BRCM_IKOS
endif
ifeq ($(strip $(IKOS_NO_DDRINIT)),1)
CFLAGS += -DIKOS_NO_DDRINIT
endif
ifeq ($(strip $(IKOS_BD_LINUX)),1)
CFLAGS += -DIKOS_BD_LINUX
endif
ifeq ($(strip $(IKOS_FAST_UART)),1)
CFLAGS += -DIKOS_FAST_UART
endif
ifeq ($(strip $(IKOS_FAST_CACHE_INIT)),1)
CFLAGS += -DIKOS_FAST_CACHE_INIT
endif
ifeq ($(strip ${CFG_DT}),1)
CFLAGS += -DCFG_DT
endif
ifeq ($(strip ${CFG_ATAG}),1)
CFLAGS += -DCFG_ATAG
endif
ifeq ($(strip ${CFG_NONSEC_BOOT}),1)
CFLAGS += -DCFG_NONSEC_BOOT
endif
#
# Set CFG_TCP=0 to not include the TCP protocol.
#

CFG_TCP=1
ifeq ($(strip ${CFG_TCP}),1)
CFLAGS += -DCFG_TCP
endif

#
# Set CFG_WEB_SERVER=0 to not include the web server.
#

CFG_WEB_SERVER=1
CFLAGS += -DCFG_WEB_SERVER=${CFG_WEB_SERVER}

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
# This is the makefile's main target.  Note that we actually
# do most of the work in 'ALL' not 'all', since we include
# other makefiles after this point.
#

all : ALL

#
# Macros that expand to the list of arch-independent files
#
LZMAOBJS = LzmaDecode.o dcapi.o
LZ4OBJS = lz4.o
DEVOBJS = 
## dev_newflash.o dev_null.o dev_promice.o dev_ide_common.o dev_ns16550.o dev_ds17887clock.o dev_flash.o
LIBOBJS = lib_setjmp.o lib_malloc.o lib_printf.o lib_queue.o lib_string.o lib_string2.o \
          lib_arena.o lib_crc.o

##lib_hssubr.o lib_physio.o lib_misc.o lib_qsort.o 
NETOBJS = net_ether.o net_tftp.o net_ip.o net_udp.o  net_dns.o net_arp.o \
          net_api.o net_tcp.o net_tcpbuf.o net_dhcp.o
##	    dev_tcpconsole.o  net_dhcp.o net_icmp.o
CFEOBJS = cfe_attach.o cfe_iocb_dispatch.o cfe_devfuncs.o \
          cfe_console.o  cfe_mem.o cfe_timer.o \
          cfe_background.o build_date.o \
	      cfe_xreq.o cfe_filesys.o 
ifeq ($(strip ${CFG_LDR_SREC}),1)
CFEOBJS += cfe_rawfs.o
endif

ifeq ($(strip ${CFG_DT}),1)
DTOBJS += fdt.o fdt_ro.o fdt_rw.o fdt_wip.o
endif   

##	  cfe_error.o cfe_rawfs.o cfe_fatfs.o cfe_httpfs.o  cfe_ldr_srec.o  cfe_autoboot.o cfe_boot.o cfe_ldr_elf.o cfe_ldr_raw.o  cfe_loader.o
##    cfe_main.o nvram_subr.o url.o cfe_savedata.o env_subr.o cfe_zlibfs.o
UIOBJS  = ui_command.o ui_cmddisp.o 
##	  ui_pcicmds.o \ui_tcpcmds.o ui_memcmds.o ui_loadcmds.o ui_flash.o  ui_envcmds.o ui_devcmds.o ui_netcmds.o
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

ifeq ($(strip ${INC_KERMIT}),1)
DEVOBJS += cfe_kermit.o
endif

-include ../../../../../.config
MODEL = $(subst -,,$(BUILD_NAME))
ifneq ($(strip ${MODEL}),)
CFLAGS += -D$(MODEL)
endif
ifneq ($(strip ${MOD}),)
CFLAGS += -D$(MOD)
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

# Include CFE user commands to mark/clear bad blocks
ifeq ($(strip $(BLD_FLASH_TOOLS)),1)
CFLAGS += -DINC_FLASH_TOOLS=1
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

ALLOBJS += $(LIBOBJS) $(DEVOBJS) $(CFEOBJS) $(VENOBJS) $(UIOBJS) $(NETOBJS) $(LZMAOBJS) 

ifeq ($(strip ${CFG_KER_LZ4}),1)
CFLAGS += -DUSE_LZ4_DECOMPRESSOR
ALLOBJS += $(LZ4OBJS)
endif

ifeq ($(strip ${CFG_DT}),1)
ALLOBJS += $(DTOBJS)
endif

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

ifeq ($(strip ${CFG_ZLIB}),1)
include ${TOP}/zlib/Makefile
CFLAGS += -DCFG_ZLIB=1 -DMY_ZCALLOC -DNO_MEMCPY
endif

#
# Vendor extensions come next - they live in their own directory.
#

include ${TOP}/vendor/Makefile

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

#build_date.c :
	#echo "const char *builddate = \"`date`\";" > ram_build_date.c
	#echo "const char *builduser = \"`whoami`@`hostname`\";" >> ram_build_date.c
	#mv ram_build_date.c build_date.c

#
# Make a define for the board name
#

CFLAGS += -D_$(patsubst "%",%,${CFG_BOARDNAME})_ -D_BUILDDATE_="\"$(shell date)\"" -D_BUILDUSER_="\"$(shell whoami)@$(shell hostname)\""

LIBCFE = libcfe.a

%.o : %.c
	$(GCC) $(CFLAGS) -o $@ $<

%.o : %.S
	$(GCC) $(CFLAGS) -D_LANGUAGE_ASSEMBLY -o $@ $<


%.o : %.html
	echo unsigned char $*_html[] = { > $*.c
	hexdump -v -e '"    " 12/1 "0x%2.2x, " "\n"' $< | sed -e "s/ 0x .*$\//" -e "\$$s/, *$$//" >> $*.c
	echo '    };' >> $*.c
	echo 'int $*_html_size = sizeof($*_html);' >> $*.c
	echo >> $*.c
	$(GCC) $(CFLAGS) -o $@ $*.c
	rm -f $*.c

%.o : %.css
	echo unsigned char $*_css[] = { > $*.c
	hexdump -v -e '"    " 12/1 "0x%2.2x, " "\n"' $< | sed -e "s/ 0x .*$\//" -e "\$$s/, *$$//" >> $*.c
	echo '    };' >> $*.c
	echo 'int $*_css_size = sizeof($*_css);' >> $*.c
	echo >> $*.c
	$(GCC) $(CFLAGS) -o $@ $*.c
	rm -f $*.c

%.o : %.gif
	echo unsigned char $*_gif[] = { > $*.c
	hexdump -v -e '"    " 12/1 "0x%2.2x, " "\n"' $< | sed -e "s/ 0x .*$\//" -e "\$$s/, *$$//" >> $*.c
	echo '    };' >> $*.c
	echo 'int $*_gif_size = sizeof($*_gif);' >> $*.c
	echo >> $*.c
	$(GCC) $(CFLAGS) -o $@ $*.c
	rm -f $*.c

#
# This rule constructs "libcfe.a" which contains most of the object
# files.
#

$(LIBCFE) : $(ALLOBJS)
	rm -f $(LIBCFE)
	$(GAR) cr $(LIBCFE) $(ALLOBJS)
	$(GRANLIB) $(LIBCFE)





