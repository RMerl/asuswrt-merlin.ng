AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan \
	-I$(top_srcdir)/src/libtncif \
	-I$(top_srcdir)/src/libtls

AM_LDFLAGS = \
	-no-undefined

ipseclib_LTLIBRARIES = libtnccs.la

libtnccs_la_LIBADD = \
	$(top_builddir)/src/libstrongswan/libstrongswan.la \
	$(top_builddir)/src/libtncif/libtncif.la

if USE_WINDOWS
  libtnccs_la_LIBADD += -lws2_32
endif

libtnccs_la_SOURCES = \
tnc/tnc.h tnc/tnc.c \
tnc/imc/imc.h tnc/imc/imc_manager.h \
tnc/imv/imv.h tnc/imv/imv_manager.h \
tnc/imv/imv_recommendations.h tnc/imv/imv_recommendations.c \
tnc/tnccs/tnccs.h tnc/tnccs/tnccs.c \
tnc/tnccs/tnccs_manager.h tnc/tnccs/tnccs_manager.c

EXTRA_DIST = Android.mk

# build optional plugins
########################

if MONOLITHIC
SUBDIRS =
else
SUBDIRS = .
endif

if USE_TNC_TNCCS
  SUBDIRS += plugins/tnc_tnccs
if MONOLITHIC
  libtnccs_la_LIBADD += plugins/tnc_tnccs/libstrongswan-tnc-tnccs.la
endif
endif

if USE_TNC_IMC
  SUBDIRS += plugins/tnc_imc
if MONOLITHIC
  libtnccs_la_LIBADD += plugins/tnc_imc/libstrongswan-tnc-imc.la
endif
endif

if USE_TNC_IMV
  SUBDIRS += plugins/tnc_imv
if MONOLITHIC
  libtnccs_la_LIBADD += plugins/tnc_imv/libstrongswan-tnc-imv.la
endif
endif

if USE_TNCCS_11
  SUBDIRS += plugins/tnccs_11
if MONOLITHIC
  libtnccs_la_LIBADD += plugins/tnccs_11/libstrongswan-tnccs-11.la
endif
endif

if USE_TNCCS_20
  SUBDIRS += plugins/tnccs_20
if MONOLITHIC
  libtnccs_la_LIBADD += plugins/tnccs_20/libstrongswan-tnccs-20.la
endif
endif

if USE_TNCCS_DYNAMIC
  SUBDIRS += plugins/tnccs_dynamic
if MONOLITHIC
  libtnccs_la_LIBADD += plugins/tnccs_dynamic/libstrongswan-tnccs-dynamic.la
endif
endif
