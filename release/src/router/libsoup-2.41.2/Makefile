#
# Makefile for glib
#
include ../common.mak
include $(SRCBASE)/.config
MYROOT=$(CURDIR)
LIBSOUPROOT=$(MYROOT)
TOPDIR=$(CURDIR)/..
GLIBROOT=$(TOP)/glib-2.35.1
XML2ROOT=$(TOP)/libxml2-2.8.0
#XML2ROOT=$(TOP)/libxml2
SUBDIRS = libsoup
#export NM=$(CONFIG_CROSS_COMPILER_PATH)/mipsel-linux-nm
export TOPDIR GLIBROOT LIBSOUPROOT XML2ROOT

all shared :
	@for X in $(SUBDIRS) ; do \
		$(MAKE) -C $$X; done
	@for X in $(SUBDIRS) ; do \
		cd $$X/.libs; \
		$(STRIP) *.so*; \
		cp -af *.so* $(TOPDIR)/lib; \
		cd $(MYROOT); done

install:	
		install -D libsoup/.libs/libsoup-2.4.so.1.5.0 $(INSTALLDIR)/lib/libsoup-2.4.so.1.5.0
		$(STRIP) $(INSTALLDIR)/lib/*.so.*
		cd $(INSTALLDIR)/lib && \
		ln -sf libsoup-2.4.so.1.5.0 libsoup-2.4.so.1 && \
		ln -sf libsoup-2.4.so.1.5.0 libsoup-2.4.so

clean:
	@for X in $(SUBDIRS) ; do \
		$(MAKE) -C $$X clean; done

.PHONY: dummy all clean
