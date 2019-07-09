AUTOMAKE_OPTIONS = foreign

SUBDIRS = properties auth-dialog po

nmvpnservice_DATA = nm-strongswan-service.name

@INTLTOOL_DESKTOP_RULE@

appdatadir = $(datadir)/appdata
appdata_DATA = $(appdata_in_files:.xml.in=.xml)
appdata_in_files = NetworkManager-strongswan.appdata.xml.in
@INTLTOOL_XML_RULE@

if WITH_LIBNM_GLIB
# Install a file with full path to plugins for an old gnome-shell
# https://bugzilla.gnome.org/show_bug.cgi?id=693590
install-data-hook:
	mkdir -p $(DESTDIR)$(sysconfdir)/NetworkManager/VPN
	sed -e "1s|^|# This file is obsoleted by a file in $(nmvpnservicedir)\n\n|" \
	    -e 's|[@]NM_LIBEXECDIR[@]|$(nm_libexecdir)|g' \
	    -e 's|[@]NM_PLUGINDIR[@]|$(nm_plugindir)|g' \
	    -e 's|[@]NM_PLUGINDIR_ABS[@]|$(nm_plugindir)|g' \
	    -e 's|[@]CHARON[@]|$(charon)|' \
	    <$(srcdir)/nm-strongswan-service.name.in \
	    >$(DESTDIR)$(sysconfdir)/NetworkManager/VPN/nm-strongswan-service.name

uninstall-hook:
	 rm -f $(DESTDIR)$(sysconfdir)/NetworkManager/VPN/nm-strongswan-service.name
endif

nm-strongswan-service.name: $(srcdir)/nm-strongswan-service.name.in
	$(AM_V_GEN) \
	sed -e 's|[@]NM_LIBEXECDIR[@]|$(nm_libexecdir)|' \
	    -e 's|[@]NM_PLUGINDIR[@]|$(nm_plugindir)|g' \
	    -e 's|[@]NM_PLUGINDIR_ABS[@]/|$(nm_plugindir_abs)|g' \
	    -e 's|[@]CHARON[@]|$(charon)|' $< >$@

EXTRA_DIST = \
    nm-strongswan-service.name.in  \
    $(appdata_in_files)  \
    intltool-extract.in  \
    intltool-merge.in    \
    intltool-update.in

CLEANFILES = $(nmvpnservice_DATA) $(appdata_DATA) *~
DISTCLEANFILES = intltool-extract intltool-merge intltool-update

ACLOCAL_AMFLAGS = -I m4
