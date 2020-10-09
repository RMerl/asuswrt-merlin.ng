#define _GNU_SOURCE

#include <stdio.h>
#include <getopt.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <ext2fs/ext2fs.h>

#include "perms.h"
#include "base_fs.h"
#include "block_list.h"
#include "basefs_allocator.h"
#include "create_inode.h"

#ifndef UID_GID_MAP_MAX_EXTENTS
/*
 * The value is defined in linux/user_namspace.h.
 * The value is (arbitrarily) 5 in 4.14 and earlier, or 340 in 4.15 and later.
 * Here, the bigger value is taken. See also man user_namespace(7).
 */
#define UID_GID_MAP_MAX_EXTENTS 340
#endif

static char *prog_name = "e2fsdroid";
static char *in_file;
static char *block_list;
static char *basefs_out;
static char *basefs_in;
static char *mountpoint = "";
static time_t fixed_time = -1;
static char *fs_config_file;
static struct selinux_opt seopt_file[8];
static int max_nr_opt = (int)sizeof(seopt_file) / sizeof(seopt_file[0]);
static char *product_out;
static char *src_dir;
static int android_configure;
static int android_sparse_file = 1;

static void usage(int ret)
{
	fprintf(stderr, "%s [-B block_list] [-D basefs_out] [-T timestamp]\n"
			"\t[-C fs_config] [-S file_contexts] [-p product_out]\n"
			"\t[-a mountpoint] [-d basefs_in] [-f src_dir] [-e] [-s]\n"
			"\t[-u uid-mapping] [-g gid-mapping] image\n",
                prog_name);
	exit(ret);
}

static char *absolute_path(const char *file)
{
	char *ret;
	char cwd[PATH_MAX];

	if (file[0] != '/') {
		if (getcwd(cwd, PATH_MAX) == NULL) {
			fprintf(stderr, "Failed to getcwd\n");
			exit(EXIT_FAILURE);
		}
		ret = malloc(strlen(cwd) + 1 + strlen(file) + 1);
		if (ret)
			sprintf(ret, "%s/%s", cwd, file);
	} else
		ret = strdup(file);
	return ret;
}

static int parse_ugid_map_entry(char* line, struct ugid_map_entry* result)
{
	char *token, *token_saveptr;
	size_t num_tokens;
	unsigned int *parsed[] = {&result->child_id,
				  &result->parent_id,
				  &result->length};
	for (token = strtok_r(line, " ", &token_saveptr), num_tokens = 0;
	     token && num_tokens < 3;
	     token = strtok_r(NULL, " ", &token_saveptr), ++num_tokens) {
		char* endptr = NULL;
		*parsed[num_tokens] = strtoul(token, &endptr, 10);
		if ((*parsed[num_tokens] == ULONG_MAX && errno) || *endptr) {
			fprintf(stderr, "Malformed u/gid mapping line\n");
			return 0;
		}
	}
	if (num_tokens < 3 || strtok_r(NULL, " ", &token_saveptr) != NULL) {
		fprintf(stderr, "Malformed u/gid mapping line\n");
		return 0;
	}
	if (result->child_id + result->length < result->child_id ||
	    result->parent_id + result->length < result->parent_id) {
		fprintf(stderr, "u/gid mapping overflow\n");
		return 0;
	}
	return 1;
}

/*
 * Returns 1 if [begin1, begin1+length1) and [begin2, begin2+length2) have
 * overlapping range. Otherwise 0.
 */
static int is_overlapping(unsigned int begin1, unsigned int length1,
			  unsigned int begin2, unsigned int length2)
{
	unsigned int end1 = begin1 + length1;
	unsigned int end2 = begin2 + length2;
	return !(end1 <= begin2 || end2 <= begin1);
}

/*
 * Verifies if the given mapping works.
 * - Checks if the number of entries is less than or equals to
 *   UID_GID_MAP_MAX_EXTENTS.
 * - Checks if there is no overlapped ranges.
 * Returns 1 if valid, otherwise 0.
 */
static int is_valid_ugid_map(const struct ugid_map* mapping)
{
	size_t i, j;

	if (mapping->size > UID_GID_MAP_MAX_EXTENTS) {
		fprintf(stderr, "too many u/gid mapping entries\n");
		return 0;
	}

	for (i = 0; i < mapping->size; ++i) {
		const struct ugid_map_entry *entry1 = &mapping->entries[i];
		for (j = i + 1; j < mapping->size; ++j) {
			const struct ugid_map_entry *entry2 =
				&mapping->entries[j];
			if (is_overlapping(entry1->child_id, entry1->length,
					   entry2->child_id, entry2->length)) {
				fprintf(stderr,
					"Overlapping child u/gid: [%d %d %d],"
					" [%d %d %d]\n",
					entry1->child_id, entry1->parent_id,
					entry1->length, entry2->child_id,
					entry2->parent_id, entry2->length);
				return 0;
			}
			if (is_overlapping(entry1->parent_id, entry1->length,
					   entry2->parent_id, entry2->length)) {
				fprintf(stderr,
					"Overlapping parent u/gid: [%d %d %d],"
					" [%d %d %d]\n",
					entry1->child_id, entry1->parent_id,
					entry1->length, entry2->child_id,
					entry2->parent_id, entry2->length);
				return 0;
			}
		}
	}
	return 1;
}

/*
 * Parses the UID/GID mapping argument. The argument could be a multi-line
 * string (separated by '\n', no trailing '\n' is allowed). Each line must
 * contain exact three integer tokens; the first token is |child_id|,
 * the second is |parent_id|, and the last is |length| of the mapping range.
 * See also user_namespace(7) man page.
 * On success, the parsed entries are stored in |result|, and it returns 1.
 * Otherwise, returns 0.
 */
static int parse_ugid_map(char* arg, struct ugid_map* result)
{
	int i;
	char *line, *line_saveptr;
	size_t current_index;

	/* Count the number of lines. */
	result->size = 1;
	for (i = 0; arg[i]; ++i) {
		if (arg[i] == '\n')
			++result->size;
	}

	/* Allocate memory for entries. */
	result->entries = malloc(sizeof(struct ugid_map_entry) * result->size);
	if (!result->entries) {
		result->size = 0;
		return 0;
	}

	/* Parse each line */
	for (line = strtok_r(arg, "\n", &line_saveptr), current_index = 0;
	     line;
	     line = strtok_r(NULL, "\n", &line_saveptr), ++current_index) {
		if (!parse_ugid_map_entry(
			line, &result->entries[current_index])) {
			return 0;
		}
	}

	return is_valid_ugid_map(result);
}

int main(int argc, char *argv[])
{
	int c;
	char *p;
	int flags = EXT2_FLAG_RW;
	errcode_t retval;
	io_manager io_mgr;
	ext2_filsys fs = NULL;
	struct fs_ops_callbacks fs_callbacks = { NULL, NULL };
	char *token;
	int nr_opt = 0;
	ext2_ino_t inodes_count;
	ext2_ino_t free_inodes_count;
	blk64_t blocks_count;
	blk64_t free_blocks_count;
	struct ugid_map uid_map = { 0, NULL }, gid_map = { 0, NULL };

	add_error_table(&et_ext2_error_table);

	while ((c = getopt (argc, argv, "T:C:S:p:a:D:d:B:f:esu:g:")) != EOF) {
		switch (c) {
		case 'T':
			fixed_time = strtoul(optarg, &p, 0);
			android_configure = 1;
			break;
		case 'C':
			fs_config_file = absolute_path(optarg);
			android_configure = 1;
			break;
		case 'S':
			token = strtok(optarg, ",");
			while (token) {
				if (nr_opt == max_nr_opt) {
					fprintf(stderr, "Expected at most %d selinux opts\n",
						max_nr_opt);
					exit(EXIT_FAILURE);
				}
				seopt_file[nr_opt].type = SELABEL_OPT_PATH;
				seopt_file[nr_opt].value = absolute_path(token);
				nr_opt++;
				token = strtok(NULL, ",");
			}
			android_configure = 1;
			break;
		case 'p':
			product_out = absolute_path(optarg);
			android_configure = 1;
			break;
		case 'a':
			mountpoint = strdup(optarg);
			break;
		case 'D':
			basefs_out = absolute_path(optarg);
			break;
		case 'd':
			basefs_in = absolute_path(optarg);
			break;
		case 'B':
			block_list = absolute_path(optarg);
			break;
		case 'f':
			src_dir = absolute_path(optarg);
			break;
		case 'e':
			android_sparse_file = 0;
			break;
		case 's':
			flags |= EXT2_FLAG_SHARE_DUP;
			break;
		case 'u':
			if (!parse_ugid_map(optarg, &uid_map))
				exit(EXIT_FAILURE);
			android_configure = 1;
			break;
		case 'g':
			if (!parse_ugid_map(optarg, &gid_map))
				exit(EXIT_FAILURE);
			android_configure = 1;
			break;
		default:
			usage(EXIT_FAILURE);
		}
	}
	if (optind >= argc) {
		fprintf(stderr, "Expected filename after options\n");
		exit(EXIT_FAILURE);
	}

	if (android_sparse_file) {
		io_mgr = sparse_io_manager;
		if (asprintf(&in_file, "(%s)", argv[optind]) == -1) {
			fprintf(stderr, "Failed to allocate file name\n");
			exit(EXIT_FAILURE);
		}
	} else {
		io_mgr = unix_io_manager;
		in_file = strdup(argv[optind]);
	}
	retval = ext2fs_open(in_file, flags, 0, 0, io_mgr, &fs);
	if (retval) {
		com_err(prog_name, retval, "while opening file %s\n", in_file);
		return retval;
	}

	if (src_dir) {
		ext2fs_read_bitmaps(fs);
		if (basefs_in) {
			retval = base_fs_alloc_load(fs, basefs_in, mountpoint,
				src_dir);
			if (retval) {
				com_err(prog_name, retval, "%s",
				"while reading base_fs file");
			    exit(1);
			}
			fs_callbacks.create_new_inode =
				base_fs_alloc_set_target;
			fs_callbacks.end_create_new_inode =
				base_fs_alloc_unset_target;
		}
		retval = populate_fs2(fs, EXT2_ROOT_INO, src_dir,
				      EXT2_ROOT_INO, &fs_callbacks);
		if (retval) {
			com_err(prog_name, retval, "%s",
			"while populating file system");
		    exit(1);
		}
		if (basefs_in)
			base_fs_alloc_cleanup(fs);
	}

	if (android_configure) {
		retval = android_configure_fs(
			fs, src_dir, product_out, mountpoint, seopt_file,
			nr_opt, fs_config_file, fixed_time, &uid_map, &gid_map);
		if (retval) {
			com_err(prog_name, retval, "%s",
				"while configuring the file system");
			exit(1);
		}
	}

	if (block_list) {
		retval = fsmap_iter_filsys(fs, &block_list_format, block_list,
					   mountpoint);
		if (retval) {
			com_err(prog_name, retval, "%s",
				"while creating the block_list");
			exit(1);
		}
	}

	if (basefs_out) {
		retval = fsmap_iter_filsys(fs, &base_fs_format,
					   basefs_out, mountpoint);
		if (retval) {
			com_err(prog_name, retval, "%s",
				"while creating the basefs file");
			exit(1);
		}
	}

	inodes_count = fs->super->s_inodes_count;
	free_inodes_count = fs->super->s_free_inodes_count;
	blocks_count = ext2fs_blocks_count(fs->super);
	free_blocks_count = ext2fs_free_blocks_count(fs->super);

	retval = ext2fs_close_free(&fs);
	if (retval) {
		com_err(prog_name, retval, "%s",
				"while writing superblocks");
		exit(1);
	}

	printf("Created filesystem with %u/%u inodes and %llu/%llu blocks\n",
			inodes_count - free_inodes_count, inodes_count,
			blocks_count - free_blocks_count, blocks_count);

	remove_error_table(&et_ext2_error_table);
	return 0;
}
