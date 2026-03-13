# Makefile for `Wget' utility
# Copyright (C) 1995-1997, 2006-2024 Free Software Foundation, Inc.

# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

# Additional permission under GNU GPL version 3 section 7

# If you modify this program, or any covered work, by linking or
# combining it with the OpenSSL project's OpenSSL library (or a
# modified version of that library), containing parts covered by the
# terms of the OpenSSL or SSLeay licenses, the Free Software Foundation
# grants you additional permission to convey the resulting work.
# Corresponding Source for a non-source form of such a combination
# shall include the source code for the parts of OpenSSL used as well
# as that of the covered work.

#
# Version: @VERSION@
#


# We can't help that installing wget.info leaves /usr/share/info/dir
# around, and we need to prevent uninstallation of the possibly
# previously-existing /etc/wgetrc.
distuninstallcheck_listfiles = find . -type f | \
			       grep -Ev '(/share/info/dir|/etc/wgetrc)$$'

# Search for macros in the m4 subdirectory:
ACLOCAL_AMFLAGS = -I m4

# subdirectories in the distribution
SUBDIRS = lib src doc po gnulib_po util fuzz tests testenv

EXTRA_DIST = MAILING-LIST \
             msdos/config.h msdos/Makefile.DJ \
             msdos/Makefile.WC ABOUT-NLS \
             build-aux/build_info.pl build-aux/git-version-gen .version

CLEANFILES = *~ *.bak $(DISTNAME).tar.gz
DISTCLEANFILES = po/stamp-po gnulib_po/stamp-po

BUILT_SOURCES = .version

clean-generic:
	rm -f install-info

.version:
	echo $(VERSION) > $@-t && mv $@-t $@

# Arrange so that .tarball-version appears only in the distribution
# tarball, and never in a checked-out repository.
dist-hook: gen-ChangeLog
	$(AM_V_GEN)echo $(VERSION) > $(distdir)/.tarball-version

gen_start_date = 2014-12-10
.PHONY: gen-ChangeLog
gen-ChangeLog:
	$(AM_V_GEN)if test -d .git; then				\
	  log_fix="$(srcdir)/build-aux/git-log-fix";			\
	  test -e "$$log_fix"						\
	    || amend_git_log=;						\
	  $(top_srcdir)/build-aux/gitlog-to-changelog			\
	    $$amend_git_log --since=$(gen_start_date) > $(distdir)/cl-t && \
            echo >>  $(distdir)/cl-t &&                                 \
            cat ChangeLog-2014-12-10 >>  $(distdir)/cl-t &&             \
	    { rm -f $(distdir)/ChangeLog &&				\
	      mv $(distdir)/cl-t $(distdir)/ChangeLog; }		\
	fi

check-valgrind:
	TESTS_ENVIRONMENT="VALGRIND_TESTS=1" $(MAKE) check

LCOV_INFO=wget.info

clean-lcov:
	rm -rf $(LCOV_INFO) */*.gc?? */.libs/*.gc?? lcov/
	lcov --zerocounters --directory src/

check-coverage: clean clean-lcov
	$(MAKE) CFLAGS="$(CFLAGS) --coverage" LDFLAGS="$(LDFLAGS) --coverage"
	lcov --capture --initial --ignore-errors source,range --directory src/ --output-file $(LCOV_INFO)
	$(MAKE) CFLAGS="$(CFLAGS) --coverage" LDFLAGS="$(LDFLAGS) --coverage" VALGRIND_TESTS=0 check
	lcov --capture --ignore-errors source,range --directory src/ --output-file $(LCOV_INFO)
	lcov --remove $(LCOV_INFO) --ignore-errors source,range,unused '/usr/include/*' '*/lib/*' '*/src/css.l' -o $(LCOV_INFO)
	genhtml --prefix . --ignore-errors source,range,unmapped $(LCOV_INFO) --legend --title "Wget" --output-directory=lcov
	@echo
	@echo "You can now view the coverage report with 'xdg-open lcov/index.html'"

fuzz-coverage: clean clean-lcov
	$(MAKE) -C lib
	$(MAKE) -C src CFLAGS="$(CFLAGS) --coverage" LDFLAGS="$(LDFLAGS) --coverage"
	$(MAKE) -C fuzz check CFLAGS="$(CFLAGS) --coverage" LDFLAGS="$(LDFLAGS) --coverage"
	lcov --capture --initial --ignore-errors source,range --directory fuzz --directory src --output-file $(LCOV_INFO)
	lcov --capture --ignore-errors source,range --directory fuzz --directory src --output-file $(LCOV_INFO)
	lcov --remove $(LCOV_INFO) --ignore-errors source,range '/usr/include/*' '*/lib/*' '*/src/css.l' -o $(LCOV_INFO)
	genhtml --prefix . --ignore-errors source,range $(LCOV_INFO) --legend --title "Wget-fuzz" --output-directory=lcov
	@echo
	@echo "You can now view the coverage report with 'xdg-open lcov/index.html'"
