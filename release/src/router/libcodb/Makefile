#***********************************************************************
# 
# Makefile
#
#***********************************************************************
include $(LINUXDIR)/.config
include ../common.mak
include $(SRCBASE)/.config
include ../.config

OBJS = cosql_utils.o codb_utils.o log.o

CFLAGS += -g -Wall -fPIC
CFLAGS += -I$(TOP)/sqlite
CFLAGS += -I$(TOP)/json-c
CFLAGS += -I$(TOP)/shared
CFLAGS += -I$(SRCBASE)/include

LDFLAGS += -L$(TOP)/sqlite/.libs -lsqlite3
LDFLAGS += -L$(TOP)/json-c/.libs -ljson-c
LDFLAGS += -L$(TOP)/shared -lshared

ifeq ($(RTCONFIG_BCMARM),y)
CFLAGS += -I$(SRCBASE)/shared/bcmwifi/include -DTYPEDEF_FLOAT_T
endif

all: libcodb.so

.c.o:
	$(CC) -c $*.c $(CFLAGS)

libcodb.so: $(OBJS)
	$(CC) -shared $(OBJS) $(LDFLAGS) -o libcodb.so

stage: all
	install -m 644 $(foreach h,$(wildcard *.h),$(h)) $(STAGEDIR)/usr/include
	install -m 755 libcodb.so $(STAGEDIR)/usr/lib

.PHONY: clean
clean:
	@echo LINUXDIR=$(LINUXDIR)
	-rm -rf libcodb.so cosql_utils.o codb_utils.o $(OBJS)

