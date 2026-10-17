/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Usefuls routines based on the LzmaTest.c file from LZMA SDK 4.65
 *
 * Copyright (C) 2007-2008 Industrie Dial Face S.p.A.
 * Luigi 'Comio' Mantellini (luigi.mantellini@idf-hit.com)
 *
 * Copyright (C) 1999-2005 Igor Pavlov
 */

#ifndef __LZMA_TOOL_H__
#define __LZMA_TOOL_H__

#include <lzma/LzmaTypes.h>

/**
 * lzmaBuffToBuffDecompress() - Decompress LZMA data
 *
 * @outStream: output buffer
 * @uncompressedSize: On entry, the mnaximum uncompressed size of the data;
 *	on exit, the actual uncompressed size after processing
 * @inStream: Compressed bytes to decompress
 * @length: Sizeof @inStream
 * @return 0 if OK, SZ_ERROR_DATA if the data is in a format that cannot be
 *	decompressed; SZ_ERROR_OUTPUT_EOF if *uncompressedSize is too small;
 *	see also other SZ_ERROR... values
 */
int lzmaBuffToBuffDecompress(unsigned char *outStream, SizeT *uncompressedSize,
			     const unsigned char *inStream, SizeT length);

#endif
