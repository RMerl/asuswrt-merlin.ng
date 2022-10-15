/* exif-mnote.c
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

#include <config.h>

#include <stdio.h>
#include <stdlib.h>

#include <libexif/exif-data.h>

static int
test_exif_data (ExifData *d)
{
	unsigned int i, c;
	char v[1024], *p;
	ExifMnoteData *md;

	printf("Byte order: %s\n",
		exif_byte_order_get_name (exif_data_get_byte_order (d)));

	printf("Parsing maker note...\n");
	md = exif_data_get_mnote_data (d);
	if (!md) {
		fprintf (stderr, "Could not parse maker note!\n");
		exif_data_unref (d);
		return 1;
	}

	printf("Increasing ref-count...\n");
	exif_mnote_data_ref (md);

	printf("Decreasing ref-count...\n");
	exif_mnote_data_unref (md);

	printf("Counting entries...\n");
	c = exif_mnote_data_count (md);
	printf("Found %i entries.\n", c);
	for (i = 0; i < c; i++) {
		printf("Dumping entry number %i...\n", i);
		printf("  Name: '%s'\n",
				exif_mnote_data_get_name (md, i));
		printf("  Title: '%s'\n",
				exif_mnote_data_get_title (md, i));
		printf("  Description: '%s'\n",
				exif_mnote_data_get_description (md, i));
		p = exif_mnote_data_get_value (md, i, v, sizeof (v));
		if (p) { printf("  Value: '%s'\n", v); }
	}

	return 0;
}

int
main (int argc, char **argv)
{
	ExifData *d;
	unsigned int buf_size;
	unsigned char *buf;
	int r;

	if (argc <= 1) {
		fprintf (stderr, "You need to supply a filename!\n");
		return 1;
	}

	printf("Loading '%s'...\n", argv[1]);
	d = exif_data_new_from_file (argv[1]);
	if (!d) {
		fprintf (stderr, "Could not load data from '%s'!\n", argv[1]);
		return 1;
	}
	printf("Loaded '%s'.\n", argv[1]);

	printf("######### Test 1 #########\n");
	r = test_exif_data (d);
	if (r) return r;

	exif_data_save_data (d, &buf, &buf_size);
	exif_data_unref (d);
	d = exif_data_new_from_data (buf, buf_size);
	if (!d) {
		fprintf (stderr, "Could not load data from buf!\n");
		return 1;
	}
	free (buf);

	printf ("######### Test 2 #########\n");
	r = test_exif_data (d);
	if (r) return r;

	printf ("Test successful!\n");

	return 1;
}
