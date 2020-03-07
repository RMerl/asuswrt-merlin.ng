LIB := libgen_util.so

OBJS := hexbinary.o \
        crc.o \
        base64encode.o base64decode.o

CFLAGS += -Werror -Wfatal-errors -I$(HND_SRC)/userspace/public/include/linux/

# These are public API headers needed by other apps which want to call this lib.
# Private/internal headers should not be listed here.
HEADERS := genutil_base64.h genutil_crc.h genutil_hexbinary.h

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
	$(CC) -shared $(BCM_LD_FLAGS) -Wl,--whole-archive,-soname,$(notdir $@) -o $@ $(OBJS) -Wl,--no-whole-archive


# Set VPATH because we want to compile in a seperate dir than source.
name := $(lastword $(MAKEFILE_LIST))
mdir := $(realpath $(dir $(name)))
VPATH=$(mdir)

# Generate and use dependencies.
CFLAGS += -MD
-include $(OBJS:%.o=%.d)
