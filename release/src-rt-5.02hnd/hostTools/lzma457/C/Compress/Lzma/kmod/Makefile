
# Copyright (C) 2006-2008 Junjiro Okajima
# Copyright (C) 2006-2008 Tomas Matejicek, slax.org
#
# LICENSE follows the described one in lzma.txt.

# $Id: Makefile,v 1.3 2008-03-12 01:49:39 jro Exp $

# # the environment variables are not inherited since 2.6.23
# ifdef SQLZMA_EXTRA_CFLAGS
# EXTRA_CFLAGS += ${SQLZMA_EXTRA_CFLAGS}
# endif

obj-m += unlzma.o sqlzma.o
unlzma-y := module.o
sqlzma-y := uncomp.o
