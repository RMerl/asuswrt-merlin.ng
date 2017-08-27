INCLUDES=-I$(top_srcdir)/include

SUBDIRS = include alsactl utils m4 po
if ALSAMIXER
SUBDIRS += alsamixer
endif
if HAVE_MIXER
SUBDIRS += amixer
endif
if HAVE_RAWMIDI
SUBDIRS += amidi
endif
if ALSACONF
SUBDIRS += alsaconf
endif
if HAVE_PCM
SUBDIRS += aplay iecset speaker-test
endif
if HAVE_SEQ
SUBDIRS += seq
endif

EXTRA_DIST= config.rpath  TODO gitcompile
AUTOMAKE_OPTIONS=foreign
ACLOCAL_AMFLAGS = -I m4

rpm: dist
	$(MAKE) -C $@

dist-hook:
	-chmod -R a+r $(distdir)
	@if ! test -z "$(AMTAR)"; then \
		$(AMTAR) --create --verbose --file=- $(distdir) | bzip2 -c -9 > $(distdir).tar.bz2 ; \
	else \
		$(TAR) --create --verbose --file=- $(distdir) | bzip2 -c -9 > $(distdir).tar.bz2 ; \
	fi
