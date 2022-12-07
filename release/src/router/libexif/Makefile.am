SUBDIRS = m4m po libexif test doc binary-dist contrib

EXTRA_DIST = @PACKAGE_TARNAME@.spec README-Win32.txt

pkgconfigdir = $(libdir)/pkgconfig

pkgconfig_DATA = libexif.pc
EXTRA_DIST    += libexif.pc.in

noinst_DATA = libexif-uninstalled.pc
EXTRA_DIST += libexif-uninstalled.pc.in

ACLOCAL_AMFLAGS = -I auto-m4 -I m4m

EXTRA_DIST += SECURITY.md
doc_DATA = README AUTHORS NEWS ChangeLog ABOUT-NLS COPYING SECURITY.md

#######################################################################
# Help for the maintainer
#

# Simulate something like
#   EXTRA_DIST_IF_EXIST = ChangeLog.git
# If present, ship ChangeLog.git in source tarball.
# If not present, don't ship it.
dist-hook:
	if test -f $(srcdir)/ChangeLog.git; then \
		cp -p $(srcdir)/ChangeLog.git $(distdir)/ChangeLog.git; \
	fi

.PHONY: git-changelog
git-changelog: $(srcdir)/ChangeLog.git

.PHONY: $(srcdir)/ChangeLog.git
$(srcdir)/ChangeLog.git:
	(cd "$(srcdir)" && git2cl > $@)

.PHONY: git-tag-release
git-tag-release:
	@tag="$$(echo "$(PACKAGE_TARNAME)-$(PACKAGE_VERSION)-release" | sed 's|\.|_|g')"; \
	echo "Are you sure you want to git tag \`$${tag}' your source files?"; \
	echo "Press Ctrl-C to abort, Enter to continue."; \
	read; \
	cd "$(srcdir)" && git tag "$${tag}"

# End of Makefile.am.
