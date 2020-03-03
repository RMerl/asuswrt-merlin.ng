#include "callchain.h"
#include "debug.h"
#include "event.h"
#include "evsel.h"
#include "hist.h"
#include "machine.h"
#include "map.h"
#include "sort.h"
#include "strlist.h"
#include "thread.h"
#include "vdso.h"
#include <stdbool.h>
#include <symbol/kallsyms.h>
#include "unwind.h"
#include "linux/hash.h"

static void dsos__init(struct dsos *dsos)
{
	INIT_LIST_HEAD(&dsos->head);
	dsos->root = RB_ROOT;
}

int machine__init(struct machine *machine, const char *root_dir, pid_t pid)
{
	map_groups__init(&machine->kmaps, machine);
	RB_CLEAR_NODE(&machine->rb_node);
	dsos__init(&machine->user_dsos);
	dsos__init(&machine->kernel_dsos);

	machine->threads = RB_ROOT;
	INIT_LIST_HEAD(&machine->dead_threads);
	machine->last_match = NULL;

	machine->vdso_info = NULL;

	machine->pid = pid;

	machine->symbol_filter = NULL;
	machine->id_hdr_size = 0;
	machine->comm_exec = false;
	machine->kernel_start = 0;

	machine->root_dir = strdup(root_dir);
	if (machine->root_dir == NULL)
		return -ENOMEM;

	if (pid != HOST_KERNEL_ID) {
		struct thread *thread = machine__findnew_thread(machine, -1,
								pid);
		char comm[64];

		if (thread == NULL)
			return -ENOMEM;

		snprintf(comm, sizeof(comm), "[guest/%d]", pid);
		thread__set_comm(thread, comm, 0);
	}

	machine->current_tid = NULL;

	return 0;
}

struct machine *machine__new_host(void)
{
	struct machine *machine = malloc(sizeof(*machine));

	if (machine != NULL) {
		machine__init(machine, "", HOST_KERNEL_ID);

		if (machine__create_kernel_maps(machine) < 0)
			goto out_delete;
	}

	return machine;
out_delete:
	free(machine);
	return NULL;
}

static void dsos__delete(struct dsos *dsos)
{
	struct dso *pos, *n;

	list_for_each_entry_safe(pos, n, &dsos->head, node) {
		RB_CLEAR_NODE(&pos->rb_node);
		list_del(&pos->node);
		dso__delete(pos);
	}
}

void machine__delete_threads(struct machine *machine)
{
	struct rb_node *nd = rb_first(&machine->threads);

	while (nd) {
		struct thread *t = rb_entry(nd, struct thread, rb_node);

		nd = rb_next(nd);
		machine__remove_thread(machine, t);
	}
}

void machine__exit(struct machine *machine)
{
	map_groups__exit(&machine->kmaps);
	dsos__delete(&machine->user_dsos);
	dsos__delete(&machine->kernel_dsos);
	vdso__exit(machine);
	zfree(&machine->root_dir);
	zfree(&machine->current_tid);
}

void machine__delete(struct machine *machine)
{
	machine__exit(machine);
	free(machine);
}

void machines__init(struct machines *machines)
{
	machine__init(&machines->host, "", HOST_KERNEL_ID);
	machines->guests = RB_ROOT;
	machines->symbol_filter = NULL;
}

void machines__exit(struct machines *machines)
{
	machine__exit(&machines->host);
	/* XXX exit guest */
}

struct machine *machines__add(struct machines *machines, pid_t pid,
			      const char *root_dir)
{
	struct rb_node **p = &machines->guests.rb_node;
	struct rb_node *parent = NULL;
	struct machine *pos, *machine = malloc(sizeof(*machine));

	if (machine == NULL)
		return NULL;

	if (machine__init(machine, root_dir, pid) != 0) {
		free(machine);
		return NULL;
	}

	machine->symbol_filter = machines->symbol_filter;

	while (*p != NULL) {
		parent = *p;
		pos = rb_entry(parent, struct machine, rb_node);
		if (pid < pos->pid)
			p = &(*p)->rb_left;
		else
			p = &(*p)->rb_right;
	}

	rb_link_node(&machine->rb_node, parent, p);
	rb_insert_color(&machine->rb_node, &machines->guests);

	return machine;
}

void machines__set_symbol_filter(struct machines *machines,
				 symbol_filter_t symbol_filter)
{
	struct rb_node *nd;

	machines->symbol_filter = symbol_filter;
	machines->host.symbol_filter = symbol_filter;

	for (nd = rb_first(&machines->guests); nd; nd = rb_next(nd)) {
		struct machine *machine = rb_entry(nd, struct machine, rb_node);

		machine->symbol_filter = symbol_filter;
	}
}

void machines__set_comm_exec(struct machines *machines, bool comm_exec)
{
	struct rb_node *nd;

	machines->host.comm_exec = comm_exec;

	for (nd = rb_first(&machines->guests); nd; nd = rb_next(nd)) {
		struct machine *machine = rb_entry(nd, struct machine, rb_node);

		machine->comm_exec = comm_exec;
	}
}

struct machine *machines__find(struct machines *machines, pid_t pid)
{
	struct rb_node **p = &machines->guests.rb_node;
	struct rb_node *parent = NULL;
	struct machine *machine;
	struct machine *default_machine = NULL;

	if (pid == HOST_KERNEL_ID)
		return &machines->host;

	while (*p != NULL) {
		parent = *p;
		machine = rb_entry(parent, struct machine, rb_node);
		if (pid < machine->pid)
			p = &(*p)->rb_left;
		else if (pid > machine->pid)
			p = &(*p)->rb_right;
		else
			return machine;
		if (!machine->pid)
			default_machine = machine;
	}

	return default_machine;
}

struct machine *machines__findnew(struct machines *machines, pid_t pid)
{
	char path[PATH_MAX];
	const char *root_dir = "";
	struct machine *machine = machines__find(machines, pid);

	if (machine && (machine->pid == pid))
		goto out;

	if ((pid != HOST_KERNEL_ID) &&
	    (pid != DEFAULT_GUEST_KERNEL_ID) &&
	    (symbol_conf.guestmount)) {
		sprintf(path, "%s/%d", symbol_conf.guestmount, pid);
		if (access(path, R_OK)) {
			static struct strlist *seen;

			if (!seen)
				seen = strlist__new(true, NULL);

			if (!strlist__has_entry(seen, path)) {
				pr_err("Can't access file %s\n", path);
				strlist__add(seen, path);
			}
			machine = NULL;
			goto out;
		}
		root_dir = path;
	}

	machine = machines__add(machines, pid, root_dir);
out:
	return machine;
}

void machines__process_guests(struct machines *machines,
			      machine__process_t process, void *data)
{
	struct rb_node *nd;

	for (nd = rb_first(&machines->guests); nd; nd = rb_next(nd)) {
		struct machine *pos = rb_entry(nd, struct machine, rb_node);
		process(pos, data);
	}
}

char *machine__mmap_name(struct machine *machine, char *bf, size_t size)
{
	if (machine__is_host(machine))
		snprintf(bf, size, "[%s]", "kernel.kallsyms");
	else if (machine__is_default_guest(machine))
		snprintf(bf, size, "[%s]", "guest.kernel.kallsyms");
	else {
		snprintf(bf, size, "[%s.%d]", "guest.kernel.kallsyms",
			 machine->pid);
	}

	return bf;
}

void machines__set_id_hdr_size(struct machines *machines, u16 id_hdr_size)
{
	struct rb_node *node;
	struct machine *machine;

	machines->host.id_hdr_size = id_hdr_size;

	for (node = rb_first(&machines->guests); node; node = rb_next(node)) {
		machine = rb_entry(node, struct machine, rb_node);
		machine->id_hdr_size = id_hdr_size;
	}

	return;
}

static void machine__update_thread_pid(struct machine *machine,
				       struct thread *th, pid_t pid)
{
	struct thread *leader;

	if (pid == th->pid_ || pid == -1 || th->pid_ != -1)
		return;

	th->pid_ = pid;

	if (th->pid_ == th->tid)
		return;

	leader = machine__findnew_thread(machine, th->pid_, th->pid_);
	if (!leader)
		goto out_err;

	if (!leader->mg)
		leader->mg = map_groups__new(machine);

	if (!leader->mg)
		goto out_err;

	if (th->mg == leader->mg)
		return;

	if (th->mg) {
		/*
		 * Maps are created from MMAP events which provide the pid and
		 * tid.  Consequently there never should be any maps on a thread
		 * with an unknown pid.  Just print an error if there are.
		 */
		if (!map_groups__empty(th->mg))
			pr_err("Discarding thread maps for %d:%d\n",
			       th->pid_, th->tid);
		map_groups__delete(th->mg);
	}

	th->mg = map_groups__get(leader->mg);

	return;

out_err:
	pr_err("Failed to join map groups for %d:%d\n", th->pid_, th->tid);
}

static struct thread *__machine__findnew_thread(struct machine *machine,
						pid_t pid, pid_t tid,
						bool create)
{
	struct rb_node **p = &machine->threads.rb_node;
	struct rb_node *parent = NULL;
	struct thread *th;

	/*
	 * Front-end cache - TID lookups come in blocks,
	 * so most of the time we dont have to look up
	 * the full rbtree:
	 */
	th = machine->last_match;
	if (th != NULL) {
		if (th->tid == tid) {
			machine__update_thread_pid(machine, th, pid);
			return th;
		}

		thread__zput(machine->last_match);
	}

	while (*p != NULL) {
		parent = *p;
		th = rb_entry(parent, struct thread, rb_node);

		if (th->tid == tid) {
			machine->last_match = thread__get(th);
			machine__update_thread_pid(machine, th, pid);
			return th;
		}

		if (tid < th->tid)
			p = &(*p)->rb_left;
		else
			p = &(*p)->rb_right;
	}

	if (!create)
		return NULL;

	th = thread__new(pid, tid);
	if (th != NULL) {
		rb_link_node(&th->rb_node, parent, p);
		rb_insert_color(&th->rb_node, &machine->threads);

		/*
		 * We have to initialize map_groups separately
		 * after rb tree is updated.
		 *
		 * The reason is that we call machine__findnew_thread
		 * within thread__init_map_groups to find the thread
		 * leader and that would screwed the rb tree.
		 */
		if (thread__init_map_groups(th, machine)) {
			rb_erase(&th->rb_node, &machine->threads);
			thread__delete(th);
			return NULL;
		}
		/*
		 * It is now in the rbtree, get a ref
		 */
		thread__get(th);
		machine->last_match = thread__get(th);
	}

	return th;
}

struct thread *machine__findnew_thread(struct machine *machine, pid_t pid,
				       pid_t tid)
{
	return __machine__findnew_thread(machine, pid, tid, true);
}

struct thread *machine__find_thread(struct machine *machine, pid_t pid,
				    pid_t tid)
{
	return __machine__findnew_thread(machine, pid, tid, false);
}

struct comm *machine__thread_exec_comm(struct machine *machine,
				       struct thread *thread)
{
	if (machine->comm_exec)
		return thread__exec_comm(thread);
	else
		return thread__comm(thread);
}

int machine__process_comm_event(struct machine *machine, union perf_event *event,
				struct perf_sample *sample)
{
	struct thread *thread = machine__findnew_thread(machine,
							event->comm.pid,
							event->comm.tid);
	bool exec = event->header.misc & PERF_RECORD_MISC_COMM_EXEC;

	if (exec)
		machine->comm_exec = true;

	if (dump_trace)
		perf_event__fprintf_comm(event, stdout);

	if (thread == NULL ||
	    __thread__set_comm(thread, event->comm.comm, sample->time, exec)) {
		dump_printf("problem processing PERF_RECORD_COMM, skipping event.\n");
		return -1;
	}

	return 0;
}

int machine__process_lost_event(struct machine *machine __maybe_unused,
				union perf_event *event, struct perf_sample *sample __maybe_unused)
{
	dump_printf(": id:%" PRIu64 ": lost:%" PRIu64 "\n",
		    event->lost.id, event->lost.lost);
	return 0;
}

static struct dso*
machine__module_dso(struct machine *machine, struct kmod_path *m,
		    const char *filename)
{
	struct dso *dso;

	dso = dsos__find(&machine->kernel_dsos, m->name, true);
	if (!dso) {
		dso = dsos__addnew(&machine->kernel_dsos, m->name);
		if (dso == NULL)
			return NULL;

		if (machine__is_host(machine))
			dso->symtab_type = DSO_BINARY_TYPE__SYSTEM_PATH_KMODULE;
		else
			dso->symtab_type = DSO_BINARY_TYPE__GUEST_KMODULE;

		/* _KMODULE_COMP should be next to _KMODULE */
		if (m->kmod && m->comp)
			dso->symtab_type++;

		dso__set_short_name(dso, strdup(m->name), true);
		dso__set_long_name(dso, strdup(filename), true);
	}

	return dso;
}

struct map *machine__new_module(struct machine *machine, u64 start,
				const char *filename)
{
	struct map *map = NULL;
	struct dso *dso;
	struct kmod_path m;

	if (kmod_path__parse_name(&m, filename))
		return NULL;

	map = map_groups__find_by_name(&machine->kmaps, MAP__FUNCTION,
				       m.name);
	if (map)
		goto out;

	dso = machine__module_dso(machine, &m, filename);
	if (dso == NULL)
		goto out;

	map = map__new2(start, dso, MAP__FUNCTION);
	if (map == NULL)
		goto out;

	map_groups__insert(&machine->kmaps, map);

out:
	free(m.name);
	return map;
}

size_t machines__fprintf_dsos(struct machines *machines, FILE *fp)
{
	struct rb_node *nd;
	size_t ret = __dsos__fprintf(&machines->host.kernel_dsos.head, fp) +
		     __dsos__fprintf(&machines->host.user_dsos.head, fp);

	for (nd = rb_first(&machines->guests); nd; nd = rb_next(nd)) {
		struct machine *pos = rb_entry(nd, struct machine, rb_node);
		ret += __dsos__fprintf(&pos->kernel_dsos.head, fp);
		ret += __dsos__fprintf(&pos->user_dsos.head, fp);
	}

	return ret;
}

size_t machine__fprintf_dsos_buildid(struct machine *m, FILE *fp,
				     bool (skip)(struct dso *dso, int parm), int parm)
{
	return __dsos__fprintf_buildid(&m->kernel_dsos.head, fp, skip, parm) +
	       __dsos__fprintf_buildid(&m->user_dsos.head, fp, skip, parm);
}

size_t machines__fprintf_dsos_buildid(struct machines *machines, FILE *fp,
				     bool (skip)(struct dso *dso, int parm), int parm)
{
	struct rb_node *nd;
	size_t ret = machine__fprintf_dsos_buildid(&machines->host, fp, skip, parm);

	for (nd = rb_first(&machines->guests); nd; nd = rb_next(nd)) {
		struct machine *pos = rb_entry(nd, struct machine, rb_node);
		ret += machine__fprintf_dsos_buildid(pos, fp, skip, parm);
	}
	return ret;
}

size_t machine__fprintf_vmlinux_path(struct machine *machine, FILE *fp)
{
	int i;
	size_t printed = 0;
	struct dso *kdso = machine->vmlinux_maps[MAP__FUNCTION]->dso;

	if (kdso->has_build_id) {
		char filename[PATH_MAX];
		if (dso__build_id_filename(kdso, filename, sizeof(filename)))
			printed += fprintf(fp, "[0] %s\n", filename);
	}

	for (i = 0; i < vmlinux_path__nr_entries; ++i)
		printed += fprintf(fp, "[%d] %s\n",
				   i + kdso->has_build_id, vmlinux_path[i]);

	return printed;
}

size_t machine__fprintf(struct machine *machine, FILE *fp)
{
	size_t ret = 0;
	struct rb_node *nd;

	for (nd = rb_first(&machine->threads); nd; nd = rb_next(nd)) {
		struct thread *pos = rb_entry(nd, struct thread, rb_node);

		ret += thread__fprintf(pos, fp);
	}

	return ret;
}

static struct dso *machine__get_kernel(struct machine *machine)
{
	const char *vmlinux_name = NULL;
	struct dso *kernel;

	if (machine__is_host(machine)) {
		vmlinux_name = symbol_conf.vmlinux_name;
		if (!vmlinux_name)
			vmlinux_name = "[kernel.kallsyms]";

		kernel = dso__kernel_findnew(machine, vmlinux_name,
					     "[kernel]",
					     DSO_TYPE_KERNEL);
	} else {
		char bf[PATH_MAX];

		if (machine__is_default_guest(machine))
			vmlinux_name = symbol_conf.default_guest_vmlinux_name;
		if (!vmlinux_name)
			vmlinux_name = machine__mmap_name(machine, bf,
							  sizeof(bf));

		kernel = dso__kernel_findnew(machine, vmlinux_name,
					     "[guest.kernel]",
					     DSO_TYPE_GUEST_KERNEL);
	}

	if (kernel != NULL && (!kernel->has_build_id))
		dso__read_running_kernel_build_id(kernel, machine);

	return kernel;
}

struct process_args {
	u64 start;
};

static void machine__get_kallsyms_filename(struct machine *machine, char *buf,
					   size_t bufsz)
{
	if (machine__is_default_guest(machine))
		scnprintf(buf, bufsz, "%s", symbol_conf.default_guest_kallsyms);
	else
		scnprintf(buf, bufsz, "%s/proc/kallsyms", machine->root_dir);
}

const char *ref_reloc_sym_names[] = {"_text", "_stext", NULL};

/* Figure out the start address of kernel map from /proc/kallsyms.
 * Returns the name of the start symbol in *symbol_name. Pass in NULL as
 * symbol_name if it's not that important.
 */
static u64 machine__get_running_kernel_start(struct machine *machine,
					     const char **symbol_name)
{
	char filename[PATH_MAX];
	int i;
	const char *name;
	u64 addr = 0;

	machine__get_kallsyms_filename(machine, filename, PATH_MAX);

	if (symbol__restricted_filename(filename, "/proc/kallsyms"))
		return 0;

	for (i = 0; (name = ref_reloc_sym_names[i]) != NULL; i++) {
		addr = kallsyms__get_function_start(filename, name);
		if (addr)
			break;
	}

	if (symbol_name)
		*symbol_name = name;

	return addr;
}

int __machine__create_kernel_maps(struct machine *machine, struct dso *kernel)
{
	enum map_type type;
	u64 start = machine__get_running_kernel_start(machine, NULL);

	for (type = 0; type < MAP__NR_TYPES; ++type) {
		struct kmap *kmap;

		machine->vmlinux_maps[type] = map__new2(start, kernel, type);
		if (machine->vmlinux_maps[type] == NULL)
			return -1;

		machine->vmlinux_maps[type]->map_ip =
			machine->vmlinux_maps[type]->unmap_ip =
				identity__map_ip;
		kmap = map__kmap(machine->vmlinux_maps[type]);
		if (!kmap)
			return -1;

		kmap->kmaps = &machine->kmaps;
		map_groups__insert(&machine->kmaps,
				   machine->vmlinux_maps[type]);
	}

	return 0;
}

void machine__destroy_kernel_maps(struct machine *machine)
{
	enum map_type type;

	for (type = 0; type < MAP__NR_TYPES; ++type) {
		struct kmap *kmap;

		if (machine->vmlinux_maps[type] == NULL)
			continue;

		kmap = map__kmap(machine->vmlinux_maps[type]);
		map_groups__remove(&machine->kmaps,
				   machine->vmlinux_maps[type]);
		if (kmap && kmap->ref_reloc_sym) {
			/*
			 * ref_reloc_sym is shared among all maps, so free just
			 * on one of them.
			 */
			if (type == MAP__FUNCTION) {
				zfree((char **)&kmap->ref_reloc_sym->name);
				zfree(&kmap->ref_reloc_sym);
			} else
				kmap->ref_reloc_sym = NULL;
		}

		map__delete(machine->vmlinux_maps[type]);
		machine->vmlinux_maps[type] = NULL;
	}
}

int machines__create_guest_kernel_maps(struct machines *machines)
{
	int ret = 0;
	struct dirent **namelist = NULL;
	int i, items = 0;
	char path[PATH_MAX];
	pid_t pid;
	char *endp;

	if (symbol_conf.default_guest_vmlinux_name ||
	    symbol_conf.default_guest_modules ||
	    symbol_conf.default_guest_kallsyms) {
		machines__create_kernel_maps(machines, DEFAULT_GUEST_KERNEL_ID);
	}

	if (symbol_conf.guestmount) {
		items = scandir(symbol_conf.guestmount, &namelist, NULL, NULL);
		if (items <= 0)
			return -ENOENT;
		for (i = 0; i < items; i++) {
			if (!isdigit(namelist[i]->d_name[0])) {
				/* Filter out . and .. */
				continue;
			}
			pid = (pid_t)strtol(namelist[i]->d_name, &endp, 10);
			if ((*endp != '\0') ||
			    (endp == namelist[i]->d_name) ||
			    (errno == ERANGE)) {
				pr_debug("invalid directory (%s). Skipping.\n",
					 namelist[i]->d_name);
				continue;
			}
			sprintf(path, "%s/%s/proc/kallsyms",
				symbol_conf.guestmount,
				namelist[i]->d_name);
			ret = access(path, R_OK);
			if (ret) {
				pr_debug("Can't access file %s\n", path);
				goto failure;
			}
			machines__create_kernel_maps(machines, pid);
		}
failure:
		free(namelist);
	}

	return ret;
}

void machines__destroy_kernel_maps(struct machines *machines)
{
	struct rb_node *next = rb_first(&machines->guests);

	machine__destroy_kernel_maps(&machines->host);

	while (next) {
		struct machine *pos = rb_entry(next, struct machine, rb_node);

		next = rb_next(&pos->rb_node);
		rb_erase(&pos->rb_node, &machines->guests);
		machine__delete(pos);
	}
}

int machines__create_kernel_maps(struct machines *machines, pid_t pid)
{
	struct machine *machine = machines__findnew(machines, pid);

	if (machine == NULL)
		return -1;

	return machine__create_kernel_maps(machine);
}

int machine__load_kallsyms(struct machine *machine, const char *filename,
			   enum map_type type, symbol_filter_t filter)
{
	struct map *map = machine->vmlinux_maps[type];
	int ret = dso__load_kallsyms(map->dso, filename, map, filter);

	if (ret > 0) {
		dso__set_loaded(map->dso, type);
		/*
		 * Since /proc/kallsyms will have multiple sessions for the
		 * kernel, with modules between them, fixup the end of all
		 * sections.
		 */
		__map_groups__fixup_end(&machine->kmaps, type);
	}

	return ret;
}

int machine__load_vmlinux_path(struct machine *machine, enum map_type type,
			       symbol_filter_t filter)
{
	struct map *map = machine->vmlinux_maps[type];
	int ret = dso__load_vmlinux_path(map->dso, map, filter);

	if (ret > 0)
		dso__set_loaded(map->dso, type);

	return ret;
}

static void map_groups__fixup_end(struct map_groups *mg)
{
	int i;
	for (i = 0; i < MAP__NR_TYPES; ++i)
		__map_groups__fixup_end(mg, i);
}

static char *get_kernel_version(const char *root_dir)
{
	char version[PATH_MAX];
	FILE *file;
	char *name, *tmp;
	const char *prefix = "Linux version ";

	sprintf(version, "%s/proc/version", root_dir);
	file = fopen(version, "r");
	if (!file)
		return NULL;

	version[0] = '\0';
	tmp = fgets(version, sizeof(version), file);
	fclose(file);

	name = strstr(version, prefix);
	if (!name)
		return NULL;
	name += strlen(prefix);
	tmp = strchr(name, ' ');
	if (tmp)
		*tmp = '\0';

	return strdup(name);
}

static bool is_kmod_dso(struct dso *dso)
{
	return dso->symtab_type == DSO_BINARY_TYPE__SYSTEM_PATH_KMODULE ||
	       dso->symtab_type == DSO_BINARY_TYPE__GUEST_KMODULE;
}

static int map_groups__set_module_path(struct map_groups *mg, const char *path,
				       struct kmod_path *m)
{
	struct map *map;
	char *long_name;

	map = map_groups__find_by_name(mg, MAP__FUNCTION, m->name);
	if (map == NULL)
		return 0;

	long_name = strdup(path);
	if (long_name == NULL)
		return -ENOMEM;

	dso__set_long_name(map->dso, long_name, true);
	dso__kernel_module_get_build_id(map->dso, "");

	/*
	 * Full name could reveal us kmod compression, so
	 * we need to update the symtab_type if needed.
	 */
	if (m->comp && is_kmod_dso(map->dso))
		map->dso->symtab_type++;

	return 0;
}

static int map_groups__set_modules_path_dir(struct map_groups *mg,
				const char *dir_name, int depth)
{
	struct dirent *dent;
	DIR *dir = opendir(dir_name);
	int ret = 0;

	if (!dir) {
		pr_debug("%s: cannot open %s dir\n", __func__, dir_name);
		return -1;
	}

	while ((dent = readdir(dir)) != NULL) {
		char path[PATH_MAX];
		struct stat st;

		/*sshfs might return bad dent->d_type, so we have to stat*/
		snprintf(path, sizeof(path), "%s/%s", dir_name, dent->d_name);
		if (stat(path, &st))
			continue;

		if (S_ISDIR(st.st_mode)) {
			if (!strcmp(dent->d_name, ".") ||
			    !strcmp(dent->d_name, ".."))
				continue;

			/* Do not follow top-level source and build symlinks */
			if (depth == 0) {
				if (!strcmp(dent->d_name, "source") ||
				    !strcmp(dent->d_name, "build"))
					continue;
			}

			ret = map_groups__set_modules_path_dir(mg, path,
							       depth + 1);
			if (ret < 0)
				goto out;
		} else {
			struct kmod_path m;

			ret = kmod_path__parse_name(&m, dent->d_name);
			if (ret)
				goto out;

			if (m.kmod)
				ret = map_groups__set_module_path(mg, path, &m);

			free(m.name);

			if (ret)
				goto out;
		}
	}

out:
	closedir(dir);
	return ret;
}

static int machine__set_modules_path(struct machine *machine)
{
	char *version;
	char modules_path[PATH_MAX];

	version = get_kernel_version(machine->root_dir);
	if (!version)
		return -1;

	snprintf(modules_path, sizeof(modules_path), "%s/lib/modules/%s",
		 machine->root_dir, version);
	free(version);

	return map_groups__set_modules_path_dir(&machine->kmaps, modules_path, 0);
}

static int machine__create_module(void *arg, const char *name, u64 start)
{
	struct machine *machine = arg;
	struct map *map;

	map = machine__new_module(machine, start, name);
	if (map == NULL)
		return -1;

	dso__kernel_module_get_build_id(map->dso, machine->root_dir);

	return 0;
}

static int machine__create_modules(struct machine *machine)
{
	const char *modules;
	char path[PATH_MAX];

	if (machine__is_default_guest(machine)) {
		modules = symbol_conf.default_guest_modules;
	} else {
		snprintf(path, PATH_MAX, "%s/proc/modules", machine->root_dir);
		modules = path;
	}

	if (symbol__restricted_filename(modules, "/proc/modules"))
		return -1;

	if (modules__parse(modules, machine, machine__create_module))
		return -1;

	if (!machine__set_modules_path(machine))
		return 0;

	pr_debug("Problems setting modules path maps, continuing anyway...\n");

	return 0;
}

int machine__create_kernel_maps(struct machine *machine)
{
	struct dso *kernel = machine__get_kernel(machine);
	const char *name;
	u64 addr = machine__get_running_kernel_start(machine, &name);
	if (!addr)
		return -1;

	if (kernel == NULL ||
	    __machine__create_kernel_maps(machine, kernel) < 0)
		return -1;

	if (symbol_conf.use_modules && machine__create_modules(machine) < 0) {
		if (machine__is_host(machine))
			pr_debug("Problems creating module maps, "
				 "continuing anyway...\n");
		else
			pr_debug("Problems creating module maps for guest %d, "
				 "continuing anyway...\n", machine->pid);
	}

	/*
	 * Now that we have all the maps created, just set the ->end of them:
	 */
	map_groups__fixup_end(&machine->kmaps);

	if (maps__set_kallsyms_ref_reloc_sym(machine->vmlinux_maps, name,
					     addr)) {
		machine__destroy_kernel_maps(machine);
		return -1;
	}

	return 0;
}

static void machine__set_kernel_mmap_len(struct machine *machine,
					 union perf_event *event)
{
	int i;

	for (i = 0; i < MAP__NR_TYPES; i++) {
		machine->vmlinux_maps[i]->start = event->mmap.start;
		machine->vmlinux_maps[i]->end   = (event->mmap.start +
						   event->mmap.len);
		/*
		 * Be a bit paranoid here, some perf.data file came with
		 * a zero sized synthesized MMAP event for the kernel.
		 */
		if (machine->vmlinux_maps[i]->end == 0)
			machine->vmlinux_maps[i]->end = ~0ULL;
	}
}

static bool machine__uses_kcore(struct machine *machine)
{
	struct dso *dso;

	list_for_each_entry(dso, &machine->kernel_dsos.head, node) {
		if (dso__is_kcore(dso))
			return true;
	}

	return false;
}

static int machine__process_kernel_mmap_event(struct machine *machine,
					      union perf_event *event)
{
	struct map *map;
	char kmmap_prefix[PATH_MAX];
	enum dso_kernel_type kernel_type;
	bool is_kernel_mmap;

	/* If we have maps from kcore then we do not need or want any others */
	if (machine__uses_kcore(machine))
		return 0;

	machine__mmap_name(machine, kmmap_prefix, sizeof(kmmap_prefix));
	if (machine__is_host(machine))
		kernel_type = DSO_TYPE_KERNEL;
	else
		kernel_type = DSO_TYPE_GUEST_KERNEL;

	is_kernel_mmap = memcmp(event->mmap.filename,
				kmmap_prefix,
				strlen(kmmap_prefix) - 1) == 0;
	if (event->mmap.filename[0] == '/' ||
	    (!is_kernel_mmap && event->mmap.filename[0] == '[')) {
		map = machine__new_module(machine, event->mmap.start,
					  event->mmap.filename);
		if (map == NULL)
			goto out_problem;

		map->end = map->start + event->mmap.len;
	} else if (is_kernel_mmap) {
		const char *symbol_name = (event->mmap.filename +
				strlen(kmmap_prefix));
		/*
		 * Should be there already, from the build-id table in
		 * the header.
		 */
		struct dso *kernel = NULL;
		struct dso *dso;

		list_for_each_entry(dso, &machine->kernel_dsos.head, node) {
			if (is_kernel_module(dso->long_name))
				continue;

			kernel = dso;
			break;
		}

		if (kernel == NULL)
			kernel = __dsos__findnew(&machine->kernel_dsos,
						 kmmap_prefix);
		if (kernel == NULL)
			goto out_problem;

		kernel->kernel = kernel_type;
		if (__machine__create_kernel_maps(machine, kernel) < 0)
			goto out_problem;

		if (strstr(kernel->long_name, "vmlinux"))
			dso__set_short_name(kernel, "[kernel.vmlinux]", false);

		machine__set_kernel_mmap_len(machine, event);

		/*
		 * Avoid using a zero address (kptr_restrict) for the ref reloc
		 * symbol. Effectively having zero here means that at record
		 * time /proc/sys/kernel/kptr_restrict was non zero.
		 */
		if (event->mmap.pgoff != 0) {
			maps__set_kallsyms_ref_reloc_sym(machine->vmlinux_maps,
							 symbol_name,
							 event->mmap.pgoff);
		}

		if (machine__is_default_guest(machine)) {
			/*
			 * preload dso of guest kernel and modules
			 */
			dso__load(kernel, machine->vmlinux_maps[MAP__FUNCTION],
				  NULL);
		}
	}
	return 0;
out_problem:
	return -1;
}

int machine__process_mmap2_event(struct machine *machine,
				 union perf_event *event,
				 struct perf_sample *sample __maybe_unused)
{
	u8 cpumode = event->header.misc & PERF_RECORD_MISC_CPUMODE_MASK;
	struct thread *thread;
	struct map *map;
	enum map_type type;
	int ret = 0;

	if (dump_trace)
		perf_event__fprintf_mmap2(event, stdout);

	if (cpumode == PERF_RECORD_MISC_GUEST_KERNEL ||
	    cpumode == PERF_RECORD_MISC_KERNEL) {
		ret = machine__process_kernel_mmap_event(machine, event);
		if (ret < 0)
			goto out_problem;
		return 0;
	}

	thread = machine__findnew_thread(machine, event->mmap2.pid,
					event->mmap2.tid);
	if (thread == NULL)
		goto out_problem;

	if (event->header.misc & PERF_RECORD_MISC_MMAP_DATA)
		type = MAP__VARIABLE;
	else
		type = MAP__FUNCTION;

	map = map__new(machine, event->mmap2.start,
			event->mmap2.len, event->mmap2.pgoff,
			event->mmap2.pid, event->mmap2.maj,
			event->mmap2.min, event->mmap2.ino,
			event->mmap2.ino_generation,
			event->mmap2.prot,
			event->mmap2.flags,
			event->mmap2.filename, type, thread);

	if (map == NULL)
		goto out_problem;

	thread__insert_map(thread, map);
	return 0;

out_problem:
	dump_printf("problem processing PERF_RECORD_MMAP2, skipping event.\n");
	return 0;
}

int machine__process_mmap_event(struct machine *machine, union perf_event *event,
				struct perf_sample *sample __maybe_unused)
{
	u8 cpumode = event->header.misc & PERF_RECORD_MISC_CPUMODE_MASK;
	struct thread *thread;
	struct map *map;
	enum map_type type;
	int ret = 0;

	if (dump_trace)
		perf_event__fprintf_mmap(event, stdout);

	if (cpumode == PERF_RECORD_MISC_GUEST_KERNEL ||
	    cpumode == PERF_RECORD_MISC_KERNEL) {
		ret = machine__process_kernel_mmap_event(machine, event);
		if (ret < 0)
			goto out_problem;
		return 0;
	}

	thread = machine__findnew_thread(machine, event->mmap.pid,
					 event->mmap.tid);
	if (thread == NULL)
		goto out_problem;

	if (event->header.misc & PERF_RECORD_MISC_MMAP_DATA)
		type = MAP__VARIABLE;
	else
		type = MAP__FUNCTION;

	map = map__new(machine, event->mmap.start,
			event->mmap.len, event->mmap.pgoff,
			event->mmap.pid, 0, 0, 0, 0, 0, 0,
			event->mmap.filename,
			type, thread);

	if (map == NULL)
		goto out_problem;

	thread__insert_map(thread, map);
	return 0;

out_problem:
	dump_printf("problem processing PERF_RECORD_MMAP, skipping event.\n");
	return 0;
}

void machine__remove_thread(struct machine *machine, struct thread *th)
{
	if (machine->last_match == th)
		thread__zput(machine->last_match);

	rb_erase(&th->rb_node, &machine->threads);
	/*
	 * Move it first to the dead_threads list, then drop the reference,
	 * if this is the last reference, then the thread__delete destructor
	 * will be called and we will remove it from the dead_threads list.
	 */
	list_add_tail(&th->node, &machine->dead_threads);
	thread__put(th);
}

int machine__process_fork_event(struct machine *machine, union perf_event *event,
				struct perf_sample *sample)
{
	struct thread *thread = machine__find_thread(machine,
						     event->fork.pid,
						     event->fork.tid);
	struct thread *parent = machine__findnew_thread(machine,
							event->fork.ppid,
							event->fork.ptid);

	/* if a thread currently exists for the thread id remove it */
	if (thread != NULL)
		machine__remove_thread(machine, thread);

	thread = machine__findnew_thread(machine, event->fork.pid,
					 event->fork.tid);
	if (dump_trace)
		perf_event__fprintf_task(event, stdout);

	if (thread == NULL || parent == NULL ||
	    thread__fork(thread, parent, sample->time) < 0) {
		dump_printf("problem processing PERF_RECORD_FORK, skipping event.\n");
		return -1;
	}

	return 0;
}

int machine__process_exit_event(struct machine *machine, union perf_event *event,
				struct perf_sample *sample __maybe_unused)
{
	struct thread *thread = machine__find_thread(machine,
						     event->fork.pid,
						     event->fork.tid);

	if (dump_trace)
		perf_event__fprintf_task(event, stdout);

	if (thread != NULL)
		thread__exited(thread);

	return 0;
}

int machine__process_event(struct machine *machine, union perf_event *event,
			   struct perf_sample *sample)
{
	int ret;

	switch (event->header.type) {
	case PERF_RECORD_COMM:
		ret = machine__process_comm_event(machine, event, sample); break;
	case PERF_RECORD_MMAP:
		ret = machine__process_mmap_event(machine, event, sample); break;
	case PERF_RECORD_MMAP2:
		ret = machine__process_mmap2_event(machine, event, sample); break;
	case PERF_RECORD_FORK:
		ret = machine__process_fork_event(machine, event, sample); break;
	case PERF_RECORD_EXIT:
		ret = machine__process_exit_event(machine, event, sample); break;
	case PERF_RECORD_LOST:
		ret = machine__process_lost_event(machine, event, sample); break;
	default:
		ret = -1;
		break;
	}

	return ret;
}

static bool symbol__match_regex(struct symbol *sym, regex_t *regex)
{
	if (sym->name && !regexec(regex, sym->name, 0, NULL, 0))
		return 1;
	return 0;
}

static void ip__resolve_ams(struct thread *thread,
			    struct addr_map_symbol *ams,
			    u64 ip)
{
	struct addr_location al;

	memset(&al, 0, sizeof(al));
	/*
	 * We cannot use the header.misc hint to determine whether a
	 * branch stack address is user, kernel, guest, hypervisor.
	 * Branches may straddle the kernel/user/hypervisor boundaries.
	 * Thus, we have to try consecutively until we find a match
	 * or else, the symbol is unknown
	 */
	thread__find_cpumode_addr_location(thread, MAP__FUNCTION, ip, &al);

	ams->addr = ip;
	ams->al_addr = al.addr;
	ams->sym = al.sym;
	ams->map = al.map;
}

static void ip__resolve_data(struct thread *thread,
			     u8 m, struct addr_map_symbol *ams, u64 addr)
{
	struct addr_location al;

	memset(&al, 0, sizeof(al));

	thread__find_addr_location(thread, m, MAP__VARIABLE, addr, &al);
	if (al.map == NULL) {
		/*
		 * some shared data regions have execute bit set which puts
		 * their mapping in the MAP__FUNCTION type array.
		 * Check there as a fallback option before dropping the sample.
		 */
		thread__find_addr_location(thread, m, MAP__FUNCTION, addr, &al);
	}

	ams->addr = addr;
	ams->al_addr = al.addr;
	ams->sym = al.sym;
	ams->map = al.map;
}

struct mem_info *sample__resolve_mem(struct perf_sample *sample,
				     struct addr_location *al)
{
	struct mem_info *mi = zalloc(sizeof(*mi));

	if (!mi)
		return NULL;

	ip__resolve_ams(al->thread, &mi->iaddr, sample->ip);
	ip__resolve_data(al->thread, al->cpumode, &mi->daddr, sample->addr);
	mi->data_src.val = sample->data_src;

	return mi;
}

static int add_callchain_ip(struct thread *thread,
			    struct symbol **parent,
			    struct addr_location *root_al,
			    u8 *cpumode,
			    u64 ip)
{
	struct addr_location al;

	al.filtered = 0;
	al.sym = NULL;
	if (!cpumode) {
		thread__find_cpumode_addr_location(thread, MAP__FUNCTION,
						   ip, &al);
	} else {
		if (ip >= PERF_CONTEXT_MAX) {
			switch (ip) {
			case PERF_CONTEXT_HV:
				*cpumode = PERF_RECORD_MISC_HYPERVISOR;
				break;
			case PERF_CONTEXT_KERNEL:
				*cpumode = PERF_RECORD_MISC_KERNEL;
				break;
			case PERF_CONTEXT_USER:
				*cpumode = PERF_RECORD_MISC_USER;
				break;
			default:
				pr_debug("invalid callchain context: "
					 "%"PRId64"\n", (s64) ip);
				/*
				 * It seems the callchain is corrupted.
				 * Discard all.
				 */
				callchain_cursor_reset(&callchain_cursor);
				return 1;
			}
			return 0;
		}
		thread__find_addr_location(thread, *cpumode, MAP__FUNCTION,
					   ip, &al);
	}

	if (al.sym != NULL) {
		if (sort__has_parent && !*parent &&
		    symbol__match_regex(al.sym, &parent_regex))
			*parent = al.sym;
		else if (have_ignore_callees && root_al &&
		  symbol__match_regex(al.sym, &ignore_callees_regex)) {
			/* Treat this symbol as the root,
			   forgetting its callees. */
			*root_al = al;
			callchain_cursor_reset(&callchain_cursor);
		}
	}

	return callchain_cursor_append(&callchain_cursor, al.addr, al.map, al.sym);
}

struct branch_info *sample__resolve_bstack(struct perf_sample *sample,
					   struct addr_location *al)
{
	unsigned int i;
	const struct branch_stack *bs = sample->branch_stack;
	struct branch_info *bi = calloc(bs->nr, sizeof(struct branch_info));

	if (!bi)
		return NULL;

	for (i = 0; i < bs->nr; i++) {
		ip__resolve_ams(al->thread, &bi[i].to, bs->entries[i].to);
		ip__resolve_ams(al->thread, &bi[i].from, bs->entries[i].from);
		bi[i].flags = bs->entries[i].flags;
	}
	return bi;
}

#define CHASHSZ 127
#define CHASHBITS 7
#define NO_ENTRY 0xff

#define PERF_MAX_BRANCH_DEPTH 127

/* Remove loops. */
static int remove_loops(struct branch_entry *l, int nr)
{
	int i, j, off;
	unsigned char chash[CHASHSZ];

	memset(chash, NO_ENTRY, sizeof(chash));

	BUG_ON(PERF_MAX_BRANCH_DEPTH > 255);

	for (i = 0; i < nr; i++) {
		int h = hash_64(l[i].from, CHASHBITS) % CHASHSZ;

		/* no collision handling for now */
		if (chash[h] == NO_ENTRY) {
			chash[h] = i;
		} else if (l[chash[h]].from == l[i].from) {
			bool is_loop = true;
			/* check if it is a real loop */
			off = 0;
			for (j = chash[h]; j < i && i + off < nr; j++, off++)
				if (l[j].from != l[i + off].from) {
					is_loop = false;
					break;
				}
			if (is_loop) {
				memmove(l + i, l + i + off,
					(nr - (i + off)) * sizeof(*l));
				nr -= off;
			}
		}
	}
	return nr;
}

/*
 * Recolve LBR callstack chain sample
 * Return:
 * 1 on success get LBR callchain information
 * 0 no available LBR callchain information, should try fp
 * negative error code on other errors.
 */
static int resolve_lbr_callchain_sample(struct thread *thread,
					struct perf_sample *sample,
					struct symbol **parent,
					struct addr_location *root_al,
					int max_stack)
{
	struct ip_callchain *chain = sample->callchain;
	int chain_nr = min(max_stack, (int)chain->nr);
	u8 cpumode = PERF_RECORD_MISC_USER;
	int i, j, err;
	u64 ip;

	for (i = 0; i < chain_nr; i++) {
		if (chain->ips[i] == PERF_CONTEXT_USER)
			break;
	}

	/* LBR only affects the user callchain */
	if (i != chain_nr) {
		struct branch_stack *lbr_stack = sample->branch_stack;
		int lbr_nr = lbr_stack->nr;
		/*
		 * LBR callstack can only get user call chain.
		 * The mix_chain_nr is kernel call chain
		 * number plus LBR user call chain number.
		 * i is kernel call chain number,
		 * 1 is PERF_CONTEXT_USER,
		 * lbr_nr + 1 is the user call chain number.
		 * For details, please refer to the comments
		 * in callchain__printf
		 */
		int mix_chain_nr = i + 1 + lbr_nr + 1;

		if (mix_chain_nr > PERF_MAX_STACK_DEPTH + PERF_MAX_BRANCH_DEPTH) {
			pr_warning("corrupted callchain. skipping...\n");
			return 0;
		}

		for (j = 0; j < mix_chain_nr; j++) {
			if (callchain_param.order == ORDER_CALLEE) {
				if (j < i + 1)
					ip = chain->ips[j];
				else if (j > i + 1)
					ip = lbr_stack->entries[j - i - 2].from;
				else
					ip = lbr_stack->entries[0].to;
			} else {
				if (j < lbr_nr)
					ip = lbr_stack->entries[lbr_nr - j - 1].from;
				else if (j > lbr_nr)
					ip = chain->ips[i + 1 - (j - lbr_nr)];
				else
					ip = lbr_stack->entries[0].to;
			}

			err = add_callchain_ip(thread, parent, root_al, &cpumode, ip);
			if (err)
				return (err < 0) ? err : 0;
		}
		return 1;
	}

	return 0;
}

static int thread__resolve_callchain_sample(struct thread *thread,
					    struct perf_evsel *evsel,
					    struct perf_sample *sample,
					    struct symbol **parent,
					    struct addr_location *root_al,
					    int max_stack)
{
	struct branch_stack *branch = sample->branch_stack;
	struct ip_callchain *chain = sample->callchain;
	int chain_nr = min(max_stack, (int)chain->nr);
	u8 cpumode = PERF_RECORD_MISC_USER;
	int i, j, err;
	int skip_idx = -1;
	int first_call = 0;

	callchain_cursor_reset(&callchain_cursor);

	if (has_branch_callstack(evsel)) {
		err = resolve_lbr_callchain_sample(thread, sample, parent,
						   root_al, max_stack);
		if (err)
			return (err < 0) ? err : 0;
	}

	/*
	 * Based on DWARF debug information, some architectures skip
	 * a callchain entry saved by the kernel.
	 */
	if (chain->nr < PERF_MAX_STACK_DEPTH)
		skip_idx = arch_skip_callchain_idx(thread, chain);

	/*
	 * Add branches to call stack for easier browsing. This gives
	 * more context for a sample than just the callers.
	 *
	 * This uses individual histograms of paths compared to the
	 * aggregated histograms the normal LBR mode uses.
	 *
	 * Limitations for now:
	 * - No extra filters
	 * - No annotations (should annotate somehow)
	 */

	if (branch && callchain_param.branch_callstack) {
		int nr = min(max_stack, (int)branch->nr);
		struct branch_entry be[nr];

		if (branch->nr > PERF_MAX_BRANCH_DEPTH) {
			pr_warning("corrupted branch chain. skipping...\n");
			goto check_calls;
		}

		for (i = 0; i < nr; i++) {
			if (callchain_param.order == ORDER_CALLEE) {
				be[i] = branch->entries[i];
				/*
				 * Check for overlap into the callchain.
				 * The return address is one off compared to
				 * the branch entry. To adjust for this
				 * assume the calling instruction is not longer
				 * than 8 bytes.
				 */
				if (i == skip_idx ||
				    chain->ips[first_call] >= PERF_CONTEXT_MAX)
					first_call++;
				else if (be[i].from < chain->ips[first_call] &&
				    be[i].from >= chain->ips[first_call] - 8)
					first_call++;
			} else
				be[i] = branch->entries[branch->nr - i - 1];
		}

		nr = remove_loops(be, nr);

		for (i = 0; i < nr; i++) {
			err = add_callchain_ip(thread, parent, root_al,
					       NULL, be[i].to);
			if (!err)
				err = add_callchain_ip(thread, parent, root_al,
						       NULL, be[i].from);
			if (err == -EINVAL)
				break;
			if (err)
				return err;
		}
		chain_nr -= nr;
	}

check_calls:
	if (chain->nr > PERF_MAX_STACK_DEPTH) {
		pr_warning("corrupted callchain. skipping...\n");
		return 0;
	}

	for (i = first_call; i < chain_nr; i++) {
		u64 ip;

		if (callchain_param.order == ORDER_CALLEE)
			j = i;
		else
			j = chain->nr - i - 1;

#ifdef HAVE_SKIP_CALLCHAIN_IDX
		if (j == skip_idx)
			continue;
#endif
		ip = chain->ips[j];

		err = add_callchain_ip(thread, parent, root_al, &cpumode, ip);

		if (err)
			return (err < 0) ? err : 0;
	}

	return 0;
}

static int unwind_entry(struct unwind_entry *entry, void *arg)
{
	struct callchain_cursor *cursor = arg;
	return callchain_cursor_append(cursor, entry->ip,
				       entry->map, entry->sym);
}

int thread__resolve_callchain(struct thread *thread,
			      struct perf_evsel *evsel,
			      struct perf_sample *sample,
			      struct symbol **parent,
			      struct addr_location *root_al,
			      int max_stack)
{
	int ret = thread__resolve_callchain_sample(thread, evsel,
						   sample, parent,
						   root_al, max_stack);
	if (ret)
		return ret;

	/* Can we do dwarf post unwind? */
	if (!((evsel->attr.sample_type & PERF_SAMPLE_REGS_USER) &&
	      (evsel->attr.sample_type & PERF_SAMPLE_STACK_USER)))
		return 0;

	/* Bail out if nothing was captured. */
	if ((!sample->user_regs.regs) ||
	    (!sample->user_stack.size))
		return 0;

	return unwind__get_entries(unwind_entry, &callchain_cursor,
				   thread, sample, max_stack);

}

int machine__for_each_thread(struct machine *machine,
			     int (*fn)(struct thread *thread, void *p),
			     void *priv)
{
	struct rb_node *nd;
	struct thread *thread;
	int rc = 0;

	for (nd = rb_first(&machine->threads); nd; nd = rb_next(nd)) {
		thread = rb_entry(nd, struct thread, rb_node);
		rc = fn(thread, priv);
		if (rc != 0)
			return rc;
	}

	list_for_each_entry(thread, &machine->dead_threads, node) {
		rc = fn(thread, priv);
		if (rc != 0)
			return rc;
	}
	return rc;
}

int __machine__synthesize_threads(struct machine *machine, struct perf_tool *tool,
				  struct target *target, struct thread_map *threads,
				  perf_event__handler_t process, bool data_mmap)
{
	if (target__has_task(target))
		return perf_event__synthesize_thread_map(tool, threads, process, machine, data_mmap);
	else if (target__has_cpu(target))
		return perf_event__synthesize_threads(tool, process, machine, data_mmap);
	/* command specified */
	return 0;
}

pid_t machine__get_current_tid(struct machine *machine, int cpu)
{
	if (cpu < 0 || cpu >= MAX_NR_CPUS || !machine->current_tid)
		return -1;

	return machine->current_tid[cpu];
}

int machine__set_current_tid(struct machine *machine, int cpu, pid_t pid,
			     pid_t tid)
{
	struct thread *thread;

	if (cpu < 0)
		return -EINVAL;

	if (!machine->current_tid) {
		int i;

		machine->current_tid = calloc(MAX_NR_CPUS, sizeof(pid_t));
		if (!machine->current_tid)
			return -ENOMEM;
		for (i = 0; i < MAX_NR_CPUS; i++)
			machine->current_tid[i] = -1;
	}

	if (cpu >= MAX_NR_CPUS) {
		pr_err("Requested CPU %d too large. ", cpu);
		pr_err("Consider raising MAX_NR_CPUS\n");
		return -EINVAL;
	}

	machine->current_tid[cpu] = tid;

	thread = machine__findnew_thread(machine, pid, tid);
	if (!thread)
		return -ENOMEM;

	thread->cpu = cpu;

	return 0;
}

int machine__get_kernel_start(struct machine *machine)
{
	struct map *map = machine__kernel_map(machine, MAP__FUNCTION);
	int err = 0;

	/*
	 * The only addresses above 2^63 are kernel addresses of a 64-bit
	 * kernel.  Note that addresses are unsigned so that on a 32-bit system
	 * all addresses including kernel addresses are less than 2^32.  In
	 * that case (32-bit system), if the kernel mapping is unknown, all
	 * addresses will be assumed to be in user space - see
	 * machine__kernel_ip().
	 */
	machine->kernel_start = 1ULL << 63;
	if (map) {
		err = map__load(map, machine->symbol_filter);
		if (map->start)
			machine->kernel_start = map->start;
	}
	return err;
}
