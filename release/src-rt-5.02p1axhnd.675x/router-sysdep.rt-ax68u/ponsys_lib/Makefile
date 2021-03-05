LIB = libponsys.so

SAVED = $(LIB)_$(ARCH).shipped

OBJS = ponsys.o

CFLAGS += -Werror -Wfatal-errors

LIB_INSTALL_DIR       := $(BCM_FSBUILD_DIR)/private/lib

all install: $(LIB)
	mkdir -p $(LIB_INSTALL_DIR)
	install -p -t $(LIB_INSTALL_DIR) $(LIB)

clean:
	rm -f *.o *.d $(LIB)
	rm -f $(LIB_INSTALL_DIR)/$(LIB)

# Set VPATH because we want to compile in a seperate dir than source.
name := $(lastword $(MAKEFILE_LIST))
mdir := $(realpath $(dir $(name)))
VPATH=$(mdir)

ifneq ($(wildcard $(mdir)/*.c),)

$(LIB): $(OBJS)
	$(CC) -shared $(BCM_LD_FLAGS) -Wl,--whole-archive,-soname,$(notdir $@) -o $@ $(OBJS) -Wl,--no-whole-archive

else

$(LIB): $(SAVED)
	cat $< > $@

endif

binaryonly_prepare:
	-cp $(LIB) $(SAVED)
	-$(MAKE) -f $(name) clean
	rm -rf $(mdir)/*.c $(mdir)/*.h


# Generate and use dependencies.
CFLAGS += -MD
-include $(OBJS:%.o=%.d)
