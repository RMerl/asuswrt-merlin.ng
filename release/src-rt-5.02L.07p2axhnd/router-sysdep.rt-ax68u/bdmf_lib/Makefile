#
# In most cases, you only need to modify this first section.
#
LIB = libbdmf.so

OBJS = bdmf_user.o

all dynamic install: conditional_build

clean: generic_clean
	rm -f $(INSTALL_DIR)/lib/public/$(LIB)



#
# Set our CommEngine directory (by splitting the pwd into two words
# at /userspace and taking the first word only).
# Then include the common defines under CommEngine.
# You do not need to modify this part.
#
CURR_DIR := $(shell pwd)
BUILD_DIR:=$(HND_SRC)

include $(BUILD_DIR)/make.common

ifneq ($(strip $(BUILD_RDPA)),)
conditional_build: generic_public_lib_install
else
conditional_build:
	@echo "skipping $(LIB) (not configured)"
endif



#
# Implicit rule will make the .c into a .o
# Implicit rule is $(CC) -c $(CPPFLAGS) $(CFLAGS)
# See Section 10.2 of Gnu Make manual
# 
$(LIB):
	cp -f prebuilt/libbdmf.so .



#
# Include the rule for making dependency files.
# The '-' in front of the second include suppresses
# error messages when make cannot find the .d files.
# It will just regenerate them.
# See Section 4.14 of Gnu Make.
#

include $(BUILD_DIR)/make.deprules

