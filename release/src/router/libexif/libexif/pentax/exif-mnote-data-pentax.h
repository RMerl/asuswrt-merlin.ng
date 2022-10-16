/* exif-mnote-data-pentax.h
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

#ifndef LIBEXIF_EXIF_MNOTE_DATA_PENTAX_H
#define LIBEXIF_EXIF_MNOTE_DATA_PENTAX_H

#include <libexif/exif-byte-order.h>
#include <libexif/exif-mnote-data.h>
#include <libexif/exif-mnote-data-priv.h>
#include <libexif/pentax/mnote-pentax-entry.h>
#include <libexif/exif-data.h>
#include <libexif/exif-mem.h>

enum PentaxVersion {pentaxV1 = 1, pentaxV2 = 2, pentaxV3 = 3, casioV2 = 4 };

typedef struct _ExifMnoteDataPentax ExifMnoteDataPentax;

struct _ExifMnoteDataPentax {
	ExifMnoteData parent;

	MnotePentaxEntry *entries;
	unsigned int count;

	ExifByteOrder order;
	unsigned int offset;

	enum PentaxVersion version;
};

/*! Detect if MakerNote is recognized as one handled by the Pentax module.
 * 
 * \param[in] ed image #ExifData to identify as as a Pentax type
 * \param[in] e #ExifEntry for EXIF_TAG_MAKER_NOTE, from within ed but
 *   duplicated here for convenience
 * \return 0 if not recognized, nonzero if recognized. The specific nonzero 
 *   value returned may identify a subtype unique within this module.
 */
int exif_mnote_data_pentax_identify (const ExifData *ed, const ExifEntry *e);

ExifMnoteData *exif_mnote_data_pentax_new (ExifMem *);

#endif /* !defined(LIBEXIF_EXIF_MNOTE_DATA_PENTAX_H) */
