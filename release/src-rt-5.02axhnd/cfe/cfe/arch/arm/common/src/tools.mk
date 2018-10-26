#
# Addresses of things unless overridden
#

#
# BOOTRAM mode (runs from ROM vector assuming ROM is writable)
# implies no relocation.
#

ifeq ($(strip ${CFG_BOOTRAM}),1)
  CFG_RELOC = 0
endif


#
# Basic compiler options and preprocessor flags
#

# Add GCC lib
LDLIBS += -L $(shell dirname `$(GCC) $(CFLAGS) -print-libgcc-file-name`) -lgcc

CFLAGS += -gdwarf-2 -c -fno-builtin -ffreestanding
CFLAGS += -Wall -Werror -Wstrict-prototypes -fno-stack-protector -fno-delete-null-pointer-checks
# O0 gives the best compression rate 
ifeq ($(strip ${CFG_RAMAPP}),1) 
CFLAGS += -Wframe-larger-than=2560
CFLAGS += -O0
else
CFLAGS += -Os
endif 

ifndef TOOLCHAIN_BASE
  TOOLCHAIN_BASE := /opt/toolchains/
endif

#
# Tools locations
#
ifndef TOOLCHAIN_BASE
TOOLCHAIN_BASE := /opt/toolchains/
endif

ifeq ($(strip ${CFG_ARMV8_AARCH64}),1)
   TOOLS :=$(TOOLCHAIN_BASE)/crosstools-aarch64-gcc-5.5-linux-4.1-glibc-2.26-binutils-2.28.1/usr/bin/
   TOOLPREFIX :=aarch64-buildroot-linux-gnu-
else
   TOOLS :=$(TOOLCHAIN_BASE)/crosstools-arm-gcc-5.5-linux-4.1-glibc-2.26-binutils-2.28.1/usr/bin/
   TOOLPREFIX :=arm-buildroot-linux-gnueabi-
endif

GCC        ?= $(TOOLS)$(TOOLPREFIX)gcc
GCPP       ?= $(TOOLS)$(TOOLPREFIX)cpp
GLD        ?= $(TOOLS)$(TOOLPREFIX)ld
GAR        ?= $(TOOLS)$(TOOLPREFIX)ar
GOBJDUMP   ?= $(TOOLS)$(TOOLPREFIX)objdump
GOBJCOPY   ?= $(TOOLS)$(TOOLPREFIX)objcopy
GRANLIB    ?= $(TOOLS)$(TOOLPREFIX)ranlib
STRIP      ?= $(TOOLS)$(TOOLPREFIX)strip

#
# Check for 64-bit mode
#
ifeq ($(strip ${CFG_ARMV8_AARCH64}),1)
ifeq ($(strip ${CFG_MLONG64}),1)
  CFLAGS += -mabi=lp64
else
  CFLAGS += -mabi=ilp32
endif
LDFLAGS += -g --script ${ARCH_SRC}/cfe_aarch64.lds
else
LDFLAGS += -g --script ${ARCH_SRC}/cfe.lds
endif

ifeq ($(strip ${CFG_MLONG64}),1)
  CFLAGS += -D__long64
endif

#use the 16KB below text as MMU pagetable
ifeq ($(strip ${CFG_RAMAPP}),1)
   CFLAGS += -DCFG_RAMAPP=1
   LDFLAGS += -Ttext ${RAMAPP_TEXT}
   CFLAGS += -DRAMAPP_TEXT=${RAMAPP_TEXT}
else
   ifeq ($(strip ${CFG_BOOTBTRM}),1)
     LDFLAGS += -Ttext ${BOOTBTRM_TEXT} 
     ifneq ($(strip $(BOOTBTRM_DATA)),)
       LDFLAGS += -Tdata ${BOOTBTRM_DATA}
     endif
   else # cfe rom
      LDFLAGS += -Ttext ${CFEROM_TEXT}
   endif # cfe rom
   ifeq ($(strip ${CFG_RELOC}),0)
      ifeq ($(strip ${CFG_BOOTRAM}),1)
         CFLAGS += -DCFG_BOOTRAM=1
      else
         CFLAGS += -DCFG_BOOTRAM=0
      endif
   else
      CFLAGS += -membedded-pic -mlong-calls -DCFG_EMBEDDED_PIC=1
      LDFLAGS +=  --embedded-relocs
   endif
endif

