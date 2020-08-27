
include ../common.mak
include $(TOP)/.config

CURL_DIR=$(TOP)/curl-7.21.7
XML2_DIR=$(TOP)/libxml2
OPENSSL_DIR=$(TOP)/openssl
NVRAM_DIR=$(TOP)/nvram$(BCMEX)$(EX7)
JSON_C_DIR=$(TOP)/json-c
SHARED_DIR=$(TOP)/shared


#ROUTER_LIBS=-lnvram
#ROUTER_LDFLAGS=-L$(NVRAM_DIR)

ROUTER_LIBS=-lnvram -lshared
ROUTER_LDFLAGS=-L$(NVRAM_DIR) -L$(SHARED_DIR)




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





# CFLAGS += -D_FILE_OFFSET_BITS=64 

CFLAGS += -g
CFLAGS += -I.
CFLAGS += -I$(CURL_DIR)/include
CFLAGS += -I$(OPENSSL_DIR)/include
CFLAGS += -I$(XML2_DIR)/include
CFLAGS += -I$(JSON_C_DIR)
CFLAGS += -I$(NVRAM_DIR)
CFLAGS += -I$(SRCBASE)/include


LIBS = -lcurl -lssl -lcrypto -lxml2 -ljson-c -ldl -pthread -lrt $(ROUTER_LIBS) -lgcc_s

LDFLAGS = -L$(OPENSSL_DIR) -L$(CURL_DIR)/lib/.libs -L$(XML2_DIR)/.libs -L$(JSON_C_DIR)/.libs -L. $(ROUTER_LDFLAGS)

ifeq ($(RTCONFIG_QCA),y)
LDFLAGS += -L$(LIBDIR) -lm
endif


ifeq ($(HND_ROUTER),y)
CFLAGS += -DPTHREAD_STACK_SIZE=0x51200
LDFLAGS += -L$(TOP)/wlcsm -lwlcsm -lm
endif

#ifeq ($(RTCONFIG_SW_HW_AUTH),y)
#CFLAGS += -I$(SW_HW_AUTH_DIR) -DSW_HW_AUTH
#endif


#CROSS_CMP = arm-unknown-linux-uclibcgnueabi-
#CROSS_CMP = arm-linux-gnueabihf-
#CROSS_CMP = mips-linux-
#CROSS_CMP = rsdk-linux-

#CC=$(CROSS_CMP)gcc
#STRIP=$(CROSS_CMP)strip

#LD = mips-linux-ld


#ifeq ($(RTCONFIG_BCMARM),y)
#CFLAGS += -DHAVE_TYPE_FLOAT
#endif

ifeq ($(RTCONFIG_QTN),y)
LDFLAGS += -L$(TOP)/libqcsapi_client -lqcsapi_client -L
endif

# aicam - 3-party lib
# LDFLAGS += -L/home/markcool/project/asusnatnl/apps/aaews/lib
#-L/home/markcool/project/toolchain/linaro-multilib-2014.06-gcc4.9/arm-linux-gnueabihf/libc/usr/lib/armv6k-linux-gnueabi

#xml: xml.o 
#	gcc xml.o -o xml $(CFLAGS) 

#xml.o: xml.c
#	gcc -c xml.c $(CFLAGS) 

#xml: xml.o 
#	$(CC) $^ -o $@ $(CFLAGS) 

#hello: jsonP.o 
#	$(CC) $(LDFLAGS)  $^ -o $@ $(CFLAGS) $(LIBS) 

#


all: uploader
	$(STRIP) uploader



#uploader: uploader_google.o  
#		$(CC) -lgcc_s $(LDFLAGS)  $^ -o $@ $(CFLAGS) $(LIBS)



uploader: time_util.o log.o des.o base64encode.o base64decode.o uploader_config.o uploader.o api.o function.o list.o mem_pool.o \
		apiTsdbInputNoId.o  toolComposeAuthorizationHeader.o toolCurl.o  
		$(CC) -lgcc_s $(LDFLAGS)  $^ -o $@ $(CFLAGS) $(LIBS)


#hello: hello.o api.o function.o list.o mem_pool.o
#	$(CC) $(LDFLAGS)  $^ -o $@ $(CFLAGS) $(LIBS)
#
#hello: hello.o api2.o
#	gcc hello.o -o hello $(CFLAGS) 

#hello.o: hello.c
#	gcc -c hello.c api2.c $(CFLAGS) 

#api2: api2.o 
#	gcc api2.o -o api2 $(CFLAGS) 

#api2.o: api2.c
#	gcc -c api2.c $(CFLAGS) 



.PHONY: clean

clean: 
	rm -f *.o
