/** \file test-null.c
 * \brief Pass NULL values into libexif APIs and ensure it doesn't crash.
 *
 * Copyright (C) 2019 Dan Fandrich <dan@coneharvesters.com>
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
 *
 */

#include "libexif/exif-data.h"
#include "libexif/exif-entry.h"
#include "libexif/exif-loader.h"
#include "libexif/exif-mnote-data.h"

#include <stdio.h>
#include <stdlib.h>

static void loader_null_test(void)
{
	ExifLoader *l;
	ExifData *d;
	unsigned char ret;
	const unsigned char *ccp;
	unsigned int i;

	l = exif_loader_new_mem(NULL);
	if (l) {
		fprintf(stderr, "Unexpected success in %s\n", "exif_loader_new_mem");
		exit(13);
	}

	exif_loader_ref(NULL);

	exif_loader_unref(NULL);

	exif_loader_write_file(NULL, "test");

	exif_loader_write_file(NULL, NULL);

	ret = exif_loader_write(NULL, (unsigned char *)"x", 1);
	if (ret) {
		fprintf(stderr, "Unexpected success in %s\n", "exif_loader_write");
		exit(13);
	}

	exif_loader_write(NULL, NULL, 123);

	exif_loader_reset(NULL);

	d = exif_loader_get_data(NULL);
	if (d) {
		fprintf(stderr, "Unexpected success in %s\n", "exif_loader_get_data");
		exit(13);
	}

	exif_loader_get_buf(NULL, NULL, NULL);

	exif_loader_log(NULL, NULL);

	l = exif_loader_new();
	if (!l) {
		fprintf(stderr, "Out of memory\n");
		exit(13);
	}

	exif_loader_write_file(l, NULL);

	exif_loader_write(l, NULL, 0);
	exif_loader_write(l, NULL, 1);

	exif_loader_get_buf(l, NULL, NULL);
	exif_loader_get_buf(l, &ccp, NULL);
	exif_loader_get_buf(l, NULL, &i);

	exif_loader_log(l, NULL);

	exif_loader_unref(l);
}

static void data_null_test(void)
{
	/* exif_data_new_from_file() is untested since it doesn't check path */

	ExifData *d;
	ExifByteOrder bo;
	ExifMnoteData *m;
	ExifDataType dt;
	unsigned char *buf;
	unsigned int len;

	d = exif_data_new_mem(NULL);
	if (d) {
		fprintf(stderr, "Unexpected success in %s\n", "exif_data_new_mem");
		exit(13);
	}

	d = exif_data_new_from_data(NULL, 123);
	if (d) {
		exif_data_unref(d);
	}

	bo = exif_data_get_byte_order(NULL);
	(void) bo;

	exif_data_set_byte_order(NULL, EXIF_BYTE_ORDER_MOTOROLA);

	m = exif_data_get_mnote_data(NULL);
	if (m) {
		fprintf(stderr, "Unexpected success in %s\n", "exif_data_get_mnote_data");
		exit(13);
	}

	exif_data_fix(NULL);

	exif_data_foreach_content(NULL, NULL, NULL);

	exif_data_set_option(NULL, EXIF_DATA_OPTION_FOLLOW_SPECIFICATION);

	exif_data_unset_option(NULL, EXIF_DATA_OPTION_FOLLOW_SPECIFICATION);

	exif_data_set_data_type(NULL, EXIF_DATA_TYPE_UNCOMPRESSED_CHUNKY);

	dt = exif_data_get_data_type(NULL);
	(void) dt;

	exif_data_load_data(NULL, NULL, 123);

	exif_data_save_data(NULL, NULL, NULL);

	exif_data_log(NULL, NULL);

	exif_data_dump(NULL);

	exif_data_ref(NULL);

	exif_data_unref(NULL);

	d = exif_data_new();
	if (!d) {
		fprintf(stderr, "Out of memory\n");
		exit(13);
	}

	exif_data_load_data(d, NULL, 123);

	exif_data_save_data(d, NULL, &len);
	exif_data_save_data(d, &buf, NULL);
	exif_data_save_data(d, NULL, NULL);

	exif_data_foreach_content(d, NULL, NULL);

	exif_data_log(d, NULL);

	exif_data_unref(d);
}

static void content_null_test(void)
{
	ExifContent *c;
	ExifIfd i;

	c = exif_content_new_mem(NULL);
	if (c) {
		fprintf(stderr, "Unexpected success in %s\n", "exif_content_new_mem");
		exit(13);
	}

	exif_content_ref(NULL);

	exif_content_unref(NULL);

	exif_content_free(NULL);

	exif_content_get_entry(NULL, EXIF_TAG_COMPRESSION);

	exif_content_fix(NULL);

	exif_content_foreach_entry(NULL, NULL, NULL);

	i = exif_content_get_ifd(NULL);
	(void) i;

	exif_content_dump(NULL, 0);

	exif_content_add_entry(NULL, NULL);

	exif_content_remove_entry(NULL, NULL);

	exif_content_log(NULL, NULL);

	c = exif_content_new();
	if (!c) {
		fprintf(stderr, "Out of memory\n");
		exit(13);
	}

	exif_content_add_entry(c, NULL);

	exif_content_remove_entry(c, NULL);

	exif_content_log(c, NULL);

	exif_content_foreach_entry(c, NULL, NULL);

	exif_content_unref(c);
}

static void entry_null_test(void)
{
	ExifEntry *e;
	const char *v = NULL;
	char buf[] = {0};
	ExifData *d;

	e = exif_entry_new_mem(NULL);
	if (e) {
		fprintf(stderr, "Unexpected success in %s\n", "exif_entry_new_mem");
		exit(13);
	}

	exif_entry_ref(NULL);

	exif_entry_unref(NULL);

	exif_entry_free(NULL);

	exif_entry_initialize(NULL, EXIF_TAG_COMPRESSION);

	exif_entry_fix(NULL);

	v = exif_entry_get_value(NULL, NULL, 123);
	if (v) {
		fprintf(stderr, "Unexpected success in %s\n", "exif_entry_get_value");
		exit(13);
	}

	v = exif_entry_get_value(NULL, buf, 1);
	if (v != buf) {
		fprintf(stderr, "Unexpected value in %s\n", "exif_entry_get_value");
		exit(13);
	}

	exif_entry_dump(NULL, 0);

	/* Creating a plain ExifEntry isn't enough, since some functions require
	 * that it exists in an IFD.
	 */
	d = exif_data_new();
	if (!d) {
		fprintf(stderr, "Out of memory\n");
		exit(13);
	}
	/* Create the mandatory EXIF fields so we have something to work with */
	exif_data_fix(d);
	e = exif_content_get_entry (d->ifd[EXIF_IFD_0], EXIF_TAG_X_RESOLUTION);

	(void) exif_entry_get_value(e, NULL, 0);
	(void) exif_entry_get_value(e, NULL, 123);

	exif_data_unref(d);
}

static void mnote_null_test(void)
{
	/* Note that these APIs aren't tested with a real ExifMnoteData pointer
	 * because it's impossible to create one without real data.
	 */
	exif_mnote_data_ref(NULL);

	exif_mnote_data_unref(NULL);

	exif_mnote_data_load(NULL, NULL, 123);

	exif_mnote_data_load(NULL, (const unsigned char *)"x", 1);

	exif_mnote_data_save(NULL, NULL, NULL);

	exif_mnote_data_count(NULL);

	exif_mnote_data_get_id(NULL, 0);

	exif_mnote_data_get_name(NULL, 0);

	exif_mnote_data_get_title(NULL, 0);

	exif_mnote_data_get_description(NULL, 0);

	exif_mnote_data_get_value(NULL, 0, NULL, 123);

	exif_mnote_data_log(NULL, NULL);
}

static void log_null_test(void)
{
	ExifLog *l;
	static va_list va;

	l = exif_log_new_mem(NULL);
	if (l) {
		fprintf(stderr, "Unexpected success in %s\n", "exif_log_new_mem");
		exit(13);
	}

	exif_log_ref(NULL);

	exif_log_unref(NULL);

	exif_log_free(NULL);

	exif_log_set_func(NULL, NULL, NULL);

	exif_log(NULL, EXIF_LOG_CODE_CORRUPT_DATA, "XXX", "YYY");

	exif_logv(NULL, EXIF_LOG_CODE_CORRUPT_DATA, "XXX", "YYY", va);

	l = exif_log_new();
	if (!l) {
		fprintf(stderr, "Out of memory\n");
		exit(13);
	}

	exif_log_set_func(l, NULL, NULL);

	exif_log_unref(l);
}

int main(void)
{
	loader_null_test();
	data_null_test();
	content_null_test();
	entry_null_test();
	mnote_null_test();
	log_null_test();

	/* If it gets here, we didn't get a SIGSEGV, so success! */
	return 0;
}
