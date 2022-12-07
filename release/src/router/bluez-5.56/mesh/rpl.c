// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2020  Intel Corporation. All rights reserved.
 *
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define _GNU_SOURCE
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <unistd.h>
#include <dirent.h>

#include <sys/stat.h>

#include <ell/ell.h>

#include "mesh/mesh-defs.h"

#include "mesh/node.h"
#include "mesh/net.h"
#include "mesh/util.h"
#include "mesh/rpl.h"

const char *rpl_dir = "/rpl";

bool rpl_put_entry(struct mesh_node *node, uint16_t src, uint32_t iv_index,
								uint32_t seq)
{
	const char *node_path;
	char src_file[PATH_MAX];
	char seq_txt[7];
	bool result = false;
	DIR *dir;
	int fd;

	if (!IS_UNICAST(src))
		return false;

	node_path = node_get_storage_dir(node);

	if (strlen(node_path) + strlen(rpl_dir) + 15 >= PATH_MAX)
		return false;

	snprintf(src_file, PATH_MAX, "%s%s/%8.8x", node_path, rpl_dir,
								iv_index);
	dir = opendir(src_file);

	if (!dir)
		mkdir(src_file, 0755);
	else
		closedir(dir);

	snprintf(src_file, PATH_MAX, "%s%s/%8.8x/%4.4x", node_path, rpl_dir,
								iv_index, src);

	fd = open(src_file, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
	if (fd >= 0) {
		snprintf(seq_txt, 7, "%6.6x", seq);
		if (write(fd, seq_txt, 6) == 6)
			result = true;

		close(fd);
	}

	if (!result)
		return false;

	/* Delete RPL entry from old iv_index (if it exists) */
	iv_index--;
	snprintf(src_file, PATH_MAX, "%s%s/%8.8x/%4.4x", node_path, rpl_dir,
								iv_index, src);
	remove(src_file);


	return result;
}

void rpl_del_entry(struct mesh_node *node, uint16_t src)
{
	const char *node_path;
	char rpl_path[PATH_MAX];
	struct dirent *entry;
	DIR *dir;

	if (!IS_UNICAST(src))
		return;

	node_path = node_get_storage_dir(node);

	if (strlen(node_path) + strlen(rpl_dir) + 15 >= PATH_MAX)
		return;

	snprintf(rpl_path, PATH_MAX, "%s%s", node_path, rpl_dir);
	dir = opendir(rpl_path);

	if (!dir)
		return;

	/* Remove all instances of src address */
	while ((entry = readdir(dir)) != NULL) {
		if (entry->d_type == DT_DIR && entry->d_name[0] != '.') {
			snprintf(rpl_path, PATH_MAX, "%s%s/%s/%4.4x",
					node_path, rpl_dir, entry->d_name, src);
			remove(rpl_path);
		}
	}

	closedir(dir);
}

static bool match_src(const void *a, const void *b)
{
	const struct mesh_rpl *rpl = a;
	uint16_t src = L_PTR_TO_UINT(b);

	return rpl->src == src;
}

static void get_entries(const char *iv_path, struct l_queue *rpl_list)
{
	struct mesh_rpl *rpl;
	struct dirent *entry;
	DIR *dir;
	int fd;
	const char *iv_txt;
	char src_path[PATH_MAX];
	char seq_txt[7];
	uint32_t iv_index, seq;
	uint16_t src;

	dir = opendir(iv_path);

	if (!dir)
		return;

	iv_txt = basename(iv_path);
	if (sscanf(iv_txt, "%08x", &iv_index) != 1)
		return;

	memset(seq_txt, 0, sizeof(seq_txt));

	while ((entry = readdir(dir)) != NULL) {
		/* RPL sequences are stored in src files under iv_index */
		if (entry->d_type == DT_REG) {
			if (sscanf(entry->d_name, "%04hx", &src) != 1)
				continue;

			snprintf(src_path, PATH_MAX, "%s/%4.4x", iv_path, src);
			fd = open(src_path, O_RDONLY);

			if (fd < 0)
				continue;

			if (read(fd, seq_txt, 6) == 6 &&
					sscanf(seq_txt, "%06x", &seq) == 1) {

				rpl = l_queue_find(rpl_list, match_src,
						L_UINT_TO_PTR(src));

				if (rpl) {
					/* Replace older entries */
					if (rpl->iv_index < iv_index) {
						rpl->iv_index = iv_index;
						rpl->seq = seq;
					}
				} else if (seq <= SEQ_MASK && IS_UNICAST(src)) {
					rpl = l_new(struct mesh_rpl, 1);
					rpl->src = src;
					rpl->iv_index = iv_index;
					rpl->seq = seq;

					l_queue_push_head(rpl_list, rpl);
				}
			}
			close(fd);
		}
	}

	closedir(dir);
}

bool rpl_get_list(struct mesh_node *node, struct l_queue *rpl_list)
{
	const char *node_path;
	struct dirent *entry;
	char *rpl_path;
	size_t len;
	DIR *dir;

	if (!rpl_list)
		return false;

	node_path = node_get_storage_dir(node);

	len = strlen(node_path) + strlen(rpl_dir) + 15;

	if (len > PATH_MAX)
		return false;

	rpl_path = l_malloc(len);
	snprintf(rpl_path, len, "%s%s", node_path, rpl_dir);

	dir = opendir(rpl_path);

	if (!dir) {
		l_error("Failed to read RPL dir: %s", rpl_path);
		l_free(rpl_path);
		return false;
	}

	while ((entry = readdir(dir)) != NULL) {
		/* RPL sequences are stored in files under iv_indexs */
		if (entry->d_type == DT_DIR && entry->d_name[0] != '.') {
			snprintf(rpl_path, len, "%s%s/%s",
					node_path, rpl_dir, entry->d_name);
			get_entries(rpl_path, rpl_list);
		}
	}

	l_free(rpl_path);
	closedir(dir);

	return true;
}

void rpl_update(struct mesh_node *node, uint32_t cur)
{
	uint32_t old = cur - 1;
	const char *node_path;
	struct dirent *entry;
	char path[PATH_MAX];
	DIR *dir;

	node_path = node_get_storage_dir(node);
	if (!node_path)
		return;

	if (strlen(node_path) + strlen(rpl_dir) + 15 >= PATH_MAX)
		return;

	/* Make sure path exists */
	snprintf(path, PATH_MAX, "%s%s", node_path, rpl_dir);
	mkdir(path, 0755);

	dir = opendir(path);
	if (!dir)
		return;

	/* Cleanup any stale or malformed trees */
	while ((entry = readdir(dir)) != NULL) {
		if (entry->d_type == DT_DIR && entry->d_name[0] != '.') {
			uint32_t val;
			bool del = false;

			if (strlen(entry->d_name) != 8)
				del = true;
			else if (sscanf(entry->d_name, "%08x", &val) != 1)
				del = true;

			/* Delete all invalid iv_index trees */
			if (del || (val != cur && val != old)) {
				snprintf(path, PATH_MAX, "%s%s/%s",
					node_path, rpl_dir, entry->d_name);
				del_path(path);
			}
		}
	}

	closedir(dir);
}

bool rpl_init(const char *node_path)
{
	char path[PATH_MAX];

	if (strlen(node_path) + strlen(rpl_dir) + 15 >= PATH_MAX)
		return false;

	snprintf(path, PATH_MAX, "%s%s", node_path, rpl_dir);
	mkdir(path, 0755);
	return true;
}
