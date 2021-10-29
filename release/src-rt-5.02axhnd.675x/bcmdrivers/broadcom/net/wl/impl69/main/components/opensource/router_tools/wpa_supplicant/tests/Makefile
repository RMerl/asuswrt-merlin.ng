TESTS=test-base64 test-md4 test-milenage \
	test-rsa-sig-ver \
	test-sha1 \
	test-sha256 test-aes test-asn1 test-x509 test-x509v3 test-list test-rc4

all: $(TESTS)

ifdef LIBFUZZER
CC=clang
CFLAGS = -MMD -O2 -Wall -g
CFLAGS += -fsanitize=fuzzer,address,signed-integer-overflow,unsigned-integer-overflow
CFLAGS += -DTEST_LIBFUZZER
LDFLAGS += -fsanitize=fuzzer,address,signed-integer-overflow,unsigned-integer-overflow
TEST_FUZZ=y
endif

ifndef CC
CC=gcc
endif

ifndef LDO
LDO=$(CC)
endif

ifndef CFLAGS
CFLAGS = -MMD -O2 -Wall -g
endif

ifdef TEST_FUZZ
CFLAGS += -DCONFIG_NO_RANDOM_POOL
CFLAGS += -DTEST_FUZZ
endif

CFLAGS += -DCONFIG_IEEE80211R_AP
CFLAGS += -DCONFIG_IEEE80211R
CFLAGS += -DCONFIG_TDLS

CFLAGS += -I../src
CFLAGS += -I../src/utils

SLIBS = ../src/utils/libutils.a

DLIBS = ../src/crypto/libcrypto.a \
	../src/common/libcommon.a \
	../src/ap/libap.a \
	../src/eapol_auth/libeapol_auth.a \
	../src/eapol_supp/libeapol_supp.a \
	../src/eap_peer/libeap_peer.a \
	../src/eap_server/libeap_server.a \
	../src/eap_common/libeap_common.a \
	../src/radius/libradius.a \
	../src/l2_packet/libl2_packet.a \
	../src/wps/libwps.a \
	../src/rsn_supp/librsn_supp.a \
	../src/tls/libtls.a

LIBS = $(SLIBS) $(DLIBS)
LLIBS = -Wl,--start-group $(DLIBS) -Wl,--end-group $(SLIBS)

# glibc < 2.17 needs -lrt for clock_gettime()
LLIBS += -lrt

../src/utils/libutils.a:
	$(MAKE) -C ../src/utils

../src/common/libcommon.a:
	$(MAKE) -C ../src/common

../src/crypto/libcrypto.a:
	$(MAKE) -C ../src/crypto

../src/tls/libtls.a:
	$(MAKE) -C ../src/tls

../src/ap/libap.a:
	$(MAKE) -C ../src/ap

../src/radius/libradius.a:
	$(MAKE) -C ../src/radius

../src/l2_packet/libl2_packet.a:
	$(MAKE) -C ../src/l2_packet

../src/wps/libwps.a:
	$(MAKE) -C ../src/wps

../src/eap_peer/libeap_peer.a:
	$(MAKE) -C ../src/eap_peer

../src/eap_server/libeap_server.a:
	$(MAKE) -C ../src/eap_server

../src/eap_common/libeap_common.a:
	$(MAKE) -C ../src/eap_common

../src/eapol_auth/libeapol_auth.a:
	$(MAKE) -C ../src/eapol_auth

../src/eapol_supp/libeapol_supp.a:
	$(MAKE) -C ../src/eapol_supp

../src/rsn_supp/librsn_supp.a:
	$(MAKE) -C ../src/rsn_supp

test-aes: test-aes.o $(LIBS)
	$(LDO) $(LDFLAGS) -o $@ $^ $(LLIBS)

test-asn1: test-asn1.o $(LIBS)
	$(LDO) $(LDFLAGS) -o $@ $^ $(LLIBS)

test-base64: test-base64.o $(LIBS)
	$(LDO) $(LDFLAGS) -o $@ $^ $(LLIBS)

test-eapol: test-eapol.o $(LIBS)
	$(LDO) $(LDFLAGS) -o $@ $< $(LLIBS)

test-https: test-https.o $(LIBS)
	$(LDO) $(LDFLAGS) -o $@ $< $(LLIBS)

test-https_server: test-https_server.o $(LIBS)
	$(LDO) $(LDFLAGS) -o $@ $< $(LLIBS)

test-json: test-json.o $(LIBS)
	$(LDO) $(LDFLAGS) -o $@ $^ $(LLIBS)

test-list: test-list.o $(LIBS)
	$(LDO) $(LDFLAGS) -o $@ $^ $(LLIBS)

test-md4: test-md4.o $(LIBS)
	$(LDO) $(LDFLAGS) -o $@ $^ $(LLIBS)

test-milenage: test-milenage.o $(LIBS)
	$(LDO) $(LDFLAGS) -o $@ $^ $(LLIBS)

test-rc4: test-rc4.o $(LIBS)
	$(LDO) $(LDFLAGS) -o $@ $^ $(LLIBS)

test-rsa-sig-ver: test-rsa-sig-ver.o $(LIBS)
	$(LDO) $(LDFLAGS) -o $@ $< $(LLIBS)

test-sha1: test-sha1.o $(LIBS)
	$(LDO) $(LDFLAGS) -o $@ $^ $(LLIBS)

test-sha256: test-sha256.o $(LIBS)
	$(LDO) $(LDFLAGS) -o $@ $^ $(LLIBS)

test-tls: test-tls.o $(LIBS)
	$(LDO) $(LDFLAGS) -o $@ $< $(LLIBS)

test-x509: test-x509.o $(LIBS)
	$(LDO) $(LDFLAGS) -o $@ $< $(LLIBS)

test-x509v3: test-x509v3.o $(LIBS)
	$(LDO) $(LDFLAGS) -o $@ $< $(LLIBS)

run-tests: $(TESTS)
	./test-aes
	./test-list
	./test-md4
	./test-milenage
	./test-rsa-sig-ver
	./test-sha1
	./test-sha256
	@echo
	@echo All tests completed successfully.

clean:
	$(MAKE) -C ../src clean
	rm -f $(TESTS) *~ *.o *.d
	rm -f test-eapol
	rm -f test-https
	rm -f test-json
	rm -f test-tls
	rm -f test_x509v3_nist.out.*
	rm -f test_x509v3_nist2.out.*

-include $(OBJS:%.o=%.d)
