check_LTLIBRARIES = libtest.la

libtest_la_SOURCES = \
  test_suite.c test_suite.h \
  test_runner.c test_runner.h \
  utils/test_rng.c utils/test_rng.h

libtest_la_CFLAGS = \
  -I$(top_srcdir)/src/libstrongswan \
  @COVERAGE_CFLAGS@

libtest_la_LDFLAGS = @COVERAGE_LDFLAGS@
libtest_la_LIBADD = \
  $(top_builddir)/src/libstrongswan/libstrongswan.la \
  $(PTHREADLIB) $(ATOMICLIB)


TESTS = libstrongswan_tests

check_PROGRAMS = $(TESTS)

libstrongswan_tests_SOURCES = tests.h tests.c \
  suites/test_linked_list.c \
  suites/test_enumerator.c \
  suites/test_linked_list_enumerator.c \
  suites/test_bio_reader.c \
  suites/test_bio_writer.c \
  suites/test_chunk.c \
  suites/test_enum.c \
  suites/test_hashtable.c \
  suites/test_identification.c \
  suites/test_traffic_selector.c \
  suites/test_threading.c \
  suites/test_process.c \
  suites/test_watcher.c \
  suites/test_stream.c \
  suites/test_fetch_http.c \
  suites/test_utils.c \
  suites/test_settings.c \
  suites/test_vectors.c \
  suites/test_array.c \
  suites/test_ecdsa.c \
  suites/test_rsa.c \
  suites/test_certpolicy.c \
  suites/test_certnames.c \
  suites/test_host.c \
  suites/test_auth_cfg.c \
  suites/test_hasher.c \
  suites/test_crypter.c \
  suites/test_proposal.c \
  suites/test_crypto_factory.c \
  suites/test_iv_gen.c \
  suites/test_pen.c \
  suites/test_asn1.c \
  suites/test_asn1_parser.c \
  suites/test_printf.c \
  suites/test_test_rng.c \
  suites/test_mgf1.c \
  suites/test_ntru.c \
  suites/test_ed25519.c \
  suites/test_ed448.c \
  suites/test_signature_params.c

libstrongswan_tests_CFLAGS = \
  -I$(top_srcdir)/src/libstrongswan \
  -I$(top_srcdir)/src/libstrongswan/tests \
  -DPLUGINDIR=\""$(abs_top_builddir)/src/libstrongswan/plugins\"" \
  -DPLUGINS=\""${s_plugins}\"" \
  @COVERAGE_CFLAGS@

libstrongswan_tests_LDFLAGS = @COVERAGE_LDFLAGS@
libstrongswan_tests_LDADD = \
  $(top_builddir)/src/libstrongswan/libstrongswan.la \
  libtest.la
