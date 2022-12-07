/* exif-mnote-data-apple.c
 *
 * Copyright (c) 2018 zhanwang-sky <zhanwang_sky@163.com>
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
#include "exif-mnote-data-apple.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libexif/exif-data.h>
#include <libexif/exif-utils.h>

static void
exif_mnote_data_apple_free(ExifMnoteData *md) {
    ExifMnoteDataApple *d = (ExifMnoteDataApple *) md;
    unsigned int i;

    /*printf("%s\n", __FUNCTION__);*/

    if (!d) {
        return;
    }

    if (d->entries) {
        for (i = 0; i < d->count; i++) {
            if (d->entries[i].data) {
                exif_mem_free(md->mem, d->entries[i].data);
            }
        }
        exif_mem_free(md->mem, d->entries);
        d->entries = NULL;
        d->count = 0;
    }

    return;
}

static void
exif_mnote_data_apple_load(ExifMnoteData *md, const unsigned char *buf, unsigned int buf_size) {
    ExifMnoteDataApple *d = (ExifMnoteDataApple *) md;
    unsigned int tcount, i;
    unsigned int dsize;
    unsigned int ofs, dofs;

    /*printf("%s\n", __FUNCTION__);*/

    if (!d || !buf || (buf_size < 6 + 16)) {
        exif_log(md->log, EXIF_LOG_CODE_CORRUPT_DATA,
                 "ExifMnoteDataApple", "Short MakerNote");
        return;
    }

    /* Start of interesting data */
    ofs = d->offset + 6;
    if (ofs > buf_size - 16) {
        exif_log(md->log, EXIF_LOG_CODE_CORRUPT_DATA,
                 "ExifMnoteDataApple", "Short MakerNote");
        return;
    }

    if ((buf[ofs + 12] == 'M') && (buf[ofs + 13] == 'M')) {
        d->order = EXIF_BYTE_ORDER_MOTOROLA;
    } else if ((buf[ofs + 12] == 'I') && (buf[ofs + 13] == 'I')) {
        d->order = EXIF_BYTE_ORDER_INTEL;
    } else {
        exif_log(md->log, EXIF_LOG_CODE_CORRUPT_DATA,
                "ExifMnoteDataApple", "Unrecognized byte order");
        return;
    }

    tcount = (unsigned int) exif_get_short(buf + ofs + 14, d->order);

    /* Sanity check the offset */
    if (buf_size < d->offset + 6 + 16 + tcount * 12 + 4) {
        exif_log(md->log, EXIF_LOG_CODE_CORRUPT_DATA,
                 "ExifMnoteDataApple", "Short MakerNote");
        return;
    }

    ofs += 16;

    exif_mnote_data_apple_free(md);

    /* Reserve enough space for all the possible MakerNote tags */
    d->entries = exif_mem_alloc(md->mem, sizeof(MnoteAppleEntry) * tcount);
    if (!d->entries) {
        EXIF_LOG_NO_MEMORY(md->log, "ExifMnoteApple", sizeof(MnoteAppleEntry) * tcount);
        return;
    }
    memset(d->entries, 0, sizeof(MnoteAppleEntry) * tcount);

    for (i = 0; i < tcount; i++) {
	if (ofs + 12 > buf_size) {
		exif_log (md->log, EXIF_LOG_CODE_CORRUPT_DATA,
                                  "ExifMnoteApplet", "Tag size overflow detected (%u vs size %u)", ofs + 12, buf_size);
		break;
	}
        d->entries[i].tag = exif_get_short(buf + ofs, d->order);
        d->entries[i].format = exif_get_short(buf + ofs + 2, d->order);
        d->entries[i].components = exif_get_long(buf + ofs + 4, d->order);
        d->entries[i].order = d->order;
	if ((d->entries[i].components) && (buf_size / d->entries[i].components < exif_format_get_size(d->entries[i].format))) {
		exif_log (md->log, EXIF_LOG_CODE_CORRUPT_DATA,
                                  "ExifMnoteApplet", "Tag size overflow detected (components %lu vs size %u)", d->entries[i].components, buf_size);
		break;
	}
        dsize = exif_format_get_size(d->entries[i].format) * d->entries[i].components;
	if ((dsize > 65536) || (dsize > buf_size)) {
		/* Corrupt data: EXIF data size is limited to the
		 * maximum size of a JPEG segment (64 kb).
		 */
		break;
	}
        if (dsize > 4) {
            dofs = d->offset + exif_get_long(buf + ofs + 8, d->order);
        } else {
            dofs = ofs + 8;
        }
	if (dofs > buf_size) {
		exif_log (md->log, EXIF_LOG_CODE_CORRUPT_DATA,
                                  "ExifMnoteApplet", "Tag size overflow detected (%u vs size %u)", dofs, buf_size);
		continue;
	}
        ofs += 12;
        d->entries[i].data = exif_mem_alloc(md->mem, dsize);
        if (!d->entries[i].data) {
            EXIF_LOG_NO_MEMORY(md->log, "ExifMnoteApple", dsize);
            continue;
        }
	if (dofs + dsize > buf_size) {
		exif_log (md->log, EXIF_LOG_CODE_CORRUPT_DATA,
                                  "ExifMnoteApplet", "Tag size overflow detected (%u vs size %u)", dofs + dsize, buf_size);
		continue;
	}
        memcpy(d->entries[i].data, buf + dofs, dsize);
        d->entries[i].size = dsize;
    }
    d->count = tcount;

    return;
}

static void
exif_mnote_data_apple_set_offset(ExifMnoteData *md, unsigned int o) {
    /*printf("%s\n", __FUNCTION__);*/

    if (md) {
        ((ExifMnoteDataApple *) md)->offset = o;
    }

    return;
}

static void
exif_mnote_data_apple_set_byte_order(ExifMnoteData *md , ExifByteOrder o) {
    ExifMnoteDataApple *d = (ExifMnoteDataApple *) md;
    unsigned int i;

    /*printf("%s\n", __FUNCTION__);*/

    if (!d || d->order == o) {
        return;
    }

    for (i = 0; i < d->count; i++) {
	if (d->entries[i].components && (d->entries[i].size/d->entries[i].components < exif_format_get_size (d->entries[i].format)))
		continue;
        exif_array_set_byte_order(d->entries[i].format, d->entries[i].data,
                                  d->entries[i].components, d->entries[i].order, o);
        d->entries[i].order = o;
    }
    d->order = o;

    return;
}

static unsigned int
exif_mnote_data_apple_count(ExifMnoteData *md){
    /*printf("%s\n", __FUNCTION__);*/

    return md ? ((ExifMnoteDataApple *) md)->count : 0;
}

static unsigned int
exif_mnote_data_apple_get_id(ExifMnoteData *md, unsigned int i) {
    ExifMnoteDataApple *d = (ExifMnoteDataApple *) md;

    if (!d || (d->count <= i)) {
        return 0;
    }

    return d->entries[i].tag;
}

static const char *
exif_mnote_data_apple_get_name(ExifMnoteData *md, unsigned int i) {
    ExifMnoteDataApple *d = (ExifMnoteDataApple *) md;

    if (!d || (d->count <= i)) {
        return NULL;
    }

    return mnote_apple_tag_get_name(d->entries[i].tag);
}

static const char *
exif_mnote_data_apple_get_title(ExifMnoteData *md, unsigned int i) {
    ExifMnoteDataApple *d = (ExifMnoteDataApple *) md;

    if (!d || (d->count <= i)) {
        return NULL;
    }

    return mnote_apple_tag_get_title(d->entries[i].tag);
}

static const char *
exif_mnote_data_apple_get_description(ExifMnoteData *md, unsigned int i) {
    ExifMnoteDataApple *d = (ExifMnoteDataApple *) md;

    if (!d || (d->count <= i)) {
        return NULL;
    }

    return mnote_apple_tag_get_description(d->entries[i].tag);
}

static char *
exif_mnote_data_apple_get_value(ExifMnoteData *md, unsigned int i, char *val, unsigned int maxlen) {
    ExifMnoteDataApple *d = (ExifMnoteDataApple *) md;

    if (!val || !d || (d->count <= i)) {
        return NULL;
    }

    return mnote_apple_entry_get_value(&d->entries[i], val, maxlen);
}

int
exif_mnote_data_apple_identify(const ExifData *ed, const ExifEntry *e) {
    (void) ed;

    if (e->size < strlen("Apple iOS")+1)
	return 0;

    return !memcmp((const char *) e->data, "Apple iOS", strlen("Apple iOS"));
}

ExifMnoteData *
exif_mnote_data_apple_new(ExifMem *mem) {
    ExifMnoteData *md;

    /*printf("%s\n", __FUNCTION__);*/

    if (!mem) {
        return NULL;
    }

    md = exif_mem_alloc(mem, sizeof(ExifMnoteDataApple));
    if (!md) {
        return NULL;
    }

    exif_mnote_data_construct(md, mem);

    md->methods.free = exif_mnote_data_apple_free;
    md->methods.load = exif_mnote_data_apple_load;
    md->methods.set_offset = exif_mnote_data_apple_set_offset;
    md->methods.set_byte_order = exif_mnote_data_apple_set_byte_order;
    md->methods.count = exif_mnote_data_apple_count;
    md->methods.get_id = exif_mnote_data_apple_get_id;
    md->methods.get_name = exif_mnote_data_apple_get_name;
    md->methods.get_title = exif_mnote_data_apple_get_title;
    md->methods.get_description = exif_mnote_data_apple_get_description;
    md->methods.get_value = exif_mnote_data_apple_get_value;

    return md;
}
