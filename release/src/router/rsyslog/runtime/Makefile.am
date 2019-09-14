sbin_PROGRAMS =
man_MANS = 
noinst_LTLIBRARIES = librsyslog.la
pkglib_LTLIBRARIES =
#pkglib_LTLIBRARIES = librsyslog.la

librsyslog_la_SOURCES = \
	rsyslog.c \
	rsyslog.h \
	typedefs.h \
	dnscache.c \
	dnscache.h \
	unicode-helper.h \
	atomic.h \
	batch.h \
	syslogd-types.h \
	module-template.h \
	im-helper.h \
	obj-types.h \
	sigprov.h \
	cryprov.h \
	nsd.h \
	glbl.h \
	glbl.c \
	unlimited_select.h \
	conf.c \
	conf.h \
	janitor.c \
	janitor.h \
	rsconf.c \
	rsconf.h \
	parser.h \
	parser.c \
	strgen.h \
	strgen.c \
	msg.c \
	msg.h \
	linkedlist.c \
	linkedlist.h \
	objomsr.c \
	objomsr.h \
	stringbuf.c \
	stringbuf.h \
	datetime.c \
	datetime.h \
	srutils.c \
	srUtils.h \
	errmsg.c \
	errmsg.h \
	debug.c \
	debug.h \
	obj.c \
	obj.h \
	modules.c \
	modules.h \
	statsobj.c \
	statsobj.h \
	dynstats.c \
	dynstats.h \
	statsobj.h \
	stream.c \
	stream.h \
	var.c \
	var.h \
	wtp.c \
	wtp.h \
	wti.c \
	wti.h \
	queue.c \
	queue.h \
	ruleset.c \
	ruleset.h \
	prop.c \
	prop.h \
	ratelimit.c \
	ratelimit.h \
	lookup.c \
	lookup.h \
	cfsysline.c \
	cfsysline.h \
	\
	../action.h \
	../action.c \
	../threads.c \
	../threads.h \
	\
	../parse.c \
	../parse.h \
	\
	hashtable.c \
	hashtable.h \
	hashtable_itr.c \
	hashtable_itr.h \
	hashtable_private.h \
	\
	../outchannel.c \
	../outchannel.h \
	../template.c \
	../template.h#
# the files with ../ we need to work on - so that they either become part of the
# runtime or will no longer be needed. -- rgerhards, 2008-06-13
#
#if OS_LINUX
#librsyslog_la_SOURCES += \
#endif

if WITH_MODDIRS
librsyslog_la_CPPFLAGS = -DSD_EXPORT_SYMBOLS -D_PATH_MODDIR=\"$(pkglibdir)/:$(moddirs)\"
else
librsyslog_la_CPPFLAGS = -DSD_EXPORT_SYMBOLS -D_PATH_MODDIR=\"$(pkglibdir)/\" -I\$(top_srcdir) -I\$(top_srcdir)/grammar
endif
#librsyslog_la_LDFLAGS = -module -avoid-version
librsyslog_la_CPPFLAGS += $(PTHREADS_CFLAGS) $(RSRT_CFLAGS) $(LIBUUID_CFLAGS) $(LIBFASTJSON_CFLAGS) ${LIBESTR_CFLAGS} ${LIBLOGGING_STDLOG_CFLAGS} -I\$(top_srcdir)/tools
librsyslog_la_LIBADD =  $(DL_LIBS) $(RT_LIBS) $(LIBUUID_LIBS) $(LIBFASTJSON_LIBS) ${LIBESTR_LIBS} ${LIBLOGGING_STDLOG_LIBS}

#
# regular expression support
# 
if ENABLE_REGEXP
pkglib_LTLIBRARIES += lmregexp.la
lmregexp_la_SOURCES = regexp.c regexp.h
lmregexp_la_CPPFLAGS = $(PTHREADS_CFLAGS) $(RSRT_CFLAGS) $(LIBLOGGING_STDLOG_CFLAGS)
lmregexp_la_LDFLAGS = -module -avoid-version $(LIBLOGGING_STDLOG_LIBS)
lmregexp_la_LIBADD =
endif

#
# zlib support
# 
pkglib_LTLIBRARIES += lmzlibw.la
lmzlibw_la_SOURCES = zlibw.c zlibw.h
lmzlibw_la_CPPFLAGS = $(PTHREADS_CFLAGS) $(RSRT_CFLAGS) $(LIBLOGGING_STDLOG_CFLAGS)
lmzlibw_la_LDFLAGS = -module -avoid-version $(LIBLOGGING_STDLOG_LIBS)
lmzlibw_la_LIBADD =


if ENABLE_INET
pkglib_LTLIBRARIES += lmnet.la lmnetstrms.la
#
# network support
# 
lmnet_la_SOURCES = net.c net.h
lmnet_la_CPPFLAGS = $(PTHREADS_CFLAGS) $(RSRT_CFLAGS) $(LIBLOGGING_STDLOG_CFLAGS)
lmnet_la_LDFLAGS = -module -avoid-version ../compat/compat_la-getifaddrs.lo $(LIBLOGGING_STDLOG_LIBS)
lmnet_la_LIBADD =

# network stream master class and stream factory
lmnetstrms_la_SOURCES = netstrms.c netstrms.h \
			netstrm.c netstrm.h \
			nssel.c nssel.h \
			nspoll.c nspoll.h
lmnetstrms_la_CPPFLAGS = $(PTHREADS_CFLAGS) $(RSRT_CFLAGS) $(LIBLOGGING_STDLOG_CFLAGS)
lmnetstrms_la_LDFLAGS = -module -avoid-version $(LIBLOGGING_STDLOG_LIBS)
lmnetstrms_la_LIBADD =

# generic stream server framework
pkglib_LTLIBRARIES += lmstrmsrv.la
lmstrmsrv_la_SOURCES = strmsrv.c strmsrv.h strms_sess.c strms_sess.h
lmstrmsrv_la_CPPFLAGS = $(PTHREADS_CFLAGS) $(RSRT_CFLAGS) $(LIBLOGGING_STDLOG_CFLAGS)
lmstrmsrv_la_LDFLAGS = -module -avoid-version $(LIBLOGGING_STDLOG_LIBS)
lmstrmsrv_la_LIBADD =

# netstream drivers

# plain tcp driver - main driver
pkglib_LTLIBRARIES += lmnsd_ptcp.la
lmnsd_ptcp_la_SOURCES = nsd_ptcp.c nsd_ptcp.h \
		  	nsdsel_ptcp.c nsdsel_ptcp.h \
			nsdpoll_ptcp.c nsdpoll_ptcp.h
lmnsd_ptcp_la_CPPFLAGS = $(PTHREADS_CFLAGS) $(RSRT_CFLAGS) $(LIBLOGGING_STDLOG_CFLAGS)
lmnsd_ptcp_la_LDFLAGS = -module -avoid-version $(LIBLOGGING_STDLOG_LIBS)
lmnsd_ptcp_la_LIBADD =
endif # if ENABLE_INET

#
# openssl netstream driver
# 
if ENABLE_OPENSSL
pkglib_LTLIBRARIES += lmnsd_ossl.la
lmnsd_ossl_la_SOURCES = nsd_ossl.c nsd_ossl.h nsdsel_ossl.c  nsdsel_ossl.h
lmnsd_ossl_la_CPPFLAGS = $(PTHREADS_CFLAGS) $(RSRT_CFLAGS) $(OPENSSL_CFLAGS)
lmnsd_ossl_la_LDFLAGS = -module -avoid-version
lmnsd_ossl_la_LIBADD = $(OPENSSL_LIBS)
endif

#
# GnuTLS netstream driver
# 
if ENABLE_GNUTLS
pkglib_LTLIBRARIES += lmnsd_gtls.la
lmnsd_gtls_la_SOURCES = nsd_gtls.c nsd_gtls.h nsdsel_gtls.c  nsdsel_gtls.h
lmnsd_gtls_la_CPPFLAGS = $(PTHREADS_CFLAGS) $(RSRT_CFLAGS) $(GNUTLS_CFLAGS)
lmnsd_gtls_la_LDFLAGS = -module -avoid-version
lmnsd_gtls_la_LIBADD = $(GNUTLS_LIBS)
endif

#
# support library for libgcrypt
#
if ENABLE_LIBGCRYPT
   noinst_LTLIBRARIES += libgcry.la
   libgcry_la_SOURCES = libgcry.c libgcry_common.c libgcry.h
   libgcry_la_CPPFLAGS = $(RSRT_CFLAGS) $(LIBGCRYPT_CFLAGS)
   pkglib_LTLIBRARIES += lmcry_gcry.la
   lmcry_gcry_la_DEPENDENCIES = librsyslog.la libgcry.la
   lmcry_gcry_la_SOURCES = lmcry_gcry.c lmcry_gcry.h
   lmcry_gcry_la_CPPFLAGS = $(RSRT_CFLAGS) $(LIBGCRYPT_CFLAGS)
   lmcry_gcry_la_LDFLAGS = -module -avoid-version \
	-Wl,--whole-archive,$(top_builddir)/runtime/.libs/librsyslog.a,--no-whole-archive
   lmcry_gcry_la_LIBADD = libgcry.la $(LIBGCRYPT_LIBS)
endif


#
# gssapi support
# 
if ENABLE_GSSAPI
pkglib_LTLIBRARIES += lmgssutil.la
lmgssutil_la_SOURCES = gss-misc.c gss-misc.h
lmgssutil_la_CPPFLAGS = $(PTHREADS_CFLAGS) $(RSRT_CFLAGS)
lmgssutil_la_LDFLAGS = -module -avoid-version
lmgssutil_la_LIBADD = $(GSS_LIBS)
endif

pkglib_LTLIBRARIES += lmtcpsrv.la lmtcpclt.la
#
#
# TCP (stream) server support
#
lmtcpsrv_la_SOURCES = \
	tcps_sess.c \
	tcps_sess.h \
	tcpsrv.c \
	tcpsrv.h
lmtcpsrv_la_CPPFLAGS = $(PTHREADS_CFLAGS) $(RSRT_CFLAGS) $(LIBLOGGING_STDLOG_CFLAGS)
lmtcpsrv_la_LDFLAGS = -module -avoid-version $(LIBLOGGING_STDLOG_LIBS)
lmtcpsrv_la_LIBADD = 

#
# TCP (stream) client support
#
lmtcpclt_la_SOURCES = \
	tcpclt.c \
	tcpclt.h
lmtcpclt_la_CPPFLAGS = $(PTHREADS_CFLAGS) $(RSRT_CFLAGS) $(LIBLOGGING_STDLOG_CFLAGS)
lmtcpclt_la_LDFLAGS = -module -avoid-version $(LIBLOGGING_STDLOG_LIBS)
lmtcpclt_la_LIBADD = 


#
# support library for Guardtime KSI-LS12
#
if ENABLE_KSI_LS12
   pkglib_LTLIBRARIES += lmsig_ksi_ls12.la
   lmsig_ksi_ls12_la_SOURCES = lmsig_ksi-ls12.c lmsig_ksi-ls12.h lib_ksils12.c \
			lib_ksils12.h lib_ksi_queue.c lib_ksi_queue.h
   lmsig_ksi_ls12_la_CPPFLAGS = $(RSRT_CFLAGS) $(GT_KSI_LS12_CFLAGS)
   lmsig_ksi_ls12_la_LDFLAGS = -module -avoid-version $(GT_KSI_LS12_LIBS)
endif

update-systemd:
	curl http://cgit.freedesktop.org/systemd/systemd/plain/src/libsystemd-daemon/sd-daemon.c > sd-daemon.c
	curl http://cgit.freedesktop.org/systemd/systemd/plain/src/systemd/sd-daemon.h > sd-daemon.h
