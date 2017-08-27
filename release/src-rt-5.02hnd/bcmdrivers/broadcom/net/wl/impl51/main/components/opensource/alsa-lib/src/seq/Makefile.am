EXTRA_LTLIBRARIES=libseq.la

libseq_la_SOURCES = seq_hw.c seq.c seq_event.c seqmid.c seq_midi_event.c \
		    seq_symbols.c
if KEEP_OLD_SYMBOLS
libseq_la_SOURCES += seq_old.c
endif
noinst_HEADERS = seq_local.h

all: libseq.la


INCLUDES=-I$(top_srcdir)/include
