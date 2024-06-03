#
# In the old days, when flash memory space was precious, libcreduction tried
# to reduce the size of libc by deleting unused functions in libc.  Now that
# flash memory space is much larger, libcreduction does not modify libc.
#
# Today, "libcreduction" does the following:
# - scan all apps and libs to find out which libs they require.
# - verify that all required libs are in the target filesystem.
# - copy any toolchain libs (e.g. libm.so, librt.so) to target filesystem.
# - if we are on a 32bit userspace, copy any 64bit libs to /usr/lib64 and vice
#   versa.
# - add support for musl (alternate libc implementation).
# 
# Copyright (C) 2004 Broadcom Corporation
#

default: all

#
# Figure out where we are in the build tree so we can set BUILD_DIR so we can
# include make.common.
#
CURR_DIR := $(shell pwd)
BUILD_DIR:=$(subst /hostTools, /hostTools,$(CURR_DIR))
BUILD_DIR:=$(word 1, $(BUILD_DIR))
BUILD_ARCH:=$(USER_ARCH)
RESTRICTED_LIBS:= 

# Check if we are in the middle of a consumer release build
CONSUMER_BUILD := $(shell find $(PROFILE_DIR) -maxdepth 1 -name filesystem.tgz)

ifneq ($(strip $(BRCM_GLIBC)),)
LIBC := glibc
LINKER_PREFIX := ld-linux
else
LIBC := musl
LINKER_PREFIX := ld-musl
endif

# Dont search for libs if the 'clean' target is being built
ifneq ($(MAKECMDGOALS),clean)

# List of dirs to search for apps and libs
# TODO: replace all occurances of INSTALL_DIR with BCM_FSINSTALL_DIR
SEARCH_DIRS := $(BCM_FSINSTALL_DIR)/bin $(BCM_FSINSTALL_DIR)/sbin \
               $(BCM_FSINSTALL_DIR)/lib $(BCM_FSINSTALL_DIR)/usr

# Get list of all 32 bit apps and libs to scan
APPSLIBS32 := $(shell find -L $(SEARCH_DIRS) -type f | file -f - \
             | grep "ELF 32-bit" | cut -d':' -f1)

# Get list of all required 32-bit libraries in final image
ifneq ($(APPSLIBS32),)
SHLIBS32 := $(shell find -L $(SEARCH_DIRS) -type f | file -f - \
             | grep "ELF 32-bit" | cut -d':' -f1 | xargs $(READELF) -d | grep NEEDED \
	     | cut -d'[' -f2 | sed -e 's/]//g' | sort | uniq)
endif

# Get list of all 64 bit apps and libs to scan
APPSLIBS64 := $(shell find -L $(SEARCH_DIRS) -type f | file -f - \
             | grep "ELF 64-bit" | cut -d':' -f1)

# Get list of all required 64-bit libraries in final image
ifneq ($(APPSLIBS64),)
SHLIBS64 := $(shell find -L $(SEARCH_DIRS) -type f | file -f - \
             | grep "ELF 64-bit" | cut -d':' -f1 | xargs $(READELF) -d | grep NEEDED \
	     | cut -d'[' -f2 | sed -e 's/]//g' | sort | uniq)
endif

########## Look for restricted libs #########
RESTRICTED_LIBS32 := $(findstring $(RESTRICTED_LIBS),$(SHLIBS32))
RESTRICTED_LIBS64 := $(findstring $(RESTRICTED_LIBS),$(SHLIBS64))
RESTRICTED_LIBS := $(RESTRICTED_LIBS32) $(RESTRICTED_LIBS64)
ifneq ($(strip $(RESTRICTED_LIBS)),)
ifneq ($(strip $(BUILD_GDBSERVER)),)
  $(warning libcreduction: Restricted library references found: $(RESTRICTED_LIBS)!! Build will continue since GDBSERVER is enabled and this is a debug build!)
else
  $(warning libcreduction: Restricted library references found: $(RESTRICTED_LIBS)!! Build will not continue unless these library references are removed from all apps and libs!)
  $(error grep for $(RESTRICTED_LIBS) in $(BCM_FSINSTALL_DIR) to find out which app-lib is the culprit)
endif
endif

############ Process 32-bit libs ############
ifneq ($(SHLIBS32),)

# If processing 32-bit libs in 32-bit build, all libs are in /lib
# If processing 32-bit libs in 64-bit build, all libs are in /lib/$(INSTALL_SUFFIX32)
ifeq ($(BUILD_ARCH),aarch64)
override CURRENT_ARCH := arm
else
override CURRENT_ARCH := $(BUILD_ARCH)
endif

include $(BUILD_DIR)/make.common

ifeq ($(BUILD_ARCH),aarch64)
INSTALL_SUFFIX32 := $(BCM_INSTALL_SUFFIX)
else
INSTALL_SUFFIX32 :=
endif

$(warning "libcreduction 32-bit: build:$(BUILD_ARCH) current:$(CURRENT_ARCH) install_dir:/lib/$(INSTALL_SUFFIX32) $(TOOLCHAIN_TOP)")

ifeq ($(LIBC),glibc)
   # Libs which are loaded via dlopen()
   EXTRALIBS32 := libnss_dns.so.2 libnss_files.so.2
endif

# Get dynamic linker
TOOLCHAIN_LINKER32 := $(shell find -L $(TOOLCHAIN_TOP) -name "$(LINKER_PREFIX)*" -print -quit;)
LINKER32 := $(shell basename -z $(TOOLCHAIN_LINKER32))

# Remove dynamic linker from list of libs
SHLIBS32 := $(filter-out $(LINKER32),$(SHLIBS32))

# Add extra libs
SHLIBS32 += $(EXTRALIBS32)

# Remove all shared libs which are already present in INSTALL_DIR/lib
# Whatever libs are left must be the toolchain libs.  This logic only works on a clean build.
# On an incremental build, all the toolchain libs have been copied to INSTALL_DIR
# so EXP_TOOLCHAIN_LIBSxx will be empty.
EXP_TOOLCHAIN_LIBS32 := $(shell for j in $(SHLIBS32); do if test -n "$$(find -L $(INSTALL_DIR)/lib/$(INSTALL_SUFFIX32) -name $$j)" \
                               ; then echo ""; else echo $$j; fi; done;)
EXP_TOOLCHAIN_LIBS32 := $(shell for j in $(EXP_TOOLCHAIN_LIBS32); do if test -n "$$(find -L $(INSTALL_DIR)/usr/lib/$(INSTALL_SUFFIX32) -name $$j)" \
                               ; then echo ""; else echo $$j; fi; done;)
ifneq ($(BUILD_PRPL_FEEDS),)
EXP_TOOLCHAIN_LIBS32 := $(shell for j in $(EXP_TOOLCHAIN_LIBS32); do if test -n "$$(find -L $(INSTALL_DIR)/opt/prplos/usr -name $$j)" \
                               ; then echo ""; else echo $$j; fi; done;)
endif
ifneq ($(BUILD_OPENJDK),)
EXP_TOOLCHAIN_LIBS32 := $(shell for j in $(EXP_TOOLCHAIN_LIBS32); do if test -n "$$(find -L $(INSTALL_DIR)/usr/local/jre -name $$j)" \
                               ; then echo ""; else echo $$j; fi; done;)
endif

# Get full paths for toolchain libs
TOOLCHAIN_LIBS32 := $(shell for j in $(EXP_TOOLCHAIN_LIBS32); do find -L $(TOOLCHAIN_TOP) -name $$j -print -quit; done;)

# Determine which libs are missing
MISSING_LIBS32 := $(shell for j in $(EXP_TOOLCHAIN_LIBS32); do if test -n "$$(find -L $(TOOLCHAIN_TOP) -name $$j)" \
                           ; then echo ""; else echo $$j; fi; done;)

# If linker was not found, add to list of missing libs
ifeq ($(TOOLCHAIN_LINKER32),)
MISSING_LIBS32 := $(MISSING_LIBS32) $(LINKER32)
else
# Add dynamic linker to list of dynamic linkers so that its get copied to /lib
TOOLCHAIN_LINKER += $(TOOLCHAIN_LINKER32)
endif 

endif

############ Process 64-bit libs ############
ifneq ($(SHLIBS64),)

# If processing 64-bit libs in 64-bit build, all libs are in /lib
# If processing 64-bit libs in 32-bit build, all libs are in /lib/$(INSTALL_SUFFIX64)
ifeq ($(BUILD_ARCH),arm)
override CURRENT_ARCH := aarch64
else
override CURRENT_ARCH := $(BUILD_ARCH)
endif

include $(BUILD_DIR)/make.common

ifeq ($(BUILD_ARCH),arm)
INSTALL_SUFFIX64 := $(BCM_INSTALL_SUFFIX)
else
INSTALL_SUFFIX64 :=
endif

$(warning "libcreduction 64-bit: build:$(BUILD_ARCH) current:$(CURRENT_ARCH) install_dir:/lib/$(INSTALL_SUFFIX64) $(TOOLCHAIN_TOP)")

ifeq ($(LIBC),glibc)
   # Libs which are loaded via dlopen()
   EXTRALIBS64 := libnss_dns.so.2 libnss_files.so.2
endif

# Get dynamic linker
TOOLCHAIN_LINKER64 := $(shell find -L $(TOOLCHAIN_TOP) -name "$(LINKER_PREFIX)*" -print -quit;)
LINKER64 := $(shell basename -z $(TOOLCHAIN_LINKER64))

# Remove dynamic linker from list of libs
SHLIBS64 := $(filter-out $(LINKER64),$(SHLIBS64))

# Add extra libs
SHLIBS64 += $(EXTRALIBS64)

# Remove all shared libs which are already present in INSTALL_DIR/lib
#EXP_TOOLCHAIN_LIBS64 := $(shell for j in $(SHLIBS64); do if test -n "$$(find -L $(INSTALL_DIR)/lib/$(INSTALL_SUFFIX64) -name $$j)" \
                               ; then echo ""; else echo $$j; fi; done;)
EXP_TOOLCHAIN_LIBS64 := $(shell for j in $(SHLIBS64); do if test -n "$$(find -L $(INSTALL_DIR)/usr/lib -name $$j)" \
                               ; then echo ""; else echo $$j; fi; done;)
EXP_TOOLCHAIN_LIBS64 := $(shell for j in $(EXP_TOOLCHAIN_LIBS64); do if test -n "$$(find -L $(INSTALL_DIR)/usr/lib/$(INSTALL_SUFFIX64) -name $$j)" \
                               ; then echo ""; else echo $$j; fi; done;)

# Get full paths for toolchain libs
TOOLCHAIN_LIBS64 := $(shell for j in $(EXP_TOOLCHAIN_LIBS64); do find -L $(TOOLCHAIN_TOP) -name $$j -print -quit; done;)

# Determine which libs are missing
MISSING_LIBS64 := $(shell for j in $(EXP_TOOLCHAIN_LIBS64); do if test -n "$$(find -L $(TOOLCHAIN_TOP) -name $$j)" \
                           ; then echo ""; else echo $$j; fi; done;)

# If linker was not found, add to list of missing libs
ifeq ($(TOOLCHAIN_LINKER64),)
MISSING_LIBS64 := $(MISSING_LIBS64) $(LINKER64)
else
# Add dynamic linker to list of dynamic linkers so that it gets copied to /lib
TOOLCHAIN_LINKER += $(TOOLCHAIN_LINKER64)   
endif 

endif

############ Handle Missing Libs ############
ifneq ($(MISSING_LIBS32),)
ifneq ($(CONSUMER_BUILD),)
$(warning libcreduction: Consumer Release Build Detected, ignoring missing 32-bit libs: $(MISSING_LIBS32))
else
$(warning libcreduction: All Required 32-bit libraries: $(SHLIBS32))
$(error libcreduction FATAL: Missing 32-bit libraries: $(MISSING_LIBS32))
endif
endif

ifneq ($(MISSING_LIBS64),)
ifneq ($(CONSUMER_BUILD),)
$(warning libcreduction: Consumer Release Build Detected, ignoring missing 64-bit libs: $(MISSING_LIBS64))
else
$(warning libcreduction: All Required 64-bit libraries: $(SHLIBS64))
$(error libcreduction FATAL: Missing 64-bit libraries: $(MISSING_LIBS64))
endif
endif

endif 
#MAKECMDGOALS=clean check

############ Targets ############
print_libs:
	@echo ""
	@echo "###################################################"
	@echo "libcreduction: Installing C run-time libraries..."
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
	    echo "########## 32-bit toolchain libs to copy (this will be empty on incremental builds) ##########";\
	    echo $(TOOLCHAIN_LIBS32);\
	fi
	@if [ -n "$(TOOLCHAIN_LIBS64)" ]; then \
	    echo "########## 64-bit toolchain libs to copy (this will be empty on incremental builds) ##########";\
	    echo $(TOOLCHAIN_LIBS64);\
	fi
	@if [ -n "$(TOOLCHAIN_LINKER)" ]; then \
	    echo "##########  Dynamic linker libs to copy (this will be empty on incremental builds) ##########";\
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
ifeq ($(LIBC),musl)
	@if [ -n "$(LINKER32)" ]; then \
		cd $(INSTALL_DIR)/lib; \
		if [ -n ../lib/libc.so ]; then \
			ln -fs ../lib/libc.so $(LINKER32); \
		fi; \
	fi
	@if [ -n "$(LINKER64)" ]; then \
		cd $(INSTALL_DIR)/lib; \
		if [ -n ../lib64/libc.so ]; then \
			ln -fs ../lib64/libc.so $(LINKER64); \
		fi; \
	fi
else
	@if [ -n "$(TOOLCHAIN_LINKER)" ]; then install $(TOOLCHAIN_LINKER) $(INSTALL_DIR)/lib; fi
endif
ifneq ($(strip $(BRCM_IKOS)),)
        # Delete libraries not needed by the ikos Linux image. 
	rm -rfv $(INSTALL_DIR)/lib/private $(INSTALL_DIR)/lib/libdl.so.0 $(INSTALL_DIR)/lib/libutil.so.0
endif

clean:
	@if [ "$(INSTALL_DIR)" != "" -a "$(INSTALL_DIR)" != "/" ]; then rm -rf $(INSTALL_DIR)/lib/*; fi
