include ../common.mak

CFLAGS	+= -Os -Wall $(EXTRACFLAGS) #-mips32
CFLAGS	+= -I$(SRCBASE)/include -I$(TOP)/shared -DRTCONFIG_ISP_METER -DDEBUG
LDFLAGS	= -L$(TOP_PLATFORM)/nvram${BCMEX}${EX7} -lnvram -L$(TOP)/shared -lshared
ifeq ($(HND_ROUTER),y)
LDFLAGS += -L$(TOP_PLATFORM)/wlcsm -lwlcsm
endif

ifeq ($(RTN14U)$(RTAC52U)$(RTAC51U)$(RTN11P),y)
CFLAGS += -DRA_ESW
endif

ifeq ($(RTCONFIG_BCMARM),y)
CFLAGS += -I$(SRCBASE)/shared/bcmwifi/include -DTYPEDEF_FLOAT_T
CFLAGS += -I$(SRCBASE)/common/include
endif

ifeq ($(RTCONFIG_QTN),y)
CFLAGS += -I$(TOP)/libqcsapi_client -I$(TOP)/shared/sysdeps/qtn
LDFLAGS += -L$(TOP)/libqcsapi_client -lqcsapi_client
endif

OBJS = rstats.o

all: rstats

rstats: $(OBJS)
	@echo " [rstats] CC -o $@"
	$(CC) $(LDFLAGS) -o $@ $(OBJS)

	$(SIZECHECK)
	$(CPTMP)

clean:
	rm -f rstats .*.depend
	rm -f *.o

install: all
	@echo " [rstats] Installing..."
	install -D rstats $(INSTALLDIR)/bin/rstats
	$(STRIP) $(INSTALLDIR)/bin/rstats
	chmod 0500 $(INSTALLDIR)/bin/rstats

%.o: %.c .%.depend
	@echo " [rstats] CC $@"
	@$(CC) $(CFLAGS) -c $<

.depend: $(OBJS:%.o=%.c)
	@$(CC) $(CFLAGS) -M $^ > .depend

.%.depend: %.c
	@$(CC) $(CFLAGS) -M $< > $@

-include $(OBJS:%.o=.%.depend)
