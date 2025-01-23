/* b64enc.c - Simple Base64 encoder.
 * Copyright (C) 2001, 2003, 2004, 2008, 2010,
 *               2011 Free Software Foundation, Inc.
 * Copyright (C) 2001, 2003, 2004, 2008, 2010,
 *               2011, 2018 g10 Code GmbH
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
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * This file was originally a part of GnuPG.
 */

#include <config.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "gpgrt-int.h"


#define B64ENC_DID_HEADER   1
#define B64ENC_DID_TRAILER  2
#define B64ENC_NO_LINEFEEDS 16
#define B64ENC_USE_PGPCRC   32

/* The base-64 character list */
static unsigned char const bintoasc[64] = ("ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                           "abcdefghijklmnopqrstuvwxyz"
                                           "0123456789+/");

/* Stuff required to create the OpenPGP CRC.  This crc_table has been
   created using this code:

   #include <stdio.h>
   #include <stdint.h>

   #define CRCPOLY 0x864CFB

   int
   main (void)
   {
     int i, j;
     uint32_t t;
     uint32_t crc_table[256];

     crc_table[0] = 0;
     for (i=j=0; j < 128; j++ )
       {
         t = crc_table[j];
         if ( (t & 0x00800000) )
           {
             t <<= 1;
             crc_table[i++] = t ^ CRCPOLY;
             crc_table[i++] = t;
       }
         else
           {
             t <<= 1;
             crc_table[i++] = t;
             crc_table[i++] = t ^ CRCPOLY;
           }
       }

     puts ("static const u32 crc_table[256] = {");
     for (i=j=0; i < 256; i++)
       {
         printf ("%s 0x%08lx", j? "":" ", (unsigned long)crc_table[i]);
         if (i != 255)
           {
             putchar (',');
             if ( ++j > 5)
               {
                 j = 0;
                 putchar ('\n');
               }
           }
       }
     puts ("\n};");
     return 0;
   }
*/
#define CRCINIT 0xB704CE
static const uint32_t crc_table[256] = {
  0x00000000, 0x00864cfb, 0x018ad50d, 0x010c99f6, 0x0393e6e1, 0x0315aa1a,
  0x021933ec, 0x029f7f17, 0x07a18139, 0x0727cdc2, 0x062b5434, 0x06ad18cf,
  0x043267d8, 0x04b42b23, 0x05b8b2d5, 0x053efe2e, 0x0fc54e89, 0x0f430272,
  0x0e4f9b84, 0x0ec9d77f, 0x0c56a868, 0x0cd0e493, 0x0ddc7d65, 0x0d5a319e,
  0x0864cfb0, 0x08e2834b, 0x09ee1abd, 0x09685646, 0x0bf72951, 0x0b7165aa,
  0x0a7dfc5c, 0x0afbb0a7, 0x1f0cd1e9, 0x1f8a9d12, 0x1e8604e4, 0x1e00481f,
  0x1c9f3708, 0x1c197bf3, 0x1d15e205, 0x1d93aefe, 0x18ad50d0, 0x182b1c2b,
  0x192785dd, 0x19a1c926, 0x1b3eb631, 0x1bb8faca, 0x1ab4633c, 0x1a322fc7,
  0x10c99f60, 0x104fd39b, 0x11434a6d, 0x11c50696, 0x135a7981, 0x13dc357a,
  0x12d0ac8c, 0x1256e077, 0x17681e59, 0x17ee52a2, 0x16e2cb54, 0x166487af,
  0x14fbf8b8, 0x147db443, 0x15712db5, 0x15f7614e, 0x3e19a3d2, 0x3e9fef29,
  0x3f9376df, 0x3f153a24, 0x3d8a4533, 0x3d0c09c8, 0x3c00903e, 0x3c86dcc5,
  0x39b822eb, 0x393e6e10, 0x3832f7e6, 0x38b4bb1d, 0x3a2bc40a, 0x3aad88f1,
  0x3ba11107, 0x3b275dfc, 0x31dced5b, 0x315aa1a0, 0x30563856, 0x30d074ad,
  0x324f0bba, 0x32c94741, 0x33c5deb7, 0x3343924c, 0x367d6c62, 0x36fb2099,
  0x37f7b96f, 0x3771f594, 0x35ee8a83, 0x3568c678, 0x34645f8e, 0x34e21375,
  0x2115723b, 0x21933ec0, 0x209fa736, 0x2019ebcd, 0x228694da, 0x2200d821,
  0x230c41d7, 0x238a0d2c, 0x26b4f302, 0x2632bff9, 0x273e260f, 0x27b86af4,
  0x252715e3, 0x25a15918, 0x24adc0ee, 0x242b8c15, 0x2ed03cb2, 0x2e567049,
  0x2f5ae9bf, 0x2fdca544, 0x2d43da53, 0x2dc596a8, 0x2cc90f5e, 0x2c4f43a5,
  0x2971bd8b, 0x29f7f170, 0x28fb6886, 0x287d247d, 0x2ae25b6a, 0x2a641791,
  0x2b688e67, 0x2beec29c, 0x7c3347a4, 0x7cb50b5f, 0x7db992a9, 0x7d3fde52,
  0x7fa0a145, 0x7f26edbe, 0x7e2a7448, 0x7eac38b3, 0x7b92c69d, 0x7b148a66,
  0x7a181390, 0x7a9e5f6b, 0x7801207c, 0x78876c87, 0x798bf571, 0x790db98a,
  0x73f6092d, 0x737045d6, 0x727cdc20, 0x72fa90db, 0x7065efcc, 0x70e3a337,
  0x71ef3ac1, 0x7169763a, 0x74578814, 0x74d1c4ef, 0x75dd5d19, 0x755b11e2,
  0x77c46ef5, 0x7742220e, 0x764ebbf8, 0x76c8f703, 0x633f964d, 0x63b9dab6,
  0x62b54340, 0x62330fbb, 0x60ac70ac, 0x602a3c57, 0x6126a5a1, 0x61a0e95a,
  0x649e1774, 0x64185b8f, 0x6514c279, 0x65928e82, 0x670df195, 0x678bbd6e,
  0x66872498, 0x66016863, 0x6cfad8c4, 0x6c7c943f, 0x6d700dc9, 0x6df64132,
  0x6f693e25, 0x6fef72de, 0x6ee3eb28, 0x6e65a7d3, 0x6b5b59fd, 0x6bdd1506,
  0x6ad18cf0, 0x6a57c00b, 0x68c8bf1c, 0x684ef3e7, 0x69426a11, 0x69c426ea,
  0x422ae476, 0x42aca88d, 0x43a0317b, 0x43267d80, 0x41b90297, 0x413f4e6c,
  0x4033d79a, 0x40b59b61, 0x458b654f, 0x450d29b4, 0x4401b042, 0x4487fcb9,
  0x461883ae, 0x469ecf55, 0x479256a3, 0x47141a58, 0x4defaaff, 0x4d69e604,
  0x4c657ff2, 0x4ce33309, 0x4e7c4c1e, 0x4efa00e5, 0x4ff69913, 0x4f70d5e8,
  0x4a4e2bc6, 0x4ac8673d, 0x4bc4fecb, 0x4b42b230, 0x49ddcd27, 0x495b81dc,
  0x4857182a, 0x48d154d1, 0x5d26359f, 0x5da07964, 0x5cace092, 0x5c2aac69,
  0x5eb5d37e, 0x5e339f85, 0x5f3f0673, 0x5fb94a88, 0x5a87b4a6, 0x5a01f85d,
  0x5b0d61ab, 0x5b8b2d50, 0x59145247, 0x59921ebc, 0x589e874a, 0x5818cbb1,
  0x52e37b16, 0x526537ed, 0x5369ae1b, 0x53efe2e0, 0x51709df7, 0x51f6d10c,
  0x50fa48fa, 0x507c0401, 0x5542fa2f, 0x55c4b6d4, 0x54c82f22, 0x544e63d9,
  0x56d11cce, 0x56575035, 0x575bc9c3, 0x57dd8538
};


/* Prepare for Base-64 writing to STREAM.  If TITLE is not NULL and
 * not an empty string, that string will be used as the title for the
 * armor lines, with TITLE being an empty string, we don't write the
 * header lines and furthermore even don't write any linefeeds.  If
 * TITLE starts with "PGP " the OpenPGP CRC checksum will be written
 * as well.  With TITLE being NULL, we merely don't write header but
 * make sure that lines are not too long.  Note, that we don't write
 * anything unless at least one byte is written using b64enc_write.
 * On success an enoder object is returned which needs to be released
 * using _gpgrt_b64dec_finish.  On error NULL is returned an ERRNO is
 * set.
 */
gpgrt_b64state_t
_gpgrt_b64enc_start (estream_t stream, const char *title)
{
  gpgrt_b64state_t state;

  state = xtrycalloc (1, sizeof *state);
  if (!state)
    return NULL;

  state->stream = stream;
  if (title && !*title)
    state->flags |= B64ENC_NO_LINEFEEDS;
  else if (title)
    {
      if (!strncmp (title, "PGP ", 4))
        {
          state->flags |= B64ENC_USE_PGPCRC;
          state->crc = CRCINIT;
        }
      state->title = xtrystrdup (title);
      if (!state->title)
        {
          xfree (state);
          return NULL;
        }
    }

  return state;
}


/* Write NBYTES from BUFFER to the Base 64 stream identified by STATE.
 * With BUFFER and NBYTES being 0, merely do a fflush on the stream.
 */
gpg_err_code_t
_gpgrt_b64enc_write (gpgrt_b64state_t state, const void *buffer, size_t nbytes)
{
  unsigned char radbuf[4];
  int idx, quad_count;
  const unsigned char *p;

  if (state->lasterr)
    return state->lasterr;

  if (!nbytes)
    {
      if (buffer)
        if (_gpgrt_fflush (state->stream))
          goto write_error;
      return 0;
    }

  if (!(state->flags & B64ENC_DID_HEADER))
    {
      if (state->title)
        {
          if ( _gpgrt_fputs ("-----BEGIN ", state->stream) == EOF
               || _gpgrt_fputs (state->title, state->stream) == EOF
               || _gpgrt_fputs ("-----\n", state->stream) == EOF)
            goto write_error;
          if ( (state->flags & B64ENC_USE_PGPCRC)
               && _gpgrt_fputs ("\n", state->stream) == EOF)
            goto write_error;
        }

      state->flags |= B64ENC_DID_HEADER;
    }

  idx = state->idx;
  quad_count = state->quad_count;
  gpgrt_assert (idx < 4);
  memcpy (radbuf, state->radbuf, idx);

  if ( (state->flags & B64ENC_USE_PGPCRC) )
    {
      size_t n;
      uint32_t crc = state->crc;

      for (p=buffer, n=nbytes; n; p++, n-- )
        crc = ((uint32_t)crc << 8) ^ crc_table[((crc >> 16)&0xff) ^ *p];
      state->crc = (crc & 0x00ffffff);
    }

  for (p=buffer; nbytes; p++, nbytes--)
    {
      radbuf[idx++] = *p;
      if (idx > 2)
        {
          char tmp[4];

          tmp[0] = bintoasc[(*radbuf >> 2) & 077];
          tmp[1] = bintoasc[(((*radbuf<<4)&060)|((radbuf[1] >> 4)&017))&077];
          tmp[2] = bintoasc[(((radbuf[1]<<2)&074)|((radbuf[2]>>6)&03))&077];
          tmp[3] = bintoasc[radbuf[2]&077];
          for (idx=0; idx < 4; idx++)
            _gpgrt_fputc (tmp[idx], state->stream);
          idx = 0;
          if (_gpgrt_ferror (state->stream))
            goto write_error;

          if (++quad_count >= (64/4))
            {
              quad_count = 0;
              if (!(state->flags & B64ENC_NO_LINEFEEDS)
                  && _gpgrt_fputs ("\n", state->stream) == EOF)
                goto write_error;
            }
        }
    }
  memcpy (state->radbuf, radbuf, idx);
  state->idx = idx;
  state->quad_count = quad_count;
  return 0;

 write_error:
  state->lasterr = _gpg_err_code_from_syserror ();
  if (state->title)
    {
      xfree (state->title);
      state->title = NULL;
    }
  return state->lasterr;
}


gpg_err_code_t
_gpgrt_b64enc_finish (gpgrt_b64state_t state)
{
  gpg_err_code_t err = 0;
  unsigned char radbuf[4];
  int idx, quad_count;
  char tmp[4];

  if (!state)
    return 0;  /* Already released.  */

  if (state->using_decoder)
    {
      err = GPG_ERR_CONFLICT;  /* State was created for the decoder.  */
      goto cleanup;
    }

  if (state->lasterr)
    {
      err = state->lasterr;
      goto cleanup;
    }

  if (!(state->flags & B64ENC_DID_HEADER))
    goto cleanup;

  /* Flush the base64 encoding */
  idx = state->idx;
  quad_count = state->quad_count;
  gpgrt_assert (idx < 4);
  memcpy (radbuf, state->radbuf, idx);

  if (idx)
    {
      tmp[0] = bintoasc[(*radbuf>>2)&077];
      if (idx == 1)
        {
          tmp[1] = bintoasc[((*radbuf << 4) & 060) & 077];
          tmp[2] = '=';
          tmp[3] = '=';
        }
      else
        {
          tmp[1] = bintoasc[(((*radbuf<<4)&060)|((radbuf[1]>>4)&017))&077];
          tmp[2] = bintoasc[((radbuf[1] << 2) & 074) & 077];
          tmp[3] = '=';
        }
      for (idx=0; idx < 4; idx++)
        _gpgrt_fputc (tmp[idx], state->stream);
      if (_gpgrt_ferror (state->stream))
        goto write_error;

      if (++quad_count >= (64/4))
        {
          quad_count = 0;
          if (!(state->flags & B64ENC_NO_LINEFEEDS)
              && _gpgrt_fputs ("\n", state->stream) == EOF)
            goto write_error;
        }
    }

  /* Finish the last line and write the trailer. */
  if (quad_count
      && !(state->flags & B64ENC_NO_LINEFEEDS)
      && _gpgrt_fputs ("\n", state->stream) == EOF)
    goto write_error;

  if ( (state->flags & B64ENC_USE_PGPCRC) )
    {
      /* Write the CRC.  */
      _gpgrt_fputs ("=", state->stream);
      radbuf[0] = state->crc >>16;
      radbuf[1] = state->crc >> 8;
      radbuf[2] = state->crc;
      tmp[0] = bintoasc[(*radbuf>>2)&077];
      tmp[1] = bintoasc[(((*radbuf<<4)&060)|((radbuf[1]>>4)&017))&077];
      tmp[2] = bintoasc[(((radbuf[1]<<2)&074)|((radbuf[2]>>6)&03))&077];
      tmp[3] = bintoasc[radbuf[2]&077];
      for (idx=0; idx < 4; idx++)
        _gpgrt_fputc (tmp[idx], state->stream);
      if (_gpgrt_ferror (state->stream))
        goto write_error;

      if (!(state->flags & B64ENC_NO_LINEFEEDS)
          && _gpgrt_fputs ("\n", state->stream) == EOF)
        goto write_error;
    }

  if (state->title)
    {
      if ( _gpgrt_fputs ("-----END ", state->stream) == EOF
           || _gpgrt_fputs (state->title, state->stream) == EOF
           || _gpgrt_fputs ("-----\n", state->stream) == EOF)
        goto write_error;
    }

 cleanup:
  xfree (state->title);
  xfree (state);
  return err;

 write_error:
  err = gpg_error_from_syserror ();
  goto cleanup;
}
