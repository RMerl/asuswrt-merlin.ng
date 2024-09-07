LIB := libmdm2.so

SAVED := $(LIB)_$(ARCH).shipped

all: $(LIB)


# Set VPATH so we can compile in a seperate dir than source.
name := $(lastword $(MAKEFILE_LIST))
mdir := $(realpath $(dir $(name)))
VPATH=$(mdir)


ifneq ($(wildcard $(mdir)/Makefile.fullsrc),)

include $(mdir)/Makefile.fullsrc

else

$(LIB): $(SAVED)
	cat $< > $@

save_binaries:

delete_source:

endif


install: all
	mkdir -p $(LIB_INSTALL_DIR)
	install -p -m 755 -t $(LIB_INSTALL_DIR) $(LIB)


clean:
	rm -f *.d *.o $(LIB)
	rm -f $(LIB_INSTALL_DIR)/$(LIB)
