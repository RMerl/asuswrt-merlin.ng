SUBDIRS=. lsb

check_PROGRAMS=control pcm pcm_min latency seq \
	       playmidi1 timer rawmidi midiloop \
	       oldapi queue_timer namehint client_event_filter

control_LDADD=../src/libasound.la
pcm_LDADD=../src/libasound.la
pcm_min_LDADD=../src/libasound.la
latency_LDADD=../src/libasound.la
seq_LDADD=../src/libasound.la
playmidi1_LDADD=../src/libasound.la
timer_LDADD=../src/libasound.la
rawmidi_LDADD=../src/libasound.la
midiloop_LDADD=../src/libasound.la
oldapi_LDADD=../src/libasound.la
queue_timer_LDADD=../src/libasound.la
namehint_LDADD=../src/libasound.la
client_event_filter_LDADD=../src/libasound.la
code_CFLAGS=-Wall -pipe -g -O2

INCLUDES=-I$(top_srcdir)/include
AM_CFLAGS=-Wall -pipe -g

EXTRA_DIST=seq-decoder.c seq-sender.c midifile.h midifile.c midifile.3
