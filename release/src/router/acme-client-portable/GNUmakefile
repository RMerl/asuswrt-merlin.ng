# If you end up changing anything significant in this makefile that's
# not overly system-specific, please submit a bug report.

PREFIX	?= /usr
MAN1DIR	?= $(PREFIX)/man/man1
BINDIR	?= $(PREFIX)/bin

CFLAGS += $(if $(STAGING_DIR),--sysroot=$(STAGING_DIR))
LDFLAGS += $(if $(STAGING_DIR),--sysroot=$(STAGING_DIR))

CFLAGS	+= -g -W -Wall -DHAVE_CONFIG_H
OBJS 	 = acctproc.o \
	   base64.o \
	   certproc.o \
	   chngproc.o \
	   dbg.o \
	   dnsproc.o \
	   fileproc.o \
	   http.o \
	   jsmn.o \
	   json.o \
	   keyproc.o \
	   main.o \
	   netproc.o \
	   revokeproc.o \
	   rsa.o \
	   util.o

# All of the below should happen in a nice shell-script that does the
# configuration for us and generate config.h.
# However, for now this works fine.

USE_OPENSSL=y
ifeq ($(USE_OPENSSL), y)

CFLAGS	+= -DOPSSL
CFLAGS	+= -I$(TOP)/openssl/include
LDFLAGS += -L$(TOP)/openssl

OBJS	+= util-portable.o
OBJS	+= sandbox-null.o
OBJS	+= strlcat.o
OBJS	+= strlcpy.o

else ifeq ($(USE_OPENSSL), n)

CFLAGS	+= -I$(TOP)/libbsd/include
LDFLAGS += -L$(TOP)/libbsd/src/.libs -lbsd

CFLAGS	+= -I$(TOP)/libressl/include
LDFLAGS += -L$(TOP)/libressl/crypto/.libs
LDFLAGS += -L$(TOP)/libressl/ssl/.libs
LDFLAGS += -L$(TOP)/libressl/tls/.libs -ltls

LDFLAGS += -lpthread
OBJS	+= util-portable.o
OBJS	+= sandbox-null.o

else ifeq ($(shell uname), Linux)

# Compiling on Linux.
# Linux is the biggest "moving target" because there are lots of
# variants and nits.

# Start by checking whether we're on a musl libc system, which provides
# the functions other systems require from libbsd.

ifeq ($(shell ldd --version 2>&1 | grep 'musl libc'),)
LIBADD	+= $(shell pkg-config --libs libbsd)
CFLAGS	+= $(shell pkg-config --cflags libbsd)
else
CFLAGS	+= -DMUSL_LIBC
endif

CFLAGS	+= -I/usr/local/include/libressl
LDFLAGS += -L/usr/local/lib
OBJS	+= util-portable.o

# Do we have libseccomp installed?
# If so, pull in the seccomp sandbox package.
# If the sandbox doesn't work for you, just comment out all lines but
# "OBJS += sandbox-null.o" and recompile.
# PLEASE TELL ME IF IT DOESN'T WORK.

# For the time being, I am disabling seccomp.
# It does not work consistently between machines and is difficult to
# debug.
# See https://github.com/kristapsdz/acme-client-portable if you are
# willing to enable this (at your own discretion) and synchronise the
# seccomp code with me.

#ifeq ($(shell pkg-config --exists libseccomp && echo 1),1)
#OBJS	+= sandbox-seccomp.o
#LIBADD	+= $(shell pkg-config --libs libseccomp)
#CFLAGS	+= $(shell pkg-config --cflags libseccomp)
#else
OBJS	+= sandbox-null.o
#endif

else ifeq ($(shell uname), Darwin)

# Compiling on Mac OS X.
# If we show deprecations, everything in libressl shows up.

CFLAGS	+= -I/usr/local/opt/libressl/include -Wno-deprecated-declarations 
LDFLAGS	+= -L/usr/local/opt/libressl/lib
OBJS	+= util-portable.o \
	   sandbox-darwin.o \
	   compat-setresuid.o \
	   compat-setresgid.o

else ifeq ($(shell uname), OpenBSD)

# Compiling on OpenBSD.
# Obviously the following is a temporary solution... I'll remove it when
# some of my systems no longer run 5.8 without pledge(2).

ifeq ($(shell uname -r), 5.9)
OBJS	+= util-pledge.o \
	   sandbox-pledge.o
else ifeq ($(shell uname -r), 6.0)
OBJS	+= util-pledge.o \
	   sandbox-pledge.o
else
OBJS	+= util-portable.o \
	   sandbox-null.o
endif

else ifeq ($(shell uname), FreeBSD)

# Compiling on FreeBSD.
# LibreSSL is assumed to be in packages (/usr/local).

CFLAGS	+= -I/usr/local/include
LDFLAGS	+= -L/usr/local/lib
OBJS	+= util-portable.o \
	   sandbox-null.o

else ifeq ($(shell uname), NetBSD)

# Compiling on NetBSD.
# LibreSSL is assumed to be in packages (/usr/pkg).

CFLAGS	+= $(shell pkg-config --cflags libtls 2>/dev/null || echo '-I/usr/pkg/libressl/include')
LDFLAGS	+= $(shell pkg-config --libs libtls 2>/dev/null || echo '-L/usr/pkg/libressl/lib')
OBJS	+= util-portable.o \
	   sandbox-null.o \
	   compat-setresuid.o \
	   compat-setresgid.o
endif

# If you need to change anything below here, it's a bug.

all: acme-client

acme-client: $(OBJS)
	$(CC) -o $@ $(OBJS) $(LDFLAGS) -lssl -lcrypto $(LIBADD)

# This is for synchronising from -portable to the master.
rmerge:
	@for f in ../letskencrypt/*.[1ch]; do \
		ff=`basename $$f` ; \
		TMP1=`mktemp /tmp/merge.XXXXXX` || exit 1 ; \
		TMP2=`mktemp /tmp/merge.XXXXXX` || exit 1 ; \
		tail -n+2 $$f > $$TMP1 ; \
		tail -n+2 $$ff > $$TMP2 ; \
		cmp $$TMP1 $$TMP2 ; \
		rc=$$? ; \
		rm -f $$TMP1 $$TMP2 ; \
		[ 0 -eq $$rc ] && continue ; \
		diff -u $$f $$ff | less ; \
		/bin/echo -n "Replace [Y/n]: " ; \
		read in ; \
		if [ -z "$$in" -o "y" = "$$in" -o "Y" = "$$in" ]; then \
			cp -f $$ff $$f ; \
		fi \
	done

# This is for synchronising from the master to -portable.
merge:
	@for f in ../letskencrypt/*.[1ch]; do \
		ff=`basename $$f` ; \
		if [ ! -f $$ff ]; then \
			/bin/echo "Installed $$ff" ; \
			cp $$f $$ff ; \
			continue ; \
		fi ; \
		TMP1=`mktemp /tmp/merge.XXXXXX` || exit 1 ; \
		TMP2=`mktemp /tmp/merge.XXXXXX` || exit 1 ; \
		tail -n+2 $$f > $$TMP1 ; \
		tail -n+2 $$ff > $$TMP2 ; \
		cmp $$TMP1 $$TMP2 ; \
		rc=$$? ; \
		rm -f $$TMP1 $$TMP2 ; \
		[ 0 -eq $$rc ] && continue ; \
		diff -u $$ff $$f | less ; \
		/bin/echo -n "Replace [Y/n]: " ; \
		read in ; \
		if [ -z "$$in" -o "y" = "$$in" -o "Y" = "$$in" ]; then \
			cp -f $$f $$ff ; \
		fi \
	done

install: acme-client
	mkdir -p $(DESTDIR)$(MAN1DIR)
	mkdir -p $(DESTDIR)$(BINDIR)
	install -m 0755 acme-client $(DESTDIR)$(BINDIR)
	install -m 0644 acme-client.1 $(DESTDIR)$(MAN1DIR)

$(OBJS): extern.h config.h

rsa.o acctproc.o keyproc.o: rsa.h

http.o netproc.o: http.h

jsmn.o json.o: jsmn.h

clean:
	rm -f acme-client $(OBJS)
	rm -rf acme-client.dSYM
