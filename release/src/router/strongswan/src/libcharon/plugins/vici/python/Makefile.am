EXTRA_DIST = LICENSE README.rst MANIFEST.in \
	setup.py.in \
	tox.ini \
	tox.sh \
	test/__init__.py \
	test/test_protocol.py \
	test/test_session.py \
	vici/__init__.py \
	vici/command_wrappers.py \
	vici/event_listener.py \
	vici/exception.py \
	vici/protocol.py \
	vici/session.py

$(srcdir)/setup.py: $(srcdir)/setup.py.in $(top_builddir)/config.status
	$(AM_V_GEN) sed \
	-e "s:@EGG_VERSION@:$(PYTHON_PACKAGE_VERSION):" \
	$(srcdir)/setup.py.in > $@

all-local: dist/vici-$(PYTHON_PACKAGE_VERSION)-py3-none-any.whl

dist/vici-$(PYTHON_PACKAGE_VERSION)-py3-none-any.whl: $(EXTRA_DIST) $(srcdir)/setup.py
	(cd $(srcdir); $(PYTHON) -m build -o $(abs_builddir)/dist)

clean-local:
	rm -rf $(srcdir)/setup.py $(srcdir)/vici.egg-info $(builddir)/dist

if USE_TOX
  TESTS_ENVIRONMENT += export TOX=$(TOX);
  AM_TESTS_FD_REDIRECT = 1>&2
  TESTS = tox.sh
endif

if USE_PY_TEST
  TESTS = $(PY_TEST)
endif
