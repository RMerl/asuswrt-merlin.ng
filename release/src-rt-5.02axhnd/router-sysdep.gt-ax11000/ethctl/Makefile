
#GLOBAL_RELEASE_SCRIPT_NOTOUCH


ifneq ($(wildcard Makefile.fullsrc),)

include Makefile.fullsrc

else

EXE = ethctl
dynamic: install

clean:
	rm -f $(EXE)

ethctl:
ifneq ($(PREBUILT_BCMBIN),1)
	cp $(EXE)_$(PROFILE).save $(EXE)
else
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

install: $(EXE)
	install -m 755 $(EXE) $(INSTALL_DIR)/bin
	$(STRIP) $(INSTALL_DIR)/bin/$(EXE)
	mkdir -p $(INSTALL_DIR)/sbin
	ln -sf ../bin/$(EXE) $(INSTALL_DIR)/sbin/$(EXE)

