// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2013, Google Inc.
 * Written by Simon Glass <sjg@chromium.org>
 *
 * Perform a grep of an FDT either displaying the source subset or producing
 * a new .dtb subset which can be used as required.
 */

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <getopt.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "fdt_host.h"
#include "libfdt_internal.h"

/* Define DEBUG to get some debugging output on stderr */
#ifdef DEBUG
#define debug(a, b...) fprintf(stderr, a, ## b)
#else
#define debug(a, b...)
#endif

/* A linked list of values we are grepping for */
struct value_node {
	int type;		/* Types this value matches (FDT_IS... mask) */
	int include;		/* 1 to include matches, 0 to exclude */
	const char *string;	/* String to match */
	struct value_node *next;	/* Pointer to next node, or NULL */
};

/* Output formats we support */
enum output_t {
	OUT_DTS,		/* Device tree source */
	OUT_DTB,		/* Valid device tree binary */
	OUT_BIN,		/* Fragment of .dtb, for hashing */
};

/* Holds information which controls our output and options */
struct display_info {
	enum output_t output;	/* Output format */
	int add_aliases;	/* Add aliases node to output */
	int all;		/* Display all properties/nodes */
	int colour;		/* Display output in ANSI colour */
	int region_list;	/* Output a region list */
	int flags;		/* Flags (FDT_REG_...) */
	int list_strings;	/* List strings in string table */
	int show_offset;	/* Show offset */
	int show_addr;		/* Show address */
	int header;		/* Output an FDT header */
	int diff;		/* Show +/- diff markers */
	int include_root;	/* Include the root node and all properties */
	int remove_strings;	/* Remove unused strings */
	int show_dts_version;	/* Put '/dts-v1/;' on the first line */
	int types_inc;		/* Mask of types that we include (FDT_IS...) */
	int types_exc;		/* Mask of types that we exclude (FDT_IS...) */
	int invert;		/* Invert polarity of match */
	struct value_node *value_head;	/* List of values to match */
	const char *output_fname;	/* Output filename */
	FILE *fout;		/* File to write dts/dtb output */
};

static void report_error(const char *where, int err)
{
	fprintf(stderr, "Error at '%s': %s\n", where, fdt_strerror(err));
}

/* Supported ANSI colours */
enum {
	COL_BLACK,
	COL_RED,
	COL_GREEN,
	COL_YELLOW,
	COL_BLUE,
	COL_MAGENTA,
	COL_CYAN,
	COL_WHITE,

	COL_NONE = -1,
};

/**
 * print_ansi_colour() - Print out the ANSI sequence for a colour
 *
 * @fout:	Output file
 * @col:	Colour to output (COL_...), or COL_NONE to reset colour
 */
static void print_ansi_colour(FILE *fout, int col)
{
	if (col == COL_NONE)
		fprintf(fout, "\033[0m");
	else
		fprintf(fout, "\033[1;%dm", col + 30);
}


/**
 * value_add() - Add a new value to our list of things to grep for
 *
 * @disp:	Display structure, holding info about our options
 * @headp:	Pointer to header pointer of list
 * @type:	Type of this value (FDT_IS_...)
 * @include:	1 if we want to include matches, 0 to exclude
 * @str:	String value to match
 */
static int value_add(struct display_info *disp, struct value_node **headp,
		     int type, int include, const char *str)
{
	struct value_node *node;

	/*
	 * Keep track of which types we are excluding/including. We don't
	 * allow both including and excluding things, because it doesn't make
	 * sense. 'Including' means that everything not mentioned is
	 * excluded. 'Excluding' means that everything not mentioned is
	 * included. So using the two together would be meaningless.
	 */
	if (include)
		disp->types_inc |= type;
	else
		disp->types_exc |= type;
	if (disp->types_inc & disp->types_exc & type) {
		fprintf(stderr,
			"Cannot use both include and exclude for '%s'\n", str);
		return -1;
	}

	str = strdup(str);
	if (!str)
		goto err_mem;
	node = malloc(sizeof(*node));
	if (!node)
		goto err_mem;
	node->next = *headp;
	node->type = type;
	node->include = include;
	node->string = str;
	*headp = node;

	return 0;
err_mem:
	fprintf(stderr, "Out of memory\n");
	return -1;
}

static bool util_is_printable_string(const void *data, int len)
{
	const char *s = data;
	const char *ss, *se;

	/* zero length is not */
	if (len == 0)
		return 0;

	/* must terminate with zero */
	if (s[len - 1] != '\0')
		return 0;

	se = s + len;

	while (s < se) {
		ss = s;
		while (s < se && *s && isprint((unsigned char)*s))
			s++;

		/* not zero, or not done yet */
		if (*s != '\0' || s == ss)
			return 0;

		s++;
	}

	return 1;
}

static void utilfdt_print_data(const char *data, int len)
{
	int i;
	const char *p = data;
	const char *s;

	/* no data, don't print */
	if (len == 0)
		return;

	if (util_is_printable_string(data, len)) {
		printf(" = ");

		s = data;
		do {
			printf("\"%s\"", s);
			s += strlen(s) + 1;
			if (s < data + len)
				printf(", ");
		} while (s < data + len);

	} else if ((len % 4) == 0) {
		const uint32_t *cell = (const uint32_t *)data;

		printf(" = <");
		for (i = 0, len /= 4; i < len; i++)
			printf("0x%08x%s", fdt32_to_cpu(cell[i]),
			       i < (len - 1) ? " " : "");
		printf(">");
	} else {
		printf(" = [");
		for (i = 0; i < len; i++)
			printf("%02x%s", *p++, i < len - 1 ? " " : "");
		printf("]");
	}
}

/**
 * display_fdt_by_regions() - Display regions of an FDT source
 *
 * This dumps an FDT as source, but only certain regions of it. This is the
 * final stage of the grep - we have a list of regions we want to display,
 * and this function displays them.
 *
 * @disp:	Display structure, holding info about our options
 * @blob:	FDT blob to display
 * @region:	List of regions to display
 * @count:	Number of regions
 */
static int display_fdt_by_regions(struct display_info *disp, const void *blob,
		struct fdt_region region[], int count)
{
	struct fdt_region *reg = region, *reg_end = region + count;
	uint32_t off_mem_rsvmap = fdt_off_mem_rsvmap(blob);
	int base = fdt_off_dt_struct(blob);
	int version = fdt_version(blob);
	int offset, nextoffset;
	int tag, depth, shift;
	FILE *f = disp->fout;
	uint64_t addr, size;
	int in_region;
	int file_ofs;
	int i;

	if (disp->show_dts_version)
		fprintf(f, "/dts-v1/;\n");

	if (disp->header) {
		fprintf(f, "// magic:\t\t0x%x\n", fdt_magic(blob));
		fprintf(f, "// totalsize:\t\t0x%x (%d)\n", fdt_totalsize(blob),
			fdt_totalsize(blob));
		fprintf(f, "// off_dt_struct:\t0x%x\n",
			fdt_off_dt_struct(blob));
		fprintf(f, "// off_dt_strings:\t0x%x\n",
			fdt_off_dt_strings(blob));
		fprintf(f, "// off_mem_rsvmap:\t0x%x\n", off_mem_rsvmap);
		fprintf(f, "// version:\t\t%d\n", version);
		fprintf(f, "// last_comp_version:\t%d\n",
			fdt_last_comp_version(blob));
		if (version >= 2) {
			fprintf(f, "// boot_cpuid_phys:\t0x%x\n",
				fdt_boot_cpuid_phys(blob));
		}
		if (version >= 3) {
			fprintf(f, "// size_dt_strings:\t0x%x\n",
				fdt_size_dt_strings(blob));
		}
		if (version >= 17) {
			fprintf(f, "// size_dt_struct:\t0x%x\n",
				fdt_size_dt_struct(blob));
		}
		fprintf(f, "\n");
	}

	if (disp->flags & FDT_REG_ADD_MEM_RSVMAP) {
		const struct fdt_reserve_entry *p_rsvmap;

		p_rsvmap = (const struct fdt_reserve_entry *)
				((const char *)blob + off_mem_rsvmap);
		for (i = 0; ; i++) {
			addr = fdt64_to_cpu(p_rsvmap[i].address);
			size = fdt64_to_cpu(p_rsvmap[i].size);
			if (addr == 0 && size == 0)
				break;

			fprintf(f, "/memreserve/ %llx %llx;\n",
				(unsigned long long)addr,
				(unsigned long long)size);
		}
	}

	depth = 0;
	nextoffset = 0;
	shift = 4;	/* 4 spaces per indent */
	do {
		const struct fdt_property *prop;
		const char *name;
		int show;
		int len;

		offset = nextoffset;

		/*
		 * Work out the file offset of this offset, and decide
		 * whether it is in the region list or not
		 */
		file_ofs = base + offset;
		if (reg < reg_end && file_ofs >= reg->offset + reg->size)
			reg++;
		in_region = reg < reg_end && file_ofs >= reg->offset &&
				file_ofs < reg->offset + reg->size;
		tag = fdt_next_tag(blob, offset, &nextoffset);

		if (tag == FDT_END)
			break;
		show = in_region || disp->all;
		if (show && disp->diff)
			fprintf(f, "%c", in_region ? '+' : '-');

		if (!show) {
			/* Do this here to avoid 'if (show)' in every 'case' */
			if (tag == FDT_BEGIN_NODE)
				depth++;
			else if (tag == FDT_END_NODE)
				depth--;
			continue;
		}
		if (tag != FDT_END) {
			if (disp->show_addr)
				fprintf(f, "%4x: ", file_ofs);
			if (disp->show_offset)
				fprintf(f, "%4x: ", file_ofs - base);
		}

		/* Green means included, red means excluded */
		if (disp->colour)
			print_ansi_colour(f, in_region ? COL_GREEN : COL_RED);

		switch (tag) {
		case FDT_PROP:
			prop = fdt_get_property_by_offset(blob, offset, NULL);
			name = fdt_string(blob, fdt32_to_cpu(prop->nameoff));
			fprintf(f, "%*s%s", depth * shift, "", name);
			utilfdt_print_data(prop->data,
					   fdt32_to_cpu(prop->len));
			fprintf(f, ";");
			break;

		case FDT_NOP:
			fprintf(f, "%*s// [NOP]", depth * shift, "");
			break;

		case FDT_BEGIN_NODE:
			name = fdt_get_name(blob, offset, &len);
			fprintf(f, "%*s%s {", depth++ * shift, "",
				*name ? name : "/");
			break;

		case FDT_END_NODE:
			fprintf(f, "%*s};", --depth * shift, "");
			break;
		}

		/* Reset colour back to normal before end of line */
		if (disp->colour)
			print_ansi_colour(f, COL_NONE);
		fprintf(f, "\n");
	} while (1);

	/* Print a list of strings if requested */
	if (disp->list_strings) {
		const char *str;
		int str_base = fdt_off_dt_strings(blob);

		for (offset = 0; offset < fdt_size_dt_strings(blob);
				offset += strlen(str) + 1) {
			str = fdt_string(blob, offset);
			int len = strlen(str) + 1;
			int show;

			/* Only print strings that are in the region */
			file_ofs = str_base + offset;
			in_region = reg < reg_end &&
					file_ofs >= reg->offset &&
					file_ofs + len < reg->offset +
						reg->size;
			show = in_region || disp->all;
			if (show && disp->diff)
				printf("%c", in_region ? '+' : '-');
			if (disp->show_addr)
				printf("%4x: ", file_ofs);
			if (disp->show_offset)
				printf("%4x: ", offset);
			printf("%s\n", str);
		}
	}

	return 0;
}

/**
 * dump_fdt_regions() - Dump regions of an FDT as binary data
 *
 * This dumps an FDT as binary, but only certain regions of it. This is the
 * final stage of the grep - we have a list of regions we want to dump,
 * and this function dumps them.
 *
 * The output of this function may or may not be a valid FDT. To ensure it
 * is, these disp->flags must be set:
 *
 *   FDT_REG_SUPERNODES: ensures that subnodes are preceded by their
 *		parents. Without this option, fragments of subnode data may be
 *		output without the supernodes above them. This is useful for
 *		hashing but cannot produce a valid FDT.
 *   FDT_REG_ADD_STRING_TAB: Adds a string table to the end of the FDT.
 *		Without this none of the properties will have names
 *   FDT_REG_ADD_MEM_RSVMAP: Adds a mem_rsvmap table - an FDT is invalid
 *		without this.
 *
 * @disp:	Display structure, holding info about our options
 * @blob:	FDT blob to display
 * @region:	List of regions to display
 * @count:	Number of regions
 * @out:	Output destination
 */
static int dump_fdt_regions(struct display_info *disp, const void *blob,
		struct fdt_region region[], int count, char *out)
{
	struct fdt_header *fdt;
	int size, struct_start;
	int ptr;
	int i;

	/* Set up a basic header (even if we don't actually write it) */
	fdt = (struct fdt_header *)out;
	memset(fdt, '\0', sizeof(*fdt));
	fdt_set_magic(fdt, FDT_MAGIC);
	struct_start = FDT_ALIGN(sizeof(struct fdt_header),
					sizeof(struct fdt_reserve_entry));
	fdt_set_off_mem_rsvmap(fdt, struct_start);
	fdt_set_version(fdt, FDT_LAST_SUPPORTED_VERSION);
	fdt_set_last_comp_version(fdt, FDT_FIRST_SUPPORTED_VERSION);

	/*
	 * Calculate the total size of the regions we are writing out. The
	 * first will be the mem_rsvmap if the FDT_REG_ADD_MEM_RSVMAP flag
	 * is set. The last will be the string table if FDT_REG_ADD_STRING_TAB
	 * is set.
	 */
	for (i = size = 0; i < count; i++)
		size += region[i].size;

	/* Bring in the mem_rsvmap section from the old file if requested */
	if (count > 0 && (disp->flags & FDT_REG_ADD_MEM_RSVMAP)) {
		struct_start += region[0].size;
		size -= region[0].size;
	}
	fdt_set_off_dt_struct(fdt, struct_start);

	/* Update the header to have the correct offsets/sizes */
	if (count >= 2 && (disp->flags & FDT_REG_ADD_STRING_TAB)) {
		int str_size;

		str_size = region[count - 1].size;
		fdt_set_size_dt_struct(fdt, size - str_size);
		fdt_set_off_dt_strings(fdt, struct_start + size - str_size);
		fdt_set_size_dt_strings(fdt, str_size);
		fdt_set_totalsize(fdt, struct_start + size);
	}

	/* Write the header if required */
	ptr = 0;
	if (disp->header) {
		ptr = sizeof(*fdt);
		while (ptr < fdt_off_mem_rsvmap(fdt))
			out[ptr++] = '\0';
	}

	/* Output all the nodes including any mem_rsvmap/string table */
	for (i = 0; i < count; i++) {
		struct fdt_region *reg = &region[i];

		memcpy(out + ptr, (const char *)blob + reg->offset, reg->size);
		ptr += reg->size;
	}

	return ptr;
}

/**
 * show_region_list() - Print out a list of regions
 *
 * The list includes the region offset (absolute offset from start of FDT
 * blob in bytes) and size
 *
 * @reg:	List of regions to print
 * @count:	Number of regions
 */
static void show_region_list(struct fdt_region *reg, int count)
{
	int i;

	printf("Regions: %d\n", count);
	for (i = 0; i < count; i++, reg++) {
		printf("%d:  %-10x  %-10x\n", i, reg->offset,
		       reg->offset + reg->size);
	}
}

static int check_type_include(void *priv, int type, const char *data, int size)
{
	struct display_info *disp = priv;
	struct value_node *val;
	int match, none_match = FDT_IS_ANY;

	/* If none of our conditions mention this type, we know nothing */
	debug("type=%x, data=%s\n", type, data ? data : "(null)");
	if (!((disp->types_inc | disp->types_exc) & type)) {
		debug("   - not in any condition\n");
		return -1;
	}

	/*
	 * Go through the list of conditions. For inclusive conditions, we
	 * return 1 at the first match. For exclusive conditions, we must
	 * check that there are no matches.
	 */
	if (data) {
		for (val = disp->value_head; val; val = val->next) {
			if (!(type & val->type))
				continue;
			match = fdt_stringlist_contains(data, size,
							val->string);
			debug("      - val->type=%x, str='%s', match=%d\n",
			      val->type, val->string, match);
			if (match && val->include) {
				debug("   - match inc %s\n", val->string);
				return 1;
			}
			if (match)
				none_match &= ~val->type;
		}
	}

	/*
	 * If this is an exclusive condition, and nothing matches, then we
	 * should return 1.
	 */
	if ((type & disp->types_exc) && (none_match & type)) {
		debug("   - match exc\n");
		/*
		 * Allow FDT_IS_COMPAT to make the final decision in the
		 * case where there is no specific type
		 */
		if (type == FDT_IS_NODE && disp->types_exc == FDT_ANY_GLOBAL) {
			debug("   - supressed exc node\n");
			return -1;
		}
		return 1;
	}

	/*
	 * Allow FDT_IS_COMPAT to make the final decision in the
	 * case where there is no specific type (inclusive)
	 */
	if (type == FDT_IS_NODE && disp->types_inc == FDT_ANY_GLOBAL)
		return -1;

	debug("   - no match, types_inc=%x, types_exc=%x, none_match=%x\n",
	      disp->types_inc, disp->types_exc, none_match);

	return 0;
}

/**
 * h_include() - Include handler function for fdt_find_regions()
 *
 * This function decides whether to include or exclude a node, property or
 * compatible string. The function is defined by fdt_find_regions().
 *
 * The algorithm is documented in the code - disp->invert is 0 for normal
 * operation, and 1 to invert the sense of all matches.
 *
 * See
 */
static int h_include(void *priv, const void *fdt, int offset, int type,
		     const char *data, int size)
{
	struct display_info *disp = priv;
	int inc, len;

	inc = check_type_include(priv, type, data, size);
	if (disp->include_root && type == FDT_IS_PROP && offset == 0 && inc)
		return 1;

	/*
	 * If the node name does not tell us anything, check the
	 * compatible string
	 */
	if (inc == -1 && type == FDT_IS_NODE) {
		debug("   - checking compatible2\n");
		data = fdt_getprop(fdt, offset, "compatible", &len);
		inc = check_type_include(priv, FDT_IS_COMPAT, data, len);
	}

	/* If we still have no idea, check for properties in the node */
	if (inc != 1 && type == FDT_IS_NODE &&
	    (disp->types_inc & FDT_NODE_HAS_PROP)) {
		debug("   - checking node '%s'\n",
		      fdt_get_name(fdt, offset, NULL));
		for (offset = fdt_first_property_offset(fdt, offset);
		     offset > 0 && inc != 1;
		     offset = fdt_next_property_offset(fdt, offset)) {
			const struct fdt_property *prop;
			const char *str;

			prop = fdt_get_property_by_offset(fdt, offset, NULL);
			if (!prop)
				continue;
			str = fdt_string(fdt, fdt32_to_cpu(prop->nameoff));
			inc = check_type_include(priv, FDT_NODE_HAS_PROP, str,
						 strlen(str));
		}
		if (inc == -1)
			inc = 0;
	}

	switch (inc) {
	case 1:
		inc = !disp->invert;
		break;
	case 0:
		inc = disp->invert;
		break;
	}
	debug("   - returning %d\n", inc);

	return inc;
}

static int h_cmp_region(const void *v1, const void *v2)
{
	const struct fdt_region *region1 = v1, *region2 = v2;

	return region1->offset - region2->offset;
}

static int fdtgrep_find_regions(const void *fdt,
		int (*include_func)(void *priv, const void *fdt, int offset,
				 int type, const char *data, int size),
		struct display_info *disp, struct fdt_region *region,
		int max_regions, char *path, int path_len, int flags)
{
	struct fdt_region_state state;
	int count;
	int ret;

	count = 0;
	ret = fdt_first_region(fdt, include_func, disp,
			&region[count++], path, path_len,
			disp->flags, &state);
	while (ret == 0) {
		ret = fdt_next_region(fdt, include_func, disp,
				count < max_regions ? &region[count] : NULL,
				path, path_len, disp->flags, &state);
		if (!ret)
			count++;
	}
	if (ret && ret != -FDT_ERR_NOTFOUND)
		return ret;

	/* Find all the aliases and add those regions back in */
	if (disp->add_aliases && count < max_regions) {
		int new_count;

		new_count = fdt_add_alias_regions(fdt, region, count,
						  max_regions, &state);
		if (new_count == -FDT_ERR_NOTFOUND) {
			/* No alias node found */
		} else if (new_count < 0) {
			return new_count;
		} else if (new_count <= max_regions) {
			/*
			* The alias regions will now be at the end of the list.
			* Sort the regions by offset to get things into the
			* right order
			*/
			count = new_count;
			qsort(region, count, sizeof(struct fdt_region),
			      h_cmp_region);
		}
	}

	return count;
}

int utilfdt_read_err_len(const char *filename, char **buffp, off_t *len)
{
	int fd = 0;	/* assume stdin */
	char *buf = NULL;
	off_t bufsize = 1024, offset = 0;
	int ret = 0;

	*buffp = NULL;
	if (strcmp(filename, "-") != 0) {
		fd = open(filename, O_RDONLY);
		if (fd < 0)
			return errno;
	}

	/* Loop until we have read everything */
	buf = malloc(bufsize);
	if (!buf)
		return -ENOMEM;
	do {
		/* Expand the buffer to hold the next chunk */
		if (offset == bufsize) {
			bufsize *= 2;
			buf = realloc(buf, bufsize);
			if (!buf)
				return -ENOMEM;
		}

		ret = read(fd, &buf[offset], bufsize - offset);
		if (ret < 0) {
			ret = errno;
			break;
		}
		offset += ret;
	} while (ret != 0);

	/* Clean up, including closing stdin; return errno on error */
	close(fd);
	if (ret)
		free(buf);
	else
		*buffp = buf;
	*len = bufsize;
	return ret;
}

int utilfdt_read_err(const char *filename, char **buffp)
{
	off_t len;
	return utilfdt_read_err_len(filename, buffp, &len);
}

char *utilfdt_read_len(const char *filename, off_t *len)
{
	char *buff;
	int ret = utilfdt_read_err_len(filename, &buff, len);

	if (ret) {
		fprintf(stderr, "Couldn't open blob from '%s': %s\n", filename,
			strerror(ret));
		return NULL;
	}
	/* Successful read */
	return buff;
}

char *utilfdt_read(const char *filename)
{
	off_t len;
	return utilfdt_read_len(filename, &len);
}

/**
 * Run the main fdtgrep operation, given a filename and valid arguments
 *
 * @param disp		Display information / options
 * @param filename	Filename of blob file
 * @param return 0 if ok, -ve on error
 */
static int do_fdtgrep(struct display_info *disp, const char *filename)
{
	struct fdt_region *region = NULL;
	int max_regions;
	int count = 100;
	char path[1024];
	char *blob;
	int i, ret;

	blob = utilfdt_read(filename);
	if (!blob)
		return -1;
	ret = fdt_check_header(blob);
	if (ret) {
		fprintf(stderr, "Error: %s\n", fdt_strerror(ret));
		return ret;
	}

	/* Allow old files, but they are untested */
	if (fdt_version(blob) < 17 && disp->value_head) {
		fprintf(stderr,
			"Warning: fdtgrep does not fully support version %d files\n",
			fdt_version(blob));
	}

	/*
	 * We do two passes, since we don't know how many regions we need.
	 * The first pass will count the regions, but if it is too many,
	 * we do another pass to actually record them.
	 */
	for (i = 0; i < 2; i++) {
		region = malloc(count * sizeof(struct fdt_region));
		if (!region) {
			fprintf(stderr, "Out of memory for %d regions\n",
				count);
			return -1;
		}
		max_regions = count;
		count = fdtgrep_find_regions(blob,
				h_include, disp,
				region, max_regions, path, sizeof(path),
				disp->flags);
		if (count < 0) {
			report_error("fdt_find_regions", count);
			free(region);
			return -1;
		}
		if (count <= max_regions)
			break;
		free(region);
		fprintf(stderr, "Internal error with fdtgrep_find_region)(\n");
		return -1;
	}

	/* Optionally print a list of regions */
	if (disp->region_list)
		show_region_list(region, count);

	/* Output either source .dts or binary .dtb */
	if (disp->output == OUT_DTS) {
		ret = display_fdt_by_regions(disp, blob, region, count);
	} else {
		void *fdt;
		/* Allow reserved memory section to expand slightly */
		int size = fdt_totalsize(blob) + 16;

		fdt = malloc(size);
		if (!fdt) {
			fprintf(stderr, "Out_of_memory\n");
			ret = -1;
			goto err;
		}
		size = dump_fdt_regions(disp, blob, region, count, fdt);
		if (disp->remove_strings) {
			void *out;

			out = malloc(size);
			if (!out) {
				fprintf(stderr, "Out_of_memory\n");
				ret = -1;
				goto err;
			}
			ret = fdt_remove_unused_strings(fdt, out);
			if (ret < 0) {
				fprintf(stderr,
					"Failed to remove unused strings: err=%d\n",
					ret);
				goto err;
			}
			free(fdt);
			fdt = out;
			ret = fdt_pack(fdt);
			if (ret < 0) {
				fprintf(stderr, "Failed to pack: err=%d\n",
					ret);
				goto err;
			}
			size = fdt_totalsize(fdt);
		}

		if (size != fwrite(fdt, 1, size, disp->fout)) {
			fprintf(stderr, "Write failure, %d bytes\n", size);
			free(fdt);
			ret = 1;
			goto err;
		}
		free(fdt);
	}
err:
	free(blob);
	free(region);

	return ret;
}

static const char usage_synopsis[] =
	"fdtgrep - extract portions from device tree\n"
	"\n"
	"Usage:\n"
	"	fdtgrep <options> <dt file>|-\n\n"
	"Output formats are:\n"
	"\tdts - device tree soure text\n"
	"\tdtb - device tree blob (sets -Hmt automatically)\n"
	"\tbin - device tree fragment (may not be a valid .dtb)";

/* Helper for usage_short_opts string constant */
#define USAGE_COMMON_SHORT_OPTS "hV"

/* Helper for aligning long_opts array */
#define a_argument required_argument

/* Helper for usage_long_opts option array */
#define USAGE_COMMON_LONG_OPTS \
	{"help",      no_argument, NULL, 'h'}, \
	{"version",   no_argument, NULL, 'V'}, \
	{NULL,        no_argument, NULL, 0x0}

/* Helper for usage_opts_help array */
#define USAGE_COMMON_OPTS_HELP \
	"Print this help and exit", \
	"Print version and exit", \
	NULL

/* Helper for getopt case statements */
#define case_USAGE_COMMON_FLAGS \
	case 'h': usage(NULL); \
	case 'V': util_version(); \
	case '?': usage("unknown option");

static const char usage_short_opts[] =
		"haAc:b:C:defg:G:HIlLmn:N:o:O:p:P:rRsStTv"
		USAGE_COMMON_SHORT_OPTS;
static struct option const usage_long_opts[] = {
	{"show-address",	no_argument, NULL, 'a'},
	{"colour",		no_argument, NULL, 'A'},
	{"include-node-with-prop", a_argument, NULL, 'b'},
	{"include-compat",	a_argument, NULL, 'c'},
	{"exclude-compat",	a_argument, NULL, 'C'},
	{"diff",		no_argument, NULL, 'd'},
	{"enter-node",		no_argument, NULL, 'e'},
	{"show-offset",		no_argument, NULL, 'f'},
	{"include-match",	a_argument, NULL, 'g'},
	{"exclude-match",	a_argument, NULL, 'G'},
	{"show-header",		no_argument, NULL, 'H'},
	{"show-version",	no_argument, NULL, 'I'},
	{"list-regions",	no_argument, NULL, 'l'},
	{"list-strings",	no_argument, NULL, 'L'},
	{"include-mem",		no_argument, NULL, 'm'},
	{"include-node",	a_argument, NULL, 'n'},
	{"exclude-node",	a_argument, NULL, 'N'},
	{"include-prop",	a_argument, NULL, 'p'},
	{"exclude-prop",	a_argument, NULL, 'P'},
	{"remove-strings",	no_argument, NULL, 'r'},
	{"include-root",	no_argument, NULL, 'R'},
	{"show-subnodes",	no_argument, NULL, 's'},
	{"skip-supernodes",	no_argument, NULL, 'S'},
	{"show-stringtab",	no_argument, NULL, 't'},
	{"show-aliases",	no_argument, NULL, 'T'},
	{"out",			a_argument, NULL, 'o'},
	{"out-format",		a_argument, NULL, 'O'},
	{"invert-match",	no_argument, NULL, 'v'},
	USAGE_COMMON_LONG_OPTS,
};
static const char * const usage_opts_help[] = {
	"Display address",
	"Show all nodes/tags, colour those that match",
	"Include contains containing property",
	"Compatible nodes to include in grep",
	"Compatible nodes to exclude in grep",
	"Diff: Mark matching nodes with +, others with -",
	"Enter direct subnode names of matching nodes",
	"Display offset",
	"Node/property/compatible string to include in grep",
	"Node/property/compatible string to exclude in grep",
	"Output a header",
	"Put \"/dts-v1/;\" on first line of dts output",
	"Output a region list",
	"List strings in string table",
	"Include mem_rsvmap section in binary output",
	"Node to include in grep",
	"Node to exclude in grep",
	"Property to include in grep",
	"Property to exclude in grep",
	"Remove unused strings from string table",
	"Include root node and all properties",
	"Show all subnodes matching nodes",
	"Don't include supernodes of matching nodes",
	"Include string table in binary output",
	"Include matching aliases in output",
	"-o <output file>",
	"-O <output format>",
	"Invert the sense of matching (select non-matching lines)",
	USAGE_COMMON_OPTS_HELP
};

/**
 * Call getopt_long() with standard options
 *
 * Since all util code runs getopt in the same way, provide a helper.
 */
#define util_getopt_long() getopt_long(argc, argv, usage_short_opts, \
				       usage_long_opts, NULL)

void util_usage(const char *errmsg, const char *synopsis,
		const char *short_opts, struct option const long_opts[],
		const char * const opts_help[])
{
	FILE *fp = errmsg ? stderr : stdout;
	const char a_arg[] = "<arg>";
	size_t a_arg_len = strlen(a_arg) + 1;
	size_t i;
	int optlen;

	fprintf(fp,
		"Usage: %s\n"
		"\n"
		"Options: -[%s]\n", synopsis, short_opts);

	/* prescan the --long opt length to auto-align */
	optlen = 0;
	for (i = 0; long_opts[i].name; ++i) {
		/* +1 is for space between --opt and help text */
		int l = strlen(long_opts[i].name) + 1;
		if (long_opts[i].has_arg == a_argument)
			l += a_arg_len;
		if (optlen < l)
			optlen = l;
	}

	for (i = 0; long_opts[i].name; ++i) {
		/* helps when adding new applets or options */
		assert(opts_help[i] != NULL);

		/* first output the short flag if it has one */
		if (long_opts[i].val > '~')
			fprintf(fp, "      ");
		else
			fprintf(fp, "  -%c, ", long_opts[i].val);

		/* then the long flag */
		if (long_opts[i].has_arg == no_argument) {
			fprintf(fp, "--%-*s", optlen, long_opts[i].name);
		} else {
			fprintf(fp, "--%s %s%*s", long_opts[i].name, a_arg,
				(int)(optlen - strlen(long_opts[i].name) -
				a_arg_len), "");
		}

		/* finally the help text */
		fprintf(fp, "%s\n", opts_help[i]);
	}

	if (errmsg) {
		fprintf(fp, "\nError: %s\n", errmsg);
		exit(EXIT_FAILURE);
	} else {
		exit(EXIT_SUCCESS);
	}
}

/**
 * Show usage and exit
 *
 * If you name all your usage variables with usage_xxx, then you can call this
 * help macro rather than expanding all arguments yourself.
 *
 * @param errmsg	If non-NULL, an error message to display
 */
#define usage(errmsg) \
	util_usage(errmsg, usage_synopsis, usage_short_opts, \
		   usage_long_opts, usage_opts_help)

void util_version(void)
{
	printf("Version: %s\n", "(U-Boot)");
	exit(0);
}

static void scan_args(struct display_info *disp, int argc, char *argv[])
{
	int opt;

	while ((opt = util_getopt_long()) != EOF) {
		int type = 0;
		int inc = 1;

		switch (opt) {
		case_USAGE_COMMON_FLAGS
		case 'a':
			disp->show_addr = 1;
			break;
		case 'A':
			disp->all = 1;
			break;
		case 'b':
			type = FDT_NODE_HAS_PROP;
			break;
		case 'C':
			inc = 0;
			/* no break */
		case 'c':
			type = FDT_IS_COMPAT;
			break;
		case 'd':
			disp->diff = 1;
			break;
		case 'e':
			disp->flags |= FDT_REG_DIRECT_SUBNODES;
			break;
		case 'f':
			disp->show_offset = 1;
			break;
		case 'G':
			inc = 0;
			/* no break */
		case 'g':
			type = FDT_ANY_GLOBAL;
			break;
		case 'H':
			disp->header = 1;
			break;
		case 'l':
			disp->region_list = 1;
			break;
		case 'L':
			disp->list_strings = 1;
			break;
		case 'm':
			disp->flags |= FDT_REG_ADD_MEM_RSVMAP;
			break;
		case 'N':
			inc = 0;
			/* no break */
		case 'n':
			type = FDT_IS_NODE;
			break;
		case 'o':
			disp->output_fname = optarg;
			break;
		case 'O':
			if (!strcmp(optarg, "dtb"))
				disp->output = OUT_DTB;
			else if (!strcmp(optarg, "dts"))
				disp->output = OUT_DTS;
			else if (!strcmp(optarg, "bin"))
				disp->output = OUT_BIN;
			else
				usage("Unknown output format");
			break;
		case 'P':
			inc = 0;
			/* no break */
		case 'p':
			type = FDT_IS_PROP;
			break;
		case 'r':
			disp->remove_strings = 1;
			break;
		case 'R':
			disp->include_root = 1;
			break;
		case 's':
			disp->flags |= FDT_REG_ALL_SUBNODES;
			break;
		case 'S':
			disp->flags &= ~FDT_REG_SUPERNODES;
			break;
		case 't':
			disp->flags |= FDT_REG_ADD_STRING_TAB;
			break;
		case 'T':
			disp->add_aliases = 1;
			break;
		case 'v':
			disp->invert = 1;
			break;
		case 'I':
			disp->show_dts_version = 1;
			break;
		}

		if (type && value_add(disp, &disp->value_head, type, inc,
				      optarg))
			usage("Cannot add value");
	}

	if (disp->invert && disp->types_exc)
		usage("-v has no meaning when used with 'exclude' conditions");
}

int main(int argc, char *argv[])
{
	char *filename = NULL;
	struct display_info disp;
	int ret;

	/* set defaults */
	memset(&disp, '\0', sizeof(disp));
	disp.flags = FDT_REG_SUPERNODES;	/* Default flags */

	scan_args(&disp, argc, argv);

	/* Show matched lines in colour if we can */
	disp.colour = disp.all && isatty(0);

	/* Any additional arguments can match anything, just like -g */
	while (optind < argc - 1) {
		if (value_add(&disp, &disp.value_head, FDT_IS_ANY, 1,
			      argv[optind++]))
			usage("Cannot add value");
	}

	if (optind < argc)
		filename = argv[optind++];
	if (!filename)
		usage("Missing filename");

	/* If a valid .dtb is required, set flags to ensure we get one */
	if (disp.output == OUT_DTB) {
		disp.header = 1;
		disp.flags |= FDT_REG_ADD_MEM_RSVMAP | FDT_REG_ADD_STRING_TAB;
	}

	if (disp.output_fname) {
		disp.fout = fopen(disp.output_fname, "w");
		if (!disp.fout)
			usage("Cannot open output file");
	} else {
		disp.fout = stdout;
	}

	/* Run the grep and output the results */
	ret = do_fdtgrep(&disp, filename);
	if (disp.output_fname)
		fclose(disp.fout);
	if (ret)
		return 1;

	return 0;
}
