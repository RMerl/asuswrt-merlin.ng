
#
# In most cases, you only need to modify this first section.
#
EXE = bp3
OBJS = bp3_main.o bp3.o bp3_host.o quick_ssdp.o
LIBS = -lbp3hal -lcurl -lcjson
LIBS += -L$(BCM_FSBUILD_DIR)/public/lib -lcivetweb -lpthread -ldl

# Do a generic EXE compile and install, then do an explicit install
# of the prbs script file
all dynamic install : $(EXE) generic_exe_install
	install -m 755 $(EXE) $(INSTALL_DIR)/bin

clean: generic_clean
	rm -f $(OBJS)
	rm -f $(INSTALL_DIR)/bin/$(EXE)

binaryonly_dist_clean: clean generic_binaryonly_dist_clean
	rm -f Makefile.fullsrc


#
# Set our CommEngine directory (by splitting the pwd into two words
# at /userspace and taking the first word only).
# Then include the common defines under CommEngine.
#
CURR_DIR := $(shell pwd)
BUILD_DIR:=$(subst /userspace, /userspace,$(CURR_DIR))
BUILD_DIR:=$(word 1, $(BUILD_DIR))

include $(BUILD_DIR)/make.common

CFLAGS += -DBP3_TELCO_SUPPORT -DBP3_TA_FEATURE_READ_SUPPORT
#CFLAGS += $(SSP_TYP_COMPILER_OPTS)
#LIBS += $(SSP_TYP_LIBS)
#LIBS += $(BCM_LD_FLAGS)


#
# Private apps and libs are allowed to include header files from the
# private and public directories.
#
# WARNING: Do not modify this section unless you understand the
# license implications of what you are doing.

ALLOWED_INCLUDE_PATHS := -I.\
                         -I$(BCM_FSBUILD_DIR)/public/include \
                         -I$(BCM_FSBUILD_DIR)/public/include/cjson \
                         -I$(BUILD_DIR)/userspace/public/include  \
                         -I$(BUILD_DIR)/userspace/public/include/$(OALDIR) \
                         -I$(BUILD_DIR)/userspace/private/include  \
                         -I$(BUILD_DIR)/userspace/private/include/$(OALDIR) \
			             -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD)  \
                         -I$(INC_BRCMDRIVER_PRIV_PATH)/$(BRCM_BOARD) \
                         -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD)


#
# Private apps and libs are allowed to link with libraries from the
# private and public directories.
#
# WARNING: Do not modify this section unless you understand the
# license implications of what you are doing.
#
ALLOWED_LIB_DIRS := /lib:/lib/private:/lib/public


# CFLAGS += -D
CFLAGS += -Werror


LIB_RPATH = $(INSTALL_DIR)$(subst :,:$(INSTALL_DIR),$(ALLOWED_LIB_DIRS))

#
# Implicit rule will make the .c into a .o
# Implicit rule is $(CC) -c $(CPPFLAGS) $(CFLAGS)
# See Section 10.2 of Gnu Make manual
#
$(EXE): $(OBJS)
	@echo building $@
	$(CC) $(BCM_LD_FLAGS) -o $@ $^ -Wl,-rpath-link=$(LIB_RPATH):$(CMS_LIB_RPATH) $(CMS_LIB_PATH) $(LIBS)



#
# Include the rule for making dependency files.
# The '-' in front of the second include suppresses
# error messages when make cannot find the .d files.
# It will just regenerate them.
# See Section 4.14 of Gnu Make.
#

include $(BUILD_DIR)/make.deprules

-include $(OBJS:.o=.d)

