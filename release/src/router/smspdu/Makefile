PROG = smspdu pullsms
LIB = libsmspdu.so

CFLAGS += -fPIC -Wall
#CFLAGS += -DSAVESMS
#LDFLAGS += -L$(TOP)/nvram${BCMEX} -lnvram -L$(TOP)/shared -lshared
LDFLAGS += -L. -lsmspdu

ifeq ($(wildcard $(TOP)/smspdu/*.c),)
all:
	-cp -f $(TOP)/smspdu/prebuild/$(LIB) $(LIB)
	-cp -f $(TOP)/smspdu/prebuild/pullsms pullsms
	-cp -f $(TOP)/smspdu/prebuild/smspdu smspdu
else
all: $(LIB) $(PROG)
endif

$(LIB): libsmspdu.o
	$(LD) -shared -o $@ $^

smspdu: $(LIB) smspdu.o
	$(CC) -o $@ smspdu.o $(CFLAGS) $(LDFLAGS)

pullsms: $(LIB) pullsms.o
	$(CC) -o $@ pullsms.o $(CFLAGS) $(LDFLAGS)

install: all
	install -D $(LIB) $(INSTALLDIR)/usr/lib/$(LIB)
	install -D smspdu $(INSTALLDIR)/usr/sbin/smspdu
	$(STRIP) $(INSTALLDIR)/usr/sbin/smspdu
	install -D pullsms $(INSTALLDIR)/usr/sbin/pullsms
	$(STRIP) $(INSTALLDIR)/usr/sbin/pullsms
	cd $(INSTALLDIR)/usr/sbin && ln -sf smspdu initial_smspdu
	cd $(INSTALLDIR)/usr/sbin && ln -sf smspdu listSMSIndex
	cd $(INSTALLDIR)/usr/sbin && ln -sf smspdu getSMSPDUbyType
	cd $(INSTALLDIR)/usr/sbin && ln -sf smspdu getSMSPDUbyIndex
	cd $(INSTALLDIR)/usr/sbin && ln -sf smspdu getallSMSPDU
	cd $(INSTALLDIR)/usr/sbin && ln -sf smspdu saveSMSPDU
	cd $(INSTALLDIR)/usr/sbin && ln -sf smspdu sendSMSPDU2
	cd $(INSTALLDIR)/usr/sbin && ln -sf smspdu sendSMSPDU
	cd $(INSTALLDIR)/usr/sbin && ln -sf smspdu delSMSPDU
	cd $(INSTALLDIR)/usr/sbin && ln -sf smspdu modSMSdraft
	cd $(INSTALLDIR)/usr/sbin && ln -sf smspdu decomposeSMSPDU
	cd $(INSTALLDIR)/usr/sbin && ln -sf smspdu composeSMSPDU

clean:
	rm -f $(LIB) $(PROG) *.o

