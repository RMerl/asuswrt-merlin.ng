/** \file test-extract.c
 * \brief Extract EXIF data from a file and write it to another file.
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

#include <string.h>
#include <stdio.h>
#include <stdlib.h>


static const unsigned char header[4] = {'\xff', '\xd8', '\xff', '\xe1'};

int main(const int argc, const char *argv[])
{
  int first = 1;
  const char *fn = "input.jpg";
  const char *outfn = "output.exif";
  ExifData *d;
  unsigned char *buf;
  unsigned int len;
  FILE *f;
  unsigned char lenbuf[2];

  if (argc > 1 && !strcmp(argv[1], "-o")) {
      outfn = argv[2];
      first += 2;
  }
  if (argc > first) {
      fn = argv[first];
      ++first;
  }
  if (argc > first) {
      fprintf (stderr, "Too many arguments\n");
      return 1;
  }

  d = exif_data_new_from_file(fn);
  if (!d) {
      fprintf (stderr, "Could not load data from '%s'!\n", fn);
      return 1;
  }

  exif_data_save_data(d, &buf, &len);
  exif_data_unref(d);

  if (!buf) {
      fprintf (stderr, "Could not extract EXIF data!\n");
      return 2;
  }

  f = fopen(outfn, "wb");
  if (!f) {
      fprintf (stderr, "Could not open '%s' for writing!\n", outfn);
      return 1;
  }
  /* Write EXIF with headers and length. */
  if (fwrite(header, 1, sizeof(header), f) != sizeof(header)) {
      fprintf (stderr, "Could not write to '%s'!\n", outfn);
      return 3;
  }
  /*
   * FIXME: The buffer from exif_data_save_data() seems to contain extra 0xffd8
   * 0xffd9 JPEG markers at the end that I wasn't expecting, making the length
   * seem too long. Should those markers really be included?
   */
  exif_set_short(lenbuf, EXIF_BYTE_ORDER_MOTOROLA, len);
  if (fwrite(lenbuf, 1, 2, f) != 2) {
      fprintf (stderr, "Could not write to '%s'!\n", outfn);
      return 3;
  }
  if (fwrite(buf, 1, len, f) != len) {
      fprintf (stderr, "Could not write to '%s'!\n", outfn);
      return 3;
  }
  if (fclose(f) != 0) {
      fprintf (stderr, "Could not close '%s'!\n", outfn);
      return 3;
  }
  free(buf);
  fprintf (stderr, "Wrote EXIF data to '%s'\n", outfn);

  return 0;
}
