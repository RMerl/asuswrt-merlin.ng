
name := $(lastword $(MAKEFILE_LIST))
mdir := $(realpath $(dir $(name)))
VPATH=$(mdir) $(mdir)/3rdparty/hmac_sha


EXE  = mmc 

OBJS = mmc.o mmc_cmds.o lsmmc.o hmac_sha2.o sha2.o

SAVED = mmc.o_saved


all install : $(EXE)
	mkdir -p $(INSTALL_DIR)/bin;  \
	install -m 755 $(EXE) $(INSTALL_DIR)/bin;


ifneq ($(wildcard $(mdir)/*.c),)
$(EXE): $(OBJS)
	$(CC) $(BCM_LD_FLAGS) -o $@ $(OBJS) -Wl,-rpath,$(LIBS)

else
$(EXE): $(SAVED)
	cat $< > $@

endif

clean: 
	-rm -f *.o *.d $(EXE) $(PREFIX)/bin/$(EXE)

binaryonly_prepare:
	-cp $(EXE) $(SAVED)
	-$(MAKE) -f $(name) clean
	rm -rf $(mdir)/*.c $(mdir)/*.h 


# Generate and use dependencies.
CFLAGS += -MD
-include $(OBJS:%.o=%.d)
