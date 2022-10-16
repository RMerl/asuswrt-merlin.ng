#This target is to ensure accidental execution of Makefile as a bash script will not execute commands like rm in unexpected directories and exit gracefully.
.prevent_execution:
	exit 0


include ../../../common.mak
include $(TOP)/.config



IPCLIB=libawsiot_ipc.so


IPC_SRC=../ipc
OUTPUT=output


SRC=$(wildcard *.c)
OBJS=$(patsubst %.c, %.o, $(SRC))
#WS_SOURCES := $(WS_SRC)/curl_api.c $(WS_SRC)/parse_xml.c $(WS_SRC)/wb.c $(WS_SRC)/wb_util.c $(WS_SRC)/ssl_api.c $(WS_SRC)/log.c
IPC_SOURCES := $(wildcard $(IPC_SRC)/*.c)
IPC_LIB_OBJS := $(IPC_SOURCES:%.c=$(OUTPUT)/%.o)


NVRAM_DIR=$(TOP_PLATFORM)/nvram$(BCMEX)$(EX7)
SHARED_DIR=$(TOP)/shared

ROUTER_LIBS=-lnvram -lshared 
ROUTER_LDFLAGS=-L$(NVRAM_DIR) -L$(SHARED_DIR)

ifeq ($(HND_ROUTER),y)
LDFLAGS += -L$(TOP_PLATFORM)/wlcsm$(EXHND) -lwlcsm
endif

LDFLAGS += $(if $(QCA),$(EXTRALDFLAGS))


#remove @ for no make command prints
DEBUG = 

APP_DIR = .
APP_INCLUDE_DIRS += -I $(APP_DIR)

APP_NAME = ipc_main_test
APP_SRC_FILES = ipc.c mnt.c
APP_SRC_FILES += $(APP_NAME).c 

#CC = gcc

#AWSIOT client directory
AWSIOT_DIR = ..
OPENSSL_DIR = $(TOP)/openssl
JSON_C_DIR = $(TOP)/json-c
CURL_DIR = $(TOP)/curl-7.21.7


JSON_C_LDFLAGS += -L$(JSON_C_DIR)/.libs/ 
ifeq ($(RTCONFIG_QCA),y)
LIBS += -lm
endif


OPENSSL_LDFLAGS = -L$(OPENSSL_DIR)


#Aggregate LDFLAGS
LDFLAGS += $(OPENSSL_LDFLAGS) $(JSON_C_LDFLAGS) 
LDFLAGS += -L$(CURL_DIR)/lib/.libs 

OPENSSL_LIBS = -lssl -lcrypto -lpthread 
LIBS +=  $(OPENSSL_LIBS) -ljson-c 
LIBS +=  -ldl  
# LIBS +=  -lm 
LIBS +=  -lcurl


#Aggregate all include and src directories
CFLAGS += -fPIC
CFLAGS += -g
CFLAGS += -I$(OPENSSL_DIR)/include
CFLAGS += -I$(NVRAM_DIR)
CFLAGS += -I$(SRCBASE)/include
CFLAGS += -I$(SHARED_DIR) 
CFLAGS += -I$(JSON_C_DIR) 
CFLAGS += -I$(CURL_DIR)/include/curl
CFLAGS += -I$(CURL_DIR)/include

ifeq ($(RTCONFIG_BCMARM),y)
CFLAGS += -I$(SRCBASE)/shared/bcmwifi/include -DTYPEDEF_FLOAT_T
endif

SRC_FILES += $(APP_SRC_FILES)
SRC_FILES += $(IOT_SRC_FILES)


MAKE_CMD = $(CC) $(SRC_FILES) $(CFLAGS) -o $(APP_NAME) $(LDFLAGS)  $(LIBS)



$(IPCLIB): $(IPC_LIB_OBJS)
	$(LD) -shared -o $@ $^ $(LDFLAGS) $(LIBS)

$(IPC_LIB_OBJS): $(OUTPUT)/%.o: %.c
	mkdir -p $(dir $@)	
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@ 

all: $(IPCLIB)


install: all
	$(STRIP) $(IPCLIB)
	install -D $(IPCLIB) $(INSTALLDIR)/lib/$(IPCLIB)

clean:
	rm -rf *.o  *.so output