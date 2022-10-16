/* exif-mnote-data-fuji.h
 *
 * Copyright (c) 2002 Lutz Mueller <lutz@users.sourceforge.net>
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

#ifndef LIBEXIF_EXIF_MNOTE_DATA_FUJI_H
#define LIBEXIF_EXIF_MNOTE_DATA_FUJI_H

#include <libexif/exif-mnote-data.h>
#include <libexif/exif-mnote-data-priv.h>
#include <libexif/exif-data.h>
#include <libexif/fuji/mnote-fuji-entry.h>

typedef struct _ExifMnoteDataFuji        ExifMnoteDataFuji;

struct _ExifMnoteDataFuji {
	ExifMnoteData parent;

	MnoteFujiEntry *entries;
	unsigned int count;

	ExifByteOrder order;
	unsigned int offset;
};

/*! Detect if MakerNote is recognized as one handled by the Fuji module.
 * 
 * \param[in] ed image #ExifData to identify as as a Fuji type
 * \param[in] e #ExifEntry for EXIF_TAG_MAKER_NOTE, from within ed but
 *   duplicated here for convenience
 * \return 0 if not recognized, nonzero if recognized. The specific nonzero 
 *   value returned may identify a subtype unique within this module.
 */
int exif_mnote_data_fuji_identify (const ExifData *ed, const ExifEntry *e);

ExifMnoteData *exif_mnote_data_fuji_new (ExifMem *);

#endif /* !defined(LIBEXIF_EXIF_MNOTE_DATA_FUJI_H) */
