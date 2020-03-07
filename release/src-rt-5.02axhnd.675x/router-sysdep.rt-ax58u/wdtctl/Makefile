EXE  = wdtctl

OBJS = wdtctl.o


all install : $(EXE)
	mkdir -p $(INSTALL_DIR)/bin
	install -p -m 755 $(EXE) $(INSTALL_DIR)/bin


$(EXE):
	echo "Copying $@"
	-cp -f $(TOP_PLATFORM)/$(shell pwd | sed 's/\(.*router-sysdep\/\)//')/prebuilt/$@ ./


clean: 
	-rm -f *.o *.d $(EXE) $(PREFIX)/bin/$(EXE)


# Set VPATH because we want to compile in a seperate dir than source.
name := $(lastword $(MAKEFILE_LIST))
mdir := $(realpath $(dir $(name)))
VPATH=$(mdir)

# Generate and use dependencies.
CFLAGS += -MD
-include $(OBJS:%.o=%.d)
