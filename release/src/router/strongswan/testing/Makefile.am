EXTRA_DIST = do-tests make-testing ssh start-testing stop-testing \
             testing.conf ssh_config config hosts images scripts tests \
			 README

# exclude all files ignored by Git from the tarball
dist-hook:
	if git -C $(srcdir) ls-files --error-unmatch hosts > /dev/null 2>&1; then \
		find $(abs_srcdir)/hosts $(abs_srcdir)/tests -print0 | \
			sed -z -e "s|^$(abs_srcdir)/||" | \
			git -C $(srcdir) check-ignore -z --stdin | \
			xargs -0 -I{} rm -rf $(distdir)/{}; \
	fi
