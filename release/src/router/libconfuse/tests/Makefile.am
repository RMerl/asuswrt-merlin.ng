EXTRA_DIST        = annotate.conf a.conf b.conf spdir check_confuse.h

TESTS             =
TESTS            += suite_single
TESTS            += suite_dup
TESTS            += suite_func
TESTS            += suite_list
TESTS            += suite_validate
TESTS            += list_plus_syntax
TESTS            += section_title_dupes
TESTS            += single_title_sections
TESTS            += section_remove
TESTS            += section_add
TESTS            += quote_before_print
TESTS            += include
TESTS            += searchpath
TESTS            += env
TESTS            += ignore_parm
TESTS            += annotate

check_PROGRAMS    = $(TESTS)

DEFS              = -DSRC_DIR='"$(srcdir)"'
LDFLAGS           = -static
LDADD             = -L../src ../src/libconfuse.la $(LTLIBINTL)
CLEANFILES        = *~


