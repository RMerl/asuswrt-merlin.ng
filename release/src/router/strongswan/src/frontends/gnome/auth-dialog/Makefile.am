nm_libexec_PROGRAMS = nm-strongswan-auth-dialog

nm_strongswan_auth_dialog_CPPFLAGS = \
	$(GTK_CFLAGS) \
	$(LIBSECRET_CFLAGS) \
	$(LIBNM_CFLAGS) \
	$(LIBNMA_CFLAGS) \
	-DG_DISABLE_DEPRECATED \
	-DGNOME_DISABLE_DEPRECATED \
	-DGNOMELOCALEDIR=\"$(datadir)/locale\" \
	-DVERSION=\"$(VERSION)\"

nm_strongswan_auth_dialog_SOURCES =	\
	main.c

nm_strongswan_auth_dialog_LDADD = \
	$(GTK_LIBS) \
	$(LIBSECRET_LIBS) \
	$(LIBNM_LIBS) \
	$(LIBNMA_LIBS)
