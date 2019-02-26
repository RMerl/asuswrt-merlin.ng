managerdir = ${ipsecdir}/manager

manager_PROGRAMS = manager.fcgi

manager_fcgi_SOURCES = \
main.c manager.c manager.h gateway.h gateway.c storage.h storage.c xml.h xml.c \
controller/auth_controller.c controller/auth_controller.h \
controller/ikesa_controller.c controller/ikesa_controller.h \
controller/control_controller.c controller/control_controller.h \
controller/config_controller.c controller/config_controller.h \
controller/gateway_controller.c controller/gateway_controller.h

manager_fcgi_LDADD = $(top_builddir)/src/libstrongswan/libstrongswan.la $(top_builddir)/src/libfast/libfast.la ${xml_LIBS}
main.o :	$(top_builddir)/config.status

AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan \
	-I$(top_srcdir)/src/libfast \
	-DIPSECDIR=\"${ipsecdir}\" \
	-DIPSEC_PIDDIR=\"${piddir}\" \
	-DPLUGINS=\""${manager_plugins}\""

AM_CFLAGS = \
	${xml_CFLAGS} \
	$(PLUGIN_CFLAGS)

# Don't forget to add templates to EXTRA_DIST !!! How to automate?
manager_templatesdir = ${managerdir}/templates
manager_templates_DATA = templates/header.cs templates/footer.cs templates/error.cs

manager_templates_authdir = ${manager_templatesdir}/auth
manager_templates_auth_DATA = templates/auth/login.cs

manager_templates_gatewaydir = ${manager_templatesdir}/gateway
manager_templates_gateway_DATA = templates/gateway/list.cs

manager_templates_ikesadir = ${manager_templatesdir}/ikesa
manager_templates_ikesa_DATA = templates/ikesa/list.cs

manager_templates_controldir = ${manager_templatesdir}/control
manager_templates_control_DATA = templates/control/result.cs

manager_templates_configdir = ${manager_templatesdir}/config
manager_templates_config_DATA = templates/config/list.cs

manager_templates_staticdir = ${manager_templatesdir}/static
manager_templates_static_DATA = templates/static/style.css templates/static/script.js templates/static/jquery.js \
templates/static/pipe.png templates/static/pipe-good.png templates/static/pipe-bad.png \
templates/static/pipe-thin.png templates/static/pipe-thin-left.png templates/static/pipe-thin-right.png \
templates/static/pipe-thin-green.png templates/static/pipe-thin-left-green.png templates/static/pipe-thin-right-green.png \
templates/static/gateway-left.png templates/static/client-left.png templates/static/strongswan.png \
templates/static/router.png templates/static/gateway-right.png templates/static/client-right.png \
templates/static/close.png templates/static/initiate.png

EXTRA_DIST = templates/header.cs templates/footer.cs templates/error.cs \
templates/auth/login.cs templates/gateway/list.cs templates/ikesa/list.cs \
templates/config/list.cs templates/control/result.cs \
templates/static/style.css templates/static/script.js templates/static/jquery.js \
templates/static/pipe.png templates/static/pipe-good.png templates/static/pipe-bad.png \
templates/static/pipe-thin.png templates/static/pipe-thin-left.png templates/static/pipe-thin-right.png \
templates/static/pipe-thin-green.png templates/static/pipe-thin-left-green.png templates/static/pipe-thin-right-green.png \
templates/static/gateway-left.png templates/static/client-left.png templates/static/strongswan.png \
templates/static/router.png templates/static/gateway-right.png templates/static/client-right.png \
templates/static/close.png templates/static/initiate.png
