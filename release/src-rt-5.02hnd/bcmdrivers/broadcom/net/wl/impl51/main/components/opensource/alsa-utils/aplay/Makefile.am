LIBRT = @LIBRT@

INCLUDES = -I$(top_srcdir)/include
LDADD = $(LIBINTL) $(LIBRT)

# debug flags
#LDFLAGS = -static
#LDADD += -ldl

bin_PROGRAMS = aplay
man_MANS = aplay.1 arecord.1
noinst_HEADERS = formats.h

EXTRA_DIST = aplay.1 arecord.1
EXTRA_CLEAN = arecord

arecord: aplay
	rm -f arecord
	$(LN_S) $< $@

arecord.1: aplay.1
	rm -f arecord.1
	$(LN_S) $< $@

install-exec-hook: arecord
	rm -f $(DESTDIR)$(bindir)/arecord
	(cd $(DESTDIR)$(bindir) && $(LN_S) aplay arecord)

install-data-hook:
	rm -f $(DESTDIR)$(mandir)/man1/arecord.1
	(cd $(DESTDIR)$(mandir)/man1 && $(LN_S) aplay.1 arecord.1)
