#This target is to ensure accidental execution of Makefile as a bash script will not execute commands like rm in unexpected directories and exit gracefully.
.prevent_execution:
	exit 0


include ../../common.mak
include $(TOP)/.config


#CC = gcc

UPLOADER_DIR = $(TOP)/uploader


NVRAM_DIR=$(TOP_PLATFORM)/nvram$(BCMEX)$(EX7)
SHARED_DIR=$(TOP)/shared
SQLITE_DIR=$(TOP)/sqlite
BWDPI_DIR=$(TOP)/bwdpi_source
CURL_DIR=$(TOP)/curl-7.21.7
JSON_C_DIR=$(TOP)/json-c
OPENSSL_DIR=$(TOP)/openssl
LIBWEBAPI_DIR=$(TOP)/libwebapi
CFGMNT_DIR=$(TOP)/cfg_mnt


ROUTER_LIBS=-lnvram -lshared -lwebapi 
ROUTER_LDFLAGS=-L$(NVRAM_DIR) -L$(SHARED_DIR) -L$(LIBWEBAPI_DIR)


ifeq ($(RTCONFIG_BWDPI),y)
ROUTER_LIBS += -lsqlite3
ROUTER_LIBS += -lbwdpi
ROUTER_LIBS += -lbwdpi_sql

ROUTER_LDFLAGS += -L$(SQLITE_DIR)/.libs
ROUTER_LDFLAGS += -L$(BWDPI_DIR)/asus
ROUTER_LDFLAGS += -L$(BWDPI_DIR)/asus_sql
endif


ifeq ($(RTCONFIG_CFGSYNC),y)
ROUTER_LIBS += -lcfgmnt
ROUTER_LDFLAGS += -L$(CFGMNT_DIR)
endif

LDFLAGS = $(ROUTER_LDFLAGS) $(ROUTER_LIBS)  

ifeq ($(HND_ROUTER),y)
LDFLAGS += -L$(TOP_PLATFORM)/wlcsm$(EXHND) -lwlcsm
endif


#remove @ for no make command prints
DEBUG = 

APP_DIR = .
APP_INCLUDE_DIRS += -I $(APP_DIR)

APP_NAME = uploader
APP_SRC_FILES = time_util.c log.c api.c uploader_ipc.c uploader_mnt.c
UPLOAD_SO_FILE = upload_api.c
# APP_SRC_FILES = api.c
APP_SRC_FILES += $(APP_NAME).c 



ifeq ($(RTCONFIG_QCA),y)
LIBS += -lm
endif


#Aggregate LDFLAGS
LDFLAGS += -L$(CURL_DIR)/lib/.libs 
LDFLAGS += -L$(JSON_C_DIR)/.libs/ 
LDFLAGS += -L$(OPENSSL_DIR)
LDFLAGS += -L$(UPLOADER_DIR)/src 



OPENSSL_LIBS = -lssl -lcrypto -lpthread
LIBS +=  $(OPENSSL_LIBS) -ljson-c 
LIBS +=  -lc -lcurl -ldl -lupload
# LIBS = -lc -lcurl -lxml2 -lssl -lcrypto -ljson-c -lpthread -ldl -lm 
 


#Aggregate all include and src directories
CFLAGS += -g
CFLAGS += -I.

CFLAGS += -I$(UPLOADER_DIR)/src
CFLAGS += $(APP_INCLUDE_DIRS)
CFLAGS += -I$(CURL_DIR)/include/curl
CFLAGS += -I$(CURL_DIR)/include
CFLAGS += -I$(JSON_C_DIR) 
CFLAGS += -I$(OPENSSL_DIR)/include
CFLAGS += -I$(NVRAM_DIR)
CFLAGS += -I$(SRCBASE)/include
CFLAGS += -I$(SHARED_DIR) 
CFLAGS += -I$(LIBWEBAPI_DIR)


ifeq ($(RTCONFIG_BWDPI),y)
CFLAGS += -I$(SQLITE_DIR)	
CFLAGS += -I$(BWDPI_DIR)/include
CFLAGS += -I$(BWDPI_DIR)/asus_include
endif


ifeq ($(RTCONFIG_CFGSYNC),y)
CFLAGS += -I$(SHARED_DIR)/sysdeps/amas
CFLAGS += -I$(CFGMNT_DIR)
endif


ifeq ($(RTCONFIG_BCMARM),y)
CFLAGS += -I$(SRCBASE)/shared/bcmwifi/include -DTYPEDEF_FLOAT_T
endif

ifeq ($(RTCONFIG_USB),y)
CFLAGS += -I$(TOP)/libdisk
LIBS += -L$(TOP)/libdisk -ldisk
ifeq ($(RTCONFIG_USB_SMS_MODEM),y)
CFLAGS += -I$(TOP)/smspdu
LIBS += -L$(TOP)/smspdu -lsmspdu
endif
endif

SRC_FILES += $(APP_SRC_FILES)

UPLOAD_SO_CMD = $(CC) -fPIC -c $(UPLOAD_SO_FILE) $(CFLAGS) -o upload_api.o $(LDFLAGS) 

UPLOAD_SO_CMD2 = $(CC) -shared upload_api.o -o libupload.so

MAKE_CMD = $(CC) $(SRC_FILES) $(CFLAGS) -o $(APP_NAME) $(LDFLAGS)  $(LIBS)

# MAKE_CMD = $(CC) $(SRC_FILES) $(CFLAGS) -o $(APP_NAME).o $(LDFLAGS)  $(LIBS)

all:
	$(DEBUG)$(UPLOAD_SO_CMD)
	$(DEBUG)$(UPLOAD_SO_CMD2)
	$(DEBUG)$(MAKE_CMD)

clean:
	rm -f $(APP_DIR)/$(APP_NAME)
