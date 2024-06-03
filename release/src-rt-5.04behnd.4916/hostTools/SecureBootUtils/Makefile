def: vars

ifndef TOP_DIR
BUILD_DIR = $(shell pwd)/../../
else
BUILD_DIR = $(TOP_DIR)
endif

include $(BUILD_DIR)/make.common

HOST_PERLARCH_COOKIE := $(BUILD_DIR)/.host_perlarch
HOST_PERLARCH        := $(strip $(shell cat $(HOST_PERLARCH_COOKIE) 2>/dev/null))

HOSTTOOLS_DIR = $(BUILD_DIR)/hostTools
HOSTTOOLS_PERL_LIB = $(HOSTTOOLS_DIR)/PerlLib:$(HOSTTOOLS_DIR)/local_install/Perl/:$(HOSTTOOLS_DIR)/PerlLib/$(HOST_PERLARCH)/

vars :
	echo $(HOSTTOOLS_PERL_LIB)
all: vars

clean:

.PHONY: vars
