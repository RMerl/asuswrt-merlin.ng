/* mnote-olympus-data.h
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

#ifndef LIBEXIF_EXIF_MNOTE_DATA_OLYMPUS_H
#define LIBEXIF_EXIF_MNOTE_DATA_OLYMPUS_H

#include <libexif/exif-mnote-data-priv.h>
#include <libexif/olympus/mnote-olympus-entry.h>
#include <libexif/exif-byte-order.h>
#include <libexif/exif-data.h>
#include <libexif/exif-mem.h>

enum OlympusVersion {
	unrecognized = 0,
	nikonV1 = 1,
	nikonV2 = 2,
	olympusV1 = 3,
	olympusV2 = 4,
	sanyoV1 = 5,
	epsonV1 = 6,
	nikonV0 = 7
};


typedef struct _ExifMnoteDataOlympus ExifMnoteDataOlympus;

struct _ExifMnoteDataOlympus {
	ExifMnoteData parent;

	MnoteOlympusEntry *entries;
	unsigned int count;

	ExifByteOrder order;
	unsigned int offset;
	enum OlympusVersion version;
};

/*! Detect if MakerNote is recognized as one handled by the Olympus module.
 * 
 * \param[in] ed image #ExifData to identify as as an Olympus type
 * \param[in] e #ExifEntry for EXIF_TAG_MAKER_NOTE, from within ed but
 *   duplicated here for convenience
 * \return 0 if not recognized, nonzero if recognized. The specific nonzero 
 *   value returned may identify a subtype unique within this module.
 */
int exif_mnote_data_olympus_identify (const ExifData *ed, const ExifEntry *e);

ExifMnoteData *exif_mnote_data_olympus_new (ExifMem *);

#endif /* !defined(LIBEXIF_EXIF_MNOTE_DATA_OLYMPUS_H) */
