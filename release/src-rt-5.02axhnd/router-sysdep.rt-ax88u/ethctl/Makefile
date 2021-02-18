
#GLOBAL_RELEASE_SCRIPT_NOTOUCH


ifneq ($(wildcard Makefile.fullsrc),)

include Makefile.fullsrc

else

EXE = ethctl
dynamic: generic_exe_install

clean:
	rm -f $(EXE)

ethctl:
	cp $(EXE)_$(PROFILE).save $(EXE)

#
# Set our CommEngine directory (by splitting the pwd into two words
# at /userspace and taking the first word only).
# Then include the common defines under CommEngine.
#

CURR_DIR := $(shell pwd)
BUILD_DIR:=$(HND_SRC)

include $(BUILD_DIR)/make.common
endif



