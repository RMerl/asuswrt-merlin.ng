/* mnote-apple-tag.h
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

#ifndef LIBEXIF_MNOTE_APPLE_TAG_H
#define LIBEXIF_MNOTE_APPLE_TAG_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

enum _MnoteAppleTag {
    MNOTE_APPLE_TAG_RUNTIME		= 0x3,
    MNOTE_APPLE_TAG_ACCELERATION_VECTOR	= 0x9,
    MNOTE_APPLE_TAG_HDR			= 0xA,
    MNOTE_APPLE_TAG_BURST_UUID 		= 0xB,
    MNOTE_APPLE_TAG_MEDIA_GROUP_UUID	= 0x11,
    MNOTE_APPLE_TAG_IMAGE_UNIQUE_ID	= 0x15
};
typedef enum _MnoteAppleTag MnoteAppleTag;

const char *mnote_apple_tag_get_name(MnoteAppleTag);
const char *mnote_apple_tag_get_title(MnoteAppleTag);
const char *mnote_apple_tag_get_description(MnoteAppleTag);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !defined(LIBEXIF_MNOTE_APPLE_TAG_H) */
