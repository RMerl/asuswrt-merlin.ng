if REALLYSTATIC
AM_LIBS		= $(top_builddir)/libntfs-3g/.libs/libntfs-3g.a $(NTFSPROGS_STATIC_LIBS)
# older builds may need -static instead of newer -all-static
AM_LFLAGS	= -static
STATIC_LINK     = $(CC) $(AM_CFLAGS) $(CFLAGS) $(LDFLAGS) -o $@
else
AM_LIBS		= $(top_builddir)/libntfs-3g/libntfs-3g.la
AM_LFLAGS	= $(all_libraries)
LIBTOOL_LINK    = $(LIBTOOL) --mode=link $(CCLD) $(AM_CFLAGS) $(CFLAGS) $(LDFLAGS) -o $@
endif

# Workaround to make REALLYSTATIC work with automake 1.5.
LINK=$(STATIC_LINK) $(LIBTOOL_LINK)

if ENABLE_NTFSPROGS

bin_PROGRAMS		= ntfsfix ntfsinfo ntfscluster ntfsls ntfscat ntfscmp
sbin_PROGRAMS		= mkntfs ntfslabel ntfsundelete ntfsresize ntfsclone \
			  ntfscp
EXTRA_PROGRAM_NAMES	= ntfswipe ntfstruncate ntfsrecover \
			  ntfsusermap ntfssecaudit

QUARANTINED_PROGRAM_NAMES = ntfsdump_logfile ntfsmftalloc ntfsmove ntfsck \
			   ntfsfallocate

man_MANS		= mkntfs.8 ntfsfix.8 ntfslabel.8 ntfsinfo.8 \
			  ntfsundelete.8 ntfsresize.8 ntfsprogs.8 ntfsls.8 \
			  ntfsclone.8 ntfscluster.8 ntfscat.8 ntfscp.8 \
			  ntfscmp.8 ntfswipe.8 ntfstruncate.8 \
			  ntfsdecrypt.8 ntfsfallocate.8 ntfsrecover.8 \
			  ntfsusermap.8 ntfssecaudit.8
EXTRA_MANS		=

CLEANFILES		= $(EXTRA_PROGRAMS)

MAINTAINERCLEANFILES = Makefile.in

if ENABLE_CRYPTO
EXTRA_PROGRAM_NAMES	+= ntfsdecrypt
endif

if ENABLE_EXTRAS
bin_PROGRAMS		+= $(EXTRA_PROGRAM_NAMES)
if ENABLE_QUARANTINED
bin_PROGRAMS		+= $(QUARANTINED_PROGRAM_NAMES)
endif
else
EXTRA_PROGRAMS		= $(EXTRA_PROGRAM_NAMES)
endif

# Set the include path.
AM_CPPFLAGS		= -I$(top_srcdir)/include/ntfs-3g $(all_includes)

ntfsfix_SOURCES		= ntfsfix.c utils.c utils.h
ntfsfix_LDADD		= $(AM_LIBS)
ntfsfix_LDFLAGS		= $(AM_LFLAGS)

mkntfs_CPPFLAGS		= $(AM_CPPFLAGS) $(MKNTFS_CPPFLAGS)
mkntfs_SOURCES		= attrdef.c attrdef.h boot.c boot.h sd.c sd.h mkntfs.c utils.c utils.h
mkntfs_LDADD		= $(AM_LIBS) $(MKNTFS_LIBS)
mkntfs_LDFLAGS		= $(AM_LFLAGS)

ntfslabel_SOURCES	= ntfslabel.c utils.c utils.h
ntfslabel_LDADD		= $(AM_LIBS)
ntfslabel_LDFLAGS	= $(AM_LFLAGS)

ntfsinfo_SOURCES	= ntfsinfo.c utils.c utils.h
ntfsinfo_LDADD		= $(AM_LIBS)
ntfsinfo_LDFLAGS	= $(AM_LFLAGS)

ntfsundelete_SOURCES	= ntfsundelete.c ntfsundelete.h utils.c utils.h list.h
ntfsundelete_LDADD	= $(AM_LIBS)
ntfsundelete_LDFLAGS	= $(AM_LFLAGS)

ntfsresize_SOURCES	= ntfsresize.c utils.c utils.h
ntfsresize_LDADD	= $(AM_LIBS)
ntfsresize_LDFLAGS	= $(AM_LFLAGS)

ntfsclone_SOURCES	= ntfsclone.c utils.c utils.h
ntfsclone_LDADD		= $(AM_LIBS)
ntfsclone_LDFLAGS	= $(AM_LFLAGS)

ntfscluster_SOURCES	= ntfscluster.c ntfscluster.h cluster.c cluster.h utils.c utils.h
ntfscluster_LDADD	= $(AM_LIBS)
ntfscluster_LDFLAGS	= $(AM_LFLAGS)

ntfsls_SOURCES		= ntfsls.c utils.c utils.h list.h
ntfsls_LDADD		= $(AM_LIBS)
ntfsls_LDFLAGS		= $(AM_LFLAGS)

ntfscat_SOURCES		= ntfscat.c ntfscat.h utils.c utils.h
ntfscat_LDADD		= $(AM_LIBS)
ntfscat_LDFLAGS		= $(AM_LFLAGS)

ntfscp_SOURCES		= ntfscp.c utils.c utils.h
ntfscp_LDADD		= $(AM_LIBS)
ntfscp_LDFLAGS		= $(AM_LFLAGS)

ntfsck_SOURCES		= ntfsck.c utils.c utils.h
ntfsck_LDADD		= $(AM_LIBS)
ntfsck_LDFLAGS		= $(AM_LFLAGS)

ntfscmp_SOURCES		= ntfscmp.c utils.c utils.h
ntfscmp_LDADD		= $(AM_LIBS)
ntfscmp_LDFLAGS		= $(AM_LFLAGS)

ntfsrecover_SOURCES	= playlog.c ntfsrecover.c utils.c utils.h ntfsrecover.h
ntfsrecover_LDADD	= $(AM_LIBS) $(NTFSRECOVER_LIBS)
ntfsrecover_LDFLAGS	= $(AM_LFLAGS)

ntfsusermap_SOURCES	= ntfsusermap.c utils.c utils.h
ntfsusermap_LDADD	= $(AM_LIBS) $(NTFSRECOVER_LIBS)
ntfsusermap_LDFLAGS	= $(AM_LFLAGS)

ntfssecaudit_SOURCES	= ntfssecaudit.c utils.c utils.h
ntfssecaudit_LDADD	= $(AM_LIBS) $(NTFSRECOVER_LIBS)
ntfssecaudit_LDFLAGS	= $(AM_LFLAGS)

# We don't distribute these

ntfstruncate_SOURCES	= attrdef.c ntfstruncate.c utils.c utils.h
ntfstruncate_LDADD	= $(AM_LIBS)
ntfstruncate_LDFLAGS	= $(AM_LFLAGS)

ntfsmftalloc_SOURCES	= ntfsmftalloc.c utils.c utils.h
ntfsmftalloc_LDADD	= $(AM_LIBS)
ntfsmftalloc_LDFLAGS	= $(AM_LFLAGS)

ntfsmove_SOURCES	= ntfsmove.c ntfsmove.h utils.c utils.h
ntfsmove_LDADD		= $(AM_LIBS)
ntfsmove_LDFLAGS	= $(AM_LFLAGS)

ntfswipe_SOURCES	= ntfswipe.c ntfswipe.h utils.c utils.h
ntfswipe_LDADD		= $(AM_LIBS)
ntfswipe_LDFLAGS	= $(AM_LFLAGS)

ntfsdump_logfile_SOURCES= ntfsdump_logfile.c
ntfsdump_logfile_LDADD	= $(AM_LIBS)
ntfsdump_logfile_LDFLAGS= $(AM_LFLAGS)

ntfsfallocate_SOURCES	= ntfsfallocate.c utils.c utils.h
ntfsfallocate_LDADD	= $(AM_LIBS)
ntfsfallocate_LDFLAGS	= $(AM_LFLAGS)

if ENABLE_CRYPTO
ntfsdecrypt_SOURCES	= ntfsdecrypt.c utils.c utils.h
ntfsdecrypt_LDADD	= $(AM_LIBS) $(GNUTLS_LIBS) $(LIBGCRYPT_LIBS)
ntfsdecrypt_LDFLAGS	= $(AM_LFLAGS)
ntfsdecrypt_CFLAGS	= $(GNUTLS_CFLAGS) $(LIBGCRYPT_CFLAGS)
endif

# Extra targets

strip:	$(bin_PROGRAMS) $(sbin_PROGRAMS)
	$(STRIP) $^

libs:
	(cd ../libntfs-3g && $(MAKE) libs) || exit 1;

extra:	extras

extras:	libs $(EXTRA_PROGRAMS)

# mkfs.ntfs[.8] hard link

if ENABLE_MOUNT_HELPER
install-exec-hook:
	$(INSTALL) -d $(DESTDIR)/$(sbindir)
	$(LN_S) -f $(sbindir)/mkntfs $(DESTDIR)$(sbindir)/mkfs.ntfs

install-data-hook:
	$(INSTALL) -d $(DESTDIR)$(man8dir)
	$(LN_S) -f mkntfs.8 $(DESTDIR)$(man8dir)/mkfs.ntfs.8

uninstall-local:
	$(RM) -f $(DESTDIR)/sbin/mkfs.ntfs
	$(RM) -f $(DESTDIR)$(man8dir)/mkfs.ntfs.8
endif

endif
