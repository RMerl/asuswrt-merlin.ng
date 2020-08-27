#***********************************************************************
# 
# Makefile
#
#***********************************************************************

CFLAGS += -I$(TOP)/libev-4.33 -I$(TOP)/libnfnetlink-1.0.1/include 
CFLAGS += -I$(TOP)/libmnl-1.0.4/include -I$(TOP)/libnfnetlink-1.0.1/include 
CFLAGS += -I$(TOP)/libnetfilter_conntrack-1.0.7/include -I$(TOP)/sqlite -DSQL -I$(TOP)/json-c
CFLAGS += -I$(TOP)/shared

LDFLAGS += -L$(TOP)/libev-4.33/.libs -lev 
LDFLAGS += -L$(TOP)/libnfnetlink-1.0.1/src/.libs -lnfnetlink 
LDFLAGS += -L$(TOP)/libmnl-1.0.4/src/.libs -lmnl 
LDFLAGS += -L$(TOP)/libnetfilter_conntrack-1.0.7/src/.libs -lnetfilter_conntrack
LDFLAGS += -L$(TOP)/sqlite/.libs -lsqlite3
LDFLAGS += -L$(TOP)/json-c/.libs -ljson-c
LDFLAGS += -lm -lpthread -ldl -lnvram
LDFLAGS += -L$(TOP)/shared -lshared

OBJS = log.o nfct.o nfarp.o nfjs.o nfsql.o nfcm.o

ifeq ($(QCA), y)
  OBJS += nffdb.o
  CFLAGS += -DQCA
  CFLAGS += -I$(TOP)/shared/sysdeps/qca
  CFlAGS += -I$(TOP)/wireless_tools
  CFLAGS += -I$(TOP)/kernel_header/include -I$(SRCBASE)/include
  LDFLAGS += -L$(TOP_PLATFORM)/nvram$(BCMEX)
else ifeq ($(HND), y)
  OBJS += nfmc.o
  CFLAGS += -DHND -I$(HND_SRC)/userspace/private/include 
  CFLAGS += -I$(SRCBASE)/wl/sys
  LDFLAGS += -lethswctl
  ifeq ($(AX), y)
    CFLAGS += -I$(SRCBASE)/../components/proto/include
    CFLAGS += -I$(SRCBASE)/../components/wlioctl/include
    CFLAGS += -I$(SRCBASE)/wl 
    CFLAGS += -I$(SRCBASE)/shared/bcmwifi/include -DTYPEDEF_FLOAT_T
    CFLAGS += -I$(SRCBASE)/common/include

    LDFLAGS += -L$(SRCBASE)/router-sysdep/ethswctl_lib
    LDFLAGS += -L$(TOP_PLATFORM)/nvram$(BCMEX)$(EX7) 
    LDFLAGS += -L$(TOP_PLATFORM)/nvram$(BCMEX)
    LDFLAGS += -L$(TOP_PLATFORM)/wlcsm$(EXHND) -lwlcsm
  else
    CFLAGS += -I$(HND_SRC)/bcmdrivers/opensource/include/bcm963xx
	CFLAGS += -I$(SRCBASE)/include
    CFLAGS += -I$(SRCBASE)/shared/bcmwifi/include -DTYPEDEF_FLOAT_T
    CFLAGS += -I$(SRCBASE)/common/include

    LDFLAGS += -L$(SRCBASE)/router/ethswctl_lib
    LDFLAGS += -L$(TOP_PLATFORM)/nvram$(BCMEX)$(EX7) 
    LDFLAGS += -L$(TOP_PLATFORM)/nvram$(BCMEX)
    LDFLAGS += -L$(TOP_PLATFORM)/wlcsm$(EXHND) -lwlcsm
  endif
else
  OBJS += nfrob.o
  CFLAGS += -I$(SRCBASE)/wl/sys
  CFLAGS += -I$(SRCBASE)/shared/bcmwifi/include/
  CFLAGS += -I$(SRCBASE)/common/include
  CFLAGS += -I$(SRCBASE)/include
  LDFLAGS += -L$(TOP_PLATFORM)/nvram$(BCMEX)$(EX7) 
  LDFLAGS += -L$(TOP_PLATFORM)/nvram$(BCMEX)
endif

all: nfcm

.c.o:
	$(CC) -c $*.c $(CFLAGS)

nfcm: $(OBJS)
#	@echo LDFLAGS=$(LDFLAGS)
#	@echo "=============================";
#	@echo TOP=$(TOP)
#	@echo TOP_PLATFORM=$(TOP_PLATFORM)
#	@echo SRCBASE=$(SRCBASE)
#	@echo HND_SRC=$(HND_SRC)
#	@echo BCMEX=${BCMEX}
#	@echo EX7=$(EX7)
#	@echo HND=$(HND)
#	@echo AX=$(AX)
#	@echo QCA=$(QCA)
#	@echo "=============================";
	$(CC) -o nfcm $(OBJS) $(LDFLAGS)

.PHONY: clean
clean:
#	@echo OBJS=$(OBJS)
	-rm -rf nfcm stamp-h1 $(OBJS)

