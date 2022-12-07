/* mnote-apple-entry.h
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

#ifndef LIBEXIF_MNOTE_APPLE_ENTRY_H
#define LIBEXIF_MNOTE_APPLE_ENTRY_H

#include <libexif/exif-byte-order.h>
#include <libexif/exif-format.h>
#include <libexif/apple/mnote-apple-tag.h>

typedef struct _MnoteAppleEntry MnoteAppleEntry;

struct _MnoteAppleEntry {
    MnoteAppleTag tag;
    ExifFormat format;
    unsigned long components;
    unsigned char *data;
    unsigned int size;
    ExifByteOrder order;
};

char *mnote_apple_entry_get_value(MnoteAppleEntry *, char *, unsigned int);

#endif /* !defined(LIBEXIF_MNOTE_APPLE_ENTRY_H) */
