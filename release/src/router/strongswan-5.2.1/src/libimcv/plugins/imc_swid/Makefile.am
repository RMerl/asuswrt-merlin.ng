regid = regid.2004-03.org.strongswan
unique_sw_id = strongSwan-$(PACKAGE_VERSION_MAJOR)-$(PACKAGE_VERSION_MINOR)-$(PACKAGE_VERSION_BUILD)$(PACKAGE_VERSION_REVIEW)
swid_tag = $(regid)_$(unique_sw_id).swidtag

swiddir = $(prefix)/share/$(regid)
swid_DATA = $(swid_tag)
ipsec_DATA = $(swid_tag)
EXTRA_DIST = $(regid)_strongSwan.swidtag.in
CLEANFILES = $(regid)_strongSwan*.swidtag

$(swid_tag) : regid.2004-03.org.strongswan_strongSwan.swidtag.in
	$(AM_V_GEN) \
	sed \
	-e "s:@VERSION_MAJOR@:$(PACKAGE_VERSION_MAJOR):" \
	-e "s:@VERSION_MINOR@:$(PACKAGE_VERSION_MINOR):" \
	-e "s:@VERSION_BUILD@:$(PACKAGE_VERSION_BUILD):" \
	-e "s:@VERSION_REVIEW@:$(PACKAGE_VERSION_REVIEW):" \
	$(srcdir)/$(regid)_strongSwan.swidtag.in > $@

AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan \
	-I$(top_srcdir)/src/libtncif \
	-I$(top_srcdir)/src/libimcv \
	-DSWID_DIRECTORY=\"${prefix}/share\"

AM_CFLAGS = \
	$(PLUGIN_CFLAGS)

imcv_LTLIBRARIES = imc-swid.la

imc_swid_la_LIBADD = \
	$(top_builddir)/src/libimcv/libimcv.la \
	$(top_builddir)/src/libstrongswan/libstrongswan.la

imc_swid_la_SOURCES = imc_swid.c imc_swid_state.h imc_swid_state.c

imc_swid_la_LDFLAGS = -module -avoid-version -no-undefined
