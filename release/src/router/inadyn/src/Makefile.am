AUTOMAKE_OPTIONS = subdir-objects
AM_CPPFLAGS      = -I$(top_srcdir)/include -DSYSCONFDIR=\"@sysconfdir@\"
AM_CPPFLAGS     += -DLOCALSTATEDIR=\"@localstatedir@\" -DRUNSTATEDIR=\"@runstatedir@\"
AM_CPPFLAGS     += -D_GNU_SOURCE -D_BSD_SOURCE -D_DEFAULT_SOURCE
AM_CFLAGS        = -W -Wall -Wextra -Wno-unused-parameter -std=gnu99

sbin_PROGRAMS	 = inadyn
inadyn_SOURCES	 = main.c	ddns.c		cache.c		\
		   error.c	conf.c		os.c		\
		   http.c	plugin.c	tcp.c		\
		   json.c	jsmn.c		log.c		\
		   makepath.c
inadyn_CFLAGS    = $(confuse_CFLAGS) $(OpenSSL_CFLAGS) $(MbedTLS_CFLAGS) $(GnuTLS_CFLAGS)
inadyn_LDADD     = $(confuse_LIBS)   $(OpenSSL_LIBS)   $(MbedTLS_LIBS)   $(GnuTLS_LIBS)
inadyn_LDADD    += $(LIBS) $(LIBOBJS)

if ENABLE_SSL
if ENABLE_OPENSSL
inadyn_SOURCES  += openssl.c openssl_base64.c openssl_hash.c
else
if ENABLE_MBEDTLS
inadyn_SOURCES  += mbedtls.c mbedtls_base64.c mbedtls_hash.c
else
inadyn_SOURCES  += gnutls.c gnutls_base64.c gnutls_hash.c
endif
endif
else
inadyn_SOURCES  += base64.c md5.c sha1.c
endif

## Plugins are currently built-in, and built from this directory instead
## of where they reside.  They should be built by plugins/Makefile.am
## and be installed into $libdir/inadyn/plugins/ as *.so files
inadyn_SOURCES  += ../plugins/common.c		../plugins/changeip.c		\
		   ../plugins/cloudflare.c	../plugins/cloudxns.c		\
		   ../plugins/ddnss.c		../plugins/dhis.c		\
		   ../plugins/dnsexit.c		../plugins/dnspod.c		\
		   ../plugins/duckdns.c		../plugins/duiadns.c		\
		   ../plugins/dyndns.c		../plugins/dynv6.c		\
		   ../plugins/easydns.c		../plugins/freedns.c		\
		   ../plugins/freemyip.c	../plugins/generic.c		\
		   ../plugins/giradns.c						\
		   ../plugins/sitelutions.c	../plugins/tunnelbroker.c	\
		   ../plugins/yandex.c		../plugins/zoneedit.c		\
		   ../plugins/goip.c		../plugins/domaindiscount24.c	\
		   ../plugins/desec.c		../plugins/all-inkl.c		\
		   ../plugins/core-networks.c	../plugins/dnsever.c		\
		   ../plugins/dnshome.c		../plugins/dnsmadeeasy.c	\
		   ../plugins/dnsmax.c		../plugins/mydns.c		\
		   ../plugins/myonlineportal.c	../plugins/namecheap.c		\
		   ../plugins/regfish.c		../plugins/twodns.c		\
		   ../plugins/ipv64.c
