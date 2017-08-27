#
# $Id$
#

SOURCES		:= rlm_ippool_tool.c
TARGET		:= rlm_ippool_tool
TGT_PREREQS	:= libfreeradius-radius.a
TGT_PRLIBS	:= ${LIBS}

SRC_CFLAGS	:= $(rlm_ippool_CFLAGS) 
TGT_LDLIBS	:= $(rlm_ippool_LDLIBS)

MAN		:= rlm_ippool_tool.8
