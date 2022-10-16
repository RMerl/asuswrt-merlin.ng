SUBDIRS = nls

# By default, a few definitions like bindir, srcdir are already set.
AM_TESTS_ENVIRONMENT =

# Run all tests without i18n and l10n for proper comparisons
AM_TESTS_ENVIRONMENT += LC_ALL='C'; export LC_ALL;

# Some AC_SUBST variables needed in test case scripts
AM_TESTS_ENVIRONMENT += DIFF='$(DIFF)'; export DIFF;
AM_TESTS_ENVIRONMENT += DIFF_U='$(DIFF_U)'; export DIFF_U;
AM_TESTS_ENVIRONMENT += EXEEXT='$(EXEEXT)'; export EXEEXT;
AM_TESTS_ENVIRONMENT += FAILMALLOC_PATH='$(FAILMALLOC_PATH)'; export FAILMALLOC_PATH;

# Notes about tests:
#  - Add "small" tests and stuff here.
#  - Add "big"   tests and stuff to explicitly test for (fixed) bugs
#    to libexif-testsuite:
#    * We don't want to force people to download dozens of big test images
#      just for one small library. The ones included here are carefully chosen
#      as a basic self-contained regression test set.
#    * Proper testing is done with the "exif" program in libexif-testsuite.
#      And this is just the lib - we don't have the program available
#      here yet.

TESTS = test-mem test-value test-integers test-parse test-parse-from-data test-tagtable test-sorted \
	test-fuzzer test-null parse-regression.sh swap-byte-order.sh \
	extract-parse.sh test-gps

TESTS += check-failmalloc.sh

check_PROGRAMS = test-mem test-mnote test-value test-integers test-parse test-parse-from-data \
	test-tagtable test-sorted test-fuzzer test-extract test-null test-gps

LDADD = $(top_builddir)/libexif/libexif.la $(LTLIBINTL)

EXTRA_DIST = \
	test-fuzzer-persistent.c \
	parse-regression.sh \
	swap-byte-order.sh \
	extract-parse.sh \
	check-failmalloc.sh \
	testdata/canon_makernote_variant_1.jpg \
	testdata/canon_makernote_variant_1.jpg.parsed \
	testdata/fuji_makernote_variant_1.jpg \
	testdata/fuji_makernote_variant_1.jpg.parsed \
	testdata/olympus_makernote_variant_2.jpg \
	testdata/olympus_makernote_variant_2.jpg.parsed \
	testdata/olympus_makernote_variant_3.jpg \
	testdata/olympus_makernote_variant_3.jpg.parsed \
	testdata/olympus_makernote_variant_4.jpg \
	testdata/olympus_makernote_variant_4.jpg.parsed \
	testdata/olympus_makernote_variant_5.jpg \
	testdata/olympus_makernote_variant_5.jpg.parsed \
	testdata/pentax_makernote_variant_2.jpg \
	testdata/pentax_makernote_variant_2.jpg.parsed \
	testdata/pentax_makernote_variant_3.jpg \
	testdata/pentax_makernote_variant_3.jpg.parsed \
	testdata/pentax_makernote_variant_4.jpg \
	testdata/pentax_makernote_variant_4.jpg.parsed

EXTRA_DIST += inc-comparetool.sh
