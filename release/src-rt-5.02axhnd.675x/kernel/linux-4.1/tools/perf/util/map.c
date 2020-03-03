#include "symbol.h"
#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include "map.h"
#include "thread.h"
#include "strlist.h"
#include "vdso.h"
#include "build-id.h"
#include "util.h"
#include "debug.h"
#include "machine.h"
#include <linux/string.h>

const char *map_type__name[MAP__NR_TYPES] = {
	[MAP__FUNCTION] = "Functions",
	[MAP__VARIABLE] = "Variables",
};

static inline int is_anon_memory(const char *filename)
{
	return !strcmp(filename, "//anon") ||
	       !strcmp(filename, "/dev/zero (deleted)") ||
	       !strcmp(filename, "/anon_hugepage (deleted)");
}

static inline int is_no_dso_memory(const char *filename)
{
	return !strncmp(filename, "[stack", 6) ||
	       !strncmp(filename, "/SYSV",5)   ||
	       !strcmp(filename, "[heap]");
}

static inline int is_android_lib(const char *filename)
{
	return !strncmp(filename, "/data/app-lib", 13) ||
	       !strncmp(filename, "/system/lib", 11);
}

static inline bool replace_android_lib(const char *filename, char *newfilename)
{
	const char *libname;
	char *app_abi;
	size_t app_abi_length, new_length;
	size_t lib_length = 0;

	libname  = strrchr(filename, '/');
	if (libname)
		lib_length = strlen(libname);

	app_abi = getenv("APP_ABI");
	if (!app_abi)
		return false;

	app_abi_length = strlen(app_abi);

	if (!strncmp(filename, "/data/app-lib", 13)) {
		char *apk_path;

		if (!app_abi_length)
			return false;

		new_length = 7 + app_abi_length + lib_length;

		apk_path = getenv("APK_PATH");
		if (apk_path) {
			new_length += strlen(apk_path) + 1;
			if (new_length > PATH_MAX)
				return false;
			snprintf(newfilename, new_length,
				 "%s/libs/%s/%s", apk_path, app_abi, libname);
		} else {
			if (new_length > PATH_MAX)
				return false;
			snprintf(newfilename, new_length,
				 "libs/%s/%s", app_abi, libname);
		}
		return true;
	}

	if (!strncmp(filename, "/system/lib/", 11)) {
		char *ndk, *app;
		const char *arch;
		size_t ndk_length;
		size_t app_length;

		ndk = getenv("NDK_ROOT");
		app = getenv("APP_PLATFORM");

		if (!(ndk && app))
			return false;

		ndk_length = strlen(ndk);
		app_length = strlen(app);

		if (!(ndk_length && app_length && app_abi_length))
			return false;

		arch = !strncmp(app_abi, "arm", 3) ? "arm" :
		       !strncmp(app_abi, "mips", 4) ? "mips" :
		       !strncmp(app_abi, "x86", 3) ? "x86" : NULL;

		if (!arch)
			return false;

		new_length = 27 + ndk_length +
			     app_length + lib_length
			   + strlen(arch);

		if (new_length > PATH_MAX)
			return false;
		snprintf(newfilename, new_length,
			"%s/platforms/%s/arch-%s/usr/lib/%s",
			ndk, app, arch, libname);

		return true;
	}
	return false;
}

void map__init(struct map *map, enum map_type type,
	       u64 start, u64 end, u64 pgoff, struct dso *dso)
{
	map->type     = type;
	map->start    = start;
	map->end      = end;
	map->pgoff    = pgoff;
	map->reloc    = 0;
	map->dso      = dso;
	map->map_ip   = map__map_ip;
	map->unmap_ip = map__unmap_ip;
	RB_CLEAR_NODE(&map->rb_node);
	map->groups   = NULL;
	map->referenced = false;
	map->erange_warned = false;
}

struct map *map__new(struct machine *machine, u64 start, u64 len,
		     u64 pgoff, u32 pid, u32 d_maj, u32 d_min, u64 ino,
		     u64 ino_gen, u32 prot, u32 flags, char *filename,
		     enum map_type type, struct thread *thread)
{
	struct map *map = malloc(sizeof(*map));

	if (map != NULL) {
		char newfilename[PATH_MAX];
		struct dso *dso;
		int anon, no_dso, vdso, android;

		android = is_android_lib(filename);
		anon = is_anon_memory(filename);
		vdso = is_vdso_map(filename);
		no_dso = is_no_dso_memory(filename);

		map->maj = d_maj;
		map->min = d_min;
		map->ino = ino;
		map->ino_generation = ino_gen;
		map->prot = prot;
		map->flags = flags;

		if ((anon || no_dso) && type == MAP__FUNCTION) {
			snprintf(newfilename, sizeof(newfilename), "/tmp/perf-%d.map", pid);
			filename = newfilename;
		}

		if (android) {
			if (replace_android_lib(filename, newfilename))
				filename = newfilename;
		}

		if (vdso) {
			pgoff = 0;
			dso = vdso__dso_findnew(machine, thread);
		} else
			dso = __dsos__findnew(&machine->user_dsos, filename);

		if (dso == NULL)
			goto out_delete;

		map__init(map, type, start, start + len, pgoff, dso);

		if (anon || no_dso) {
			map->map_ip = map->unmap_ip = identity__map_ip;

			/*
			 * Set memory without DSO as loaded. All map__find_*
			 * functions still return NULL, and we avoid the
			 * unnecessary map__load warning.
			 */
			if (type != MAP__FUNCTION)
				dso__set_loaded(dso, map->type);
		}
	}
	return map;
out_delete:
	free(map);
	return NULL;
}

/*
 * Constructor variant for modules (where we know from /proc/modules where
 * they are loaded) and for vmlinux, where only after we load all the
 * symbols we'll know where it starts and ends.
 */
struct map *map__new2(u64 start, struct dso *dso, enum map_type type)
{
	struct map *map = calloc(1, (sizeof(*map) +
				     (dso->kernel ? sizeof(struct kmap) : 0)));
	if (map != NULL) {
		/*
		 * ->end will be filled after we load all the symbols
		 */
		map__init(map, type, start, 0, 0, dso);
	}

	return map;
}

void map__delete(struct map *map)
{
	free(map);
}

void map__fixup_start(struct map *map)
{
	struct rb_root *symbols = &map->dso->symbols[map->type];
	struct rb_node *nd = rb_first(symbols);
	if (nd != NULL) {
		struct symbol *sym = rb_entry(nd, struct symbol, rb_node);
		map->start = sym->start;
	}
}

void map__fixup_end(struct map *map)
{
	struct rb_root *symbols = &map->dso->symbols[map->type];
	struct rb_node *nd = rb_last(symbols);
	if (nd != NULL) {
		struct symbol *sym = rb_entry(nd, struct symbol, rb_node);
		map->end = sym->end;
	}
}

#define DSO__DELETED "(deleted)"

int map__load(struct map *map, symbol_filter_t filter)
{
	const char *name = map->dso->long_name;
	int nr;

	if (dso__loaded(map->dso, map->type))
		return 0;

	nr = dso__load(map->dso, map, filter);
	if (nr < 0) {
		if (map->dso->has_build_id) {
			char sbuild_id[BUILD_ID_SIZE * 2 + 1];

			build_id__sprintf(map->dso->build_id,
					  sizeof(map->dso->build_id),
					  sbuild_id);
			pr_warning("%s with build id %s not found",
				   name, sbuild_id);
		} else
			pr_warning("Failed to open %s", name);

		pr_warning(", continuing without symbols\n");
		return -1;
	} else if (nr == 0) {
#ifdef HAVE_LIBELF_SUPPORT
		const size_t len = strlen(name);
		const size_t real_len = len - sizeof(DSO__DELETED);

		if (len > sizeof(DSO__DELETED) &&
		    strcmp(name + real_len + 1, DSO__DELETED) == 0) {
			pr_warning("%.*s was updated (is prelink enabled?). "
				"Restart the long running apps that use it!\n",
				   (int)real_len, name);
		} else {
			pr_warning("no symbols found in %s, maybe install "
				   "a debug package?\n", name);
		}
#endif
		return -1;
	}

	return 0;
}

struct symbol *map__find_symbol(struct map *map, u64 addr,
				symbol_filter_t filter)
{
	if (map__load(map, filter) < 0)
		return NULL;

	return dso__find_symbol(map->dso, map->type, addr);
}

struct symbol *map__find_symbol_by_name(struct map *map, const char *name,
					symbol_filter_t filter)
{
	if (map__load(map, filter) < 0)
		return NULL;

	if (!dso__sorted_by_name(map->dso, map->type))
		dso__sort_by_name(map->dso, map->type);

	return dso__find_symbol_by_name(map->dso, map->type, name);
}

struct map *map__clone(struct map *map)
{
	return memdup(map, sizeof(*map));
}

int map__overlap(struct map *l, struct map *r)
{
	if (l->start > r->start) {
		struct map *t = l;
		l = r;
		r = t;
	}

	if (l->end > r->start)
		return 1;

	return 0;
}

size_t map__fprintf(struct map *map, FILE *fp)
{
	return fprintf(fp, " %" PRIx64 "-%" PRIx64 " %" PRIx64 " %s\n",
		       map->start, map->end, map->pgoff, map->dso->name);
}

size_t map__fprintf_dsoname(struct map *map, FILE *fp)
{
	const char *dsoname = "[unknown]";

	if (map && map->dso && (map->dso->name || map->dso->long_name)) {
		if (symbol_conf.show_kernel_path && map->dso->long_name)
			dsoname = map->dso->long_name;
		else if (map->dso->name)
			dsoname = map->dso->name;
	}

	return fprintf(fp, "%s", dsoname);
}

int map__fprintf_srcline(struct map *map, u64 addr, const char *prefix,
			 FILE *fp)
{
	char *srcline;
	int ret = 0;

	if (map && map->dso) {
		srcline = get_srcline(map->dso,
				      map__rip_2objdump(map, addr), NULL, true);
		if (srcline != SRCLINE_UNKNOWN)
			ret = fprintf(fp, "%s%s", prefix, srcline);
		free_srcline(srcline);
	}
	return ret;
}

/**
 * map__rip_2objdump - convert symbol start address to objdump address.
 * @map: memory map
 * @rip: symbol start address
 *
 * objdump wants/reports absolute IPs for ET_EXEC, and RIPs for ET_DYN.
 * map->dso->adjust_symbols==1 for ET_EXEC-like cases except ET_REL which is
 * relative to section start.
 *
 * Return: Address suitable for passing to "objdump --start-address="
 */
u64 map__rip_2objdump(struct map *map, u64 rip)
{
	if (!map->dso->adjust_symbols)
		return rip;

	if (map->dso->rel)
		return rip - map->pgoff;

	return map->unmap_ip(map, rip) - map->reloc;
}

/**
 * map__objdump_2mem - convert objdump address to a memory address.
 * @map: memory map
 * @ip: objdump address
 *
 * Closely related to map__rip_2objdump(), this function takes an address from
 * objdump and converts it to a memory address.  Note this assumes that @map
 * contains the address.  To be sure the result is valid, check it forwards
 * e.g. map__rip_2objdump(map->map_ip(map, map__objdump_2mem(map, ip))) == ip
 *
 * Return: Memory address.
 */
u64 map__objdump_2mem(struct map *map, u64 ip)
{
	if (!map->dso->adjust_symbols)
		return map->unmap_ip(map, ip);

	if (map->dso->rel)
		return map->unmap_ip(map, ip + map->pgoff);

	return ip + map->reloc;
}

void map_groups__init(struct map_groups *mg, struct machine *machine)
{
	int i;
	for (i = 0; i < MAP__NR_TYPES; ++i) {
		mg->maps[i] = RB_ROOT;
		INIT_LIST_HEAD(&mg->removed_maps[i]);
	}
	mg->machine = machine;
	mg->refcnt = 1;
}

static void maps__delete(struct rb_root *maps)
{
	struct rb_node *next = rb_first(maps);

	while (next) {
		struct map *pos = rb_entry(next, struct map, rb_node);

		next = rb_next(&pos->rb_node);
		rb_erase(&pos->rb_node, maps);
		map__delete(pos);
	}
}

static void maps__delete_removed(struct list_head *maps)
{
	struct map *pos, *n;

	list_for_each_entry_safe(pos, n, maps, node) {
		list_del(&pos->node);
		map__delete(pos);
	}
}

void map_groups__exit(struct map_groups *mg)
{
	int i;

	for (i = 0; i < MAP__NR_TYPES; ++i) {
		maps__delete(&mg->maps[i]);
		maps__delete_removed(&mg->removed_maps[i]);
	}
}

bool map_groups__empty(struct map_groups *mg)
{
	int i;

	for (i = 0; i < MAP__NR_TYPES; ++i) {
		if (maps__first(&mg->maps[i]))
			return false;
		if (!list_empty(&mg->removed_maps[i]))
			return false;
	}

	return true;
}

struct map_groups *map_groups__new(struct machine *machine)
{
	struct map_groups *mg = malloc(sizeof(*mg));

	if (mg != NULL)
		map_groups__init(mg, machine);

	return mg;
}

void map_groups__delete(struct map_groups *mg)
{
	map_groups__exit(mg);
	free(mg);
}

void map_groups__put(struct map_groups *mg)
{
	if (--mg->refcnt == 0)
		map_groups__delete(mg);
}

void map_groups__flush(struct map_groups *mg)
{
	int type;

	for (type = 0; type < MAP__NR_TYPES; type++) {
		struct rb_root *root = &mg->maps[type];
		struct rb_node *next = rb_first(root);

		while (next) {
			struct map *pos = rb_entry(next, struct map, rb_node);
			next = rb_next(&pos->rb_node);
			rb_erase(&pos->rb_node, root);
			/*
			 * We may have references to this map, for
			 * instance in some hist_entry instances, so
			 * just move them to a separate list.
			 */
			list_add_tail(&pos->node, &mg->removed_maps[pos->type]);
		}
	}
}

struct symbol *map_groups__find_symbol(struct map_groups *mg,
				       enum map_type type, u64 addr,
				       struct map **mapp,
				       symbol_filter_t filter)
{
	struct map *map = map_groups__find(mg, type, addr);

	/* Ensure map is loaded before using map->map_ip */
	if (map != NULL && map__load(map, filter) >= 0) {
		if (mapp != NULL)
			*mapp = map;
		return map__find_symbol(map, map->map_ip(map, addr), filter);
	}

	return NULL;
}

struct symbol *map_groups__find_symbol_by_name(struct map_groups *mg,
					       enum map_type type,
					       const char *name,
					       struct map **mapp,
					       symbol_filter_t filter)
{
	struct rb_node *nd;

	for (nd = rb_first(&mg->maps[type]); nd; nd = rb_next(nd)) {
		struct map *pos = rb_entry(nd, struct map, rb_node);
		struct symbol *sym = map__find_symbol_by_name(pos, name, filter);

		if (sym == NULL)
			continue;
		if (mapp != NULL)
			*mapp = pos;
		return sym;
	}

	return NULL;
}

int map_groups__find_ams(struct addr_map_symbol *ams, symbol_filter_t filter)
{
	if (ams->addr < ams->map->start || ams->addr >= ams->map->end) {
		if (ams->map->groups == NULL)
			return -1;
		ams->map = map_groups__find(ams->map->groups, ams->map->type,
					    ams->addr);
		if (ams->map == NULL)
			return -1;
	}

	ams->al_addr = ams->map->map_ip(ams->map, ams->addr);
	ams->sym = map__find_symbol(ams->map, ams->al_addr, filter);

	return ams->sym ? 0 : -1;
}

size_t __map_groups__fprintf_maps(struct map_groups *mg, enum map_type type,
				  FILE *fp)
{
	size_t printed = fprintf(fp, "%s:\n", map_type__name[type]);
	struct rb_node *nd;

	for (nd = rb_first(&mg->maps[type]); nd; nd = rb_next(nd)) {
		struct map *pos = rb_entry(nd, struct map, rb_node);
		printed += fprintf(fp, "Map:");
		printed += map__fprintf(pos, fp);
		if (verbose > 2) {
			printed += dso__fprintf(pos->dso, type, fp);
			printed += fprintf(fp, "--\n");
		}
	}

	return printed;
}

static size_t map_groups__fprintf_maps(struct map_groups *mg, FILE *fp)
{
	size_t printed = 0, i;
	for (i = 0; i < MAP__NR_TYPES; ++i)
		printed += __map_groups__fprintf_maps(mg, i, fp);
	return printed;
}

static size_t __map_groups__fprintf_removed_maps(struct map_groups *mg,
						 enum map_type type, FILE *fp)
{
	struct map *pos;
	size_t printed = 0;

	list_for_each_entry(pos, &mg->removed_maps[type], node) {
		printed += fprintf(fp, "Map:");
		printed += map__fprintf(pos, fp);
		if (verbose > 1) {
			printed += dso__fprintf(pos->dso, type, fp);
			printed += fprintf(fp, "--\n");
		}
	}
	return printed;
}

static size_t map_groups__fprintf_removed_maps(struct map_groups *mg,
					       FILE *fp)
{
	size_t printed = 0, i;
	for (i = 0; i < MAP__NR_TYPES; ++i)
		printed += __map_groups__fprintf_removed_maps(mg, i, fp);
	return printed;
}

size_t map_groups__fprintf(struct map_groups *mg, FILE *fp)
{
	size_t printed = map_groups__fprintf_maps(mg, fp);
	printed += fprintf(fp, "Removed maps:\n");
	return printed + map_groups__fprintf_removed_maps(mg, fp);
}

int map_groups__fixup_overlappings(struct map_groups *mg, struct map *map,
				   FILE *fp)
{
	struct rb_root *root = &mg->maps[map->type];
	struct rb_node *next = rb_first(root);
	int err = 0;

	while (next) {
		struct map *pos = rb_entry(next, struct map, rb_node);
		next = rb_next(&pos->rb_node);

		if (!map__overlap(pos, map))
			continue;

		if (verbose >= 2) {
			fputs("overlapping maps:\n", fp);
			map__fprintf(map, fp);
			map__fprintf(pos, fp);
		}

		rb_erase(&pos->rb_node, root);
		/*
		 * Now check if we need to create new maps for areas not
		 * overlapped by the new map:
		 */
		if (map->start > pos->start) {
			struct map *before = map__clone(pos);

			if (before == NULL) {
				err = -ENOMEM;
				goto move_map;
			}

			before->end = map->start;
			map_groups__insert(mg, before);
			if (verbose >= 2)
				map__fprintf(before, fp);
		}

		if (map->end < pos->end) {
			struct map *after = map__clone(pos);

			if (after == NULL) {
				err = -ENOMEM;
				goto move_map;
			}

			after->start = map->end;
			map_groups__insert(mg, after);
			if (verbose >= 2)
				map__fprintf(after, fp);
		}
move_map:
		/*
		 * If we have references, just move them to a separate list.
		 */
		if (pos->referenced)
			list_add_tail(&pos->node, &mg->removed_maps[map->type]);
		else
			map__delete(pos);

		if (err)
			return err;
	}

	return 0;
}

/*
 * XXX This should not really _copy_ te maps, but refcount them.
 */
int map_groups__clone(struct map_groups *mg,
		      struct map_groups *parent, enum map_type type)
{
	struct rb_node *nd;
	for (nd = rb_first(&parent->maps[type]); nd; nd = rb_next(nd)) {
		struct map *map = rb_entry(nd, struct map, rb_node);
		struct map *new = map__clone(map);
		if (new == NULL)
			return -ENOMEM;
		map_groups__insert(mg, new);
	}
	return 0;
}

void maps__insert(struct rb_root *maps, struct map *map)
{
	struct rb_node **p = &maps->rb_node;
	struct rb_node *parent = NULL;
	const u64 ip = map->start;
	struct map *m;

	while (*p != NULL) {
		parent = *p;
		m = rb_entry(parent, struct map, rb_node);
		if (ip < m->start)
			p = &(*p)->rb_left;
		else
			p = &(*p)->rb_right;
	}

	rb_link_node(&map->rb_node, parent, p);
	rb_insert_color(&map->rb_node, maps);
}

void maps__remove(struct rb_root *maps, struct map *map)
{
	rb_erase(&map->rb_node, maps);
}

struct map *maps__find(struct rb_root *maps, u64 ip)
{
	struct rb_node **p = &maps->rb_node;
	struct rb_node *parent = NULL;
	struct map *m;

	while (*p != NULL) {
		parent = *p;
		m = rb_entry(parent, struct map, rb_node);
		if (ip < m->start)
			p = &(*p)->rb_left;
		else if (ip >= m->end)
			p = &(*p)->rb_right;
		else
			return m;
	}

	return NULL;
}

struct map *maps__first(struct rb_root *maps)
{
	struct rb_node *first = rb_first(maps);

	if (first)
		return rb_entry(first, struct map, rb_node);
	return NULL;
}

struct map *maps__next(struct map *map)
{
	struct rb_node *next = rb_next(&map->rb_node);

	if (next)
		return rb_entry(next, struct map, rb_node);
	return NULL;
}

struct kmap *map__kmap(struct map *map)
{
	if (!map->dso || !map->dso->kernel) {
		pr_err("Internal error: map__kmap with a non-kernel map\n");
		return NULL;
	}
	return (struct kmap *)(map + 1);
}

struct map_groups *map__kmaps(struct map *map)
{
	struct kmap *kmap = map__kmap(map);

	if (!kmap || !kmap->kmaps) {
		pr_err("Internal error: map__kmaps with a non-kernel map\n");
		return NULL;
	}
	return kmap->kmaps;
}
