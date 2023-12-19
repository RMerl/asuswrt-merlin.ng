/* -*- mode: c; c-basic-offset: 2 -*- */
/* 
 * Copyright (C) 2003, 2004, 2005 Mondru AB.
 * Copyright (C) 2007-2012 David Bird (Coova Technologies) <support@coova.com>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * 
 */

#include "chilli.h"

char *safe_strncpy(char *dst, const char *src, size_t size) {
  if (!size) return dst;
  dst[--size] = '\0';
  return strncpy(dst, src, size);
}

int statedir_file(char *dst, int dlen, char *file, char *deffile) {
  char *statedir = _options.statedir ? _options.statedir : DEFSTATEDIR;
  if (!file && deffile) {
    safe_snprintf(dst, dlen, "%s/%s", statedir, deffile);
  } else if (file) {
    if (file[0]=='/')
      safe_snprintf(dst, dlen, "%s", file);
    else
      safe_snprintf(dst, dlen, "%s/%s", statedir, file);
  }
  return 0;
}

int bblk_fromfd(bstring s, int fd, int len) {
  int blen = len > 0 ? len : 128;
  int rd, rlen=0;
  while (1) {
    ballocmin(s, s->slen + blen);
    rd = safe_read(fd, s->data + s->slen, blen);
    if (rd <= 0) break;
    s->slen += rd;
    rlen += rd;
    if (len > 0 && rlen == len) break;
  }
  return rlen;
}

int bstring_fromfd(bstring s, int fd) {
  return bblk_fromfd(s, fd, -1);
}

inline void copy_mac6(uint8_t *dst, uint8_t *src) {
  dst[0]=src[0]; dst[1]=src[1];
  dst[2]=src[2]; dst[3]=src[3];
  dst[4]=src[4]; dst[5]=src[5];
}

/* Extract domain name and port from URL */
int get_urlparts(char *src, char *host, int hostsize, int *port, int *uripos) {
  char *slashslash = NULL;
  char *slash = NULL;
  char *colon = NULL;
  int hostlen;
  
  *port = 0;

  if (!memcmp(src, "http://", 7)) {
    *port = DHCP_HTTP;
    slashslash = src + 7;
  }
  else if (!memcmp(src, "https://", 8)) {
    *port = DHCP_HTTPS;
    slashslash = src + 8;
  }
  else {
    log_err(0, "URL must start with http:// or https:// [%s]!", src);
    return -1;
  }
  
  slash = strstr(slashslash, "/");
  colon = strstr(slashslash, ":");
  
  if (slash != NULL && colon != NULL && slash < colon) {
    /* .../...: */
    hostlen = slash - slashslash;
  }
  else if ((slash != NULL) && (colon == NULL)) {
    /* .../... */
    hostlen = slash - slashslash;
  }
  else if (colon != NULL) {
    /* ...:port/... */
    hostlen = colon - slashslash;
    if (1 != sscanf(colon+1, "%d", port)) {
      log_err(0, "Not able to parse URL port: %s!", src);
      return -1;
    }
  }
  else {
    hostlen = strlen(slashslash);
  }

  if (hostlen > (hostsize-1)) {
    log_err(0, "URL hostname larger than %d: %s!", hostsize-1, src);
    return -1;
  }

  safe_strncpy(host, slashslash, hostsize);
  host[hostlen] = 0;

  if (uripos) {
    *uripos = slash ? (slash - src) : strlen(src);
  }

  return 0;
}


/* This file is free software; you can redistribute it and/or modify */
/* it under the terms of the GNU General Public License as published by */
/* the Free Software Foundation; either version 2, or (at your option) */
/* any later version. */

/* This file is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/* GNU General Public License for more details. */

/* You should have received a copy of the GNU General Public License */
/* along with GNU Emacs; see the file COPYING.  If not, write to */
/* the Free Software Foundation, Inc., 59 Temple Place - Suite 330, */
/* Boston, MA 02111-1307, USA. */

/* Copyright (C) 2004 Ian Zimmerman */

/* $Id: getline.c,v 1.3 2004/05/18 22:45:18 summerisle Exp $ */

#ifndef HAVE_GETLINE
#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>

#define GETLINE_BUFSIZE 4096

ssize_t
getline (char** lineptr, size_t* n, FILE* stream) {
  char* lptr1;
  size_t nn;
  int c;

  if (*lineptr == NULL && n == NULL)
    {
      lptr1 = malloc (GETLINE_BUFSIZE);
      if (lptr1 == NULL) return EOF;
      nn = GETLINE_BUFSIZE;
    }
  else
    {
      lptr1 = *lineptr;
      nn = *n;
    }
  c = fgetc (stream);
  if (c == EOF) return EOF;
  {
    size_t offset;

    offset = 0;
    while (c != EOF)
      {
        if (offset >= nn - 1)
          {
            char* lptr2;
            lptr2 = realloc (lptr1, 2 * nn);
            if (lptr2 == NULL) return EOF;
            lptr1 = lptr2;
            nn *= 2;
          }
        lptr1[offset++] = (char)c;
        if (c == '\n') break;
        c = fgetc (stream);
      }
    lptr1[offset] = '\0';
    *lineptr = lptr1;
    *n = nn;
    return offset;
  }  
}
#endif
