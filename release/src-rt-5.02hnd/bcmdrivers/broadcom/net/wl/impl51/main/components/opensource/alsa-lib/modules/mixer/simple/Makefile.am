alsaplugindir = @ALSA_PLUGIN_DIR@
pkglibdir = $(alsaplugindir)/smixer

AM_CFLAGS = -g -O2 -W -Wall

INCLUDES=-I$(top_srcdir)/include

pkglib_LTLIBRARIES = smixer-sbase.la \
		     smixer-ac97.la \
		     smixer-hda.la

if BUILD_PYTHON
pkglib_LTLIBRARIES += smixer-python.la
endif

noinst_HEADERS = sbase.h

smixer_sbase_la_SOURCES = sbase.c
smixer_sbase_la_LDFLAGS = -module -avoid-version $(LDFLAGS_NOUNDEFINED)
smixer_sbase_la_LIBADD = ../../../src/libasound.la

smixer_ac97_la_SOURCES = ac97.c sbasedl.c
smixer_ac97_la_LDFLAGS = -module -avoid-version $(LDFLAGS_NOUNDEFINED)
smixer_ac97_la_LIBADD = ../../../src/libasound.la -ldl

smixer_hda_la_SOURCES = hda.c sbasedl.c
smixer_hda_la_LDFLAGS = -module -avoid-version $(LDFLAGS_NOUNDEFINED)
smixer_hda_la_LIBADD = ../../../src/libasound.la -ldl

if BUILD_PYTHON
smixer_python_la_SOURCES = python.c
smixer_python_la_LDFLAGS = -module -avoid-version $(LDFLAGS_NOUNDEFINED)
smixer_python_la_CFLAGS = $(PYTHON_INCLUDES)
smixer_python_la_LIBADD = ../../../src/libasound.la $(PYTHON_LIBS)
endif
