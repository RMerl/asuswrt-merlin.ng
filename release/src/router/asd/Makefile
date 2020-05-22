include ../common.mak
include $(SRCBASE)/.config

OBJ = main.o feature_def.o signature.o utility.o protect_name.o
OBJ += blockfile.o chknvram.o misc.o

CFLAGS += -idirafter$(LINUXDIR)/include -I$(TOP)/shared  -I$(SRCBASE)/include
CFLAGS += -I$(TOP)/curl-7.21.7/include
CFLAGS += -I$(TOP)/openssl/include
CFLAGS += -I$(TOP)/libasc
ifeq ($(RTCONFIG_BCMARM),y)
CFLAGS += -I$(SRCBASE)/shared/bcmwifi/include
endif
CFLAGS += -I$(TOP)/json-c

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
LDFLAGS += -L$(TOP)/curl-7.21.7/lib/.libs -lcurl -L$(TOP)/
LDFLAGS += -L$(TOP)/openssl -lcrypto -lssl
LDFLAGS += -L$(TOP)/libasc -lasc
LDFLAGS += -L$(TOP)/json-c/.libs -ljson-c

TARGET = asd


ifeq ($(wildcard $(SRCBASE)/router/asd/*.c),)
all:
	-cp -f prebuild/$(BUILD_NAME)/asd ./
else
all: $(TARGET)
endif

%.o: %.h
%.o: %.c
	$(CC) $(CFLAGS) -fPIC -c $<

$(TARGET): $(OBJ)
	$(CC) -o $@ $(OBJ) $(CFLAGS) $(LIBS) $(LDFLAGS)

install:

clean:
	-rm *.o $(TARGET)

.PHONY: all clean



