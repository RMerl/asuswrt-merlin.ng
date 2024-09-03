ALL_OBJS    :=  fft_capture_app.o

ifneq ($(CLASSIFIER_DEBUG),)
	SRCBASE =  ../../../../src/
endif
ifeq ($(SRCBASE),)
$(error SRCBASE undefined)
endif

$(info TOOLCHAINPATH=$(TOOLCHAINPATH)$n)
$(info SYSROOT=$(SYSROOT)$n)
$(info GCCLIBPATH=$(GCCLIBPATH)$n)
$(info SRCBASE=$(SRCBASE)$n)
$(info CURDIR=$(CURDIR)$n)
$(info SRCBASE_ROUTER=$(SRCBASE_ROUTER)$n)

#####################
# Search for main/components/router (a EAGLE-or-later build)
#         or main/src/router (a BISON build)
#####################
ifneq ($(wildcard $(BASEDIR)/components/wlioctl)),)
# KUDU
$(info Air-iQ build detected KUDU or later router path)
BRANCH_DEP_INCS := -I$(BASEDIR)/components/wlioctl/include \
                   -I$(BASEDIR)/components/proto/include

else ifneq ($(wildcard $(BASEDIR)/components/shared/devctrl_if)),)
#EAGLE or later
$(info Air-iQ build detected EAGLE or later router path)
BRANCH_DEP_INCS := -I$(BASEDIR)/components/shared \
                   -I$(BASEDIR)/components/shared/devctrl_if \
                   -I$(BASEDIR)/components/shared/proto

else ifneq (,$(findstring main/src/router,$(SRCBASE_ROUTER)))
#BISON
$(info Air-iQ build detected BISON router path)
BRANCH_DEP_INCS := -I$(BASEDIR)/src/common/include \
                   -I$(BASEDIR)/src/common/include/devctrl_if \
                   -I$(BASEDIR)/src/common/include/proto
else
$(error NOT a recognized router build)
endif
#######################

INCDIR			:=	-I$(TOP)/shared \
				-I$(SRCBASE)/include \
				-I$(SRCBASE)/shared/bcmwifi/include \
				$(BRANCH_DEP_INCS)

# WL scanning flags
CCFLAGS += -DBRCM_WL_IOCTL -DLINUX
CCFLAGS += -DWLENT_AIRIQ
CCFLAGS +=  -static -pthread $(INCDIR)
LDFLAGS =  -lpthread -lm
ROUTER_LDFLAGS = -L$(TOP)/shared -lshared -L$(TOP)/nvram -L$(INSTALLDIR)/nvram/usr/lib  -lnvram
LDFLAGS += $(ROUTER_LDFLAGS) -lgcc_s
LDFLAGS += $(USR_LDFLAGS)

ALL_OBJS := fft_capture_app.o kiss_fft.o

APP = fft_capture_app

all: $(APP)

$(APP): $(ALL_OBJS)
	$(CC) $(LDFLAGS) -o $@  $(ALL_OBJS)

$(ALL_OBJS): %.o: %.c
	@echo " "
	@echo "Making $<"
	$(CC) $(CCFLAGS) -c $<

install:
	install -d $(INSTALLDIR)/usr/sbin
	install -m 755 $(APP) $(INSTALLDIR)/usr/sbin

.PHONY: clean all install $(APP) $(ALL_OBJS)
clean:
	-rm -f *.o *~ *.a $(APP)
