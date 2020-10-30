# Automake input for strace.

SUBDIRS = tests

bin_PROGRAMS = strace
man_MANS = strace.1
bin_SCRIPTS = strace-graph strace-log-merge

OS		= linux
# ARCH is `i386', `m68k', `sparc', etc.
ARCH		= @arch@

ACLOCAL_AMFLAGS = -I m4
AM_CFLAGS = $(WARN_CFLAGS)
AM_CPPFLAGS = -I$(srcdir)/$(OS)/$(ARCH) -I$(srcdir)/$(OS) -I$(builddir)/$(OS)

strace_SOURCES =	\
	bjm.c		\
	block.c		\
	count.c		\
	desc.c		\
	file.c		\
	io.c		\
	ioctl.c		\
	ipc.c		\
	loop.c		\
	mem.c		\
	mtd.c		\
	net.c		\
	pathtrace.c	\
	process.c	\
	quota.c		\
	resource.c	\
	scsi.c		\
	signal.c	\
	sock.c		\
	strace.c	\
	stream.c	\
	syscall.c	\
	system.c	\
	term.c		\
	time.c		\
	util.c		\
	vsprintf.c

noinst_HEADERS = defs.h
# Enable this to get link map generated
#strace_CFLAGS = $(AM_CFLAGS) -Wl,-Map=strace.mapfile

EXTRA_DIST =				\
	$(man_MANS)			\
	.version			\
	COPYING				\
	CREDITS				\
	ChangeLog			\
	ChangeLog-CVS			\
	README-linux			\
	README-linux-ptrace		\
	debian/changelog		\
	debian/compat			\
	debian/control			\
	debian/copyright		\
	debian/rules			\
	debian/source/format		\
	debian/strace-udeb.install	\
	debian/strace.docs		\
	debian/strace.examples		\
	debian/strace.install		\
	debian/strace.manpages		\
	debian/strace64.install		\
	debian/strace64.manpages	\
	debian/watch			\
	errnoent.sh			\
	ioctlsort.c			\
	linux/aarch64/errnoent1.h	\
	linux/aarch64/ioctlent.h.in	\
	linux/aarch64/ioctlent1.h	\
	linux/aarch64/signalent1.h	\
	linux/aarch64/syscallent.h	\
	linux/aarch64/syscallent1.h	\
	linux/alpha/errnoent.h		\
	linux/alpha/ioctlent.h.in	\
	linux/alpha/signalent.h		\
	linux/alpha/syscallent.h	\
	linux/arm/ioctlent.h.in		\
	linux/arm/syscallent.h		\
	linux/avr32/ioctlent.h.in	\
	linux/avr32/syscallent.h	\
	linux/bfin/ioctlent.h.in	\
	linux/bfin/syscallent.h		\
	linux/dummy.h			\
	linux/errnoent.h		\
	linux/hppa/errnoent.h		\
	linux/hppa/ioctlent.h.in	\
	linux/hppa/signalent.h		\
	linux/hppa/syscallent.h		\
	linux/i386/ioctlent.h.in	\
	linux/i386/syscallent.h		\
	linux/ia64/ioctlent.h.in	\
	linux/ia64/signalent.h		\
	linux/ia64/syscallent.h		\
	linux/ioctlent-filter.awk	\
	linux/ioctlent.h.in		\
	linux/ioctlent.sh		\
	linux/ioctlsort.c		\
	linux/m68k/ioctlent.h.in	\
	linux/m68k/syscallent.h		\
	linux/metag/ioctlent.h.in	\
	linux/metag/syscallent.h	\
	linux/microblaze/ioctlent.h.in	\
	linux/microblaze/syscallent.h	\
	linux/mips/errnoent.h		\
	linux/mips/ioctlent.h.in	\
	linux/mips/ioctlent.sh		\
	linux/mips/signalent.h		\
	linux/mips/syscallent-compat.h	\
	linux/mips/syscallent-n32.h	\
	linux/mips/syscallent-n64.h	\
	linux/mips/syscallent-o32.h	\
	linux/mips/syscallent.h		\
	linux/mtd-abi.h			\
	linux/or1k/ioctlent.h.in	\
	linux/or1k/syscallent.h		\
	linux/powerpc/errnoent1.h	\
	linux/powerpc/ioctlent.h.in	\
	linux/powerpc/ioctlent1.h	\
	linux/powerpc/signalent1.h	\
	linux/powerpc/syscallent.h	\
	linux/powerpc/syscallent1.h	\
	linux/s390/ioctlent.h.in	\
	linux/s390/syscallent.h		\
	linux/s390x/ioctlent.h.in	\
	linux/s390x/syscallent.h	\
	linux/sh/ioctlent.h.in		\
	linux/sh/syscallent.h		\
	linux/sh64/ioctlent.h.in	\
	linux/sh64/syscallent.h		\
	linux/signalent.h		\
	linux/sparc/dummy2.h		\
	linux/sparc/errnoent.h		\
	linux/sparc/errnoent1.h		\
	linux/sparc/gen.pl		\
	linux/sparc/ioctlent.h.in	\
	linux/sparc/ioctlent1.h		\
	linux/sparc/signalent.h		\
	linux/sparc/signalent1.h	\
	linux/sparc/syscall1.h		\
	linux/sparc/syscallent.h	\
	linux/sparc/syscallent1.h	\
	linux/sparc64/dummy2.h		\
	linux/sparc64/errnoent.h	\
	linux/sparc64/errnoent1.h	\
	linux/sparc64/errnoent2.h	\
	linux/sparc64/ioctlent.h.in	\
	linux/sparc64/ioctlent1.h	\
	linux/sparc64/ioctlent2.h	\
	linux/sparc64/signalent.h	\
	linux/sparc64/signalent1.h	\
	linux/sparc64/signalent2.h	\
	linux/sparc64/syscall1.h	\
	linux/sparc64/syscallent.h	\
	linux/sparc64/syscallent1.h	\
	linux/sparc64/syscallent2.h	\
	linux/subcall.h			\
	linux/syscall.h			\
	linux/tile/errnoent1.h		\
	linux/tile/ioctlent.h.in	\
	linux/tile/ioctlent1.h		\
	linux/tile/signalent1.h		\
	linux/tile/syscallent.h		\
	linux/tile/syscallent1.h	\
	linux/ubi-user.h		\
	linux/x32/errnoent.h		\
	linux/x32/errnoent1.h		\
	linux/x32/ioctlent.h.in		\
	linux/x32/ioctlent1.h		\
	linux/x32/signalent.h		\
	linux/x32/signalent1.h		\
	linux/x32/syscallent.h		\
	linux/x32/syscallent1.h		\
	linux/x86_64/errnoent1.h	\
	linux/x86_64/errnoent2.h	\
	linux/x86_64/gentab.pl		\
	linux/x86_64/ioctlent.h.in	\
	linux/x86_64/ioctlent1.h	\
	linux/x86_64/ioctlent2.h	\
	linux/x86_64/signalent1.h	\
	linux/x86_64/signalent2.h	\
	linux/x86_64/syscallent.h	\
	linux/x86_64/syscallent1.h	\
	linux/x86_64/syscallent2.h	\
	linux/xtensa/ioctlent.h.in	\
	linux/xtensa/syscallent.h	\
	signalent.sh			\
	strace-graph			\
	strace-log-merge		\
	strace.spec			\
	syscallent.sh			\
	xlate.el

.PHONY: srpm
srpm: dist-xz
	rpmbuild --define '%_srcrpmdir .' -ts $(distdir).tar.xz

BUILT_SOURCES = .version

$(srcdir)/.version:
	$(AM_V_GEN)echo $(VERSION) > $@-t && mv $@-t $@

dist-hook:
	$(AM_V_GEN)echo $(VERSION) > $(distdir)/.tarball-version

today = $(shell date +%Y-%m-%d)
version_regexp = $(subst .,\.,$(VERSION))
news_check_regexp = 'Noteworthy changes in release $(version_regexp) ($(today))'

news-check: NEWS
	$(AM_V_GEN)if head -1 $< |				\
	  grep -x $(news_check_regexp) >/dev/null; then		\
		:;						\
	else							\
		echo >&2 '$<: check failed';			\
		exit 1;						\
	fi

if MAINTAINER_MODE

gen_changelog_start_date = 2009-07-08 20:00
$(srcdir)/ChangeLog: $(srcdir)/gitlog-to-changelog $(srcdir)/Makefile.in \
		     $(srcdir)/.version
	@rm -f $@.new
	(cd $(srcdir); \
	 ./gitlog-to-changelog --append-dot \
	   --since='$(gen_changelog_start_date)'; \
	 echo; echo; echo 'See ChangeLog-CVS for older changes.' \
	) > $@.new
	chmod 444 $@.new
	mv -f $@.new $@

$(srcdir)/CREDITS: $(srcdir)/CREDITS.in $(srcdir)/.mailmap \
		   $(srcdir)/Makefile.in $(srcdir)/.version
	$(AM_V_GEN)							\
	{								\
	  cd $(srcdir);							\
	  sed '/^##/,$$d' CREDITS.in;					\
	  { sed -n '1,/^##>/d; s/  \+/\t/; s/^./&/p' CREDITS.in;	\
	    git log --pretty=format:'%aN	%aE';			\
	  } | LC_ALL=C sort -u						\
	    | awk -F'\t' '{printf("\t%s <%s>\n",$$1,$$2)}';		\
	} > $@-t && mv $@-t $@

export TAR_OPTIONS = --owner=0 --group=0 --numeric-owner --mode=go-w,go+rX

IOCTLDIR = /usr/include
IOCTLASM = asm
IOCTLSORT_INCLUDEDIR = $(IOCTLDIR)

ioctlent_h_in = linux/ioctlent.h.in

BUILT_SOURCES += $(ioctlent_h_in)

$(srcdir)/$(ioctlent_h_in): ioctlsort
	$(<D)/$(<F) > $@
ioctlsort: $(srcdir)/linux/ioctlsort.c ioctls.h ioctldefs.h
	$(filter-out -I%,$(LINK.c)) -I. -I$(IOCTLSORT_INCLUDEDIR) \
	$(filter -I%,$(LINK.c)) \
	  -o $@ $<
ioctls.h: $(srcdir)/linux/ioctlent.sh
	$(SHELL) $< $(IOCTLDIR) $(IOCTLASM)
ioctldefs.h: ioctls.h ;

endif

ioctlent_h = $(builddir)/$(OS)/ioctlent.h
BUILT_SOURCES += $(ioctlent_h)
CLEANFILES = $(ioctlent_h)
ioctlent_h_deps = $(srcdir)/$(OS)/ioctlent.h.in $(srcdir)/$(OS)/$(ARCH)/ioctlent.h.in
$(ioctlent_h): $(top_builddir)/config.status $(ioctlent_h_deps)
	$(MKDIR_P) $(builddir)/$(OS)
	cat $(ioctlent_h_deps) | \
		$(COMPILE) -E -P - | \
		LC_ALL=C sort -u -k3,3 -k2,2 | \
		awk -f $(srcdir)/linux/ioctlent-filter.awk > $@
