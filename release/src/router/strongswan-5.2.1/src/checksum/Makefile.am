# this lib is not built until make install is called (see rules at the bottom)
EXTRA_LTLIBRARIES = libchecksum.la
ipseclib_LTLIBRARIES = $(LIBCHECKSUM_LIBS)
nodist_libchecksum_la_SOURCES = checksum.c
libchecksum_la_LDFLAGS = -module -avoid-version -rpath '$(ipseclibdir)'

EXTRA_PROGRAMS = checksum_builder
checksum_builder_SOURCES = checksum_builder.c
checksum_builder_LDADD = \
	$(top_builddir)/src/libstrongswan/libstrongswan.la \
	$(top_builddir)/src/libhydra/libhydra.la \
	$(top_builddir)/src/libcharon/libcharon.la \
	$(DLLIB)
checksum_builder_LDFLAGS = -rpath '$(DESTDIR)$(ipseclibdir)'

CLEANFILES = checksum.c $(EXTRA_PROGRAMS)

AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan \
	-I$(top_srcdir)/src/libhydra \
	-I$(top_srcdir)/src/libcharon \
	-DPLUGINDIR=\"${DESTDIR}${plugindir}\"

AM_CFLAGS = \
	$(PLUGIN_CFLAGS)

# we keep track of build dependencies in deps and use libs to store the paths
# to the installed libraries. for executables we use the built files directly
# as these are not relinked during installation.
deps = $(top_builddir)/src/libstrongswan/libstrongswan.la
libs = $(DESTDIR)$(ipseclibdir)/libstrongswan.so
exes =

if !MONOLITHIC
  AM_CPPFLAGS += -DS_PLUGINS=\""${s_plugins}\""
endif

if USE_LIBHYDRA
  deps += $(top_builddir)/src/libhydra/libhydra.la
  libs += $(DESTDIR)$(ipseclibdir)/libhydra.so
if !MONOLITHIC
  AM_CPPFLAGS += -DH_PLUGINS=\""${h_plugins}\""
endif
endif

if USE_LIBIPSEC
  deps += $(top_builddir)/src/libipsec/libipsec.la
  libs += $(DESTDIR)$(ipseclibdir)/libipsec.so
endif

if USE_TLS
  deps += $(top_builddir)/src/libtls/libtls.la
  libs += $(DESTDIR)$(ipseclibdir)/libtls.so
endif

if USE_RADIUS
  deps += $(top_builddir)/src/libradius/libradius.la
  libs += $(DESTDIR)$(ipseclibdir)/libradius.so
endif

if USE_LIBPTTLS
  deps += $(top_builddir)/src/libpttls/libpttls.la
  libs += $(DESTDIR)$(ipseclibdir)/libpttls.so
endif

if USE_LIBTNCCS
  deps += $(top_builddir)/src/libtnccs/libtnccs.la
  libs += $(DESTDIR)$(ipseclibdir)/libtnccs.so
if !MONOLITHIC
  AM_CPPFLAGS += -DT_PLUGINS=\""${t_plugins}\""
endif
endif

if USE_SIMAKA
  deps += $(top_builddir)/src/libsimaka/libsimaka.la
  libs += $(DESTDIR)$(ipseclibdir)/libsimaka.so
endif

if USE_IMCV
  deps += $(top_builddir)/src/libimcv/libimcv.la
  libs += $(DESTDIR)$(ipseclibdir)/libimcv.so
endif

if USE_CHARON
  deps += $(top_builddir)/src/libcharon/libcharon.la
  libs += $(DESTDIR)$(ipseclibdir)/libcharon.so
  exes += $(DESTDIR)$(ipsecdir)/charon
if !MONOLITHIC
  AM_CPPFLAGS += -DC_PLUGINS=\""${c_plugins}\""
endif
endif

if USE_CMD
  exes += $(DESTDIR)$(sbindir)/charon-cmd
endif

if USE_SCEPCLIENT
  exes += $(DESTDIR)$(ipsecdir)/scepclient
endif

if USE_PKI
  exes += $(DESTDIR)$(bindir)/pki
endif

if USE_SWANCTL
  exes += $(DESTDIR)$(sbindir)/swanctl
endif

if USE_ATTR_SQL
  exes += $(DESTDIR)$(ipsecdir)/pool
endif

if USE_IMV_ATTESTATION
  exes += $(DESTDIR)$(ipsecdir)/attest
endif

checksum.c : checksum_builder $(deps) $(exes)
		./checksum_builder $(libs) $(exes) > checksum.c

install-data-hook : $(EXTRA_LTLIBRARIES)
	$(MAKE) $(AM_MAKEFLAGS) LIBCHECKSUM_LIBS='$(EXTRA_LTLIBRARIES)' install-ipseclibLTLIBRARIES

uninstall-hook :
	$(MAKE) $(AM_MAKEFLAGS) LIBCHECKSUM_LIBS='$(EXTRA_LTLIBRARIES)' uninstall-ipseclibLTLIBRARIES

clean-local :
	$(MAKE) $(AM_MAKEFLAGS) LIBCHECKSUM_LIBS='$(EXTRA_LTLIBRARIES)' clean-ipseclibLTLIBRARIES
