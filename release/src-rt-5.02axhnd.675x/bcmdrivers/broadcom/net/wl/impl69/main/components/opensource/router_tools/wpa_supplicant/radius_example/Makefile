ALL=radius_example

all: $(ALL)

ifndef CC
CC=gcc
endif

ifndef LDO
LDO=$(CC)
endif

ifndef CFLAGS
CFLAGS = -MMD -O2 -Wall -g
endif

CFLAGS += -I.
CFLAGS += -I../src
CFLAGS += -I../src/utils

LIBS = ../src/radius/libradius.a
LIBS += ../src/crypto/libcrypto.a
LIBS += ../src/utils/libutils.a
LLIBS = -lrt

../src/utils/libutils.a:
	$(MAKE) -C ../src/utils

../src/crypto/libcrypto.a:
	$(MAKE) -C ../src/crypto

../src/radius/libradius.a:
	$(MAKE) -C ../src/radius

#CLAGS += -DCONFIG_IPV6

OBJS_ex = radius_example.o

radius_example: $(OBJS_ex) $(LIBS)
	$(LDO) $(LDFLAGS) -o radius_example $(OBJS_ex) $(LIBS) $(LLIBS)

clean:
	$(MAKE) -C ../src clean
	rm -f core *~ *.o *.d $(ALL)

-include $(OBJS:%.o=%.d)
