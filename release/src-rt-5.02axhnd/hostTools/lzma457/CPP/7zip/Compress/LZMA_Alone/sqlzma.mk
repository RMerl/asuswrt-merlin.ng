
# Copyright (C) 2006-2007 Junjiro Okajima
# Copyright (C) 2006-2007 Tomas Matejicek, slax.org
#
# LICENSE follows the described one in lzma.txt.

# $Id: sqlzma.mk,v 1.1 2007-11-05 05:43:36 jro Exp $

ifndef Sqlzma
$(error Sqlzma is not defined)
endif

include makefile.gcc

ifdef UseDebugFlags
DebugFlags = -Wall -O0 -g -UNDEBUG
endif
# -pthread
CXXFLAGS = ${CFLAGS} -D_REENTRANT -include pthread.h -DNDEBUG ${DebugFlags}
Tgt = liblzma_r.a

all: ${Tgt}

RObjs = LZMAEncoder_r.o Alloc_r.o StreamUtils_r.o MatchFinder_r.o \
	RangeCoderBit_r.o OutBuffer_r.o 7zCrc_r.o

%_r.cc: ../LZMA/%.cpp
	ln $< $@
%_r.c: ../../../../C/%.c
	ln $< $@
%_r.c: ../../../../C/Compress/Lz/%.c
	ln $< $@
%_r.cc: ../../Common/%.cpp
	ln $< $@
%_r.cc: ../RangeCoder/%.cpp
	ln $< $@
LZMAEncoder_r.o: CXXFLAGS += -I../LZMA
Alloc_r.o: CFLAGS += -I../../../../C
StreamUtils_r.o: CXXFLAGS += -I../../Common
MatchFinder_r.o: CFLAGS += -I../../../../C/Compress/Lz
RangeCoderBit_r.o: CXXFLAGS += -I../RangeCoder
OutBuffer_r.o: CXXFLAGS += -I../../Common
7zCrc_r.o: CFLAGS += -I../../../../C

comp.o: CXXFLAGS += -I${Sqlzma}
comp.o: comp.cc ${Sqlzma}/sqlzma.h

liblzma_r.a: ${RObjs} comp.o
	${AR} cr $@ $^

clean: clean_sqlzma
clean_sqlzma:
	$(RM) comp.o *_r.o ${Tgt} *~

# Local variables: ;
# compile-command: (concat "make Sqlzma=../../../../.. -f " (file-name-nondirectory (buffer-file-name)));
# End: ;
