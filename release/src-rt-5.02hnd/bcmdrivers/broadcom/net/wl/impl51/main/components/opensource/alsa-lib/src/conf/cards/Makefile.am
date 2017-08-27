alsaconfigdir = @ALSA_CONFIG_DIR@
alsadir = $(alsaconfigdir)/cards
cfg_files = aliases.conf \
	AACI.conf \
	ATIIXP.conf \
	ATIIXP-SPDMA.conf \
	ATIIXP-MODEM.conf \
	AU8810.conf \
	AU8820.conf \
	AU8830.conf \
	Audigy.conf \
	Audigy2.conf \
	Aureon51.conf \
	Aureon71.conf \
	CA0106.conf \
	CMI8338.conf \
	CMI8338-SWIEC.conf \
	CMI8738-MC6.conf \
	CMI8738-MC8.conf \
	CMI8788.conf \
	CS46xx.conf \
	EMU10K1.conf \
	EMU10K1X.conf \
	ENS1370.conf \
	ENS1371.conf \
	ES1968.conf \
	FM801.conf \
	GUS.conf \
	HDA-Intel.conf \
	ICE1712.conf \
	ICE1724.conf \
	ICH.conf \
	ICH4.conf \
	ICH-MODEM.conf \
	Maestro3.conf \
	NFORCE.conf \
	PC-Speaker.conf \
	PMac.conf \
	PMacToonie.conf \
	PS3.conf \
	RME9636.conf \
	RME9652.conf \
	SI7018.conf \
	SB-XFi.conf \
	TRID4DWAVENX.conf \
	USB-Audio.conf \
	YMF744.conf \
	VIA686A.conf \
	VIA8233.conf \
	VIA8233A.conf \
	VIA8237.conf \
	VX222.conf \
	VXPocket.conf \
	VXPocket440.conf

if BUILD_ALISP
cfg_files += aliases.alisp
endif

alsa_DATA = $(cfg_files)

if BUILD_ALISP
SI7018dir = $(alsaconfigdir)/cards/SI7018
SI7018_files = \
	SI7018/sndoc-mixer.alisp \
	SI7018/sndop-mixer.alisp
SI7018_DATA = $(SI7018_files)
else
SI7018_files=
endif

EXTRA_DIST = \
	$(cfg_files) \
	$(SI7018_files)
