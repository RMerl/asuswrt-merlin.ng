ALL=eap_example

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

OBJS_both += ../src/utils/libutils.a
OBJS_both += ../src/crypto/libcrypto.a
OBJS_both += ../src/tls/libtls.a

OBJS_both += ../src/eap_common/eap_peap_common.o
OBJS_both += ../src/eap_common/eap_psk_common.o
OBJS_both += ../src/eap_common/eap_pax_common.o
OBJS_both += ../src/eap_common/eap_sake_common.o
OBJS_both += ../src/eap_common/eap_gpsk_common.o
OBJS_both += ../src/eap_common/chap.o

OBJS_peer += ../src/eap_peer/eap_tls.o
OBJS_peer += ../src/eap_peer/eap_peap.o
OBJS_peer += ../src/eap_peer/eap_ttls.o
OBJS_peer += ../src/eap_peer/eap_md5.o
OBJS_peer += ../src/eap_peer/eap_mschapv2.o
OBJS_peer += ../src/eap_peer/mschapv2.o
OBJS_peer += ../src/eap_peer/eap_otp.o
OBJS_peer += ../src/eap_peer/eap_gtc.o
OBJS_peer += ../src/eap_peer/eap_leap.o
OBJS_peer += ../src/eap_peer/eap_psk.o
OBJS_peer += ../src/eap_peer/eap_pax.o
OBJS_peer += ../src/eap_peer/eap_sake.o
OBJS_peer += ../src/eap_peer/eap_gpsk.o
OBJS_peer += ../src/eap_peer/eap.o
OBJS_peer += ../src/eap_common/eap_common.o
OBJS_peer += ../src/eap_peer/eap_methods.o
OBJS_peer += ../src/eap_peer/eap_tls_common.o

CFLAGS += -DEAP_TLS
CFLAGS += -DEAP_PEAP
CFLAGS += -DEAP_TTLS
CFLAGS += -DEAP_MD5
CFLAGS += -DEAP_MSCHAPv2
CFLAGS += -DEAP_GTC
CFLAGS += -DEAP_OTP
CFLAGS += -DEAP_LEAP
CFLAGS += -DEAP_PSK
CFLAGS += -DEAP_PAX
CFLAGS += -DEAP_SAKE
CFLAGS += -DEAP_GPSK -DEAP_GPSK_SHA256

CFLAGS += -DEAP_SERVER_IDENTITY
CFLAGS += -DEAP_SERVER_TLS
CFLAGS += -DEAP_SERVER_PEAP
CFLAGS += -DEAP_SERVER_TTLS
CFLAGS += -DEAP_SERVER_MD5
CFLAGS += -DEAP_SERVER_MSCHAPV2
CFLAGS += -DEAP_SERVER_GTC
CFLAGS += -DEAP_SERVER_PSK
CFLAGS += -DEAP_SERVER_PAX
CFLAGS += -DEAP_SERVER_SAKE
CFLAGS += -DEAP_SERVER_GPSK -DEAP_SERVER_GPSK_SHA256

CFLAGS += -DIEEE8021X_EAPOL

# Optional components to add EAP server support
OBJS_server += ../src/eap_server/eap_server_tls.o
OBJS_server += ../src/eap_server/eap_server_peap.o
OBJS_server += ../src/eap_server/eap_server_ttls.o
OBJS_server += ../src/eap_server/eap_server_md5.o
OBJS_server += ../src/eap_server/eap_server_mschapv2.o
OBJS_server += ../src/eap_server/eap_server_gtc.o
OBJS_server += ../src/eap_server/eap_server_psk.o
OBJS_server += ../src/eap_server/eap_server_pax.o
OBJS_server += ../src/eap_server/eap_server_sake.o
OBJS_server += ../src/eap_server/eap_server_gpsk.o
OBJS_server += ../src/eap_server/eap_server.o
OBJS_server += ../src/eap_server/eap_server_identity.o
OBJS_server += ../src/eap_server/eap_server_methods.o
OBJS_server += ../src/eap_server/eap_server_tls_common.o
CFLAGS += -DEAP_SERVER

ifndef LDO
LDO=$(CC)
endif

Q=@
E=echo
ifeq ($(V), 1)
Q=
E=true
endif

%.o: %.c
	$(Q)$(CC) -c -o $@ $(CFLAGS) $<
	@$(E) "  CC " $<

OBJS_lib=$(OBJS_both) $(OBJS_peer) $(OBJS_server)

OBJS_ex = eap_example.o eap_example_peer.o eap_example_server.o

../src/utils/libutils.a:
	$(MAKE) -C ../src/utils

../src/crypto/libcrypto.a:
	$(MAKE) -C ../src/crypto

../src/tls/libtls.a:
	$(MAKE) -C ../src/tls

ifneq ($(CONFIG_SOLIB), yes)
LIBEAP = libeap.a
libeap.a: $(OBJS_lib)
	$(AR) crT libeap.a $(OBJS_lib)
	$(RANLIB) libeap.a

else
CFLAGS  += -fPIC -DPIC
LDFLAGS += -shared

LIBEAP  = libeap.so
libeap.so: $(OBJS_lib)
	$(LDO) $(LDFLAGS) $(OBJS_lib) -o $(LIBEAP)

endif

eap_example: $(OBJS_ex) $(LIBEAP)
	$(LDO) $(LDFLAGS) -o eap_example $(OBJS_ex) -L. -leap $(LIBS)

clean:
	$(MAKE) -C ../src clean
	rm -f core *~ *.o *.d libeap.a libeap.so $(ALL)

-include $(OBJS:%.o=%.d)
