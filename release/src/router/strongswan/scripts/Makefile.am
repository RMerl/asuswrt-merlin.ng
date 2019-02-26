AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan \
	-I$(top_srcdir)/src/libtls \
	-DPLUGINS="\"${scripts_plugins}\""

noinst_PROGRAMS = bin2array bin2sql id2sql key2keyid keyid2sql oid2der \
	thread_analysis dh_speed pubkey_speed crypt_burn hash_burn fetch \
	dnssec malloc_speed aes-test settings-test timeattack

if USE_TLS
  noinst_PROGRAMS += tls_test
  tls_test_SOURCES = tls_test.c
  tls_test_LDADD = $(top_builddir)/src/libstrongswan/libstrongswan.la \
					$(top_builddir)/src/libtls/libtls.la
endif

bin2array_SOURCES = bin2array.c
bin2sql_SOURCES = bin2sql.c
id2sql_SOURCES = id2sql.c
key2keyid_SOURCES = key2keyid.c
keyid2sql_SOURCES = keyid2sql.c
oid2der_SOURCES = oid2der.c
thread_analysis_SOURCES = thread_analysis.c
dh_speed_SOURCES = dh_speed.c
pubkey_speed_SOURCES = pubkey_speed.c
crypt_burn_SOURCES = crypt_burn.c
hash_burn_SOURCES = hash_burn.c
malloc_speed_SOURCES = malloc_speed.c
fetch_SOURCES = fetch.c
dnssec_SOURCES = dnssec.c
timeattack_SOURCES = timeattack.c
id2sql_LDADD = $(top_builddir)/src/libstrongswan/libstrongswan.la
key2keyid_LDADD = $(top_builddir)/src/libstrongswan/libstrongswan.la
keyid2sql_LDADD = $(top_builddir)/src/libstrongswan/libstrongswan.la
oid2der_LDADD = $(top_builddir)/src/libstrongswan/libstrongswan.la
dh_speed_LDADD = $(top_builddir)/src/libstrongswan/libstrongswan.la $(RTLIB)
pubkey_speed_LDADD = $(top_builddir)/src/libstrongswan/libstrongswan.la $(RTLIB)
crypt_burn_LDADD = $(top_builddir)/src/libstrongswan/libstrongswan.la
hash_burn_LDADD = $(top_builddir)/src/libstrongswan/libstrongswan.la
malloc_speed_LDADD = $(top_builddir)/src/libstrongswan/libstrongswan.la $(RTLIB)
fetch_LDADD = $(top_builddir)/src/libstrongswan/libstrongswan.la
dnssec_LDADD = $(top_builddir)/src/libstrongswan/libstrongswan.la
aes_test_LDADD = $(top_builddir)/src/libstrongswan/libstrongswan.la
settings_test_LDADD = $(top_builddir)/src/libstrongswan/libstrongswan.la
timeattack_LDADD = $(top_builddir)/src/libstrongswan/libstrongswan.la $(RTLIB)

key2keyid.o :	$(top_builddir)/config.status

keyid2sql.o :	$(top_builddir)/config.status
