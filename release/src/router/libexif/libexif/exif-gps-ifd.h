/*! \file exif-gps-ifd.h
 *  \brief Info about GPS tags
 */
/*
 * Copyright (c) 2020 Heiko Lewin <hlewin@gmx.de>
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

#ifndef LIBEXIF_EXIF_GPS_IFD_H
#define LIBEXIF_EXIF_GPS_IFD_H

#include <stdint.h>
#include <libexif/exif-format.h>
#include <libexif/exif-tag.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct ExifGPSIfdTagInfo {

  /* The ExifTag this entry describes */
  uint16_t tag;

  /* The format of the tag (following the spec) */
  ExifFormat format;

  /* The expected number of components. Note for some ASCII values without a default this is indicated as 0 */
  uint16_t components;

  /* The size (in bytes) of the raw default value (or 0) */
  uint16_t default_size;

  /* A pointer to the default value. Using char* works here as there are only defaults for BYTE and ASCII values */
  const char *default_value;

} ExifGPSIfdTagInfo;


const ExifGPSIfdTagInfo* exif_get_gps_tag_info(ExifTag tag);


#ifdef __cplusplus
}
#endif /* __cplusplus */



#endif /* !defined(LIBEXIF_EXIF_GPS_IFD_H) */
