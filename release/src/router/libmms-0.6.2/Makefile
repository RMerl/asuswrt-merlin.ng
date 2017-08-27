#
# Makefile for libmms
#
include ../common.mak
include $(SRCBASE)/.config
TOPDIR=../
#include $(TOPDIR)Rules.mak

GLIBROOT=$(TOP)/glib-2.35.1
RUNTIMELIB=$(TOP)/lib

MAJOR_VERSION=0
MINOR_VERSION=0
SUBLEVEL=2

LIBMAD=libmms.a
LIBMAD_SHARED=libmms.so
LIBMAD_SHARED_FULLNAME=libmms.so.$(MAJOR_VERSION).$(MINOR_VERSION).$(SUBLEVEL)

CSRC=mms.c mmsh.c mmsx.c uri.c
OBJS=$(patsubst %.c,%.o, $(CSRC))

CFLAGS = -Wall -g -O -fPIC -DPIC
CFLAGS += -DHAVE_CONFIG_H -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64
#CFLAGS += -I. -I$(GLIBROOT)/include
CFLAGS += -I. -I$(TOP)/glib-2.35.1/glib -I$(TOP)/glib-2.35.1
LFLAGS = -L$(RUNTIMELIB) -lglib-2.0 -lc 



all : $(LIBMAD) $(LIBMAD_SHARED_FULLNAME)
	#$(LD) $(LDFLAGS) -soname=$(LIBMAD_SHARED).$(MAJOR_VERSION) \
	#	-o $(LIBMAD_SHARED_FULLNAME) --whole-archive $(LIBMAD) \
	#	--no-whole-archive $(TOPDIR)libc/misc/internals/interp.o \
	#	$(LFLAGS) $(LDADD_LIBFLOAT) $(LIBGCC);
	ln -sf $(LIBMAD_SHARED_FULLNAME) $(LIBMAD_SHARED)
	ln -sf $(LIBMAD_SHARED_FULLNAME) $(LIBMAD_SHARED).$(MAJOR_VERSION)
	install -d $(TOP)/lib
	$(RM) $(TOP)/lib/$(LIBMAD_SHARED_FULLNAME) $(TOP)/lib/$(LIBMAD_SHARED).$(MAJOR_VERSION)
	install -m 644 $(LIBMAD_SHARED_FULLNAME) $(TOP)/lib
	ln -sf $(LIBMAD_SHARED_FULLNAME) $(TOP)/lib/$(LIBMAD_SHARED)
	ln -sf $(LIBMAD_SHARED_FULLNAME) $(TOP)/lib/$(LIBMAD_SHARED).$(MAJOR_VERSION)

$(LIBMAD_SHARED_FULLNAME): $(OBJS)
	@$(CC)  -lc -shared $(CFLAGS) -o $@ $^

install:
	install -D libmms.so.0.0.2 $(INSTALLDIR)/lib/libmms.so.0.0.2
	$(STRIP) $(INSTALLDIR)/lib/*.so.*
	cd $(INSTALLDIR)/lib && \
	ln -sf libmms.so.0.0.2 libmms.so.0 && \
	ln -sf libmms.so.0.0.2 libmms.so

$(LIBMAD) ar-target: $(OBJS)
	$(AR) $(ARFLAGS) $(LIBMAD) $(OBJS)
	install -d $(TOP)/lib
	$(RM) $(TOP)/lib/$(LIBMAD)
	install -m 644 $(LIBMAD) $(TOP)/lib/

$(OBJS): %.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@
	$(STRIP) -x -R .note -R .comment $*.o


clean:
	$(RM) *.[oa] *~ core $(LIBMAD_SHARED)* $(LIBMAD_SHARED_FULLNAME)*
