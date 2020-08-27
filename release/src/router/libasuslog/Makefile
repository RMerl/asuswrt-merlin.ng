include ../common.mak
include $(SRCBASE)/router/.config

OBJS = libasuslog.o
PROGS = libasuslog.so

CFLAGS += -fPIC $(EXTRACFLAGS)
CFLAGS += -I. -I$(TOP)/shared -I$(SRCBASE)/include -I$(KERNEL_HEADER_DIR) -Wall
#CFLAGS += $(PRIVATE_EXTRACFLAGS)
LDFLAGS_SO = -L$(TOP_PLATFORM)/nvram${BCMEX}${EX7} -lnvram -L$(TOP)/shared -lshared 
ifeq ($(HND_ROUTER),y)
LDFLAGS_SO += -lpthread -lgcc_s
endif
ifeq ($(RTCONFIG_BCMARM),y)
CFLAGS += -I$(SRCBASE)/shared/bcmwifi/include -DTYPEDEF_FLOAT_T
CFLAGS += -I$(SRCBASE)/common/include
LDFLAGS += -lpthread -lgcc_s
endif

ifeq ($(RTCONFIG_QCA),y)
LDFLAGS += -ldl
endif


#SHARE_STATIC=y


ifeq ($(wildcard $(TOP)/libasuslog/*.c),)
all:
	-cp -f $(TOP)/libasuslog/prebuild/$(BUILD_NAME)/libasuslog.so libasuslog.so
else
all: $(PROGS)
endif




libasuslog.so: $(OBJS)
ifeq ($(HND_ROUTER),y)
	$(CC) $(LDFLAGS_SO) -shared -o $@ $^
else
	$(LD) $(LDFLAGS_SO) -shared -o $@ $^
endif


install: all
	$(STRIP) $(PROGS)

	install -D libasuslog.so $(INSTALLDIR)/lib/libasuslog.so

clean:
	rm -f *.o $(PROGS)
