
SUBDIRS =

if USE_LEGACY_SYSTEMD
if USE_CHARON
  SUBDIRS += systemd
endif
endif

if USE_SYSTEMD
if USE_SWANCTL
  SUBDIRS += systemd-swanctl
endif
endif
