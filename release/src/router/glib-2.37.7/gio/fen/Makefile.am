include $(top_srcdir)/glib.mk

noinst_LTLIBRARIES += libfen.la

libfen_la_SOURCES = 		\
	fen-dump.c		\
	fen-dump.h		\
	fen-kernel.c 		\
	fen-kernel.h 		\
	fen-node.c		\
	fen-node.h		\
	fen-helper.c 		\
	fen-helper.h		\
	gfenfilemonitor.c		\
	gfenfilemonitor.h		\
	gfendirectorymonitor.c	\
	gfendirectorymonitor.h	\
	$(NULL)

libfen_la_CFLAGS = \
	$(GLIB_HIDDEN_VISIBILITY_CFLAGS)	\
	-DG_LOG_DOMAIN=\"GLib-GIO\"	\
	$(gio_INCLUDES) 			\
	$(GLIB_DEBUG_FLAGS)		\
	-DGIO_MODULE_DIR=\"$(GIO_MODULE_DIR)\"	\
	-DGIO_COMPILATION		\
	-DG_DISABLE_DEPRECATED
