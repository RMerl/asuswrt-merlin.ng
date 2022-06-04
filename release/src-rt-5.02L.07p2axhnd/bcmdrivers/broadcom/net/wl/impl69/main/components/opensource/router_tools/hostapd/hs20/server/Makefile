all: hs20_spp_server

ifndef CC
CC=gcc
endif

ifndef LDO
LDO=$(CC)
endif

ifndef CFLAGS
CFLAGS = -MMD -O2 -Wall -g
endif

CFLAGS += -I../../src
CFLAGS += -I../../src/utils
CFLAGS += -I../../src/crypto

LIBS += -lsqlite3

# Using glibc < 2.17 requires -lrt for clock_gettime()
LIBS += -lrt

ifndef CONFIG_NO_GITVER
# Add VERSION_STR postfix for builds from a git repository
ifeq ($(wildcard ../../.git),../../.git)
GITVER := $(shell git describe --dirty=+)
ifneq ($(GITVER),)
CFLAGS += -DGIT_VERSION_STR_POSTFIX=\"-$(GITVER)\"
endif
endif
endif

OBJS=spp_server.o
OBJS += hs20_spp_server.o
OBJS += ../../src/utils/xml-utils.o
OBJS += ../../src/utils/base64.o
OBJS += ../../src/utils/common.o
OBJS += ../../src/utils/os_unix.o
OBJS += ../../src/utils/wpa_debug.o
OBJS += ../../src/crypto/md5-internal.o
CFLAGS += $(shell xml2-config --cflags)
LIBS += $(shell xml2-config --libs)
OBJS += ../../src/utils/xml_libxml2.o

hs20_spp_server: $(OBJS)
	$(LDO) $(LDFLAGS) -o hs20_spp_server $(OBJS) $(LIBS)

clean:
	rm -f core *~ *.o *.d hs20_spp_server
	rm -f ../../src/utils/*.o
	rm -f ../../src/utils/*.d
	rm -f ../../src/crypto/*.o
	rm -f ../../src/crypto/*.d

-include $(OBJS:%.o=%.d)
