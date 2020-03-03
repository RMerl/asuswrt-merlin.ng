ALL_OBJS    :=  airiq_app.o

# Optional make variables:
# AIRIQ_APP_LOCAL: If set link against libairiq.so
# else link against libairiq_sirp.so

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

INCDIR   	:=	-I$(TOP)/shared \
				-I$(SRCBASE)/include \
				-I$(SRCBASE)/shared/bcmwifi/include \
				-I$(AIRIQ_COMPONENTDIR)/include \
				$(BRANCH_DEP_INCS)

# WL scanning flags
ifeq ($(CLASSIFIER_DEBUG),)
CCFLAGS += -DAIRIQ_APP_WL -DBRCM_WL_IOCTL -DLINUX

ifeq ($(AIRIQ_APP_LOCAL),)
CCFLAGS += -DAIRIQ_APP_SIRP
else
CCFLAGS += -DAIRIQ_APP_LOCAL
endif

CCFLAGS += -DWLENT_AIRIQ
CCFLAGS +=  -static -pthread $(INCDIR)
LDFLAGS = -L$(AIRIQ_COMPONENTDIR)/prebuilt/$(PLATFORM) -lpthread
ROUTER_LDFLAGS = -L$(TOP)/shared -lshared -L$(TOP)/nvram -L$(INSTALLDIR)/nvram/usr/lib  -lnvram
LDFLAGS += $(ROUTER_LDFLAGS) -lgcc_s
LDFLAGS += $(USR_LDFLAGS)

ifeq ($(AIRIQ_APP_LOCAL),)
AIRIQLIB = -lairiq_sirp
else
AIRIQLIB = -lairiq
endif

APP = airiq_app

all: $(APP)

$(APP): $(ALL_OBJS)
	$(CC) $(LDFLAGS) -o $@ $(ALL_OBJS) $(AIRIQLIB)
#	$(STRIP) $@
else
CCFLAGS += -DAIRIQ_APP_SIRP $(INCDIR) -m32 -DCLASSIFIER_DEBUG=1 -DLINUX -ggdb
LDFLAGS =  -L..  $(STDLIBDIR) -m32  -pthread
AIRIQLIB = -lc -lairiq_sirp
APP = classifier_debug
ALL_OBJS := airiq_app.o

all: $(APP)

$(APP): $(ALL_OBJS)
	$(CC) $(LDFLAGS) -o $@  $(AIRIQLIB) $(ALL_OBJS)
endif

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
