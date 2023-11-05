/***************************************************************************
 *                                  _   _ ____  _
 *  Project                     ___| | | |  _ \| |
 *                             / __| | | | |_) | |
 *                            | (__| |_| |  _ <| |___
 *                             \___|\___/|_| \_\_____|
 *
 * Copyright (C) Daniel Stenberg, <daniel@haxx.se>, et al.
 *
 * This software is licensed as described in the file COPYING, which
 * you should have received as part of this distribution. The terms
 * are also available at https://curl.se/docs/copyright.html.
 *
 * You may opt to use, copy, modify, merge, publish, distribute and/or sell
 * copies of the Software, and permit persons to whom the Software is
 * furnished to do so, under the terms of the COPYING file.
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
 * KIND, either express or implied.
 *
 * SPDX-License-Identifier: curl
 *
 ***************************************************************************/
#include "tool_setup.h"

#include <sys/stat.h>

#ifdef WIN32
#  include <direct.h>
#endif

#define ENABLE_CURLX_PRINTF
/* use our own printf() functions */
#include "curlx.h"

#include "tool_dirhie.h"
#include "tool_msgs.h"

#include "memdebug.h" /* keep this as LAST include */

#if defined(WIN32) || (defined(MSDOS) && !defined(__DJGPP__))
#  define mkdir(x,y) (mkdir)((x))
#  ifndef F_OK
#    define F_OK 0
#  endif
#endif

static void show_dir_errno(struct GlobalConfig *global, const char *name)
{
  switch(errno) {
#ifdef EACCES
  case EACCES:
    errorf(global, "You don't have permission to create %s", name);
    break;
#endif
#ifdef ENAMETOOLONG
  case ENAMETOOLONG:
    errorf(global, "The directory name %s is too long", name);
    break;
#endif
#ifdef EROFS
  case EROFS:
    errorf(global, "%s resides on a read-only file system", name);
    break;
#endif
#ifdef ENOSPC
  case ENOSPC:
    errorf(global, "No space left on the file system that will "
           "contain the directory %s", name);
    break;
#endif
#ifdef EDQUOT
  case EDQUOT:
    errorf(global, "Cannot create directory %s because you "
           "exceeded your quota", name);
    break;
#endif
  default:
    errorf(global, "Error creating directory %s", name);
    break;
  }
}

/*
 * Create the needed directory hierarchy recursively in order to save
 *  multi-GETs in file output, ie:
 *  curl "http://example.org/dir[1-5]/file[1-5].txt" -o "dir#1/file#2.txt"
 *  should create all the dir* automagically
 */

#if defined(WIN32) || defined(__DJGPP__)
/* systems that may use either or when specifying a path */
#define PATH_DELIMITERS "\\/"
#else
#define PATH_DELIMITERS DIR_CHAR
#endif


CURLcode create_dir_hierarchy(const char *outfile, struct GlobalConfig *global)
{
  char *tempdir;
  char *tempdir2;
  char *outdup;
  char *dirbuildup;
  CURLcode result = CURLE_OK;
  size_t outlen;

  outlen = strlen(outfile);
  outdup = strdup(outfile);
  if(!outdup)
    return CURLE_OUT_OF_MEMORY;

  dirbuildup = malloc(outlen + 1);
  if(!dirbuildup) {
    Curl_safefree(outdup);
    return CURLE_OUT_OF_MEMORY;
  }
  dirbuildup[0] = '\0';

  /* Allow strtok() here since this isn't used threaded */
  /* !checksrc! disable BANNEDFUNC 2 */
  tempdir = strtok(outdup, PATH_DELIMITERS);

  while(tempdir) {
    bool skip = false;
    tempdir2 = strtok(NULL, PATH_DELIMITERS);
    /* since strtok returns a token for the last word even
       if not ending with DIR_CHAR, we need to prune it */
    if(tempdir2) {
      size_t dlen = strlen(dirbuildup);
      if(dlen)
        msnprintf(&dirbuildup[dlen], outlen - dlen, "%s%s", DIR_CHAR, tempdir);
      else {
        if(outdup == tempdir) {
#if defined(MSDOS) || defined(WIN32)
          /* Skip creating a drive's current directory.
             It may seem as though that would harmlessly fail but it could be
             a corner case if X: did not exist, since we would be creating it
             erroneously.
             eg if outfile is X:\foo\bar\filename then don't mkdir X:
             This logic takes into account unsupported drives !:, 1:, etc. */
          char *p = strchr(tempdir, ':');
          if(p && !p[1])
            skip = true;
#endif
          /* the output string doesn't start with a separator */
          strcpy(dirbuildup, tempdir);
        }
        else
          msnprintf(dirbuildup, outlen, "%s%s", DIR_CHAR, tempdir);
      }
      /* Create directory. Ignore access denied error to allow traversal. */
      if(!skip && (-1 == mkdir(dirbuildup, (mode_t)0000750)) &&
         (errno != EACCES) && (errno != EEXIST)) {
        show_dir_errno(global, dirbuildup);
        result = CURLE_WRITE_ERROR;
        break; /* get out of loop */
      }
    }
    tempdir = tempdir2;
  }

  Curl_safefree(dirbuildup);
  Curl_safefree(outdup);

  return result;
}
