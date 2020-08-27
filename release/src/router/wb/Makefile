include ../common.mak

WSLIB=libws.so

CURL_DIR=$(TOP)/curl-7.21.7
XML2_DIR=$(TOP)/libxml2
JSON_C_DIR=$(TOP)/json-c
OPENSSL_DIR=$(TOP)/openssl
NTCENTER_DIR=$(TOP)/nt_center

WS_SRC=ws_src
OUTPUT=output

SRC=$(wildcard *.c)
OBJS=$(patsubst %.c, %.o, $(SRC))
#WS_SOURCES := $(WS_SRC)/curl_api.c $(WS_SRC)/parse_xml.c $(WS_SRC)/wb.c $(WS_SRC)/wb_util.c $(WS_SRC)/ssl_api.c $(WS_SRC)/log.c
WS_SOURCES := $(wildcard $(WS_SRC)/*.c)
WSLIB_OBJS := $(WS_SOURCES:%.c=$(OUTPUT)/%.o)

ifeq ($(wildcard $(TOP)/wb/ws_src/*.c),)
all:
	-cp -f $(TOP)/wb/prebuild/$(BUILD_NAME)/libws.so libws.so
else
all: $(WSLIB)
endif

CFLAGS += 
CFLAGS += -fPIC
ifeq ($(RTCONFIG_NOTIFICATION_CENTER), y)
CFLAGS += -DRTCONFIG_NOTIFICATION_CENTER=y
endif
ifeq ($(RTCONFIG_IFTTT), y)
CFLAGS += -DRTCONFIG_IFTTT=y
endif
ifeq ($(RTCONFIG_ALEXA), y)
CFLAGS += -DRTCONFIG_ALEXA=y
endif
CFLAGS += -I$(CURL_DIR)/include
CFLAGS += -I$(OPENSSL_DIR)/include
CFLAGS += -I$(XML2_DIR)/include
CFLAGS += -I$(JSON_C_DIR)
CFLAGS += -I$(WS_SRC)
CFLAGS += -I.

LDFLAGS = -L$(OPENSSL_DIR) -lcrypto -lssl
LDFLAGS += -L$(CURL_DIR)/lib/.libs -lcurl
LDFLAGS += -L$(XML2_DIR)/.libs -lxml2
LDFLAGS += -L$(JSON_C_DIR)/.libs -ljson-c
ifeq ($(HND_ROUTER),y)
LDFLAGS += -lm -lgcc_s
endif

$(WSLIB): $(WSLIB_OBJS)
	$(LD) -shared -o $@ $^ $(LDFLAGS)

$(WSLIB_OBJS): $(OUTPUT)/%.o: %.c
	mkdir -p $(dir $@)	
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@ 

install: all
	$(STRIP) $(WSLIB)
	install -D $(WSLIB) $(INSTALLDIR)/lib/$(WSLIB)

clean:
	rm -rf *.o  *.so output

