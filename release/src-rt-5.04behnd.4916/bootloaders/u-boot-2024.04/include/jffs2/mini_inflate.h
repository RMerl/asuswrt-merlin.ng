/* SPDX-License-Identifier: GPL-2.0+ */
/*-------------------------------------------------------------------------
 * Filename:      mini_inflate.h
 * Version:       $Id: mini_inflate.h,v 1.2 2002/01/17 00:53:20 nyet Exp $
 * Copyright:     Copyright (C) 2001, Russ Dill
 * Author:        Russ Dill <Russ.Dill@asu.edu>
 * Description:   Mini deflate implementation
 *-----------------------------------------------------------------------*/

typedef __SIZE_TYPE__ size;

#define NO_ERROR 0
#define COMP_UNKNOWN 1	 /* The specififed bytype is invalid */
#define CODE_NOT_FOUND 2 /* a huffman code in the stream could not be decoded */
#define TOO_MANY_BITS 3	 /* pull_bits was passed an argument that is too
			  * large */

/* This struct represents an entire huffman code set. It has various lookup
 * tables to speed decoding */
struct huffman_set {
	int bits;	 /* maximum bit length */
	int num_symbols; /* Number of symbols this code can represent */
	int *lengths;	 /* The bit length of symbols */
	int *symbols;	 /* All of the symbols, sorted by the huffman code */
	int *count;	 /* the number of codes of this bit length */
	int *first;	 /* the first code of this bit length */
	int *pos;	 /* the symbol that first represents (in the symbols
			  * array) */
};

struct bitstream {
	unsigned char *data; /* increments as we move from byte to byte */
	unsigned char bit;   /* 0 to 7 */
	void *(*memcpy)(void *, const void *, size);
	unsigned long decoded; /* The number of bytes decoded */
	int error;

	int  distance_count[16];
	int  distance_first[16];
	int  distance_pos[16];
	int  distance_lengths[32];
	int  distance_symbols[32];

	int  code_count[8];
	int  code_first[8];
	int  code_pos[8];
	int  code_lengths[19];
	int  code_symbols[19];

	int  length_count[16];
	int  length_first[16];
	int  length_pos[16];
	int  length_lengths[288];
	int  length_symbols[288];

	struct huffman_set codes;
	struct huffman_set lengths;
	struct huffman_set distance;
};

#define NO_COMP 0
#define FIXED_COMP 1
#define DYNAMIC_COMP 2

long decompress_block(unsigned char *dest, unsigned char *source,
		      void *(*inflate_memcpy)(void *dest, const void *src, size n));
