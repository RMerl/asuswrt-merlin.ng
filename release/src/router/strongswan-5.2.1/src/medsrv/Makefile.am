medsrvdir = ${ipsecdir}/medsrv

medsrv_PROGRAMS = medsrv.fcgi

medsrv_fcgi_SOURCES = user.h user.c \
main.c filter/auth_filter.c filter/auth_filter.h \
controller/user_controller.c controller/user_controller.h \
controller/peer_controller.c controller/peer_controller.h

medsrv_fcgi_LDADD = $(top_builddir)/src/libstrongswan/libstrongswan.la $(top_builddir)/src/libfast/libfast.la
main.o :	$(top_builddir)/config.status

AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan \
	-I$(top_srcdir)/src/libfast \
	-DIPSECDIR=\"${ipsecdir}\" \
	-DIPSEC_PIDDIR=\"${piddir}\" \
	-DPLUGINS=\""${medsrv_plugins}\""

AM_CFLAGS = \
	$(PLUGIN_CFLAGS)

# Don't forget to add templates to EXTRA_DIST !!! How to automate?
medsrv_templatesdir = ${medsrvdir}/templates
medsrv_templates_DATA = templates/header.cs templates/footer.cs

medsrv_templates_userdir = ${medsrv_templatesdir}/user
medsrv_templates_user_DATA = templates/user/add.cs templates/user/edit.cs \
templates/user/login.cs templates/user/help.cs

medsrv_templates_peerdir = ${medsrv_templatesdir}/peer
medsrv_templates_peer_DATA = templates/peer/add.cs templates/peer/edit.cs \
templates/peer/list.cs

medsrv_templates_staticdir = ${medsrv_templatesdir}/static
medsrv_templates_static_DATA = templates/header.cs templates/footer.cs \
templates/static/style.css templates/static/strongswan.png \
templates/static/favicon.ico templates/static/mootools.js templates/static/script.js

EXTRA_DIST = templates/header.cs templates/footer.cs \
templates/static/style.css templates/static/strongswan.png \
templates/static/favicon.ico templates/static/mootools.js templates/static/script.js \
templates/peer/add.cs templates/peer/edit.cs templates/peer/list.cs \
templates/user/login.cs templates/user/add.cs templates/user/edit.cs \
templates/user/help.cs
