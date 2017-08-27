PREFIX?=/usr
# brcm: modify the LIBDIR, SBINDIR
LIBDIR?=$(PREFIX)/lib
SBINDIR=$(INSTALL_DIR)/bin
CONFDIR=$(INSTALL_DIR)/etc/iproute2
DATADIR?=$(PREFIX)/share
DOCDIR?=$(DATADIR)/doc/iproute2
MANDIR?=$(DATADIR)/man
ARPDDIR?=/var/lib/arpd
# brcm: modify the KERNEL_INCLUDES and add LIBC_INCLUDE
KERNEL_INCLUDES:=$(KERNEL_DIR)/include $(KERNEL_DIR)/arch/$(ARCH)/include
LIBC_INCLUDE=$(KERNEL_DIR)/include/linux

# Path to db_185.h include
DBM_INCLUDE:=$(DESTDIR)/usr/include

SHARED_LIBS = y

DEFINES= -DRESOLVE_HOSTNAMES -DLIBDIR=\"$(LIBDIR)\"
ifneq ($(SHARED_LIBS),y)
DEFINES+= -DNO_SHARED_LIBS
endif

DEFINES+=-DCONFDIR=\"$(CONFDIR)\"

#options for decnet
# brcm: not support decnet
#ADDLIB+=dnet_ntop.o dnet_pton.o

#options for ipx
# brcm: not support ipx 
#ADDLIB+=ipx_ntop.o ipx_pton.o

#options for mpls
# brcm: not support mpls
#ADDLIB+=mpls_ntop.o mpls_pton.o

CC = gcc
HOSTCC = gcc
DEFINES += -D_GNU_SOURCE
# Turn on transparent support for LFS
DEFINES += -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE
CCOPTS = -O2
WFLAGS := -Wall -Wstrict-prototypes  -Wmissing-prototypes
WFLAGS += -Wmissing-declarations -Wold-style-definition -Wformat=2

CFLAGS := $(WFLAGS) $(CCOPTS) -I../include $(DEFINES)
YACCFLAGS = -d -t -v

# brcm: only build lib ip tc misc
SUBDIRS=lib ip tc bridge misc netem genl tipc man

# brcm: modify the libutil.a to libipr2util.a
LIBNETLINK=../lib/libnetlink.a ../lib/libipr2util.a
LDLIBS += $(LIBNETLINK)

all: Config
	@set -e; \
	for i in $(SUBDIRS); \
	do $(MAKE) $(MFLAGS) -C $$i; done

Config:
	sh configure $(KERNEL_INCLUDE)

install: all
	install -m 0755 -d $(DESTDIR)$(SBINDIR)
	install -m 0755 -d $(DESTDIR)$(CONFDIR)
	install -m 0755 -d $(DESTDIR)$(ARPDDIR)
	install -m 0755 -d $(DESTDIR)$(DOCDIR)/examples
	install -m 0755 -d $(DESTDIR)$(DOCDIR)/examples/diffserv
	install -m 0644 README.iproute2+tc $(shell find examples -maxdepth 1 -type f) \
		$(DESTDIR)$(DOCDIR)/examples
	install -m 0644 $(shell find examples/diffserv -maxdepth 1 -type f) \
		$(DESTDIR)$(DOCDIR)/examples/diffserv
	@for i in $(SUBDIRS) doc; do $(MAKE) -C $$i install; done
	install -m 0644 $(shell find etc/iproute2 -maxdepth 1 -type f) $(DESTDIR)$(CONFDIR)

snapshot:
	echo "static const char SNAPSHOT[] = \""`date +%y%m%d`"\";" \
		> include/SNAPSHOT.h

clean:
	@for i in $(SUBDIRS) doc; \
	do $(MAKE) $(MFLAGS) -C $$i clean; done

clobber:
	touch Config
	$(MAKE) $(MFLAGS) clean
	rm -f Config cscope.*

distclean: clobber

cscope:
	cscope -b -q -R -Iinclude -sip -slib -smisc -snetem -stc

.EXPORT_ALL_VARIABLES:
