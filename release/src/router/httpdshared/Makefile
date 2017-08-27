#***********************************************************************
#
#  Copyright (c) 2006-2007  Broadcom Corporation
#  All Rights Reserved
#
#***********************************************************************

LIB :=  libhttpdshared.so
OBJS := httpdshared.o

#
# Set our CommEngine directory (by splitting the pwd into two words
# at /userspace and taking the first word only).
# Then include the common defines under CommEngine.
# You do not need to modify this part.
#
CURR_DIR := $(shell pwd)
BUILD_DIR:=$(HND_SRC)

all: build_$(LIB)
	@echo "Done Building $@"

install:
	@echo done

.PHONY: all

include $(BUILD_DIR)/make.common

ifeq ($(strip $(BUILD_WEB_SOCKETS)),)
$(info "SKIPPING HTTPDSHARED")
build_$(LIB):
else

$(info "BUILDING build_$(LIB)")

.PHONY: build_$(LIB)

build_$(LIB): sanity_check $(LIB) generic_public_lib_install
	@echo "Done Building $@"

clean: generic_clean
	rm -f $(INSTALL_DIR)/lib/public/$(LIB)

#
# Public libs are only allowed to include header files from the
# public directory.
#
# WARNING: Do not modify this section unless you understand the
# license implications of what you are doing.
#
ALLOWED_INCLUDE_PATHS := -I.\
                         -I$(BUILD_DIR)/userspace/public/include


# treat all warnings as errors
CUSTOM_CFLAGS += -Werror -Wfatal-errors


#
# Implicit rule will make the .c into a .o
# Implicit rule is $(CC) -c $(CPPFLAGS) $(CFLAGS)
# See Section 10.2 of Gnu Make manual
# 
$(LIB): $(OBJS)
	$(CC) -shared $(BCM_LD_FLAGS) -Wl,--whole-archive,-soname,$(notdir $@) -o $@ $(OBJS) -Wl,--no-whole-archive



#
# Include the rule for making dependency files.
# The '-' in front of the second include suppresses
# error messages when make cannot find the .d files.
# It will just regenerate them.
# See Section 4.14 of Gnu Make.
#

include $(BUILD_DIR)/make.deprules

-include $(OBJS:.o=.d)


endif
