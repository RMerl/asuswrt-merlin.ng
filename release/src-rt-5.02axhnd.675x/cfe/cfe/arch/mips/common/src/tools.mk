#
# Basic compiler options and preprocessor flags
#

CFLAGS += -g -c  -ffreestanding
CFLAGS += -Wall -Werror -Wstrict-prototypes -Wmissing-prototypes
ifeq ($(strip ${CFG_RAMAPP}),1) 
CFLAGS += -O0
else
CFLAGS += -Os
endif

#
# Tools locations
#

ifndef TOOLCHAIN_BASE
  TOOLCHAIN_BASE := /opt/toolchains/
endif

ifeq ($(strip ${CFG_LITTLE}),1)
  TOOLS :=$(TOOLCHAIN_BASE)/crosstools-mipsel-gcc-5.3-linux-4.1-uclibc-1.0.12-binutils-2.25-NPTL/usr/bin/
  TOOLPREFIX :=mipsel-buildroot-linux-uclibc-
else
  TOOLS :=$(TOOLCHAIN_BASE)/crosstools-mips-gcc-5.3-linux-4.1-uclibc-1.0.12-binutils-2.25-NPTL/usr/bin/
  TOOLPREFIX :=mips-buildroot-linux-uclibc-
endif


GCC        ?= $(TOOLS)$(TOOLPREFIX)gcc
GLD        ?= $(TOOLS)$(TOOLPREFIX)ld
GAR         ?= $(TOOLS)$(TOOLPREFIX)ar
GOBJDUMP    ?= $(TOOLS)$(TOOLPREFIX)objdump
GOBJCOPY    ?= $(TOOLS)$(TOOLPREFIX)objcopy
GRANLIB     ?= $(TOOLS)$(TOOLPREFIX)ranlib

#
# Check for 64-bit mode
#

ifeq ($(strip ${CFG_MLONG64}),1)
  CFLAGS += -mlong64 -D__long64
endif

#
# Figure out which linker script to use
#

ifeq ($(strip ${CFG_RAMAPP}),1)
   CFLAGS += -DCFG_RAMAPP=1
   LDFLAGS += -g --script ${ARCH_SRC}/cfe_ramapp.lds
   LDFLAGS += -Ttext ${RAMAPP_TEXT}
   CFLAGS += -DRAMAPP_TEXT=${RAMAPP_TEXT}
   CFLAGS += -DCFG_RUNFROMKSEG0=1
else
  ifeq ($(strip ${CFG_RELOC}),0)
    ifeq ($(strip ${CFG_BOOTRAM}),1)
      CFLAGS += -DCFG_BOOTRAM=1
      ROMRAM = ram
    else
      CFLAGS += -DCFG_BOOTRAM=0
      ifeq ($(strip ${CFG_BOOTBTRM}),1)
        ROMRAM = btrm
      else
        ROMRAM = rom
        ifeq ($(strip ${CFG_BOOTUTILS}),1)
          ROMRAM = utils
          CFLAGS += -DCFG_BOOTUTILS=1 
          #-ffunction-sections -Wl,--gc-sections
        endif
      endif
    endif
    ifeq ($(strip ${CFG_UNCACHED}),1)
      CFLAGS += -DCFG_RUNFROMKSEG0=0
	  LDFLAGS += -g --script ${ARCH_SRC}/cfe_${ROMRAM}_uncached.lds
    else
      CFLAGS += -DCFG_RUNFROMKSEG0=1
      ifeq ($(strip ${BRCM_CHIP}),6848)
        LDFLAGS += -g --script ${ARCH_SRC}/cfe_${ROMRAM}_psram.lds
      else
        ifeq ($(strip ${CFG_COPY_PSRAM}),1)
          ifeq ($(strip ${BLD_NAND}),1)
            LDFLAGS += -g --script ${ARCH_SRC}/cfe_${ROMRAM}_psram.lds
          else
            LDFLAGS += -g --script ${ARCH_SRC}/cfe_${ROMRAM}_psram_nor.lds
          endif
        else
          ifeq ($(strip ${CFG_BOOT_PSRAM}),1)
            ifeq ($(strip ${BLD_NAND}),1)
              LDFLAGS += -g --script ${ARCH_SRC}/cfe_${ROMRAM}_psram.lds
            else
              LDFLAGS += -g --script ${ARCH_SRC}/cfe_${ROMRAM}_psram_nor.lds
            endif
          else
            LDFLAGS += -g --script ${ARCH_SRC}/cfe_${ROMRAM}_cached.lds
          endif
        endif
       endif    
    endif

    ifeq ($(strip ${CFG_BOOTUTILS}),1)
      LDFLAGS += -Ttext ${BOOTUTILS_TEXT} 
      #--gc-sections
    else
      ifeq ($(strip ${CFG_BOOTBTRM}),1)
        LDFLAGS += -Ttext ${BOOTBTRM_TEXT} 
      else # cfe rom
        LDFLAGS += -Ttext ${CFEROM_TEXT}
      endif # cfe rom
    endif # cfe util
  else # CFG RELOC 
    CFLAGS += -membedded-pic -mlong-calls -DCFG_EMBEDDED_PIC=1
    ifeq ($(strip ${CFG_UNCACHED}),1)
      CFLAGS += -DCFG_RUNFROMKSEG0=0
      LDFLAGS += -g --script ${ARCH_SRC}/cfe_rom_reloc_uncached.lds --embedded-relocs
    else
      CFLAGS += -DCFG_RUNFROMKSEG0=1
      ifeq ($(strip ${CFG_TEXTAT1MB}),1)
        LDFLAGS += -g --script ${ARCH_SRC}/cfe_rom_reloc_cached_biendian.lds --embedded-relocs
      else
        LDFLAGS += -g --script ${ARCH_SRC}/cfe_rom_reloc_cached.lds --embedded-relocs
      endif
    endif
  endif # CFG_RELOC
endif # CFG_RAMAPP
#
# Determine target endianness
#

ifeq ($(strip ${CFG_LITTLE}),1)
  CFLAGS += -EL
  LDFLAGS += -EL
else
  CFLAGS += -EB
  LDFLAGS += -EB
endif

