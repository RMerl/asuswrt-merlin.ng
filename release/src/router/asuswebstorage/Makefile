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
ifeq ($(RTCONFIG_BCM_502L07P2),y)
LDFLAGS += -L$(TOP_PLATFORM)/ethctl_lib -lethctl
endif

ifeq ($(RTCONFIG_TR069),y)
CFLAGS += -I$(TOP)/libssh2/include -I$(TOP)/zlib/include
LDFLAGS += -L$(TOP)/libssh2/src/.libs -lssh2 -L$(TOP)/zlib -lz
endif

ifeq ($(HND_ROUTER),y)
LDFLAGS += -L$(TOP_PLATFORM)/wlcsm$(EXHND) -lwlcsm -lgcc_s
endif

LIBS = -lcurl -lxml2 -lssl -lcrypto -lpthread -ldl -lnvram -lm

ifeq ($(RTCONFIG_BCMARM),y)
CFLAGS += -DTYPEDEF_FLOAT_T
endif

ifeq ($(HND_ROUTER_BE_4916), y)
CFLAGS+= -fcommon
endif

ifeq ($(RTCONFIG_QTN),y)
LDFLAGS += -L$(TOP)/libqcsapi_client -lqcsapi_client
endif

ifeq ($(RTCONFIG_TRUSTZONE),y)
LDFLAGS += -L$(TOP)/ateed/ -latee
endif

ifeq ($(BUILD_NAME),$(filter $(BUILD_NAME),RT-BE82M GS-BE18000 GS-BE12000 GS7_PRO GT7))
LDFLAGS += -L$(TOP)/libhostapi -lhostapi
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
