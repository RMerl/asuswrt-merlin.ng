
EXTRA_DIST = ntfs-3g_common.h

MAINTAINERCLEANFILES = $(srcdir)/Makefile.in

if FUSE_INTERNAL
FUSE_CFLAGS = -I$(top_srcdir)/include/fuse-lite
FUSE_LIBS   = $(top_builddir)/libfuse-lite/libfuse-lite.la
else
FUSE_CFLAGS = $(FUSE_MODULE_CFLAGS)
FUSE_LIBS   = $(FUSE_MODULE_LIBS)
endif

if !DISABLE_PLUGINS
plugindir        = $(libdir)/ntfs-3g
PLUGIN_CFLAGS    = -DPLUGIN_DIR=\"$(plugindir)\"
endif

if ENABLE_NTFS_3G

bin_PROGRAMS	 = ntfs-3g.probe
rootbin_PROGRAMS = ntfs-3g lowntfs-3g
rootsbin_DATA 	 = #Create directory
man_MANS	 = ntfs-3g.8 ntfs-3g.probe.8

ntfs_3g_LDADD    = $(LIBDL) $(FUSE_LIBS) $(top_builddir)/libntfs-3g/libntfs-3g.la
if REALLYSTATIC
ntfs_3g_LDFLAGS  = $(AM_LDFLAGS) -all-static
endif
ntfs_3g_CFLAGS   =			\
	$(AM_CFLAGS) 			\
	-DFUSE_USE_VERSION=26 		\
	$(FUSE_CFLAGS) 			\
	-I$(top_srcdir)/include/ntfs-3g	\
	$(PLUGIN_CFLAGS)
ntfs_3g_SOURCES  = ntfs-3g.c ntfs-3g_common.c

lowntfs_3g_LDADD    = $(LIBDL) $(FUSE_LIBS) $(top_builddir)/libntfs-3g/libntfs-3g.la
if REALLYSTATIC
lowntfs_3g_LDFLAGS  = $(AM_LDFLAGS) -all-static
endif
lowntfs_3g_CFLAGS   =			\
	$(AM_CFLAGS) 			\
	-DFUSE_USE_VERSION=26 		\
	$(FUSE_CFLAGS) 			\
	-I$(top_srcdir)/include/ntfs-3g	\
	$(PLUGIN_CFLAGS)
lowntfs_3g_SOURCES  = lowntfs-3g.c ntfs-3g_common.c

ntfs_3g_probe_LDADD 	= $(top_builddir)/libntfs-3g/libntfs-3g.la
if REALLYSTATIC
ntfs_3g_probe_LDFLAGS	= $(AM_LDFLAGS) -all-static
endif
ntfs_3g_probe_CFLAGS  	= $(AM_CFLAGS) -I$(top_srcdir)/include/ntfs-3g
ntfs_3g_probe_SOURCES 	= ntfs-3g.probe.c

drivers : $(FUSE_LIBS) ntfs-3g lowntfs-3g

install-exec-hook:
if RUN_LDCONFIG
	$(LDCONFIG)
endif
if !DISABLE_PLUGINS
	$(MKDIR_P) $(DESTDIR)/$(plugindir)
endif

if ENABLE_MOUNT_HELPER
install-exec-local:	install-rootbinPROGRAMS
	$(MKDIR_P) "$(DESTDIR)/sbin"
	$(LN_S) -f "$(rootbindir)/ntfs-3g" "$(DESTDIR)/sbin/mount.ntfs-3g"
	$(LN_S) -f "$(rootbindir)/lowntfs-3g" "$(DESTDIR)/sbin/mount.lowntfs-3g"

install-data-local:	install-man8
	$(LN_S) -f ntfs-3g.8 "$(DESTDIR)$(man8dir)/mount.ntfs-3g.8"
	$(LN_S) -f ntfs-3g.8 "$(DESTDIR)$(man8dir)/mount.lowntfs-3g.8"

uninstall-local:
	$(RM) -f "$(DESTDIR)$(man8dir)/mount.ntfs-3g.8"
	$(RM) -f "$(DESTDIR)/sbin/mount.ntfs-3g" "$(DESTDIR)/sbin/mount.lowntfs-3g"
endif

endif # ENABLE_NTFS_3G
