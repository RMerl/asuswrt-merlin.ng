// SPDX-License-Identifier: GPL-2.0+
/*-------------------------------------------------------------------------
 * Filename:      mini_inflate.c
 * Version:       $Id: mini_inflate.c,v 1.3 2002/01/24 22:58:42 rfeany Exp $
 * Copyright:     Copyright (C) 2001, Russ Dill
 * Author:        Russ Dill <Russ.Dill@asu.edu>
 * Description:   Mini inflate implementation (RFC 1951)
 *-----------------------------------------------------------------------*/

#include <config.h>
#include <jffs2/mini_inflate.h>

/* The order that the code lengths in section 3.2.7 are in */
static unsigned char huffman_order[] = {16, 17, 18,  0,  8,  7,  9,  6, 10,  5,
					11,  4, 12,  3, 13,  2, 14,  1, 15};

static inline void cramfs_memset(int *s, const int c, size n)
{
	n--;
	for (;n > 0; n--) s[n] = c;
	s[0] = c;
}

/* associate a stream with a block of data and reset the stream */
static void init_stream(struct bitstream *stream, unsigned char *data,
			void *(*inflate_memcpy)(void *, const void *, size))
{
	stream->error = NO_ERROR;
	stream->memcpy = inflate_memcpy;
	stream->decoded = 0;
	stream->data = data;
	stream->bit = 0;	/* The first bit of the stream is the lsb of the
				 * first byte */

	/* really sorry about all this initialization, think of a better way,
	 * let me know and it will get cleaned up */
	stream->codes.bits = 8;
	stream->codes.num_symbols = 19;
	stream->codes.lengths = stream->code_lengths;
	stream->codes.symbols = stream->code_symbols;
	stream->codes.count = stream->code_count;
	stream->codes.first = stream->code_first;
	stream->codes.pos = stream->code_pos;

	stream->lengths.bits = 16;
	stream->lengths.num_symbols = 288;
	stream->lengths.lengths = stream->length_lengths;
	stream->lengths.symbols = stream->length_symbols;
	stream->lengths.count = stream->length_count;
	stream->lengths.first = stream->length_first;
	stream->lengths.pos = stream->length_pos;

	stream->distance.bits = 16;
	stream->distance.num_symbols = 32;
	stream->distance.lengths = stream->distance_lengths;
	stream->distance.symbols = stream->distance_symbols;
	stream->distance.count = stream->distance_count;
	stream->distance.first = stream->distance_first;
	stream->distance.pos = stream->distance_pos;

}

/* pull 'bits' bits out of the stream. The last bit pulled it returned as the
 * msb. (section 3.1.1)
 */
static inline unsigned long pull_bits(struct bitstream *stream,
				      const unsigned int bits)
{
	unsigned long ret;
	int i;

	ret = 0;
	for (i = 0; i < bits; i++) {
		ret += ((*(stream->data) >> stream->bit) & 1) << i;

		/* if, before incrementing, we are on bit 7,
		 * go to the lsb of the next byte */
		if (stream->bit++ == 7) {
			stream->bit = 0;
			stream->data++;
		}
	}
	return ret;
}

static inline int pull_bit(struct bitstream *stream)
{
	int ret = ((*(stream->data) >> stream->bit) & 1);
	if (stream->bit++ == 7) {
		stream->bit = 0;
		stream->data++;
	}
	return ret;
}

/* discard bits up to the next whole byte */
static void discard_bits(struct bitstream *stream)
{
	if (stream->bit != 0) {
		stream->bit = 0;
		stream->data++;
	}
}

/* No decompression, the data is all literals (section 3.2.4) */
static void decompress_none(struct bitstream *stream, unsigned char *dest)
{
	unsigned int length;

	discard_bits(stream);
	length = *(stream->data++);
	length += *(stream->data++) << 8;
	pull_bits(stream, 16);	/* throw away the inverse of the size */

	stream->decoded += length;
	stream->memcpy(dest, stream->data, length);
	stream->data += length;
}

/* Read in a symbol from the stream (section 3.2.2) */
static int read_symbol(struct bitstream *stream, struct huffman_set *set)
{
	int bits = 0;
	int code = 0;
	while (!(set->count[bits] && code < set->first[bits] +
					     set->count[bits])) {
		code = (code << 1) + pull_bit(stream);
		if (++bits > set->bits) {
			/* error decoding (corrupted data?) */
			stream->error = CODE_NOT_FOUND;
			return -1;
		}
	}
	return set->symbols[set->pos[bits] + code - set->first[bits]];
}

/* decompress a stream of data encoded with the passed length and distance
 * huffman codes */
static void decompress_huffman(struct bitstream *stream, unsigned char *dest)
{
	struct huffman_set *lengths = &(stream->lengths);
	struct huffman_set *distance = &(stream->distance);

	int symbol, length, dist, i;

	do {
		if ((symbol = read_symbol(stream, lengths)) < 0) return;
		if (symbol < 256) {
			*(dest++) = symbol; /* symbol is a literal */
			stream->decoded++;
		} else if (symbol > 256) {
			/* Determine the length of the repitition
			 * (section 3.2.5) */
			if (symbol < 265) length = symbol - 254;
			else if (symbol == 285) length = 258;
			else {
				length = pull_bits(stream, (symbol - 261) >> 2);
				length += (4 << ((symbol - 261) >> 2)) + 3;
				length += ((symbol - 1) % 4) <<
					  ((symbol - 261) >> 2);
			}

			/* Determine how far back to go */
			if ((symbol = read_symbol(stream, distance)) < 0)
				return;
			if (symbol < 4) dist = symbol + 1;
			else {
				dist = pull_bits(stream, (symbol - 2) >> 1);
				dist += (2 << ((symbol - 2) >> 1)) + 1;
				dist += (symbol % 2) << ((symbol - 2) >> 1);
			}
			stream->decoded += length;
			for (i = 0; i < length; i++) {
				*dest = dest[-dist];
				dest++;
			}
		}
	} while (symbol != 256); /* 256 is the end of the data block */
}

/* Fill the lookup tables (section 3.2.2) */
static void fill_code_tables(struct huffman_set *set)
{
	int code = 0, i, length;

	/* fill in the first code of each bit length, and the pos pointer */
	set->pos[0] = 0;
	for (i = 1; i < set->bits; i++) {
		code = (code + set->count[i - 1]) << 1;
		set->first[i] = code;
		set->pos[i] = set->pos[i - 1] + set->count[i - 1];
	}

	/* Fill in the table of symbols in order of their huffman code */
	for (i = 0; i < set->num_symbols; i++) {
		if ((length = set->lengths[i]))
			set->symbols[set->pos[length]++] = i;
	}

	/* reset the pos pointer */
	for (i = 1; i < set->bits; i++) set->pos[i] -= set->count[i];
}

static void init_code_tables(struct huffman_set *set)
{
	cramfs_memset(set->lengths, 0, set->num_symbols);
	cramfs_memset(set->count, 0, set->bits);
	cramfs_memset(set->first, 0, set->bits);
}

/* read in the huffman codes for dynamic decoding (section 3.2.7) */
static void decompress_dynamic(struct bitstream *stream, unsigned char *dest)
{
	/* I tried my best to minimize the memory footprint here, while still
	 * keeping up performance. I really dislike the _lengths[] tables, but
	 * I see no way of eliminating them without a sizable performance
	 * impact. The first struct table keeps track of stats on each bit
	 * length. The _length table keeps a record of the bit length of each
	 * symbol. The _symbols table is for looking up symbols by the huffman
	 * code (the pos element points to the first place in the symbol table
	 * where that bit length occurs). I also hate the initization of these
	 * structs, if someone knows how to compact these, lemme know. */

	struct huffman_set *codes = &(stream->codes);
	struct huffman_set *lengths = &(stream->lengths);
	struct huffman_set *distance = &(stream->distance);

	int hlit = pull_bits(stream, 5) + 257;
	int hdist = pull_bits(stream, 5) + 1;
	int hclen = pull_bits(stream, 4) + 4;
	int length, curr_code, symbol, i, last_code;

	last_code = 0;

	init_code_tables(codes);
	init_code_tables(lengths);
	init_code_tables(distance);

	/* fill in the count of each bit length' as well as the lengths
	 * table */
	for (i = 0; i < hclen; i++) {
		length = pull_bits(stream, 3);
		codes->lengths[huffman_order[i]] = length;
		if (length) codes->count[length]++;

	}
	fill_code_tables(codes);

	/* Do the same for the length codes, being carefull of wrap through
	 * to the distance table */
	curr_code = 0;
	while (curr_code < hlit) {
		if ((symbol = read_symbol(stream, codes)) < 0) return;
		if (symbol == 0) {
			curr_code++;
			last_code = 0;
		} else if (symbol < 16) { /* Literal length */
			lengths->lengths[curr_code] =  last_code = symbol;
			lengths->count[symbol]++;
			curr_code++;
		} else if (symbol == 16) { /* repeat the last symbol 3 - 6
					    * times */
			length = 3 + pull_bits(stream, 2);
			for (;length; length--, curr_code++)
				if (curr_code < hlit) {
					lengths->lengths[curr_code] =
						last_code;
					lengths->count[last_code]++;
				} else { /* wrap to the distance table */
					distance->lengths[curr_code - hlit] =
						last_code;
					distance->count[last_code]++;
				}
		} else if (symbol == 17) { /* repeat a bit length 0 */
			curr_code += 3 + pull_bits(stream, 3);
			last_code = 0;
		} else { /* same, but more times */
			curr_code += 11 + pull_bits(stream, 7);
			last_code = 0;
		}
	}
	fill_code_tables(lengths);

	/* Fill the distance table, don't need to worry about wrapthrough
	 * here */
	curr_code -= hlit;
	while (curr_code < hdist) {
		if ((symbol = read_symbol(stream, codes)) < 0) return;
		if (symbol == 0) {
			curr_code++;
			last_code = 0;
		} else if (symbol < 16) {
			distance->lengths[curr_code] = last_code = symbol;
			distance->count[symbol]++;
			curr_code++;
		} else if (symbol == 16) {
			length = 3 + pull_bits(stream, 2);
			for (;length; length--, curr_code++) {
				distance->lengths[curr_code] =
					last_code;
				distance->count[last_code]++;
			}
		} else if (symbol == 17) {
			curr_code += 3 + pull_bits(stream, 3);
			last_code = 0;
		} else {
			curr_code += 11 + pull_bits(stream, 7);
			last_code = 0;
		}
	}
	fill_code_tables(distance);

	decompress_huffman(stream, dest);
}

/* fill in the length and distance huffman codes for fixed encoding
 * (section 3.2.6) */
static void decompress_fixed(struct bitstream *stream, unsigned char *dest)
{
	/* let gcc fill in the initial values */
	struct huffman_set *lengths = &(stream->lengths);
	struct huffman_set *distance = &(stream->distance);

	cramfs_memset(lengths->count, 0, 16);
	cramfs_memset(lengths->first, 0, 16);
	cramfs_memset(lengths->lengths, 8, 144);
	cramfs_memset(lengths->lengths + 144, 9, 112);
	cramfs_memset(lengths->lengths + 256, 7, 24);
	cramfs_memset(lengths->lengths + 280, 8, 8);
	lengths->count[7] = 24;
	lengths->count[8] = 152;
	lengths->count[9] = 112;

	cramfs_memset(distance->count, 0, 16);
	cramfs_memset(distance->first, 0, 16);
	cramfs_memset(distance->lengths, 5, 32);
	distance->count[5] = 32;


	fill_code_tables(lengths);
	fill_code_tables(distance);


	decompress_huffman(stream, dest);
}

/* returns the number of bytes decoded, < 0 if there was an error. Note that
 * this function assumes that the block starts on a byte boundry
 * (non-compliant, but I don't see where this would happen). section 3.2.3 */
long decompress_block(unsigned char *dest, unsigned char *source,
		      void *(*inflate_memcpy)(void *, const void *, size))
{
	int bfinal, btype;
	struct bitstream stream;

	init_stream(&stream, source, inflate_memcpy);
	do {
		bfinal = pull_bit(&stream);
		btype = pull_bits(&stream, 2);
		if (btype == NO_COMP) decompress_none(&stream, dest + stream.decoded);
		else if (btype == DYNAMIC_COMP)
			decompress_dynamic(&stream, dest + stream.decoded);
		else if (btype == FIXED_COMP) decompress_fixed(&stream, dest + stream.decoded);
		else stream.error = COMP_UNKNOWN;
	} while (!bfinal && !stream.error);

#if 0
	putstr("decompress_block start\r\n");
	putLabeledWord("stream.error = ",stream.error);
	putLabeledWord("stream.decoded = ",stream.decoded);
	putLabeledWord("dest = ",dest);
	putstr("decompress_block end\r\n");
#endif
	return stream.error ? -stream.error : stream.decoded;
}
