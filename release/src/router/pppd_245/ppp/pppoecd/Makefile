# PPPoEcd Makefile
#
# Copyright (C) 2001 Broadcom Corporation
#
# $Id: Makefile,v 1.1.1.1 2001/08/08 01:20:46 mhuang Exp $
# 

# Uncomment the following line to enable CCP compression
# CCP_SUPPORT=y

# Uncomment the following line to enable standard CHAP authentication
CHAP_SUPPORT=y

# Uncomment the following to enable debugging
# DFLAGS	+= -g -DDEBUG

CFLAGS	:= -D_linux_=1 -DHAVE_PATHS_H -DHAVE_MMAP
CFLAGS	+= -I. -I../pppd -I../../app/inc -O2 -pipe -Wall
CFLAGS	+= $(DFLAGS)

vpath	%.c ../pppd
OBJS	:= magic.o fsm.o lcp.o ipcp.o upap.o
vpath	%.c ../pppd/plugins/pppoe
OBJS	+= pppoehash.o pppoe_client.o libpppoe.o
OBJS	+= main.o auth.o options.o utils.o sys-linux.o pppoe.o

ifdef CCP_SUPPORT
CFLAGS	+= -DCCP_SUPPORT=1
OBJS	+= ccp.o
endif

ifdef CHAP_SUPPORT
CFLAGS	+= -DCHAP_SUPPORT=1
OBJS	+= md5.o chap.o
endif

# BRCM begin
all: pppoecd.a

pppoecd.a: $(OBJS)
	$(AR) rcs pppoecd.a  $(OBJS)
# BRCM end

pppoecd: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f *.o pppoecd
