cfg_files = default.conf front.conf rear.conf center_lfe.conf side.conf\
	    surround40.conf surround41.conf \
	    surround50.conf surround51.conf \
	    surround71.conf iec958.conf hdmi.conf modem.conf \
	    dmix.conf dsnoop.conf \
	    dpl.conf

EXTRA_DIST = $(cfg_files)

alsaconfigdir = @ALSA_CONFIG_DIR@
alsadir = $(alsaconfigdir)/pcm
alsa_DATA = $(cfg_files)
