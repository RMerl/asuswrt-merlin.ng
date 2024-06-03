/* Glue between u-boot and upstream zlib */
#ifndef __GLUE_ZLIB_H__
#define __GLUE_ZLIB_H__

#include <common.h>
#include <linux/compiler.h>
#include <asm/unaligned.h>
#include <watchdog.h>
#include "u-boot/zlib.h"

/* avoid conflicts */
#undef OFF
#undef ASMINF
#undef POSTINC
#undef NO_GZIP
#define GUNZIP
#undef STDC
#undef NO_ERRNO_H

#endif
