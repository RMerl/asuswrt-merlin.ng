/* mnote-apple-data.h
 *
 * Copyright (c) 2018 zhanwang-sky <zhanwang_sky@163.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA  02110-1301  USA.
 */

#ifndef LIBEXIF_EXIF_MNOTE_DATA_APPLE_H
#define LIBEXIF_EXIF_MNOTE_DATA_APPLE_H

#include <libexif/exif-byte-order.h>
#include <libexif/exif-data.h>
#include <libexif/exif-mem.h>
#include <libexif/exif-mnote-data-priv.h>
#include <libexif/apple/mnote-apple-entry.h>

typedef struct _ExifMnoteDataApple ExifMnoteDataApple;

struct _ExifMnoteDataApple {
    ExifMnoteData parent;
    ExifByteOrder order;
    unsigned int offset;
    MnoteAppleEntry *entries;
    unsigned int count;
};

int exif_mnote_data_apple_identify(const ExifData *, const ExifEntry *);

ExifMnoteData *exif_mnote_data_apple_new(ExifMem *);

#endif /* !defined(LIBEXIF_EXIF_MNOTE_DATA_APPLE_H) */
