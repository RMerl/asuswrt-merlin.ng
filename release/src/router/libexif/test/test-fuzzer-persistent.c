/**file test-fuzzer-persistent.c
 * from test-parse.c and test-mnote.c
 *
 * \brief Persistent AFL fuzzing binary (reaches 4 digits execs / second)
 *
 * Copyright (C) 2007 Hans Ulrich Niedermann <gp@n-dimensional.de>
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
 *
 */

#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "libexif/exif-data.h"
#include "libexif/exif-loader.h"
#include "libexif/exif-system.h"

__AFL_FUZZ_INIT();

#undef USE_LOG

#ifdef USE_LOG
static void
logfunc(ExifLog *log, ExifLogCode code, const char *domain, const char *format, va_list args, void *data)
{
	fprintf( stderr, "test-fuzzer: code=%d domain=%s ", code, domain);
	vfprintf (stderr, format, args);
	fprintf (stderr, "\n");
}
#endif

/** Callback function handling an ExifEntry. */
void content_foreach_func(ExifEntry *entry, void *callback_data);
void content_foreach_func(ExifEntry *entry, void *UNUSED(callback_data))
{
	char buf[2001];

	/* ensure \0 */
	buf[sizeof(buf)-1] = 0;
	buf[sizeof(buf)-2] = 0;
	exif_tag_get_name(entry->tag);
	exif_format_get_name(entry->format);
	exif_entry_get_value(entry, buf, sizeof(buf)-1);
	if (buf[sizeof(buf)-2] != 0) abort();
}


/** Callback function handling an ExifContent (corresponds 1:1 to an IFD). */
void data_foreach_func(ExifContent *content, void *callback_data);
void data_foreach_func(ExifContent *content, void *callback_data)
{
	printf("  Content %p: ifd=%d\n", (void *)content, exif_content_get_ifd(content));
	exif_content_foreach_entry(content, content_foreach_func, callback_data);
}
static int
test_exif_data (ExifData *d)
{
	unsigned int i, c;
	char v[1024];
	ExifMnoteData *md;

	fprintf (stdout, "Byte order: %s\n",
		exif_byte_order_get_name (exif_data_get_byte_order (d)));

	md = exif_data_get_mnote_data (d);
	if (!md) {
		fprintf (stderr, "Could not parse maker note!\n");
		return 1;
	}

	exif_mnote_data_ref (md);
	exif_mnote_data_unref (md);

	c = exif_mnote_data_count (md);
	for (i = 0; i < c; i++) {
		const char *name = exif_mnote_data_get_name (md, i);
		if (!name) continue;
		exif_mnote_data_get_name (md, i);
		exif_mnote_data_get_title (md, i);
		exif_mnote_data_get_description (md, i);
		exif_mnote_data_get_value (md, i, v, sizeof (v));
	}

	return 0;
}

/** Main program. */
int main(const int argc, const char *argv[])
{
	int		i;
	ExifData	*d;
	ExifLoader	*loader = exif_loader_new();
	unsigned int	xbuf_size;
	unsigned char	*xbuf;
	FILE		*f;
	struct		stat stbuf;
#ifdef USE_LOG
	ExifLog		*log = exif_log_new ();

	exif_log_set_func(log, logfunc, NULL);
#endif

#ifdef __AFL_HAVE_MANUAL_CONTROL
	__AFL_INIT();
#endif

	unsigned char *buf = __AFL_FUZZ_TESTCASE_BUF;  // must be after __AFL_INIT
                                                 // and before __AFL_LOOP!

	while (__AFL_LOOP(10000)) {

		int len = __AFL_FUZZ_TESTCASE_LEN;  // don't use the macro directly in a call!

		d = exif_data_new_from_data(buf, len);

		/* try the exif loader */
	#ifdef USE_LOG
		exif_data_log (d, log);
	#endif
		exif_data_foreach_content(d, data_foreach_func, NULL);
		test_exif_data (d);

		xbuf = NULL;
		exif_data_save_data (d, &xbuf, &xbuf_size);
		free (xbuf);

		exif_data_set_byte_order(d, EXIF_BYTE_ORDER_INTEL);

		xbuf = NULL;
		exif_data_save_data (d, &xbuf, &xbuf_size);
		free (xbuf);

		exif_data_unref(d);

#if 0
		/* try the exif data writer ... different than the loader */

		exif_loader_write(loader, buf, len);

		d = exif_loader_get_data(loader);
		exif_data_foreach_content(d, data_foreach_func, NULL);
		test_exif_data (d);
		exif_loader_unref(loader);
		exif_data_unref(d);
#endif
	}
	return 0;
}
