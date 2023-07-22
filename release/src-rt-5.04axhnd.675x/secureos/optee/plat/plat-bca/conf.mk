PLATFORM_FLAVOR ?= 63138

CFG_TEE_CORE_LOG_LEVEL = 1
# 32-bit flags
#core_arm32-platform-aflags	+= -mfpu=neon

AFLAGS += -DOPTEE_BUILD -Wno-cast-function-type -Wno-implicit-fallthrough
CFLAGS += -DOPTEE_BUILD -Wno-cast-function-type -Wno-implicit-fallthrough

$(call force,CFG_TEE_CORE_USER_MEM_DEBUG,0)
ifeq ($(PLATFORM_FLAVOR),63138)
include core/arch/arm/cpu/cortex-a9.mk
CFLAGS += -DCORTEX_A9
AFLAGS += -DA9
endif

ifneq ($(findstring _$(strip $(PLATFORM_FLAVOR))_,_63178_6846_6878_47622_6756_),)
include core/arch/arm/cpu/cortex-a7.mk
$(call force,CFG_PL011,y)
endif

ifneq ($(findstring _$(strip $(PLATFORM_FLAVOR))_,_63158_6858_6856_63146_4912_4908_4906_),)
include core/arch/arm/cpu/cortex-armv8-0.mk
$(call force,CFG_ARM64_core,y)
$(call force,CFG_PL011,y)
CROSS_COMPILE64 = $(CROSS_COMPILE)
endif

$(call force,CFG_SECURE_TIME_SOURCE_CNTPCT,y)

platform-cflags-generic += -I../../../shared/opensource/include/bcm963xx
core-platform-cppflags  += -I../../../shared/opensource/include/bcm963xx

ifeq ($(platform-debugger-arm),1)
# ARM debugger needs this
platform-cflags-debug-info = -gdwarf-2
platform-aflags-debug-info = -gdwarf-2
endif

ifeq ($(platform-flavor-armv8),1)
$(call force,CFG_WITH_ARM_TRUSTED_FW,y)
endif

$(call force,CFG_GENERIC_BOOT,y)
$(call force,CFG_GIC,y)
$(call force,CFG_BCM_UART,y)
$(call force,CFG_PM_STUBS,y)

ifeq ($(PLATFORM_FLAVOR),63138)
core-platform-subdirs += $(platform-dir)/bcm63138
$(call force,CFG_PL310,y)
$(call force,CFG_CC_OPTIMIZE_FOR_SIZE,n)
endif

ifeq ($(CFG_ARM64_core),y)
$(call force,CFG_WITH_LPAE,y)
ta-targets += ta_arm64
supported-ta-targets = ta_arm64
else
ta-targets = ta_arm32
supported-ta-targets = ta_arm32
$(call force,CFG_ARM32_core,y)
$(call force,CFG_PSCI_ARM32,y)
endif

CFG_WITH_STACK_CANARIES ?= y
CFG_WITH_STATS ?= y

$(call force,CFG_SM_NO_CYCLE_COUNTING,n)

# disable NEON for following platforms
ifneq ($(findstring _$(strip $(PLATFORM_FLAVOR))_,_63138_6846_6878_),)
$(call force,CFG_WITH_VFP,n)
endif
