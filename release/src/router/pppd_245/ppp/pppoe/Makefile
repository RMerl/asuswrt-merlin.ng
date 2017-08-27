#
# pppd makefile for Linux
# $Id: Makefile.linux,v 1.41 2001/03/08 05:00:35 paulus Exp $
#

# Default installation locations
BINDIR = /usr/sbin
MANDIR = /usr/man

#cwu
ATMLIBDIR = $(BUILD_DIR)/userspace/gpl/apps/atm2684/atm/lib

PPPDSRCS = main.c magic.c fsm.c lcp.c ipcp.c upap.c chap.c md5.c \
	   auth.c options.c sys-linux.c md4.c chap_ms.c cbcp.c \
	   demand.c utils.c tdb.c
HEADERS =  callout.h pathnames.h patchlevel.h chap.h md5.h chap_ms.h md4.h \
	   ipxcp.h cbcp.h tdb.h
MANPAGES = pppd.8
PPPDOBJS = main.o magic.o fsm.o lcp.o ipcp.o upap.o chap.o md5.o \
	   auth.o options.o demand.o utils.o sys-linux.o \
	   tdb.o \
	    pppoe.o pppoehash.o pppoe_client.o libpppoe.o \
	    pppoe_utils.o openl2tp.o pppol2tp.o tty.o ccp.o
	    #pppoe_utils.o pppoatm.o openl2tp.o pppol2tp.o tty.o ccp.o
vpath	%.c .
vpath	%.c ./plugins/pppoe
vpath	%.h .
vpath	%.h ./include
#cwu

#all: pppd
# cwu
all: pppd.a
#all: pppd

#
# include dependancies if present and backup if as a header file
ifeq (.depend,$(wildcard .depend))
include .depend
endif

#
# COPTS = -O2 -pipe -Wall -g
#COPTS = -O2 -pipe -Wall -DDEBUG
COPTS = -s -pipe -Wall -Os -fno-strict-aliasing 
COPTS += $(BRCM_WERROR_CFLAGS)
#LIBS = $(ATMLIBDIR)/libatm.a
LIBS = 

ifneq ($(wildcard $(LIBCDIR)/lib/libcrypt.*),)
LIBS += -lcrypt
#LIBS += $(LIBCDIR)/lib/libcrypt.a
endif

# Uncomment the next 2 lines to include support for Microsoft's
# MS-CHAP authentication protocol.
# cwu
CHAPMS=y
USE_CRYPT=y
#ifneq ($(wildcard /usr/lib/libcrypt.*),)
#HAVE_CRYPT_H=y
#endif

# Uncomment the next line to include support for PPP packet filtering.
# This requires that the libpcap library and headers be installed
# and that the kernel driver support PPP packet filtering, which it
# doesn't yet.
#FILTER=y

#HAS_SHADOW=y
#USE_PAM=y
#ifneq ($(strip $(BUILD_IPV6)),)
HAVE_INET6=y
#endif

#PLUGIN=y

# cwu
INCLUDE_DIRS= -I. -I../include \
              -I$(KERNEL_LINKS_DIR) \
              -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD) \
              -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD) \
              -I$(ATMLIBDIR) \
              -I$(BUILD_DIR)/userspace/public/include \
              -I$(BUILD_DIR)/userspace/public/include/$(OALDIR)
#-I/opt/sgi17/linux/include

COMPILE_FLAGS= -D_linux_=1 -DHAVE_PATHS_H -DHAVE_MMAP -DCONFIG_MIPS_BRCM
ifeq ($(strip $(BUILD_PPPD)), static)
COMPILE_FLAGS += -DBUILD_STATIC
endif

CFLAGS= $(COPTS) $(COMPILE_FLAGS) $(INCLUDE_DIRS)
CFLAGS+=-I$(TOOLCHAIN)/include

ifdef CHAPMS
CFLAGS   += -DCHAPMS=1
ifndef USE_CRYPT
LIBS     := -ldes $(LIBS)
else
CFLAGS   += -DUSE_CRYPT=1
ifneq ($(wildcard /usr/include/crypt.h),)
CFLAGS   += -DHAVE_CRYPT_H=1
endif
endif
PPPDOBJS += md4.o chap_ms.o
ifdef MSLANMAN
CFLAGS   += -DMSLANMAN=1
endif
endif

ifdef HAS_SHADOW
CFLAGS   += -DHAS_SHADOW
#LIBS     := -lshadow $(LIBS)
endif

# For "Pluggable Authentication Modules", see ftp.redhat.com:/pub/pam/.
ifdef USE_PAM
CFLAGS   += -DUSE_PAM
LIBS     := -lpam -ldl $(LIBS)
endif

# Lock library binary for Linux is included in 'linux' subdirectory.
ifdef LOCKLIB
LIBS     := -llock $(LIBS)
CFLAGS   += -DLOCKLIB=1
endif

ifdef PLUGIN
CFLAGS	+= -DPLUGIN
LDFLAGS	+= -Wl,-E
LIBS	+= -ldl
endif

ifdef FILTER
LIBS    += -lpcap
CFLAGS  += -DPPP_FILTER -I/usr/include/pcap
endif


LIBS   += -L$(INSTALL_DIR)/lib -L$(INSTALL_DIR)/lib
ifneq ($(strip $(BUILD_BRCM_CMS)),)
LIBS   += -lcms_msg $(CMS_COMMON_LIBS)
CFLAGS         += -DBRCM_CMS_BUILD -DCMS_LOG3
else
LIBS    += -lcrypt
endif

# treat all warnings as errors
#CFLAGS += -Werror -Wfatal-errors -Wno-aggressive-loop-optimizations

ifdef HAVE_INET6
     PPPDSRCS += ipv6cp.c eui64.c
     HEADERS  += ipv6cp.h eui64.h
     PPPDOBJS += ipv6cp.o eui64.o
     CFLAGS   += -DINET6=1
endif

ifeq ($(strip $(PPP_AUTODISCONN)),y)
CFLAGS += -DAUTODISCONN
endif

INSTALL= install -o root

static: pppd.a

#all dynamic: libatm pppd install
all dynamic: pppd install

# mwang: ppp should not trigger build of libatm.
# top level Makefile should build everything in the right order
libatm:
	$(MAKE) -C $(ATMLIBDIR) $(MFLAGS) all

install:
	@echo install
#install: pppd
	#mkdir -p $(INSTALL_DIR)/bin
	#install -m 755 pppd $(INSTALL_DIR)/bin/pppd_245
	#$(STRIP) $(INSTALL_DIR)/bin/pppd_245

#install: pppd
#	mkdir -p $(BINDIR) $(MANDIR)
#	$(INSTALL) -s -c -m 555 pppd $(BINDIR)/pppd
#	if chgrp pppusers $(BINDIR)/pppd 2>/dev/null; then \
#	  chmod o-rx,u+s $(BINDIR)/pppd; fi
#	$(INSTALL) -c -m 444 pppd.8 $(MANDIR)/man8

# cwu
pppd.a: $(PPPDOBJS)
	$(AR) rcs pppd.a  $(PPPDOBJS)

# cwu
#pppd: $(PPPDOBJS) libatm
pppd: $(PPPDOBJS)
#	$(CC)  -static $(CFLAGS) $(LDFLAGS) -Wl,--allow-shlib-undefined -o pppd $(PPPDOBJS) $(LIBS)
	$(CC)  $(CFLAGS) $(LDFLAGS) -Wl,--allow-shlib-undefined -o pppd $(PPPDOBJS) $(LIBS)
	#$(CC)  $(CFLAGS) $(LDFLAGS) -Wl,--allow-shlib-undefined -o pppd $(PPPDOBJS)

#	$(CC) -Wl,--allow-shlib-undefined $(CFLAGS) $(LDFLAGS) -v -o pppd.o $(PPPDOBJS) $(LIBS)
#	$(LD) --allow-shlib-undefined -o $@ pppd.o $(LIBS)

clean:
	rm -f $(PPPDOBJS) pppd pppd.a *~ #* core

depend:
	$(CPP) -M $(CFLAGS) $(PPPDSRCS) >.depend
