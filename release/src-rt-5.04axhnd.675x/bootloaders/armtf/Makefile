# ARM Trusted Firmware Makefile
ARM_TF_VER := 2.2
ARM_TF_TAR := v$(ARM_TF_VER).tar.gz
ARM_TF_DIR := arm-trusted-firmware-$(ARM_TF_VER)
ARM_TF_DBG := 1
ifeq ($(ARM_TF_DBG),0)
ARM_TF_OUT := $(ARM_TF_DIR)/build/bcm/release
else
ARM_TF_OUT := $(ARM_TF_DIR)/build/bcm/debug
endif

ifeq ($(strip $(KERNEL_ARCH)),aarch64)
ARCH_PARAM :=
else
ifeq ($(strip $(BRCM_CHIP)),63138)
ARCH_PARAM := ARCH=aarch32 ARM_ARCH_MAJOR=7 ARM_CORTEX_A9=yes ARM_WITH_NEON=yes AARCH32_SP=sp_min
else
ifeq ($(strip $(BRCM_CHIP)),63148)
ARCH_PARAM := ARCH=aarch32 ARM_ARCH_MAJOR=7 ARM_CORTEX_A15=yes ARM_WITH_NEON=yes AARCH32_SP=sp_min
else
ARCH_PARAM := ARCH=aarch32 ARM_ARCH_MAJOR=7 ARM_CORTEX_A7=yes ARM_WITH_NEON=yes AARCH32_SP=sp_min
endif
endif
endif

ifeq ($(strip $(BCM_OPTEE)),y)
ARM_TF_BUILD_PARMS = PLAT=bcm \
                     DEBUG=$(ARM_TF_DBG) \
                     SPD=opteed
else
ARM_TF_BUILD_PARMS = PLAT=bcm \
                     DEBUG=$(ARM_TF_DBG)
endif

ARM_TF_BUILD_PARMS += $(ARCH_PARAM)

Q := @
ifneq ($(strip $(VERBOSE)),)
Q :=
endif

.PHONY: all armtf clean

BUILD_IN_UBOOT = y
all: armtf.bin
include $(BUILD_DIR)/make.common
CROSS_COMPILE := $(KCROSS_COMPILE)
GCCVERSION := $(shell $(CROSS_COMPILE)gcc -dumpversion)
LIBPATH1 := $(shell echo $(shell echo $(shell echo $(CROSS_COMPILE) | sed -e 's/usr\/bin/lib\/gcc/g'))xyz | sed -e 's/-xyz//g')
LIBPATH2 := $(shell echo $(shell echo $(shell echo $(CROSS_COMPILE) | sed -e 's/usr\/bin/usr\/lib\/gcc/g'))xyz | sed -e 's/-xyz//g')
LIBPATH3 := $(shell echo $(shell echo $(shell echo $(CROSS_COMPILE) | sed -e 's/bin/usr\/lib/g'))xyz | sed -e 's/-xyz//g')
#in RDK_BUILD, there is no '/usr/bin' in CROSS_COMPILE, only /bin
LIBPATH4 := $(shell echo $(shell echo $(shell echo $(CROSS_COMPILE) | sed -e 's/\/bin/lib\/gcc/g'))xyz | sed -e 's/-xyz//g')
export LDLIBS := -L$(LIBPATH1)/$(GCCVERSION) -L$(LIBPATH2)/$(GCCVERSION) -L$(LIBPATH3)/$(GCCVERSION) -L$(LIBPATH4)/$(GCCVERSION) -lgcc

$(ARM_TF_DIR)/Makefile: $(ARM_TF_TAR) $(ARM_TF_VER).patch Makefile
	$(Q)rm -rf $(ARM_TF_DIR)
	@echo "Untarring source and overrides..."
	$(Q)tar xf $(ARM_TF_TAR) 2> /dev/null
	@echo "Removing .git folder..."
	$(Q)rm -fr $(ARM_TF_DIR)/.git
	@echo "Applying patches to ARM_TF..."
	$(Q)patch -p1 -b -N -s -d$(ARM_TF_DIR) < $(ARM_TF_VER).patch
	@echo "Link platform bcm"
	$(Q)ln -s ../../plat/bcm $(ARM_TF_DIR)/plat/bcm
	$(Q)touch $@

armtf: $(ARM_TF_DIR)/Makefile
	$(Q)$(MAKE) -C $(ARM_TF_DIR) $(ARM_TF_BUILD_PARMS)

armtf.bin: armtf
ifeq ($(strip $(KERNEL_ARCH)),aarch64)
	$(Q)install -m 0644 $(ARM_TF_OUT)/bl31.bin armtf.bin
	$(Q)install -m 0644 $(ARM_TF_OUT)/bl31/bl31.elf armtf.elf
else
	$(Q)install -m 0644 $(ARM_TF_OUT)/bl32.bin armtf.bin
	$(Q)install -m 0644 $(ARM_TF_OUT)/bl32/bl32.elf armtf.elf
endif

clean:
	if [ -d $(ARM_TF_DIR) ]; then \
		$(MAKE) -C $(ARM_TF_DIR) $(ARM_TF_BUILD_PARMS) clean; \
		rm -f armtf.*; \
		patch -R -p1 -b -N -s -d$(ARM_TF_DIR) < $(ARM_TF_VER).patch; \
		touch  Makefile; \
	fi

distclean: clean
	@rm -rf $(ARM_TF_DIR)
