/* Copyright (c) 2003, Roger Dingledine
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file path.c
 *
 * \brief Manipulate strings that contain filesystem paths.
 **/

#include "lib/fs/path.h"
#include "lib/malloc/malloc.h"
#include "lib/log/log.h"
#include "lib/log/util_bug.h"
#include "lib/container/smartlist.h"
#include "lib/sandbox/sandbox.h"
#include "lib/string/printf.h"
#include "lib/string/util_string.h"
#include "lib/string/compat_ctype.h"
#include "lib/string/compat_string.h"
#include "lib/fs/files.h"
#include "lib/fs/dir.h"
#include "lib/fs/userdb.h"

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef _WIN32
#include <windows.h>
#include <shlwapi.h>
#else /* !(defined(_WIN32)) */
#include <dirent.h>
#include <glob.h>
#endif /* defined(_WIN32) */

#include <errno.h>
#include <string.h>

/** Removes enclosing quotes from <b>path</b> and unescapes quotes between the
 * enclosing quotes. Backslashes are not unescaped. Return the unquoted
 * <b>path</b> on success or 0 if <b>path</b> is not quoted correctly. */
char *
get_unquoted_path(const char *path)
{
  size_t len = strlen(path);

  if (len == 0) {
    return tor_strdup("");
  }

  int has_start_quote = (path[0] == '\"');
  int has_end_quote = (len > 0 && path[len-1] == '\"');
  if (has_start_quote != has_end_quote || (len == 1 && has_start_quote)) {
    return NULL;
  }

  char *unquoted_path = tor_malloc(len - has_start_quote - has_end_quote + 1);
  char *s = unquoted_path;
  size_t i;
  for (i = has_start_quote; i < len - has_end_quote; i++) {
    if (path[i] == '\"' && (i > 0 && path[i-1] == '\\')) {
      *(s-1) = path[i];
    } else if (path[i] != '\"') {
      *s++ = path[i];
    } else {  /* unescaped quote */
      tor_free(unquoted_path);
      return NULL;
    }
  }
  *s = '\0';
  return unquoted_path;
}

/** Expand any homedir prefix on <b>filename</b>; return a newly allocated
 * string. */
char *
expand_filename(const char *filename)
{
  tor_assert(filename);
#ifdef _WIN32
  /* Might consider using GetFullPathName() as described here:
   * http://etutorials.org/Programming/secure+programming/
   *     Chapter+3.+Input+Validation/3.7+Validating+Filenames+and+Paths/
   */
  return tor_strdup(filename);
#else /* !defined(_WIN32) */
  if (*filename == '~') {
    char *home, *result=NULL;
    const char *rest;

    if (filename[1] == '/' || filename[1] == '\0') {
      home = getenv("HOME");
      if (!home) {
        log_warn(LD_CONFIG, "Couldn't find $HOME environment variable while "
                 "expanding \"%s\"; defaulting to \"\".", filename);
        home = tor_strdup("");
      } else {
        home = tor_strdup(home);
      }
      rest = strlen(filename)>=2?(filename+2):"";
    } else {
#ifdef HAVE_PWD_H
      char *username, *slash;
      slash = strchr(filename, '/');
      if (slash)
        username = tor_strndup(filename+1,slash-filename-1);
      else
        username = tor_strdup(filename+1);
      if (!(home = get_user_homedir(username))) {
        log_warn(LD_CONFIG,"Couldn't get homedir for \"%s\"",username);
        tor_free(username);
        return NULL;
      }
      tor_free(username);
      rest = slash ? (slash+1) : "";
#else /* !defined(HAVE_PWD_H) */
      log_warn(LD_CONFIG, "Couldn't expand homedir on system without pwd.h");
      return tor_strdup(filename);
#endif /* defined(HAVE_PWD_H) */
    }
    tor_assert(home);
    /* Remove trailing slash. */
    if (strlen(home)>1 && !strcmpend(home,PATH_SEPARATOR)) {
      home[strlen(home)-1] = '\0';
    }
    tor_asprintf(&result,"%s"PATH_SEPARATOR"%s",home,rest);
    tor_free(home);
    return result;
  } else {
    return tor_strdup(filename);
  }
#endif /* defined(_WIN32) */
}

/** Return true iff <b>filename</b> is a relative path. */
int
path_is_relative(const char *filename)
{
  if (filename && filename[0] == '/')
    return 0;
#ifdef _WIN32
  else if (filename && filename[0] == '\\')
    return 0;
  else if (filename && strlen(filename)>3 && TOR_ISALPHA(filename[0]) &&
           filename[1] == ':' && filename[2] == '\\')
    return 0;
#endif /* defined(_WIN32) */
  else
    return 1;
}

/** Clean up <b>name</b> so that we can use it in a call to "stat".  On Unix,
 * we do nothing.  On Windows, we remove a trailing slash, unless the path is
 * the root of a disk. */
void
clean_fname_for_stat(char *name)
{
#ifdef _WIN32
  size_t len = strlen(name);
  if (!len)
    return;
  if (name[len-1]=='\\' || name[len-1]=='/') {
    if (len == 1 || (len==3 && name[1]==':'))
      return;
    name[len-1]='\0';
  }
#else /* !defined(_WIN32) */
  (void)name;
#endif /* defined(_WIN32) */
}

/** Modify <b>fname</b> to contain the name of its parent directory.  Doesn't
 * actually examine the filesystem; does a purely syntactic modification.
 *
 * The parent of the root director is considered to be itself.
 *
 * Path separators are the forward slash (/) everywhere and additionally
 * the backslash (\) on Win32.
 *
 * Cuts off any number of trailing path separators but otherwise ignores
 * them for purposes of finding the parent directory.
 *
 * Returns 0 if a parent directory was successfully found, -1 otherwise (fname
 * did not have any path separators or only had them at the end).
 * */
int
get_parent_directory(char *fname)
{
  char *cp;
  int at_end = 1;
  tor_assert(fname);
#ifdef _WIN32
  /* If we start with, say, c:, then don't consider that the start of the path
   */
  if (fname[0] && fname[1] == ':') {
    fname += 2;
  }
#endif /* defined(_WIN32) */
  /* Now we want to remove all path-separators at the end of the string,
   * and to remove the end of the string starting with the path separator
   * before the last non-path-separator.  In perl, this would be
   *   s#[/]*$##; s#/[^/]*$##;
   * on a unixy platform.
   */
  cp = fname + strlen(fname);
  at_end = 1;
  while (--cp >= fname) {
    int is_sep = (*cp == '/'
#ifdef _WIN32
                  || *cp == '\\'
#endif
                  );
    if (is_sep) {
      if (cp == fname) {
        /* This is the first separator in the file name; don't remove it! */
        cp[1] = '\0';
        return 0;
      }
      *cp = '\0';
      if (! at_end)
        return 0;
    } else {
      at_end = 0;
    }
  }
  return -1;
}

#ifndef _WIN32
/** Return a newly allocated string containing the output of getcwd(). Return
 * NULL on failure. (We can't just use getcwd() into a PATH_MAX buffer, since
 * Hurd hasn't got a PATH_MAX.)
 */
static char *
alloc_getcwd(void)
{
#ifdef HAVE_GET_CURRENT_DIR_NAME
  /* Glibc makes this nice and simple for us. */
  char *cwd = get_current_dir_name();
  char *result = NULL;
  if (cwd) {
    /* We make a copy here, in case tor_malloc() is not malloc(). */
    result = tor_strdup(cwd);
    raw_free(cwd); // alias for free to avoid tripping check-spaces.
  }
  return result;
#else /* !defined(HAVE_GET_CURRENT_DIR_NAME) */
  size_t size = 1024;
  char *buf = NULL;
  char *ptr = NULL;

  while (ptr == NULL) {
    buf = tor_realloc(buf, size);
    ptr = getcwd(buf, size);

    if (ptr == NULL && errno != ERANGE) {
      tor_free(buf);
      return NULL;
    }

    size *= 2;
  }
  return buf;
#endif /* defined(HAVE_GET_CURRENT_DIR_NAME) */
}
#endif /* !defined(_WIN32) */

/** Expand possibly relative path <b>fname</b> to an absolute path.
 * Return a newly allocated string, which may be a duplicate of <b>fname</b>.
 */
char *
make_path_absolute(const char *fname)
{
#ifdef _WIN32
  char *absfname_malloced = _fullpath(NULL, fname, 1);

  /* We don't want to assume that tor_free can free a string allocated
   * with malloc.  On failure, return fname (it's better than nothing). */
  char *absfname = tor_strdup(absfname_malloced ? absfname_malloced : fname);
  if (absfname_malloced) raw_free(absfname_malloced);

  return absfname;
#else /* !defined(_WIN32) */
  char *absfname = NULL, *path = NULL;

  tor_assert(fname);

  if (fname[0] == '/') {
    absfname = tor_strdup(fname);
  } else {
    path = alloc_getcwd();
    if (path) {
      tor_asprintf(&absfname, "%s/%s", path, fname);
      tor_free(path);
    } else {
      /* LCOV_EXCL_START Can't make getcwd fail. */
      /* If getcwd failed, the best we can do here is keep using the
       * relative path.  (Perhaps / isn't readable by this UID/GID.) */
      log_warn(LD_GENERAL, "Unable to find current working directory: %s",
               strerror(errno));
      absfname = tor_strdup(fname);
      /* LCOV_EXCL_STOP */
    }
  }
  return absfname;
#endif /* defined(_WIN32) */
}

/* The code below implements tor_glob and get_glob_opened_files. Because it is
 * not easy to understand it by looking at individual functions, the big
 * picture explanation here should be read first.
 *
 * Purpose of the functions:
 * - tor_glob - receives a pattern and returns all the paths that result from
 *   its glob expansion, globs can be present on all path components.
 * - get_glob_opened_files - receives a pattern and returns all the paths that
 *   are opened during its expansion (the paths before any path fragment that
 *   contains a glob as they have to be opened to check for glob matches). This
 *   is used to get the paths that have to be added to the seccomp sandbox
 *   allowed list.
 *
 * Due to OS API differences explained below, the implementation of tor_glob is
 * completely different for Windows and POSIX systems, so we ended up with
 * three different implementations:
 * - tor_glob for POSIX - as POSIX glob does everything we need, we simply call
 *   it and process the results. This is completely implemented in tor_glob.
 * - tor_glob for WIN32 - because the WIN32 API only supports expanding globs
 *   in the last path fragment, we need to expand the globs in each path
 *   fragment manually and call recursively to get the same behaviour as POSIX
 *   glob. When there are no globs in pattern, we know we are on the last path
 *   fragment and collect the full path.
 * - get_glob_opened_files - because the paths before any path fragment with a
 *   glob will be opened to check for matches, we need to collect them and we
 *   need to expand the globs in each path fragments and call recursively until
 *   we find no more globs.
 *
 * As seen from the description above, both tor_glob for WIN32 and
 * get_glob_opened_files receive a pattern and return a list of paths and have
 * to expand all path fragments that contain globs and call themselves
 * recursively. The differences are:
 * - get_glob_opened_files collects paths before path fragments with globs
 *   while tor_glob for WIN32 collects full paths resulting from the expansion
 *   of all globs.
 * - get_glob_opened_files can call tor_glob to expand path fragments with
 *   globs while tor_glob for WIN32 cannot because it IS tor_glob. For tor_glob
 *   for WIN32, an auxiliary function has to be used for this purpose.
 *
 * To avoid code duplication, the logic of tor_glob for WIN32 and
 * get_glob_opened_files is implemented in get_glob_paths. The differences are
 * configured by the extra function parameters:
 * - final - if true, returns a list of paths obtained from expanding pattern
 *   (implements tor_glob). Otherwise, returns the paths before path fragments
 *   with globs (implements get_glob_opened_files).
 * - unglob - function used to expand a path fragment. The function signature
 *   is defined by the unglob_fn typedef. Two implementations are available:
 *   - unglob_win32 - uses tor_listdir and PathMatchSpec (for tor_glob WIN32)
 *   - unglob_opened_files - uses tor_glob (for get_glob_opened_files)
 */

/** Returns true if the character at position <b>pos</b> in <b>pattern</b> is
 * considered a glob. Returns false otherwise. Takes escaping into account on
 * systems where escaping globs is supported. */
static inline bool
is_glob_char(const char *pattern, int pos)
{
  bool is_glob = pattern[pos] == '*' || pattern[pos] == '?';
#ifdef _WIN32
  return is_glob;
#else /* !defined(_WIN32) */
  bool is_escaped = pos > 0 && pattern[pos-1] == '\\';
  return is_glob && !is_escaped;
#endif /* defined(_WIN32) */
}

/** Expands the first path fragment of <b>pattern</b> that contains globs. The
 * path fragment is between <b>prev_sep</b> and <b>next_sep</b>. If the path
 * fragment is the last fragment of <b>pattern</b>, <b>next_sep</b> will be the
 * index of the last char. Returns a list of paths resulting from the glob
 * expansion of the path fragment. Anything after <b>next_sep</b> is not
 * included in the returned list. Returns NULL on failure. */
typedef struct smartlist_t * unglob_fn(const char *pattern, int prev_sep,
                                       int next_sep);

/** Adds <b>path</b> to <b>result</b> if it exists and is a file type we can
 * handle. Returns false if <b>path</b> is a file type we cannot handle,
 * returns true otherwise. Used on tor_glob for WIN32. */
static bool
add_non_glob_path(const char *path, struct smartlist_t *result)
{
  file_status_t file_type = file_status(path);
  if (file_type == FN_ERROR) {
    return false;
  } else if (file_type != FN_NOENT) {
    char *to_add = tor_strdup(path);
    clean_fname_for_stat(to_add);
    smartlist_add(result, to_add);
  }
  /* If WIN32 tor_glob is called with a non-existing path, we want it to
   * return an empty list instead of error to match the regular version */
  return true;
}

/** Auxiliary function used by get_glob_opened_files and WIN32 tor_glob.
 * Returns a list of paths obtained from <b>pattern</b> using <b>unglob</b> to
 * expand each path fragment. If <b>final</b> is true, the paths are the result
 * of the glob expansion of <b>pattern</b> (implements tor_glob). Otherwise,
 * the paths are the paths opened by glob while expanding <b>pattern</b>
 * (implements get_glob_opened_files). Returns NULL on failure. */
static struct smartlist_t *
get_glob_paths(const char *pattern, unglob_fn unglob, bool final)
{
  smartlist_t *result = smartlist_new();
  int i, prev_sep = -1, next_sep = -1;
  bool is_glob = false, error_found = false, is_sep = false, is_last = false;

  // find first path fragment with globs
  for (i = 0; pattern[i]; i++) {
    is_glob = is_glob || is_glob_char(pattern, i);
    is_last = !pattern[i+1];
    is_sep = pattern[i] == *PATH_SEPARATOR || pattern[i] == '/';
    if (is_sep || is_last) {
      prev_sep = next_sep;
      next_sep = i; // next_sep+1 is start of next fragment or end of string
      if (is_glob) {
        break;
      }
    }
  }

  if (!is_glob) { // pattern fully expanded or no glob in pattern
    if (final && !add_non_glob_path(pattern, result)) {
      error_found = true;
      goto end;
    }
    return result;
  }

  if (!final) {
    // add path before the glob to result
    int len = prev_sep < 1 ? prev_sep + 1 : prev_sep; // handle /*
    char *path_until_glob = tor_strndup(pattern, len);
    smartlist_add(result, path_until_glob);
  }

  smartlist_t *unglobbed_paths = unglob(pattern, prev_sep, next_sep);
  if (!unglobbed_paths) {
    error_found = true;
  } else {
    // for each path for current fragment, add the rest of the pattern
    // and call recursively to get all expanded paths
    SMARTLIST_FOREACH_BEGIN(unglobbed_paths, char *, current_path) {
      char *next_path;
      tor_asprintf(&next_path, "%s"PATH_SEPARATOR"%s", current_path,
                   &pattern[next_sep+1]);
      smartlist_t *opened_next = get_glob_paths(next_path, unglob, final);
      tor_free(next_path);
      if (!opened_next) {
        error_found = true;
        break;
      }
      smartlist_add_all(result, opened_next);
      smartlist_free(opened_next);
    } SMARTLIST_FOREACH_END(current_path);
    SMARTLIST_FOREACH(unglobbed_paths, char *, p, tor_free(p));
    smartlist_free(unglobbed_paths);
  }

end:
  if (error_found) {
    SMARTLIST_FOREACH(result, char *, p, tor_free(p));
    smartlist_free(result);
    result = NULL;
  }
  return result;
}

#ifdef _WIN32
/** Expands globs in <b>pattern</b> for the path fragment between
 * <b>prev_sep</b> and <b>next_sep</b> using the WIN32 API. Returns NULL on
 * failure. Used by the WIN32 implementation of tor_glob. Implements unglob_fn,
 * see its description for more details. */
static struct smartlist_t *
unglob_win32(const char *pattern, int prev_sep, int next_sep)
{
  smartlist_t *result = smartlist_new();
  int len = prev_sep < 1 ? prev_sep + 1 : prev_sep; // handle /*
  char *path_until_glob = tor_strndup(pattern, len);

  if (!is_file(file_status(path_until_glob))) {
    smartlist_t *filenames = tor_listdir(path_until_glob);
    if (!filenames) {
      smartlist_free(result);
      result = NULL;
    } else {
      SMARTLIST_FOREACH_BEGIN(filenames, char *, filename) {
        TCHAR tpattern[MAX_PATH] = {0};
        TCHAR tfile[MAX_PATH] = {0};
        char *full_path;
        tor_asprintf(&full_path, "%s"PATH_SEPARATOR"%s",
                     path_until_glob, filename);
        char *path_curr_glob = tor_strndup(pattern, next_sep + 1);
        // *\ must return only dirs, remove \ from the pattern so it matches
        if (is_dir(file_status(full_path))) {
          clean_fname_for_stat(path_curr_glob);
        }
#ifdef UNICODE
        mbstowcs(tpattern, path_curr_glob, MAX_PATH);
        mbstowcs(tfile, full_path, MAX_PATH);
#else /* !defined(UNICODE) */
        strlcpy(tpattern, path_curr_glob, MAX_PATH);
        strlcpy(tfile, full_path, MAX_PATH);
#endif /* defined(UNICODE) */
        if (PathMatchSpec(tfile, tpattern)) {
          smartlist_add(result, full_path);
        } else {
          tor_free(full_path);
        }
        tor_free(path_curr_glob);
      } SMARTLIST_FOREACH_END(filename);
      SMARTLIST_FOREACH(filenames, char *, p, tor_free(p));
      smartlist_free(filenames);
    }
  }
  tor_free(path_until_glob);
  return result;
}
#elif HAVE_GLOB
#ifdef GLOB_ALTDIRFUNC  // prevent warning about unused functions
/** Same as opendir but calls sandbox_intern_string before */
static DIR *
prot_opendir(const char *name)
{
  if (sandbox_interned_string_is_missing(name)) {
    errno = EPERM;
    return NULL;
  }
  return opendir(sandbox_intern_string(name));
}

/** Same as stat but calls sandbox_intern_string before */
static int
prot_stat(const char *pathname, struct stat *buf)
{
  if (sandbox_interned_string_is_missing(pathname)) {
    errno = EPERM;
    return -1;
  }
  return stat(sandbox_intern_string(pathname), buf);
}

/** Same as lstat but calls sandbox_intern_string before */
static int
prot_lstat(const char *pathname, struct stat *buf)
{
  if (sandbox_interned_string_is_missing(pathname)) {
    errno = EPERM;
    return -1;
  }
  return lstat(sandbox_intern_string(pathname), buf);
}
/** As closedir, but has the right type for gl_closedir */
static void
wrap_closedir(void *arg)
{
  closedir(arg);
}
#endif /* defined(GLOB_ALTDIRFUNC) */

/** Function passed to glob to handle processing errors. <b>epath</b> is the
 * path that caused the error and <b>eerrno</b> is the errno set by the
 * function that failed. We want to ignore ENOENT and ENOTDIR because, in BSD
 * systems, these are not ignored automatically, which makes glob fail when
 * globs expand to non-existing paths and GLOB_ERR is set.
 */
static int
glob_errfunc(const char *epath, int eerrno)
{
    (void)epath;
    return eerrno == ENOENT || eerrno == ENOTDIR ? 0 : -1;
}
#endif /* defined(HAVE_GLOB) */

/** Return a new list containing the paths that match the pattern
 * <b>pattern</b>. Return NULL on error. On POSIX systems, errno is set by the
 * glob function or is set to EPERM if glob tried to access a file not allowed
 * by the seccomp sandbox.
 */
struct smartlist_t *
tor_glob(const char *pattern)
{
  smartlist_t *result = NULL;

#ifdef _WIN32
  // PathMatchSpec does not support forward slashes, change them to backslashes
  char *pattern_normalized = tor_strdup(pattern);
  tor_strreplacechar(pattern_normalized, '/', *PATH_SEPARATOR);
  result = get_glob_paths(pattern_normalized, unglob_win32, true);
  tor_free(pattern_normalized);
#elif HAVE_GLOB /* !(defined(_WIN32)) */
  glob_t matches;
  int flags = GLOB_NOSORT;
#ifdef GLOB_ALTDIRFUNC
  /* use functions that call sandbox_intern_string */
  flags |= GLOB_ALTDIRFUNC;
  typedef void *(*gl_opendir)(const char * name);
  typedef struct dirent *(*gl_readdir)(void *);
  typedef void (*gl_closedir)(void *);
  matches.gl_opendir = (gl_opendir) &prot_opendir;
  matches.gl_readdir = (gl_readdir) &readdir;
  matches.gl_closedir = (gl_closedir) &wrap_closedir;
  matches.gl_stat = &prot_stat;
  matches.gl_lstat = &prot_lstat;
#endif /* defined(GLOB_ALTDIRFUNC) */
  // use custom error handler to workaround BSD quirks and do not set GLOB_ERR
  // because it would make glob fail on error even if the error handler ignores
  // the error
  int ret = glob(pattern, flags, glob_errfunc, &matches);
  if (ret == GLOB_NOMATCH) {
    return smartlist_new();
  } else if (ret != 0) {
    return NULL;
  }

  // #40141, !249: workaround for glibc bug where patterns ending in path
  // separator match files and folders instead of folders only.
  // this could be in #ifdef __GLIBC__ but: 1. it might affect other libcs too,
  // and 2. it doesn't cost much to stat each match again since libc is already
  // supposed to do it (otherwise the file may be on slow NFS or something)
  size_t pattern_len = strlen(pattern);
  bool dir_only = pattern_len > 0 && pattern[pattern_len-1] == *PATH_SEPARATOR;

  result = smartlist_new();
  size_t i;
  for (i = 0; i < matches.gl_pathc; i++) {
    char *match = tor_strdup(matches.gl_pathv[i]);
    size_t len = strlen(match);
    if (len > 0 && match[len-1] == *PATH_SEPARATOR) {
      match[len-1] = '\0';
    }

    if (!dir_only || (dir_only && is_dir(file_status(match)))) {
      smartlist_add(result, match);
    } else {
      tor_free(match);
    }
  }
  globfree(&matches);
#else
  (void)pattern;
  return result;
#endif /* defined(_WIN32) || ... */

  return result;
}

/** Returns true if <b>s</b> contains characters that can be globbed.
 * Returns false otherwise. */
bool
has_glob(const char *s)
{
  int i;
  for (i = 0; s[i]; i++) {
    if (is_glob_char(s, i)) {
      return true;
    }
  }
  return false;
}

/** Expands globs in <b>pattern</b> for the path fragment between
 * <b>prev_sep</b> and <b>next_sep</b> using tor_glob. Returns NULL on
 * failure. Used by get_glob_opened_files. Implements unglob_fn, see its
 * description for more details. */
static struct smartlist_t *
unglob_opened_files(const char *pattern, int prev_sep, int next_sep)
{
  (void)prev_sep;
  smartlist_t *result = smartlist_new();
  // if the following fragments have no globs, we're done
  if (has_glob(&pattern[next_sep+1])) {
    // if there is a glob after next_sep, we know next_sep is a separator and
    // not the last char and glob_path will have the path without the separator
    char *glob_path = tor_strndup(pattern, next_sep);
    smartlist_t *child_paths = tor_glob(glob_path);
    tor_free(glob_path);
    if (!child_paths) {
      smartlist_free(result);
      result = NULL;
    } else {
      smartlist_add_all(result, child_paths);
      smartlist_free(child_paths);
    }
  }
  return result;
}

/** Returns a list of files that are opened by the tor_glob function when
 * called with <b>pattern</b>. Returns NULL on error. The purpose of this
 * function is to create a list of files to be added to the sandbox white list
 * before the sandbox is enabled. */
struct smartlist_t *
get_glob_opened_files(const char *pattern)
{
  return get_glob_paths(pattern, unglob_opened_files, false);
}
