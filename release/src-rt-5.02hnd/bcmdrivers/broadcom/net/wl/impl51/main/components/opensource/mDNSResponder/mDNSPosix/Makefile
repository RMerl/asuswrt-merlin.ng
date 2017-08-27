# -*- tab-width: 4 -*-
#
# Copyright (c) 2002-2004, Apple Computer, Inc. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without 
# modification, are permitted provided that the following conditions are met:
#
# 1.  Redistributions of source code must retain the above copyright notice, 
#     this list of conditions and the following disclaimer. 
# 2.  Redistributions in binary form must reproduce the above copyright notice, 
#     this list of conditions and the following disclaimer in the documentation 
#     and/or other materials provided with the distribution. 
# 3.  Neither the name of Apple Computer, Inc. ("Apple") nor the names of its 
#     contributors may be used to endorse or promote products derived from this 
#     software without specific prior written permission. 
#
# THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY 
# EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
# DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY 
# DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND 
# ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
# IMPORTANT NOTE: This is a Makefile for *GNU make*
# On some systems, a different program may be the default "make" command.
#
# This Makefile builds an mDNSResponder daemon and a libdns_sd.so shared library 
# for Linux. It also builds several example programs for embedded systems. 
#
# Make with no arguments to build all production targets.
# 'make DEBUG=1' to build debugging targets.
# 'make clean' or 'make clean DEBUG=1' to delete prod/debug objects & targets
# 'sudo make install [DEBUG=1]' to install mdnsd daemon and libdns_sd.
#
# Notes:
# $@ means "The file name of the target of the rule"
# $< means "The name of the first prerequisite"
# $* means "The stem with which an implicit rule matches"
# $+ means "The names of all the prerequisites, with spaces between them, exactly as given"
# For more magic automatic variables, see
# <http://www.gnu.org/software/make/manual/html_node/Automatic-Variables.html>

#############################################################################

LIBVERS = 1

DISTDIR ?= mdns-dist

COREDIR = ../mDNSCore
SHAREDDIR ?= ../mDNSShared
JDK = /usr/jdk

CC = @cc
BISON = @bison
FLEX = @flex
LD = ld -shared
CP = cp
RM = rm
LN = ln -s -f
CFLAGS_COMMON = -I$(COREDIR) -I$(SHAREDDIR) -I$(OBJDIR) -fwrapv -W -Wall -DPID_FILE=\"/var/run/mdnsd.pid\" -DMDNS_UDS_SERVERPATH=\"/var/run/mdnsd\"
CFLAGS_PTHREAD =
LINKOPTS =
LINKOPTS_PTHREAD = -lpthread
LDSUFFIX = so
JAVACFLAGS_OS = -fPIC -shared -ldns_sd

START_MDNS_SERVICE ?= yes

# Set up diverging paths for debug vs. prod builds
DEBUG=0
ifeq ($(DEBUG),1)
CFLAGS_DEBUG = -g -DMDNS_DEBUGMSGS=2 
OBJDIR = objects/debug
BUILDDIR = build/debug
STRIP = echo 
else
# We use -Os for two reasons:
# 1. We want to make small binaries, suitable for putting into hardware devices
# 2. Some of the code analysis warnings only work when some form of optimization is enabled
CFLAGS_DEBUG = -Os -DMDNS_DEBUGMSGS=0 
OBJDIR ?= objects/prod
BUILDDIR ?= build/prod
STRIP = strip -S 
endif

# Configure per-OS peculiarities
ifeq ($(os),solaris)
CFLAGS_DEBUG = -O0 -DMDNS_DEBUGMSGS=0
CFLAGS_OS = -DNOT_HAVE_DAEMON -DNOT_HAVE_SA_LEN -DNOT_HAVE_SOCKLEN_T -DNOT_HAVE_IF_NAMETOINDEX \
	 -DLOG_PERROR=0 -D_XPG4_2 -D__EXTENSIONS__ -DHAVE_BROKEN_RECVIF_NAME -DTARGET_OS_SOLARIS
CC = gcc
LD = gcc -shared
LINKOPTS = -lsocket -lnsl -lresolv
JAVACFLAGS_OS += -I$(JDK)/include/solaris
ifneq ($(DEBUG),1)
STRIP = strip
endif
else

# any target that contains the string "linux"
ifeq ($(findstring linux,$(os)),linux)
CFLAGS_OS = -D_GNU_SOURCE -DHAVE_IPV6 -DNOT_HAVE_SA_LEN -DUSES_NETLINK -DHAVE_LINUX -DTARGET_OS_LINUX -fno-strict-aliasing
LD = gcc -shared
FLEXFLAGS_OS = -l
JAVACFLAGS_OS += -I$(JDK)/include/linux

# uClibc does not support Name Service Switch
ifneq ($(os),linux-uclibc)
ifeq ($(HAVE_UCLIBC),)
OPTIONALTARG = nss_mdns
OPTINSTALL   = InstalledNSS
OPTDIST      = $(DISTDIR)/lib/$(NSSLIBFILE)
OPTDIST     += $(DISTDIR)/lib/$(NSSLINKNAME)
endif
endif

# Specific to BRCM linux-arm...
ifeq ($(os),linux-arm)
TOOLCHAIN_PREFIX := $(CROSS_COMPILE)
BUILD_DIR ?= ../$(os)-obj
BR2_STAGING ?= $(BUILD_DIR)/buildroot/output/staging
BR2_TARGET ?= $(BUILD_DIR)/buildroot/output/target
NSSINSTPATH = $(BR2_STAGING)
NSSCONFPATH = $(BR2_TARGET)/etc
INSTBASE = $(BR2_STAGING)/usr
STARTUPSCRIPTDIR = $(BR2_TARGET)/etc/init.d
RUNLEVELSCRIPTSDIR =
MANPATH = $(INSTBASE)/usr/share/man
START_MDNS_SERVICE = no
CC = $(TOOLCHAIN_PREFIX)gcc
LD = $(TOOLCHAIN_PREFIX)gcc -shared
STRIP = $(TOOLCHAIN_PREFIX)strip -S
CFLAGS_OS += -fPIC
endif
else

ifeq ($(os),netbsd)
CFLAGS_OS =
LDCONFIG = ldconfig
else

ifeq ($(os),freebsd)
# If not already defined, set LOCALBASE to /usr/local
LOCALBASE?=/usr/local
INSTBASE=$(LOCALBASE)
CFLAGS_OS = -DHAVE_IPV6
# FreeBSD 4 requires threaded code to be compiled and linked using the "-pthread" option,
# and requires that the "-lpthread" link option NOT be used
# This appies only to FreeBSD -- "man cc" on FreeBSD says:
#   FreeBSD SPECIFIC OPTIONS
#     -pthread
#       Link a user-threaded process against libc_r instead of libc.
CFLAGS_PTHREAD   = -pthread -D_THREAD_SAFE
LINKOPTS_PTHREAD = -pthread
JAVACFLAGS_OS += -I$(JDK)/include/freebsd
LDCONFIG = ldconfig
else

ifeq ($(os),openbsd)
CFLAGS_OS = -DHAVE_BROKEN_RECVDSTADDR
LDCONFIG = ldconfig
else

ifeq ($(os),x)
# We have to define __MAC_OS_X_VERSION_MIN_REQUIRED=__MAC_OS_X_VERSION_10_4 or on Leopard
# we get build failures: ‘daemon’ is deprecated (declared at /usr/include/stdlib.h:283)
CFLAGS_OS = -DHAVE_IPV6 -no-cpp-precomp -Werror -Wdeclaration-after-statement \
	-D__MAC_OS_X_VERSION_MIN_REQUIRED=__MAC_OS_X_VERSION_10_4 \
	-D__APPLE_USE_RFC_2292 #-Wunreachable-code
CC = gcc
LD = $(CC) -dynamiclib
LINKOPTS = -lSystem
LDSUFFIX = dylib
JDK = /System/Library/Frameworks/JavaVM.framework/Home
JAVACFLAGS_OS = -dynamiclib -I/System/Library/Frameworks/JavaVM.framework/Headers -framework JavaVM 
else

$(error ERROR: Must specify target OS on command-line, e.g. "make os=x [target]".\
Supported operating systems include: x, linux, linux-uclibc, netbsd, freebsd, openbsd, solaris) 
endif
endif
endif
endif
endif
endif

NSSLIBNAME  := libnss_mdns
NSSVERSION  := 0.2
NSSLIBFILE  := $(NSSLIBNAME)-$(NSSVERSION).so
NSSLINKNAME := $(NSSLIBNAME).so.2
NSSINSTPATH ?= /lib
NSSCONFPATH ?= /etc

# If not otherwise defined, we install into /usr/lib and /usr/include
# and our startup script is called mdns (e.g. /etc/init.d/mdns)
INSTBASE?=/usr
STARTUPSCRIPTNAME?=mdns

ifeq ($(HAVE_IPV6),1)
CFLAGS_OS += -DHAVE_IPV6=1
else
ifeq ($(HAVE_IPV6),0)
CFLAGS_OS += -DHAVE_IPV6=0
endif
endif

# If directory /usr/share/man exists, then we install man pages into that, else /usr/man
ifeq ($(wildcard /usr/share/man), /usr/share/man)
MANPATH ?= /usr/share/man
else
MANPATH ?= /usr/man
endif

# If directories /etc/init.d/rc*.d exist, then we install into that (Suse)
ifeq ($(wildcard /etc/init.d/rc2.d/), /etc/init.d/rc2.d/)
STARTUPSCRIPTDIR ?= /etc/init.d
RUNLEVELSCRIPTSDIR ?= /etc/init.d
else
# else if directory /etc/rc.d/init.d/ exists, then we install into that (old Linux)
ifeq ($(wildcard /etc/rc.d/init.d/), /etc/rc.d/init.d/)
STARTUPSCRIPTDIR ?= /etc/rc.d/init.d
RUNLEVELSCRIPTSDIR ?= /etc/rc.d
else
# else if directory /etc/init.d/ exists, then we install into that (new Linux)
ifeq ($(wildcard /etc/init.d/), /etc/init.d/)
STARTUPSCRIPTDIR ?= /etc/init.d
RUNLEVELSCRIPTSDIR ?= /etc
else
# else install into /etc/rc.d/ (*BSD)
STARTUPSCRIPTDIR ?= $(INSTBASE)/etc/rc.d
endif
endif
endif

CFLAGS = $(CFLAGS_COMMON) $(CFLAGS_OS) $(CFLAGS_DEBUG)

#############################################################################

all: setup Daemon libdns_sd Clients SAClient SAResponder SAProxyResponder Identify NetMonitor dnsextd $(OPTIONALTARG)

install: setup InstalledDaemon InstalledStartup InstalledLib InstalledManPages InstalledClients $(OPTINSTALL)

dist: setup DistPackage

# 'setup' sets up the build directory structure the way we want
setup:
	@if test ! -d $(OBJDIR)   ; then mkdir -p $(OBJDIR)   ; fi
	@if test ! -d $(BUILDDIR) ; then mkdir -p $(BUILDDIR) ; fi
	@if test ! -d $(DISTDIR) ; then mkdir -p $(DISTDIR) ; fi

# clean removes targets and objects
clean:
	@if test -d $(OBJDIR)   ; then rm -r $(OBJDIR)   ; fi
	@if test -d $(BUILDDIR) ; then rm -r $(BUILDDIR) ; fi
	@$(MAKE) -C ../Clients clean CC=$(CC)

#############################################################################

# daemon target builds the daemon
DAEMONOBJS = $(OBJDIR)/PosixDaemon.c.o $(OBJDIR)/mDNSPosix.c.o $(OBJDIR)/mDNSUNP.c.o $(OBJDIR)/mDNS.c.o \
             $(OBJDIR)/DNSDigest.c.o $(OBJDIR)/uDNS.c.o $(OBJDIR)/DNSCommon.c.o $(OBJDIR)/uds_daemon.c.o \
             $(OBJDIR)/mDNSDebug.c.o $(OBJDIR)/dnssd_ipc.c.o $(OBJDIR)/GenLinkedList.c.o $(OBJDIR)/PlatformCommon.c.o 

# dnsextd target build dnsextd
DNSEXTDOBJ = $(OBJDIR)/mDNSPosix.c.o $(OBJDIR)/mDNSUNP.c.o $(OBJDIR)/mDNSDebug.c.o $(OBJDIR)/GenLinkedList.c.o $(OBJDIR)/DNSDigest.c.o \
             $(OBJDIR)/DNSCommon.c.o $(OBJDIR)/PlatformCommon.c.o $(OBJDIR)/dnsextd_parser.y.o $(OBJDIR)/dnsextd_lexer.l.o

Daemon: setup $(BUILDDIR)/mdnsd
	@echo "Responder daemon done"

$(BUILDDIR)/mdnsd: $(DAEMONOBJS)
	$(CC) -o $@ $+ $(LINKOPTS)
	@$(STRIP) $@

# libdns_sd target builds the client library
libdns_sd: setup $(BUILDDIR)/libdns_sd.$(LDSUFFIX)
	@echo "Client library done"

CLIENTLIBOBJS = $(OBJDIR)/dnssd_clientlib.c.so.o $(OBJDIR)/dnssd_clientstub.c.so.o $(OBJDIR)/dnssd_ipc.c.so.o

$(BUILDDIR)/libdns_sd.$(LDSUFFIX): $(CLIENTLIBOBJS)
	@$(LD) $(LINKOPTS) -o $@ $+
	@$(STRIP) $@

Clients: setup libdns_sd ../Clients/build/dns-sd
	@echo "Clients done"

../Clients/build/dns-sd:
	@$(MAKE) -C ../Clients CC=$(CC)

# nss_mdns target builds the Name Service Switch module
nss_mdns: setup $(BUILDDIR)/$(NSSLIBFILE)
	@echo "Name Service Switch module done"

$(BUILDDIR)/$(NSSLIBFILE): $(CLIENTLIBOBJS) $(OBJDIR)/nss_mdns.c.so.o
	@$(LD) $(LINKOPTS) -o $@ $+
	@$(STRIP) $@

#############################################################################

# The Install targets place built stuff in their proper places
InstalledDaemon: $(INSTBASE)/sbin/mdnsd
	@echo $+ " installed"

InstalledLib: $(INSTBASE)/lib/libdns_sd.$(LDSUFFIX).$(LIBVERS) $(INSTBASE)/include/dns_sd.h
	@echo $+ " installed"

InstalledStartup: $(STARTUPSCRIPTDIR)/$(STARTUPSCRIPTNAME)
	@echo $+ " installed"

InstalledManPages: $(MANPATH)/man8/mdnsd.8
	@echo $+ " installed"

InstalledClients: $(INSTBASE)/bin/dns-sd
	@echo $+ " installed"

InstalledNSS: $(NSSINSTPATH)/$(NSSLINKNAME) $(NSSCONFPATH)/nss_mdns.conf $(MANPATH)/man5/nss_mdns.conf.5 $(MANPATH)/man8/libnss_mdns.8
	@echo $+ " installed"

# Adding distribution package
DistPackage: \
	$(DISTDIR)/usr/sbin/mdnsd \
	$(DISTDIR)/usr/lib/libdns_sd.$(LDSUFFIX).$(LIBVERS) \
	$(DISTDIR)/etc/init.d/$(STARTUPSCRIPTNAME) \
	$(DISTDIR)/usr/bin/dns-sd \
	$(OPTDIST) \
	$(DISTDIR)/etc/nss_mdns.conf
	@echo $+ " ready for distribution !"

$(DISTDIR)/usr/sbin/mdnsd: $(BUILDDIR)/mdnsd
	mkdir -p $(dir $@)
	$(CP) $< $@

$(DISTDIR)/usr/lib/libdns_sd.$(LDSUFFIX).$(LIBVERS): $(BUILDDIR)/libdns_sd.$(LDSUFFIX)
	mkdir -p $(dir $@)
	$(CP) $< $@
	cd $(dir $@) && $(LN) $(notdir $@) libdns_sd.$(LDSUFFIX)

$(DISTDIR)/etc/init.d/$(STARTUPSCRIPTNAME): mdnsd.sh
	mkdir -p $(dir $@)
	$(CP) $< $@
	chmod ugo+x $@
	cd $(dir $@) && $(LN) $(notdir $@) S52mdns

$(DISTDIR)/usr/bin/dns-sd: ../Clients/build/dns-sd
	mkdir -p $(dir $@)
	$(CP) $< $@

$(DISTDIR)/lib/$(NSSLINKNAME): $(DISTDIR)/lib/$(NSSLIBFILE)
	cd $(dir $@) && $(LN) $(notdir $<) $(notdir $@)

$(DISTDIR)/lib/$(NSSLIBFILE): $(BUILDDIR)/$(NSSLIBFILE)
	mkdir $(dir $@)
	$(CP) $< $@
	chmod 444 $@

$(DISTDIR)/etc/nss_mdns.conf: nss_mdns.conf
	mkdir -p $(dir $@)
	$(CP) $< $@
	chmod 444 $@

# Note: If daemon already installed, we make sure it's stopped before overwriting it
$(INSTBASE)/sbin/mdnsd: $(BUILDDIR)/mdnsd
ifeq ($(START_MDNS_SERVICE),yes)
	@if test -x $@; then $(STARTUPSCRIPTDIR)/$(STARTUPSCRIPTNAME) stop; fi
endif
	$(CP) $< $@

$(INSTBASE)/lib/libdns_sd.$(LDSUFFIX).$(LIBVERS): $(BUILDDIR)/libdns_sd.$(LDSUFFIX)
	$(CP) $< $@
	$(LN) $@ $(INSTBASE)/lib/libdns_sd.$(LDSUFFIX)
ifdef LDCONFIG
    # -m means 'merge into existing database', -R means 'rescan directories'
	$(LDCONFIG) -mR
endif

$(INSTBASE)/include/dns_sd.h: $(SHAREDDIR)/dns_sd.h
	$(CP) $< $@

# We make this target dependent on $(INSTBASE)/sbin/mdnsd because we need to ensure
# that the daemon is installed *before* we try to execute the command to start it.
$(STARTUPSCRIPTDIR)/$(STARTUPSCRIPTNAME): mdnsd.sh $(STARTUPSCRIPTDIR) $(INSTBASE)/sbin/mdnsd
	$(CP) $< $@
	chmod ugo+x $@
ifeq ($(START_MDNS_SERVICE),yes)
	$@ start
endif
ifdef RUNLEVELSCRIPTSDIR
ifeq ($(wildcard $(RUNLEVELSCRIPTSDIR)/runlevels/default), $(RUNLEVELSCRIPTSDIR)/runlevels/default)
	$(LN) $@ $(RUNLEVELSCRIPTSDIR)/runlevels/default/mdns
else
	$(LN) $@ $(RUNLEVELSCRIPTSDIR)/rc2.d/S52mdns
	$(LN) $@ $(RUNLEVELSCRIPTSDIR)/rc3.d/S52mdns
	$(LN) $@ $(RUNLEVELSCRIPTSDIR)/rc4.d/S52mdns
	$(LN) $@ $(RUNLEVELSCRIPTSDIR)/rc5.d/S52mdns
	$(LN) $@ $(RUNLEVELSCRIPTSDIR)/rc0.d/K16mdns
	$(LN) $@ $(RUNLEVELSCRIPTSDIR)/rc6.d/K16mdns
endif
endif

$(MANPATH)/man5/%.5: %.5
	mkdir -p $(dir $@)
	cp $< $@
	chmod 444 $@

$(MANPATH)/man8/%.8: %.8
	mkdir -p $(dir $@)
	cp $< $@
	chmod 444 $@

$(MANPATH)/man8/mdnsd.8: $(SHAREDDIR)/mDNSResponder.8
	mkdir -p $(dir $@)
	cp $< $@
	chmod 444 $@

$(INSTBASE)/bin/dns-sd: ../Clients/build/dns-sd
	$(CP) $< $@

$(NSSINSTPATH)/$(NSSLINKNAME): $(NSSINSTPATH)/$(NSSLIBFILE)
	$(LN) $< $@
ifdef LDCONFIG
	ldconfig
endif

$(NSSINSTPATH)/$(NSSLIBFILE): $(BUILDDIR)/$(NSSLIBFILE)
	$(CP) $< $@
	chmod 444 $@

$(NSSCONFPATH)/nss_mdns.conf: nss_mdns.conf
	$(CP) $< $@
	chmod 444 $@
	# Check the nsswitch.conf file.
	# If 'mdns' does not already appear on the "hosts:" line, then add it right before 'dns'
	cp -f $(NSSCONFPATH)/nsswitch.conf $(NSSCONFPATH)/nsswitch.conf.pre-mdns
	sed -e '/mdns/!s/^\(hosts:.*\)dns\(.*\)/\1mdns dns\2/' $(NSSCONFPATH)/nsswitch.conf.pre-mdns > $(NSSCONFPATH)/nsswitch.conf

#############################################################################

# The following targets build Java wrappers for the dns-sd.h API.
# Note that the JavaForXcode targets are used when building the project for OS X using Xcode

JAVAC = $(JDK)/bin/javac
JAVAH = $(JDK)/bin/javah
JAVADOC = $(JDK)/bin/javadoc
JAR = $(JDK)/bin/jar
JAVACFLAGS = $(CFLAGS) $(JAVACFLAGS_OS) -I$(JDK)/include

JavaForXcode_: setup $(BUILDDIR)/dns_sd.jar $(PROJECT_DERIVED_FILE_DIR)/DNSSD.java.h
	@echo $@ done

$(PROJECT_DERIVED_FILE_DIR)/DNSSD.java.h: $(OBJDIR)/DNSSD.java.h
	@if test ! -d $(PROJECT_DERIVED_FILE_DIR) ; then mkdir -p $(PROJECT_DERIVED_FILE_DIR) ; fi
	$(CP) $< $@

JavaForXcode_clean:
	@if test -d $(OBJDIR) ; then rm -r $(OBJDIR) ; fi
	@if test -f $(PROJECT_DERIVED_FILE_DIR)/DNSSD.java.h ; then $(RM) $(PROJECT_DERIVED_FILE_DIR)/DNSSD.java.h ; fi
	@if test -f $(BUILDDIR)/dns_sd.jar ; then $(RM) $(BUILDDIR)/dns_sd.jar ; fi
	@echo $@ done

JavaForXcode_installhdrs:
	@echo $@ NOOP

JavaForXcode_install: JavaForXcode_ $(DSTROOT)/$(SYSTEM_LIBRARY_DIR)/Java/Extensions/dns_sd.jar
	@echo $@ done

$(DSTROOT)/$(SYSTEM_LIBRARY_DIR)/Java/Extensions/dns_sd.jar: $(BUILDDIR)/dns_sd.jar
	@if test ! -d $(DSTROOT)/$(SYSTEM_LIBRARY_DIR)/Java/Extensions ; then mkdir -p $(DSTROOT)/$(SYSTEM_LIBRARY_DIR)/Java/Extensions ; fi
	$(CP) $< $@

Java: setup $(BUILDDIR)/dns_sd.jar $(BUILDDIR)/libjdns_sd.$(LDSUFFIX)
	@echo "Java wrappers done"

JAVASRC	= $(SHAREDDIR)/Java
JARCONTENTS =	$(OBJDIR)/com/apple/dnssd/DNSSDService.class \
				$(OBJDIR)/com/apple/dnssd/DNSSDException.class \
				$(OBJDIR)/com/apple/dnssd/DNSRecord.class \
				$(OBJDIR)/com/apple/dnssd/TXTRecord.class \
				$(OBJDIR)/com/apple/dnssd/DNSSDRegistration.class \
				$(OBJDIR)/com/apple/dnssd/BaseListener.class \
				$(OBJDIR)/com/apple/dnssd/BrowseListener.class \
				$(OBJDIR)/com/apple/dnssd/ResolveListener.class \
				$(OBJDIR)/com/apple/dnssd/RegisterListener.class \
				$(OBJDIR)/com/apple/dnssd/QueryListener.class \
				$(OBJDIR)/com/apple/dnssd/DomainListener.class \
				$(OBJDIR)/com/apple/dnssd/RegisterRecordListener.class \
				$(OBJDIR)/com/apple/dnssd/DNSSDRecordRegistrar.class \
				$(OBJDIR)/com/apple/dnssd/DNSSD.class

$(BUILDDIR)/dns_sd.jar: $(JARCONTENTS) setup
	$(JAR) -cf $@ -C $(OBJDIR) com

$(BUILDDIR)/libjdns_sd.$(LDSUFFIX): $(JAVASRC)/JNISupport.c $(OBJDIR)/DNSSD.java.h setup libdns_sd
	$(CC) -o $@ $< $(JAVACFLAGS) -I$(OBJDIR) -L$(BUILDDIR)

$(OBJDIR)/com/apple/dnssd/%.class:	$(JAVASRC)/%.java
	$(JAVAC) -d $(OBJDIR) -classpath $(OBJDIR) $<

$(OBJDIR)/DNSSD.java.h: $(OBJDIR)/com/apple/dnssd/DNSSD.class
	$(JAVAH) -force -classpath $(OBJDIR) -o $@ \
		com.apple.dnssd.AppleDNSSD \
		com.apple.dnssd.AppleBrowser \
		com.apple.dnssd.AppleResolver \
		com.apple.dnssd.AppleRegistration \
		com.apple.dnssd.AppleQuery \
		com.apple.dnssd.AppleDomainEnum \
		com.apple.dnssd.AppleService \
		com.apple.dnssd.AppleDNSRecord \
		com.apple.dnssd.AppleRecordRegistrar

#############################################################################

# The following target builds documentation for the Java wrappers.

JavaDoc: Java setup
	$(JAVADOC) $(JAVASRC)/*.java -classpath $(OBJDIR) -d $(BUILDDIR) -public

#############################################################################

# The following targets build embedded example programs
SPECIALOBJ = $(OBJDIR)/mDNSPosix.c.o $(OBJDIR)/mDNSUNP.c.o $(OBJDIR)/mDNSDebug.c.o $(OBJDIR)/GenLinkedList.c.o \
	$(OBJDIR)/DNSDigest.c.o $(OBJDIR)/uDNS.c.o $(OBJDIR)/DNSCommon.c.o $(OBJDIR)/PlatformCommon.c.o
COMMONOBJ  = $(SPECIALOBJ) $(OBJDIR)/mDNS.c.o
APPOBJ     = $(COMMONOBJ) $(OBJDIR)/ExampleClientApp.c.o

SAClient: setup $(BUILDDIR)/mDNSClientPosix
	@echo "Embedded Standalone Client done"

SAResponder: setup $(BUILDDIR)/mDNSResponderPosix
	@echo "Embedded Standalone Responder done"

SAProxyResponder: setup $(BUILDDIR)/mDNSProxyResponderPosix
	@echo "Embedded Standalone ProxyResponder done"

Identify: setup $(BUILDDIR)/mDNSIdentify
	@echo "Identify done"

NetMonitor: setup $(BUILDDIR)/mDNSNetMonitor
	@echo "NetMonitor done"

dnsextd: setup $(BUILDDIR)/dnsextd
	@echo "dnsextd done"

$(BUILDDIR)/mDNSClientPosix:         $(APPOBJ)     $(OBJDIR)/Client.c.o
	$(CC) $+ -o $@ $(LINKOPTS)

$(BUILDDIR)/mDNSResponderPosix:      $(COMMONOBJ)  $(OBJDIR)/Responder.c.o
	$(CC) $+ -o $@ $(LINKOPTS)

$(BUILDDIR)/mDNSProxyResponderPosix: $(COMMONOBJ)  $(OBJDIR)/ProxyResponder.c.o
	$(CC) $+ -o $@ $(LINKOPTS)

$(BUILDDIR)/mDNSIdentify:            $(SPECIALOBJ) $(OBJDIR)/Identify.c.o
	$(CC) $+ -o $@ $(LINKOPTS)

$(OBJDIR)/Identify.c.o:              $(COREDIR)/mDNS.c # Note: Identify.c textually imports mDNS.c

$(BUILDDIR)/mDNSNetMonitor:          $(SPECIALOBJ) $(OBJDIR)/NetMonitor.c.o
	$(CC) $+ -o $@ $(LINKOPTS)

$(OBJDIR)/NetMonitor.c.o:            $(COREDIR)/mDNS.c # Note: NetMonitor.c textually imports mDNS.c

$(BUILDDIR)/dnsextd:                 $(DNSEXTDOBJ) $(OBJDIR)/dnsextd.c.threadsafe.o
	$(CC) $+ -o $@ $(LINKOPTS) $(LINKOPTS_PTHREAD)

#############################################################################

# Implicit rules
$(OBJDIR)/%.c.o:	%.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJDIR)/%.c.o:	$(COREDIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJDIR)/%.c.o:	$(SHAREDDIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJDIR)/%.c.threadsafe.o:	%.c
	$(CC) $(CFLAGS) $(CFLAGS_PTHREAD) -D_REENTRANT -c -o $@ $<

$(OBJDIR)/%.c.threadsafe.o:	$(SHAREDDIR)/%.c
	$(CC) $(CFLAGS) $(CFLAGS_PTHREAD) -D_REENTRANT -c -o $@ $<

$(OBJDIR)/%.c.so.o:	%.c
	$(CC) $(CFLAGS) -c -fPIC -o $@ $<

$(OBJDIR)/%.c.so.o:	$(SHAREDDIR)/%.c
	$(CC) $(CFLAGS) -c -fPIC -o $@ $<

$(OBJDIR)/%.y.o: $(SHAREDDIR)/%.y
	$(BISON)              -o $(OBJDIR)/$*.c -d $<
	$(CC) $(CFLAGS) -c -o $@ $(OBJDIR)/$*.c

$(OBJDIR)/%.l.o: $(SHAREDDIR)/%.l
	$(FLEX) $(FLEXFLAGS_OS) -i             -o$(OBJDIR)/$*.l.c $<
	$(CC) $(CFLAGS) -Wno-error -c -o $@ $(OBJDIR)/$*.l.c
