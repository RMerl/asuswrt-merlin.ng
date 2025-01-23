/* b64dec.c - Simple Base64 decoder.
 * Copyright (C) 2008, 2011 Free Software Foundation, Inc.
 * Copyright (C) 2008, 2011, 2016 g10 Code GmbH
 *
 * This file is part of Libgpg-error.
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * This file is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * This file was originally a part of GnuPG.
 */

#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gpgrt-int.h"


/* The reverse base-64 list used for base-64 decoding. */
static unsigned char const asctobin[128] =
  {
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0x3e, 0xff, 0xff, 0xff, 0x3f,
    0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b,
    0x3c, 0x3d, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
    0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e,
    0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16,
    0x17, 0x18, 0x19, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20,
    0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28,
    0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 0x30,
    0x31, 0x32, 0x33, 0xff, 0xff, 0xff, 0xff, 0xff
  };

enum decoder_states
  {
    s_init, s_idle, s_lfseen, s_beginseen, s_waitheader, s_waitblank, s_begin,
    s_b64_0, s_b64_1, s_b64_2, s_b64_3,
    s_waitendtitle, s_waitend
  };



/* Allocate and initialize the context for the base64 decoder.  If
   TITLE is NULL a plain base64 decoding is done.  If it is the empty
   string the decoder will skip everything until a "-----BEGIN " line
   has been seen, decoding ends at a "----END " line.  */
gpgrt_b64state_t
_gpgrt_b64dec_start (const char *title)
{
  gpgrt_b64state_t state;
  char *t = NULL;

  if (title)
    {
      t = xtrystrdup (title);
      if (!t)
        return NULL;
    }

  state = xtrycalloc (1, sizeof (struct _gpgrt_b64state));
  if (!state)
    {
      xfree (t);
      return NULL;
    }

  if (t)
    {
      state->title = t;
      state->idx = s_init;
    }
  else
    state->idx = s_b64_0;

  state->using_decoder = 1;

  return state;
}


/* Do in-place decoding of base-64 data of LENGTH in BUFFER.  Stores the
   new length of the buffer at R_NBYTES. */
gpg_err_code_t
_gpgrt_b64dec_proc (gpgrt_b64state_t state, void *buffer, size_t length,
                    size_t *r_nbytes)
{
  enum decoder_states ds = state->idx;
  unsigned char val = state->radbuf[0];
  int pos = state->quad_count;
  char *d, *s;

  if (state->lasterr)
    return state->lasterr;

  if (state->stop_seen)
    {
      *r_nbytes = 0;
      state->lasterr = GPG_ERR_EOF;
      xfree (state->title);
      state->title = NULL;
      return state->lasterr;
    }

  for (s=d=buffer; length && !state->stop_seen; length--, s++)
    {
    again:
      switch (ds)
        {
        case s_idle:
          if (*s == '\n')
            {
              ds = s_lfseen;
              pos = 0;
            }
          break;
        case s_init:
          ds = s_lfseen;
          /* Fall through */
        case s_lfseen:
          if (*s != "-----BEGIN "[pos])
            {
              ds = s_idle;
              goto again;
            }
          else if (pos == 10)
            {
              pos = 0;
              ds = s_beginseen;
            }
          else
            pos++;
          break;
        case s_beginseen:
          if (*s != "PGP "[pos])
            ds = s_begin; /* Not a PGP armor.  */
          else if (pos == 3)
            ds = s_waitheader;
          else
            pos++;
          break;
        case s_waitheader:
          if (*s == '\n')
            ds = s_waitblank;
          break;
        case s_waitblank:
          if (*s == '\n')
            ds = s_b64_0; /* blank line found.  */
          else if (*s == ' ' || *s == '\r' || *s == '\t')
            ; /* Ignore spaces. */
          else
            {
              /* Armor header line.  Note that we don't care that our
               * FSM accepts a header prefixed with spaces.  */
              ds = s_waitheader; /* Wait for next header.  */
            }
          break;
        case s_begin:
          if (*s == '\n')
            ds = s_b64_0;
          break;
        case s_b64_0:
        case s_b64_1:
        case s_b64_2:
        case s_b64_3:
          {
            int c;

            if (*s == '-' && state->title)
              {
                /* Not a valid Base64 character: assume end
                   header.  */
                ds = s_waitend;
              }
            else if (*s == '=')
              {
                /* Pad character: stop */
                if (ds == s_b64_1)
                  *d++ = val;
                ds = state->title? s_waitendtitle : s_waitend;
              }
            else if (*s == '\n' || *s == ' ' || *s == '\r' || *s == '\t')
              ; /* Skip white spaces. */
            else if ( (*s & 0x80)
                      || (c = asctobin[*(unsigned char *)s]) == 255)
              {
                /* Skip invalid encodings.  */
                state->invalid_encoding = 1;
              }
            else if (ds == s_b64_0)
              {
                val = c << 2;
                ds = s_b64_1;
              }
            else if (ds == s_b64_1)
              {
                val |= (c>>4)&3;
                *d++ = val;
                val = (c<<4)&0xf0;
                ds = s_b64_2;
              }
            else if (ds == s_b64_2)
              {
                val |= (c>>2)&15;
                *d++ = val;
                val = (c<<6)&0xc0;
                ds = s_b64_3;
              }
            else
              {
                val |= c&0x3f;
                *d++ = val;
                ds = s_b64_0;
              }
          }
          break;
        case s_waitendtitle:
          if (*s == '-')
            ds = s_waitend;
          break;
        case s_waitend:
          if ( *s == '\n')
            state->stop_seen = 1;
          break;
        default:
          gpgrt_assert (!"invalid state");
        }
    }


  state->idx = ds;
  state->radbuf[0] = val;
  state->quad_count = pos;
  *r_nbytes = (d -(char*) buffer);
  return 0;
}


/* Return an error code in case an encoding error has been found
   during decoding. */
gpg_err_code_t
_gpgrt_b64dec_finish (gpgrt_b64state_t state)
{
  gpg_error_t err;

  if (!state)
    return 0;  /* Already released.  */

  if (!state->using_decoder)
    err = GPG_ERR_CONFLICT;  /* State was allocated for the encoder.  */
  else if (state->lasterr)
    err = state->lasterr;
  else
    {
      xfree (state->title);
      err = state->invalid_encoding? GPG_ERR_BAD_DATA : 0;
    }
  xfree (state);

  return err;
}
