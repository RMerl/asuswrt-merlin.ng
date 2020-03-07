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


/*
 * The following implementation of base64 decoding was based on code
 * found in libmime (specifically, in mimeenc.c).  It has been adapted to
 * use PR types and naming as well as to provide other necessary semantics
 * (like buffer-in/buffer-out in addition to "streaming" without undue
 * performance hit of extra copying if you made the buffer versions
 * use the output_fn).  It also incorporates some aspects of the current
 * NSPR base64 decoding code.  As such, you may find similarities to
 * both of those implementations.  I tried to use names that reflected
 * the original code when possible.  For this reason you may find some
 * inconsistencies -- libmime used lots of "in" and "out" whereas the
 * NSPR version uses "src" and "dest"; sometimes I changed one to the other
 * and sometimes I left them when I thought the subroutines were at least
 * self-consistent.
 */


/*
 * Opaque object used by the decoder to store state.
 */
typedef struct PLBase64DecoderStr {
    /* Current token (or portion, if token_size < 4) being decoded. */
    unsigned char token[4];
    SINT32 token_size;

    /*
     * Where the decoded output goes -- this will
     * be the entire buffered result for users of the buffer version.
     */
    unsigned char *output_buffer;
    UINT32 output_buflen;	/* the total length of allocated buffer */
    UINT32 output_length;	/* the length that is currently populated */
} PLBase64Decoder;



/*
 * Table to convert an ascii "code" to its corresponding binary value.
 * For ease of use, the binary values in the table are the actual values
 * PLUS ONE.  This is so that the special value of zero can denote an
 * invalid mapping; that was much easier than trying to fill in the other
 * values with some value other than zero, and to check for it.
 * Just remember to SUBTRACT ONE when using the value retrieved.
 */
static unsigned char base64_codetovaluep1[256] = {
/*   0: */	  0,	  0,	  0,	  0,	  0,	  0,	  0,	  0,
/*   8: */	  0,	  0,	  0,	  0,	  0,	  0,	  0,	  0,
/*  16: */	  0,	  0,	  0,	  0,	  0,	  0,	  0,	  0,
/*  24: */	  0,	  0,	  0,	  0,	  0,	  0,	  0,	  0,
/*  32: */	  0,	  0,	  0,	  0,	  0,	  0,	  0,	  0,
/*  40: */	  0,	  0,	  0,	 63,	  0,	  0,	  0,	 64,
/*  48: */	 53,	 54,	 55,	 56,	 57,	 58,	 59,	 60,
/*  56: */	 61,	 62,	  0,	  0,	  0,	  0,	  0,	  0,
/*  64: */	  0,	  1,	  2,	  3,	  4,	  5,	  6,	  7,
/*  72: */	  8,	  9,	 10,	 11,	 12,	 13,	 14,	 15,
/*  80: */	 16,	 17,	 18,	 19,	 20,	 21,	 22,	 23,
/*  88: */	 24,	 25,	 26,	  0,	  0,	  0,	  0,	  0,
/*  96: */	  0,	 27,	 28,	 29,	 30,	 31,	 32,	 33,
/* 104: */	 34,	 35,	 36,	 37,	 38,	 39,	 40,	 41,
/* 112: */	 42,	 43,	 44,	 45,	 46,	 47,	 48,	 49,
/* 120: */	 50,	 51,	 52,	  0,	  0,	  0,	  0,	  0,
/* 128: */	  0,	  0,	  0,	  0,	  0,	  0,	  0,	  0
/* and rest are all zero as well */
};

#define B64_PAD	'='


/*
 * Reads 4; writes 3 (known, or expected, to have no trailing padding).
 * Returns bytes written; -1 on error (unexpected character).
 */
static int
pl_base64_decode_4to3 (const unsigned char *in, unsigned char *out)
{
    int j;
    SINT32 num = 0;
    unsigned char bits;

    for (j = 0; j < 4; j++) {
	bits = base64_codetovaluep1[in[j]];
	if (bits == 0)
	    return -1;
	num = (num << 6) | (bits - 1);
    }

    out[0] = (unsigned char) (num >> 16);
    out[1] = (unsigned char) ((num >> 8) & 0xFF);
    out[2] = (unsigned char) (num & 0xFF);

    return 3;
}

/*
 * Reads 3; writes 2 (caller already confirmed EOF or trailing padding).
 * Returns bytes written; -1 on error (unexpected character).
 */
static int
pl_base64_decode_3to2 (const unsigned char *in, unsigned char *out)
{
    SINT32 num = 0;
    unsigned char bits1, bits2, bits3;

    bits1 = base64_codetovaluep1[in[0]];
    bits2 = base64_codetovaluep1[in[1]];
    bits3 = base64_codetovaluep1[in[2]];

    if ((bits1 == 0) || (bits2 == 0) || (bits3 == 0))
	return -1;

    num = ((UINT32)(bits1 - 1)) << 10;
    num |= ((UINT32)(bits2 - 1)) << 4;
    num |= ((UINT32)(bits3 - 1)) >> 2;

    out[0] = (unsigned char) (num >> 8);
    out[1] = (unsigned char) (num & 0xFF);

    return 2;
}

/*
 * Reads 2; writes 1 (caller already confirmed EOF or trailing padding).
 * Returns bytes written; -1 on error (unexpected character).
 */
static int
pl_base64_decode_2to1 (const unsigned char *in, unsigned char *out)
{
    SINT32 num = 0;
    unsigned char bits1, bits2;

    bits1 = base64_codetovaluep1[in[0]];
    bits2 = base64_codetovaluep1[in[1]];

    if ((bits1 == 0) || (bits2 == 0))
	return -1;

    num = ((UINT32)(bits1 - 1)) << 2;
    num |= ((UINT32)(bits2 - 1)) >> 4;

    out[0] = (unsigned char) num;

    return 1;
}

/*
 * Reads 4; writes 0-3.  Returns bytes written or -1 on error.
 * (Writes less than 3 only at (presumed) EOF.)
 */
static int
pl_base64_decode_token (const unsigned char *in, unsigned char *out)
{
    if (in[3] != B64_PAD)
	return pl_base64_decode_4to3 (in, out);

    if (in[2] == B64_PAD)
	return pl_base64_decode_2to1 (in, out);

    return pl_base64_decode_3to2 (in, out);
}

static SINT32
pl_base64_decode_buffer (PLBase64Decoder *data, const unsigned char *in, UINT32 length)
{
   unsigned char *out = data->output_buffer;
   unsigned char *token = data->token;
   int i, n = 0;

   i = data->token_size;
   data->token_size = 0;

   while (length > 0)
   {
      while (i < 4 && length > 0)
      {
	      /*
	       * XXX Note that the following simply ignores any unexpected
	       * characters.  This is exactly what the original code in
	       * libmime did, and I am leaving it.  We certainly want to skip
	       * over whitespace (we must); this does much more than that.
	       * I am not confident changing it, and I don't want to slow
	       * the processing down doing more complicated checking, but
	       * someone else might have different ideas in the future.
	       */
          if (base64_codetovaluep1[*in] > 0 || *in == B64_PAD)
          {
		       token[i++] = *in;
          }
	       in++;
	       length--;
	    }

	    if (i < 4)
       {
	       /* Didn't get enough for a complete token. */
	       data->token_size = i;
	       break;
	    }
	    i = 0;

	    assert((out - data->output_buffer + 3) <= (SINT32) data->output_buflen);

      /*
	    * Assume we are not at the end; the following function only works
       * for an internal token (no trailing padding characters) but is
       * faster that way.  If it hits an invalid character (padding) it
       * will return an error; we break out of the loop and try again
       * calling the routine that will handle a final token.
       * Note that we intentionally do it this way rather than explicitly
       * add a check for padding here (because that would just slow down
       * the normal case) nor do we rely on checking whether we have more
       * input to process (because that would also slow it down but also
       * because we want to allow trailing garbage, especially white space
       * and cannot tell that without read-ahead, also a slow proposition).
       * Whew.  Understand?
       */
      n = pl_base64_decode_4to3 (token, out);
      if (n < 0)
      {
         break;
      }

      /* Advance "out" by the number of bytes just written to it. */
      out += n;
      n = 0;
   }

   /*
    * See big comment above, before call to pl_base64_decode_4to3.
    * Here we check if we error'd out of loop, and allow for the case
    * that we are processing the last interesting token.  If the routine
    * which should handle padding characters also fails, then we just
    * have bad input and give up.
    */
   if (n < 0)
   {
      n = pl_base64_decode_token (token, out);
      if (n < 0)
      {
         return B64RET_INTERNAL_ERROR;
      }

      out += n;
   }

   /*
    * As explained above, we can get here with more input remaining, but
    * it should be all characters we do not care about (i.e. would be
    * ignored when transferring from "in" to "token" in loop above,
    * except here we choose to ignore extraneous pad characters, too).
    * Swallow it, performing that check.  If we find more characters that
    * we would expect to decode, something is wrong.
    */
   while (length > 0)
   {
      if (base64_codetovaluep1[*in] > 0)
      {
         return B64RET_INTERNAL_ERROR;
      }
      in++;
      length--;
   }

   /* Record the length of decoded data we have left in output_buffer. */
   data->output_length = (UINT32) (out - data->output_buffer);
   return B64RET_SUCCESS;
}


/*
 * Flush any remaining buffered characters.  Given well-formed input,
 * this will have nothing to do.  If the input was missing the padding
 * characters at the end, though, there could be 1-3 characters left
 * behind -- we will tolerate that by adding the padding for them.
 */
static SINT32 pl_base64_decode_flush (PLBase64Decoder *data)
{
    int count;

    /*
     * If no remaining characters, or all are padding (also not well-formed
     * input, but again, be tolerant), then nothing more to do.  (And, that
     * is considered successful.)
     */
    if (data->token_size == 0 || data->token[0] == B64_PAD)
    {
	    return B64RET_SUCCESS;
    }

    /*
     * Assume we have all the interesting input except for some expected
     * padding characters.  Add them and decode the resulting token.
     */
    while (data->token_size < 4)
    {
	    data->token[data->token_size++] = B64_PAD;
    }

    data->token_size = 0;	/* so a subsequent flush call is a no-op */

    count = pl_base64_decode_token (data->token,
				    data->output_buffer + data->output_length);
    if (count < 0)
    {
	    return B64RET_INTERNAL_ERROR;
    }

    data->output_length += count;

    return B64RET_SUCCESS;
}


/*
 * The maximum space needed to hold the output of the decoder given
 * input data of length "size".
 */
UINT32 genUtl_b64DecodedBufferLength(UINT32 size)
{
    return ((size * 3) / 4);
}


/*
 * A distinct internal creation function for the buffer version to use.
 * (It does not want to specify an output_fn, and we want the normal
 * Create function to require that.)  If more common initialization
 * of the decoding context needs to be done, it should be done *here*.
 */
static PLBase64Decoder *
pl_base64_create_decoder (void)
{
    return ((PLBase64Decoder *) calloc(1, sizeof(PLBase64Decoder)));
}


/*
 * When you're done decoding, call this to free the data.
 */
static void PL_DestroyBase64Decoder (PLBase64Decoder *data)
{
   /* don't free output_buffer.  That is the user's buffer.
    * just free the context buffer. */
   free(data);

   return;
}


SINT32 genUtl_b64DecodeMalloc(const char *b64Str,
                           UINT8 **binaryBuf, UINT32 *binaryBufLen)
{
   UINT32 b64StrLen;
   SINT32 ret;

   if (b64Str == NULL || b64Str[0] == '\0' ||
       binaryBuf == NULL || binaryBufLen == NULL)
   {
      return B64RET_INVALID_ARGUMENTS;
   }

   b64StrLen = strlen(b64Str);
   *binaryBufLen = genUtl_b64DecodedBufferLength(b64StrLen);
   *binaryBuf = calloc(1, *binaryBufLen);
   if (*binaryBuf == NULL)
   {
      return B64RET_RESOURCE_EXCEEDED;
   }

   ret = genUtl_b64Decode(b64Str, *binaryBuf, binaryBufLen);

   if (ret != B64RET_SUCCESS)
   {
      free(*binaryBuf);
      *binaryBuf = NULL;
      *binaryBufLen = 0;
   }

   return ret;
}

SINT32 genUtl_b64Decode(const char *b64Str,
                         UINT8 *binaryBuf, UINT32 *binaryBufLen)
{
    PLBase64Decoder *data = NULL;
    UINT32 b64StrLen;
    SINT32 ret;

    if (b64Str == NULL || b64Str[0] == '\0')
    {
      return B64RET_INVALID_ARGUMENTS;
    }
    b64StrLen = strlen(b64Str);

    /*
     * Allocate the decoding structure.
     */
    if ((data = pl_base64_create_decoder()) == NULL)
    {
        return B64RET_INTERNAL_ERROR;
    }

    data->output_buflen = *binaryBufLen;
    data->output_buffer = binaryBuf;

    ret = pl_base64_decode_buffer(data, (const unsigned char *) b64Str, b64StrLen);


    /*
     * We do not wait for Destroy to flush, because Destroy will also
     * get rid of our decoder context, which we need to look at first!
     */
    if (ret == B64RET_SUCCESS)
    {
	    ret = pl_base64_decode_flush(data);
    }


    if (ret == B64RET_SUCCESS)
    {
       assert(data->output_length <= *binaryBufLen);
       *binaryBufLen = data->output_length;
    }

    PL_DestroyBase64Decoder(data);

    return ret;
}
