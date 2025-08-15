# SPDX-License-Identifier: 0BSD

CC ?= cc
VERSION ?= $(shell git describe)
DATE_EPOCH = LC_ALL=C date $(shell date -d @0 > /dev/null 2>&1 && echo "-d @" || echo "-r ")
SOURCE_DATE_EPOCH ?= $(shell git log -1 --no-show-signature --format=%at "netstat-nat.1.in")
MANUAL_DATE ?= $(shell $(DATE_EPOCH)$(SOURCE_DATE_EPOCH) +"%B %e, %Y")
PREFIX ?= /usr/local


CPPFLAGS += -D_GNU_SOURCE
CFLAGS   += -O3 -g -Wall -Wextra -Wno-string-plus-int


.PHONY: all check clean install

all: netstat-nat netstat-nat.1
clean:
	rm -f netstat-nat tst netstat-nat.1
check: tst netstat-nat
	./tst
	./test


install: all
	mkdir -p $(DESTDIR)$(PREFIX)/bin $(DESTDIR)$(PREFIX)/share/man/man1
	cp netstat-nat   $(DESTDIR)$(PREFIX)/bin
	cp netstat-nat.1 $(DESTDIR)$(PREFIX)/share/man/man1

netstat-nat.1: netstat-nat.1.in
	awk '{gsub(/ \^/, " \\(ha"); gsub(/ ~/, " \\(ti"); if($$1 == ".Dd") $$2 = "$(MANUAL_DATE)"; if($$1 == ".Dt") print ".ds doc-volume-operating-system"; if($$1 == ".Os") $$2 = "netstat-nat-ng $(VERSION)"; print}' < $< > $@

tst: CPPFLAGS += -DTEST
tst: CFLAGS   += -Wno-unused-function -Wno-unused-const-variable
