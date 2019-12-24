include ../common.mak

CFLAGS  += -Os -Wall $(EXTRACFLAGS) -I$(SRCBASE)/include -I$(TOP)/shared
ifeq ($(RTCONFIG_BCMARM),y)
CFLAGS += -I$(SRCBASE)/shared/bcmwifi/include -DTYPEDEF_FLOAT_T
CFLAGS += -I$(SRCBASE)/common/include
endif
LDFLAGS = -L$(TOP_PLATFORM)/nvram${BCMEX}${EX7} -lnvram -L$(TOP)/shared -lshared
ifeq ($(HND_ROUTER),y)
LDFLAGS += -L$(TOP_PLATFORM)/wlcsm -lwlcsm
endif
ifeq ($(RTCONFIG_QTN),y)
CFLAGS += -I$(TOP)/libqcsapi_client
LDFLAGS += -L$(TOP)/libqcsapi_client -lqcsapi_client
endif

PACKAGE=faketc
OBJS=$(PACKAGE).o

all: $(PACKAGE)

$(PACKAGE): $(PACKAGE).o $(LIBS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJS) -o $(PACKAGE) $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $<

install: all
	$(STRIP) $(PACKAGE)
	install -D $(PACKAGE) $(INSTALLDIR)/usr/sbin/$(PACKAGE)

clean:
	rm -f *.o $(PACKAGE)
