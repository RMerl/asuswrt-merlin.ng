EXTRA_LTLIBRARIES = libcontrol.la

libcontrol_la_SOURCES = cards.c tlv.c namehint.c hcontrol.c \
                        control.c control_hw.c setup.c control_symbols.c
if BUILD_CTL_PLUGIN_SHM
libcontrol_la_SOURCES += control_shm.c
endif
if BUILD_CTL_PLUGIN_EXT
libcontrol_la_SOURCES += control_ext.c
endif

noinst_HEADERS = control_local.h

all: libcontrol.la


INCLUDES=-I$(top_srcdir)/include
