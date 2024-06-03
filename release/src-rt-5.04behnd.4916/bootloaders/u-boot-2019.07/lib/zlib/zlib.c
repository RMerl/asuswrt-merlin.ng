/*
 * This file is derived from various .h and .c files from the zlib-1.2.3
 * distribution by Jean-loup Gailly and Mark Adler, with some additions
 * by Paul Mackerras to aid in implementing Deflate compression and
 * decompression for PPP packets.  See zlib.h for conditions of
 * distribution and use.
 *
 * Changes that have been made include:
 * - changed functions not used outside this file to "local"
 * - added minCompression parameter to deflateInit2
 * - added Z_PACKET_FLUSH (see zlib.h for details)
 * - added inflateIncomp
 */

#include <common.h>

#ifdef CONFIG_GZIP_COMPRESSED
#define NO_DUMMY_DECL
#include "deflate.c"
#include "trees.c"
#endif

#include "zutil.h"
#include "inftrees.h"
#include "inflate.h"
#include "inffast.h"
#include "inffixed.h"
#include "inffast.c"
#include "inftrees.c"
#include "inflate.c"
#include "zutil.c"
#include "adler32.c"
