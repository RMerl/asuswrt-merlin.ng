LIB  := libbcm_boardctl.so

OBJS := 

CFLAGS += -Werror -Wfatal-errors -I$(HND_SRC)/router-sysdep/sys_util/ -I$(HND_SRC)/userspace/public/include/linux/ -I$(HND_SRC)/userspace/public/include/ -I$(HND_SRC)/router-sysdep/bcm_util/

# These are public API headers needed by other apps which want to call this lib.
# Private/internal headers should not be listed here.
HEADERS := bcm_boardctl.h bcm_boarddriverctl.h

# OS abstraction layer
OALDIR := linux

all: $(LIB)

LIB_INSTALL_DIR       := $(BCM_FSBUILD_DIR)/public/lib
HEADER_INSTALL_DIR    := $(BCM_FSBUILD_DIR)/public/include

install: all
	mkdir -p $(LIB_INSTALL_DIR)
	install -t $(LIB_INSTALL_DIR) $(LIB)
	mkdir -p $(INSTALLDIR)/lib
	install -t $(INSTALLDIR)/lib $(LIB)
	mkdir -p $(HEADER_INSTALL_DIR)
	install -p -m 444 -t $(HEADER_INSTALL_DIR) $(addprefix $(mdir)/,$(HEADERS))

clean:
	rm -f $(LIB) $(OBJS)
	rm -f $(OALDIR)/oal.a $(OALDIR)/*.o
	rm -f $(LIB_INSTALL_DIR)/$(LIB)
	rm -f $(addprefix $(HEADER_INSTALL_DIR)/,$(HEADERS))


$(LIB): $(OBJS) force_oal
	$(CC) -shared $(BCM_LD_FLAGS) -Wl,--whole-archive,-soname,$(notdir $@) \
	    -o $@ $(OBJS) $(OALDIR)/oal.a -Wl,--no-whole-archive
	mkdir -p $(HEADER_INSTALL_DIR)
	install -p -m 444 -t $(HEADER_INSTALL_DIR) $(addprefix $(mdir)/,$(HEADERS))

# At this level, we don't know if anything in oaldir has changed, so always go
# down into that dir and do a make.
force_oal:
	mkdir -p $(OALDIR)
	make -C $(OALDIR) -f $(mdir)/$(OALDIR)/Makefile


# Set VPATH because we want to compile in a seperate dir than source.
name := $(lastword $(MAKEFILE_LIST))
mdir := $(realpath $(dir $(name)))
VPATH=$(mdir)

-include $(OBJS:%.o=%.d)
