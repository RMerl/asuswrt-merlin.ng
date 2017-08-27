include $(top_srcdir)/glib-tap.mk

dist_uninstalled_test_data =
test_ltlibraries =

SUBDIRS = gdbus-object-manager-example services

LDADD = \
	$(top_builddir)/glib/libglib-2.0.la		\
	$(top_builddir)/gobject/libgobject-2.0.la	\
	$(top_builddir)/gmodule/libgmodule-2.0.la	\
	$(top_builddir)/gio/libgio-2.0.la
AM_CPPFLAGS = $(gio_INCLUDES) $(GLIB_DEBUG_FLAGS) -I$(top_builddir)/gio -I$(top_srcdir)/gio
DEFS = -DG_LOG_DOMAIN=\"GLib-GIO\" -DTEST_SERVICES=\""$(abs_top_builddir)/gio/tests/services"\"
AM_CFLAGS = $(GLIB_WARN_CFLAGS)

# -----------------------------------------------------------------------------
#  Test programs buildable on all platforms

test_programs = \
	async-close-output-stream		\
	buffered-input-stream			\
	buffered-output-stream			\
	cancellable				\
	contexts				\
	converter-stream			\
	credentials				\
	data-input-stream			\
	data-output-stream			\
	fileattributematcher			\
	filter-streams				\
	g-file					\
	g-file-info				\
	g-icon					\
	gdbus-addresses				\
	gdbus-message				\
	inet-address				\
	io-stream				\
	memory-input-stream			\
	memory-output-stream			\
	monitor					\
	network-address				\
	network-monitor				\
	permission				\
	pollable				\
	proxy-test				\
	readwrite				\
	simple-async-result			\
	simple-proxy				\
	sleepy-stream				\
	socket					\
	srvtarget				\
	task					\
	tls-interaction				\
	vfs					\
	volumemonitor				\
	$(NULL)

uninstalled_test_programs = \
	$(NULL)

dist_test_data = \
	contexts.c				\
	g-icon.c				\
	$(NULL)

test_data = \
	test.gresource				\
	$(NULL)

uninstalled_test_extra_programs = \
	echo-server				\
	filter-cat				\
	gapplication-example-actions		\
	gapplication-example-cmdline		\
	gapplication-example-cmdline2		\
	gapplication-example-cmdline3		\
	gapplication-example-dbushooks		\
	gapplication-example-open		\
	gdbus-example-export			\
	gdbus-example-own-name			\
	gdbus-example-peer			\
	gdbus-example-proxy-subclass		\
	gdbus-example-server			\
	gdbus-example-subtree			\
	gdbus-example-watch-name		\
	gdbus-example-watch-proxy		\
	httpd					\
	proxy					\
	resolver				\
	send-data				\
	socket-server				\
	$(NULL)

test_extra_programs = \
	gdbus-connection-flush-helper		\
	gdbus-testserver			\
	$(NULL)

dist_uninstalled_test_data += $(addprefix schema-tests/,$(schema_tests))
schema_tests = \
	array-default-not-in-choices.gschema.xml	\
	bad-choice.gschema.xml				\
	bad-key.gschema.xml				\
	bad-key2.gschema.xml				\
	bad-key3.gschema.xml				\
	bad-key4.gschema.xml				\
	bad-type.gschema.xml				\
	bare-alias.gschema.xml				\
	choice-alias.gschema.xml			\
	choice-bad.gschema.xml				\
	choice-badtype.gschema.xml			\
	choice-invalid-alias.gschema.xml		\
	choice-missing-value.gschema.xml		\
	choice-shadowed-alias.gschema.xml		\
	choice-upside-down.gschema.xml			\
	choice.gschema.xml				\
	choices-wrong-type.gschema.xml			\
	default-in-aliases.gschema.xml			\
	default-not-in-choices.gschema.xml		\
	default-out-of-range.gschema.xml		\
	empty-key.gschema.xml				\
	enum-with-aliases.gschema.xml			\
	enum-with-bad-default.gschema.xml		\
	enum-with-chained-alias.gschema.xml		\
	enum-with-choice.gschema.xml			\
	enum-with-invalid-alias.gschema.xml		\
	enum-with-repeated-alias.gschema.xml		\
	enum-with-repeated-nick.gschema.xml		\
	enum-with-repeated-value.gschema.xml		\
	enum-with-shadow-alias.gschema.xml		\
	enum.gschema.xml				\
	extend-and-shadow-indirect.gschema.xml		\
	extend-and-shadow.gschema.xml			\
	extend-missing.gschema.xml			\
	extend-nonlist.gschema.xml			\
	extend-self.gschema.xml				\
	extend-wrong-list-indirect.gschema.xml		\
	extend-wrong-list.gschema.xml			\
	extending.gschema.xml				\
	flags-aliased-default.gschema.xml		\
	flags-bad-default.gschema.xml			\
	flags-more-than-one-bit.gschema.xml		\
	flags-with-enum-attr.gschema.xml		\
	flags-with-enum-tag.gschema.xml			\
	from-docs.gschema.xml				\
	incomplete-list.gschema.xml			\
	inherit-gettext-domain.gschema.xml		\
	invalid-path.gschema.xml			\
	key-in-list-indirect.gschema.xml		\
	key-in-list.gschema.xml				\
	list-of-missing.gschema.xml			\
	missing-quotes.gschema.xml			\
	no-default.gschema.xml				\
	overflow.gschema.xml				\
	override-missing.gschema.xml			\
	override-range-error.gschema.xml		\
	override-then-key.gschema.xml			\
	override-twice.gschema.xml			\
	override-type-error.gschema.xml			\
	override.gschema.xml				\
	range-badtype.gschema.xml			\
	range-default-high.gschema.xml			\
	range-default-low.gschema.xml			\
	range-high-default.gschema.xml			\
	range-low-default.gschema.xml			\
	range-missing-max.gschema.xml			\
	range-missing-min.gschema.xml			\
	range-parse-error.gschema.xml			\
	range-wrong-type.gschema.xml			\
	range.gschema.xml				\
	wrong-category.gschema.xml			\
	$(NULL)

test_programs += tls-certificate
tls_certificate_SOURCES = \
	tls-certificate.c			\
	gtesttlsbackend.c			\
	gtesttlsbackend.h
dist_test_data += $(cert_data_files)
cert_data_files = $(addprefix cert-tests/,$(cert_tests))
cert_tests = \
	cert1.pem	\
	cert2.pem	\
	cert3.pem	\
	cert-key.pem	\
	cert-list.pem	\
	key8.pem	\
	key-cert.pem	\
	key.pem		\
	nothing.pem	\
	$(NULL)

uninstalled_test_extra_programs += socket-client
socket_client_SOURCES = \
	socket-client.c				\
	gtlsconsoleinteraction.c		\
	gtlsconsoleinteraction.h
EXTRA_DIST += socket-common.c

uninstalled_test_extra_programs += gdbus-daemon
nodist_gdbus_daemon_SOURCES = \
	$(top_builddir)/gio/gdbus-daemon-generated.c
gdbus_daemon_SOURCES = \
	gdbus-daemon.c				\
	$(top_srcdir)/gio/gdbusdaemon.c

# -----------------------------------------------------------------------------
#  Test programs buildable on UNIX only

if OS_UNIX
test_programs += \
	basic-application			\
	contenttype				\
	file					\
	gdbus-peer-object-manager		\
	live-g-file				\
	socket-address				\
	unix-fd					\
	unix-streams				\
	$(NULL)

# Uninstalled because of the check-for-executable logic in DesktopAppInfo unable to find the installed executable
uninstalled_test_programs += \
	appinfo					\
	desktop-app-info			\
	$(NULL)

dist_test_data += \
	appinfo-test-actions.desktop		\
	appinfo-test-gnome.desktop		\
	appinfo-test-notgnome.desktop		\
	appinfo-test.desktop			\
	appinfo-test2.desktop			\
	file.c					\
	org.gtk.test.dbusappinfo.desktop	\
	$(NULL)

test_extra_programs += \
	appinfo-test				\
	$(NULL)

uninstalled_test_extra_programs += \
	gdbus-example-unix-fd-client		\
	$(NULL)

test_programs += mimeapps
clean-local: clean-mimeapps
clean-mimeapps:
	rm -rf xdgdatadir xdgdatahome

uninstalled_test_programs += gsettings gschema-compile
gsettings_DEPENDENCIES = test.mo
CLEANFILES += test.mo de/LC_MESSAGES/test.mo
gsettings_CFLAGS = $(AM_CFLAGS) -DSRCDIR=\"$(abs_srcdir)\"
test.mo: de.po
	$(AM_V_GEN) $(MSGFMT) -o test.mo $(srcdir)/de.po; \
	$(MKDIR_P) de/LC_MESSAGES; \
	cp -f test.mo de/LC_MESSAGES
EXTRA_DIST += de.po
dist_uninstalled_test_data += \
	org.gtk.test.gschema.xml		\
	org.gtk.schemasourcecheck.gschema.xml	\
	testenum.h				\
	enums.xml.template
# Generated while running the testcase itself...
CLEANFILES += \
	org.gtk.test.enums.xml		\
	gsettings.store			\
	gschemas.compiled		\
	schema-source/gschemas.compiled

test_programs += gdbus-connection-flush
gdbus_connection_flush_SOURCES = \
	gdbus-connection-flush.c		\
	test-io-stream.c			\
	test-io-stream.h			\
	test-pipe-unix.c			\
	test-pipe-unix.h

test_programs += gdbus-non-socket
gdbus_non_socket_SOURCES = \
	gdbus-non-socket.c			\
	gdbus-tests.c				\
	gdbus-tests.h				\
	test-io-stream.c			\
	test-io-stream.h			\
	test-pipe-unix.c			\
	test-pipe-unix.h

# These three are manual-run tests because they need a session bus but don't bring one up themselves
uninstalled_test_extra_programs += gdbus-example-objectmanager-client
gdbus_example_objectmanager_client_LDADD = gdbus-object-manager-example/libgdbus-example-objectmanager.la $(LDADD)

uninstalled_test_extra_programs += gdbus-example-objectmanager-server
gdbus_example_objectmanager_server_LDADD = gdbus-object-manager-example/libgdbus-example-objectmanager.la $(LDADD)

uninstalled_test_extra_programs += gdbus-test-fixture
gdbus_test_fixture_LDADD = gdbus-object-manager-example/libgdbus-example-objectmanager.la $(LDADD)

# This is peer to peer so it doesn't need a session bus (so we can run it normally)
test_programs += gdbus-peer
gdbus_peer_LDADD = gdbus-object-manager-example/libgdbus-example-objectmanager.la $(LDADD)

# This test is currently unreliable
test_extra_programs += gdbus-overflow

# -----------------------------------------------------------------------------
#  Test programs that need to bring up a session bus (requires dbus-daemon)

if HAVE_DBUS_DAEMON
gdbus_sessionbus_sources = gdbus-sessionbus.c gdbus-sessionbus.h gdbus-tests.h gdbus-tests.c

test_programs += \
	actions					\
	dbus-appinfo				\
	gapplication				\
	gdbus-auth				\
	gdbus-bz627724				\
	gdbus-close-pending			\
	gdbus-connection			\
	gdbus-connection-loss			\
	gdbus-connection-slow			\
	gdbus-error				\
	gdbus-exit-on-close			\
	gdbus-export				\
	gdbus-introspection			\
	gdbus-names				\
	gdbus-proxy				\
	gdbus-proxy-threads			\
	gdbus-proxy-well-known-name		\
	gdbus-test-codegen			\
	gdbus-threading				\
	gmenumodel				\
	$(NULL)

gdbus_proxy_threads_CFLAGS = $(AM_CFLAGS) $(DBUS1_CFLAGS)
actions_SOURCES                          = $(gdbus_sessionbus_sources) actions.c
dbus_appinfo_SOURCES                     = $(gdbus_sessionbus_sources) dbus-appinfo.c
gapplication_SOURCES                     = $(gdbus_sessionbus_sources) gapplication.c
gdbus_auth_SOURCES                       = $(gdbus_sessionbus_sources) gdbus-auth.c
gdbus_bz627724_SOURCES                   = $(gdbus_sessionbus_sources) gdbus-bz627724.c
gdbus_close_pending_SOURCES              = $(gdbus_sessionbus_sources) gdbus-close-pending.c
gdbus_connection_SOURCES                 = $(gdbus_sessionbus_sources) gdbus-connection.c
gdbus_connection_loss_SOURCES            = $(gdbus_sessionbus_sources) gdbus-connection-loss.c
gdbus_connection_slow_SOURCES            = $(gdbus_sessionbus_sources) gdbus-connection-slow.c
gdbus_error_SOURCES                      = $(gdbus_sessionbus_sources) gdbus-error.c
gdbus_exit_on_close_SOURCES              = $(gdbus_sessionbus_sources) gdbus-exit-on-close.c
gdbus_export_SOURCES                     = $(gdbus_sessionbus_sources) gdbus-export.c
gdbus_introspection_SOURCES              = $(gdbus_sessionbus_sources) gdbus-introspection.c
gdbus_names_SOURCES                      = $(gdbus_sessionbus_sources) gdbus-names.c
gdbus_proxy_SOURCES                      = $(gdbus_sessionbus_sources) gdbus-proxy.c
gdbus_proxy_threads_SOURCES              = $(gdbus_sessionbus_sources) gdbus-proxy-threads.c
gdbus_proxy_well_known_name_SOURCES      = $(gdbus_sessionbus_sources) gdbus-proxy-well-known-name.c
gdbus_test_codegen_SOURCES               = $(gdbus_sessionbus_sources) gdbus-test-codegen.c
gdbus_threading_SOURCES                  = $(gdbus_sessionbus_sources) gdbus-threading.c
gmenumodel_SOURCES                       = $(gdbus_sessionbus_sources) gmenumodel.c
nodist_gdbus_test_codegen_SOURCES        = gdbus-test-codegen-generated.c gdbus-test-codegen-generated.h

gdbus-test-codegen.o: gdbus-test-codegen-generated.h
gdbus-test-codegen-generated.h gdbus-test-codegen-generated.c: test-codegen.xml Makefile $(top_builddir)/gio/gdbus-2.0/codegen/gdbus-codegen
	$(AM_V_GEN) UNINSTALLED_GLIB_SRCDIR=$(top_srcdir) \
		UNINSTALLED_GLIB_BUILDDIR=$(top_builddir) \
		$(PYTHON) $(top_builddir)/gio/gdbus-2.0/codegen/gdbus-codegen \
		--interface-prefix org.project. \
		--generate-c-code gdbus-test-codegen-generated \
		--c-generate-object-manager \
		--c-namespace Foo_iGen \
		--generate-docbook gdbus-test-codegen-generated-doc \
		--annotate "org.project.Bar" Key1 Value1 \
		--annotate "org.project.Bar" org.gtk.GDBus.Internal Value2 \
		--annotate "org.project.Bar.HelloWorld()" Key3 Value3 \
		--annotate "org.project.Bar::TestSignal" Key4 Value4 \
		--annotate "org.project.Bar:ay" Key5 Value5 \
		--annotate "org.project.Bar.TestPrimitiveTypes()[val_int32]" Key6 Value6 \
		--annotate "org.project.Bar.TestPrimitiveTypes()[ret_uint32]" Key7 Value7 \
		--annotate "org.project.Bar::TestSignal[array_of_strings]" Key8 Value8 \
		$(srcdir)/test-codegen.xml \
		$(NULL)

EXTRA_DIST += test-codegen.xml
CLEANFILES += gdbus-test-codegen-generated.[ch] gdbus-test-codegen-generated-doc-*.xml
endif # OS_UNIX
endif # HAVE_DBUS_DAEMON

# -----------------------------------------------------------------------------

if OS_WIN32
test_programs += win32-streams
endif

if PLATFORM_WIN32
no_undefined = -no-undefined
endif

if HAVE_DBUS1
test_programs += gdbus-serialization
gdbus_serialization_CFLAGS = $(AM_CFLAGS) $(DBUS1_CFLAGS)
gdbus_serialization_LDADD = $(LDADD) $(DBUS1_LIBS)
gdbus_serialization_SOURCES = \
	gdbus-serialization.c			\
	gdbus-tests.h				\
	gdbus-tests.c
endif

# -----------------------------------------------------------------------------
#  The resources test is a bit more complicated...

test_programs += resources
resources_SOURCES = resources.c
nodist_resources_SOURCES = test_resources.c test_resources2.c test_resources2.h
resources_DEPENDENCIES = test.gresource

test_ltlibraries += libresourceplugin.la
libresourceplugin_la_SOURCES = resourceplugin.c plugin_resources.c
libresourceplugin_la_LDFLAGS = -avoid-version -module -export-dynamic $(no_undefined)
libresourceplugin_la_LIBADD = $(LDADD)

# libtool contains a bug whereby the created .la file doesn't contain the correct dlname='' in the case that
# you're building a library but not installing it.  This is apparently because the only considered use for an
# uninstalled library is as a convenience library for linking (despite the fact that we give -module).  The lack
# of dlname='' in the .la trips up libltdl and GModule as well.  We can trick libtool into believing that we
# will install the module by giving it a bogus -rpath for the uninstalled cases.
#
# See http://lists.gnu.org/archive/html/bug-libtool/2013-05/msg00009.html
if !ENABLE_INSTALLED_TESTS
libresourceplugin_la_LDFLAGS += -rpath /
endif

if CROSS_COMPILING
  glib_compile_resources=$(GLIB_COMPILE_RESOURCES)
else
  glib_compile_resources=$(top_builddir)/gio/glib-compile-resources
endif

resources.o: test_resources2.h
test_resources.c: test2.gresource.xml Makefile $(shell $(glib_compile_resources) --sourcedir=$(srcdir) --generate-dependencies $(srcdir)/test2.gresource.xml)
	$(AM_V_GEN) $(glib_compile_resources) --target=$@ --sourcedir=$(srcdir) --generate-source --c-name _g_test1 $<

test_resources2.h test_resources2.c: test3.gresource.xml Makefile $(shell $(glib_compile_resources) --sourcedir=$(srcdir) --generate-dependencies $(srcdir)/test3.gresource.xml)
	$(AM_V_GEN) $(glib_compile_resources) --target=$@ --sourcedir=$(srcdir) --generate --c-name _g_test2 --manual-register $<

plugin_resources.c: test4.gresource.xml Makefile $(shell $(glib_compile_resources) --sourcedir=$(srcdir) --generate-dependencies $(srcdir)/test4.gresource.xml)
	$(AM_V_GEN) $(glib_compile_resources) --target=$@ --sourcedir=$(srcdir) --generate-source --c-name _g_plugin $<

test.gresource: test.gresource.xml Makefile $(shell $(glib_compile_resources) --sourcedir=$(srcdir) --generate-dependencies $(srcdir)/test.gresource.xml)
	$(AM_V_GEN) $(glib_compile_resources) --target=$@ --sourcedir=$(srcdir) $<

EXTRA_DIST += test.gresource.xml test1.txt test2.gresource.xml test2.txt test3.gresource.xml test3.txt test4.gresource.xml

CLEANFILES += test_resources.c test_resources2.[ch] plugin_resources.c test.gresource
