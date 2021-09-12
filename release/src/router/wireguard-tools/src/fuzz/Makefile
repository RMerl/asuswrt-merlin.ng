# SPDX-License-Identifier: GPL-2.0
#
# Copyright (C) 2018-2020 Jason A. Donenfeld <Jason@zx2c4.com>. All Rights Reserved.

FUZZERS := config uapi stringlist cmd set setconf

all: $(FUZZERS)

CFLAGS ?= -O3 -march=native -g
CFLAGS += -fsanitize=fuzzer -fsanitize=address -std=gnu11 -idirafter ../uapi -D_GNU_SOURCE
CC := clang

config: config.c ../config.c ../encoding.c
	$(CC) $(CFLAGS) -o $@ $<

uapi: uapi.c ../ipc.c ../curve25519.c ../encoding.c
	$(CC) $(CFLAGS) -o $@ $<

stringlist: stringlist.c ../ipc.c ../curve25519.c ../encoding.c
	$(CC) $(CFLAGS) -o $@ $<

cmd: cmd.c $(wildcard ../*.c)
	$(CC) $(CFLAGS) -D'RUNSTATEDIR="/var/empty"' -D'main(a,b)=wg_main(a,b)' -o $@ $^

set: set.c ../set.c ../ipc.c ../encoding.c ../curve25519.c ../config.c
	$(CC) $(CFLAGS) -o $@ $<

setconf: setconf.c ../setconf.c ../ipc.c ../encoding.c ../curve25519.c ../config.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	$(RM) $(FUZZERS)

.PHONY: all clean
