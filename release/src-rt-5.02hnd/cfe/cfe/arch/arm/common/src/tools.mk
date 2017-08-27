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
CFLAGS += -Wall -Werror -Wstrict-prototypes -fno-stack-protector -fno-delete-null-pointer-checks -Wframe-larger-than=2560
# O0 gives the best compression rate 
ifeq ($(strip ${CFG_RAMAPP}),1) 
CFLAGS += -O0
else
CFLAGS += -Os
endif 

#
# Tools locations
#
ifeq ($(strip ${CFG_ARMV8_AARCH64}),1)
   TOOLS :=/opt/toolchains/crosstools-aarch64-gcc-5.3-linux-4.1-glibc-2.22-binutils-2.25/usr/bin/
   TOOLPREFIX :=aarch64-buildroot-linux-gnu-
else
   TOOLS :=/opt/toolchains/crosstools-arm-gcc-5.3-linux-4.1-glibc-2.22-binutils-2.25/usr/bin/
   TOOLPREFIX :=arm-buildroot-linux-gnueabi-
endif

GCC        ?= $(TOOLS)$(TOOLPREFIX)gcc
GCPP       ?= $(TOOLS)$(TOOLPREFIX)cpp
GLD        ?= $(TOOLS)$(TOOLPREFIX)ld
GAR        ?= $(TOOLS)$(TOOLPREFIX)ar
OBJDUMP    ?= $(TOOLS)$(TOOLPREFIX)objdump
OBJCOPY    ?= $(TOOLS)$(TOOLPREFIX)objcopy
RANLIB     ?= $(TOOLS)$(TOOLPREFIX)ranlib
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
else
   ifeq ($(strip $(IKOS_NO_DDRINIT)),1)
      #LDFLAGS += -Ttext 0xe00000
      # temporary change, should use the lds file to customize the linker address
      ifeq ($(strip ${BRCM_CHIP}),6858)
          ifeq ($(strip ${BOOT_PRE_CFE}),1)
              LDFLAGS += -Ttext 0x60000000
          else
              LDFLAGS += -Ttext 0x82620000
          endif
      else
           ifeq ($(strip ${BRCM_CHIP}),68360)
              LDFLAGS += -Ttext 0x82600000
           else
              ifneq ($(BRCM_CHIP),4908)
                  LDFLAGS += -Ttext 0x80710000
              else
                  LDFLAGS += -Ttext 0x90000000
              endif
           endif
      endif
   else
      ifeq ($(strip ${INC_BTRM_BUILD}),1)
         ifneq ($(findstring _$(strip $(BRCM_CHIP))_,_4908_6858_63158_68360_),)
            LDFLAGS += -Ttext 0xfff00000
            LDFLAGS += -Tdata 0x90000000
         else
            LDFLAGS += -Ttext 0x80704000
         endif
      else
         ifeq ($(strip ${BRCM_CHIP}),6858)
             ifeq ($(strip ${BOOT_PRE_CFE}),1)
                 LDFLAGS += -Ttext 0x60000000
             else
                 LDFLAGS += -Ttext 0x82620000
             endif
         else
             ifeq ($(strip ${BRCM_CHIP}),68360)
                 LDFLAGS += -Ttext 0x82600000
             else
                 ifneq ($(BRCM_CHIP),4908)
                     ifeq ($(BRCM_CHIP), 47189)
                         LDFLAGS += -Ttext 0x4000
                     else
                         LDFLAGS += -Ttext 0x80710000
                     endif
                 else
                     LDFLAGS += -Ttext 0x90000000
                 endif
             endif      
         endif
      endif
   endif
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

