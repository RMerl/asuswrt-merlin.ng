#
# In most cases, you only need to modify this first section.
#
LIB = oal.a

OBJS = adsl_api.o adsl.o adsl_api_trace.o

all: $(LIB)

clean: generic_clean

ifeq ($(BCM_MODULAR_BUILD),)
# Old way: infer location of make.common based on pwd.
CURR_DIR := $(shell pwd)
BUILD_DIR:=$(HND_SRC)
include $(BUILD_DIR)/make.common
else
# New Modular Build way: EXT_BUILD_DIR must be set.
# Also point BUILD_DIR to EXT_BUILD_DIR
BUILD_DIR := $(EXT_BUILD_DIR)
include $(EXT_BUILD_DIR)/make.common
endif


ALLOWED_INCLUDE_PATHS := -I. \
                         -I$(BCM_FSBUILD_DIR)/include \
                         -I$(BCM_FSBUILD_DIR)/public/include \
                         -I$(BCM_FSBUILD_DIR)/private/include \
                         -I$(BCM_FSBUILD_DIR)/shared/opensource/include/$(BRCM_BOARD) \
                         -I$(BCM_FSBUILD_DIR)/bcmdrivers/include \
                         -I$(BUILD_DIR)/userspace/private/include \
                         -I$(BUILD_DIR)/userspace/private/include/$(OALDIR)


$(LIB) : $(OBJS)
	$(AR) -rcs $@ $(OBJS)


# Generate and use dependencies.
CFLAGS += -MD
-include $(OBJS:%.o=%.d)
