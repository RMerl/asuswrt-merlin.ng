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

/* In this file, stdio.h's stderr macro is not overridden. */
#define CURL_DO_NOT_OVERRIDE_STDERR

#include "tool_setup.h"

#include "tool_stderr.h"

#include "memdebug.h" /* keep this as LAST include */

/* In other tool files stderr is defined as tool_stderr by tool_setup.h */
FILE *tool_stderr;

void tool_init_stderr(void)
{
  tool_stderr = stderr;
}

void tool_set_stderr_file(char *filename)
{
  FILE *fp;

  if(!filename)
    return;

  if(!strcmp(filename, "-")) {
    tool_stderr = stdout;
    return;
  }

  /* precheck that filename is accessible to lessen the chance that the
     subsequent freopen will fail. */
  fp = fopen(filename, FOPEN_WRITETEXT);
  if(!fp) {
    fprintf(tool_stderr, "Warning: Failed to open %s!\n", filename);
    return;
  }
  fclose(fp);

  /* freopen the actual stderr (stdio.h stderr) instead of tool_stderr since
     the latter may be set to stdout. */
  fp = freopen(filename, FOPEN_WRITETEXT, stderr);
  if(!fp) {
    /* stderr may have been closed by freopen. there is nothing to be done. */
    DEBUGASSERT(0);
    return;
  }
  tool_stderr = stderr;
}
