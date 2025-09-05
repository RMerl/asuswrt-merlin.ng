include ../common.mak

OBJ = main.o download.o shared_func.o protect_name.o verification.o
LIB_OBJ = feature_def.o signature.o utility.o protect_name.o libasd.o misc.o monitorproc.o shared_func.o chknvram.o blockfile.o event_queue.o

CFLAGS += $(if $(RTCONFIG_DEBUG),-g) -idirafter$(LINUXDIR)/include -I$(TOP)/shared  -I$(SRCBASE)/include
CFLAGS += -I$(TOP)/curl/include
CFLAGS += -I$(TOP)/openssl/include
CFLAGS += -I$(TOP)/libasc
ifeq ($(RTCONFIG_BCMARM),y)
CFLAGS += -I$(SRCBASE)/shared/bcmwifi/include
endif
CFLAGS += -I$(TOP)/json-c

ifeq ($(HND_ROUTER_BE_4916), y)
CFLAGS+= -fcommon
endif

LIB_CFLAGS = $(CFLAGS) -shared -DASD_LIB

ifeq ($(RTCONFIG_REALTEK),y)
LDFLAGS = -L$(TOP)/nvram${BCMEX} -lnvram -L$(TOP)/shared -lshared -lcrypt -ldl
else
LDFLAGS = -L$(TOP)/nvram${BCMEX}${EX7} -lnvram -L$(TOP)/shared -lshared -lcrypt -ldl
endif

ifeq ($(HND_ROUTER),y)
LDFLAGS += -L$(TOP)/wlcsm -lwlcsm
endif
ifeq ($(HND_ROUTER_AX),y)
LDFLAGS += -L$(TOP_PLATFORM)/nvram -lnvram
LDFLAGS += -L$(TOP_PLATFORM)/wlcsm -lwlcsm
endif

LDFLAGS += -lpthread -lm
LDFLAGS += $(LIBS) -L$(TOP)/shared -lshared -L$(TOP)/nvram -lnvram
ifeq ($(RTCONFIG_BCM_502L07P2),y)
LDFLAGS += -L$(TOP_PLATFORM)/ethctl_lib -lethctl
endif
LDFLAGS += -L$(TOP)/curl/lib/.libs -lcurl -L$(TOP)/
LDFLAGS += -L$(TOP)/openssl -lcrypto -lssl
LDFLAGS += -L$(TOP)/libasc -lasc
LDFLAGS += -L$(TOP)/json-c/.libs -ljson-c
ifeq ($(RTCONFIG_TR069),y)
LDFLAGS += -L$(TOP)/libssh2/src/.libs -lssh2
LDFLAGS += -L$(TOP)/zlib -lz
endif

ifeq ($(RTCONFIG_TRUSTZONE),y)
LDFLAGS += -L$(TOP)/ateed/ -latee
endif

TARGET = asd libasd.so


ifeq ($(wildcard $(SRCBASE)/router/asd2.1/*.c),)
all:
	-cp -f prebuild/$(BUILD_NAME)/asd ./
	-cp -f prebuild/$(BUILD_NAME)/libasd.so ./
else
all: $(TARGET)
endif

%.o: %.h
%.o: %.c
	$(CC) $(CFLAGS) -fPIC -c $<

libasd.so: $(LIB_OBJ)
	$(CC) -o $@ $(LIB_OBJ) $(LIB_CFLAGS) $(LIBS) $(LDFLAGS)

asd: $(OBJ)
	$(CC) -o $@ $(OBJ) $(CFLAGS) $(LIBS) $(LDFLAGS)

install:

clean:
	-rm *.o $(TARGET)

.PHONY: all clean



