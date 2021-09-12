all: p2p-fuzzer

ifndef CC
CC=gcc
endif

ifndef LDO
LDO=$(CC)
endif

ifndef CFLAGS
CFLAGS = -MMD -O2 -Wall -g
endif

SRC=../../src

CFLAGS += -I$(SRC)

$(SRC)/utils/libutils.a:
	$(MAKE) -C $(SRC)/utils

$(SRC)/common/libcommon.a:
	$(MAKE) -C $(SRC)/common

$(SRC)/crypto/libcrypto.a:
	$(MAKE) -C $(SRC)/crypto

$(SRC)/tls/libtls.a:
	$(MAKE) -C $(SRC)/tls

$(SRC)/p2p/libp2p.a:
	$(MAKE) -C $(SRC)/p2p

$(SRC)/wps/libwps.a:
	$(MAKE) -C $(SRC)/wps

LIBS += $(SRC)/utils/libutils.a
LIBS += $(SRC)/common/libcommon.a
LIBS += $(SRC)/crypto/libcrypto.a
LIBS += $(SRC)/p2p/libp2p.a
LIBS += $(SRC)/tls/libtls.a
LIBS += $(SRC)/wps/libwps.a

p2p-fuzzer: p2p-fuzzer.o $(LIBS)
	$(LDO) $(LDFLAGS) -o $@ $^ $(LIBS)

clean:
	$(MAKE) -C $(SRC) clean
	rm -f p2p-fuzzer *~ *.o *.d

-include $(OBJS:%.o=%.d)
