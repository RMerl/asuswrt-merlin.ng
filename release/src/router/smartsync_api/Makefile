include ../common.mak

CFLAGS = -I$(TOP)/openssl/include -I$(TOP)/curl-7.21.7/include -I$(TOP)/libxml2/include
CFLAGS += -I$(SRCBASE) -I$(SRCBASE)/include -I. -I$(TOP)/shared -D_FILE_OFFSET_BITS=64 -DNVRAM_
CFLAGS += $(if $(QCA),$(EXTRACFLAGS))

LDFLAGS = -L$(TOP)/curl-7.21.7/lib/.libs -L$(TOP)/openssl -L$(TOP)/nvram$(BCMEX)$(EX7) -L$(TOP)/shared -L$(TOP)/libxml2/.libs

LIBS = -lnvram -lshared -lcrypto -lssl -lcurl -lxml2

#ifeq ($(RTCONFIG_QTN),y)
#LDFLAGS += -L$(TOP)/libqcsapi_client
#LIBS += -lqcsapi_client
#endif
#ifeq ($(RTCONFIG_BCMARM),y)
#CFLAGS += -I$(SRCBASE)/common/include
#endif

all: libsmartsync_api.so

api.o: api.c
	$(CC)  $(CFLAGS) $(LDFLAGS) $(LIBS)  -c  -fPIC $^

google.o: google.c
	$(CC)  $(CFLAGS) $(LDFLAGS) $(LIBS)  -c  -fPIC $^

cJSON.o: cJSON.c
	$(CC)  $(CFLAGS) $(LDFLAGS) $(LIBS)  -c  -fPIC $^

libsmartsync_api.so: api.o google.o cJSON.o
	$(CC)  $(CFLAGS) $(LDFLAGS) $(LIBS) -shared -o $@ -fPIC $^

#install: all
#	@echo " [smartsync_api] Installing as $(INSTALLDIR)/usr/lib/libsmartsync_api.so"
#	@install -d $(INSTALLDIR)/usr/lib
#	@install -m 755 libsmartsync_api.so $(INSTALLDIR)/usr/lib
#	@$(STRIP) $(INSTALLDIR)/usr/lib/libsmartsync_api.so

.PHONY: clean
clean:
	rm -f *.o libsmartsync_api.so

