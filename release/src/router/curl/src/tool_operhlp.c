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
#include "tool_operate.h"

#include "strcase.h"

#define ENABLE_CURLX_PRINTF
/* use our own printf() functions */
#include "curlx.h"

#include "tool_cfgable.h"
#include "tool_doswin.h"
#include "tool_operhlp.h"

#include "memdebug.h" /* keep this as LAST include */

void clean_getout(struct OperationConfig *config)
{
  if(config) {
    struct getout *next;
    struct getout *node = config->url_list;

    while(node) {
      next = node->next;
      Curl_safefree(node->url);
      Curl_safefree(node->outfile);
      Curl_safefree(node->infile);
      Curl_safefree(node);
      node = next;
    }
    config->url_list = NULL;
  }
  single_transfer_cleanup(config);
}

bool output_expected(const char *url, const char *uploadfile)
{
  if(!uploadfile)
    return TRUE;  /* download */
  if(checkprefix("http://", url) || checkprefix("https://", url))
    return TRUE;   /* HTTP(S) upload */

  return FALSE; /* non-HTTP upload, probably no output should be expected */
}

bool stdin_upload(const char *uploadfile)
{
  return (!strcmp(uploadfile, "-") ||
          !strcmp(uploadfile, ".")) ? TRUE : FALSE;
}

/* Convert a CURLUcode into a CURLcode */
CURLcode urlerr_cvt(CURLUcode ucode)
{
  if(ucode == CURLUE_OUT_OF_MEMORY)
    return CURLE_OUT_OF_MEMORY;
  else if(ucode == CURLUE_UNSUPPORTED_SCHEME)
    return CURLE_UNSUPPORTED_PROTOCOL;
  else if(ucode == CURLUE_LACKS_IDN)
    return CURLE_NOT_BUILT_IN;
  else if(ucode == CURLUE_BAD_HANDLE)
    return CURLE_BAD_FUNCTION_ARGUMENT;
  return CURLE_URL_MALFORMAT;
}

/*
 * Adds the file name to the URL if it doesn't already have one.
 * url will be freed before return if the returned pointer is different
 */
CURLcode add_file_name_to_url(CURL *curl, char **inurlp, const char *filename)
{
  CURLcode result = CURLE_URL_MALFORMAT;
  CURLUcode uerr;
  CURLU *uh = curl_url();
  char *path = NULL;
  if(uh) {
    char *ptr;
    uerr = curl_url_set(uh, CURLUPART_URL, *inurlp,
                    CURLU_GUESS_SCHEME|CURLU_NON_SUPPORT_SCHEME);
    if(uerr) {
      result = urlerr_cvt(uerr);
      goto fail;
    }
    uerr = curl_url_get(uh, CURLUPART_PATH, &path, 0);
    if(uerr) {
      result = urlerr_cvt(uerr);
      goto fail;
    }

    ptr = strrchr(path, '/');
    if(!ptr || !*++ptr) {
      /* The URL path has no file name part, add the local file name. In order
         to be able to do so, we have to create a new URL in another buffer.*/

      /* We only want the part of the local path that is on the right
         side of the rightmost slash and backslash. */
      const char *filep = strrchr(filename, '/');
      char *file2 = strrchr(filep?filep:filename, '\\');
      char *encfile;

      if(file2)
        filep = file2 + 1;
      else if(filep)
        filep++;
      else
        filep = filename;

      /* URL encode the file name */
      encfile = curl_easy_escape(curl, filep, 0 /* use strlen */);
      if(encfile) {
        char *newpath;
        char *newurl;
        if(ptr)
          /* there is a trailing slash on the path */
          newpath = aprintf("%s%s", path, encfile);
        else
          /* there is no trailing slash on the path */
          newpath = aprintf("%s/%s", path, encfile);

        curl_free(encfile);

        if(!newpath)
          goto fail;
        uerr = curl_url_set(uh, CURLUPART_PATH, newpath, 0);
        free(newpath);
        if(uerr) {
          result = urlerr_cvt(uerr);
          goto fail;
        }
        uerr = curl_url_get(uh, CURLUPART_URL, &newurl, CURLU_DEFAULT_SCHEME);
        if(uerr) {
          result = urlerr_cvt(uerr);
          goto fail;
        }
        free(*inurlp);
        *inurlp = newurl;
        result = CURLE_OK;
      }
    }
    else
      /* nothing to do */
      result = CURLE_OK;
  }
  fail:
  curl_url_cleanup(uh);
  curl_free(path);
  return result;
}

/* Extracts the name portion of the URL.
 * Returns a pointer to a heap-allocated string or NULL if
 * no name part, at location indicated by first argument.
 */
CURLcode get_url_file_name(char **filename, const char *url)
{
  const char *pc, *pc2;
  CURLU *uh = curl_url();
  char *path = NULL;
  CURLUcode uerr;

  if(!uh)
    return CURLE_OUT_OF_MEMORY;

  *filename = NULL;

  uerr = curl_url_set(uh, CURLUPART_URL, url, CURLU_GUESS_SCHEME);
  if(!uerr) {
    uerr = curl_url_get(uh, CURLUPART_PATH, &path, 0);
    if(!uerr) {
      curl_url_cleanup(uh);

      pc = strrchr(path, '/');
      pc2 = strrchr(pc ? pc + 1 : path, '\\');
      if(pc2)
        pc = pc2;

      if(pc)
        /* duplicate the string beyond the slash */
        pc++;
      else
        /* no slash => empty string */
        pc = "";

      *filename = strdup(pc);
      curl_free(path);
      if(!*filename)
        return CURLE_OUT_OF_MEMORY;

#if defined(MSDOS) || defined(WIN32)
      {
        char *sanitized;
        SANITIZEcode sc = sanitize_file_name(&sanitized, *filename, 0);
        Curl_safefree(*filename);
        if(sc) {
          if(sc == SANITIZE_ERR_OUT_OF_MEMORY)
            return CURLE_OUT_OF_MEMORY;
          return CURLE_URL_MALFORMAT;
        }
        *filename = sanitized;
      }
#endif /* MSDOS || WIN32 */

      /* in case we built debug enabled, we allow an environment variable
       * named CURL_TESTDIR to prefix the given file name to put it into a
       * specific directory
       */
#ifdef DEBUGBUILD
      {
        char *tdir = curlx_getenv("CURL_TESTDIR");
        if(tdir) {
          char *alt = aprintf("%s/%s", tdir, *filename);
          Curl_safefree(*filename);
          *filename = alt;
          curl_free(tdir);
          if(!*filename)
            return CURLE_OUT_OF_MEMORY;
        }
      }
#endif
      return CURLE_OK;
    }
  }
  curl_url_cleanup(uh);
  return urlerr_cvt(uerr);
}
