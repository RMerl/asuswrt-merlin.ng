# stddef_h.m4 serial 14
dnl Copyright (C) 2009-2024 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

dnl A placeholder for <stddef.h>, for platforms that have issues.

AC_DEFUN_ONCE([gl_STDDEF_H],
[
  AC_REQUIRE([gl_STDDEF_H_DEFAULTS])
  AC_REQUIRE([gt_TYPE_WCHAR_T])

  dnl Persuade OpenBSD <stddef.h> to declare max_align_t.
  AC_REQUIRE([gl_USE_SYSTEM_EXTENSIONS])

  GL_GENERATE_STDDEF_H=false

  dnl Test whether the type max_align_t exists and whether its alignment
  dnl "is as great as is supported by the implementation in all contexts".
  AC_CACHE_CHECK([for good max_align_t],
    [gl_cv_type_max_align_t],
    [AC_COMPILE_IFELSE(
       [AC_LANG_PROGRAM(
          [[/* On FreeBSD 12.0/x86, max_align_t defined by <stddef.h> has
               the correct alignment with the default (wrong) definition of
               _Alignof, but a wrong alignment as soon as we activate an
               ISO C compliant _Alignof definition.  */
            #if ((defined __GNUC__ && 4 <= __GNUC__) || defined __clang__) && !defined __cplusplus
             #define _Alignof(type) __builtin_offsetof (struct { char __a; type __b; }, __b)
            #endif
            #include <stddef.h>
            unsigned int s = sizeof (max_align_t);
            #if defined __GNUC__ || defined __clang__ || defined __IBM__ALIGNOF__
            int check1[2 * (__alignof__ (double) <= __alignof__ (max_align_t)) - 1];
            int check2[2 * (__alignof__ (long double) <= __alignof__ (max_align_t)) - 1];
            #endif
            typedef struct { char a; max_align_t b; } max_helper;
            typedef struct { char a; long b; } long_helper;
            typedef struct { char a; double b; } double_helper;
            typedef struct { char a; long double b; } long_double_helper;
            int check3[2 * (offsetof (long_helper, b) <= offsetof (max_helper, b)) - 1];
            int check4[2 * (offsetof (double_helper, b) <= offsetof (max_helper, b)) - 1];
            int check5[2 * (offsetof (long_double_helper, b) <= offsetof (max_helper, b)) - 1];
          ]])],
       [gl_cv_type_max_align_t=yes],
       [gl_cv_type_max_align_t=no])
    ])
  if test $gl_cv_type_max_align_t = no; then
    HAVE_MAX_ALIGN_T=0
    GL_GENERATE_STDDEF_H=true
  fi

  if test $gt_cv_c_wchar_t = no; then
    HAVE_WCHAR_T=0
    GL_GENERATE_STDDEF_H=true
  fi

  AC_CACHE_CHECK([whether NULL can be used in arbitrary expressions],
    [gl_cv_decl_null_works],
    [AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <stddef.h>
      int test[2 * (sizeof NULL == sizeof (void *)) -1];
]])],
      [gl_cv_decl_null_works=yes],
      [gl_cv_decl_null_works=no])])
  if test $gl_cv_decl_null_works = no; then
    REPLACE_NULL=1
    GL_GENERATE_STDDEF_H=true
  fi

  AC_CACHE_CHECK([for unreachable],
    [gl_cv_func_unreachable],
    [AC_LINK_IFELSE(
       [AC_LANG_PROGRAM(
          [[#include <stddef.h>
          ]],
          [[unreachable ();
          ]])],
       [gl_cv_func_unreachable=yes],
       [gl_cv_func_unreachable=no])
    ])
  if test $gl_cv_func_unreachable = no; then
    GL_GENERATE_STDDEF_H=true
  fi

  if $GL_GENERATE_STDDEF_H; then
    gl_NEXT_HEADERS([stddef.h])
  fi
])

# gl_STDDEF_MODULE_INDICATOR([modulename])
# sets the shell variable that indicates the presence of the given module
# to a C preprocessor expression that will evaluate to 1.
# This macro invocation must not occur in macros that are AC_REQUIREd.
AC_DEFUN([gl_STDDEF_MODULE_INDICATOR],
[
  dnl Ensure to expand the default settings once only.
  gl_STDDEF_H_REQUIRE_DEFAULTS
  gl_MODULE_INDICATOR_SET_VARIABLE([$1])
])

# Initializes the default values for AC_SUBSTed shell variables.
# This macro must not be AC_REQUIREd.  It must only be invoked, and only
# outside of macros or in macros that are not AC_REQUIREd.
AC_DEFUN([gl_STDDEF_H_REQUIRE_DEFAULTS],
[
  m4_defun(GL_MODULE_INDICATOR_PREFIX[_STDDEF_H_MODULE_INDICATOR_DEFAULTS], [
  ])
  m4_require(GL_MODULE_INDICATOR_PREFIX[_STDDEF_H_MODULE_INDICATOR_DEFAULTS])
  AC_REQUIRE([gl_STDDEF_H_DEFAULTS])
])

AC_DEFUN([gl_STDDEF_H_DEFAULTS],
[
  dnl Assume proper GNU behavior unless another module says otherwise.
  REPLACE_NULL=0;                AC_SUBST([REPLACE_NULL])
  HAVE_MAX_ALIGN_T=1;            AC_SUBST([HAVE_MAX_ALIGN_T])
  HAVE_WCHAR_T=1;                AC_SUBST([HAVE_WCHAR_T])
])
