
name := $(lastword $(MAKEFILE_LIST))
mdir := $(realpath $(dir $(name)))
VPATH=$(mdir)

CFLAGS += -I$(HND_SRC)/userspace/public/include/
CFLAGS += -I$(HND_SRC)/userspace/public/include/linux/
CFLAGS += -I$(HND_SRC)/router-sysdep/bcm_util/
CFLAGS += -I$(HND_SRC)/router-sysdep/bcm_boardctl/
CFLAGS += -I$(HND_SRC)/bcmdrivers/broadcom/include/bcm963xx/
CFLAGS += -I$(HND_SRC)/userspace/private/include/linux/
CFLAGS += -I$(HND_SRC)/userspace/private/include/

LIB = libatmctl.so

SAVED = $(LIB)_$(ARCH).shipped

OBJS = 

$(LIB) :

install : $(LIB)
	# mkdir -p $(PREFIX)/lib/ 
	# install $(LIB) $(PREFIX)/lib/ 
	# mkdir -p $(PREFIX)/include/ 
	# install $(LIB) $(PREFIX)/include/ 
	mkdir -p $(INSTALL_DIR)/lib$(BCM_INSTALL_SUFFIX_DIR)
	install -m 755 $(LIB) $(INSTALL_DIR)/lib$(BCM_INSTALL_SUFFIX_DIR)

clean: 
	rm -f *.o *.d $(LIB) $(PREFIX)/lib/$(LIB)
	-$(MAKE) -C $(OALDIR) -f $(mdir)/$(OALDIR)/Makefile clean

ifneq ($(wildcard $(mdir)*.c)$(wildcard $(mdir)/$(OALDIR)/*.c),)

$(LIB): $(OBJS)
	mkdir -p $(OALDIR)
	$(MAKE) -C $(OALDIR) -f $(mdir)/$(OALDIR)/Makefile
	$(CC) -shared $(BCM_LD_FLAGS) -Wl,--whole-archive,-soname,$(notdir $@) -o $@ $(OBJS) $(OALDIR)/oal.a -Wl,--no-whole-archive

else

$(LIB): $(SAVED)
	cat $< > $@

endif

binaryonly_prepare:
	-cp $(LIB) $(SAVED)
	-$(MAKE) -f $(name) clean
	rm -rf $(mdir)/*.c $(mdir)/*.h $(mdir)/$(OALDIR)


# Generate and use dependencies.
CFLAGS += -MD
-include $(OBJS:%.o=%.d)
