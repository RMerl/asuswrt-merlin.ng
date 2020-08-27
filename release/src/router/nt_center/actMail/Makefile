include ../Makefile.config

ifeq ($(ASUSWRT_SDK),y)

MAKE = make
CFLAGS = -Wall -O2 -g  $(EXTRACFLAGS)
CFLAGS += -I$(SRCBASE) -I$(SRCBASE)/include -I. -I$(TOP)/nt_center/lib/
CFLAGS += -I$(TOP)/sqlite 
CFLAGS += -I$(TOP)/shared #Just For rtconfig.h , do not include share library.
CFLAGS += -I$(TOP)/json-c
CFLAGS += -DASUSWRT_SDK

ifeq ($(HND_ROUTER),y)
CFLAGS += -DNON_P_THREAD
endif

LDFLAGS = -L$(TOP)/nt_center/lib/ -lnt
LDFLAGS += -L$(TOP)/sqlite/.libs -lsqlite3 -lm
LDFLAGS += -L$(TOP)/json-c/.libs -ljson-c
#LDFLAGS += -L$(TOP)/nvram$(BCMEX)$(EX7) -lnvram
LIBS = -lpthread -lgcc_s

ifeq ($(wildcard $(SRCBASE)/router/nt_center/actMail/*.c),)
all: 
	-cp -f prebuild/$(BUILD_NAME)/nt_actMail nt_actMail
else
all: nt_actMail
endif

nt_actMail: nt_actMail.o

nt_actMail:
	@$(CC) ${CFLAGS} -o $@ $^ ${LDFLAGS} $(LIBS)


.PHONY : all clean
clean:
	rm -rf *.o nt_actMail

else # DSL_ASUSWRT_SDK

CC = mips-linux-uclibc-gcc
MAKE = make 

CFLAGS = -Wall -O2 -g 
CFLAGS += $(TC_CFLAGS) -I.
CFLAGS += -I$(APP_NOTIFICATION_CENTER_DIR)/lib/
CFLAGS += -I$(APP_SQLITE_DIR)/

LDFLAGS = -lm
LDFLAGS += -L$(APP_NOTIFICATION_CENTER_DIR)/lib/ -lnt -lpthread
LDFLAGS += -L$(APP_SQLITE_DIR)/.libs -lsqlite3
LIBS =

all: nt_actMail

nt_actMail: nt_actMail.o

nt_actMail:
	@$(CC) ${CFLAGS} -o $@ $^ ${LDFLAGS} $(LIBS)


.PHONY : all clean
clean:
	rm -rf *.o nt_actMail
endif
