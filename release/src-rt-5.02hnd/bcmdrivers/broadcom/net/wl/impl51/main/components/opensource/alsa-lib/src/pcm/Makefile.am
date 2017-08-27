SUBDIRS =
DIST_SUBDIRS = scopes

EXTRA_LTLIBRARIES = libpcm.la

libpcm_la_SOURCES = atomic.c mask.c interval.c \
		    pcm.c pcm_params.c pcm_simple.c \
		    pcm_hw.c pcm_misc.c pcm_mmap.c pcm_symbols.c

if BUILD_PCM_PLUGIN
libpcm_la_SOURCES += pcm_generic.c pcm_plugin.c
endif
if BUILD_PCM_PLUGIN_COPY
libpcm_la_SOURCES += pcm_copy.c
endif
if BUILD_PCM_PLUGIN_LINEAR
libpcm_la_SOURCES += pcm_linear.c
endif
if BUILD_PCM_PLUGIN_ROUTE
libpcm_la_SOURCES += pcm_route.c
endif
if BUILD_PCM_PLUGIN_MULAW
libpcm_la_SOURCES += pcm_mulaw.c
endif
if BUILD_PCM_PLUGIN_ALAW
libpcm_la_SOURCES += pcm_alaw.c
endif
if BUILD_PCM_PLUGIN_ADPCM
libpcm_la_SOURCES += pcm_adpcm.c
endif
if BUILD_PCM_PLUGIN_RATE
libpcm_la_SOURCES += pcm_rate.c pcm_rate_linear.c
endif
if BUILD_PCM_PLUGIN_PLUG
libpcm_la_SOURCES += pcm_plug.c
endif
if BUILD_PCM_PLUGIN_MULTI
libpcm_la_SOURCES += pcm_multi.c
endif
if BUILD_PCM_PLUGIN_SHM
libpcm_la_SOURCES += pcm_shm.c
endif
if BUILD_PCM_PLUGIN_FILE
libpcm_la_SOURCES += pcm_file.c
endif
if BUILD_PCM_PLUGIN_NULL
libpcm_la_SOURCES += pcm_null.c
endif
if BUILD_PCM_PLUGIN_EMPTY
libpcm_la_SOURCES += pcm_empty.c
endif
if BUILD_PCM_PLUGIN_SHARE
libpcm_la_SOURCES += pcm_share.c
endif
if BUILD_PCM_PLUGIN_METER
libpcm_la_SOURCES += pcm_meter.c
endif
if BUILD_PCM_PLUGIN_HOOKS
libpcm_la_SOURCES += pcm_hooks.c
endif
if BUILD_PCM_PLUGIN_LFLOAT
libpcm_la_SOURCES += pcm_lfloat.c
endif
if BUILD_PCM_PLUGIN_LADSPA
libpcm_la_SOURCES += pcm_ladspa.c
endif
if BUILD_PCM_PLUGIN_DMIX
libpcm_la_SOURCES += pcm_dmix.c
endif
if BUILD_PCM_PLUGIN_DSHARE
libpcm_la_SOURCES += pcm_dshare.c
endif
if BUILD_PCM_PLUGIN_DSNOOP
libpcm_la_SOURCES += pcm_dsnoop.c
endif
if BUILD_PCM_PLUGIN_DMIX
libpcm_la_SOURCES += pcm_direct.c
else
if BUILD_PCM_PLUGIN_DSHARE
libpcm_la_SOURCES += pcm_direct.c
else
if BUILD_PCM_PLUGIN_DSNOOP
libpcm_la_SOURCES += pcm_direct.c
endif
endif
endif
if BUILD_PCM_PLUGIN_ASYM
libpcm_la_SOURCES += pcm_asym.c
endif
if BUILD_PCM_PLUGIN_IEC958
libpcm_la_SOURCES += pcm_iec958.c
endif
if BUILD_PCM_PLUGIN_SOFTVOL
libpcm_la_SOURCES += pcm_softvol.c
endif
if BUILD_PCM_PLUGIN_EXTPLUG
libpcm_la_SOURCES += pcm_extplug.c
endif
if BUILD_PCM_PLUGIN_IOPLUG
libpcm_la_SOURCES += pcm_ioplug.c
endif
if BUILD_PCM_PLUGIN_MMAP_EMUL
libpcm_la_SOURCES += pcm_mmap_emul.c
endif

EXTRA_DIST = pcm_dmix_i386.c pcm_dmix_x86_64.c pcm_dmix_generic.c

noinst_HEADERS = pcm_local.h pcm_plugin.h mask.h mask_inline.h \
	         interval.h interval_inline.h plugin_ops.h ladspa.h \
		 pcm_direct.h pcm_dmix_i386.h pcm_dmix_x86_64.h \
		 pcm_generic.h pcm_ext_parm.h

alsadir = $(datadir)/alsa

all: libpcm.la

INCLUDES=-I$(top_srcdir)/include
