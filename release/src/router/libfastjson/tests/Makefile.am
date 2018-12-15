export VALGRIND=@VALGRIND@
LDADD = $(top_builddir)/libfastjson.la \
        $(top_builddir)/libfastjson-internal.la

AM_CFLAGS = $(WARN_CFLAGS)

TESTS=
TESTS+= ucs_copyright_char.test
TESTS+= test_float.test
TESTS+= test1.test
TESTS+= test2.test
TESTS+= test4.test
TESTS+= testReplaceExisting.test
TESTS+= test_parse_int64.test
TESTS+= test_cast.test
TESTS+= test_parse.test
TESTS+= test_locale.test
TESTS+= test_charcase.test
TESTS+= test_printbuf.test
TESTS+= test_obj_iter-del.test
TESTS+= test_object_object_add_ex.test
TESTS+= test_many_subobj.test
TESTS+= test_obj_obj_get_ex-null.test
# we officially do NOT support NUL bytes (however, we may
# later add a workaround to at least transparently pass them
# through, thus I keep this as reference).
#TESTS+= test_null.test 

check_PROGRAMS=
check_PROGRAMS += $(TESTS:.test=)

# some programs that do internal checking
check_PROGRAMS += chk_version \
	cr_obj_multi

TESTS += chk_version

check_PROGRAMS += chk_version \
	cr_obj_multi

cr_obj_multi_SOURCES = cr_obj_multi.c
chk_version_SOURCES = chk_version.c

test_printbuf_SOURCES = test_printbuf.c

# Note: handled by test1.test
check_PROGRAMS += test1Formatted 
test1Formatted_SOURCES = test1.c parse_flags.c parse_flags.h
test1Formatted_CPPFLAGS = -DTEST_FORMATTED

# Note: handled by test2.test
check_PROGRAMS += test2Formatted
test2Formatted_SOURCES = test2.c parse_flags.c parse_flags.h
test2Formatted_CPPFLAGS = -DTEST_FORMATTED

# Note: handled by object_object_add_ex.test
check_PROGRAMS += test_object_object_add_exFormatted 
test_object_object_add_exFormatted_SOURCES = test_object_object_add_ex.c parse_flags.c parse_flags.h
test_object_object_add_exFormatted_CPPFLAGS = -DTEST_FORMATTED

EXTRA_DIST=
EXTRA_DIST += $(TESTS)
EXTRA_DIST += test-defs.sh
EXTRA_DIST += test1.expected
EXTRA_DIST += test1Formatted_plain.expected
EXTRA_DIST += test1Formatted_pretty.expected
EXTRA_DIST += test1Formatted_spaced.expected
EXTRA_DIST += test2.expected
EXTRA_DIST += test2Formatted_plain.expected
EXTRA_DIST += test2Formatted_pretty.expected
EXTRA_DIST += test2Formatted_spaced.expected
EXTRA_DIST += test4.expected
EXTRA_DIST += ucs_copyright_char.expected
EXTRA_DIST += test_float.expected
EXTRA_DIST += test_cast.expected
EXTRA_DIST += test_charcase.expected
EXTRA_DIST += test_locale.expected
EXTRA_DIST += test_null.expected
EXTRA_DIST += test_parse.expected
EXTRA_DIST += test_parse_int64.expected
EXTRA_DIST += test_printbuf.expected
EXTRA_DIST += testReplaceExisting.expected
EXTRA_DIST += test_obj_iter-del.expected
EXTRA_DIST += test_object_object_add_ex.expected
EXTRA_DIST += test_object_object_add_exFormatted_plain.expected
EXTRA_DIST += test_object_object_add_exFormatted_pretty.expected
EXTRA_DIST += test_object_object_add_exFormatted_spaced.expected
EXTRA_DIST += test_many_subobj.expected
EXTRA_DIST += test_obj_obj_get_ex-null.expected

testsubdir=testSubDir
TESTS_ENVIRONMENT       = top_builddir=$(top_builddir)
