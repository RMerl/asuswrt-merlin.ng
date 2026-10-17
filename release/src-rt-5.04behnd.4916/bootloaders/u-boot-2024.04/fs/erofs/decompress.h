/* SPDX-License-Identifier: GPL-2.0+ */
#ifndef __EROFS_DECOMPRESS_H
#define __EROFS_DECOMPRESS_H

#include "internal.h"

struct z_erofs_decompress_req {
	char *in, *out;

	/*
	 * initial decompressed bytes that need to be skipped
	 * when finally copying to output buffer
	 */
	unsigned int decodedskip;
	unsigned int inputsize, decodedlength;

	/* cut point of interlaced uncompressed data */
	unsigned int interlaced_offset;

	/* indicate the algorithm will be used for decompression */
	unsigned int alg;
	bool partial_decoding;
};

int z_erofs_decompress(struct z_erofs_decompress_req *rq);

#endif
