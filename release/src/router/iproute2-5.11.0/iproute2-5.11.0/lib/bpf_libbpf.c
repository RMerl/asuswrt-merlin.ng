/* SPDX-License-Identifier: GPL-2.0 */
/*
 * bpf_libbpf.c		BPF code relay on libbpf
 * Authors:		Hangbin Liu <haliu@redhat.com>
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <errno.h>
#include <fcntl.h>

#include <libelf.h>
#include <gelf.h>

#include <bpf/libbpf.h>
#include <bpf/bpf.h>

#include "bpf_util.h"

static int verbose_print(enum libbpf_print_level level, const char *format, va_list args)
{
	return vfprintf(stderr, format, args);
}

static int silent_print(enum libbpf_print_level level, const char *format, va_list args)
{
	if (level > LIBBPF_WARN)
		return 0;

	/* Skip warning from bpf_object__init_user_maps() for legacy maps */
	if (strstr(format, "has unrecognized, non-zero options"))
		return 0;

	return vfprintf(stderr, format, args);
}

static const char *get_bpf_program__section_name(const struct bpf_program *prog)
{
#ifdef HAVE_LIBBPF_SECTION_NAME
	return bpf_program__section_name(prog);
#else
	return bpf_program__title(prog, false);
#endif
}

static int create_map(const char *name, struct bpf_elf_map *map,
		      __u32 ifindex, int inner_fd)
{
	struct bpf_create_map_attr map_attr = {};

	map_attr.name = name;
	map_attr.map_type = map->type;
	map_attr.map_flags = map->flags;
	map_attr.key_size = map->size_key;
	map_attr.value_size = map->size_value;
	map_attr.max_entries = map->max_elem;
	map_attr.map_ifindex = ifindex;
	map_attr.inner_map_fd = inner_fd;

	return bpf_create_map_xattr(&map_attr);
}

static int create_map_in_map(struct bpf_object *obj, struct bpf_map *map,
			     struct bpf_elf_map *elf_map, int inner_fd,
			     bool *reuse_pin_map)
{
	char pathname[PATH_MAX];
	const char *map_name;
	bool pin_map = false;
	int map_fd, ret = 0;

	map_name = bpf_map__name(map);

	if (iproute2_is_pin_map(map_name, pathname)) {
		pin_map = true;

		/* Check if there already has a pinned map */
		map_fd = bpf_obj_get(pathname);
		if (map_fd > 0) {
			if (reuse_pin_map)
				*reuse_pin_map = true;
			close(map_fd);
			return bpf_map__set_pin_path(map, pathname);
		}
	}

	map_fd = create_map(map_name, elf_map, bpf_map__ifindex(map), inner_fd);
	if (map_fd < 0) {
		fprintf(stderr, "create map %s failed\n", map_name);
		return map_fd;
	}

	ret = bpf_map__reuse_fd(map, map_fd);
	if (ret < 0) {
		fprintf(stderr, "map %s reuse fd failed\n", map_name);
		goto err_out;
	}

	if (pin_map) {
		ret = bpf_map__set_pin_path(map, pathname);
		if (ret < 0)
			goto err_out;
	}

	return 0;
err_out:
	close(map_fd);
	return ret;
}

static int
handle_legacy_map_in_map(struct bpf_object *obj, struct bpf_map *inner_map,
			 const char *inner_map_name)
{
	int inner_fd, outer_fd, inner_idx, ret = 0;
	struct bpf_elf_map imap, omap;
	struct bpf_map *outer_map;
	/* What's the size limit of map name? */
	char outer_map_name[128];
	bool reuse_pin_map = false;

	/* Deal with map-in-map */
	if (iproute2_is_map_in_map(inner_map_name, &imap, &omap, outer_map_name)) {
		ret = create_map_in_map(obj, inner_map, &imap, -1, NULL);
		if (ret < 0)
			return ret;

		inner_fd = bpf_map__fd(inner_map);
		outer_map = bpf_object__find_map_by_name(obj, outer_map_name);
		ret = create_map_in_map(obj, outer_map, &omap, inner_fd, &reuse_pin_map);
		if (ret < 0)
			return ret;

		if (!reuse_pin_map) {
			inner_idx = imap.inner_idx;
			outer_fd = bpf_map__fd(outer_map);
			ret = bpf_map_update_elem(outer_fd, &inner_idx, &inner_fd, 0);
			if (ret < 0)
				fprintf(stderr, "Cannot update inner_idx into outer_map\n");
		}
	}

	return ret;
}

static int find_legacy_tail_calls(struct bpf_program *prog, struct bpf_object *obj)
{
	unsigned int map_id, key_id;
	const char *sec_name;
	struct bpf_map *map;
	char map_name[128];
	int ret;

	/* Handle iproute2 tail call */
	sec_name = get_bpf_program__section_name(prog);
	ret = sscanf(sec_name, "%i/%i", &map_id, &key_id);
	if (ret != 2)
		return -1;

	ret = iproute2_find_map_name_by_id(map_id, map_name);
	if (ret < 0) {
		fprintf(stderr, "unable to find map id %u for tail call\n", map_id);
		return ret;
	}

	map = bpf_object__find_map_by_name(obj, map_name);
	if (!map)
		return -1;

	/* Save the map here for later updating */
	bpf_program__set_priv(prog, map, NULL);

	return 0;
}

static int update_legacy_tail_call_maps(struct bpf_object *obj)
{
	int prog_fd, map_fd, ret = 0;
	unsigned int map_id, key_id;
	struct bpf_program *prog;
	const char *sec_name;
	struct bpf_map *map;

	bpf_object__for_each_program(prog, obj) {
		map = bpf_program__priv(prog);
		if (!map)
			continue;

		prog_fd = bpf_program__fd(prog);
		if (prog_fd < 0)
			continue;

		sec_name = get_bpf_program__section_name(prog);
		ret = sscanf(sec_name, "%i/%i", &map_id, &key_id);
		if (ret != 2)
			continue;

		map_fd = bpf_map__fd(map);
		ret = bpf_map_update_elem(map_fd, &key_id, &prog_fd, 0);
		if (ret < 0) {
			fprintf(stderr, "Cannot update map key for tail call!\n");
			return ret;
		}
	}

	return 0;
}

static int handle_legacy_maps(struct bpf_object *obj)
{
	char pathname[PATH_MAX];
	struct bpf_map *map;
	const char *map_name;
	int map_fd, ret = 0;

	bpf_object__for_each_map(map, obj) {
		map_name = bpf_map__name(map);

		ret = handle_legacy_map_in_map(obj, map, map_name);
		if (ret)
			return ret;

		/* If it is a iproute2 legacy pin maps, just set pin path
		 * and let bpf_object__load() to deal with the map creation.
		 * We need to ignore map-in-maps which have pinned maps manually
		 */
		map_fd = bpf_map__fd(map);
		if (map_fd < 0 && iproute2_is_pin_map(map_name, pathname)) {
			ret = bpf_map__set_pin_path(map, pathname);
			if (ret) {
				fprintf(stderr, "map '%s': couldn't set pin path.\n", map_name);
				break;
			}
		}

	}

	return ret;
}

static int load_bpf_object(struct bpf_cfg_in *cfg)
{
	struct bpf_program *p, *prog = NULL;
	struct bpf_object *obj;
	char root_path[PATH_MAX];
	struct bpf_map *map;
	int prog_fd, ret = 0;

	ret = iproute2_get_root_path(root_path, PATH_MAX);
	if (ret)
		return ret;

	DECLARE_LIBBPF_OPTS(bpf_object_open_opts, open_opts,
			.relaxed_maps = true,
			.pin_root_path = root_path,
	);

	obj = bpf_object__open_file(cfg->object, &open_opts);
	if (libbpf_get_error(obj)) {
		fprintf(stderr, "ERROR: opening BPF object file failed\n");
		return -ENOENT;
	}

	bpf_object__for_each_program(p, obj) {
		/* Only load the programs that will either be subsequently
		 * attached or inserted into a tail call map */
		if (find_legacy_tail_calls(p, obj) < 0 && cfg->section &&
		    strcmp(get_bpf_program__section_name(p), cfg->section)) {
			ret = bpf_program__set_autoload(p, false);
			if (ret)
				return -EINVAL;
			continue;
		}

		bpf_program__set_type(p, cfg->type);
		bpf_program__set_ifindex(p, cfg->ifindex);
		if (!prog)
			prog = p;
	}

	bpf_object__for_each_map(map, obj) {
		if (!bpf_map__is_offload_neutral(map))
			bpf_map__set_ifindex(map, cfg->ifindex);
	}

	if (!prog) {
		fprintf(stderr, "object file doesn't contain sec %s\n", cfg->section);
		return -ENOENT;
	}

	/* Handle iproute2 legacy pin maps and map-in-maps */
	ret = handle_legacy_maps(obj);
	if (ret)
		goto unload_obj;

	ret = bpf_object__load(obj);
	if (ret)
		goto unload_obj;

	ret = update_legacy_tail_call_maps(obj);
	if (ret)
		goto unload_obj;

	prog_fd = fcntl(bpf_program__fd(prog), F_DUPFD_CLOEXEC, 1);
	if (prog_fd < 0)
		ret = -errno;
	else
		cfg->prog_fd = prog_fd;

unload_obj:
	/* Close obj as we don't need it */
	bpf_object__close(obj);
	return ret;
}

/* Load ebpf and return prog fd */
int iproute2_load_libbpf(struct bpf_cfg_in *cfg)
{
	int ret = 0;

	if (cfg->verbose)
		libbpf_set_print(verbose_print);
	else
		libbpf_set_print(silent_print);

	ret = iproute2_bpf_elf_ctx_init(cfg);
	if (ret < 0) {
		fprintf(stderr, "Cannot initialize ELF context!\n");
		return ret;
	}

	ret = iproute2_bpf_fetch_ancillary();
	if (ret < 0) {
		fprintf(stderr, "Error fetching ELF ancillary data!\n");
		return ret;
	}

	ret = load_bpf_object(cfg);
	if (ret)
		return ret;

	return cfg->prog_fd;
}
