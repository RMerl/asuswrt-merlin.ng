ifndef CDK
nocdk:; @echo 'The $$CDK environment variable is not set'; exit 1
else

################################################################################
#
# Configure build environment
#
################################################################################

# Build tools
COMPILER = gcc
LINKER   = ld
ARCHIVE  = ar

CC = $(CROSS_COMPILE)$(COMPILER)
LD = $(CROSS_COMPILE)$(LINKER)
AR = $(CROSS_COMPILE)$(ARCHIVE)

# Toolchain compiler flags
TC_CFLAGS += -fPIC

# Make tools
MAKE = make
ECHO = echo

# Build directory
ifndef BLDDIR
BLDDIR = $(CURDIR)
endif

# Destination directory for application
ifndef DSTDIR
DSTDIR = $(BLDDIR)
endif

################################################################################
#
# Configure compiler and linker options
#
################################################################################

ifndef SYS_NO_TTY
CFG_CPPFLAGS += -DSYS_HAS_TTY
endif

# Configure include path
CFG_CPPFLAGS += -I$(CDK)/include -I$(BMD)/include -I$(PHY)/include -I$(CURDIR)

# CONFIG_MDK_BCA_BEGIN
CFG_CPPFLAGS += -g $(ALLOWED_INCLUDE_PATHS) 
CFG_CPPFLAGS += -I$(TOOLCHAIN)/include -L$(TOOLCHAIN)/lib
# CONFIG_MDK_BCA_END


# Use CDK custom config file and system C library
# (see cdk_custom_config.h for details)
CFG_CPPFLAGS += -DCDK_INCLUDE_CUSTOM_CONFIG -DBMD_INCLUDE_CUSTOM_CONFIG -DUSE_SYSTEM_LIBC -DBMD_CONFIG_INCLUDE_DMA=0 -DBMD_SYS_USLEEP=_usleep -DPHY_SYS_USLEEP=_usleep

ifndef SYS_BE_PIO
ifeq ($(strip $(BRCM_CHIP)),63138)
CFG_CPPFLAGS += -DSYS_BE_PIO=0
else
ifeq ($(strip $(BRCM_CHIP)),63148)
CFG_CPPFLAGS += -DSYS_BE_PIO=0
else
ifeq ($(strip $(BRCM_CHIP)),4908)
CFG_CPPFLAGS += -DSYS_BE_PIO=0
else
ifeq ($(strip $(BRCM_CHIP)),47189)
CFG_CPPFLAGS += -DSYS_BE_PIO=0
else
CFG_CPPFLAGS += -DSYS_BE_PIO=1
endif 
endif 
endif 
endif 
endif
ifndef SYS_BE_PACKET
CFG_CPPFLAGS += -DSYS_BE_PACKET=0
endif
ifndef SYS_BE_OTHER
CFG_CPPFLAGS += -DSYS_BE_OTHER=0
endif

# Default application flags
MIN_GCC_VERSION = "5"
GCC_VERSION := "`$(CC) -dumpversion`"
IS_GCC_ABOVE_MIN_VERSION := $(shell expr "$(GCC_VERSION)" ">=" "$(MIN_GCC_VERSION)")
APP_CFLAGS += -Wall -Werror
ifeq "$(IS_GCC_ABOVE_MIN_VERSION)" "1"
APP_CFLAGS += -Wno-error=maybe-uninitialized
endif

# Combine all flags into standard variables
CPPFLAGS += $(CFG_CPPFLAGS)  $(TC_CPPFLAGS)  $(PL_CPPFLAGS) \
            $(APP_CPPFLAGS)  $(ADD_CPPFLAGS)  $(CMS_LOG_FLAGS) $(COMPILE_FLAGS)
CFLAGS   += $(CFG_CFLAGS)    $(TC_CFLAGS)    $(PL_CFLAGS)   \
            $(APP_CFLAGS)    $(ADD_CFLAGS)   $(CMS_LOG_FLAGS) $(COMPILE_FLAGS)
LDFLAGS  += $(CFG_LDFLAGS)   $(TC_LDFLAGS)   $(PL_LDFLAGS)  \
            $(APP_LDFLAGS)   $(ADD_LDFLAGS)  

CFLAGS += -I$(TOOLCHAIN)/include -L$(TOOLCHAIN)/lib

################################################################################
#
# Configure CDK Libraries
#
#  1) Once you import $(CDK)/targets.mk, $(CDK_LIBS) will contain the list
#     of CDK libraries in correct link order.
#
#  2) To build the CDK, the following should be specified:
#
#     CDK_BLDDIR   - the directory where the libraries will be placed
#     CDK_CPPFLAGS - C preprocessor flags
#     CDK_CFLAGS   - C compiler flags
#     CDK_CC       - compiler
#     CDK_LD       - linker (currently not used)
#     CDK_AR       - archiver (for creating libraries)
#
################################################################################

ifdef BRCM_EXT_SWITCH_TYPE
ifneq ($(strip $(BRCM_EXT_SWITCH_TYPE)),0)
CFLAGS += -DBRCM_EXT_SWITCH_TYPE=$(BRCM_EXT_SWITCH_TYPE)
CPPFLAGS += -DBRCM_EXT_SWITCH_TYPE=$(BRCM_EXT_SWITCH_TYPE)
endif
endif

ifdef BCM_PHY_54616
ifneq ($(strip $(BCM_PHY_54616)),0)
CFLAGS += -DBCM_PHY_54616
CPPFLAGS += -DBCM_PHY_54616
endif
endif

# Check for Perl support
ifdef NO_PERL
export CDK_PERL   = n/a
endif

# Configure CDK libraries
export CDK_BLDDIR = $(BLDDIR)/cdk
export CDK_CFLAGS = $(CFLAGS)
export CDK_CPPFLAGS = $(CPPFLAGS)

# Configure BMD libraries
export BMD_BLDDIR = $(BLDDIR)/bmd
export BMD_CFLAGS = $(CFLAGS)
export BMD_CPPFLAGS = $(CPPFLAGS)

# Configure PHY libraries
export PHY_BLDDIR = $(BLDDIR)/phy
export PHY_CFLAGS = $(CFLAGS)
export PHY_CPPFLAGS = $(CPPFLAGS)

# Configure Toolchain
export CDK_CC = $(CC)
export CDK_LD = $(LD)
export CDK_AR = $(AR)

# CDK_LIBS is defined in $(CDK)/targets.mk
include $(CDK)/targets.mk
CDK_BLIBS = $(addprefix $(CDK_BLDDIR)/,$(CDK_LIBS))

# BMD_LIBS is defined in $(BMD)/targets.mk
include $(BMD)/targets.mk
BMD_BLIBS = $(addprefix $(BMD_BLDDIR)/,$(BMD_LIBS))

# PHY_LIBS is defined in $(PHY)/targets.mk
include $(PHY)/targets.mk
PHY_BLIBS = $(addprefix $(PHY_BLDDIR)/,$(PHY_LIBS))

ifneq ($(filter 68%,$(strip $(BRCM_CHIP))),)
SWMDK_CHIP=6816
else
ifneq ($(findstring _$(BRCM_CHIP)_,_63178_),)
SWMDK_CHIP=63100
else
SWMDK_CHIP=6300
endif
endif

ifneq ($(findstring _$(BRCM_CHIP)_,_47622_),)
SWMDK_CHIP=63100
endif

CDK_PKG_OPTIONS = -s -c bcm$(SWMDK_CHIP)
BMD_PKG_OPTIONS = -s -c bcm$(SWMDK_CHIP)_a0
ifdef BRCM_EXT_SWITCH_TYPE
ifneq ($(strip $(BRCM_EXT_SWITCH_TYPE)),0)
CDK_PKG_OPTIONS := $(CDK_PKG_OPTIONS),bcm$(BRCM_EXT_SWITCH_TYPE)
BMD_PKG_OPTIONS := $(BMD_PKG_OPTIONS),bcm$(BRCM_EXT_SWITCH_TYPE)_$(subst ",,$(BRCM_EXT_SWITCH_REV))
endif
endif

PHY_PKG_OPTIONS = -s -b bcm6368_miim_reg_int,robo_miim_int -c bcm_generic
ifdef BRCM_EXT_SWITCH_TYPE
ifneq ($(strip $(BRCM_EXT_SWITCH_TYPE)),0)
PHY_PKG_OPTIONS := $(PHY_PKG_OPTIONS),bcm$(BRCM_EXT_SWITCH_TYPE)
endif
endif
ifdef BCM_PHY_54616
ifneq ($(strip $(BCM_PHY_54616)),0)
PHY_PKG_OPTIONS := $(PHY_PKG_OPTIONS),bcm54616
endif
endif
ifneq ($(findstring _$(strip $(SWMDK_CHIP))_,_63100_),)
PHY_PKG_OPTIONS := $(PHY_PKG_OPTIONS),bcm$(SWMDK_CHIP)
endif

export CDK_PKG_OPTIONS BMD_PKG_OPTIONS PHY_PKG_OPTIONS

MDKBLIBS = $(CDK_BLIBS) $(PHY_BLIBS) $(BMD_BLIBS) 

################################################################################
# Make rules
################################################################################

.PHONY: all cdk clean bmd phy 

all: private_lib_install

private_lib_install: cdk phy bmd
	mkdir -p $(INSTALL_DIR)/lib/private
	install -m 755 $(MDKBLIBS) $(INSTALL_DIR)/lib/private

cdk:
	$(MAKE) -C $(CDK)

bmd:
	$(MAKE) -C $(BMD)

phy:
	$(MAKE) -C $(PHY)

$(BLDDIR)/%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

clean:
	$(MAKE) -C $(CDK) clean
	$(MAKE) -C $(CDK) cleanpkgs
	$(MAKE) -C $(BMD) clean
	$(MAKE) -C $(BMD) cleanpkgs
	$(MAKE) -C $(PHY) clean
	$(MAKE) -C $(PHY) cleanpkgs       
	
bmd_clean:
	$(MAKE) -C $(BMD) clean
	$(MAKE) -C $(BMD) cleanpkgs

cdk_clean:
	$(MAKE) -C $(CDK) clean
	$(MAKE) -C $(CDK) cleanpkgs

phy_clean:
	$(MAKE) -C $(PHY) clean
	$(MAKE) -C $(PHY) cleanpkgs
endif
