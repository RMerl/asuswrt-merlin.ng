/* exif-mnote-data-olympus.c
 *
 * Copyright (c) 2002, 2003 Lutz Mueller <lutz@users.sourceforge.net>
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
#include "exif-mnote-data-olympus.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <libexif/exif-utils.h>
#include <libexif/exif-data.h>

/* Uncomment this to fix a problem with Sanyo MakerNotes. It's probably best
 * not to in most cases because it seems to only affect the thumbnail tag
 * which is duplicated in IFD 1, and fixing the offset could actually cause
 * problems with other software that expects the broken form.
 */
/*#define EXIF_OVERCOME_SANYO_OFFSET_BUG */

#define CHECKOVERFLOW(offset,datasize,structsize) (( (offset) >= (datasize)) || ((structsize) > (datasize)) || ((offset) > (datasize) - (structsize) ))

static enum OlympusVersion
exif_mnote_data_olympus_identify_variant (const unsigned char *buf,
		unsigned int buf_size);


static void
exif_mnote_data_olympus_clear (ExifMnoteDataOlympus *n)
{
	ExifMnoteData *d = (ExifMnoteData *) n;
	unsigned int i;

	if (!n) return;

	if (n->entries) {
		for (i = 0; i < n->count; i++)
			if (n->entries[i].data) {
				exif_mem_free (d->mem, n->entries[i].data);
				n->entries[i].data = NULL;
			}
		exif_mem_free (d->mem, n->entries);
		n->entries = NULL;
		n->count = 0;
	}
}

static void
exif_mnote_data_olympus_free (ExifMnoteData *n)
{
	if (!n) return;

	exif_mnote_data_olympus_clear ((ExifMnoteDataOlympus *) n);
}

static char *
exif_mnote_data_olympus_get_value (ExifMnoteData *d, unsigned int i, char *val, unsigned int maxlen)
{
	ExifMnoteDataOlympus *n = (ExifMnoteDataOlympus *) d;

	if (!d || !val) return NULL;
	if (i > n->count -1) return NULL;
/*
	exif_log (d->log, EXIF_LOG_CODE_DEBUG, "ExifMnoteDataOlympus",
		  "Querying value for tag '%s'...",
		  mnote_olympus_tag_get_name (n->entries[i].tag));
*/
	return mnote_olympus_entry_get_value (&n->entries[i], val, maxlen);
}




/** 
 * @brief save the MnoteData from ne to buf
 * 
 * @param ne extract the data from this structure 
 * @param *buf write the mnoteData to this buffer (buffer will be allocated)
 * @param buf_size the size of the buffer
 */
static void
exif_mnote_data_olympus_save (ExifMnoteData *ne,
		unsigned char **buf, unsigned int *buf_size)
{
	ExifMnoteDataOlympus *n = (ExifMnoteDataOlympus *) ne;
	size_t i, o, s, doff, base = 0, o2 = 6 + 2;
	size_t datao = 0;
	unsigned char *t;
	size_t ts;

	if (!n || !buf || !buf_size) return;

	/*
	 * Allocate enough memory for all entries and the number of entries.
	 */
	*buf_size = 6 + 2 + 2 + n->count * 12;
	switch (n->version) {
	case olympusV1:
	case sanyoV1:
	case epsonV1:
		*buf = exif_mem_alloc (ne->mem, *buf_size);
		if (!*buf) {
			EXIF_LOG_NO_MEMORY(ne->log, "ExifMnoteDataOlympus", *buf_size);
			return;
		}

		/* Write the header and the number of entries. */
		strcpy ((char *)*buf, n->version==sanyoV1?"SANYO":
					(n->version==epsonV1?"EPSON":"OLYMP"));
		exif_set_short (*buf + 6, n->order, (ExifShort) 1);
		datao = n->offset;
		break;

	case olympusV2:
		*buf_size += 8-6 + 4;
		*buf = exif_mem_alloc (ne->mem, *buf_size);
		if (!*buf) {
			EXIF_LOG_NO_MEMORY(ne->log, "ExifMnoteDataOlympus", *buf_size);
			return;
		}

		/* Write the header and the number of entries. */
		strcpy ((char *)*buf, "OLYMPUS");
		exif_set_short (*buf + 8, n->order, (ExifShort) (
			(n->order == EXIF_BYTE_ORDER_INTEL) ?
			('I' << 8) | 'I' :
			('M' << 8) | 'M'));
		exif_set_short (*buf + 10, n->order, (ExifShort) 3);
		o2 += 4;
		break;

	case nikonV1: 
		base = MNOTE_NIKON1_TAG_BASE;

		/* v1 has offsets based to main IFD, not makernote IFD */
		datao += n->offset + 10;
		/* subtract the size here, so the increment in the next case will not harm us */
		*buf_size -= 8 + 2;
		/* fall through */
	/* Fall through to nikonV2 handler */
	case nikonV2: 
	/* Write out V0 files in V2 format */
	case nikonV0: 
		*buf_size += 8 + 2;
		*buf_size += 4; /* Next IFD pointer */
		*buf = exif_mem_alloc (ne->mem, *buf_size);
		if (!*buf) {
			EXIF_LOG_NO_MEMORY(ne->log, "ExifMnoteDataOlympus", *buf_size);
			return;
		}

		/* Write the header and the number of entries. */
		strcpy ((char *)*buf, "Nikon");
		(*buf)[6] = n->version;

		if (n->version != nikonV1) {
			exif_set_short (*buf + 10, n->order, (ExifShort) (
				(n->order == EXIF_BYTE_ORDER_INTEL) ?
				('I' << 8) | 'I' :
				('M' << 8) | 'M'));
			exif_set_short (*buf + 12, n->order, (ExifShort) 0x2A);
			exif_set_long (*buf + 14, n->order, (ExifShort) 8);
			o2 += 2 + 8;
		}
		datao -= 10;
		/* Reset next IFD pointer */
		exif_set_long (*buf + o2 + 2 + n->count * 12, n->order, 0);
		break;

	default:
		return;
	}

	exif_set_short (*buf + o2, n->order, (ExifShort) n->count);
	o2 += 2;

	/* Save each entry */
	for (i = 0; i < n->count; i++) {
		o = o2 + i * 12;
		exif_set_short (*buf + o + 0, n->order,
				(ExifShort) (n->entries[i].tag - base));
		exif_set_short (*buf + o + 2, n->order,
				(ExifShort) n->entries[i].format);
		exif_set_long  (*buf + o + 4, n->order,
				n->entries[i].components);
		o += 8;
		s = exif_format_get_size (n->entries[i].format) *
						n->entries[i].components;
		if (s > 65536) {
			/* Corrupt data: EXIF data size is limited to the
			 * maximum size of a JPEG segment (64 kb).
			 */
			continue;
		}
		if (s > 4) {
			doff = *buf_size;
			ts = *buf_size + s;
			t = exif_mem_realloc (ne->mem, *buf,
						 sizeof (char) * ts);
			if (!t) {
				EXIF_LOG_NO_MEMORY(ne->log, "ExifMnoteDataOlympus", ts);
				return;
			}
			*buf = t;
			*buf_size = ts;
			exif_set_long (*buf + o, n->order, datao + doff);
		} else
			doff = o;

		/* Write the data. */
		if (n->entries[i].data) {
			memcpy (*buf + doff, n->entries[i].data, s);
		} else {
			/* Most certainly damaged input file */
			memset (*buf + doff, 0, s);
		}
	}
}

static void
exif_mnote_data_olympus_load (ExifMnoteData *en,
			      const unsigned char *buf, unsigned int buf_size)
{
	ExifMnoteDataOlympus *n = (ExifMnoteDataOlympus *) en;
	ExifShort c;
	size_t i, tcount, o, o2, datao = 6, base = 0;

	if (!n || !buf || !buf_size) {
		exif_log (en->log, EXIF_LOG_CODE_CORRUPT_DATA,
			  "ExifMnoteDataOlympus", "Short MakerNote");
		return;
	}
	o2 = 6 + n->offset; /* Start of interesting data */
	if (CHECKOVERFLOW(o2,buf_size,10)) {
		exif_log (en->log, EXIF_LOG_CODE_CORRUPT_DATA,
			  "ExifMnoteDataOlympus", "Short MakerNote");
		return;
	}

	/*
	 * Olympus headers start with "OLYMP" and need to have at least
	 * a size of 22 bytes (6 for 'OLYMP', 2 other bytes, 2 for the
	 * number of entries, and 12 for one entry.
	 *
	 * Sanyo format is identical and uses identical tags except that
	 * header starts with "SANYO".
	 *
	 * Epson format is identical and uses identical tags except that
	 * header starts with "EPSON".
	 *
	 * Nikon headers start with "Nikon" (6 bytes including '\0'), 
	 * version number (1 or 2).
	 * 
	 * Version 1 continues with 0, 1, 0, number_of_tags,
	 * or just with number_of_tags (models D1H, D1X...).
	 * 
	 * Version 2 continues with an unknown byte (0 or 10),
	 * two unknown bytes (0), "MM" or "II", another byte 0 and 
	 * lastly 0x2A.
	 */
	n->version = exif_mnote_data_olympus_identify_variant(buf+o2, buf_size-o2);
	switch (n->version) {
	case olympusV1:
	case sanyoV1:
	case epsonV1:
		exif_log (en->log, EXIF_LOG_CODE_DEBUG, "ExifMnoteDataOlympus",
			"Parsing Olympus/Sanyo/Epson maker note v1...");

		/* The number of entries is at position 8. */
		if (buf[o2 + 6] == 1)
			n->order = EXIF_BYTE_ORDER_INTEL;
		else if (buf[o2 + 6 + 1] == 1)
			n->order = EXIF_BYTE_ORDER_MOTOROLA;
		o2 += 8;
		c = exif_get_short (buf + o2, n->order);
		if ((!(c & 0xFF)) && (c > 0x500)) {
			if (n->order == EXIF_BYTE_ORDER_INTEL) {
				n->order = EXIF_BYTE_ORDER_MOTOROLA;
			} else {
				n->order = EXIF_BYTE_ORDER_INTEL;
			}
		}
		break;

	case olympusV2:
		/* Olympus S760, S770 */
		datao = o2;
		o2 += 8;
		if (CHECKOVERFLOW(o2,buf_size,4)) return;
		exif_log (en->log, EXIF_LOG_CODE_DEBUG, "ExifMnoteDataOlympus",
			"Parsing Olympus maker note v2 (0x%02x, %02x, %02x, %02x)...",
			buf[o2 + 0], buf[o2 + 1], buf[o2 + 2], buf[o2 + 3]);

		if ((buf[o2] == 'I') && (buf[o2 + 1] == 'I'))
			n->order = EXIF_BYTE_ORDER_INTEL;
		else if ((buf[o2] == 'M') && (buf[o2 + 1] == 'M'))
			n->order = EXIF_BYTE_ORDER_MOTOROLA;

		/* The number of entries is at position 8+4. */
		o2 += 4;
		break;

	case nikonV1:
		o2 += 6;
		exif_log (en->log, EXIF_LOG_CODE_DEBUG, "ExifMnoteDataOlympus",
			"Parsing Nikon maker note v1 (0x%02x, %02x, %02x, "
			"%02x)...",
			buf[o2 + 0], buf[o2 + 1], buf[o2 + 2], buf[o2 + 3]);

		/* Skip version number */
		o2 += 1;

		/* Skip an unknown byte (00 or 0A). */
		o2 += 1;

		base = MNOTE_NIKON1_TAG_BASE;
		/* Fix endianness, if needed */
		c = exif_get_short (buf + o2, n->order);
		if ((!(c & 0xFF)) && (c > 0x500)) {
			if (n->order == EXIF_BYTE_ORDER_INTEL) {
				n->order = EXIF_BYTE_ORDER_MOTOROLA;
			} else {
				n->order = EXIF_BYTE_ORDER_INTEL;
			}
		}
		break;

	case nikonV2:
		o2 += 6;
		if (CHECKOVERFLOW(o2,buf_size,12)) return;
		exif_log (en->log, EXIF_LOG_CODE_DEBUG, "ExifMnoteDataOlympus",
			"Parsing Nikon maker note v2 (0x%02x, %02x, %02x, "
			"%02x, %02x, %02x, %02x, %02x)...",
			buf[o2 + 0], buf[o2 + 1], buf[o2 + 2], buf[o2 + 3],
			buf[o2 + 4], buf[o2 + 5], buf[o2 + 6], buf[o2 + 7]);

		/* Skip version number */
		o2 += 1;

		/* Skip an unknown byte (00 or 0A). */
		o2 += 1;

		/* Skip 2 unknown bytes (00 00). */
		o2 += 2;

		/*
		 * Byte order. From here the data offset
		 * gets calculated.
		 */
		datao = o2;
		if (!strncmp ((char *)&buf[o2], "II", 2))
			n->order = EXIF_BYTE_ORDER_INTEL;
		else if (!strncmp ((char *)&buf[o2], "MM", 2))
			n->order = EXIF_BYTE_ORDER_MOTOROLA;
		else {
			exif_log (en->log, EXIF_LOG_CODE_DEBUG,
				"ExifMnoteDataOlympus", "Unknown "
				"byte order '%c%c'", buf[o2],
				buf[o2 + 1]);
			return;
		}
		o2 += 2;

		/* Skip 2 unknown bytes (00 2A). */
		o2 += 2;

		/* Go to where the number of entries is. */
		o2 = datao + exif_get_long (buf + o2, n->order);
		break;

	case nikonV0:
		exif_log (en->log, EXIF_LOG_CODE_DEBUG, "ExifMnoteDataOlympus",
			"Parsing Nikon maker note v0 (0x%02x, %02x, %02x, "
			"%02x, %02x, %02x, %02x, %02x)...",
			buf[o2 + 0], buf[o2 + 1], buf[o2 + 2], buf[o2 + 3], 
			buf[o2 + 4], buf[o2 + 5], buf[o2 + 6], buf[o2 + 7]);
		/* 00 1b is # of entries in Motorola order - the rest should also be in MM order */
		n->order = EXIF_BYTE_ORDER_MOTOROLA;
		break;

	default:
		exif_log (en->log, EXIF_LOG_CODE_DEBUG, "ExifMnoteDataOlympus",
			"Unknown Olympus variant %i.", n->version);
		return;
	}

	/* Sanity check the offset */
	if (CHECKOVERFLOW(o2,buf_size,2)) {
		exif_log (en->log, EXIF_LOG_CODE_CORRUPT_DATA,
			  "ExifMnoteOlympus", "Short MakerNote");
		return;
	}

	/* Read the number of tags */
	c = exif_get_short (buf + o2, n->order);
	o2 += 2;

	/* Just use an arbitrary max tag limit here to avoid needing to much memory or time. There are 150 named tags currently.
	 * The format allows specifying the same range of memory as often as it can, so this multiplies quickly. */
	if (c > 300) {
		exif_log (en->log, EXIF_LOG_CODE_CORRUPT_DATA, "ExifMnoteOlympus", "Too much tags (%d) in Olympus MakerNote", c);
		return;
	}

	/* Remove any old entries */
	exif_mnote_data_olympus_clear (n);

	/* Reserve enough space for all the possible MakerNote tags */
	n->entries = exif_mem_alloc (en->mem, sizeof (MnoteOlympusEntry) * c);
	if (!n->entries) {
		EXIF_LOG_NO_MEMORY(en->log, "ExifMnoteOlympus", sizeof (MnoteOlympusEntry) * c);
		return;
	}

	/* Parse all c entries, storing ones that are successfully parsed */
	tcount = 0;
	for (i = c, o = o2; i; --i, o += 12) {
		size_t s;
		memset(&n->entries[tcount], 0, sizeof(MnoteOlympusEntry));
		if (CHECKOVERFLOW(o, buf_size, 12)) {
			exif_log (en->log, EXIF_LOG_CODE_CORRUPT_DATA,
				  "ExifMnoteOlympus", "Short MakerNote");
			break;
		}

	    n->entries[tcount].tag        = exif_get_short (buf + o, n->order) + base;
	    n->entries[tcount].format     = exif_get_short (buf + o + 2, n->order);
	    n->entries[tcount].components = exif_get_long (buf + o + 4, n->order);
	    n->entries[tcount].order      = n->order;

	    exif_log (en->log, EXIF_LOG_CODE_DEBUG, "ExifMnoteOlympus",
		      "Loading entry 0x%x ('%s')...", n->entries[tcount].tag,
		      mnote_olympus_tag_get_name (n->entries[tcount].tag));
/*	    exif_log (en->log, EXIF_LOG_CODE_DEBUG, "ExifMnoteOlympus",
			    "0x%x %d %ld*(%d)",
		    n->entries[tcount].tag,
		    n->entries[tcount].format,
		    n->entries[tcount].components,
		    (int)exif_format_get_size(n->entries[tcount].format)); */

	    /* Check if we overflow the multiplication. Use buf_size as the max size for integer overflow detection,
	     * we will check the buffer sizes closer later. */
	    if (exif_format_get_size (n->entries[tcount].format) &&
		buf_size / exif_format_get_size (n->entries[tcount].format) < n->entries[tcount].components
	    ) {
		exif_log (en->log, EXIF_LOG_CODE_CORRUPT_DATA, "ExifMnoteOlympus", "Tag size overflow detected (%u * %lu)", exif_format_get_size (n->entries[tcount].format), n->entries[tcount].components);
		continue;
	    }
	    /*
	     * Size? If bigger than 4 bytes, the actual data is not
	     * in the entry but somewhere else (offset).
	     */
	    s = exif_format_get_size (n->entries[tcount].format) *
		   			 n->entries[tcount].components;
		n->entries[tcount].size = s;
		if (s) {
			size_t dataofs = o + 8;
			if (s > 4) {
				/* The data in this case is merely a pointer */
				dataofs = exif_get_long (buf + dataofs, n->order) + datao;
#ifdef EXIF_OVERCOME_SANYO_OFFSET_BUG
				/* Some Sanyo models (e.g. VPC-C5, C40) suffer from a bug when
				 * writing the offset for the MNOTE_OLYMPUS_TAG_THUMBNAILIMAGE
				 * tag in its MakerNote. The offset is actually the absolute
				 * position in the file instead of the position within the IFD.
				 */
			    if (dataofs > (buf_size - s) && n->version == sanyoV1) {
					/* fix pointer */
					dataofs -= datao + 6;
					exif_log (en->log, EXIF_LOG_CODE_DEBUG,
						  "ExifMnoteOlympus",
						  "Inconsistent thumbnail tag offset; attempting to recover");
			    }
#endif
			}
			if (CHECKOVERFLOW(dataofs, buf_size, s)) {
				exif_log (en->log, EXIF_LOG_CODE_DEBUG,
					  "ExifMnoteOlympus",
					  "Tag data past end of buffer (%u > %u)",
					  (unsigned)(dataofs + s), buf_size);
				continue;
			}

			n->entries[tcount].data = exif_mem_alloc (en->mem, s);
			if (!n->entries[tcount].data) {
				EXIF_LOG_NO_MEMORY(en->log, "ExifMnoteOlympus", s);
				continue;
			}
			memcpy (n->entries[tcount].data, buf + dataofs, s);
		}

		/* Tag was successfully parsed */
		++tcount;
	}
	/* Store the count of successfully parsed tags */
	n->count = tcount;
}

static unsigned int
exif_mnote_data_olympus_count (ExifMnoteData *n)
{
	return n ? ((ExifMnoteDataOlympus *) n)->count : 0;
}

static unsigned int
exif_mnote_data_olympus_get_id (ExifMnoteData *d, unsigned int n)
{
	ExifMnoteDataOlympus *note = (ExifMnoteDataOlympus *) d;

	if (!note) return 0;
	if (note->count <= n) return 0;
	return note->entries[n].tag;
}

static const char *
exif_mnote_data_olympus_get_name (ExifMnoteData *d, unsigned int i)
{
	ExifMnoteDataOlympus *n = (ExifMnoteDataOlympus *) d;

	if (!n) return NULL;
	if (i >= n->count) return NULL;
	return mnote_olympus_tag_get_name (n->entries[i].tag);
}

static const char *
exif_mnote_data_olympus_get_title (ExifMnoteData *d, unsigned int i)
{
	ExifMnoteDataOlympus *n = (ExifMnoteDataOlympus *) d;
	
	if (!n) return NULL;
	if (i >= n->count) return NULL;
        return mnote_olympus_tag_get_title (n->entries[i].tag);
}

static const char *
exif_mnote_data_olympus_get_description (ExifMnoteData *d, unsigned int i)
{
	ExifMnoteDataOlympus *n = (ExifMnoteDataOlympus *) d;
	
	if (!n) return NULL;
	if (i >= n->count) return NULL;
        return mnote_olympus_tag_get_description (n->entries[i].tag);
}

static void
exif_mnote_data_olympus_set_byte_order (ExifMnoteData *d, ExifByteOrder o)
{
	ExifByteOrder o_orig;
	ExifMnoteDataOlympus *n = (ExifMnoteDataOlympus *) d;
	unsigned int i;

	if (!n) return;

	o_orig = n->order;
	n->order = o;
	for (i = 0; i < n->count; i++) {
		if (n->entries[i].components && (n->entries[i].size/n->entries[i].components < exif_format_get_size (n->entries[i].format)))
			continue;
		n->entries[i].order = o;
		exif_array_set_byte_order (n->entries[i].format, n->entries[i].data,
				n->entries[i].components, o_orig, o);
	}
}

static void
exif_mnote_data_olympus_set_offset (ExifMnoteData *n, unsigned int o)
{
	if (n) ((ExifMnoteDataOlympus *) n)->offset = o;
}

static enum OlympusVersion
exif_mnote_data_olympus_identify_variant (const unsigned char *buf,
		unsigned int buf_size)
{
	/* Olympus, Nikon, Sanyo, Epson */
	if (buf_size >= 8) {
		/* Match the terminating NUL character, too */
		if (!memcmp (buf, "OLYMPUS", 8))
			   return olympusV2;
		else if (!memcmp (buf, "OLYMP", 6))
			   return olympusV1;
		else if (!memcmp (buf, "SANYO", 6))
			   return sanyoV1;
		else if (!memcmp (buf, "EPSON", 6))
			   return epsonV1;
		else if (!memcmp (buf, "Nikon", 6)) {
			switch (buf[6]) {
				case 1:  return nikonV1;
				case 2:  return nikonV2;
				default: return 0; /* Unrecognized Nikon variant */
			}
		}
	}

	/* Another variant of Nikon */
	if ((buf_size >= 2) && (buf[0] == 0x00) && (buf[1] == 0x1b)) {
		return nikonV0;
	}

	return unrecognized;
}

int
exif_mnote_data_olympus_identify (const ExifData *ed, const ExifEntry *e)
{
	int variant = exif_mnote_data_olympus_identify_variant(e->data, e->size);

	if (variant == nikonV0) {
		/* This variant needs some extra checking with the Make */
		char value[5];
		ExifEntry *em = exif_data_get_entry (ed, EXIF_TAG_MAKE);
		variant = unrecognized;

		if (em) {
			const char *v = exif_entry_get_value (em, value, sizeof(value));
			if (v && (!strncmp (v, "Nikon", sizeof(value)) || 
					  !strncmp (v, "NIKON", sizeof(value)) ))
				/* When saved, this variant will be written out like the
				 * alternative nikonV2 form above instead
				 */
				variant = nikonV0;
		}
	}

	return variant;
}


ExifMnoteData *
exif_mnote_data_olympus_new (ExifMem *mem)
{
	ExifMnoteData *d;

	if (!mem) return NULL;
	
	d = exif_mem_alloc (mem, sizeof (ExifMnoteDataOlympus));
	if (!d) return NULL;

	exif_mnote_data_construct (d, mem);

	/* Set up function pointers */
	d->methods.free            = exif_mnote_data_olympus_free;
	d->methods.set_byte_order  = exif_mnote_data_olympus_set_byte_order;
	d->methods.set_offset      = exif_mnote_data_olympus_set_offset;
	d->methods.load            = exif_mnote_data_olympus_load;
	d->methods.save            = exif_mnote_data_olympus_save;
	d->methods.count           = exif_mnote_data_olympus_count;
	d->methods.get_id          = exif_mnote_data_olympus_get_id;
	d->methods.get_name        = exif_mnote_data_olympus_get_name;
	d->methods.get_title       = exif_mnote_data_olympus_get_title;
	d->methods.get_description = exif_mnote_data_olympus_get_description;
	d->methods.get_value       = exif_mnote_data_olympus_get_value;

	return d;
}
