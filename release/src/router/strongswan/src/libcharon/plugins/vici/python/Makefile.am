EXTRA_DIST = LICENSE README.rst MANIFEST.in \
	setup.py.in \
	tox.ini \
	tox.sh \
	test/__init__.py \
	test/test_protocol.py \
	test/test_session.py \
	vici/__init__.py \
	vici/command_wrappers.py \
	vici/exception.py \
	vici/protocol.py \
	vici/session.py

$(srcdir)/setup.py: $(srcdir)/setup.py.in $(top_builddir)/config.status
	$(AM_V_GEN) sed \
	-e "s:@EGG_VERSION@:$(PYTHON_PACKAGE_VERSION):" \
	$(srcdir)/setup.py.in > $@

all-local: dist/vici-$(PYTHON_PACKAGE_VERSION)-py$(PYTHON_VERSION).egg

dist/vici-$(PYTHON_PACKAGE_VERSION)-py$(PYTHON_VERSION).egg: $(EXTRA_DIST) $(srcdir)/setup.py
	(cd $(srcdir); $(PYTHON) setup.py bdist_egg \
		-b $(shell readlink -f $(builddir))/build \
		-d $(shell readlink -f $(builddir))/dist)

package: $(EXTRA_DIST) $(srcdir)/setup.py
	(cd $(srcdir); $(PYTHON) setup.py sdist \
		-d $(shell readlink -f $(builddir))/dist \
		bdist_wheel --universal \
		-d $(shell readlink -f $(builddir))/dist)

clean-local:
	(cd $(srcdir); [ ! -f setup.py ] || $(PYTHON) setup.py clean -a)
	rm -rf $(srcdir)/setup.py $(srcdir)/vici.egg-info $(builddir)/dist

if PYTHON_EGGS_INSTALL
install-exec-local: dist/vici-$(PYTHON_PACKAGE_VERSION)-py$(PYTHON_VERSION).egg
	$(EASY_INSTALL) $(PYTHONEGGINSTALLDIR) \
		dist/vici-$(PYTHON_PACKAGE_VERSION)-py$(PYTHON_VERSION).egg
endif

if USE_TOX
  TESTS_ENVIRONMENT += export TOX=$(TOX);
  AM_TESTS_FD_REDIRECT = 1>&2
  TESTS = tox.sh
endif

if USE_PY_TEST
  TESTS = $(PY_TEST)
endif

.PHONY: package
