LIB = libbcm_util.so

OBJS = bcm_timestamp.o bcm_ulog.o bcm_net.o bcm_fsutils.o \
       bcm_boardutils.o bcm_ethswutils.o bcm_strutils.o

CFLAGS += -Werror -Wfatal-errors -Wno-stringop-truncation -I$(HND_SRC)/router-sysdep/sys_util/ -I$(HND_SRC)/userspace/public/include/linux/ -I$(HND_SRC)/userspace/public/include/
ifneq ($(strip $(BCM4906_504)),)
CFLAGS += -I$(HND_SRC)/userspace/public/include/ -I$(HND_SRC)/bcmdrivers/opensource/include/bcm963xx/ -I$(HND_SRC)/shared/opensource/include/bcm963xx/
endif

# These are public API headers needed by other apps which want to call this lib.
# Private/internal headers should not be listed here.
HEADERS = bcm_ulog.h bcm_timestamp.h bcm_net.h bcm_fsutils.h \
          bcm_boardutils.h bcm_ethswutils.h bcm_strutils.h

all: $(LIB)

LIB_INSTALL_DIR       := $(BCM_FSBUILD_DIR)/public/lib
HEADER_INSTALL_DIR    := $(BCM_FSBUILD_DIR)/public/include

install: $(LIB)
	mkdir -p $(LIB_INSTALL_DIR)
	install -t $(LIB_INSTALL_DIR) $(LIB)
	mkdir -p $(INSTALLDIR)/lib
	install -t $(INSTALLDIR)/lib $(LIB)
	mkdir -p $(HEADER_INSTALL_DIR)
	install -p -m 444 -t $(HEADER_INSTALL_DIR) $(addprefix $(mdir)/,$(HEADERS))

clean:
	rm -f *.o *.d $(LIB)
	rm -f $(LIB_INSTALL_DIR)/$(LIB)
	rm -f $(addprefix $(HEADER_INSTALL_DIR)/,$(HEADERS))

$(LIB): $(OBJS)
	$(CC) -shared $(BCM_LD_FLAGS) -Wl,--whole-archive,-soname,$(notdir $@) -o $@ $(OBJS) -Wl,--no-whole-archive -ldl


# Set VPATH because we want to compile in a seperate dir than source.
name := $(lastword $(MAKEFILE_LIST))
mdir := $(realpath $(dir $(name)))
VPATH=$(mdir)

# Generate and use dependencies.
CFLAGS += -MD
-include $(OBJS:%.o=%.d)
