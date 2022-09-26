include ../common.mak
include $(SRCBASE)/.config

OBJ = aupnpc.o aupnpc_util.o
OBJ_IPC = aupnpc_ipc.o aupnpc_util.o

CFLAGS += -idirafter$(LINUXDIR)/include -I$(TOP)/shared  -I$(SRCBASE)/include
ifeq ($(RTCONFIG_BCMARM),y)
CFLAGS += -I$(SRCBASE)/shared/bcmwifi/include
endif

ifeq ($(RTCONFIG_REALTEK),y)
LDFLAGS = -L$(TOP)/nvram${BCMEX} -lnvram -L$(TOP)/shared -lshared -ldl
else
LDFLAGS = -L$(TOP)/nvram${BCMEX}${EX7} -lnvram -L$(TOP)/shared -lshared -ldl
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

ifeq ($(RTCONFIG_QTN),y)
CFLAGS += -I$(TOP)/libqcsapi_client -I$(TOP)/httpd -I./sysdeps/qtn
LDFLAGS += -L$(TOP)/libqcsapi_client -lqcsapi_client
endif

TARGET = aupnpc-ipc aupnpc

ifeq ($(wildcard $(SRCBASE)/router/aupnpc/*.c),)
all:
	-cp -f prebuild/aupnpc ./
	-cp -f prebuild/aupnpc-ipc ./
else
all: $(TARGET)
endif

%.o: %.h
%.o: %.c
	$(CC) $(CFLAGS) -fPIC -c $<

aupnpc: $(OBJ)
	$(CC) -o $@ $(OBJ) $(CFLAGS) $(LIBS) $(LDFLAGS) 

aupnpc-ipc: $(OBJ_IPC)
	$(CC) -o $@ $(OBJ_IPC) $(CFLAGS) $(LIBS) $(LDFLAGS) 

install:

clean:
	-rm *.o $(TARGET)

.PHONY: all clean




