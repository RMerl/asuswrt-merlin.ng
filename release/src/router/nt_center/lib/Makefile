include ../Makefile.config

ifeq ($(ASUSWRT_SDK),y)
include $(SRCBASE)/router/.config

CFLAGS = -Os -Wall $(EXTRACFLAGS) -fPIC
CFLAGS += -I. -I$(TOP)/nt_center/
CFLAGS += -I$(TOP)/sqlite 
CFLAGS += -I$(TOP)/shared #Just For rtconfig.h , do not include share library.
CFLAGS += -DASUSWRT_SDK

LDFLAGS += -L$(TOP)/sqlite/.libs -lsqlite3

ifeq ($(RTCONFIG_QCA),y)
LDFLAGS += -L$(LIBDIR)
EXTRACC += -ldl
endif
LDFLAGS += -lm
EXTRACC += -lpthread #Depend on sqlite
EXTRALD += -lpthread #Depend on sqlite

LD_BINARY  += $(LDFLAGS) $(EXTRACC)
LD_LIBRARY += $(LDFLAGS) $(EXTRALD)

OBJS = nt_share.o nt_utility.o linklist.o hashtable.o nt_db_stat.o

vpath %.c $(SRCBASE)/nt_center/

ifeq ($(wildcard $(SRCBASE)/router/nt_center/lib/*.c),)
all: 
	-cp -f prebuild/libnt.so libnt.so
	-cp -f prebuild/nt_db nt_db
else
all: libnt.so libnt.a nt_db
endif

libnt.so: $(OBJS)
	@echo " [$@] LD $@"
	@$(LD) $(LD_LIBRARY) -shared -o $@ $^
	@$(SIZECHECK)
	@$(CPTMP)

libnt.a: $(OBJS)
	@echo " [$@] AR $@"
	@rm -f $@
	@$(AR) cruv $@ $^

nt_db: nt_db.o nt_utility.o nt_db_stat.o linklist.o
	@echo " [$@] CC $@"
	@$(CC) -o $@ $^ $(LD_BINARY) -lm
	@$(STRIP) $@

nt_db_test: nt_db_test.o nt_utility.o nt_db_stat.o linklist.o nt_share.o
	@echo " [$@] CC $@"
	@$(CC) -o $@ $^ $(LD_BINARY)
	@$(STRIP) $@

clean:
	rm -f *.o *.so *.a .*.depend nt_db nt_db_test

%.o: %.c .%.depend
	@echo " [libnt] CC $@ $(CFLAGS)"
	@$(CC) $(CFLAGS) -o $@ -c $<

.depend: $(OBJS:%.o=%.c)
	@$(CC) $(CFLAGS) -M $^ > .depend

.%.depend: %.c
	@$(CC) $(CFLAGS) -M $< > $@

#-include $(OBJS:%.o=.%.depend)

else # DSL_ASUSWRT_SDK

CFLAGS = -Os -Wall $(EXTRACFLAGS) -fPIC
CFLAGS += -I. $(TC_CFLAGS)
CFLAGS += -I$(APP_NOTIFICATION_CENTER_DIR)
CFLAGS += -I$(APP_SQLITE_DIR)

LDFLAGS += -lm
LDFLAGS += -L$(APP_SQLITE_DIR)/.libs -lsqlite3

OBJS = nt_share.o nt_utility.o linklist.o hashtable.o nt_db_stat.o

all: libnt.so libnt.a nt_db

libnt.so: $(OBJS)
	@echo " [$@] LD $@"
	@$(CC) -shared $(CFLAGS) -o $@ $^
	@$(STRIP) $@

libnt.a: $(OBJS)
	@echo " [$@] AR $@"
	@rm -f $@
	@$(AR) cruv $@ $^

nt_db: nt_db.o nt_utility.o nt_db_stat.o linklist.o
	@echo " [$@] CC $@"
	@$(CC) -o $@ $^ $(LDFLAGS)
	@$(STRIP) $@

nt_db_test: nt_db_test.o nt_utility.o nt_db_stat.o linklist.o nt_share.o
	@echo " [$@] CC $@"
	@$(CC) -o $@ $^ $(LDFLAGS)
	@$(STRIP) $@

clean:
	rm -f *.o *.so *.a .*.depend nt_db nt_db_test

%.o: %.c .%.depend
	@echo " [$@] CC $@ $(CFLAGS)"
	@$(CC) $(CFLAGS) -o $@ -c $<

.depend: $(OBJS:%.o=%.c)
	@$(CC) $(CFLAGS) -M $^ > .depend

.%.depend: %.c
	@$(CC) $(CFLAGS) -M $< > $@

-include $(OBJS:%.o=.%.depend)

endif
