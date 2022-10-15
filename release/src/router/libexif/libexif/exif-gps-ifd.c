/*! \file exif-gps-ifd.c
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

#include <stddef.h>
#include "exif-gps-ifd.h"

static const struct ExifGPSIfdTagInfo exif_gps_ifd_tags[] = {

    {EXIF_TAG_GPS_VERSION_ID, EXIF_FORMAT_BYTE, 4, 4, "\x02\x02\x00\x00"},
    {EXIF_TAG_GPS_LATITUDE_REF, EXIF_FORMAT_ASCII, 0, 0, 0},
    {EXIF_TAG_GPS_LATITUDE, EXIF_FORMAT_RATIONAL, 3, 0, 0},
    {EXIF_TAG_GPS_LONGITUDE_REF, EXIF_FORMAT_ASCII, 0, 0, 0},
    {EXIF_TAG_GPS_LONGITUDE, EXIF_FORMAT_RATIONAL, 3, 0, 0},
    {EXIF_TAG_GPS_ALTITUDE_REF, EXIF_FORMAT_BYTE, 1, 1, "\x00"},
    {EXIF_TAG_GPS_ALTITUDE, EXIF_FORMAT_RATIONAL, 1, 0, 0},
    {EXIF_TAG_GPS_TIME_STAMP, EXIF_FORMAT_RATIONAL, 3, 0, 0},
    {EXIF_TAG_GPS_SATELLITES, EXIF_FORMAT_ASCII, 0, 0, 0},
    {EXIF_TAG_GPS_STATUS, EXIF_FORMAT_ASCII, 0, 0, 0},
    {EXIF_TAG_GPS_MEASURE_MODE, EXIF_FORMAT_ASCII, 0, 0, 0},
    {EXIF_TAG_GPS_DOP, EXIF_FORMAT_RATIONAL, 1, 0, 0},
    {EXIF_TAG_GPS_SPEED_REF, EXIF_FORMAT_ASCII, 2, 2, "K\x00"},
    {EXIF_TAG_GPS_SPEED, EXIF_FORMAT_RATIONAL, 1, 0, 0},
    {EXIF_TAG_GPS_TRACK_REF, EXIF_FORMAT_ASCII, 2, 2, "T\x00"},
    {EXIF_TAG_GPS_TRACK, EXIF_FORMAT_RATIONAL, 1, 0, 0},
    {EXIF_TAG_GPS_IMG_DIRECTION_REF, EXIF_FORMAT_ASCII, 2, 2, "T\x00"},
    {EXIF_TAG_GPS_IMG_DIRECTION, EXIF_FORMAT_RATIONAL, 1, 0, 0},
    {EXIF_TAG_GPS_MAP_DATUM, EXIF_FORMAT_ASCII, 0, 0, 0},
    {EXIF_TAG_GPS_DEST_LATITUDE_REF, EXIF_FORMAT_ASCII, 0, 0, 0},
    {EXIF_TAG_GPS_DEST_LATITUDE, EXIF_FORMAT_RATIONAL, 3, 0, 0},
    {EXIF_TAG_GPS_DEST_LONGITUDE_REF, EXIF_FORMAT_ASCII, 0, 0, 0},
    {EXIF_TAG_GPS_DEST_LONGITUDE, EXIF_FORMAT_RATIONAL, 3, 0, 0},
    {EXIF_TAG_GPS_DEST_BEARING_REF, EXIF_FORMAT_ASCII, 2, 2, "T\x00"},
    {EXIF_TAG_GPS_DEST_BEARING, EXIF_FORMAT_RATIONAL, 1, 0, 0},
    {EXIF_TAG_GPS_DEST_DISTANCE_REF, EXIF_FORMAT_ASCII, 2, 2, "K\x00"},
    {EXIF_TAG_GPS_DEST_DISTANCE, EXIF_FORMAT_RATIONAL, 1, 0, 0},
    {EXIF_TAG_GPS_PROCESSING_METHOD, EXIF_FORMAT_UNDEFINED, 0, 0, 0},
    {EXIF_TAG_GPS_AREA_INFORMATION, EXIF_FORMAT_UNDEFINED, 0, 0, 0},
    {EXIF_TAG_GPS_DATE_STAMP, EXIF_FORMAT_ASCII, 0, 0, 0},
    {EXIF_TAG_GPS_DIFFERENTIAL, EXIF_FORMAT_SHORT, 1, 0, 0},
    {EXIF_TAG_GPS_H_POSITIONING_ERROR, EXIF_FORMAT_RATIONAL, 1, 0, 0},
};

const ExifGPSIfdTagInfo *exif_get_gps_tag_info(ExifTag tag) {
  size_t i;
  for (i = 0; i < sizeof(exif_gps_ifd_tags) / sizeof(ExifGPSIfdTagInfo); ++i) {
    if (tag==exif_gps_ifd_tags[i].tag)
      return &exif_gps_ifd_tags[i];
  }
  return NULL;
}

