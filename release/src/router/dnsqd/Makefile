include $(LINUXDIR)/.config
include ../common.mak
include $(SRCBASE)/.config
include ../.config


$(info  --------------------------------------------------------------dnsqd--------------------------------------------)
$(info HND_ROUTER=$(HND_ROUTER))
$(info HND=$(HND))
$(info AX=$(AX))
$(info QCA=$(QCA))


ifeq ($(HND_ROUTER), y)
NETLINK_LIB_VER := 1.0.1
else
NETLINK_LIB_VER := 1.0.2
endif

CFLAGS = -s -Os -fomit-frame-pointer -fPIC

#CFLAGS += -DSAVE_DNS_PACKET -DDNSQD_DEBUG

LDFLAGS = -fPIC -L../libnfnetlink-$(NETLINK_LIB_VER)/src/.libs -L../libnetfilter_queue/src/.libs -L../libmnl-1.0.4/src/.libs -L../sqlite/.libs -L../json-c/.libs  -L../libev-4.33/.libs -L../shared -L$(TOP_PLATFORM)/nvram$(BCMEX)$(EX7) -L$(TOP_PLATFORM)/nvram$(BCMEX)

LIBS = -lnetfilter_queue -lnfnetlink -lmnl -lsqlite3 -lpthread -ljson-c -lev -lshared -lnvram 

LDFLAGS += -L$(TOP)/libmnl-1.0.4/src/.libs -L$(TOP)/libnetfilter_conntrack-1.0.7/src/.libs 
LIBS +=  -lmnl -lnetfilter_conntrack


ifeq ($(HND_ROUTER), y)
LDFLAGS += -L$(TOP_PLATFORM)/wlcsm$(EXHND)
LIBS  += -lwlcsm
endif

#CFLAGS += -Wall -I../libnetfilter_queue/include -I../libnfnetlink-$(NETLINK_LIB_VER)/include -I../sqlite -I../json-c -I../httpd -I../shared -I../libev-4.33
#CFLAGS += -I$(TOP)/libnetfilter_conntrack-1.0.7/include -I$(TOP)/libnetfilter_conntrack-1.0.7/include/libnetfilter_conntrack
CFLAGS += -I$(TOP)/libnetfilter_conntrack-1.0.7/include -I$(TOP)/libnetfilter_conntrack-1.0.7/include/libnetfilter_conntrack -I$(TOP)/libnetfilter_queue/include -I$(TOP)/libnfnetlink-$(NETLINK_LIB_VER)/include -I$(TOP)/sqlite -I$(TOP)/json-c -I$(TOP)/httpd -I$(TOP)/shared -I$(SRCBASE)/include -I$(TOP)/libev-4.33


OBJS = dnsqd.o dnssql.o nfapp.o nfdev.o appstats.o appjs.o log.c block_history.o block_entry.o dnsarp.o bw_mon_ct.o nfcfg.o

LD_OBJS =  log.o dns_app_client.o dns_app_client_stat.o
LD_LIBS =  -fPIC -L../sqlite/.libs -L../json-c/.libs -lsqlite3 -lpthread -ljson-c -L../shared -lshared -L$(TOP_PLATFORM)/nvram$(BCMEX)$(EX7) -L$(TOP_PLATFORM)/nvram$(BCMEX) -lnvram

ifeq ($(HND_ROUTER), y)
LD_LIBS += -L$(TOP_PLATFORM)/wlcsm$(EXHND) -lwlcsm
endif

ifeq ($(wildcard $(SRCBASE)/router/dnsqd/*.c),)
all:
	-cp -f prebuild/dnsqd dnsqd
	-cp -f prebuild/libdnsqd_app_client.so libdns_app_client.so

else
all: dnsqd libdns_app_client.so
endif
.c.o:
	$(CC) $(CFLAGS) -c $<

libdns_app_client.so: $(LD_OBJS)
	$(CC) $(LD_OBJS) -shared -o $@ 

dns_app_client_test: dns_app_client_test.o libdns_app_client.so
	$(CC) -o $@  $< -L./ -ldns_app_client $(LD_LIBS) 

dnsqd: $(OBJS)
	$(CC) -o dnsqd $(OBJS) $(LDFLAGS) $(LIBS)

clean:
	-rm -f *.o 
	-rm -f *.so
	-rm -f dns_app_client_test
	-rm -f  dnsqd
