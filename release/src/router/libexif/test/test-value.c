/* test-value.c
 *
 * Creates all the types of tags supported in exif_entry_initialize() and
 * ensures that exif_entry_get_value() properly truncates the output of each
 * one according to the buffer size available.
 *
 * Copyright 2002 Lutz Mueller <lutz@users.sourceforge.net>
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

#include <libexif/exif-utils.h>
#include <libexif/exif-data.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * List of tags to test, one per default initialized type.
 * There should be one for every block in exif_entry_initialize() and
 * exif_entry_get_value().
 */
ExifTag trunc_test_tags[] = {
	EXIF_TAG_PIXEL_X_DIMENSION,
	EXIF_TAG_SUBJECT_LOCATION,
	EXIF_TAG_IMAGE_WIDTH,
	EXIF_TAG_ORIENTATION,
	EXIF_TAG_SAMPLES_PER_PIXEL,
	EXIF_TAG_BITS_PER_SAMPLE,
	EXIF_TAG_X_RESOLUTION,
	EXIF_TAG_WHITE_POINT,
	EXIF_TAG_REFERENCE_BLACK_WHITE,
	EXIF_TAG_DATE_TIME,
	EXIF_TAG_IMAGE_DESCRIPTION,
	EXIF_TAG_EXIF_VERSION,
	EXIF_TAG_FLASH_PIX_VERSION,
	EXIF_TAG_COPYRIGHT,
	EXIF_TAG_FILE_SOURCE,
	EXIF_TAG_COMPONENTS_CONFIGURATION,
	EXIF_TAG_SCENE_TYPE,
	EXIF_TAG_YCBCR_SUB_SAMPLING,
	EXIF_TAG_PLANAR_CONFIGURATION,
};

/*
 * These tags produce different outputs depending on the amount of buffer space
 * available.
 */
ExifTag nonuniform_test_tags[] = {
	EXIF_TAG_RESOLUTION_UNIT,
	EXIF_TAG_COLOR_SPACE,
	EXIF_TAG_METERING_MODE,
};

/*
 * These tags need a nonzero rational number to be interesting.
 * They must have space for a rational or srational created automatically by
 * exif_entry_initialize().
 */
ExifTag rational_test_tags[] = {
	EXIF_TAG_FNUMBER,
	EXIF_TAG_APERTURE_VALUE,
	EXIF_TAG_MAX_APERTURE_VALUE,
	EXIF_TAG_FOCAL_LENGTH,
	EXIF_TAG_SUBJECT_DISTANCE,
	EXIF_TAG_EXPOSURE_TIME,
	EXIF_TAG_SHUTTER_SPEED_VALUE,
	EXIF_TAG_BRIGHTNESS_VALUE,
	EXIF_TAG_EXPOSURE_BIAS_VALUE,
};

/*
 * Verify that the entry is properly truncated to the buffer length within
 * exif_entry_get_value().  If uniform is zero, then only check that the
 * resulting string fits within the buffer and don't check its content.
 */
static void check_entry_trunc(ExifEntry *e, int uniform)
{
	unsigned int i;
	char v[1024], full[1024];  /* Large enough to never truncate output */

	printf ("Tag 0x%x\n", (int) e->tag);

	/* Get the full, untruncated string to use as the expected value */
	exif_entry_get_value (e, full, sizeof(full));
	printf ("Full: '%s'\n", full);

	for (i = strlen(full); i > 0; i--) {
		/* Make sure the buffer isn't NUL-terminated to begin with */
		memset(v, '*', sizeof(v));
		exif_entry_get_value (e, v, i);
		/* Truncate the full string by one on each iteration */
		full[i-1] = '\0';
		if ((strlen(v) >= i) || (uniform && strcmp(full, v))) {
			printf("Bad truncation!\n");
			printf("Length %2i: '%s'\n", i, v);
			exit(1);
		}
	}
}

int
main ()
{
	ExifData *data;
	ExifEntry *e;
	ExifMem *mem;
	unsigned i;
	static const ExifSRational r = {1., 20.};  /* a nonzero number */
	static const char user_comment[] = "ASCII\0\0\0A Long User Comment";
	static const char xp_comment[] = "U\0C\0S\0-\0002\0 \0C\0o\0m\0m\0e\0n\0t\0";
	static const char interop[] = "R98";
	static const char subsec[] = "130 ";
	static const ExifRational gpsh = {12., 1.};
	static const ExifRational gpsm = {34., 1.};
	static const ExifRational gpss = {56780., 1000.};

	data = exif_data_new ();
	if (!data) {
		fprintf (stderr, "Error running exif_data_new()\n");
		exit(13);
	}

	/* Full initialization/truncation tests */
	for (i=0; i < sizeof(trunc_test_tags)/sizeof(trunc_test_tags[0]); ++i) {
		e = exif_entry_new ();
		if (!e) {
			fprintf (stderr, "Error running exif_entry_new()\n");
			exit(13);
		}
		exif_content_add_entry (data->ifd[EXIF_IFD_0], e);
		exif_entry_initialize (e, trunc_test_tags[i]);
		check_entry_trunc(e, 1);
		exif_content_remove_entry (data->ifd[EXIF_IFD_0], e);
		exif_entry_unref (e);
	}

	/* Nonuniform initialization/truncation tests */
	for (i=0; i < sizeof(nonuniform_test_tags)/sizeof(nonuniform_test_tags[0]);
		 ++i) {
		e = exif_entry_new ();
		if (!e) {
			fprintf (stderr, "Error running exif_entry_new()\n");
			exit(13);
		}
		exif_content_add_entry (data->ifd[EXIF_IFD_0], e);
		exif_entry_initialize (e, nonuniform_test_tags[i]);
		check_entry_trunc(e, 0);
		exif_content_remove_entry (data->ifd[EXIF_IFD_0], e);
		exif_entry_unref (e);
	}

	/* Rational number initialization/truncation tests */
	for (i=0; i < sizeof(rational_test_tags)/sizeof(rational_test_tags[0]);
		 ++i) {
		e = exif_entry_new ();
		if (!e) {
			fprintf (stderr, "Error running exif_entry_new()\n");
			exit(13);
		}
		exif_content_add_entry (data->ifd[EXIF_IFD_0], e);
		exif_entry_initialize (e, rational_test_tags[i]);
		exif_set_srational (e->data, exif_data_get_byte_order (data), r);
		/* In case this tag needs an unsigned rational instead,
		 * fix the type automatically */
		exif_entry_fix (e);
		check_entry_trunc(e, 1);
		exif_content_remove_entry (data->ifd[EXIF_IFD_0], e);
		exif_entry_unref (e);
	}

	/* Create a memory allocator to manage the remaining ExifEntry structs */
	mem = exif_mem_new_default();
	if (!mem) {
		fprintf (stderr, "Out of memory\n");
		exit(13);
	}

	/* EXIF_TAG_SUB_SEC_TIME initialization/truncation tests */
	e = exif_entry_new_mem (mem);
	if (!e) {
		fprintf (stderr, "Out of memory\n");
		exit(13);
	}
	exif_content_add_entry (data->ifd[EXIF_IFD_0], e);
	exif_entry_initialize (e, EXIF_TAG_SUB_SEC_TIME);
	e->size = sizeof(subsec);  /* include NUL */
	e->components = e->size;
	/* Allocate memory to use for holding the tag data */
	e->data = exif_mem_alloc(mem, e->size);
	if (!e->data) {
		fprintf (stderr, "Out of memory\n");
		exit(13);
	}
	memcpy(e->data, subsec, e->size);
	check_entry_trunc(e, 1);
	exif_content_remove_entry (data->ifd[EXIF_IFD_0], e);
	exif_entry_unref (e);

	/* EXIF_TAG_USER_COMMENT initialization/truncation tests */
	e = exif_entry_new_mem (mem);
	if (!e) {
		fprintf (stderr, "Out of memory\n");
		exit(13);
	}
	exif_content_add_entry (data->ifd[EXIF_IFD_0], e);
	exif_entry_initialize (e, EXIF_TAG_USER_COMMENT);
	e->size = sizeof(user_comment) - 1;
	e->components = e->size;
	/* Allocate memory to use for holding the tag data */
	e->data = exif_mem_alloc(mem, e->size);
	if (!e->data) {
		fprintf (stderr, "Out of memory\n");
		exit(13);
	}
	memcpy(e->data, user_comment, e->size);
	check_entry_trunc(e, 1);
	exif_content_remove_entry (data->ifd[EXIF_IFD_0], e);
	exif_entry_unref (e);

	/* EXIF_TAG_XP_COMMENT truncation tests */
	e = exif_entry_new_mem (mem);
	if (!e) {
		fprintf (stderr, "Out of memory\n");
		exit(13);
	}
	exif_content_add_entry (data->ifd[EXIF_IFD_0], e);
	exif_entry_initialize (e, EXIF_TAG_XP_COMMENT);
	e->format = EXIF_FORMAT_BYTE;
	e->size = sizeof(xp_comment) - 1;
	e->components = e->size;
	/* Allocate memory to use for holding the tag data */
	e->data = exif_mem_alloc(mem, e->size);
	if (!e->data) {
		fprintf (stderr, "Out of memory\n");
		exit(13);
	}
	memcpy(e->data, xp_comment, e->size);
	check_entry_trunc(e, 1);
	exif_content_remove_entry (data->ifd[EXIF_IFD_0], e);
	exif_entry_unref (e);

	/* EXIF_TAG_INTEROPERABILITY_VERSION truncation tests */
	e = exif_entry_new_mem (mem);
	if (!e) {
		fprintf (stderr, "Out of memory\n");
		exit(13);
	}
	exif_content_add_entry (data->ifd[EXIF_IFD_INTEROPERABILITY], e);
	exif_entry_initialize (e, EXIF_TAG_INTEROPERABILITY_VERSION);
	e->format = EXIF_FORMAT_UNDEFINED;  /* The spec says ASCII, but libexif
					       allows UNDEFINED */
	e->size = sizeof(interop);  /* include NUL */
	e->components = e->size;
	/* Allocate memory to use for holding the tag data */
	e->data = exif_mem_alloc(mem, e->size);
	if (!e->data) {
		fprintf (stderr, "Out of memory\n");
		exit(13);
	}
	memcpy(e->data, interop, e->size);
	check_entry_trunc(e, 1);
	exif_content_remove_entry (data->ifd[EXIF_IFD_INTEROPERABILITY], e);
	exif_entry_unref (e);

	/* EXIF_TAG_GPS_VERSION_ID truncation tests */
	e = exif_entry_new_mem (mem);
	if (!e) {
		fprintf (stderr, "Out of memory\n");
		exit(13);
	}
	exif_content_add_entry (data->ifd[EXIF_IFD_GPS], e);
	exif_entry_initialize (e, EXIF_TAG_GPS_VERSION_ID);
	e->format = EXIF_FORMAT_BYTE;
	e->size = 4;
	e->components = e->size;
	/* Allocate memory to use for holding the tag data */
	e->data = exif_mem_alloc(mem, e->size);
	if (!e->data) {
		fprintf (stderr, "Out of memory\n");
		exit(13);
	}
	e->data[0] = 2;
	e->data[1] = 2;
	e->data[2] = 0;
	e->data[3] = 0;
	check_entry_trunc(e, 1);
	exif_content_remove_entry (data->ifd[EXIF_IFD_GPS], e);
	exif_entry_unref (e);

	/* EXIF_TAG_GPS_ALTITUDE_REF truncation tests */
	e = exif_entry_new_mem (mem);
	if (!e) {
		fprintf (stderr, "Out of memory\n");
		exit(13);
	}
	exif_content_add_entry (data->ifd[EXIF_IFD_GPS], e);
	exif_entry_initialize (e, EXIF_TAG_GPS_ALTITUDE_REF);
	e->format = EXIF_FORMAT_BYTE;
	e->size = 1;
	e->components = e->size;
	/* Allocate memory to use for holding the tag data */
	e->data = exif_mem_alloc(mem, e->size);
	if (!e->data) {
		fprintf (stderr, "Out of memory\n");
		exit(13);
	}
	e->data[0] = 1;
	check_entry_trunc(e, 1);
	exif_content_remove_entry (data->ifd[EXIF_IFD_GPS], e);
	exif_entry_unref (e);

	/* EXIF_TAG_GPS_TIME_STAMP truncation tests */
	e = exif_entry_new_mem (mem);
	if (!e) {
		fprintf (stderr, "Out of memory\n");
		exit(13);
	}
	exif_content_add_entry (data->ifd[EXIF_IFD_GPS], e);
	exif_entry_initialize (e, EXIF_TAG_GPS_TIME_STAMP);
	e->format = EXIF_FORMAT_RATIONAL;
	e->components = 3;
	e->size = e->components * exif_format_get_size(EXIF_FORMAT_RATIONAL);
	/* Allocate memory to use for holding the tag data */
	e->data = exif_mem_alloc(mem, e->size);
	if (!e->data) {
		fprintf (stderr, "Out of memory\n");
		exit(13);
	}
	exif_set_rational(e->data, exif_data_get_byte_order (data), gpsh);
	exif_set_rational(e->data+8, exif_data_get_byte_order (data), gpsm);
	exif_set_rational(e->data+16, exif_data_get_byte_order (data), gpss);
	check_entry_trunc(e, 1);
	exif_content_remove_entry (data->ifd[EXIF_IFD_GPS], e);
	exif_entry_unref (e);

	/* EXIF_TAG_SUBJECT_AREA truncation tests */
	e = exif_entry_new_mem (mem);
	if (!e) {
		fprintf (stderr, "Out of memory\n");
		exit(13);
	}
	exif_content_add_entry (data->ifd[EXIF_IFD_0], e);
	exif_entry_initialize (e, EXIF_TAG_SUBJECT_AREA);
	e->format = EXIF_FORMAT_SHORT;
	/* This tags is interpreted differently depending on # components */
	/* Rectangle */
	e->components = 4;
	e->size = e->components * exif_format_get_size(EXIF_FORMAT_SHORT);
	/* Allocate memory to use for holding the tag data */
	e->data = exif_mem_alloc(mem, e->size);
	if (!e->data) {
		fprintf (stderr, "Out of memory\n");
		exit(13);
	}
	exif_set_short(e->data, exif_data_get_byte_order (data), 123);
	exif_set_short(e->data+2, exif_data_get_byte_order (data), 456);
	exif_set_short(e->data+4, exif_data_get_byte_order (data), 78);
	exif_set_short(e->data+6, exif_data_get_byte_order (data), 90);
	check_entry_trunc(e, 1);
	/* Circle */
	e->components = 3;
	e->size = e->components * exif_format_get_size(EXIF_FORMAT_SHORT);
	check_entry_trunc(e, 1);
	/* Centre */
	e->components = 2;
	e->size = e->components * exif_format_get_size(EXIF_FORMAT_SHORT);
	check_entry_trunc(e, 1);
	/* Invalid */
	e->components = 1;
	e->size = e->components * exif_format_get_size(EXIF_FORMAT_SHORT);
	check_entry_trunc(e, 1);
	exif_content_remove_entry (data->ifd[EXIF_IFD_0], e);
	exif_entry_unref (e);

	exif_mem_unref(mem);
	exif_data_unref (data);

	return 0;
}
