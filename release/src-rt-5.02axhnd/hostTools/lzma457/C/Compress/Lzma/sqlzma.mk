
# Copyright (C) 2006-2007 Junjiro Okajima
# Copyright (C) 2006-2007 Tomas Matejicek, slax.org
#
# LICENSE follows the described one in lzma.txt.

# $Id: sqlzma.mk,v 1.1 2007-11-05 05:43:36 jro Exp $

ifndef Sqlzma
$(error Sqlzma is not defined)
endif

#include makefile.gcc
CFLAGS = -c -O2 -Wall -pedantic -D _LZMA_PROB32

ifdef KDir
include kmod.mk
endif

ifdef UseDebugFlags
DebugFlags = -O0 -g -UNDEBUG
endif
CFLAGS += -DNDEBUG ${DebugFlags}
Tgt = libunlzma.a libunlzma_r.a

all: ${Tgt}

%_r.c: %.c
	ln $< $@
# -pthread
%_r.o: CFLAGS += -D_REENTRANT -include pthread.h

uncomp.o uncomp_r.o: CFLAGS += -I${Sqlzma}
uncomp.o: uncomp.c ${Sqlzma}/sqlzma.h

libunlzma.a: uncomp.o LzmaDecode.o
	${AR} cr $@ $^
libunlzma_r.a: uncomp_r.o LzmaDecode_r.o
	${AR} cr $@ $^

clean: clean_sqlzma
clean_sqlzma:
	$(RM) ${Tgt} uncomp.o uncomp_r.o LzmaDecode_r.o LzmaDecode.o  *~

# Local variables: ;
# compile-command: (concat "make Sqlzma=../../../../.. -f " (file-name-nondirectory (buffer-file-name)));
# End: ;
