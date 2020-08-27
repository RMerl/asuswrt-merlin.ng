include ../common.mak

CFLAGS += -Wall -Os $(EXTRACFLAGS)
CFLAGS += -I. -I$(TOP)/shared -I$(SRCBASE)/include -idirafter$(LINUXDIR)/include
#CFLAGS += -ffunction-sections -fdata-sections

ifeq ($(RTCONFIG_BCMARM),y)
CFLAGS += -I$(SRCBASE)/shared/bcmwifi/include -DTYPEDEF_FLOAT_T
CFLAGS += -I$(SRCBASE)/common/include
endif

LDFLAGS	= -L$(TOP_PLATFORM)/nvram${BCMEX}${EX7} -lnvram -L$(TOP)/shared -lshared $(LIBS)

ifeq ($(HND_ROUTER),y)
LDFLAGS += -L$(TOP)/wlcsm$(EXHND) -lwlcsm
endif

ifeq ($(RTCONFIG_BCMARM),y)
LIBS = -lgcc_s
endif

ifeq ($(RTCONFIG_QTN),y)
LDFLAGS += -L$(TOP)/libqcsapi_client -lqcsapi_client
endif

OBJS = cstats.o

all: cstats

cstats: $(OBJS)
	@echo " [cstats] CC -o $@"
	@$(CC) $(LDFLAGS) -o $@ $(OBJS)

	$(SIZECHECK)
	$(CPTMP)

clean:
	rm -f cstats .*.depend
	rm -f *.o

install: all
	@echo " [cstats] Installing..."
#	install -D -m 0500 cstats $(INSTALLDIR)/bin/cstats
	install -D -m 0700 cstats $(INSTALLDIR)/bin/cstats
	$(STRIP) $(INSTALLDIR)/bin/cstats

%.o: %.c .%.depend
	@echo " [cstats] CC $@"
	@$(CC) $(CFLAGS) -c $<

.depend: $(OBJS:%.o=%.c)
	@$(CC) $(CFLAGS) -M $^ > .depend

.%.depend: %.c
	@$(CC) $(CFLAGS) -M $< > $@

-include $(OBJS:%.o=.%.depend)
