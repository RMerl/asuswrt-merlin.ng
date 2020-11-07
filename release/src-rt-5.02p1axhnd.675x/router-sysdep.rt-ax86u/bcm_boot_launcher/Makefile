EXE := bcm_boot_launcher

OBJS := bcm_boot_launcher.o

LIBS := -lbcm_util -lsys_util -lrt
LIBS += -L$(HND_SRC)/router-sysdep/bcm_util -lbcm_util -L$(HND_SRC)/router-sysdep/sys_util -lsys_util -lrt

CFLAGS += -Werror -Wfatal-errors
CFLAGS += -I$(HND_SRC)/router-sysdep/bcm_util/
CFLAGS += -I$(HND_SRC)/userspace/public/include/linux/


all: $(EXE)

EXE_INSTALL_DIR       := $(BCM_FSBUILD_DIR)/bin

install: all
	mkdir -p $(EXE_INSTALL_DIR)
	install -p -m 755 $(EXE) $(EXE_INSTALL_DIR)
	mkdir -p $(INSTALLDIR)/bin
	install -p -m 755 $(EXE) $(INSTALLDIR)/bin

clean:
	rm -f *.o *.d $(EXE)
	rm -f $(EXE_INSTALL_DIR)/$(EXE)


$(EXE): $(OBJS)
	$(CC) $(BCM_LD_FLAGS) -o $@ $(OBJS) -Wl,-rpath,$(CMS_LIB_RPATH) $(CMS_LIB_PATH) $(LIBS)


# Set VPATH because we want to compile in a seperate dir than source.
name := $(lastword $(MAKEFILE_LIST))
mdir := $(realpath $(dir $(name)))
VPATH=$(mdir)

# Generate and use dependencies.
CFLAGS += -MD
-include $(OBJS:%.o=%.d)
