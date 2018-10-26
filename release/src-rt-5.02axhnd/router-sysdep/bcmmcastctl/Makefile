

#GLOBAL_RELEASE_SCRIPT_NOTOUCH


ifneq ($(wildcard Makefile.fullsrc),)

include Makefile.fullsrc

else

EXE = bcmmcastctl
LN  = bmc
dynamic: generic_exe_install

clean:
	rm -f $(EXE)
	rm -f $(INSTALL_DIR)/bin/$(LN)

ifneq ($(PREBUILT_BCMBIN),1)
bcmmcastctl:
	cp $(EXE)_$(PROFILE).save $(EXE)
	ln -sf $(EXE) $(INSTALL_DIR)/bin/$(LN)
else
bcmmcastctl:
	cp -f $(TOP_PLATFORM)/$(shell pwd | sed 's/\(.*router-sysdep\/\)//')/$(PRBM_DIR)/$@ ./
endif


#
# Set our CommEngine directory (by splitting the pwd into two words
# at /userspace and taking the first word only).
# Then include the common defines under CommEngine.
#

CURR_DIR := $(shell pwd)
BUILD_DIR:=$(HND_SRC)

include $(BUILD_DIR)/make.common
endif


