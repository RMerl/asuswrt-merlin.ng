#***********************************************************************
#
# Makefile
#
#***********************************************************************
include ../common.mak

#C = $(CC)
#TOP = $(TOP)
CFLAGS += -g -O2 -I$(TOP)/curl-7.21.7/include  -I$(TOP)/libxml2/include  -I$(TOP)/openssl/include
CFLAGS += -I$(SRCBASE) -I$(SRCBASE)/include -I. -I$(TOP)/shared
#CFLAGS = -g -rdynamic
CFLAGS += -D_FILE_OFFSET_BITS=64
LDFLAGS =  -L$(TOP)/curl-7.21.7/lib/.libs  -L$(TOP)/libxml2/.libs  -L$(TOP)/openssl -L$(TOP_PLATFORM)/nvram${BCMEX}${EX7} -L$(TOP)/shared -lshared
ifeq ($(HND_ROUTER),y)
LDFLAGS += -L$(TOP_PLATFORM)/wlcsm$(EXHND) -lwlcsm -lgcc_s
endif

LIBS = -lcurl -lxml2 -lssl -lcrypto -lpthread -ldl -lnvram -lm

ifeq ($(RTCONFIG_BCMARM),y)
CFLAGS += -DTYPEDEF_FLOAT_T
endif

ifeq ($(RTCONFIG_QTN),y)
LDFLAGS += -L$(TOP)/libqcsapi_client -lqcsapi_client
endif

ifeq ($(wildcard $(TOP)/asuswebstorage/*.c),)
all:
	-cp -f $(TOP)/asuswebstorage/prebuild/$(BUILD_NAME)/asuswebstorage asuswebstorage
else
all: asuswebstorage
endif

asuswebstorage: asuswebstorage.o api.o function.o list.o mem_pool.o
	$(CC) $(CFLAGS) $(LDFLAGS) $(LIBS)  $^ -o $@ 


.PHONY: clean
clean:
	-rm -rf *.o asuswebstorage
