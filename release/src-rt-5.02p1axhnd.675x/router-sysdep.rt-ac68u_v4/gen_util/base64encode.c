/* ***** BEGIN LICENSE BLOCK *****
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * ***** END LICENSE BLOCK ***** */

/*
 * This code is based on nssb64e.c from Mozilla.org, which allows
 * the code to be licensed under MPL/GPL/or LGPL.  We will license it
 * under LGPL  --mwang 8/22/07
 */

 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include <assert.h>
 #include "genutil_base64.h"


struct PLBase64EncodeStateStr {
    unsigned chunks;
    unsigned saved;
    unsigned char buf[3];
};


/*
 * The following implementation of base64 encoding was based on code
 * found in libmime (specifically, in mimeenc.c).  It has been adapted to
 * use PR types and naming as well as to provide other necessary semantics
 * (like buffer-in/buffer-out in addition to "streaming" without undue
 * performance hit of extra copying if you made the buffer versions
 * use the output_fn).  It also incorporates some aspects of the current
 * NSPR base64 encoding code.  As such, you may find similarities to
 * both of those implementations.  I tried to use names that reflected
 * the original code when possible.  For this reason you may find some
 * inconsistencies -- libmime used lots of "in" and "out" whereas the
 * NSPR version uses "src" and "dest"; sometimes I changed one to the other
 * and sometimes I left them when I thought the subroutines were at least
 * self-consistent.
 */


/*
 * Opaque object used by the encoder to store state.
 */
typedef struct PLBase64EncoderStr {
    /*
     * The one or two bytes pending.  (We need 3 to create a "token",
     * and hold the leftovers here.  in_buffer_count is *only* ever
     * 0, 1, or 2.
     */
    unsigned char in_buffer[2];
    int in_buffer_count;

    /*
     * If the caller wants linebreaks added, line_length specifies
     * where they come out.  It must be a multiple of 4; if the caller
     * provides one that isn't, we round it down to the nearest
     * multiple of 4.
     *
     * The value of current_column counts how many characters have been
     * added since the last linebreaks (or since the beginning, on the
     * first line).  It is also always a multiple of 4; it is unused when
     * line_length is 0.
     */
    UINT32 line_length;
    UINT32 current_column;

    /*
     * Where the encoded output goes -- either temporarily (in the streaming
     * case, staged here before it goes to the output function) or what will
     * be the entire buffered result for users of the buffer version.
     */
    char *output_buffer;
    UINT32 output_buflen;	/* the total length of allocated buffer */
    UINT32 output_length;	/* the length that is currently populated */
} PLBase64Encoder;



/*
 * Table to convert a binary value to its corresponding ascii "code".
 */
static unsigned char base64_valuetocode[64] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

#define B64_PAD	'='
#define B64_CR	'\r'
#define B64_LF	'\n'

static SINT32
pl_base64_encode_buffer(PLBase64Encoder *data, const unsigned char *in, UINT32 size)
{
   const unsigned char *end;
   char *out = data->output_buffer + data->output_length;
   UINT32 i = data->in_buffer_count;
   UINT32 n = 0;
   SINT32 off;
   UINT32 output_threshold;

   /* If this input buffer is too small, wait until next time. */
   if (size < (3 - i))
   {
      data->in_buffer[i++] = in[0];
      if (size > 1)
      {
         data->in_buffer[i++] = in[1];
      }
      assert(i < 3);
      data->in_buffer_count = i;
      return B64RET_SUCCESS;
   }

   /* If there are bytes that were put back last time, take them now. */
   if (i > 0)
   {
      n = data->in_buffer[0];
      if (i > 1)
      {
         n = (n << 8) | data->in_buffer[1];
      }
      data->in_buffer_count = 0;
   }

   /* If our total is not a multiple of three, put one or two bytes back. */
   off = (size + i) % 3;
   if (off > 0)
   {
      size -= off;
      data->in_buffer_count = off;

      data->in_buffer[0] = in[size];
      if (off > 1)
      {
         data->in_buffer[1] = in[size + 1];
      }
   }

   // calculate end pointer after size adjustment.  --mwang
   end = in + size;

   output_threshold = data->output_buflen - 3;

   /*
    * Populate the output buffer with base64 data, one line (or buffer)
    * at a time.
    */
   while (in < end)
   {
      SINT32 j, k;

      while (i < 3)
      {
         n = (n << 8) | *in++;
         i++;
      }
      i = 0;


      if (data->line_length > 0)
      {
         if (data->current_column >= data->line_length)
         {
            data->current_column = 0;
            *out++ = B64_CR;
            *out++ = B64_LF;
            data->output_length += 2;
         }
         data->current_column += 4;	/* the bytes we are about to add */
      }

      for (j = 18; j >= 0; j -= 6)
      {
         k = (n >> j) & 0x3F;
         *out++ = base64_valuetocode[k];
      }
      n = 0;
      data->output_length += 4;

      if (data->output_length >= output_threshold)
      {
         assert(data->output_length <= data->output_buflen);

         /*
          * Check that we are about to exit the loop.  (Since we
          * are over the threshold, there isn't enough room in the
          * output buffer for another trip around.)
          */
         assert(in == end);
         if (in < end)
         {
            return B64RET_INTERNAL_ERROR;
         }
      }
   }

   return B64RET_SUCCESS;
}


static SINT32 pl_base64_encode_flush (PLBase64Encoder *data)
{
   SINT32 i = data->in_buffer_count;

   if (i == 0 && data->output_length == 0)
   {
      return B64RET_SUCCESS;
   }

   if (i > 0)
   {
      char *out = data->output_buffer + data->output_length;
      UINT32 n;
      SINT32 j, k;

      n = ((UINT32) data->in_buffer[0]) << 16;
      if (i > 1)
      {
         n |= ((UINT32) data->in_buffer[1] << 8);
      }

      data->in_buffer_count = 0;

      if (data->line_length > 0)
      {
         if (data->current_column >= data->line_length)
         {
            data->current_column = 0;
            *out++ = B64_CR;
            *out++ = B64_LF;
            data->output_length += 2;
         }
      }

      /*
       * This will fill in more than we really have data for, but the
       * valid parts will end up in the correct position and the extras
       * will be over-written with pad characters below.
       */
      for (j = 18; j >= 0; j -= 6)
      {
         k = (n >> j) & 0x3F;
         *out++ = base64_valuetocode[k];
      }
      data->output_length += 4;

      /* Pad with equal-signs. */
      if (i == 1)
      {
         out[-2] = B64_PAD;
         out[-1] = B64_PAD;
      }
      else if (i == 2)
      {
         out[-1] = B64_PAD;
      }
   }

   return B64RET_SUCCESS;
}


/*
 * The maximum space needed to hold the output of the encoder given input
 * data of length "size", and allowing for CRLF added at least every
 * line_length bytes (we will add it at nearest lower multiple of 4).
 * There is no trailing CRLF.
 */
static UINT32 PL_Base64MaxEncodedLength (UINT32 size, UINT32 line_length)
{
    UINT32 tokens, tokens_per_line, full_lines, line_break_chars, remainder;

    tokens = (size + 2) / 3;

    if (line_length == 0)
    {
        return (tokens * 4 + 2); /* add 2 extra bytes for NULL terminator --mwang*/
    }

    if (line_length < 4)	/* too small! */
        line_length = 4;

    tokens_per_line = line_length / 4;
    full_lines = tokens / tokens_per_line;
    remainder = (tokens - (full_lines * tokens_per_line)) * 4;
    line_break_chars = full_lines * 2;
    if (remainder == 0)
        line_break_chars -= 2;

    return (full_lines * tokens_per_line * 4) + line_break_chars + remainder + 1; /* add 1 extra byte for NULL terminator --mwang */
}


UINT32 genUtl_b64EncodedBufferLength(UINT32 srclen)
{
   return PL_Base64MaxEncodedLength(srclen, 0);
}


/*
 * A distinct internal creation function for the buffer version to use.
 * (It does not want to specify an output_fn, and we want the normal
 * Create function to require that.)  All common initialization of the
 * encoding context should be done *here*.
 *
 * Save "line_length", rounded down to nearest multiple of 4 (if not
 * already even multiple).
 */
static PLBase64Encoder *
pl_base64_create_encoder (UINT32 line_length, char *output_buf, UINT32 output_buflen)
{
   PLBase64Encoder *data;
   UINT32 line_tokens;

   data = calloc(1, sizeof(PLBase64Encoder));
   if (data == NULL)
   {
      return NULL;
   }

   if (line_length > 0 && line_length < 4)	/* too small! */
   {
      line_length = 4;
   }

   line_tokens = line_length / 4;
   data->line_length = line_tokens * 4;

   data->output_buflen = output_buflen;
   data->output_buffer = output_buf;

   return data;
}


/*
 * When you're done encoding, call this to free the data.  If "abort_p"
 * is false, then calling this may cause the output_fn to be called
 * one last time (as the last buffered data is flushed out).
 */
static void
PL_DestroyBase64Encoder(PLBase64Encoder *data)
{
   /* don't free output_buffer.  That is the user's buffer.
    * just free the context buffer. */
   free(data);

   return;
}


SINT32 genUtl_b64EncodeMalloc(const unsigned char *src, UINT32 srclen,
                              char **b64StrBuf)
{
   UINT32 b64StrBufLen;
   SINT32 ret;

   if (src == NULL || srclen < 1 || b64StrBuf == NULL)
   {
      return B64RET_INVALID_ARGUMENTS;
   }

   b64StrBufLen = genUtl_b64EncodedBufferLength(srclen);
   *b64StrBuf = calloc(1, b64StrBufLen);
   if (*b64StrBuf == NULL)
   {
      return B64RET_RESOURCE_EXCEEDED;
   }

   ret = genUtl_b64Encode(src, srclen, *b64StrBuf, b64StrBufLen);

   if (ret != B64RET_SUCCESS)
   {
      free(*b64StrBuf);
      *b64StrBuf = NULL;
   }

   return ret;
}

/*
 * If linebreaks in the encoded output are desired, "line_length" specifies
 * where to place them -- it will be rounded down to the nearest multiple of 4
 * (if it is not already an even multiple of 4).  If it is zero, no linebreaks
 * will be added.  (FYI, a linebreak is CRLF -- two characters.)
 */
SINT32 genUtl_b64Encode(const unsigned char *src, UINT32 srclen,
                        char *b64StrBuf, UINT32 b64StrBufLen)
{
   UINT32 line_length=0;  /* leave line_length at 0.  Do not change it. miwang */
   PLBase64Encoder *data = NULL;
   SINT32 ret;

   if (src == NULL || srclen < 1 || b64StrBuf == NULL)
   {
      return B64RET_INVALID_ARGUMENTS;
   }

   if (b64StrBufLen < genUtl_b64EncodedBufferLength(srclen))
   {
      return B64RET_INVALID_ARGUMENTS;
   }

   /* make sure end of buffer is null terminated.  hold back 1 byte from the
    * real encoder.  -- mwang 10/20/18.
    */
   memset(b64StrBuf, 0, b64StrBufLen);
   if ((data = pl_base64_create_encoder(line_length, b64StrBuf, b64StrBufLen-1)) == NULL)
   {
      return B64RET_RESOURCE_EXCEEDED;
   }

   ret = pl_base64_encode_buffer(data, src, srclen);

   /*
    * We do not wait for Destroy to flush, because Destroy will also
    * get rid of our encoder context, which we need to look at first!
    */
   if (ret == B64RET_SUCCESS)
   {
      ret = pl_base64_encode_flush(data);
   }

   assert(data->output_length < data->output_buflen);

   PL_DestroyBase64Encoder(data);

   return ret;
}
