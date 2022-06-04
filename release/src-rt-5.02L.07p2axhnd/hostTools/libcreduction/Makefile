#
# Makefile to call either uclibc or glibc optimizer
# 
# Copyright (C) 2004 Broadcom Corporation
#

default: all

#
# Set our CommEngine directory (by splitting the pwd into two words
# at /userspace and taking the first word only).
# Then include the common defines under CommEngine.
# You do not need to modify this part.
#
CURR_DIR := $(shell pwd)
BUILD_DIR:=$(subst /hostTools, /hostTools,$(CURR_DIR))
BUILD_DIR:=$(word 1, $(BUILD_DIR))
BUILD_ARCH:=$(PROFILE_ARCH)
RESTRICTED_LIBS:= 

# Check if we are in the middle of a consumer release build
CONSUMER_BUILD := $(shell find $(PROFILE_DIR) -maxdepth 1 -name filesystem.tgz)

# Platform dependencies, use different utility to optimize glibc and uclibc
ifneq ($(strip $(BRCM_UCLIBC)),)
  LIBC := uclibc
else
  LIBC := glibc
endif

# Dont search for libs if the 'clean' target is being built
ifneq ($(MAKECMDGOALS),clean)

# List of dirs to search for bins
BIN_INSTALL_DIRS := $(INSTALL_DIR)/bin $(INSTALL_DIR)/sbin $(INSTALL_DIR)/usr

# Get list of all required 32-bit libraries in final image
SHLIBS32 := $(shell find -L $(BIN_INSTALL_DIRS) $(INSTALL_DIR)/lib $(INSTALL_DIR)/usr/lib -type f | file -f - \
             | grep "ELF 32-bit" )

ifneq ($(SHLIBS32),)
SHLIBS32 := $(shell find -L $(BIN_INSTALL_DIRS) $(INSTALL_DIR)/lib $(INSTALL_DIR)/usr/lib -type f | file -f - \
             | grep "ELF 32-bit" | cut -d':' -f1 | xargs $(READELF) -d | grep NEEDED \
	     | cut -d'[' -f2 | sed -e 's/]//g' | sort | uniq)
endif

# Get list of all required 64-bit libraries in final image
SHLIBS64 := $(shell find -L $(INSTALL_DIR) -type f | file -f - \
             | grep "ELF 64-bit" )

ifneq ($(SHLIBS64),)
SHLIBS64 := $(shell find -L $(INSTALL_DIR) -type f | file -f - \
             | grep "ELF 64-bit" | cut -d':' -f1 | xargs $(READELF) -d | grep NEEDED \
	     | cut -d'[' -f2 | sed -e 's/]//g' | sort | uniq)
endif

########## Look for restricted libs #########
RESTRICTED_LIBS32 := $(findstring $(RESTRICTED_LIBS),$(SHLIBS32))
RESTRICTED_LIBS64 := $(findstring $(RESTRICTED_LIBS),$(SHLIBS64))
RESTRICTED_LIBS := $(RESTRICTED_LIBS32) $(RESTRICTED_LIBS64)
ifneq ($(strip $(RESTRICTED_LIBS)),)
ifneq ($(strip $(BUILD_GDBSERVER)),)
  $(warning Restricted library references found: $(RESTRICTED_LIBS)!! Build will continue since GDBSERVER is enabled and this is a debug build!)
else
  $(warning Restricted library references found: $(RESTRICTED_LIBS)!! Build will not continue unless these library references are removed from all apps and libs!)
  $(warning grep for $(RESTRICTED_LIBS) in the $(TARGET_FS) folder to find out which app-lib is the culprit)
endif
endif

############ Process 32-bit libs ############
ifneq ($(SHLIBS32),)

# If processing 32-bit libs in 32-bit build, all libs are in /lib
# If processing 32-bit libs in 64-bit build, all libs are in /lib/$(INSTALL_SUFFIX32)
ifeq ($(BUILD_ARCH),aarch64)
CURRENT_ARCH := arm
else
CURRENT_ARCH := $(BUILD_ARCH)
endif

include $(BUILD_DIR)/make.common

ifeq ($(BUILD_ARCH),aarch64)
INSTALL_SUFFIX32 := $(BCM_INSTALL_SUFFIX)
else
INSTALL_SUFFIX32 :=
endif

$(warning "32-bit: $(ARCH) $(INSTALL_SUFFIX32) $(TOOLCHAIN_TOP)")

ifeq ($(LIBC),glibc)
  # Libs which are loaded via dlopen()
  EXTRALIBS32 := libnss_dns.so.2 libnss_files.so.2
  # Dynamic linker
  ifeq ($(strip $(ARCH)),arm)
      LINKER32 := ld-linux.so.3
  else
      LINKER32 := ld.so.1
  endif
else
  ifeq ($(LIBC),uclibc)
    ifeq ($(strip $(ARCH)),arm)
       LINKER32 := ld-uClibc.so.0
    else
       LINKER32 := ld-uClibc.so.0
    endif
  endif
endif

# Remove dynamic linker from list of libs
SHLIBS32 := $(filter-out $(LINKER32),$(SHLIBS32))

# Add extra libs
SHLIBS32 += $(EXTRALIBS32)

# Remove all shared libs which are already present in INSTALL_DIR/lib
EXP_TOOLCHAIN_LIBS32 := $(shell for j in $(SHLIBS32); do if test -n "$$(find -L $(INSTALL_DIR)/lib/$(INSTALL_SUFFIX32) -name $$j)" \
                               ; then echo ""; else echo $$j; fi; done;)
EXP_TOOLCHAIN_LIBS32 := $(shell for j in $(EXP_TOOLCHAIN_LIBS32); do if test -n "$$(find -L $(INSTALL_DIR)/usr/lib/$(INSTALL_SUFFIX32) -name $$j)" \
                               ; then echo ""; else echo $$j; fi; done;)

# Get full paths for toolchain libs
TOOLCHAIN_LIBS32 := $(shell for j in $(EXP_TOOLCHAIN_LIBS32); do if test -n "$$(find -L $(TOOLCHAIN_TOP) -name $$j)" \
                           ; then find -L $(TOOLCHAIN_TOP) -name $$j -print -quit; else echo ""; fi; done;)

# Determine which libs are missing
MISSING_LIBS32 := $(shell for j in $(EXP_TOOLCHAIN_LIBS32); do if test -n "$$(find -L $(TOOLCHAIN_TOP) -name $$j)" \
                           ; then echo ""; else echo $$j; fi; done;)

# Get full path for dynamic linker only if it doesnt already exist in INSTALL_DIR
ifeq ($(shell find -L $(INSTALL_DIR)/lib/$(INSTALL_SUFFIX32) -maxdepth 1 -type f -name $(LINKER32)),)
TOOLCHAIN_LINKER32 := $(shell if test -n "$$(find -L $(TOOLCHAIN_TOP) -name $(LINKER32))" \
                           ; then find -L $(TOOLCHAIN_TOP) -name $(LINKER32) -print -quit; else echo ""; fi;)

# If linker was not found, add to list of missing libs
ifeq ($(TOOLCHAIN_LINKER32),)
MISSING_LIBS32 := $(MISSING_LIBS32) $(LINKER32)
else
# Add dynamic linker to list of dynamic linkers so that its get copied to /lib
TOOLCHAIN_LINKER += $(TOOLCHAIN_LINKER32)
endif 
endif

endif

############ Process 64-bit libs ############
ifneq ($(SHLIBS64),)

# If processing 64-bit libs in 64-bit build, all libs are in /lib
# If processing 64-bit libs in 32-bit build, all libs are in /lib/$(INSTALL_SUFFIX64)
ifeq ($(BUILD_ARCH),arm)
CURRENT_ARCH := aarch64
else
CURRENT_ARCH := $(BUILD_ARCH)
endif

include $(BUILD_DIR)/make.common

ifeq ($(BUILD_ARCH),arm)
INSTALL_SUFFIX64 := $(BCM_INSTALL_SUFFIX)
else
INSTALL_SUFFIX64 :=
endif

$(warning "64-bit: $(ARCH) $(INSTALL_SUFFIX64) $(TOOLCHAIN_TOP)")

LINKER64 := ld-linux-aarch64.so.1

# Libs which are loaded via dlopen()
EXTRALIBS64 := libnss_dns.so.2 libnss_files.so.2

# Remove dynamic linker from list of libs
SHLIBS64 := $(filter-out $(LINKER64),$(SHLIBS64))

# Add extra libs
SHLIBS64 += $(EXTRALIBS64)

# Remove all shared libs which are already present in INSTALL_DIR/lib
#EXP_TOOLCHAIN_LIBS64 := $(shell for j in $(SHLIBS64); do if test -n "$$(find -L $(INSTALL_DIR)/lib/$(INSTALL_SUFFIX64) -name $$j)" \
                               ; then echo ""; else echo $$j; fi; done;)
EXP_TOOLCHAIN_LIBS64 := $(shell for j in $(EXP_TOOLCHAIN_LIBS64); do if test -n "$$(find -L $(INSTALL_DIR)/usr/lib/$(INSTALL_SUFFIX64) -name $$j)" \
                               ; then echo ""; else echo $$j; fi; done;)

# Get full paths for toolchain libs
TOOLCHAIN_LIBS64 := $(shell for j in $(EXP_TOOLCHAIN_LIBS64); do if test -n "$$(find -L $(TOOLCHAIN_TOP) -name $$j)" \
                           ; then find -L $(TOOLCHAIN_TOP) -name $$j -print -quit; else echo ""; fi; done;)

# Determine which libs are missing
MISSING_LIBS64 := $(shell for j in $(EXP_TOOLCHAIN_LIBS64); do if test -n "$$(find -L $(TOOLCHAIN_TOP) -name $$j)" \
                           ; then echo ""; else echo $$j; fi; done;)

# Get full path for dynamic linker only if it doesnt already exist in INSTALL_DIR
ifeq ($(shell find -L $(INSTALL_DIR)/lib/$(INSTALL_SUFFIX64) -maxdepth 1 -type f -name $(LINKER64)),)
TOOLCHAIN_LINKER64 := $(shell if test -n "$$(find -L $(TOOLCHAIN_TOP) -name $(LINKER64))" \
                           ; then find -L $(TOOLCHAIN_TOP) -name $(LINKER64) -print -quit; else echo ""; fi;)

# If linker was not found, add to list of missing libs
ifeq ($(TOOLCHAIN_LINKER64),)
MISSING_LIBS64 := $(MISSING_LIBS64) $(LINKER64)
else
# Add dynamic linker to list of dynamic linkers so that it gets copied to /lib
TOOLCHAIN_LINKER += $(TOOLCHAIN_LINKER64)   
endif 
endif

endif

############ Handle Missing Libs ############
ifneq ($(MISSING_LIBS32),)
ifneq ($(CONSUMER_BUILD),)
$(warning Consumer Release Build Detected, ignoring missing 32-bit libs: $(MISSING_LIBS32))
else
$(warning All Required 32-bit libraries: $(SHLIBS32))
$(warning 32-bit libraries: $(MISSING_LIBS32))
endif
endif

ifneq ($(MISSING_LIBS64),)
ifneq ($(CONSUMER_BUILD),)
$(warning Consumer Release Build Detected, ignoring missing 64-bit libs: $(MISSING_LIBS64))
else
$(warning All Required 64-bit libraries: $(SHLIBS64))
$(error Missing 64-bit libraries: $(MISSING_LIBS64))
endif
endif

endif 
#MAKECMDGOALS=clean check

############ Targets ############
print_libs:
	@echo ""
	@echo "###################################################"
	@echo "Installing C run-time library..."
	@if [ -n "$(SHLIBS32)" ]; then \
	    echo "######### All 32-bit required shared libs #########"; \
	    echo $(SHLIBS32); \
	fi
	@if [ -n "$(SHLIBS64)" ]; then \
	    echo "######### All 64-bit required shared libs #########"; \
	    echo $(SHLIBS64); \
	fi

all install: print_libs
	@if [ -n "$(TOOLCHAIN_LIBS32)" ]; then \
	    echo "########## 32-bit toolchain libs to copy ##########";\
	    echo $(TOOLCHAIN_LIBS32);\
	fi
	@if [ -n "$(TOOLCHAIN_LIBS64)" ]; then \
	    echo "########## 64-bit toolchain libs to copy ##########";\
	    echo $(TOOLCHAIN_LIBS64);\
	fi
	@if [ -n "$(TOOLCHAIN_LINKER)" ]; then \
	    echo "##########  Dynamic linker libs to copy  ##########";\
	    echo $(TOOLCHAIN_LINKER);\
	fi
	@echo "###################################################"
	@echo ""
	@if [ "$(BUILD_ARCH)" == "aarch64" ]; then \
		if [ -n "$(TOOLCHAIN_LIBS64)" ]; then install $(TOOLCHAIN_LIBS64) $(INSTALL_DIR)/lib; fi; \
		if [ -n "$(TOOLCHAIN_LIBS32)" ]; then \
		    if [ ! -d "$(INSTALL_DIR)/lib/$(INSTALL_SUFFIX32)" ]; then \
			mkdir $(INSTALL_DIR)/lib/$(INSTALL_SUFFIX32); \
		    fi ;\
		    install $(TOOLCHAIN_LIBS32) $(INSTALL_DIR)/lib/$(INSTALL_SUFFIX32); \
		fi ;\
	else \
		if [ -n "$(TOOLCHAIN_LIBS32)" ]; then install $(TOOLCHAIN_LIBS32) $(INSTALL_DIR)/lib; fi; \
		if [ -n "$(TOOLCHAIN_LIBS64)" ]; then \
		    if [ ! -d "$(INSTALL_DIR)/lib/$(INSTALL_SUFFIX64)" ]; then \
			mkdir $(INSTALL_DIR)/lib/$(INSTALL_SUFFIX64); \
		    fi ;\
		    install $(TOOLCHAIN_LIBS64) $(INSTALL_DIR)/lib/$(INSTALL_SUFFIX64); \
		fi ;\
	fi;
	@if [ -n "$(TOOLCHAIN_LINKER)" ]; then install $(TOOLCHAIN_LINKER) $(INSTALL_DIR)/lib; fi
ifneq ($(strip $(BRCM_IKOS)),)
        # Delete libraries not needed by the ikos Linux image. 
	rm -rfv $(INSTALL_DIR)/lib/private $(INSTALL_DIR)/lib/libdl.so.0 $(INSTALL_DIR)/lib/libutil.so.0
endif

clean:
	@if [ "$(INSTALL_DIR)" != "" -a "$(INSTALL_DIR)" != "/" ]; then rm -rf $(INSTALL_DIR)/lib/*; fi
