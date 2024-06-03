ifneq ($(wildcard Makefile.fullsrc),)

include Makefile.fullsrc

else

.NOTPARALLEL:

EXE = beepPkgBuilder
EXE2 = reposcan

dynamic:$(EXE) $(EXE2)

clean:
	rm -f $(EXE) $(EXE2)

$(EXE) :
ifeq ($(shell uname -m),x86_64)
	cp -f $(EXE).x64.save $(EXE)
else
	cp -f $(EXE).x32.save $(EXE)
endif

$(EXE2):
ifeq ($(shell uname -m),x86_64)
	cp -f $(EXE2).x64.save $(EXE2)
else
	cp -f $(EXE2).x32.save $(EXE2)
endif

.PHONY: $(EXE) $(EXE2)
	
#
# Set our CommEngine directory (by splitting the pwd into two words
# at /userspace and taking the first word only).
# Then include the common defines under CommEngine.
#

CURR_DIR := $(shell pwd)
BUILD_DIR:=$(subst /hostTools, /hostTools,$(CURR_DIR))
BUILD_DIR:=$(word 1, $(BUILD_DIR))

include $(BUILD_DIR)/make.common
endif
