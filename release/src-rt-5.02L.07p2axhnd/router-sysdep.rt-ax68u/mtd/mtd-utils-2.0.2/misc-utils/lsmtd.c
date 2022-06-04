/*
 * Copyright (C) 2017 David Oberhollenzer - sigma star gmbh
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; see the file COPYING. If not, write to the Free Software
 * Foundation, 51 Franklin Street, Fifth Floor, Boston, MA 02110-1335, USA.
 *
 * Author: David Oberhollenzer <david.oberhollenzer@sigma-star.at>
 */
#include <getopt.h>
#include <stdio.h>

#include "lsmtd.h"

#define FLAG_SI 0x0001
#define FLAG_BYTES 0x0002
#define FLAG_NO_HEADING 0x0004
#define FLAG_RAW 0x0008
#define FLAG_PAIRS 0x0010
#define FLAG_LIST 0x0020
#define FLAG_JSON 0x0040
#define FLAG_ASCII 0x0080
#define FLAG_NO_UBI 0x0100
#define FLAG_DRYRUN 0x1000

static int flags;
static struct column **selected;
static size_t num_selected;
static size_t max_selected;
struct column *sort_by;

static const struct option long_opts[] = {
	{ "help", no_argument, NULL, 'h' },
	{ "version", no_argument, NULL, 'V' },
	{ "si-units", no_argument, NULL, 'u' },
 	{ "bytes", no_argument, NULL, 'b' },
	{ "noheadings", no_argument, NULL, 'n' },
	{ "raw", no_argument, NULL, 'r' },
	{ "output", required_argument, NULL, 'o' },
	{ "output-all", no_argument, NULL, 'O' },
	{ "pairs", no_argument, NULL, 'P' },
	{ "list", no_argument, NULL, 'l' },
	{ "json", no_argument, NULL, 'J' },
	{ "sort", required_argument, NULL, 'x' },
	{ "ascii", no_argument, NULL, 'i' },
	{ "no-ubi", no_argument, NULL, 'm' },
	{ NULL, 0, NULL, 0 },
};

static const char *short_opts = "x:o:OPJlibrumnhV";
static const char *default_cols = "DEVICE,MAJ:MIN,NAME,TYPE,SIZE";

static struct column cols[] = {
	{ "DEVICE", "name of the device node", COL_DEVNAME, COL_DT_STRING, 0 },
	{ "MAJ:MIN", "major:minor device number",
		COL_DEVNUM, COL_DT_STRING, 0 },
	{ "NAME", "device name string", COL_NAME, COL_DT_STRING, 0 },
	{ "TYPE", "device type", COL_TYPE, COL_DT_STRING, 0 },
	{ "SIZE", "size of the device", COL_SIZE, COL_DT_SIZE, 0 },
	{ "EB-SIZE", "erase block size", COL_EBSIZE, COL_DT_SIZE, 0 },
	{ "EB-COUNT", "number of erase blocks", COL_EBCOUNT, COL_DT_NUMBER, 0 },
	{ "MIN-IO", "minimum I/O size", COL_MINIO, COL_DT_SIZE, 0 },
	{ "SUB-SIZE", "subpage size", COL_SUBSIZE, COL_DT_SIZE, 0 },
	{ "OOB-SIZE", "out of band data size", COL_OOBSIZE, COL_DT_SIZE, 0 },
	{ "RO", "read-only device", COL_RO, COL_DT_BOOL, 0 },
	{ "CORRUPTED", "wheather an UBI volume is corrupted",
		COL_CORRUPTED, COL_DT_BOOL, 0 },
	{ "REGIONS", "number of additional erase regions",
		COL_REGION, COL_DT_NUMBER, 0 },
	{ "BB", "wheather the MTD device may have bad eraseblocks",
		COL_BB, COL_DT_BOOL, 0 },
	{ "MAX-EC", "current highest erase counter value on UBI devices",
		COL_MAXEC, COL_DT_NUMBER, 0 },
	{ "FREE", "available bytes on an UBI device or volume",
		COL_FREE, COL_DT_SIZE, 0 },
	{ "FREE-LEB", "available LEBs on an UBI device or volume",
		COL_FREE_LEB, COL_DT_NUMBER, 0 },
	{ "BAD-COUNT", "number of bad physical eraseblocks",
		COL_BAD_COUNT, COL_DT_NUMBER, 0 },
	{ "BAD-RSVD", "number of reserved eraseblocks for bad block handling",
		COL_BAD_RSVD, COL_DT_NUMBER, 0 },
};

static NORETURN void usage(int status)
{
	FILE *outstream = status == EXIT_SUCCESS ? stdout : stderr;
	size_t i, len, max_len = 0;

	fputs(
"Usage: "PROGRAM_NAME" [options] [<device> ...]\n\n"
"List information about memory technology devices.\n\n"
"Options:\n"
"  -u, --si-units       Scale sizes by factors of 1000 instead of 1024\n"
"  -b, --bytes          Print sizes in bytes\n"
"  -i, --ascii          Use ascii characters only\n"
"  -l, --list           Use list output format (default)\n"
"  -n, --noheadings     Don't print a heading\n"
"  -r, --raw            Use raw output format\n"
"  -P, --pairs          Use key=\"value\" output format\n"
"  -J, --json           Use JSON output format\n"
"  -o, --output <list>  Comma seperated list of columns to print\n"
"  -O, --output-all     Print all columns\n"
"  -x, --sort <column>  Sort output by <column>\n"
"  -m, --no-ubi         Do not display information about UBI devices/volumes\n"
"\n"
"  -h, --help           Display this help text and exit\n"
"  -V, --version        Output version information and exit\n"
"\n"
"Available columns (for --output, --sort):\n",
	outstream);

	for (i = 0; i < sizeof(cols) / sizeof(cols[0]); ++i) {
		len = strlen(cols[i].name);
		max_len = len > max_len ? len : max_len;
	}

	for (i = 0; i < sizeof(cols) / sizeof(cols[0]); ++i) {
		fprintf(outstream, "  %*s  %s\n", (int)max_len, cols[i].name,
			cols[i].desc);
	}

	fputs("\nFor more details see "PROGRAM_NAME"(8).\n", stdout);
	exit(status);
}

static NORETURN void version(int status)
{
	common_print_version();
	fputs(
"Copyright (C) 2017 David Oberhollenzer - sigma star gmbh\n"
"License GPLv2: GNU GPL version 2 <http://gnu.org/licenses/gpl2.html>.\n"
"This is free software: you are free to change and redistribute it.\n"
"There is NO WARRANTY, to the extent permitted by law.\n\n"
"Written by David Oberhollenzer.\n",
		stdout);
	exit(status);
}

static struct column *column_by_name(const char *name, size_t len)
{
	size_t i;

	for (i = 0; i < sizeof(cols) / sizeof(cols[0]); ++i) {
		if (strncmp(cols[i].name, name, len) != 0)
			continue;
		if (strlen(cols[i].name) == len)
			return cols + i;
	}

	return NULL;
}

static int process_col_list(const char *list)
{
	struct column *col;
	const char *end;
	size_t len;

	if (*list == '+') {
		++list;
	} else {
		num_selected = 0;
	}

	while (*list) {
		end = strchrnul(list, ',');
		len = end - list;

		col = column_by_name(list, len);
		if (!col) {
			fprintf(stderr, "Unknown column '%.*s'\n",
				(int)len, list);
			return -1;
		}

		if (num_selected == max_selected) {
			max_selected = max_selected ? max_selected * 2 : 10;
			selected = xrealloc(selected, max_selected *
							sizeof(*selected));
		}

		selected[num_selected++] = col;
		list = *end ? end + 1 : end;
	}
	return 0;
}

static void select_all(void)
{
	size_t i;

	num_selected = sizeof(cols) / sizeof(cols[0]);

	if (max_selected < num_selected) {
		max_selected = num_selected;
		selected = xrealloc(selected, max_selected * sizeof(*selected));
	}

	for (i = 0; i < num_selected; ++i)
		selected[i] = cols + i;
}

static void process_args(int argc, char **argv)
{
	int i;

	process_col_list(default_cols);

	while (1) {
		i = getopt_long(argc, argv, short_opts, long_opts, NULL);
		if (i == -1)
			break;

		switch (i) {
		case 'x':
			sort_by = column_by_name(optarg, strlen(optarg));
			if (!sort_by) {
				fprintf(stderr, "Unknown column '%s'\n",
					optarg);
				goto fail;
			}
			break;
		case 'o':
			if (process_col_list(optarg) != 0)
				goto fail;
			break;
		case 'O':
			select_all();
			break;
		case 'i': flags |= FLAG_ASCII; break;
		case 'J': flags |= FLAG_JSON; break;
		case 'P': flags |= FLAG_PAIRS; break;
		case 'l': flags |= FLAG_LIST; break;
		case 'b': flags |= FLAG_BYTES; break;
		case 'r': flags |= FLAG_RAW; break;
		case 'u': flags |= FLAG_SI; break;
		case 'n': flags |= FLAG_NO_HEADING; break;
		case 'm': flags |= FLAG_NO_UBI; break;
		case 'h': usage(EXIT_SUCCESS);
		case 'V': version(EXIT_SUCCESS);
		default: usage(EXIT_FAILURE);
		}
	}

	i = flags & (FLAG_LIST|FLAG_PAIRS|FLAG_RAW|FLAG_JSON);

	if (i & (i - 1)) {
		fputs(PROGRAM_NAME": these options are mutually exclusive: "
			"--list --pairs --raw --json\n", stderr);
		goto fail;
	} else if (!i) {
		flags |= FLAG_LIST;
	}

	/*if (optind < argc)
		list_arg = optind;*/
	return;
fail:
	fputs("Try `"PROGRAM_NAME" --help` for more information\n\n", stderr);
	exit(EXIT_FAILURE);
}

static const char *tree_prefix(bool is_last)
{
	if (is_last)
		return (flags & FLAG_ASCII) ? "`-" : "└─";
	return (flags & FLAG_ASCII) ? "|-" : "├─";
}

static size_t count_chars(const char *str)
{
	size_t count = 0;
	while (*str) {
		if (((*str) & 0xC0) != 0x80)
			++count;
		++str;
	}
	return count;
}

static void devno_to_string(char *buffer, int major, int minor)
{
	sprintf(buffer, flags & FLAG_LIST ? "%3d:%d" : "%d:%d", major, minor);
}

static void bool_to_string(char *buffer, int value)
{
	if (flags & FLAG_JSON)
		strcpy(buffer, value ? "true" : "false");
	else
		strcpy(buffer, value ? "1" : "0");
}

static void size_to_string(char *buffer, long long int size)
{
	static const char *bcdmap = "0112334456678899";
	static const char *suffix = "KMGTPE";
	int scale, idx, i, remainder = 0;

	if (flags & FLAG_BYTES) {
		sprintf(buffer, "%lld", size);
		return;
	}

	scale = flags & FLAG_SI ? 1000 : 1024;

	for (idx = -1; size >= scale && (idx < 0 || suffix[idx]); ++idx) {
		if (remainder >= (scale / 2)) {
			remainder = 0;
			size = (size / scale) + 1;
		} else {
			remainder = size % scale;
			size /= scale;
		}
	}

	i = sprintf(buffer, "%lld", size);

	remainder = (remainder >> 6) & 0x0F;
	if (remainder) {
		buffer[i++] = '.';
		buffer[i++] = bcdmap[remainder];
	}
	if (idx >= 0)
		buffer[i++] = suffix[idx];
	buffer[i] = '\0';
}

static void print_json_string(const char *value)
{
	static const char *jsonrepl = "nrtfb", *jsonesc = "\n\r\t\f\b";
	const char *ptr;

	fputc('"', stdout);
	for (; *value; ++value) {
		ptr = strchr(jsonesc, *value);
		if (ptr) {
			fputc('\\', stdout);
			fputc(jsonrepl[ptr - jsonesc], stdout);
		} else if (*value == '\\' || *value == '"') {
			fputc('\\', stdout);
			fputc(*value, stdout);
		} else if (isascii(*value) &&
			(iscntrl(*value) || !isprint(*value))) {
			fprintf(stdout, "\\u%04X", *value);
		} else {
			fputc(*value, stdout);
		}
	}
	fputc('"', stdout);
}

static void print_escaped(const char *value)
{
	while (*value) {
		if (iscntrl(*value) || !isprint(*value) ||
			*value == '\\' || *value == '"') {
			fprintf(stdout, "\\x%02X", *(value++));
		} else {
			fputc(*(value++), stdout);
		}
	}
}

static void print_padded(const char *value, bool numeric, size_t width)
{
	size_t i;

	if (numeric) {
		fprintf(stdout, "%*s", (int)width, value);
	} else {
		for (i = 0; i < width && *value; ++i) {
			fputc(*(value++), stdout);
			while (((*value) & 0xC0) == 0x80)
				fputc(*(value++), stdout);
		}

		for (; i < width; ++i)
			fputc(' ', stdout);
	}
}

static void print_column(struct column *col, const char *value,
			bool is_first, int level)
{
	bool numeric = false;
	const char *key;
	size_t colw;

	if (col->datatype == COL_DT_NUMBER || col->datatype == COL_DT_SIZE ||
		col->datatype == COL_DT_BOOL) {
		numeric = true;
	}

	if (flags & FLAG_JSON) {
		if ((col->datatype == COL_DT_SIZE) && !(flags & FLAG_BYTES))
			numeric = false;

		if (!is_first)
			fputs(",\n", stdout);

		while (level--)
			fputc('\t', stdout);

		fputc('"', stdout);
		for (key = col->name; *key; ++key)
			fputc(isupper(*key) ? tolower(*key) : *key, stdout);
		fputs("\": ", stdout);

		if (numeric) {
			fputs(value, stdout);
		} else {
			print_json_string(value);
		}
	} else if (flags & FLAG_DRYRUN) {
		colw = count_chars(value);
		col->width = colw > col->width ? colw : col->width;
	} else if (flags & FLAG_PAIRS) {
		if (!is_first)
			fputc(' ', stdout);
		fprintf(stdout, "%s=\"", col->name);
		print_escaped(value);
		fputs("\"", stdout);
	} else if (flags & FLAG_RAW) {
		if (!is_first)
			fputc(' ', stdout);
		print_escaped(value);
	} else if (flags & FLAG_LIST) {
		if (!is_first)
			fputc(' ', stdout);
		print_padded(value, numeric, col->width);
	}
}

static size_t print_mtd_device(struct mtd_dev_info *info)
{
	size_t i, count = 0;
	const char *value;
	char buffer[128];

	for (i = 0; i < num_selected; ++i) {
		value = buffer;
		switch (selected[i]->type) {
		case COL_DEVNAME:
			sprintf(buffer, "mtd%d", info->mtd_num);
			break;
		case COL_DEVNUM:
			devno_to_string(buffer, info->major, info->minor);
			break;
		case COL_TYPE:
			value = info->type_str;
			break;
		case COL_NAME:
			value = info->name;
			break;
		case COL_SIZE:
			size_to_string(buffer, info->size);
			break;
		case COL_EBSIZE:
			size_to_string(buffer, info->eb_size);
			break;
		case COL_EBCOUNT:
			sprintf(buffer, "%d", info->eb_cnt);
			break;
		case COL_MINIO:
			size_to_string(buffer, info->min_io_size);
			break;
		case COL_SUBSIZE:
			size_to_string(buffer, info->subpage_size);
			break;
		case COL_OOBSIZE:
			size_to_string(buffer, info->oob_size);
			break;
		case COL_RO:
			bool_to_string(buffer, !info->writable);
			break;
		case COL_BB:
			bool_to_string(buffer, !info->bb_allowed);
			break;
		case COL_REGION:
			sprintf(buffer, "%d", info->region_cnt);
			break;
		default:
			if (flags & FLAG_JSON)
				continue;
			buffer[0] = '\0';
			break;
		}
		print_column(selected[i], value, i == 0, 2);
		++count;
	}
	return count;
}

static size_t print_ubi_device(struct mtd_dev_info *mtd,
				struct ubi_dev_info *info)
{
	size_t i, count = 0;
	char value[128];

	for (i = 0; i < num_selected; ++i) {
		switch (selected[i]->type) {
		case COL_DEVNAME:
			if (flags & FLAG_LIST) {
				sprintf(value, "%subi%d", tree_prefix(true),
							info->dev_num);
			} else {
				sprintf(value, "ubi%d", info->dev_num);
			}
			break;
		case COL_DEVNUM:
			devno_to_string(value, info->major, info->minor);
			break;
		case COL_SIZE:
			size_to_string(value, info->total_bytes);
			break;
		case COL_EBSIZE:
			size_to_string(value, info->leb_size);
			break;
		case COL_EBCOUNT:
			sprintf(value, "%d", info->total_lebs);
			break;
		case COL_MINIO:
			size_to_string(value, info->min_io_size);
			break;
		case COL_MAXEC:
			sprintf(value, "%lld", info->max_ec);
			break;
		case COL_FREE:
			size_to_string(value, info->avail_bytes);
			break;
		case COL_FREE_LEB:
			sprintf(value, "%d", info->avail_lebs);
			break;
		case COL_BAD_COUNT:
			sprintf(value, "%d", info->bad_count);
			break;
		case COL_BAD_RSVD:
			sprintf(value, "%d", info->bad_rsvd);
			break;
		case COL_RO:
			bool_to_string(value, !mtd->writable);
			break;
		default:
			if (flags & FLAG_JSON)
				continue;
			value[0] = '\0';
			break;
		}
		print_column(selected[i], value, i == 0, 3);
		++count;
	}
	return count;
}

static size_t print_ubi_vol(struct mtd_dev_info *mtd, struct ubi_dev_info *dev,
				struct ubi_vol_info *info, bool is_last)
{
	size_t i, count = 0;
	const char *value;
	char buffer[128];
	int used;

	for (i = 0; i < num_selected; ++i) {
		value = buffer;
		switch (selected[i]->type) {
		case COL_DEVNAME:
			if (flags & FLAG_LIST) {
				sprintf(buffer, "  %subi%d_%d",
						tree_prefix(is_last),
						info->dev_num, info->vol_id);
			} else {
				sprintf(buffer, "ubi%d_%d", info->dev_num,
							info->vol_id);
			}
			break;
		case COL_DEVNUM:
			devno_to_string(buffer, info->major, info->minor);
			break;
		case COL_TYPE:
			if (info->type == UBI_DYNAMIC_VOLUME) {
				value = "dynamic";
			} else {
				value = "static";
			}
			break;
		case COL_NAME:
			value = info->name;
			break;
		case COL_SIZE:
			size_to_string(buffer, info->rsvd_bytes);
			break;
		case COL_EBSIZE:
			size_to_string(buffer, info->leb_size);
			break;
		case COL_EBCOUNT:
			sprintf(buffer, "%d", info->rsvd_lebs);
			break;
		case COL_MINIO:
			size_to_string(buffer, dev->min_io_size);
			break;
		case COL_FREE:
			size_to_string(buffer,
					info->rsvd_bytes - info->data_bytes);
			break;
		case COL_FREE_LEB:
			used = info->data_bytes / info->leb_size;
			sprintf(buffer, "%d", info->rsvd_lebs - used);
			break;
		case COL_RO:
			bool_to_string(buffer, !mtd->writable);
			break;
		case COL_CORRUPTED:
			bool_to_string(buffer, info->corrupted);
			break;
		default:
			if (flags & FLAG_JSON)
				continue;
			buffer[0] = '\0';
			break;
		}
		print_column(selected[i], value, i == 0, 4);
		++count;
	}
	return count;
}

static void print_list(void)
{
	struct ubi_node *ubi;
	bool is_last;
	size_t i;
	int j;

	if (!(flags & FLAG_NO_HEADING)) {
		if (flags & (FLAG_DRYRUN | FLAG_RAW)) {
			for (i = 0; i < num_selected; ++i)
				selected[i]->width = strlen(selected[i]->name);
		}

		if (!(flags & FLAG_DRYRUN)) {
			for (i = 0; i < num_selected; ++i) {
				fprintf(stdout, "%-*s ",
					(int)selected[i]->width,
					selected[i]->name);
			}
			fputc('\n', stdout);
		}
	}

	for (i = 0; i < num_mtd_devices; ++i) {
		print_mtd_device(&mtd_dev[i].info);
		if (!(flags & FLAG_DRYRUN))
			fputc('\n', stdout);

		ubi = mtd_dev[i].ubi;
		if (!ubi)
			continue;

		print_ubi_device(&mtd_dev[i].info, &ubi->info);
		if (!(flags & FLAG_DRYRUN))
			fputc('\n', stdout);

		for (j = 0; j < ubi->info.vol_count; ++j) {
			is_last = (j == (ubi->info.vol_count - 1));
			print_ubi_vol(&mtd_dev[i].info, &ubi->info,
					ubi->vol_info + j, is_last);
			if (!(flags & FLAG_DRYRUN))
				fputc('\n', stdout);
		}
	}
}

static void print_pairs(void)
{
	struct ubi_node *ubi;
	int i, j;

	for (i = 0; i < num_mtd_devices; ++i) {
		print_mtd_device(&mtd_dev[i].info);
		fputc('\n', stdout);

		ubi = mtd_dev[i].ubi;
		if (ubi) {
			print_ubi_device(&mtd_dev[i].info, &ubi->info);
			fputc('\n', stdout);

			for (j = 0; j < ubi->info.vol_count; ++j) {
				print_ubi_vol(&mtd_dev[i].info, &ubi->info,
						ubi->vol_info + j, false);
				fputc('\n', stdout);
			}
		}
	}
}

static void print_json(void)
{
	struct ubi_node *ubi;
	int i, j;

	fputs("{\n\t\"mtddevices\": [", stdout);

	for (i = 0; i < num_mtd_devices; ++i) {
		fputs(i ? ",{\n" : "{\n", stdout);
		if (print_mtd_device(&mtd_dev[i].info) > 0)
			fputs(",\n", stdout);

		ubi = mtd_dev[i].ubi;
		if (ubi) {
			fputs("\t\t\"ubi\": {\n", stdout);
			if (print_ubi_device(&mtd_dev[i].info, &ubi->info) > 0)
				fputs(",\n", stdout);

			fputs("\t\t\t\"volumes\": [", stdout);

			for (j = 0; j < ubi->info.vol_count; ++j) {
				fputs(j ? ",{\n" : "{\n", stdout);
				print_ubi_vol(&mtd_dev[i].info, &ubi->info,
						ubi->vol_info + j, false);
				fputs("\n\t\t\t}", stdout);
			}

			fputs("]\n\t\t}\n", stdout);
		} else if (!(flags & FLAG_NO_UBI)) {
			fputs("\t\t\"ubi\": null\n", stdout);
		}
		fputs("\t}", stdout);
	}

	fputs("]\n}\n", stdout);
}

int main(int argc, char **argv)
{
	int ret, status = EXIT_FAILURE;
	libmtd_t lib_mtd;
	libubi_t lib_ubi;

	process_args(argc, argv);

	lib_mtd = libmtd_open();
	if (lib_mtd) {
		ret = scan_mtd(lib_mtd);
		libmtd_close(lib_mtd);
		if (ret)
			goto out;
	} else {
		if (errno) {
			perror("libmtd_open");
			return EXIT_FAILURE;
		}
		return EXIT_SUCCESS;
	}

	if (!(flags & FLAG_NO_UBI)) {
		lib_ubi = libubi_open();
		if (lib_ubi) {
			ret = scan_ubi(lib_ubi);
			libubi_close(lib_ubi);
			if (ret)
				goto out;
		} else if (errno) {
			perror("libubi_open");
			goto out;
		}
	}

	if (flags & FLAG_JSON) {
		print_json();
	} else if (flags & FLAG_PAIRS) {
		print_pairs();
	} else {
		flags |= FLAG_DRYRUN;
		print_list();
		flags &= ~FLAG_DRYRUN;
		print_list();
	}

	status = EXIT_SUCCESS;
out:
	scan_free();
	free(selected);
	return status;
}
