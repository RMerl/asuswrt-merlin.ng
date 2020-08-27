include ../common.mak

CFLAGS += -I. -I$(TOP)/shared$(RPEX) -I$(SRCBASE)/include
ifneq ($(BUILD_NAME),RP-AC56)
LDFLAGS += -L$(TOP_PLATFORM)/nvram -lnvram
endif
LDFLAGS +=-L$(TOP)/shared$(RPEX) -lshared

ifeq ($(RTCONFIG_AMAS),y)
LDFLAGS += -L$(TOP)/json-c/.libs -ljson-c
endif

EXEC = flash
OBJS = flash_mtd.o

ifneq ($(wildcard ./prebuild),)
all:
	-cp prebuild/$(EXEC) $(EXEC)
else
all: $(EXEC) 
endif

$(EXEC): $(OBJS)
	echo "Build_Name:"$(BUILD_NAME) 
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) $(LDFLAGS) -o $@ $(OBJS)

install: all
	install -D $(EXEC) $(INSTALLDIR)/bin/$(EXEC)
	$(STRIP) $(INSTALLDIR)/bin/$(EXEC)
	chmod 0500 $(INSTALLDIR)/bin/$(EXEC)

clean:
	-rm -f $(EXEC) *.o

