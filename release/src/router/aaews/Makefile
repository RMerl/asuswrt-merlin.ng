TARGET=aaews
WSLIB=libws.so
DOG=mastiff

include ../common.mak
include $(TOP)/.config

CURL_DIR=$(TOP)/curl-7.21.7
XML2_DIR=$(TOP)/libxml2
OPENSSL_DIR=$(TOP)/openssl
NVRAM_DIR=$(TOP_PLATFORM)/nvram$(BCMEX)$(EX7)
JSON_C_DIR=$(TOP)/json-c
SHARED_DIR=$(TOP)/shared
WB_DIR=$(TOP)/wb
QCSAPI_DIR=$(TOP)/libqcsapi_client
NT_CENTER_DIR=$(TOP)/nt_center
ifeq ($(RTCONFIG_SW_HW_AUTH),y)
SW_HW_AUTH_DIR=$(TOP)/sw-hw-auth
endif
SQLITE_DIR=$(TOP)/sqlite

NATNL_DIR=$(TOP)/asusnatnl/natnl
ASUSNATNL_CFLAGS=-I$(NATNL_DIR)
ASUSNATNL_LDFLAGS=-L$(NATNL_DIR)
ASUSNATNL_LIBS=-lasusnatnl

LIBC_DIR=$(NATNL_DIR)/../
WS_SRC=$(WB_DIR)/ws_src
OUTPUT := $(WB_DIR)/output

SRC=$(wildcard *.c)
OBJS=$(patsubst %.c, %.o, $(SRC))
OBJS_MAIN=$(filter-out $(DOG).o, $(OBJS))
WSLIB_OBJS := $(WS_SOURCES:%.c=$(OUTPUT)/%.o)

ifeq ($(wildcard $(TOP)/aaews/*.c),)
all:
	-cp -f $(TOP)/aaews/prebuild/$(BUILD_NAME)/aaews aaews
	-cp -f $(TOP)/aaews/prebuild/$(BUILD_NAME)/mastiff mastiff
else
all: $(WSLIB) $(TARGET)
endif


ifeq ("$(MAKECMDGOALS)", $(filter "$(MAKECMDGOALS)", "router" "mastiff"))
ROUTER_CFLAGS=-DNVRAM=1 -DEMBEDDED -I$(SHARED_DIR)
ROUTER_CFLAGS+=-DTNL_CALLBACK_ENABLE -DTNL_2_X -DUSE_IM_IPC
ifeq ($(RTCONFIG_QTN),y)
ROUTER_LIBS=-lnvram -lshared -lqcsapi_client
ROUTER_LDFLAGS=-L$(NVRAM_DIR) -L$(SHARED_DIR) -L$(QCSAPI_DIR)
else
ROUTER_LIBS=-lnvram -lshared
ROUTER_LDFLAGS=-L$(NVRAM_DIR) -L$(SHARED_DIR)
endif
ifeq ($(RTCONFIG_NOTIFICATION_CENTER),y)
ROUTER_LIBS += -lsqlite3
ROUTER_LDFLAGS += -L$(SQLITE_DIR)/.libs
ROUTER_LIBS += -lnt
ROUTER_LDFLAGS += -L$(NT_CENTER_DIR)/lib
endif
endif

CFLAGS += -g
ifeq ($(RTCONFIG_NOTIFICATION_CENTER), y)
CFLAGS += -DRTCONFIG_NOTIFICATION_CENTER=y
endif
ifeq ($(RTCONFIG_IFTTT), y)
CFLAGS += -DRTCONFIG_IFTTT=y
endif
ifeq ($(RTCONFIG_ALEXA), y)
CFLAGS += -DRTCONFIG_ALEXA=y
endif
CFLAGS += -I.
CFLAGS += -I$(CURL_DIR)/include
CFLAGS += -I$(OPENSSL_DIR)/include
CFLAGS += -I$(XML2_DIR)/include
CFLAGS += -I$(JSON_C_DIR)
CFLAGS += -I$(WS_SRC)
CFLAGS += -I$(NVRAM_DIR)
CFLAGS += -I$(WB_DIR)
CFLAGS += -I$(NT_CENTER_DIR)/lib
CFLAGS += -I$(SRCBASE)/include
CFLAGS += $(ASUSNATNL_CFLAGS)
CFLAGS += $(AICAM_CFLAGS)
CFLAGS += $(ROUTER_CFLAGS)
CFLAGS += -DPTHREAD_STACK_SIZE=0x51200

LIBS = -lcurl -lssl -lcrypto -lxml2 -ljson-c -ldl -pthread -lrt $(ROUTER_LIBS) -lws -lgcc_s
LDFLAGS = -L$(OPENSSL_DIR) -L$(CURL_DIR)/lib/.libs -L$(XML2_DIR)/.libs -L$(JSON_C_DIR)/.libs -L. $(ROUTER_LDFLAGS) -L$(WS_SRC) -L$(WB_DIR) -L$(WB_DIR)/output
ifeq ($(RTCONFIG_QCA),y)
LDFLAGS += -L$(LIBDIR) -lm
endif


ifeq ($(HND_ROUTER),y)
LDFLAGS += -L$(TOP_PLATFORM)/wlcsm$(EXHND) -lwlcsm
endif

ifeq ($(RTCONFIG_BCMARM),y)
CFLAGS += -I$(SRCBASE)/shared/bcmwifi/include -DTYPEDEF_FLOAT_T
endif

ifeq ($(RTCONFIG_SW_HW_AUTH),y)
CFLAGS += -I$(SW_HW_AUTH_DIR) -DSW_HW_AUTH
endif

ifeq ($(wildcard $(TOP)/aaews/*.c),)
$(DOG):
	-cp -f $(TOP)/aaews/prebuild/$(BUILD_NAME)/mastiff mastiff
else
$(DOG): $(DOG).o nat_nvram.o  ws_caller.o nw_util.o natapi.o $(WSLIB_OBJS) info_report.o
	$(CC) -fPIC $(CFLAGS) $(LDFLAGS) $(LIBS) $(ROUTER_LDFLAGS) $(ROUTER_LIBS) -o $@ $^ 
#	mipsel-uclibc-gcc -c $(CFLAGS) $(LDFLAGS) $(LIBS) -o $@ $^ 
endif
aicam:$(TARGET)

ifeq ($(wildcard $(TOP)/aaews/*.c),)
router:
	-cp -f $(TOP)/aaews/prebuild/$(BUILD_NAME)/aaews aaews
else
router: $(WSLIB) $(TARGET) $(DOG)
endif
$(WSLIB):
	cd $(WB_DIR) && \
	CC=$(CC) AR=$(AR) RANLIB=$(RANLIB) LD=$(LD) CFLAGS="-Os -Wall $(EXTRACFLAGS)" \
	make libws.so


$(TARGET):  main.o  $(WSLIB_OBJS) $(OBJS_MAIN) 
	$(CXX) -fPIC $(CFLAGS) $(LDFLAGS) $(LIBS) -o $@ $^ 

%.o: %.c
	$(CC) -c  -fPIC $(CFLAGS) $< -o $@ 

#install: all
#	$(STRIP) $(WSLIB)
#	install -D $(WSLIB) $(INSTALLDIR)/lib/$(WSLIB)
	
clean:
	echo $(WB_DIR)
	echo $(TOP)
	rm -f *.o  *.so *.dylib *.a aaews mastiff -r output

