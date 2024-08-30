
# Set VPATH because we want to compile in a seperate dir than source.
name := $(lastword $(MAKEFILE_LIST))
mdir := $(realpath $(dir $(name)))
VPATH=$(mdir)


LIB = libarcher.so

SAVED = $(LIB)_$(ARCH).shipped

OBJS = archer_api.o

BRCM_RELEASETAG ?= "unknown"
CFLAGS += -Werror -Wfatal-errors -DBRCM_RELEASETAG=\"$(BRCM_RELEASETAG)\" -I$(HND_SRC)/userspace/private/include -Wno-stringop-truncation

$(LIB) :

# from Bcmbuild.mk
LIB_INSTALL_DIR       := $(BCM_FSBUILD_DIR)/private/lib

install: $(LIB)
	mkdir -p $(LIB_INSTALL_DIR)
	install -p -t $(LIB_INSTALL_DIR) $(LIB)
	mkdir -p $(INSTALLDIR)/lib
	install -p -m 755 -t $(INSTALLDIR)/lib $(LIB)

clean:
	rm -f *.o *.d $(LIB)
	rm -f $(LIB_INSTALL_DIR)/$(LIB)
	rm -f $(INSTALLDIR)/lib/$(LIB)

ifneq ($(wildcard $(mdir)/*.c),)

$(LIB): $(OBJS)
	$(CC) -shared $(BCM_LD_FLAGS) -Wl,--whole-archive,-soname,$(notdir $@) -o $@ $(OBJS) -Wl,--no-whole-archive

else

ifneq ($(wildcard prebuilt/libarcher.so),)
$(LIB):
	cp prebuilt/libarcher.so libarcher.so
else
$(LIB): $(SAVED)
	cat $< > $@
endif

endif

binaryonly_prepare:
	-cp $(LIB) $(mdir)/$(SAVED)
	-$(MAKE) -f $(name) clean
	rm -rf $(mdir)/*.c $(mdir)/*.h


# Generate and use dependencies.
CFLAGS += -MD
-include $(OBJS:%.o=%.d)
