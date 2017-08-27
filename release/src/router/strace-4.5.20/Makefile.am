# Automake input for strace.

bin_PROGRAMS = strace
man_MANS = strace.1
bin_SCRIPTS = strace-graph

# OS is one of `linux', `sunos4', `svr4', or `freebsd'.
OS		= @opsys@
# ARCH is `i386', `m68k', `sparc', etc.
ARCH		= @arch@

AM_CFLAGS = $(WARNFLAGS)
INCLUDES = -I$(srcdir)/$(OS)/$(ARCH) -I$(srcdir)/$(OS)

strace_SOURCES = strace.c syscall.c count.c util.c desc.c file.c ipc.c \
		 io.c ioctl.c mem.c net.c process.c bjm.c quota.c \
		 resource.c signal.c sock.c system.c term.c time.c \
		 proc.c scsi.c stream.c
noinst_HEADERS = defs.h

EXTRA_DIST = $(man_MANS) errnoent.sh signalent.sh syscallent.sh ioctlsort.c \
	     debian/changelog debian/control debian/copyright debian/rules \
	     debian/compat debian/strace64.install debian/strace64.manpages \
	     debian/strace.docs debian/strace.examples debian/strace.install \
	     debian/strace.manpages debian/strace-udeb.install \
	     strace.spec \
	     strace-graph ChangeLog ChangeLog-CVS COPYRIGHT CREDITS PORTING \
	     README-freebsd README-linux README-sunos4 README-svr4 \
	     linux/ioctlsort.c linux/ioctlent.sh \
	     linux/ioctlent.h linux/errnoent.h linux/signalent.h \
	     linux/syscall.h linux/dummy.h \
	     linux/i386/syscallent.h \
	     linux/alpha/errnoent.h linux/alpha/ioctlent.h \
	     linux/alpha/signalent.h linux/alpha/syscallent.h \
	     linux/arm/syscallent.h linux/arm/syscallent1.h \
	     linux/arm/signalent1.h linux/arm/ioctlent1.h \
	     linux/arm/errnoent1.h \
	     linux/avr32/syscallent.h \
	     linux/bfin/ioctlent.h linux/bfin/syscallent.h \
	     linux/hppa/errnoent.h linux/hppa/ioctlent.h \
	     linux/hppa/signalent.h linux/hppa/syscallent.h \
	     linux/ia64/syscallent.h \
	     linux/ia64/ioctlent.h \
	     linux/m68k/syscallent.h \
	     linux/mips/ioctlent.sh linux/mips/errnoent.h \
	     linux/mips/ioctlent.h linux/mips/signalent.h \
	     linux/mips/syscallent.h \
	     linux/powerpc/syscallent.h \
	     linux/powerpc/ioctlent.h \
	     linux/s390/ioctlent.h \
	     linux/s390/syscallent.h \
	     linux/s390x/ioctlent.h \
	     linux/s390x/syscallent.h \
	     linux/sh/syscallent.h \
	     linux/sh/ioctlent.h \
	     linux/sh64/syscallent.h \
	     linux/sparc/dummy2.h \
	     linux/sparc/errnoent.h linux/sparc/errnoent1.h \
	     linux/sparc/ioctlent.h linux/sparc/ioctlent1.h \
	     linux/sparc/signalent.h linux/sparc/signalent1.h \
	     linux/sparc/syscall.h linux/sparc/syscall1.h \
	     linux/sparc/syscallent.h linux/sparc/syscallent1.h \
	     linux/sparc/gen.pl linux/sparc/syscall.h.2 \
	     linux/sparc64/dummy2.h linux/sparc64/errnoent.h \
	     linux/sparc64/errnoent1.h linux/sparc64/errnoent2.h \
	     linux/sparc64/ioctlent.h linux/sparc64/ioctlent1.h \
	     linux/sparc64/ioctlent2.h linux/sparc64/signalent.h \
	     linux/sparc64/signalent1.h linux/sparc64/signalent2.h \
	     linux/sparc64/syscall.h linux/sparc64/syscall1.h \
	     linux/sparc64/syscallent.h linux/sparc64/syscallent1.h \
	     linux/sparc64/syscallent2.h \
	     linux/tile/syscallent.h \
	     linux/x86_64/syscallent.h linux/x86_64/gentab.pl \
	     linux/x86_64/errnoent1.h linux/x86_64/ioctlent1.h \
	     linux/x86_64/signalent1.h linux/x86_64/syscallent1.h \
	     freebsd/ioctlent.sh \
	     freebsd/syscalls.cat freebsd/syscalls.pl freebsd/syscalls.print \
	     freebsd/i386/errnoent.h freebsd/i386/ioctlent.h \
	     freebsd/i386/signalent.h \
	     freebsd/i386/syscall.h freebsd/i386/syscallent.h \
	     sunos4/dummy.h sunos4/errnoent.h \
	     sunos4/ioctlent.h sunos4/ioctlent.sh sunos4/signalent.h \
	     sunos4/syscall.h sunos4/syscallent.h \
	     svr4/dummy.h svr4/errnoent.h svr4/ioctlent.h svr4/ioctlent.sh \
	     svr4/signalent.h svr4/syscall.h svr4/syscallent.h \
	     xlate.el

.PHONY: srpm
srpm: dist-bzip2
	rpmbuild --define '%_srcrpmdir .' -ts $(distdir).tar.bz2

if MAINTAINER_MODE

gen_changelog_start_date = 2009-07-08 20:00
$(srcdir)/ChangeLog: $(top_srcdir)/gitlog-to-changelog $(srcdir)/Makefile.in \
		     $(top_srcdir)/.git/refs/heads/*
	@rm -f $@.new
	(cd $(top_srcdir); \
	 ./gitlog-to-changelog --since='$(gen_changelog_start_date)'; \
	 echo; echo; echo 'See ChangeLog-CVS for older changes.' \
	) > $@.new
	chmod 444 $@.new
	mv -f $@.new $@

export TAR_OPTIONS = --owner=0 --group=0 --numeric-owner --mode=go-w,go+rX

if LINUX

IOCTLDIR = /usr/include
IOCTLASM = asm

if I386
ioctlent_h = linux/ioctlent.h
else
ioctlent_h = linux/$(ARCH)/ioctlent.h
endif

BUILT_SOURCES = $(ioctlent_h)

$(srcdir)/$(ioctlent_h): ioctlsort
	$(<D)/$(<F) > $@
ioctlsort: $(srcdir)/linux/ioctlsort.c ioctls.h ioctldefs.h
	$(filter-out -I%,$(LINK.c)) -I. -I$(IOCTLDIR) $(filter -I%,$(LINK.c)) \
	  -o $@ $<
ioctls.h: $(srcdir)/linux/ioctlent.sh
	$(SHELL) $< $(IOCTLDIR) $(IOCTLASM)
ioctldefs.h: ioctls.h ;

endif
endif
