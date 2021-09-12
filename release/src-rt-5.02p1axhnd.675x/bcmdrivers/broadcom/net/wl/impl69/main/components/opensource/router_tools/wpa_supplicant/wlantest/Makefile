ALL=wlantest wlantest_cli test_vectors

all: $(ALL)

ifndef CC
CC=gcc
endif

ifndef RANLIB
RANLIB=ranlib
endif

ifndef CFLAGS
CFLAGS = -MMD -O2 -Wall -g
endif

CFLAGS += -I.
CFLAGS += -I../src
CFLAGS += -I../src/utils

# glibc < 2.17 needs -lrt for clock_gettime()
LIBS += -lrt

ifndef LDO
LDO=$(CC)
endif

Q=@
E=echo
ifeq ($(V), 1)
Q=
E=true
endif
ifeq ($(QUIET), 1)
Q=@
E=true
endif

%.o: %.c
	$(Q)$(CC) -c -o $@ $(CFLAGS) $<
	@$(E) "  CC " $<

OBJS_lib += ../src/utils/libutils.a
OBJS_lib += ../src/crypto/libcrypto.a

CFLAGS += -DCONFIG_OCV
CFLAGS += -DCONFIG_IEEE80211R
CFLAGS += -DCONFIG_HS20
CFLAGS += -DCONFIG_DEBUG_FILE
CFLAGS += -DCONFIG_FILS
CFLAGS += -DCONFIG_SAE
CFLAGS += -DCONFIG_OWE
CFLAGS += -DCONFIG_DPP
CFLAGS += -DCONFIG_SHA384

OBJS += ../src/common/ieee802_11_common.o
OBJS += ../src/common/wpa_common.o
OBJS += ../src/radius/radius.o
OBJS += ../src/rsn_supp/wpa_ie.o

OBJS += wlantest.o
OBJS += readpcap.o
OBJS += writepcap.o
OBJS += monitor.o
OBJS += process.o
OBJS += wired.o
OBJS += rx_mgmt.o
OBJS += rx_data.o
OBJS += rx_eapol.o
OBJS += rx_ip.o
OBJS += rx_tdls.o
OBJS += bss.o
OBJS += sta.o
OBJS += ccmp.o
OBJS += tkip.o
OBJS += ctrl.o
OBJS += inject.o
OBJS += wep.o
OBJS += bip.o
OBJS += gcmp.o

LIBS += -lpcap

TOBJS += test_vectors.o
TOBJS += ccmp.o
TOBJS += tkip.o
TOBJS += wep.o
TOBJS += bip.o
TOBJS += gcmp.o

../src/utils/libutils.a:
	$(MAKE) -C ../src/utils

../src/crypto/libcrypto.a:
	$(MAKE) -C ../src/crypto

ifneq ($(CONFIG_SOLIB), yes)
LIBWLANTEST = libwlantest.a
libwlantest.a: $(OBJS_lib)
	$(AR) crT libwlantest.a $(OBJS_lib)
	$(RANLIB) libwlantest.a

else
CFLAGS  += -fPIC -DPIC
LDFLAGS += -shared

LIBWLANTEST  = libwlantest.so
libwlantest.so: $(OBJS_lib)
	$(LDO) $(LDFLAGS) $(OBJS_lib) -o $(LIBWLANTEST)

endif

OBJS_cli = wlantest_cli.o

wlantest: $(OBJS) $(LIBWLANTEST)
	$(LDO) $(LDFLAGS) -o wlantest $(OBJS) -L. -lwlantest $(LIBS)

wlantest_cli: $(OBJS_cli) $(LIBWLANTEST)
	$(LDO) $(LDFLAGS) -o wlantest_cli $(OBJS_cli) -L. -lwlantest $(LIBS)

test_vectors: $(TOBJS) $(LIBWLANTEST)
	$(LDO) $(LDFLAGS) -o test_vectors $(TOBJS) -L. -lwlantest $(LIBS)

clean:
	$(MAKE) -C ../src clean
	rm -f core *~ *.o *.d libwlantest.a libwlantest.so $(ALL)

-include $(OBJS:%.o=%.d)
