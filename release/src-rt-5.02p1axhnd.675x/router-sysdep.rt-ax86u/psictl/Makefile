#
# In most cases, you only need to modify this first section.
#
EXE = psictl
OBJS = psictl.o
LIBS = -L$(HND_SRC)/router-sysdep/bcm_flashutil -lbcm_flashutil -L$(HND_SRC)/router-sysdep/bcm_boardctl -lbcm_boardctl
LIBS += -L$(HND_SRC)/router-sysdep/bcm_util -lbcm_util -L$(HND_SRC)/router-sysdep/sys_util -lsys_util -L$(HND_SRC)/router-sysdep/gen_util -lgen_util -lrt


all dynamic install: conditional_build

clean: generic_clean
	rm -f $(INSTALL_DIR)/bin/$(EXE)



#
# Set our CommEngine directory (by splitting the pwd into two words
# at /userspace and taking the first word only).
# Then include the common defines under CommEngine.
#
CURR_DIR := $(shell pwd)
BUILD_DIR:=$(HND_SRC)

include $(BUILD_DIR)/make.common



ifneq ($(strip $(BUILD_PSICTL)),)
conditional_build: sanity_check $(EXE) generic_exe_install
else
conditional_build: sanity_check
	@echo "Skipping $(EXE) (not configured)"
endif


#
# Public apps are allowed to include header files from the
# public directory.
#
# WARNING: Do not modify this section unless you understand the
# license implications of what you are doing.
#
ALLOWED_INCLUDE_PATHS := -I.\
                         -I$(BUILD_DIR)/userspace/public/include  \
                         -I$(BUILD_DIR)/userspace/public/include/$(OALDIR) \
                         -I$(BCM_FSBUILD_DIR)/public/include \
                         -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD) \
                         -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD) \
						-I$(HND_SRC)/router-sysdep/bcm_boardctl/ \
						-I$(HND_SRC)/router-sysdep/bcm_util



#
# Public libs are allowed to link with libraries from the
# public directory.
#
# WARNING: Do not modify this section unless you understand the
# license implications of what you are doing.
#
ALLOWED_LIB_DIRS := /lib:/lib/public





#
# Implicit rule will make the .c into a .o
# Implicit rule is $(CC) -c $(CPPFLAGS) $(CFLAGS)
# See Section 10.2 of Gnu Make manual
# 
$(EXE): $(OBJS)
	$(CC) $(BCM_LD_FLAGS) -o $@ $(OBJS) -Wl,-rpath,$(CMS_LIB_RPATH) $(CMS_LIB_PATH) $(LIBS)



#
# Include the rule for making dependency files.
# The '-' in front of the second include suppresses
# error messages when make cannot find the .d files.
# It will just regenerate them.
# See Section 4.14 of Gnu Make.
#

include $(BUILD_DIR)/make.deprules

-include $(OBJS:.o=.d)


