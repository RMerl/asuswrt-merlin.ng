#
# This is the Makefile for the GPN client library.
#
# Copyright (C) 2014-2015, AAA Internet Publishing, Inc. (DBA WTFast)
#

CFLAGS += --std=c99 -Wall -Wextra -pedantic-errors -O -g -fpic
CPPFLAGS += -I$(INC)
LDFLAGS += -Wl,-soname,$(SONAME)
LIBS = -lcrypto -lgpnutils

LIB_VER_MAJOR = 1
LIB_VER_MINOR = 1
LIB_VER_MICRO = 0
LIB_VER_STRING = "\"1.1.0\""
CFLAGS += -DLIB_VER_MAJOR=$(LIB_VER_MAJOR) -DLIB_VER_MINOR=$(LIB_VER_MINOR) -DLIB_VER_MICRO=$(LIB_VER_MICRO) -DLIB_VER_STRING=$(LIB_VER_STRING)
SONAME = $(LIB_NAME).$(LIB_VER_MAJOR)

LIB_SRCS = libgpnclient.c
LIB_OBJS = $(LIB_SRCS:.c=.o)
LIB_HDRS = $(LIB_SRCS:.c=.h) libgpnclient_impl.h
LIB_NAME = $(LIB_SRCS:.c=.so)
STATIC_LIB_TARGET = $(LIB_SRCS:.c=.a)

TEST_SRCS = test_libgpnclient.c
TEST_OBJS = $(TEST_SRCS:.c=.o)

LIB_TARGET = $(LIB_NAME).$(LIB_VER_MAJOR).$(LIB_VER_MINOR).$(LIB_VER_MICRO)
TEST_TARGETS = test_libgpnclient

OBJS = $(LIB_OBJS) $(TEST_OBJS)
TARGETS = $(STATIC_LIB_TARGET)

LIB = $(DESTDIR)/usr/lib
INC = $(DESTDIR)/usr/include

.PHONY: all clean tags install test

all: $(TARGETS)

$(LIB_TARGET): $(LIB_OBJS)
	$(CC) -shared $(LDFLAGS) -o $@ $(LIB_OBJS) $(LIBS)

$(TEST_TARGETS): $(TEST_OBJS) $(LIB_OBJS)
	$(CC) -o $@ $(LDFLAGS) $(TEST_OBJS) $(LIB_OBJS) $(LIBS)

$(STATIC_LIB_TARGET): $(LIB_OBJS)
	$(AR) rcs $@ $(LIB_OBJS)

clean:
	rm -f $(TARGETS) $(OBJS)

tags:
	ctags -t $(LIB_SRCS) $(LIB_HDRS)

test: $(TEST_TARGETS)
#	./test_harness $(VERBOSE)

install:
	install -d -o 0 -g 0 -m 755 $(LIB)
	install -d -o 0 -g 0 -m 755 $(INC)
	install -o 0 -g 0 -m 644 $(LIB_TARGET) $(LIB)
	install -o 0 -g 0 -m 644 libgpnclient.h $(INC)
	ln -sf ./$(LIB_TARGET) $(LIB)/$(SONAME)
	ln -sf ./$(LIB_TARGET) $(LIB)/$(LIB_NAME)

# asuswrt
PREBUILT_SRCS = $(filter-out $(wildcard $(LIB_SRCS)),$(LIB_SRCS))
ifneq ($(PREBUILT_SRCS),)
include ../../common.mak
ifeq ($(RTCONFIG_BCMARM),y)
PREBUILT_ARCH := $(BCMEX)$(EX7)
endif
$(PREBUILT_SRCS:.c=.o): %.o: prebuilt$(PREBUILT_ARCH)/$(STATIC_LIB_TARGET)
	$(AR) xo $< $@
endif

#
# Tribute to a long-forgotten hack...
#
love:
	@echo Not war?
