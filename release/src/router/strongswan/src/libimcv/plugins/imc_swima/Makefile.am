regid = strongswan.org
unique_sw_id = strongSwan-$(PACKAGE_VERSION_MAJOR)-$(PACKAGE_VERSION_MINOR)-$(PACKAGE_VERSION_BUILD)$(PACKAGE_VERSION_REVIEW)
swid_tag = $(regid)__$(unique_sw_id).swidtag

swiddir = $(pkgdatadir)/swidtag
dist_swid_DATA = $(swid_tag)
EXTRA_DIST = $(regid)__strongSwan.swidtag.in
CLEANFILES = $(regid)__strongSwan*.swidtag

$(swid_tag) : $(regid)__strongSwan.swidtag.in
	$(AM_V_GEN) \
	sed \
	-e "s:@VERSION_MAJOR@:$(PACKAGE_VERSION_MAJOR):" \
	-e "s:@VERSION_MINOR@:$(PACKAGE_VERSION_MINOR):" \
	-e "s:@VERSION_BUILD@:$(PACKAGE_VERSION_BUILD):" \
	-e "s:@VERSION_REVIEW@:$(PACKAGE_VERSION_REVIEW):" \
	$(srcdir)/$(regid)__strongSwan.swidtag.in > $@

AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan \
	-I$(top_srcdir)/src/libtncif \
	-I$(top_srcdir)/src/libimcv \
	-DSW_COLLECTOR=\"${prefix}/sbin/sw-collector\"

AM_CFLAGS = \
	$(PLUGIN_CFLAGS) $(json_CFLAGS)


imcv_LTLIBRARIES = imc-swima.la

imc_swima_la_LIBADD = \
	$(top_builddir)/src/libimcv/libimcv.la \
	$(top_builddir)/src/libstrongswan/libstrongswan.la
imc_swima_la_SOURCES = imc_swima.c imc_swima_state.h imc_swima_state.c
imc_swima_la_LDFLAGS = -module -avoid-version -no-undefined
