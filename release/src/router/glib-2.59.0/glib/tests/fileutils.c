/* Unit tests for gfileutils
 * Copyright (C) 1995-1997  Peter Mattis, Spencer Kimball and Josh MacDonald
 *
 * This work is provided "as is"; redistribution and modification
 * in whole or in part, in any medium, physical or electronic is
 * permitted without restriction.
 *
 * This work is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * In no event shall the authors or contributors be liable for any
 * direct, indirect, incidental, special, exemplary, or consequential
 * damages (including, but not limited to, procurement of substitute
 * goods or services; loss of use, data, or profits; or business
 * interruption) however caused and on any theory of liability, whether
 * in contract, strict liability, or tort (including negligence or
 * otherwise) arising in any way out of the use of this software, even
 * if advised of the possibility of such damage.
 */

#include "config.h"
#include <string.h>
#include <errno.h>

/* We are testing some deprecated APIs here */
#define GLIB_DISABLE_DEPRECATION_WARNINGS

#include <glib.h>

/* Test our stdio wrappers here */
#define G_STDIO_NO_WRAP_ON_UNIX
#include <glib/gstdio.h>

#ifdef G_OS_UNIX
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <utime.h>
#endif
#include <fcntl.h>
#ifdef G_OS_WIN32
#include <windows.h>
#include <sys/utime.h>
#include <io.h>
#ifndef S_ISDIR
#define S_ISDIR(m) (((m) & _S_IFMT) == _S_IFDIR)
#endif
#ifndef F_OK
#define F_OK 0
#endif
#endif

#define S G_DIR_SEPARATOR_S

static void
check_string (gchar *str, const gchar *expected)
{
  g_assert_nonnull (str);
  g_assert_cmpstr (str, ==, expected);
  g_free (str);
}

static void
test_build_path (void)
{
/*  check_string (g_build_path ("", NULL), "");*/
  check_string (g_build_path ("", "", NULL), "");
  check_string (g_build_path ("", "x", NULL), "x");
  check_string (g_build_path ("", "x", "y",  NULL), "xy");
  check_string (g_build_path ("", "x", "y", "z", NULL), "xyz");

/*  check_string (g_build_path (":", NULL), "");*/
  check_string (g_build_path (":", ":", NULL), ":");
  check_string (g_build_path (":", ":x", NULL), ":x");
  check_string (g_build_path (":", "x:", NULL), "x:");
  check_string (g_build_path (":", "", "x", NULL), "x");
  check_string (g_build_path (":", "", ":x", NULL), ":x");
  check_string (g_build_path (":", ":", "x", NULL), ":x");
  check_string (g_build_path (":", "::", "x", NULL), "::x");
  check_string (g_build_path (":", "x", "", NULL), "x");
  check_string (g_build_path (":", "x:", "", NULL), "x:");
  check_string (g_build_path (":", "x", ":", NULL), "x:");
  check_string (g_build_path (":", "x", "::", NULL), "x::");
  check_string (g_build_path (":", "x", "y",  NULL), "x:y");
  check_string (g_build_path (":", ":x", "y", NULL), ":x:y");
  check_string (g_build_path (":", "x", "y:", NULL), "x:y:");
  check_string (g_build_path (":", ":x:", ":y:", NULL), ":x:y:");
  check_string (g_build_path (":", ":x::", "::y:", NULL), ":x:y:");
  check_string (g_build_path (":", "x", "","y",  NULL), "x:y");
  check_string (g_build_path (":", "x", ":", "y",  NULL), "x:y");
  check_string (g_build_path (":", "x", "::", "y",  NULL), "x:y");
  check_string (g_build_path (":", "x", "y", "z", NULL), "x:y:z");
  check_string (g_build_path (":", ":x:", ":y:", ":z:", NULL), ":x:y:z:");
  check_string (g_build_path (":", "::x::", "::y::", "::z::", NULL), "::x:y:z::");

/*  check_string (g_build_path ("::", NULL), "");*/
  check_string (g_build_path ("::", "::", NULL), "::");
  check_string (g_build_path ("::", ":::", NULL), ":::");
  check_string (g_build_path ("::", "::x", NULL), "::x");
  check_string (g_build_path ("::", "x::", NULL), "x::");
  check_string (g_build_path ("::", "", "x", NULL), "x");
  check_string (g_build_path ("::", "", "::x", NULL), "::x");
  check_string (g_build_path ("::", "::", "x", NULL), "::x");
  check_string (g_build_path ("::", "::::", "x", NULL), "::::x");
  check_string (g_build_path ("::", "x", "", NULL), "x");
  check_string (g_build_path ("::", "x::", "", NULL), "x::");
  check_string (g_build_path ("::", "x", "::", NULL), "x::");

  /* This following is weird, but keeps the definition simple */
  check_string (g_build_path ("::", "x", ":::", NULL), "x:::::");
  check_string (g_build_path ("::", "x", "::::", NULL), "x::::");
  check_string (g_build_path ("::", "x", "y",  NULL), "x::y");
  check_string (g_build_path ("::", "::x", "y", NULL), "::x::y");
  check_string (g_build_path ("::", "x", "y::", NULL), "x::y::");
  check_string (g_build_path ("::", "::x::", "::y::", NULL), "::x::y::");
  check_string (g_build_path ("::", "::x:::", ":::y::", NULL), "::x::::y::");
  check_string (g_build_path ("::", "::x::::", "::::y::", NULL), "::x::y::");
  check_string (g_build_path ("::", "x", "", "y",  NULL), "x::y");
  check_string (g_build_path ("::", "x", "::", "y",  NULL), "x::y");
  check_string (g_build_path ("::", "x", "::::", "y",  NULL), "x::y");
  check_string (g_build_path ("::", "x", "y", "z", NULL), "x::y::z");
  check_string (g_build_path ("::", "::x::", "::y::", "::z::", NULL), "::x::y::z::");
  check_string (g_build_path ("::", ":::x:::", ":::y:::", ":::z:::", NULL), ":::x::::y::::z:::");
  check_string (g_build_path ("::", "::::x::::", "::::y::::", "::::z::::", NULL), "::::x::y::z::::");
}

static void
test_build_pathv (void)
{
  gchar *args[10];

  g_assert_null (g_build_pathv ("", NULL));
  args[0] = NULL;
  check_string (g_build_pathv ("", args), "");
  args[0] = ""; args[1] = NULL;
  check_string (g_build_pathv ("", args), "");
  args[0] = "x"; args[1] = NULL;
  check_string (g_build_pathv ("", args), "x");
  args[0] = "x"; args[1] = "y"; args[2] = NULL;
  check_string (g_build_pathv ("", args), "xy");
  args[0] = "x"; args[1] = "y"; args[2] = "z", args[3] = NULL;
  check_string (g_build_pathv ("", args), "xyz");

  args[0] = NULL;
  check_string (g_build_pathv (":", args), "");
  args[0] = ":"; args[1] = NULL;
  check_string (g_build_pathv (":", args), ":");
  args[0] = ":x"; args[1] = NULL;
  check_string (g_build_pathv (":", args), ":x");
  args[0] = "x:"; args[1] = NULL;
  check_string (g_build_pathv (":", args), "x:");
  args[0] = ""; args[1] = "x"; args[2] = NULL;
  check_string (g_build_pathv (":", args), "x");
  args[0] = ""; args[1] = ":x"; args[2] = NULL;
  check_string (g_build_pathv (":", args), ":x");
  args[0] = ":"; args[1] = "x"; args[2] = NULL;
  check_string (g_build_pathv (":", args), ":x");
  args[0] = "::"; args[1] = "x"; args[2] = NULL;
  check_string (g_build_pathv (":", args), "::x");
  args[0] = "x"; args[1] = ""; args[2] = NULL;
  check_string (g_build_pathv (":", args), "x");
  args[0] = "x:"; args[1] = ""; args[2] = NULL;
  check_string (g_build_pathv (":", args), "x:");
  args[0] = "x"; args[1] = ":"; args[2] = NULL;
  check_string (g_build_pathv (":", args), "x:");
  args[0] = "x"; args[1] = "::"; args[2] = NULL;
  check_string (g_build_pathv (":", args), "x::");
  args[0] = "x"; args[1] = "y"; args[2] = NULL;
  check_string (g_build_pathv (":", args), "x:y");
  args[0] = ":x"; args[1] = "y"; args[2] = NULL;
  check_string (g_build_pathv (":", args), ":x:y");
  args[0] = "x"; args[1] = "y:"; args[2] = NULL;
  check_string (g_build_pathv (":", args), "x:y:");
  args[0] = ":x:"; args[1] = ":y:"; args[2] = NULL;
  check_string (g_build_pathv (":", args), ":x:y:");
  args[0] = ":x::"; args[1] = "::y:"; args[2] = NULL;
  check_string (g_build_pathv (":", args), ":x:y:");
  args[0] = "x"; args[1] = ""; args[2] = "y"; args[3] = NULL;
  check_string (g_build_pathv (":", args), "x:y");
  args[0] = "x"; args[1] = ":"; args[2] = "y"; args[3] = NULL;
  check_string (g_build_pathv (":", args), "x:y");
  args[0] = "x"; args[1] = "::"; args[2] = "y"; args[3] = NULL;
  check_string (g_build_pathv (":", args), "x:y");
  args[0] = "x"; args[1] = "y"; args[2] = "z"; args[3] = NULL;
  check_string (g_build_pathv (":", args), "x:y:z");
  args[0] = ":x:"; args[1] = ":y:"; args[2] = ":z:"; args[3] = NULL;
  check_string (g_build_pathv (":", args), ":x:y:z:");
  args[0] = "::x::"; args[1] = "::y::"; args[2] = "::z::"; args[3] = NULL;
  check_string (g_build_pathv (":", args), "::x:y:z::");

  args[0] = NULL;
  check_string (g_build_pathv ("::", args), "");
  args[0] = "::"; args[1] = NULL;
  check_string (g_build_pathv ("::", args), "::");
  args[0] = ":::"; args[1] = NULL;
  check_string (g_build_pathv ("::", args), ":::");
  args[0] = "::x"; args[1] = NULL;
  check_string (g_build_pathv ("::", args), "::x");
  args[0] = "x::"; args[1] = NULL;
  check_string (g_build_pathv ("::", args), "x::");
  args[0] = ""; args[1] = "x"; args[2] = NULL;
  check_string (g_build_pathv ("::", args), "x");
  args[0] = ""; args[1] = "::x"; args[2] = NULL;
  check_string (g_build_pathv ("::", args), "::x");
  args[0] = "::"; args[1] = "x"; args[2] = NULL;
  check_string (g_build_pathv ("::", args), "::x");
  args[0] = "::::"; args[1] = "x"; args[2] = NULL;
  check_string (g_build_pathv ("::", args), "::::x");
  args[0] = "x"; args[1] = ""; args[2] = NULL;
  check_string (g_build_pathv ("::", args), "x");
  args[0] = "x::"; args[1] = ""; args[2] = NULL;
  check_string (g_build_pathv ("::", args), "x::");
  args[0] = "x"; args[1] = "::"; args[2] = NULL;
  check_string (g_build_pathv ("::", args), "x::");
  /* This following is weird, but keeps the definition simple */
  args[0] = "x"; args[1] = ":::"; args[2] = NULL;
  check_string (g_build_pathv ("::", args), "x:::::");
  args[0] = "x"; args[1] = "::::"; args[2] = NULL;
  check_string (g_build_pathv ("::", args), "x::::");
  args[0] = "x"; args[1] = "y"; args[2] = NULL;
  check_string (g_build_pathv ("::", args), "x::y");
  args[0] = "::x"; args[1] = "y"; args[2] = NULL;
  check_string (g_build_pathv ("::", args), "::x::y");
  args[0] = "x"; args[1] = "y::"; args[2] = NULL;
  check_string (g_build_pathv ("::", args), "x::y::");
  args[0] = "::x::"; args[1] = "::y::"; args[2] = NULL;
  check_string (g_build_pathv ("::", args), "::x::y::");
  args[0] = "::x:::"; args[1] = ":::y::"; args[2] = NULL;
  check_string (g_build_pathv ("::", args), "::x::::y::");
  args[0] = "::x::::"; args[1] = "::::y::"; args[2] = NULL;
  check_string (g_build_pathv ("::", args), "::x::y::");
  args[0] = "x"; args[1] = ""; args[2] = "y"; args[3] = NULL;
  check_string (g_build_pathv ("::", args), "x::y");
  args[0] = "x"; args[1] = "::"; args[2] = "y"; args[3] = NULL;
  check_string (g_build_pathv ("::", args), "x::y");
  args[0] = "x"; args[1] = "::::"; args[2] = "y"; args[3] = NULL;
  check_string (g_build_pathv ("::", args), "x::y");
  args[0] = "x"; args[1] = "y"; args[2] = "z"; args[3] = NULL;
  check_string (g_build_pathv ("::", args), "x::y::z");
  args[0] = "::x::"; args[1] = "::y::"; args[2] = "::z::"; args[3] = NULL;
  check_string (g_build_pathv ("::", args), "::x::y::z::");
  args[0] = ":::x:::"; args[1] = ":::y:::"; args[2] = ":::z:::"; args[3] = NULL;
  check_string (g_build_pathv ("::", args), ":::x::::y::::z:::");
  args[0] = "::::x::::"; args[1] = "::::y::::"; args[2] = "::::z::::"; args[3] = NULL;
  check_string (g_build_pathv ("::", args), "::::x::y::z::::");
}

static void
test_build_filename (void)
{
/*  check_string (g_build_filename (NULL), "");*/
  check_string (g_build_filename (S, NULL), S);
  check_string (g_build_filename (S"x", NULL), S"x");
  check_string (g_build_filename ("x"S, NULL), "x"S);
  check_string (g_build_filename ("", "x", NULL), "x");
  check_string (g_build_filename ("", S"x", NULL), S"x");
  check_string (g_build_filename (S, "x", NULL), S"x");
  check_string (g_build_filename (S S, "x", NULL), S S"x");
  check_string (g_build_filename ("x", "", NULL), "x");
  check_string (g_build_filename ("x"S, "", NULL), "x"S);
  check_string (g_build_filename ("x", S, NULL), "x"S);
  check_string (g_build_filename ("x", S S, NULL), "x"S S);
  check_string (g_build_filename ("x", "y",  NULL), "x"S"y");
  check_string (g_build_filename (S"x", "y", NULL), S"x"S"y");
  check_string (g_build_filename ("x", "y"S, NULL), "x"S"y"S);
  check_string (g_build_filename (S"x"S, S"y"S, NULL), S"x"S"y"S);
  check_string (g_build_filename (S"x"S S, S S"y"S, NULL), S"x"S"y"S);
  check_string (g_build_filename ("x", "", "y",  NULL), "x"S"y");
  check_string (g_build_filename ("x", S, "y",  NULL), "x"S"y");
  check_string (g_build_filename ("x", S S, "y",  NULL), "x"S"y");
  check_string (g_build_filename ("x", "y", "z", NULL), "x"S"y"S"z");
  check_string (g_build_filename (S"x"S, S"y"S, S"z"S, NULL), S"x"S"y"S"z"S);
  check_string (g_build_filename (S S"x"S S, S S"y"S S, S S"z"S S, NULL), S S"x"S"y"S"z"S S);

#ifdef G_OS_WIN32

  /* Test also using the slash as file name separator */
#define Z "/"
  /* check_string (g_build_filename (NULL), ""); */
  check_string (g_build_filename (Z, NULL), Z);
  check_string (g_build_filename (Z"x", NULL), Z"x");
  check_string (g_build_filename ("x"Z, NULL), "x"Z);
  check_string (g_build_filename ("", Z"x", NULL), Z"x");
  check_string (g_build_filename ("", Z"x", NULL), Z"x");
  check_string (g_build_filename (Z, "x", NULL), Z"x");
  check_string (g_build_filename (Z Z, "x", NULL), Z Z"x");
  check_string (g_build_filename (Z S, "x", NULL), Z S"x");
  check_string (g_build_filename ("x"Z, "", NULL), "x"Z);
  check_string (g_build_filename ("x"S"y", "z"Z"a", NULL), "x"S"y"S"z"Z"a");
  check_string (g_build_filename ("x", Z, NULL), "x"Z);
  check_string (g_build_filename ("x", Z Z, NULL), "x"Z Z);
  check_string (g_build_filename ("x", S Z, NULL), "x"S Z);
  check_string (g_build_filename (Z"x", "y", NULL), Z"x"Z"y");
  check_string (g_build_filename ("x", "y"Z, NULL), "x"Z"y"Z);
  check_string (g_build_filename (Z"x"Z, Z"y"Z, NULL), Z"x"Z"y"Z);
  check_string (g_build_filename (Z"x"Z Z, Z Z"y"Z, NULL), Z"x"Z"y"Z);
  check_string (g_build_filename ("x", Z, "y",  NULL), "x"Z"y");
  check_string (g_build_filename ("x", Z Z, "y",  NULL), "x"Z"y");
  check_string (g_build_filename ("x", Z S, "y",  NULL), "x"S"y");
  check_string (g_build_filename ("x", S Z, "y",  NULL), "x"Z"y");
  check_string (g_build_filename ("x", Z "y", "z", NULL), "x"Z"y"Z"z");
  check_string (g_build_filename ("x", S "y", "z", NULL), "x"S"y"S"z");
  check_string (g_build_filename ("x", S "y", "z", Z, "a", "b", NULL), "x"S"y"S"z"Z"a"Z"b");
  check_string (g_build_filename (Z"x"Z, Z"y"Z, Z"z"Z, NULL), Z"x"Z"y"Z"z"Z);
  check_string (g_build_filename (Z Z"x"Z Z, Z Z"y"Z Z, Z Z"z"Z Z, NULL), Z Z"x"Z"y"Z"z"Z Z);

#undef Z

#endif /* G_OS_WIN32 */

}

static void
test_build_filenamev (void)
{
  gchar *args[10];

  args[0] = NULL;
  check_string (g_build_filenamev (args), "");
  args[0] = S; args[1] = NULL;
  check_string (g_build_filenamev (args), S);
  args[0] = S"x"; args[1] = NULL;
  check_string (g_build_filenamev (args), S"x");
  args[0] = "x"S; args[1] = NULL;
  check_string (g_build_filenamev (args), "x"S);
  args[0] = ""; args[1] = "x"; args[2] = NULL;
  check_string (g_build_filenamev (args), "x");
  args[0] = ""; args[1] = S"x"; args[2] = NULL;
  check_string (g_build_filenamev (args), S"x");
  args[0] = S; args[1] = "x"; args[2] = NULL;
  check_string (g_build_filenamev (args), S"x");
  args[0] = S S; args[1] = "x"; args[2] = NULL;
  check_string (g_build_filenamev (args), S S"x");
  args[0] = "x"; args[1] = ""; args[2] = NULL;
  check_string (g_build_filenamev (args), "x");
  args[0] = "x"S; args[1] = ""; args[2] = NULL;
  check_string (g_build_filenamev (args), "x"S);
  args[0] = "x"; args[1] = S; args[2] = NULL;
  check_string (g_build_filenamev (args), "x"S);
  args[0] = "x"; args[1] = S S; args[2] = NULL;
  check_string (g_build_filenamev (args), "x"S S);
  args[0] = "x"; args[1] = "y"; args[2] = NULL;
  check_string (g_build_filenamev (args), "x"S"y");
  args[0] = S"x"; args[1] = "y"; args[2] = NULL;
  check_string (g_build_filenamev (args), S"x"S"y");
  args[0] = "x"; args[1] = "y"S; args[2] = NULL;
  check_string (g_build_filenamev (args), "x"S"y"S);
  args[0] = S"x"S; args[1] = S"y"S; args[2] = NULL;
  check_string (g_build_filenamev (args), S"x"S"y"S);
  args[0] = S"x"S S; args[1] = S S"y"S; args[2] = NULL;
  check_string (g_build_filenamev (args), S"x"S"y"S);
  args[0] = "x"; args[1] = ""; args[2] = "y"; args[3] = NULL;
  check_string (g_build_filenamev (args), "x"S"y");
  args[0] = "x"; args[1] = S; args[2] = "y"; args[3] = NULL;
  check_string (g_build_filenamev (args), "x"S"y");
  args[0] = "x"; args[1] = S S; args[2] = "y"; args[3] = NULL;
  check_string (g_build_filenamev (args), "x"S"y");
  args[0] = "x"; args[1] = "y"; args[2] = "z"; args[3] = NULL;
  check_string (g_build_filenamev (args), "x"S"y"S"z");
  args[0] = S"x"S; args[1] = S"y"S; args[2] = S"z"S; args[3] = NULL;
  check_string (g_build_filenamev (args), S"x"S"y"S"z"S);
  args[0] = S S"x"S S; args[1] = S S"y"S S; args[2] = S S"z"S S; args[3] = NULL;
  check_string (g_build_filenamev (args), S S"x"S"y"S"z"S S);

#ifdef G_OS_WIN32

  /* Test also using the slash as file name separator */
#define Z "/"
  args[0] = NULL;
  check_string (g_build_filenamev (args), "");
  args[0] = Z; args[1] = NULL;
  check_string (g_build_filenamev (args), Z);
  args[0] = Z"x"; args[1] = NULL;
  check_string (g_build_filenamev (args), Z"x");
  args[0] = "x"Z; args[1] = NULL;
  check_string (g_build_filenamev (args), "x"Z);
  args[0] = ""; args[1] = Z"x"; args[2] = NULL;
  check_string (g_build_filenamev (args), Z"x");
  args[0] = ""; args[1] = Z"x"; args[2] = NULL;
  check_string (g_build_filenamev (args), Z"x");
  args[0] = Z; args[1] = "x"; args[2] = NULL;
  check_string (g_build_filenamev (args), Z"x");
  args[0] = Z Z; args[1] = "x"; args[2] = NULL;
  check_string (g_build_filenamev (args), Z Z"x");
  args[0] = Z S; args[1] = "x"; args[2] = NULL;
  check_string (g_build_filenamev (args), Z S"x");
  args[0] = "x"Z; args[1] = ""; args[2] = NULL;
  check_string (g_build_filenamev (args), "x"Z);
  args[0] = "x"S"y"; args[1] = "z"Z"a"; args[2] = NULL;
  check_string (g_build_filenamev (args), "x"S"y"S"z"Z"a");
  args[0] = "x"; args[1] = Z; args[2] = NULL;
  check_string (g_build_filenamev (args), "x"Z);
  args[0] = "x"; args[1] = Z Z; args[2] = NULL;
  check_string (g_build_filenamev (args), "x"Z Z);
  args[0] = "x"; args[1] = S Z; args[2] = NULL;
  check_string (g_build_filenamev (args), "x"S Z);
  args[0] = Z"x"; args[1] = "y"; args[2] = NULL;
  check_string (g_build_filenamev (args), Z"x"Z"y");
  args[0] = "x"; args[1] = "y"Z; args[2] = NULL;
  check_string (g_build_filenamev (args), "x"Z"y"Z);
  args[0] = Z"x"Z; args[1] = Z"y"Z; args[2] = NULL;
  check_string (g_build_filenamev (args), Z"x"Z"y"Z);
  args[0] = Z"x"Z Z; args[1] = Z Z"y"Z; args[2] = NULL;
  check_string (g_build_filenamev (args), Z"x"Z"y"Z);
  args[0] = "x"; args[1] = Z; args[2] = "y", args[3] = NULL;
  check_string (g_build_filenamev (args), "x"Z"y");
  args[0] = "x"; args[1] = Z Z; args[2] = "y", args[3] = NULL;
  check_string (g_build_filenamev (args), "x"Z"y");
  args[0] = "x"; args[1] = Z S; args[2] = "y", args[3] = NULL;
  check_string (g_build_filenamev (args), "x"S"y");
  args[0] = "x"; args[1] = S Z; args[2] = "y", args[3] = NULL;
  check_string (g_build_filenamev (args), "x"Z"y");
  args[0] = "x"; args[1] = Z "y"; args[2] = "z", args[3] = NULL;
  check_string (g_build_filenamev (args), "x"Z"y"Z"z");
  args[0] = "x"; args[1] = S "y"; args[2] = "z", args[3] = NULL;
  check_string (g_build_filenamev (args), "x"S"y"S"z");
  args[0] = "x"; args[1] = S "y"; args[2] = "z", args[3] = Z;
  args[4] = "a"; args[5] = "b"; args[6] = NULL;
  check_string (g_build_filenamev (args), "x"S"y"S"z"Z"a"Z"b");
  args[0] = Z"x"Z; args[1] = Z"y"Z; args[2] = Z"z"Z, args[3] = NULL;
  check_string (g_build_filenamev (args), Z"x"Z"y"Z"z"Z);
  args[0] = Z Z"x"Z Z; args[1] = Z Z"y"Z Z; args[2] = Z Z"z"Z Z, args[3] = NULL;
  check_string (g_build_filenamev (args), Z Z"x"Z"y"Z"z"Z Z);

#undef Z

#endif /* G_OS_WIN32 */
}

#undef S

static void
test_mkdir_with_parents_1 (const gchar *base)
{
  char *p0 = g_build_filename (base, "fum", NULL);
  char *p1 = g_build_filename (p0, "tem", NULL);
  char *p2 = g_build_filename (p1, "zap", NULL);
  FILE *f;

  g_remove (p2);
  g_remove (p1);
  g_remove (p0);

  if (g_file_test (p0, G_FILE_TEST_EXISTS))
    g_error ("failed, %s exists, cannot test g_mkdir_with_parents", p0);

  if (g_file_test (p1, G_FILE_TEST_EXISTS))
    g_error ("failed, %s exists, cannot test g_mkdir_with_parents", p1);

  if (g_file_test (p2, G_FILE_TEST_EXISTS))
    g_error ("failed, %s exists, cannot test g_mkdir_with_parents", p2);

  if (g_mkdir_with_parents (p2, 0777) == -1)
    {
      int errsv = errno;
      g_error ("failed, g_mkdir_with_parents(%s) failed: %s", p2, g_strerror (errsv));
    }

  if (!g_file_test (p2, G_FILE_TEST_IS_DIR))
    g_error ("failed, g_mkdir_with_parents(%s) succeeded, but %s is not a directory", p2, p2);

  if (!g_file_test (p1, G_FILE_TEST_IS_DIR))
    g_error ("failed, g_mkdir_with_parents(%s) succeeded, but %s is not a directory", p2, p1);

  if (!g_file_test (p0, G_FILE_TEST_IS_DIR))
    g_error ("failed, g_mkdir_with_parents(%s) succeeded, but %s is not a directory", p2, p0);

  g_rmdir (p2);
  if (g_file_test (p2, G_FILE_TEST_EXISTS))
    g_error ("failed, did g_rmdir(%s), but %s is still there", p2, p2);

  g_rmdir (p1);
  if (g_file_test (p1, G_FILE_TEST_EXISTS))
    g_error ("failed, did g_rmdir(%s), but %s is still there", p1, p1);

  f = g_fopen (p1, "w");
  if (f == NULL)
    g_error ("failed, couldn't create file %s", p1);
  fclose (f);

  if (g_mkdir_with_parents (p1, 0666) == 0)
    g_error ("failed, g_mkdir_with_parents(%s) succeeded, even if %s is a file", p1, p1);

  if (g_mkdir_with_parents (p2, 0666) == 0)
    g_error("failed, g_mkdir_with_parents(%s) succeeded, even if %s is a file", p2, p1);

  g_remove (p2);
  g_remove (p1);
  g_remove (p0);

  g_free (p2);
  g_free (p1);
  g_free (p0);
}

static void
test_mkdir_with_parents (void)
{
  gchar *cwd;
  if (g_test_verbose())
    g_printerr ("checking g_mkdir_with_parents() in subdir ./hum/");
  test_mkdir_with_parents_1 ("hum");
  g_remove ("hum");
  if (g_test_verbose())
    g_printerr ("checking g_mkdir_with_parents() in subdir ./hii///haa/hee/");
  test_mkdir_with_parents_1 ("hii///haa/hee");
  g_remove ("hii/haa/hee");
  g_remove ("hii/haa");
  g_remove ("hii");
  cwd = g_get_current_dir ();
  if (g_test_verbose())
    g_printerr ("checking g_mkdir_with_parents() in cwd: %s", cwd);
  test_mkdir_with_parents_1 (cwd);
  g_free (cwd);

  g_assert_cmpint (g_mkdir_with_parents (NULL, 0), ==, -1);
  g_assert_cmpint (errno, ==, EINVAL);
}

static void
test_format_size_for_display (void)
{
#ifdef G_OS_WIN32
  SetThreadLocale (MAKELCID (MAKELANGID (LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT));
#endif
  /* nobody called setlocale(), so we should get "C" behaviour... */
  check_string (g_format_size_for_display (0), "0 bytes");
  check_string (g_format_size_for_display (1), "1 byte");
  check_string (g_format_size_for_display (2), "2 bytes");
  check_string (g_format_size_for_display (1024), "1.0 KB");
  check_string (g_format_size_for_display (1024 * 1024), "1.0 MB");
  check_string (g_format_size_for_display (1024 * 1024 * 1024), "1.0 GB");
  check_string (g_format_size_for_display (1024ULL * 1024 * 1024 * 1024), "1.0 TB");
  check_string (g_format_size_for_display (1024ULL * 1024 * 1024 * 1024 * 1024), "1.0 PB");
  check_string (g_format_size_for_display (1024ULL * 1024 * 1024 * 1024 * 1024 * 1024), "1.0 EB");

  check_string (g_format_size (0), "0 bytes");
  check_string (g_format_size (1), "1 byte");
  check_string (g_format_size (2), "2 bytes");
  check_string (g_format_size (1000ULL), "1.0 kB");
  check_string (g_format_size (1000ULL * 1000), "1.0 MB");
  check_string (g_format_size (1000ULL * 1000 * 1000), "1.0 GB");
  check_string (g_format_size (1000ULL * 1000 * 1000 * 1000), "1.0 TB");
  check_string (g_format_size (1000ULL * 1000 * 1000 * 1000 * 1000), "1.0 PB");
  check_string (g_format_size (1000ULL * 1000 * 1000 * 1000 * 1000 * 1000), "1.0 EB");

  check_string (g_format_size_full (0, G_FORMAT_SIZE_IEC_UNITS), "0 bytes");
  check_string (g_format_size_full (1, G_FORMAT_SIZE_IEC_UNITS), "1 byte");
  check_string (g_format_size_full (2, G_FORMAT_SIZE_IEC_UNITS), "2 bytes");

  check_string (g_format_size_full (2048ULL, G_FORMAT_SIZE_IEC_UNITS), "2.0 KiB");
  check_string (g_format_size_full (2048ULL * 1024, G_FORMAT_SIZE_IEC_UNITS), "2.0 MiB");
  check_string (g_format_size_full (2048ULL * 1024 * 1024, G_FORMAT_SIZE_IEC_UNITS), "2.0 GiB");
  check_string (g_format_size_full (2048ULL * 1024 * 1024 * 1024, G_FORMAT_SIZE_IEC_UNITS), "2.0 TiB");
  check_string (g_format_size_full (2048ULL * 1024 * 1024 * 1024 * 1024, G_FORMAT_SIZE_IEC_UNITS), "2.0 PiB");
  check_string (g_format_size_full (2048ULL * 1024 * 1024 * 1024 * 1024 * 1024, G_FORMAT_SIZE_IEC_UNITS), "2.0 EiB");

  check_string (g_format_size_full (238472938, G_FORMAT_SIZE_IEC_UNITS), "227.4 MiB");
  check_string (g_format_size_full (238472938, G_FORMAT_SIZE_DEFAULT), "238.5 MB");
  check_string (g_format_size_full (238472938, G_FORMAT_SIZE_LONG_FORMAT), "238.5 MB (238472938 bytes)");


  check_string (g_format_size_full (0, G_FORMAT_SIZE_BITS), "0 bits");
  check_string (g_format_size_full (1, G_FORMAT_SIZE_BITS), "1 bit");
  check_string (g_format_size_full (2, G_FORMAT_SIZE_BITS), "2 bits");

  check_string (g_format_size_full (2000ULL, G_FORMAT_SIZE_BITS), "2.0 kb");
  check_string (g_format_size_full (2000ULL * 1000, G_FORMAT_SIZE_BITS), "2.0 Mb");
  check_string (g_format_size_full (2000ULL * 1000 * 1000, G_FORMAT_SIZE_BITS), "2.0 Gb");
  check_string (g_format_size_full (2000ULL * 1000 * 1000 * 1000, G_FORMAT_SIZE_BITS), "2.0 Tb");
  check_string (g_format_size_full (2000ULL * 1000 * 1000 * 1000 * 1000, G_FORMAT_SIZE_BITS), "2.0 Pb");
  check_string (g_format_size_full (2000ULL * 1000 * 1000 * 1000 * 1000 * 1000, G_FORMAT_SIZE_BITS), "2.0 Eb");

  check_string (g_format_size_full (238472938, G_FORMAT_SIZE_BITS), "238.5 Mb");
  check_string (g_format_size_full (238472938, G_FORMAT_SIZE_BITS | G_FORMAT_SIZE_LONG_FORMAT), "238.5 Mb (238472938 bits)");


  check_string (g_format_size_full (0, G_FORMAT_SIZE_BITS | G_FORMAT_SIZE_IEC_UNITS), "0 bits");
  check_string (g_format_size_full (1, G_FORMAT_SIZE_BITS | G_FORMAT_SIZE_IEC_UNITS), "1 bit");
  check_string (g_format_size_full (2, G_FORMAT_SIZE_BITS | G_FORMAT_SIZE_IEC_UNITS), "2 bits");

  check_string (g_format_size_full (2048ULL, G_FORMAT_SIZE_BITS | G_FORMAT_SIZE_IEC_UNITS), "2.0 Kib");
  check_string (g_format_size_full (2048ULL * 1024, G_FORMAT_SIZE_BITS | G_FORMAT_SIZE_IEC_UNITS), "2.0 Mib");
  check_string (g_format_size_full (2048ULL * 1024 * 1024, G_FORMAT_SIZE_BITS | G_FORMAT_SIZE_IEC_UNITS), "2.0 Gib");
  check_string (g_format_size_full (2048ULL * 1024 * 1024 * 1024, G_FORMAT_SIZE_BITS | G_FORMAT_SIZE_IEC_UNITS), "2.0 Tib");
  check_string (g_format_size_full (2048ULL * 1024 * 1024 * 1024 * 1024, G_FORMAT_SIZE_BITS | G_FORMAT_SIZE_IEC_UNITS), "2.0 Pib");
  check_string (g_format_size_full (2048ULL * 1024 * 1024 * 1024 * 1024 * 1024, G_FORMAT_SIZE_BITS | G_FORMAT_SIZE_IEC_UNITS), "2.0 Eib");

  check_string (g_format_size_full (238472938, G_FORMAT_SIZE_BITS | G_FORMAT_SIZE_IEC_UNITS), "227.4 Mib");
  check_string (g_format_size_full (238472938, G_FORMAT_SIZE_BITS | G_FORMAT_SIZE_IEC_UNITS | G_FORMAT_SIZE_LONG_FORMAT), "227.4 Mib (238472938 bits)");
}

static void
test_file_errors (void)
{
#ifdef EEXIST
  g_assert_cmpint (g_file_error_from_errno (EEXIST), ==, G_FILE_ERROR_EXIST);
#endif
#ifdef EISDIR
  g_assert_cmpint (g_file_error_from_errno (EISDIR), ==, G_FILE_ERROR_ISDIR);
#endif
#ifdef EACCES
  g_assert_cmpint (g_file_error_from_errno (EACCES), ==, G_FILE_ERROR_ACCES);
#endif
#ifdef ENAMETOOLONG
  g_assert_cmpint (g_file_error_from_errno (ENAMETOOLONG), ==, G_FILE_ERROR_NAMETOOLONG);
#endif
#ifdef ENOENT
  g_assert_cmpint (g_file_error_from_errno (ENOENT), ==, G_FILE_ERROR_NOENT);
#endif
#ifdef ENOTDIR
  g_assert_cmpint (g_file_error_from_errno (ENOTDIR), ==, G_FILE_ERROR_NOTDIR);
#endif
#ifdef ENXIO
  g_assert_cmpint (g_file_error_from_errno (ENXIO), ==, G_FILE_ERROR_NXIO);
#endif
#ifdef ENODEV
  g_assert_cmpint (g_file_error_from_errno (ENODEV), ==, G_FILE_ERROR_NODEV);
#endif
#ifdef EROFS
  g_assert_cmpint (g_file_error_from_errno (EROFS), ==, G_FILE_ERROR_ROFS);
#endif
#ifdef ETXTBSY
  g_assert_cmpint (g_file_error_from_errno (ETXTBSY), ==, G_FILE_ERROR_TXTBSY);
#endif
#ifdef EFAULT
  g_assert_cmpint (g_file_error_from_errno (EFAULT), ==, G_FILE_ERROR_FAULT);
#endif
#ifdef ELOOP
  g_assert_cmpint (g_file_error_from_errno (ELOOP), ==, G_FILE_ERROR_LOOP);
#endif
#ifdef ENOSPC
  g_assert_cmpint (g_file_error_from_errno (ENOSPC), ==, G_FILE_ERROR_NOSPC);
#endif
#ifdef ENOMEM
  g_assert_cmpint (g_file_error_from_errno (ENOMEM), ==, G_FILE_ERROR_NOMEM);
#endif
#ifdef EMFILE
  g_assert_cmpint (g_file_error_from_errno (EMFILE), ==, G_FILE_ERROR_MFILE);
#endif
#ifdef ENFILE
  g_assert_cmpint (g_file_error_from_errno (ENFILE), ==, G_FILE_ERROR_NFILE);
#endif
#ifdef EBADF
  g_assert_cmpint (g_file_error_from_errno (EBADF), ==, G_FILE_ERROR_BADF);
#endif
#ifdef EINVAL
  g_assert_cmpint (g_file_error_from_errno (EINVAL), ==, G_FILE_ERROR_INVAL);
#endif
#ifdef EPIPE
  g_assert_cmpint (g_file_error_from_errno (EPIPE), ==, G_FILE_ERROR_PIPE);
#endif
#ifdef EAGAIN
  g_assert_cmpint (g_file_error_from_errno (EAGAIN), ==, G_FILE_ERROR_AGAIN);
#endif
#ifdef EINTR
  g_assert_cmpint (g_file_error_from_errno (EINTR), ==, G_FILE_ERROR_INTR);
#endif
#ifdef EIO
  g_assert_cmpint (g_file_error_from_errno (EIO), ==, G_FILE_ERROR_IO);
#endif
#ifdef EPERM
  g_assert_cmpint (g_file_error_from_errno (EPERM), ==, G_FILE_ERROR_PERM);
#endif
#ifdef ENOSYS
  g_assert_cmpint (g_file_error_from_errno (ENOSYS), ==, G_FILE_ERROR_NOSYS);
#endif
}

static void
test_basename (void)
{
  gchar *b;

  b = g_path_get_basename ("");
  g_assert_cmpstr (b, ==, ".");
  g_free (b);

  b = g_path_get_basename ("///");
  g_assert_cmpstr (b, ==, G_DIR_SEPARATOR_S);
  g_free (b);

  b = g_path_get_basename ("/a/b/c/d");
  g_assert_cmpstr (b, ==, "d");
  g_free (b);
}

static void
test_dir_make_tmp (void)
{
  gchar *name;
  GError *error = NULL;
  gint ret;

  name = g_dir_make_tmp ("testXXXXXXtest", &error);
  g_assert_no_error (error);
  g_assert_true (g_file_test (name, G_FILE_TEST_IS_DIR));
  ret = g_rmdir (name);
  g_assert_cmpint (ret, ==, 0);
  g_free (name);

  name = g_dir_make_tmp (NULL, &error);
  g_assert_no_error (error);
  g_assert_true (g_file_test (name, G_FILE_TEST_IS_DIR));
  ret = g_rmdir (name);
  g_assert_cmpint (ret, ==, 0);
  g_free (name);

  name = g_dir_make_tmp ("test/XXXXXX", &error);
  g_assert_error (error, G_FILE_ERROR, G_FILE_ERROR_FAILED);
  g_clear_error (&error);
  g_assert_null (name);

  name = g_dir_make_tmp ("XXXXxX", &error);
  g_assert_error (error, G_FILE_ERROR, G_FILE_ERROR_FAILED);
  g_clear_error (&error);
  g_assert_null (name);
}

static void
test_file_open_tmp (void)
{
  gchar *name = NULL;
  GError *error = NULL;
  gint fd;

  fd = g_file_open_tmp ("testXXXXXXtest", &name, &error);
  g_assert_cmpint (fd, !=, -1);
  g_assert_no_error (error);
  g_assert_nonnull (name);
  unlink (name);
  g_free (name);
  close (fd);

  fd = g_file_open_tmp (NULL, &name, &error);
  g_assert_cmpint (fd, !=, -1);
  g_assert_no_error (error);
  g_assert_nonnull (name);
  g_unlink (name);
  g_free (name);
  close (fd);

  name = NULL;
  fd = g_file_open_tmp ("test/XXXXXX", &name, &error);
  g_assert_cmpint (fd, ==, -1);
  g_assert_null (name);
  g_assert_error (error, G_FILE_ERROR, G_FILE_ERROR_FAILED);
  g_clear_error (&error);

  fd = g_file_open_tmp ("XXXXxX", &name, &error);
  g_assert_cmpint (fd, ==, -1);
  g_assert_null (name);
  g_assert_error (error, G_FILE_ERROR, G_FILE_ERROR_FAILED);
  g_clear_error (&error);
}

static void
test_mkstemp (void)
{
  gchar *name;
  gint fd;

  name = g_strdup ("testXXXXXXtest"),
  fd = g_mkstemp (name);
  g_assert_cmpint (fd, !=, -1);
  g_assert_null (strstr (name, "XXXXXX"));
  unlink (name);
  close (fd);
  g_free (name);

  name = g_strdup ("testYYYYYYtest"),
  fd = g_mkstemp (name);
  g_assert_cmpint (fd, ==, -1);
  g_free (name);
}

static void
test_mkdtemp (void)
{
  gchar *name;
  gchar *ret;

  name = g_strdup ("testXXXXXXtest"),
  ret = g_mkdtemp (name);
  g_assert (ret == name);
  g_assert_null (strstr (name, "XXXXXX"));
  g_rmdir (name);
  g_free (name);

  name = g_strdup ("testYYYYYYtest"),
  ret = g_mkdtemp (name);
  g_assert_null (ret);
  g_free (name);
}

static void
test_set_contents (void)
{
  GError *error = NULL;
  gint fd;
  gchar *name;
  gchar *buf;
  gsize len;
  gboolean ret;

  fd = g_file_open_tmp (NULL, &name, &error);
  g_assert_no_error (error);
  write (fd, "a", 1);
  close (fd);

  ret = g_file_get_contents (name, &buf, &len, &error);
  g_assert_true (ret);
  g_assert_no_error (error);
  g_assert_cmpstr (buf, ==, "a");
  g_free (buf);

  ret = g_file_set_contents (name, "b", 1, &error);
  g_assert_true (ret);
  g_assert_no_error (error);

  ret = g_file_get_contents (name, &buf, &len, &error);
  g_assert_true (ret);
  g_assert_no_error (error);
  g_assert_cmpstr (buf, ==, "b");
  g_free (buf);

  g_remove (name);
  g_free (name);
}

static void
test_read_link (void)
{
#ifdef HAVE_READLINK
#ifdef G_OS_UNIX
  int ret;
  const gchar *oldpath;
  gchar *cwd;
  gchar *newpath;
  gchar *badpath;
  gchar *path;
  GError *error = NULL;

  cwd = g_get_current_dir ();

  oldpath = g_test_get_filename (G_TEST_DIST, "4096-random-bytes", NULL);
  newpath = g_build_filename (cwd, "page-of-junk", NULL);
  badpath = g_build_filename (cwd, "4097-random-bytes", NULL);
  remove (newpath);
  ret = symlink (oldpath, newpath);
  g_assert_cmpint (ret, ==, 0);
  path = g_file_read_link (newpath, &error);
  g_assert_no_error (error);
  g_assert_cmpstr (path, ==, oldpath);
  g_free (path);

  remove (newpath);
  ret = symlink (badpath, newpath);
  g_assert_cmpint (ret, ==, 0);
  path = g_file_read_link (newpath, &error);
  g_assert_no_error (error);
  g_assert_cmpstr (path, ==, badpath);
  g_free (path);

  path = g_file_read_link (oldpath, &error);
  g_assert_error (error, G_FILE_ERROR, G_FILE_ERROR_INVAL);
  g_assert_null (path);
  g_error_free (error);

  remove (newpath);
  g_free (cwd);
  g_free (newpath);
  g_free (badpath);

#endif
#else
  g_test_skip ("Symbolic links not supported");
#endif
}

static void
test_stdio_wrappers (void)
{
  GStatBuf buf;
  gchar *cwd, *path;
  gint ret;
  struct utimbuf ut;
  GError *error = NULL;
  GStatBuf path_statbuf, cwd_statbuf;

  /* The permissions tests here don’t work when running as root. */
#ifdef G_OS_UNIX
  if (getuid () == 0 || geteuid () == 0)
    {
      g_test_skip ("File permissions tests cannot be run as root");
      return;
    }
#endif

  g_remove ("mkdir-test/test-create");
  ret = g_rmdir ("mkdir-test");
  g_assert (ret == 0 || errno == ENOENT);

  ret = g_stat ("mkdir-test", &buf);
  g_assert_cmpint (ret, ==, -1);
  ret = g_mkdir ("mkdir-test", 0666);
  g_assert_cmpint (ret, ==, 0);
  ret = g_stat ("mkdir-test", &buf);
  g_assert_cmpint (ret, ==, 0);
  g_assert_cmpint (S_ISDIR (buf.st_mode), !=, 0);

  cwd = g_get_current_dir ();
  path = g_build_filename (cwd, "mkdir-test", NULL);
  g_free (cwd);
#ifndef G_OS_WIN32
  /* 0666 on directories means nothing to Windows, it only obeys ACLs */
  ret = g_chdir (path);
  g_assert_cmpint (errno, ==, EACCES);
  g_assert_cmpint (ret, ==, -1);
#endif
  ret = g_chmod (path, 0777);
  g_assert_cmpint (ret, ==, 0);
  ret = g_chdir (path);
  g_assert_cmpint (ret, ==, 0);
  cwd = g_get_current_dir ();
  /* We essentially want to check that cwd == path, but we can’t compare the
   * paths directly since the tests might be running under a symlink (for
   * example, /tmp is sometimes a symlink). Compare the inode numbers instead. */
  g_assert_cmpint (g_stat (cwd, &cwd_statbuf), ==, 0);
  g_assert_cmpint (g_stat (path, &path_statbuf), ==, 0);
  g_assert_true (cwd_statbuf.st_dev == path_statbuf.st_dev &&
                 cwd_statbuf.st_ino == path_statbuf.st_ino);
  g_free (cwd);
  g_free (path);

  ret = g_creat ("test-creat", 0555);
  g_close (ret, &error);
  g_assert_no_error (error);

  ret = g_access ("test-creat", F_OK);
  g_assert_cmpint (ret, ==, 0);

  ret = g_rename ("test-creat", "test-create");
  g_assert_cmpint (ret, ==, 0);

  ret = g_open ("test-create", O_RDONLY, 0666);
  g_close (ret, &error);
  g_assert_no_error (error);

#ifdef G_OS_WIN32
  /* On Windows the 5 permission bit results in a read-only file
   * that cannot be modified in any way (attribute changes included).
   * Remove the read-only attribute via chmod().
   */
  ret = g_chmod ("test-create", 0666);
  g_assert_cmpint (ret, ==, 0);
#endif

  ut.actime = ut.modtime = (time_t)0;
  ret = g_utime ("test-create", &ut);
  g_assert_cmpint (ret, ==, 0);

  ret = g_lstat ("test-create", &buf);
  g_assert_cmpint (ret, ==, 0);
  g_assert_cmpint (buf.st_atime, ==, (time_t)0);
  g_assert_cmpint (buf.st_mtime, ==, (time_t)0);

  g_chdir ("..");
  g_remove ("mkdir-test/test-create");
  g_rmdir ("mkdir-test");
}

/* Win32 does not support "wb+", but g_fopen() should automatically
 * translate this mode to its alias "w+b".
 * Also check various other file open modes for correct support accross
 * platforms.
 * See: https://gitlab.gnome.org/GNOME/glib/merge_requests/119
 */
static void
test_fopen_modes (void)
{
  char        *path = g_build_filename ("temp-fopen", NULL);
  gsize        i;
  const gchar *modes[] =
    {
      "w",
      "r",
      "a",
      "w+",
      "r+",
      "a+",
      "wb",
      "rb",
      "ab",
      "w+b",
      "r+b",
      "a+b",
      "wb+",
      "rb+",
      "ab+"
    };

  g_test_bug ("119");

  if (g_file_test (path, G_FILE_TEST_EXISTS))
    g_error ("failed, %s exists, cannot test g_fopen()", path);

  for (i = 0; i < G_N_ELEMENTS (modes); i++)
    {
      FILE *f;

      g_test_message ("Testing fopen() mode '%s'", modes[i]);

      f = g_fopen (path, modes[i]);
      g_assert_nonnull (f);
      fclose (f);
    }

  g_remove (path);
  g_free (path);
}

#ifdef G_OS_WIN32
#include "../gstdio-private.c"

static int
g_wcscmp0 (const gunichar2 *str1,
           const gunichar2 *str2)
{
  if (!str1)
    return -(str1 != str2);
  if (!str2)
    return str1 != str2;
  return wcscmp (str1, str2);
}

#define g_assert_cmpwcs(s1, cmp, s2, s1u8, s2u8) \
G_STMT_START { \
  const gunichar2 *__s1 = (s1), *__s2 = (s2); \
  if (g_wcscmp0 (__s1, __s2) cmp 0) ; else \
    g_assertion_message_cmpstr (G_LOG_DOMAIN, __FILE__, __LINE__, G_STRFUNC, \
                                #s1u8 " " #cmp " " #s2u8, s1u8, #cmp, s2u8); \
} G_STMT_END

static void
test_win32_pathstrip (void)
{
  gunichar2 *buf;
  gsize i;
#define IDENTITY_TEST(x) { x, x, FALSE }
  struct
  {
    gunichar2 *in;
    gunichar2 *out;
    gboolean   result;
  } testcases[] = {
    IDENTITY_TEST (L"\\\\?\\V"),
    IDENTITY_TEST (L"\\\\?\\Vo"),
    IDENTITY_TEST (L"\\\\?\\Volume{0700f3d3-6d24-11e3-8b2f-806e6f6e6963}\\"),
    IDENTITY_TEST (L"\\??\\V"),
    IDENTITY_TEST (L"\\??\\Vo"),
    IDENTITY_TEST (L"\\??\\Volume{0700f3d3-6d24-11e3-8b2f-806e6f6e6963}\\"),
    IDENTITY_TEST (L"\\\\?\\\x0441:\\"),
    IDENTITY_TEST (L"\\??\\\x0441:\\"),
    IDENTITY_TEST (L"a:\\"),
    IDENTITY_TEST (L"a:\\b\\c"),
    IDENTITY_TEST (L"x"),
#undef IDENTITY_TEST
    {
      L"\\\\?\\c:\\",
             L"c:\\",
      TRUE,
    },
    {
      L"\\\\?\\C:\\",
             L"C:\\",
      TRUE,
    },
    {
      L"\\\\?\\c:\\",
             L"c:\\",
      TRUE,
    },
    {
      L"\\\\?\\C:\\",
             L"C:\\",
      TRUE,
    },
    {
      L"\\\\?\\C:\\",
             L"C:\\",
      TRUE,
    },
    { 0, }
  };

  for (i = 0; testcases[i].in; i++)
    {
      gsize str_len = wcslen (testcases[i].in) + 1;
      gchar *in_u8 = g_utf16_to_utf8 (testcases[i].in, -1, NULL, NULL, NULL);
      gchar *out_u8 = g_utf16_to_utf8 (testcases[i].out, -1, NULL, NULL, NULL);

      g_assert_nonnull (in_u8);
      g_assert_nonnull (out_u8);

      buf = g_new0 (gunichar2, str_len);
      memcpy (buf, testcases[i].in, str_len * sizeof (gunichar2));
      _g_win32_strip_extended_ntobjm_prefix (buf, &str_len);
      g_assert_cmpwcs (buf, ==, testcases[i].out, in_u8, out_u8);
      g_free (buf);
      g_free (in_u8);
      g_free (out_u8);
    }
  /* Check for correct behaviour on non-NUL-terminated strings */
  for (i = 0; testcases[i].in; i++)
    {
      gsize str_len = wcslen (testcases[i].in) + 1;
      wchar_t old_endchar;
      gchar *in_u8 = g_utf16_to_utf8 (testcases[i].in, -1, NULL, NULL, NULL);
      gchar *out_u8 = g_utf16_to_utf8 (testcases[i].out, -1, NULL, NULL, NULL);

      g_assert_nonnull (in_u8);
      g_assert_nonnull (out_u8);

      buf = g_new0 (gunichar2, str_len);
      memcpy (buf, testcases[i].in, (str_len) * sizeof (gunichar2));

      old_endchar = buf[wcslen (testcases[i].out)];
      str_len -= 1;

      if (testcases[i].result)
        {
          /* Given "\\\\?\\C:\\" (len 7, unterminated),
           * we should get "C:\\" (len 3, unterminated).
           * Put a character different from "\\" (4-th character of the buffer)
           * at the end of the unterminated source buffer, into a position
           * where NUL-terminator would normally be. Then later test that 4-th character
           * in the buffer is still the old "\\".
           * After that terminate the string and use normal g_wcscmp0().
           */
          buf[str_len] = old_endchar - 1;
        }

      _g_win32_strip_extended_ntobjm_prefix (buf, &str_len);
      g_assert_cmpuint (old_endchar, ==, buf[wcslen (testcases[i].out)]);
      buf[str_len] = L'\0';
      g_assert_cmpwcs (buf, ==, testcases[i].out, in_u8, out_u8);
      g_free (buf);
      g_free (in_u8);
      g_free (out_u8);
    }
}

#endif

int
main (int   argc,
      char *argv[])
{
  g_setenv ("LC_ALL", "C", TRUE);
  g_test_init (&argc, &argv, NULL);

  g_test_bug_base ("https://gitlab.gnome.org/GNOME/glib/merge_requests/");

#ifdef G_OS_WIN32
  g_test_add_func ("/fileutils/stdio-win32-pathstrip", test_win32_pathstrip);
#endif
  g_test_add_func ("/fileutils/build-path", test_build_path);
  g_test_add_func ("/fileutils/build-pathv", test_build_pathv);
  g_test_add_func ("/fileutils/build-filename", test_build_filename);
  g_test_add_func ("/fileutils/build-filenamev", test_build_filenamev);
  g_test_add_func ("/fileutils/mkdir-with-parents", test_mkdir_with_parents);
  g_test_add_func ("/fileutils/format-size-for-display", test_format_size_for_display);
  g_test_add_func ("/fileutils/errors", test_file_errors);
  g_test_add_func ("/fileutils/basename", test_basename);
  g_test_add_func ("/fileutils/dir-make-tmp", test_dir_make_tmp);
  g_test_add_func ("/fileutils/file-open-tmp", test_file_open_tmp);
  g_test_add_func ("/fileutils/mkstemp", test_mkstemp);
  g_test_add_func ("/fileutils/mkdtemp", test_mkdtemp);
  g_test_add_func ("/fileutils/set-contents", test_set_contents);
  g_test_add_func ("/fileutils/read-link", test_read_link);
  g_test_add_func ("/fileutils/stdio-wrappers", test_stdio_wrappers);
  g_test_add_func ("/fileutils/fopen-modes", test_fopen_modes);

  return g_test_run ();
}
