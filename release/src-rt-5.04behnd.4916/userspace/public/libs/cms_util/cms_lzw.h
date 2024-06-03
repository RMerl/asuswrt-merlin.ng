/*
 * LZW decoder
 * Copyright (c) 2003 Fabrice Bellard.
 * Copyright (c) 2006 Konstantin Shishkov.
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef __CMS_LZW_H__
#define __CMS_LZW_H__

/*!\file cms_lzw.h
 * \brief Header file for LZW Encoder/Decoder functions.
 * 
 * These functions were taken from the ffmpeg library.
 */


#define LZW_MAXBITS        12
#define LZW_SIZTABLE       (1<<LZW_MAXBITS)
#define LZW_HASH_SIZE      16411
#define LZW_HASH_SHIFT     6

#define LZW_PREFIX_EMPTY   -1
#define LZW_PREFIX_FREE    -2


/** fixed length header in front of compressed config file */
#define COMPRESSED_CONFIG_HEADER_LENGTH 40

/** start of compressed config file */
#define COMPRESSED_CONFIG_HEADER        "<compressed alg=lzw len="


/** One code in hash table */
typedef struct Code{
    /// Hash code of prefix, LZW_PREFIX_EMPTY if empty prefix, or LZW_PREFIX_FREE if no code
    int hash_prefix;
    int code;               ///< LZW code
    uint8_t suffix;         ///< Last character in code block
}Code;


typedef struct PutBitContext {
    uint32_t bit_buf;
    int bit_left;
    uint8_t *buf, *buf_ptr, *buf_end;
} PutBitContext;


/** LZW encode state */
typedef struct {
    int clear_code;          ///< Value of clear code
    int end_code;            ///< Value of end code
    Code tab[LZW_HASH_SIZE]; ///< Hash table
    int tabsize;             ///< Number of values in hash table
    int bits;                ///< Actual bits code
    int bufsize;             ///< Size of output buffer
    PutBitContext pb;        ///< Put bit context for output
    int maxbits;             ///< Max bits code
    int maxcode;             ///< Max value of code
    int output_bytes;        ///< Number of written bytes
    int last_code;           ///< Value of last output code or LZW_PREFIX_EMPTY
}LZWEncoderState;


/** LZW decoder state structure */
typedef struct {
    uint8_t *pbuf, *ebuf;
    int bbits;
    unsigned int bbuf;

    int mode;                   ///< Decoder mode
    int cursize;                ///< The current code size
    int curmask;
    int codesize;
    int clear_code;
    int end_code;
    int newcodes;               ///< First available code
    int top_slot;               ///< Highest code for current size
    int extra_slot;
    int slot;                   ///< Last read code
    int fc, oc;
    uint8_t *sp;
    uint8_t stack[LZW_SIZTABLE];
    uint8_t suffix[LZW_SIZTABLE];
    uint16_t prefix[LZW_SIZTABLE];
    int bs;                     ///< current buffer size for GIF
} LZWDecoderState;


/** Create and Initialize a LZW encoder.
 *
 * @param s       (IN/OUT) LZW encoder state structure allocated by this function.
 * @param outbuf  (IN)  Output buffer, caller is responsible for allocating the buffer.
 * @param outsize (IN)  Size of output buffer
 *
 * @return CmsRet enum
 */
CmsRet cmsLzw_initEncoder(LZWEncoderState **s, UINT8 *outbuf, UINT32 outsize);


/** LZW main encode/compress function
 *
 * @param s      (IN) LZW encoder state
 * @param inbuf  (IN) Input buffer, contains data to be compressed
 * @param insize (IN) Size of input buffer
 * @return Number of bytes written or -1 on error
 */
SINT32 cmsLzw_encode(LZWEncoderState *s, const UINT8 *inbuf, UINT32 insize);


/** Write end code and flush bitstream
 *
 * @param s (IN) LZW encoder state
 * @return Number of bytes written or -1 on error.  Note the count here should
 *         be added to the count returned by cmsLzw_encode for the total number
 *         of encoded/compressed bytes.
 */
SINT32 cmsLzw_flushEncoder(LZWEncoderState *s);


/** Destroy the LZW encoder.
 *  Note the output buffer that was passed into the cmsLzw_initEncoder
 *  is not freed.  That buffer is the responsibility of the caller.
 *
 * @param s      (IN) LZW encoder state
 */
void cmsLzw_cleanupEncoder(LZWEncoderState **s);


/** Create and Initialize a LZW decoder
 *
 * @param s          (IN/OUT) LZW decoder context
 * @param inbuf      (IN) input compressed data
 * @param inbuf_size (IN) input data size
 *
 * @return CmsRet enum
 */
CmsRet cmsLzw_initDecoder(LZWDecoderState **s, UINT8 *inbuf, UINT32 inbuf_size);


/** Decode given number of bytes
 *
 * @param s       (IN) LZW decoder context
 * @param outbuf  (IN) output buffer, caller is responsible for allocating this buffer
 * @param outlen  (IN) number of bytes to decode, or the length of the output buffer.
 *                  The decompressor/decoder will stop if the end of the encoded
 *                  data is reached or if the end of the output buffer is reached.
 *
 * @return number of bytes decoded, or -1 on error.
 */
SINT32 cmsLzw_decode(LZWDecoderState *s, UINT8 *outbuf, UINT32 outlen);


/** Destroy the LZW Decoder.
 *
 * Note the input buffer that was passed into cmsLzw_initDecoder is not freed.
 * That buffer is the responsibility of the caller.
 */
void cmsLzw_cleanupDecoder(LZWDecoderState **s);



#endif /* __CMS_LZW_H__ */
