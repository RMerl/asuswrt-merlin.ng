/*
 * SpeedHQ encoder
 * Copyright (c) 2000, 2001 Fabrice Bellard
 * Copyright (c) 2003 Alex Beregszaszi
 * Copyright (c) 2003-2004 Michael Niedermayer
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

/**
 * @file
 * SpeedHQ encoder.
 */

#ifndef AVCODEC_SPEEDHQENC_H
#define AVCODEC_SPEEDHQENC_H

#include <stdint.h>

#include "mjpeg.h"
#include "mjpegenc_common.h"
#include "mpegvideo.h"
#include "put_bits.h"

int  ff_speedhq_encode_init(MpegEncContext *s);
void ff_speedhq_encode_close(MpegEncContext *s);
void ff_speedhq_encode_mb(MpegEncContext *s, int16_t block[12][64]);

void ff_speedhq_encode_picture_header(MpegEncContext *s);
void ff_speedhq_end_slice(MpegEncContext *s);

int ff_speedhq_mb_y_order_to_mb(int mb_y_order, int mb_height, int *first_in_slice);

#endif /* AVCODEC_SPEEDHQENC_H */
