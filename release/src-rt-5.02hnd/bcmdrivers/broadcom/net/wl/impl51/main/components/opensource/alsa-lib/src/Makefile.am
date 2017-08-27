EXTRA_DIST=Versions
COMPATNUM=@LIBTOOL_VERSION_INFO@

if VERSIONED_SYMBOLS
VSYMS = -Wl,--version-script=Versions
else
VSYMS =
endif

if SYMBOLIC_FUNCTIONS
SYMFUNCS = -Wl,-Bsymbolic-functions
else
SYMFUNCS =
endif

lib_LTLIBRARIES = libasound.la
libasound_la_SOURCES = conf.c confmisc.c input.c output.c async.c error.c dlmisc.c socket.c shmarea.c userfile.c names.c

SUBDIRS=control
libasound_la_LIBADD = control/libcontrol.la
if BUILD_MIXER
SUBDIRS += mixer
libasound_la_LIBADD += mixer/libmixer.la
endif
if BUILD_PCM
SUBDIRS += pcm timer
libasound_la_LIBADD += pcm/libpcm.la timer/libtimer.la
endif
if BUILD_RAWMIDI
SUBDIRS += rawmidi
libasound_la_LIBADD += rawmidi/librawmidi.la
endif
if BUILD_HWDEP
SUBDIRS += hwdep
libasound_la_LIBADD += hwdep/libhwdep.la
endif
if BUILD_SEQ
SUBDIRS += seq
libasound_la_LIBADD += seq/libseq.la
endif
if BUILD_ALISP
SUBDIRS += alisp
libasound_la_LIBADD += alisp/libalisp.la
endif
SUBDIRS += compat conf
libasound_la_LIBADD += compat/libcompat.la @ALSA_DEPLIBS@

libasound_la_LDFLAGS = -version-info $(COMPATNUM) $(VSYMS) $(SYMFUNCS) $(LDFLAGS_NOUNDEFINED)

control/libcontrol.la:
	$(MAKE) -C control libcontrol.la

mixer/libmixer.la:
	$(MAKE) -C mixer libmixer.la

ordinary_mixer/libordinarymixer.la:
	$(MAKE) -C ordinary_mixer libordinarymixer.la

pcm/libpcm.la:
	$(MAKE) -C pcm libpcm.la

ordinary_pcm/libordinarypcm.la:
	$(MAKE) -C ordinary_pcm libordinarypcm.la

rawmidi/librawmidi.la:
	$(MAKE) -C rawmidi librawmidi.la

timer/libtimer.la:
	$(MAKE) -C timer libtimer.la

hwdep/libhwdep.la:
	$(MAKE) -C hwdep libhwdep.la

seq/libseq.la:
	$(MAKE) -C seq libseq.la

instr/libinstr.la:
	$(MAKE) -C instr libinstr.la

compat/libcompat.la:
	$(MAKE) -C compat libcompat.la

alisp/libalisp.la:
	$(MAKE) -C alisp libalisp.la

INCLUDES=-I$(top_srcdir)/include
