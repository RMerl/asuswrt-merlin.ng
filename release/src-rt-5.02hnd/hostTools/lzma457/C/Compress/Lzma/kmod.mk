
# Copyright (C) 2006-2007 Junjiro Okajima
# Copyright (C) 2006-2007 Tomas Matejicek, slax.org
#
# LICENSE follows the described one in lzma.txt.

# $Id: kmod.mk,v 1.1 2007-11-05 05:43:35 jro Exp $

ifndef Sqlzma
$(error Sqlzma is not defined)
endif
ifndef KDir
$(error KDir is not defined)
endif

#include makefile.gcc

Kmod = kmod
EXTRA_CFLAGS += -Wall -Werror -I${CURDIR} -I${Sqlzma}
# -D_LZMA_PROB32
EXTRA_CFLAGS += $(shell ${CPP} ${CFLAGS} -P testflags.c)

all: ${Kmod}/uncomp.c
	${MAKE} EXTRA_CFLAGS="${EXTRA_CFLAGS}" M=${CURDIR}/${Kmod} \
		-C ${KDir} C=0 V=0 modules

${Kmod}/uncomp.c: uncomp.c
	ln $< $@

clean: clean_kmod
clean_kmod:
	${MAKE} -C ${KDir} M=${CURDIR}/${Kmod} V=0 clean
	${RM} ${Kmod}/*~
	-@test -e ${Kmod}/uncomp.c && \
		diff -q ${Kmod}/uncomp.c uncomp.c > /dev/null && \
		find ${Kmod}/uncomp.c -links +1 | xargs -r ${RM}

# Local variables: ;
# compile-command: (concat "make Sqlzma=../../../../.. KDir=/lib/modules/`uname -r`/build -f " (file-name-nondirectory (buffer-file-name)));
# End: ;
