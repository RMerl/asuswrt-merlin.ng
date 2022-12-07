/** \file test-parse-from-data.c
 * \brief Completely parse all files given on the command line.
 *
 * Copyright (C) 2007 Hans Ulrich Niedermann <gp@n-dimensional.de>
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
#include "libexif/exif-system.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>


static unsigned entry_count;

/** Callback function handling an ExifEntry. */
static void content_foreach_func(ExifEntry *entry, void *UNUSED(callback_data))
{
  char buf[2000];
  exif_entry_get_value(entry, buf, sizeof(buf));
  printf("    Entry %u: %s (%s)\n"
	 "      Size, Comps: %d, %d\n"
	 "      Value: %s\n", 
	 entry_count,
	 exif_tag_get_name(entry->tag),
	 exif_format_get_name(entry->format),
	 entry->size,
	 (int)(entry->components),
	 exif_entry_get_value(entry, buf, sizeof(buf)));
  ++entry_count;
}


/** Callback function handling an ExifContent (corresponds 1:1 to an IFD). */
static void data_foreach_func(ExifContent *content, void *callback_data)
{
  static unsigned content_count;
  entry_count = 0;
  printf("  Content %u: ifd=%d\n", content_count, exif_content_get_ifd(content));
  exif_content_foreach_entry(content, content_foreach_func, callback_data);
  ++content_count;
}

static void dump_makernote(ExifData *d) {
  ExifMnoteData *mn = exif_data_get_mnote_data(d);
  if (mn) {
    char buf[2000];
    int i;
    int num = exif_mnote_data_count(mn);
    printf("  MakerNote\n");
    for (i=0; i < num; ++i) {
      if (exif_mnote_data_get_value(mn, i, buf, sizeof(buf))) {
	const char *name = exif_mnote_data_get_name(mn, i);
	unsigned int id = exif_mnote_data_get_id(mn, i);
	if (!name)
	    name = "(unknown)";
	printf("    Entry %u: %u, %s\n"
	       "      Size: %u\n"
	       "      Value: %s\n", i, id, name, (unsigned)strlen(buf), buf);
      }
    }
  }
}

/** Run EXIF parsing test on the given file. */
static void test_parse(const char *filename, void *callback_data, int swap)
{
  ExifData *d;
  int fd;
  unsigned char *data;
  struct stat stbuf;

  /* Skip over path to display only the file name */
  const char *fn = strrchr(filename, '/');
  if (fn)
    ++fn;
  else
    fn = filename;
  printf("File %s\n", fn);

  d = exif_data_new_from_file(filename);
  fd = open(filename,O_RDONLY);
  if (fd == -1) {
    perror(filename);
    return;
  }
  if (-1 == fstat(fd, &stbuf)) {
    perror(filename);
    return;
  }
  data = malloc(stbuf.st_size);
  if (!data) {
    fprintf (stderr, "Failed to allocate %ld bytes for reading %s\n", stbuf.st_size, filename);
    return;
  }
  if (-1 == read(fd, data, stbuf.st_size)) {
    perror ("read");
    free(data);
    close(fd);
    return;
  }
  close(fd);

  d = exif_data_new_from_data(data, stbuf.st_size);
  if (!d) {
      fprintf (stderr, "Could not load data from '%s'!\n", filename);
      free(data);
      return;
  }
  printf("Byte order: %s\n",
          exif_byte_order_get_name(exif_data_get_byte_order(d)));

  if (swap) {
      ExifByteOrder order = EXIF_BYTE_ORDER_INTEL;
      if (exif_data_get_byte_order(d) == order) {
          order = EXIF_BYTE_ORDER_MOTOROLA;
      }
      /* This switches the byte order of the entire EXIF data structure,
       * including the MakerNote */
      exif_data_set_byte_order(d, order);
      printf("New byte order: %s\n",
              exif_byte_order_get_name(exif_data_get_byte_order(d)));
  }

  exif_data_foreach_content(d, data_foreach_func, callback_data);

  dump_makernote(d);

  exif_data_unref(d);
}


/** Callback function prototype for string parsing. */
typedef void (*test_parse_func) (const char *filename, void *callback_data, int swap);


/** Split string at whitespace and call callback with each substring. */
static void split_ws_string(const char *string, test_parse_func func, void *callback_data)
{
  const char *start = string;
  const char *p = start;
  for (;;) {
    if (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r' || *p == '\0' ) {
      size_t len = p-start;
      if (len > 0) {
	/* emulate strndup */
	char *str = malloc(1+len);
	if (str) {
	  memcpy(str, start, len);
	  str[len] = '\0';
	  func(str, callback_data, 0);
	  free(str);
	  start = p+1;
	}
      } else {
	start = p+1;
      }
    }
    if (*p == '\0') {
      break;
    }
    p++;
  }  
}


/** Main program. */
int main(const int argc, const char *argv[])
{
  int i;
  void *callback_data = NULL;
  int swap = 0;
  int first = 1;

  if (argc > 1 && !strcmp(argv[1], "--swap-byte-order")) {
      swap = 1;
      ++first;
  }

  if (argc > first) {
    for (i=first; i<argc; i++) {
      test_parse(argv[i], callback_data, swap);
    }
  } else {
    /* If no command-line argument is found, get the file names from
       the environment. */
    const char *envar = getenv("TEST_IMAGES");
    if (envar) {
      split_ws_string(envar, test_parse, callback_data);
    }
  }

  return 0;
}
