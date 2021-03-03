# SPDX-License-Identifier: GPL-2.0
INSTALL=install
INSTALLDIR=install -m 0755 -d
INSTALLMAN=install -m 0644
# Pass the same parameters as Lintian uses on Debian.
MAN_CHECK=LC_ALL=en_US.UTF-8 MANROFFSEQ='' MANWIDTH=80 man --warnings \
	--encoding=UTF-8 --local-file --troff-device=utf8 --ditroff
# Hide man output, count and print errors.
MAN_REDIRECT=2>&1 >/dev/null | tee /dev/fd/2 | wc -l

SUBDIRS = man3 man7 man8

all clean install check:
	@for subdir in $(SUBDIRS); do $(MAKE) -C $$subdir $@ || exit $$?; done

distclean: clean

.PHONY: install clean distclean check

.EXPORT_ALL_VARIABLES:
