/*
* <:copyright-BRCM:2023:DUAL/GPL:standard
* 
*    Copyright (c) 2023 Broadcom 
*    All Rights Reserved
* 
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License, version 2, as published by
* the Free Software Foundation (the "GPL").
* 
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* 
* 
* A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
* writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
* Boston, MA 02111-1307, USA.
* 
:>
*/

/* needed before including tiny_kcore.h */
#define TINY_KCORE_ACTIVE

#define pr_fmt(_fmt)	"[%s] %s: " _fmt, KBUILD_MODNAME, __func__

#include <linux/kmsg_dump.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/mtd/mtd.h>
#include <linux/elfcore.h>
#include <linux/elf.h>
#include <linux/nmi.h>
#include <linux/jiffies.h>
#include <linux/tiny_kcore.h>
#include <asm/irq_regs.h>
#include <asm/kgdb.h>
/* For ARM the function crash_setup_regs() is conditional but we need it */
#define CONFIG_KEXEC
#include <asm/kexec.h>
#undef CONFIG_KEXEC

#if defined(CONFIG_BCM96XXX_WDT) || defined(CONFIG_BCM3390_WDT)
#include "board.h"
#endif

#define TKCORE_DUMPFILE			"tinykcore"
#define TKCORE_MTD_WRITE_BLOCKSIZE	(PAGE_SIZE << 4)
#define TKCORE_SEG_ALIGN_SIZE		4
#define TKCORE_SEGMENT_ALIGN(_sz)	ALIGN((_sz), TKCORE_SEG_ALIGN_SIZE)
#ifndef ELF_CORE_EFLAGS
#define ELF_CORE_EFLAGS			(0)
#endif

#ifdef DEBUG
#define TKCORE_WDT_RESTART_VALUE	60000000
#else
#define TKCORE_WDT_RESTART_VALUE	10000000
#endif

/**
 * struct tkcore_mem_entry - memory entry to be included in coredump file
 *
 * @list: linked list
 * @addr: address of the memory entry
 * @size: size of the memory entry
 * @offset: file offset where this segment will be placed
 * @iter: index of the alloc iterator by which the entry was added; -1 if not applicable
 * @need_bounce: if true, need a bounce buffer to pass this entry to user-space
 */
struct tkcore_mem_entry {
	struct list_head list;
	void *addr;
	size_t size;
	size_t offset;
	int iter;
	bool need_bounce;
};

/**
 * struct tkcore_file_info - ELF core file information
 *
 * @n_segs: number of memory segments to be stored in ELF core file
 * @segs_len: length of ELF segment headers
 * @notes_len: length of ELF notes segment
 * @offset: file offset at which the memory segments will be written
 */
struct tkcore_file_info {
	int n_segs;
	size_t segs_len;
	size_t notes_len;
	size_t offset;
};

/**
 * struct tkcore_proc_priv - procfs related private data
 *
 * @fi: @struct tkcore_file_info used for procfs
 * @bounce: bounce buffer used for memory entries which have &tkcore_mem_entry.need_bounce set
 */
struct tkcore_proc_priv {
	struct tkcore_file_info fi;
	char bounce[PAGE_SIZE];
};

/**
 * struct tkcore_dest_buf - destination buffer info
 *
 * @kern_buf: pointer to kernel buffer
 * @user_buf: pointer to user-space buffer
 * @len: length of the buffer
 * @off: offset in the ELF core file written
 *
 * This buffer info is used by the functions tkcore_copy_to() and tkcore_clear() to
 * handle constructing the ELF core file generically for procfs and panic handling.
 */
struct tkcore_dest_buf {
	union {
		void *kern_buf;
		char __user *user_buf;
	};
	size_t len;
	loff_t *off;
};

/**
 * struct tkcore_tsk_entry - task reference for &tkcore_context.tsk_list
 *
 * @list: linked list
 * @tsk: task struct reference
 * @flags: additional info for handling task entry
 *
 * For each task a NOTE section is added and the task structure and task stack
 * are included in the ELF core file.
 */
struct tkcore_tsk_entry {
	struct list_head list;
	struct task_struct *tsk;
	enum {
		TKCORE_TSK_INTERNAL	= 0x01,	/* entry for internal use */
		TKCORE_TSK_SKIP_NOTE	= 0x02,	/* skip note creation iterating task list */
	} flags:8;
};

/**
 * struct tkcore_alloc_iterator - allocation iterator info
 *
 * @ctx: context reference used in calling the iterator callback function
 * @iter_fn: iterator callback function to obtain allocations
 *
 * The iterator callback will be called repeatedly creating a &struct tkcore_mem_entry
 * until the callback return -%ENOENT.
 */
struct tkcore_alloc_iterator {
	void *ctx;
	int (*iter_fn)(void *ctx, struct tkcore_alloc_entry *entry);
};

/**
 * struct tkcore_cpu_state - per-cpu state to include in coredump
 *
 * @curr_tsk: active task for the CPU
 * @prstatus: NT_PRSTATUS content for @curr_tsk
 *
 * The per-cpu entry is filled in tkcore_save_cpu_state() and used
 * when creating the coredump.
 */
struct tkcore_cpu_state {
	struct task_struct *curr_tsk;
	struct elf_prstatus prstatus;
};

/**
 * struct tkcore_note_info - static information about certain ELF note
 *
 * @type: type of ELF note
 * @name: name to be used when adding this note in ELF core file
 * @size: size of the note data stored
 */
struct tkcore_note_info {
	int type;
	const char* const name;
	size_t size;
};

static const struct tkcore_note_info note_info[] = {
	{ NT_PRSTATUS, "CORE", sizeof(struct elf_prstatus) },
	{ NT_PRPSINFO, "CORE", sizeof(struct elf_prpsinfo) },
	{ NT_TASKSTRUCT, "CORE", arch_task_struct_size },
};
#define TKCORE_PRSTATUS_INFO		note_info[0]

/* need to get the irq stacks in the memory dump */
DECLARE_PER_CPU(unsigned long *, irq_stack_ptr);

static int tkcore_mtddev = -1;
module_param_named(mtddev, tkcore_mtddev, int, 0600);
MODULE_PARM_DESC(mtddev, "index of the MTD device to store coredump");

const char *target_banner;

/**
 * struct tkcore_context - global context
 *
 * @root: procfs entry
 * @mtd: MTD device used for persistant coredump storage
 * @regs: CPU registers of the running process
 * @kmsg: kmsg dumper handle
 * @panic_buf: buffer for writing blocks to MTD device during panic
 * @mem_list: list with &struct tkcore_mem_entry instances
 * @file_info: &struct tkcore_file_info used during panic
 * @tsk_list: list with &struct tkcore_tsk_entry instances
 * @alloc_iter: array holding limited amount of &struct tkcore_alloc_iterator instances
 * @prstatus: NT_PRSTATUS note information per cpu, ie. running task info
 */
static struct tkcore_context {
	struct proc_dir_entry *root;
	struct mtd_info *mtd;
	struct pt_regs regs;
	struct kmsg_dumper kmsg;
	void *panic_buf;
	struct list_head mem_list;
	struct tkcore_file_info file_info;
	struct list_head tsk_list;
	struct tkcore_alloc_iterator alloc_iter[4];
	struct tkcore_cpu_state __percpu *cpus;
} tkc;

/**
 * tkcore_copy_to() - generic copy function for coredump
 * @to_user: if true destination buffer is a user-space buffer
 * @dest: destination buffer info
 * @src: source buffer to copy from
 * @size: number of bytes to copy
 *
 * The function will use the appropriate copy function based on the @to_user parameter.
 *
 * Return: 0 if OK, otherwise it is non-zero.
 */
static unsigned long
tkcore_copy_to(bool to_user, struct tkcore_dest_buf *dest, char *src, size_t size)
{
	unsigned long ret = 0;

	pr_debug("copying 0x%zx@0x%px\n", size, src);
	if (to_user) {
		ret = copy_to_user(dest->user_buf, src, size);
		if (ret)
			return ret;
		dest->user_buf += size;
	} else {
		memcpy(dest->kern_buf, src, size);
		dest->kern_buf += size;
	}
	dest->len -= size;
	*dest->off += size;
	return ret;
}

/**
 * tkcore_clear() - generic clear function for padding in coredump
 * @to_user: if true destination buffer is a user-space buffer
 * @dest: destination buffer info
 * @size: number of bytes to clear
 *
 * The function will use the appropriate copy function based on the @to_user parameter.
 *
 * Return: 0 if OK, otherwise it is non-zero.
 */
static unsigned long
tkcore_clear(bool to_user, struct tkcore_dest_buf *dest, size_t size)
{
	unsigned long ret = 0;

	if (to_user) {
		ret = clear_user(dest->user_buf, size);
		if (ret)
			return ret;
		dest->user_buf += size;
	} else {
		memset(dest->kern_buf, 0, size);
		dest->kern_buf += size;
	}
	dest->len -= size;
	*dest->off += size;
	return ret;
}

/**
 * tkcore_addr_match() - compare two memory entries
 * @me: entry to compare
 * @tmp: dummy entry containing comparison value
 *
 * The parameter @me is is compared against @tmp which the caller has declared on the stack.
 * The address of the entries are compared. When they match but the size does not a warning
 * is issued.
 *
 * Return: true if addresses match, otherwise false.
 */
static bool tkcore_addr_match(struct tkcore_mem_entry *me, struct tkcore_mem_entry *tmp)
{
	if (me->addr == tmp->addr) {
		WARN_ON(me->size != tmp->size);
		return true;
	}
	return false;
}

/**
 * tkcore_valid_addr() - helper for tkcore_add_mem_entry() function
 * @me: entry to compare
 * @addr: address to match
 * @size: entry size
 *
 * When the @me parameter matches given address it is considered invalid, ie. duplicate.
 *
 * Return: false if address matches, otherwise true.
 */
static bool tkcore_valid_addr(struct tkcore_mem_entry *me, void *addr, size_t size)
{
	struct tkcore_mem_entry tmp = {
		.addr = addr,
		.size = size
	};

	return !tkcore_addr_match(me, &tmp);
}

/**
 * tkcore_remove_mem_entry() - find and remove a memory entry
 * @tmp: dummy entry containing comparison values
 * @entry_match_fn: callback function to use for comparison
 *
 * The entry is searched and if found removed from the list and
 * returned to the caller.
 *
 * Return: NULL if not found, otherwise the removed entry.
 */
static struct tkcore_mem_entry*
tkcore_remove_mem_entry(struct tkcore_mem_entry *tmp,
	bool (*entry_match_fn)(struct tkcore_mem_entry *me, struct tkcore_mem_entry *tmp))
{
	struct tkcore_mem_entry *prev, *next;
	if (WARN_ON(list_empty(&tkc.mem_list)))
		return NULL;

	prev = list_first_entry(&tkc.mem_list, typeof(*prev), list);
	next = list_last_entry(&tkc.mem_list, typeof(*next), list);

	/* if not specified simply match on the address */
	if (!entry_match_fn)
		entry_match_fn = tkcore_addr_match;

	/* handle if list has just one entry */
	if (prev == next && entry_match_fn(prev, tmp))
		goto found;

	while (prev != next) {
		if (entry_match_fn(prev, tmp))
			goto found;
		if (entry_match_fn(next, tmp)) {
			prev = next;
			goto found;
		}

		/* proceed with care to avoid collision */
		if (list_next_entry(prev, list) == next)
			break;
		prev = list_next_entry(prev, list);
		if (list_prev_entry(next, list) == prev)
			break;
		next = list_prev_entry(next, list);
	}
	pr_debug("no entry found: addr 0x%px size %zu iter %d match_fn %ps\n",
		 tmp->addr, tmp->size, tmp->iter, entry_match_fn);
	return NULL;

found:
	pr_debug("entry: addr 0x%px size %zu iter %d\n", prev->addr, prev->size, prev->iter);
	list_del(&prev->list);
	return prev;
}

/**
 * tkcore_add_mem_entry() - add a new memory entry into the list
 * @addr: address for the new entry
 * @size: size of the new entry
 * @iter: index number of allocation iterator which adds the entry
 * @bounce: if true a bounce buffer needs to be used when exposing to user-space
 *
 * The new memory entry is added to the memory list &tkc_context.mem_list, which is ordered
 * by the address. It also avoids adding duplicate entries.
 *
 * Return: 0 if successfully added, -%EFAULT if duplicate is found.
 */
static int
tkcore_add_mem_entry(void *addr, size_t size, int iter, bool bounce)
{
	struct tkcore_mem_entry *prev = NULL;
	struct tkcore_mem_entry *next = NULL;
	struct tkcore_mem_entry *new;

	if (list_empty(&tkc.mem_list))
		goto alloc_new;

	prev = list_first_entry(&tkc.mem_list, typeof(*new), list);
	next = list_last_entry(&tkc.mem_list, typeof(*new), list);

	/* deal with single-entry list */
	if (prev == next) {
		if (!tkcore_valid_addr(prev, addr, size))
			return -EFAULT;

		if (prev->addr > addr)
			prev = NULL;
		else
			next = NULL;
		goto alloc_new;

	}

	while (prev != next) {
		/* validate new entry */
		if (!tkcore_valid_addr(prev, addr, size) ||
		    !tkcore_valid_addr(next, addr, size)) {
			return -EFAULT;
		}
		if (prev->addr > addr) {
			next = prev;
			if (prev->list.prev == &tkc.mem_list)
				prev = NULL;
			else
				prev = list_prev_entry(next, list);
			break;
		}
		if (next->addr < addr) {
			prev = next;
			if (next->list.next == &tkc.mem_list)
				next = NULL;
			else
				next = list_next_entry(prev, list);
			break;
		}
		/* proceed with care to avoid collision */
		if (list_next_entry(prev, list) == next)
			break;
		prev = list_next_entry(prev, list);
		if (list_prev_entry(next, list) == prev)
			break;
		next = list_prev_entry(next, list);
	}
alloc_new:
	new = kzalloc(sizeof(*new), GFP_KERNEL);
	if (!new)
		return -ENOMEM;

	pr_debug("entry: addr 0x%px size %zu iter %d bounce %d\n", addr, size, iter, bounce);
	new->addr = addr;
	new->size = size;
	new->iter = iter;
	new->need_bounce = bounce;

	__list_add(&new->list, prev ? &prev->list : &tkc.mem_list,
		   next ? &next->list : &tkc.mem_list);
	return 0;
}

/**
 * tkcore_iter_index_match() - compare two memory entries
 * @me: entry to compare
 * @tmp: dummy entry containing comparison value
 *
 * The parameter @me is is compared against @tmp which the caller has declared on the stack.
 * The stored iter index of the memory entries is compared.
 *
 * Return: true if iterator index matches, otherwise false.
 */
static bool tkcore_iter_index_match(struct tkcore_mem_entry *me, struct tkcore_mem_entry *tmp)
{
	return me->iter == tmp->iter;
}

/**
 * tkcore_remove_alloc_iterator_entries() - remove entries for iterator
 * @iter_index: index of allocation iterator
 *
 * Removes and frees all memory entries for a given allocation iterator index.
 */
static void
tkcore_remove_alloc_iterator_entries(int iter_index)
{
	struct tkcore_mem_entry *me;
	struct tkcore_mem_entry rem;

	if (!tkc.alloc_iter[iter_index].ctx)
		return;

	rem.iter = iter_index;

	do {
		me = tkcore_remove_mem_entry(&rem, tkcore_iter_index_match);
		kfree(me);
	} while (me);
}

/**
 * tkcore_run_alloc_iterator() - query iterator and add entries obtained
 * @iter_index: index of allocation iterator
 *
 * Queries the callback function of given allocation iterator and adds a
 * memory entry for each returned allocation until the callback function
 * returns -%ENOENT.
 */
static void
tkcore_run_alloc_iterator(int iter_index)
{
	struct tkcore_alloc_iterator *iter = &tkc.alloc_iter[iter_index];
	struct tkcore_alloc_entry ae = {};
	int ret = 0;

	/* just empty slot so silently ignore */
	if (!iter->iter_fn && !iter->ctx)
		return;

	/* something quirky happened */
	if (WARN_ON(!iter->iter_fn))
		return;
	if (WARN_ON(!iter->ctx))
		return;

	while (ret == 0) {
		ret = iter->iter_fn(iter->ctx, &ae);
		if (ret == -ENOENT)
			return;

		tkcore_add_mem_entry(ae.addr, ae.size, iter_index, false);
	}
}

/**
 * tkcore_fill_note() - fill ELF note info
 * @p_note: position where ELF note must be written
 * @ni: static note information
 * @desc: data associated with the ELF note
 *
 * Fills the note information and data and returns the position for the next ELF note.
 *
 * Return: pointer to the location of the next ELF note.
 */
static void *
tkcore_fill_note(void *p_note, const struct tkcore_note_info *ni, const void *desc)
{
	struct elf_note *note = (struct elf_note *)p_note;

	note->n_namesz = strlen(ni->name) + 1;
	note->n_descsz = ni->size;
	note->n_type = ni->type;
	p_note += ALIGN(sizeof(*note), sizeof(Elf_Word));
	memcpy(p_note, ni->name, note->n_namesz);
	p_note += ALIGN(note->n_namesz, sizeof(Elf_Word));
	memcpy(p_note, desc, ni->size);
	p_note += ALIGN(ni->size, sizeof(Elf_Word));

	pr_debug("@%px: type %x name %s desc %zx => note_size %lx\n",
		 note, ni->type, ni->name, ni->size, (unsigned long)(p_note - (void *)note));
	return p_note;
}

/**
 * tkcore_copy_task_regs() - copy saved cpu registers for given task
 * @tsk: thread to obtain cpu registers from
 * @prstatus: ELF PRSTATUS note info
 *
 * Architecture dependent code to obtain the saved CPU registers for
 * a given task.
 */
static void
tkcore_copy_task_regs(struct task_struct *tsk, struct elf_prstatus *prstatus)
{
#if defined(CONFIG_ARM64)
		struct cpu_context *cpu_context = &tsk->thread.cpu_context;

		prstatus->pr_reg[19] = cpu_context->x19;
		prstatus->pr_reg[20] = cpu_context->x20;
		prstatus->pr_reg[21] = cpu_context->x21;
		prstatus->pr_reg[22] = cpu_context->x22;
		prstatus->pr_reg[23] = cpu_context->x23;
		prstatus->pr_reg[24] = cpu_context->x24;
		prstatus->pr_reg[25] = cpu_context->x25;
		prstatus->pr_reg[26] = cpu_context->x26;
		prstatus->pr_reg[27] = cpu_context->x27;
		prstatus->pr_reg[28] = cpu_context->x28;
		prstatus->pr_reg[29] = cpu_context->fp;

		prstatus->pr_reg[31] = cpu_context->sp;
		prstatus->pr_reg[32] = cpu_context->pc;
#elif defined(CONFIG_ARM)
		struct cpu_context_save	*cpu_context = &task_thread_info(tsk)->cpu_context;

		prstatus->pr_reg[_R4]	= cpu_context->r4;
		prstatus->pr_reg[_R5]	= cpu_context->r5;
		prstatus->pr_reg[_R6]	= cpu_context->r6;
		prstatus->pr_reg[_R7]	= cpu_context->r7;
		prstatus->pr_reg[_R8]	= cpu_context->r8;
		prstatus->pr_reg[_R9]	= cpu_context->r9;
		prstatus->pr_reg[_R10]	= cpu_context->sl;
		prstatus->pr_reg[_FP]	= cpu_context->fp;
		prstatus->pr_reg[_SPT]	= cpu_context->sp;
		prstatus->pr_reg[_PC]	= cpu_context->pc;
#endif
}

/**
 * __tkcore_add_task() - create internal bookkeeping for given task
 * @tsk: task to be tracked
 * @flags: flags for new task entry
 *
 * Creates an entry on the task list for the given task and sets
 * the flags as requested which determines how it is handled in
 * dump creation and cleanup.
 */
static void __tkcore_add_task(struct task_struct *tsk, u8 flags)
{
	struct tkcore_tsk_entry *te;

	if (!tsk->stack) {
		pr_err("task %s (%d) has not stack!!\n", tsk->comm, tsk->pid);
		return;
	}
	te = kzalloc(sizeof(*te), GFP_KERNEL);
	if (!te)
		return;

	pr_debug("adding task %s (%d)\n", tsk->comm, tsk->pid);
	te->tsk = tsk;
	te->flags = flags;
	tkcore_add_mem_entry(tsk, arch_task_struct_size, -1, true);
	tkcore_add_mem_entry(tsk->stack, THREAD_SIZE, -1, false);
	list_add_tail(&te->list, &tkc.tsk_list);
}

/**
 * tkcore_find_task_entry() - lookup given task in task list
 * @tsk: task to lookup
 *
 * Return: task entry if found; %NULL otherwise.
 */
static struct tkcore_tsk_entry *
tkcore_find_task_entry(struct task_struct *tsk)
{
	struct tkcore_tsk_entry *te;

	list_for_each_entry(te, &tkc.tsk_list, list) {
		if (te->tsk == tsk) {
			return te;
		}
	}
	return NULL;
}

/**
 * tkcore_get_file_info() - prepare ELF corefile creation
 * @fi: file info to fill
 *
 * Fill file info depending on &tkcore_context.mem_list and
 * &tkcore_context.tsk_list.
 *
 * Return: size of the ELF corefile
 */
static size_t
tkcore_get_file_info(struct tkcore_file_info *fi)
{
	struct tkcore_file_info tmp = { 0 };
	struct tkcore_cpu_state *cpu_state;
	struct tkcore_mem_entry *me;
	struct tkcore_tsk_entry *te;
	size_t data_size = 0;
	size_t totlen;
	int i, cpu;

	/* add tasks for cpu state */
	for_each_possible_cpu(cpu) {
		cpu_state = per_cpu_ptr(tkc.cpus, cpu);
		if (cpu_state->prstatus.pr_pid == -1)
			continue;
		if (WARN_ON(!cpu_state->curr_tsk))
			continue;
		te = tkcore_find_task_entry(cpu_state->curr_tsk);
		if (te) {
			te->flags |= TKCORE_TSK_SKIP_NOTE;
		} else {
			__tkcore_add_task(cpu_state->curr_tsk,
					  TKCORE_TSK_INTERNAL | TKCORE_TSK_SKIP_NOTE);
		}
	}

	/* count memory blocks */
	list_for_each_entry(me, &tkc.mem_list, list) {
		me->offset = data_size;
		data_size += TKCORE_SEGMENT_ALIGN(me->size);
		tmp.n_segs++;
	}
	/* one more for the note segment */
	tmp.n_segs++;
	tmp.segs_len = tmp.n_segs * sizeof(struct elf_phdr);

	/* skip first entry, ie. NT_PRSTATUS as this is handled by processing the task list below */
	for (i = 1; i < ARRAY_SIZE(note_info); i++) {
		tmp.notes_len += ALIGN(sizeof(struct elf_note), sizeof(Elf_Word));
		tmp.notes_len += ALIGN(strlen(note_info[i].name) + 1, sizeof(Elf_Word));
		tmp.notes_len += ALIGN(note_info[i].size, sizeof(Elf_Word));
	}
	i = 0;
	pr_info("=== TKCORE TASK LIST ===\n");
	list_for_each_entry(te, &tkc.tsk_list, list) {
		pr_info("[%-2d] task %s pid %d state %c\n", i++, te->tsk->comm, te->tsk->pid,
			task_state_to_char(te->tsk));
		tmp.notes_len += ALIGN(sizeof(struct elf_note), sizeof(Elf_Word));
		tmp.notes_len += ALIGN(strlen(TKCORE_PRSTATUS_INFO.name) + 1, sizeof(Elf_Word));
		tmp.notes_len += ALIGN(TKCORE_PRSTATUS_INFO.size, sizeof(Elf_Word));
	}
	tmp.offset = PAGE_ALIGN(sizeof(struct elfhdr) + tmp.segs_len + tmp.notes_len);

	if (tkc.mtd)
		tkc.file_info = tmp;
	if (fi)
		*fi = tmp;

	totlen = tmp.offset + PAGE_ALIGN(data_size);
	pr_debug("size=%zu (%zx) n_segs=%d notes_len=%zu data=0x%zx@0x%zx\n",
		 totlen, totlen, tmp.n_segs, tmp.notes_len, data_size, tmp.offset);
	return tmp.offset + PAGE_ALIGN(data_size);
}

/**
 * tkcore_free_task_entry() - free the memory associated with given task
 * @te: task entry to free
 *
 * Free everything associated with given task entry. The task entry itself
 * should already have been removed from the task list.
 */
void
tkcore_free_task_entry(struct tkcore_tsk_entry *te)
{
	struct tkcore_mem_entry *me, rem;
	struct task_struct *tsk = te->tsk;

	pr_debug("removing task %s\n", tsk->comm);
	rem.addr = tsk;
	rem.size = arch_task_struct_size;
	me = tkcore_remove_mem_entry(&rem, NULL);
	kfree(me);

	rem.addr = tsk->stack;
	rem.size = THREAD_SIZE;
	me = tkcore_remove_mem_entry(&rem, NULL);
	kfree(me);

	kfree(te);
}

/**
 * tkcore_cleanup_tasks() - cleanup tasks and flags in task list
 *
 * Removes all tasks marked as internal and decrements refcount for
 * task entries with the %SKIP_NOTE set.
 */
void
tkcore_cleanup_tasks(void)
{
	struct tkcore_tsk_entry *te, *tmp;

	list_for_each_entry_safe(te, tmp, &tkc.tsk_list, list) {
		if (te->flags & TKCORE_TSK_SKIP_NOTE) {
			te->flags &= ~TKCORE_TSK_SKIP_NOTE;
			put_task_struct(te->tsk);
		}
		if (te->flags & TKCORE_TSK_INTERNAL) {
			list_del(&te->list);
			tkcore_free_task_entry(te);
		}
	}
}

/**
 * tkcore_save_cpu_state() - save CPU registers to be stored in coredump
 * @regs: register set of the given cpu
 * @cpu: given cpu id
 *
 * Store the task and register info. The usage count of the task is incremented
 * to avoid the very small chance that it is freed before the coredump is being
 * created. Chances are bit higher when creating dump on running system through
 * sysfs.
 *
 * note:
 *	called from handle_IPI(ipinr=IPI_CPU_STOP) for secondary CPUs (smp.c)
 *	called from die() for crashing CPU (traps.c)
 *	called from panic() for secondary CPU if it also panics
 */
void
tkcore_save_cpu_state(struct pt_regs *regs, uint cpu)
{
	struct tkcore_cpu_state *cpu_state;
	struct task_struct *tsk = current;
	struct pt_regs cpu_regs;

	/* idle cpu has nothing interesting to show for */
	if (idle_cpu(cpu)) {
		pr_debug("CPU%d: idle\n", cpu);
		return;
	}

	cpu_state = per_cpu_ptr(tkc.cpus, cpu);
	if (!cpu_state)
		return;

	/* already saved so ignore now */
	if (cpu_state->prstatus.pr_pid != -1) {
		WARN_ON(cpu_state->prstatus.pr_pid != tsk->pid);
		return;
	}

	/* store the task reference and increment usage count */
	pr_debug("CPU%d: task %s (%d)\n", cpu, tsk->comm, tsk->pid);
	get_task_struct(tsk);
	cpu_state->prstatus.pr_pid = tsk->pid;
	cpu_state->curr_tsk = tsk;
	if (!regs) {
		crash_setup_regs(&cpu_regs, NULL);
		regs = &cpu_regs;
	}
	elf_core_copy_regs(&cpu_state->prstatus.pr_reg, regs);
}

/**
 * tkcore_clear_cpu_state() - reinitializes the cpu state
 * @cpu: given cpu id
 */
void
tkcore_clear_cpu_state(int cpu)
{
	struct tkcore_cpu_state *cpu_state;

	cpu_state = per_cpu_ptr(tkc.cpus, cpu);
	if (!cpu_state)
		return;

	memset(cpu_state, 0, sizeof(*cpu_state));
	cpu_state->prstatus.pr_pid = -1;
	cpu_state->curr_tsk = NULL;
}

/**
 * tkcore_cpu_state_filled() - indicate if state of given cpu is stored
 * @cpu: given cpu id
 */
bool tkcore_cpu_state_filled(int cpu)
{
	struct tkcore_cpu_state *cpu_state;

	cpu_state = per_cpu_ptr(tkc.cpus, cpu);
	if (!cpu_state)
		return false;

	return (cpu_state->prstatus.pr_pid != -1);
}

/**
 * __tkcore_read() - fill destination buffer with portion of ELF corefile
 * @fh: procfs file handle which is only valid when handling procfs access
 * @fi: file information of the ELF corefile
 * @dest: destination buffer to fill
 *
 * Allows reading the ELF corefile in blocks so recursive calls return the next block.
 *
 * Return: > 0 is number of bytes filled, 0 when ELF corefile is complete, < 0 on error.
 */
static ssize_t
__tkcore_read(struct file *fh, struct tkcore_file_info *fi, struct tkcore_dest_buf *dest)
{
	char *bounce;
	bool to_user = fh != NULL;
	struct tkcore_mem_entry *me;
	size_t segs_offset, notes_offset, copy_size;
	size_t orig_len = dest->len;
	ssize_t ret = 0;

	segs_offset = sizeof(struct elfhdr);
	notes_offset = segs_offset + fi->segs_len;

	/* file header */
	if (dest->len && *dest->off < sizeof(struct elfhdr)) {
		struct elfhdr eh = {
			.e_ident = {
				[EI_MAG0] = ELFMAG0,
				[EI_MAG1] = ELFMAG1,
				[EI_MAG2] = ELFMAG2,
				[EI_MAG3] = ELFMAG3,
				[EI_CLASS] = ELF_CLASS,
				[EI_DATA] = ELF_DATA,
				[EI_VERSION] = EV_CURRENT,
				[EI_OSABI] = ELF_OSABI,
			},
			.e_type = ET_CORE,
			.e_machine = ELF_ARCH,
			.e_version = EV_CURRENT,
			.e_phoff = sizeof(struct elfhdr),
			.e_flags = ELF_CORE_EFLAGS,
			.e_ehsize = sizeof(struct elfhdr),
			.e_phentsize = sizeof(struct elf_phdr),
			.e_phnum = fi->n_segs
		};
		copy_size = min_t(size_t, dest->len, sizeof(struct elfhdr) - *dest->off);
		if (tkcore_copy_to(to_user, dest, (char *)&eh + *dest->off, copy_size)) {
			ret = -EFAULT;
			goto out;
		}
	}

	/* segment headers */
	if (dest->len && *dest->off < notes_offset) {
		struct tkcore_mem_entry *me;
		struct elf_phdr *segs, *seg;

		segs = kzalloc(fi->segs_len, GFP_KERNEL);
		if (!segs) {
			ret = -ENOMEM;
			goto out;
		}

		seg = &segs[0];
		seg->p_type = PT_NOTE;
		seg->p_offset = notes_offset;
		seg->p_filesz = fi->notes_len;

		list_for_each_entry(me, &tkc.mem_list, list) {
			seg++;
			seg->p_type = PT_LOAD;
			seg->p_flags = PF_R | PF_W | PF_X;
			seg->p_offset = fi->offset + me->offset;
			seg->p_vaddr = (size_t)me->addr;
			seg->p_filesz = seg->p_memsz = me->size;
			seg->p_align = TKCORE_SEG_ALIGN_SIZE;
		}

		copy_size = min_t(size_t, dest->len, notes_offset - *dest->off);
		if (tkcore_copy_to(to_user, dest,
				   (char *)segs + *dest->off - segs_offset, copy_size)) {
			kfree(segs);
			ret = -EFAULT;
			goto out;
		}
		kfree(segs);
	}

	/* notes */
	if (dest->len && *dest->off < notes_offset + fi->notes_len) {
		struct tkcore_cpu_state *cpu_state;
		struct tkcore_tsk_entry *te;
		struct elf_prpsinfo prpsinfo = {
			.pr_sname = 'R',
			.pr_fname = "vmlinux",
		};
		char *notes, *p_note;
		int cpu;

		strlcpy(prpsinfo.pr_psargs, saved_command_line, sizeof(prpsinfo.pr_psargs));

		notes = kzalloc(fi->notes_len, GFP_KERNEL);
		if (!notes) {
			ret = -ENOMEM;
			goto out;
		}

		p_note = notes;

		for_each_cpu_wrap(cpu, cpu_possible_mask, smp_processor_id()) {
			cpu_state = per_cpu_ptr(tkc.cpus, cpu);
			if (!cpu_state || cpu_state->prstatus.pr_pid == -1)
				continue;

			p_note = tkcore_fill_note(p_note, &note_info[0], &cpu_state->prstatus);
		}

		p_note = tkcore_fill_note(p_note, &note_info[1], &prpsinfo);
		p_note = tkcore_fill_note(p_note, &note_info[2], current);

		list_for_each_entry(te, &tkc.tsk_list, list) {
			struct elf_prstatus tskstatus;

			if (te->flags & TKCORE_TSK_SKIP_NOTE) {
				pr_debug("skip running task %s\n", te->tsk->comm);
				continue;
			}
			memset(&tskstatus, 0, sizeof(tskstatus));
			tskstatus.pr_pid = te->tsk->pid;
			tkcore_copy_task_regs(te->tsk, &tskstatus);
			p_note = tkcore_fill_note(p_note, &TKCORE_PRSTATUS_INFO, &tskstatus);
		}

		copy_size = min_t(size_t, dest->len, notes_offset + fi->notes_len - *dest->off);
		if (tkcore_copy_to(to_user, dest, notes + *dest->off - notes_offset, copy_size)) {
			kfree(notes);
			ret = -EFAULT;
			goto out;
		}
		kfree(notes);
	}

	list_for_each_entry(me, &tkc.mem_list, list) {
		size_t seg_offset = fi->offset + me->offset;

		/* fill page alignment space */
		if (dest->len && *dest->off < seg_offset) {
			copy_size = min_t(size_t, dest->len, seg_offset - *dest->off);
			if (tkcore_clear(to_user, dest, copy_size)) {
				ret = -EFAULT;
				goto out;
			}
		}
		/* now copy actual data */
		if (dest->len && *dest->off < seg_offset + me->size) {
			copy_size = min_t(size_t, dest->len, seg_offset + me->size - *dest->off);
			if (to_user && me->need_bounce) {
				struct tkcore_proc_priv *pd = fh->private_data;
				bounce = pd->bounce;
				/* limit copy size to bounce buffer size */
				copy_size = min_t(size_t, copy_size, PAGE_SIZE);
				if (probe_kernel_read(bounce,
						      (void *)(me->addr + *dest->off - seg_offset),
						      copy_size)) {
					if (tkcore_clear(to_user, dest, copy_size)) {
						ret = -EFAULT;
						goto out;
					}
				} else {
					if (tkcore_copy_to(to_user, dest, bounce, copy_size)) {
						ret = -EFAULT;
						goto out;
					}
				}
			} else {
				if (tkcore_copy_to(to_user, dest,
						    (void *)(me->addr + *dest->off - seg_offset),
						    copy_size)) {
					ret = -EFAULT;
					goto out;
				}
			}
		}
	}
	ret = orig_len - dest->len;
out:
	return ret;
}

/**
 * tkcore_watchdog_ping() - keep-alive ping for watchdog
 *
 * assure the watchdog does not reboot the platform before
 * we can complete the coredump creation and have it securely
 * stored on the MTD partition.
 */
static void
tkcore_watchdog_ping(void)
{
#if defined(CONFIG_BCM96XXX_WDT) || defined(CONFIG_BCM3390_WDT)
	pr_debug("restart watchdog\n");
	bcmbca_wd_start(TKCORE_WDT_RESTART_VALUE);
#endif
}

/**
 * tkcore_do_dump() - obtain and store ELF corefile on MTD partition
 * @dumper: kmsg dumper structure
 * @reason: reason for calling this function
 *
 * Reads the ELF corefile in portions and writes it to the MTD partition.
 */
static void tkcore_do_dump(struct kmsg_dumper *dumper, enum kmsg_dump_reason reason)
{
	loff_t curpos, newpos;
	struct tkcore_dest_buf dest = {
		.kern_buf = tkc.panic_buf,
		.len = TKCORE_MTD_WRITE_BLOCKSIZE,
		.off = &newpos
	};
	struct tkcore_file_info fi = {};
	struct pt_regs regs;
	ssize_t ret = 1;
	struct mtd_info *mtd;
	size_t mtd_retlen, len;
	size_t bytes_written = 0;
	int mtd_ret, i, cpu = smp_processor_id();

	if (reason != KMSG_DUMP_PANIC)
		return;

	pr_debug("panic in task %s stack 0x%px\n", current->comm, current->stack);

	if (!tkc.mtd)
		return;

	tkcore_watchdog_ping();

	for (i = 0; i < ARRAY_SIZE(tkc.alloc_iter); i++) {
		tkcore_run_alloc_iterator(i);
	}

	/* only current CPU running when we end up here */
	if (!tkcore_cpu_state_filled(cpu)) {
		crash_setup_regs(&regs, NULL);
		tkcore_save_cpu_state(&regs, cpu);
	}

	len = tkcore_get_file_info(&fi);
	mtd = tkc.mtd;

	if (len > mtd->size) {
		pr_err("coredump size %zu exceeds mtd%d size %llu\n", len, mtd->index, mtd->size);
		return;
	}

	pr_info("creating coredump: size=%zu (0x%zx)\n", len, len);
	newpos = 0;
	while (ret > 0) {
		tkcore_watchdog_ping();

		/* reset buffer info */
		dest.kern_buf = tkc.panic_buf;
		dest.len = TKCORE_MTD_WRITE_BLOCKSIZE;
		curpos = newpos;

		/* read chunk of data and update newpos */
		ret = __tkcore_read(NULL, &fi, &dest);
		if (ret <= 0)
			goto out;

		len = PAGE_ALIGN(newpos - curpos);
		pr_debug("write to mtd: 0x%zx bytes @ 0x%llx\n", len, curpos);
		mtd_ret = mtd_panic_write(tkc.mtd, curpos, len, &mtd_retlen, tkc.panic_buf);
		if (mtd_ret == -EOPNOTSUPP) {
			pr_err("mtd_panic_write() not supported by mtd%d\n", mtd->index);
			goto out;
		}
		if (mtd_retlen != len || mtd_ret < 0) {
			pr_err("write failure (ret=%d) @ 0x%llx (0x%zx of 0x%zx written)\n",
			       mtd_ret, curpos, mtd_retlen, len);
		} else {
			bytes_written += mtd_retlen;
		}
	}
out:
	pr_info("coredump complete: size 0x%zx mtd%d (%s)\n", bytes_written,
		mtd->index, mtd->name);
}

/**
 * tkcore_mtd_add() - notifier called by MTD subsystem when MTD partition becomes available
 * @mtd: newly available MTD partition
 *
 * When MTD partition is to be used for coredump register tkcore_panic_callback(). It also
 * issues a warning when the MTD partition is not empty.
 */
static void tkcore_mtd_add(struct mtd_info *mtd)
{
	u32 signature;
	size_t mtd_retlen;
	int err;

	BUILD_BUG_ON(TKCORE_MTD_WRITE_BLOCKSIZE & 0xFFF);
	if (mtd->index != tkcore_mtddev)
		return;

	tkc.panic_buf = kzalloc(TKCORE_MTD_WRITE_BLOCKSIZE, GFP_KERNEL);
	if (!tkc.panic_buf)
		return;

	err = mtd_read(mtd, 0, sizeof(signature), &mtd_retlen, (u8 *)&signature);
	if (!err && mtd_retlen == sizeof(signature) && signature != 0xffffffff) {
		pr_warn(">>>> MTD NOT EMPTY (%sELF magic signature found) <<<<\n",
			memcmp(&signature,ELFMAG, sizeof(signature)) ? "no " : "");
	} else {
		pr_info(">>>>>>> MTD IS EMPTY <<<<<<<\n");
	}

	tkc.kmsg.max_reason = KMSG_DUMP_PANIC;
	tkc.kmsg.dump = tkcore_do_dump;
	err = kmsg_dump_register(&tkc.kmsg);
	if (err) {
		pr_warn("dumper registration failed: err=%d\n", err);
		kfree(tkc.panic_buf);
		return;
	}

	pr_info("using mtd%d (%s) for coredump storage\n", mtd->index, mtd->name);
	tkc.mtd = mtd;
}

/**
 * tkcore_mtd_remove() - notifier called by MTD subsystem when MTD partition is removed
 * @mtd: removed MTD partition
 *
 * When MTD partition is to be used for coredump unregister tkcore_panic_callback().
 */
static void tkcore_mtd_remove(struct mtd_info *mtd)
{
	int err;

	if (mtd->index != tkcore_mtddev)
		return;

	err = kmsg_dump_unregister(&tkc.kmsg);
	if (err)
		pr_warn("dumper unregister failed: err=%d\n", err);

	tkc.mtd = NULL;
	kfree(tkc.panic_buf);
}

static struct mtd_notifier tkcore_mtd_notifier = {
	.add	= tkcore_mtd_add,
	.remove	= tkcore_mtd_remove,
};

/**
 * tkcore_read() - read function for procfs access
 * @fh: procfs file handle
 * @user_buf: user-space buffer to fill
 * @len: available space in user-space buffer
 * @off: current file offset
 *
 * Reads the ELF corefile in portions and returns it to user-space.
 *
 * Return: > 0 is number of bytes filled, 0 when ELF corefile is complete, < 0 on error.
 */
static ssize_t
tkcore_read(struct file *fh, char __user *user_buf, size_t len, loff_t *off)
{
	struct tkcore_proc_priv *pd = fh->private_data;
	struct tkcore_dest_buf tmp = {
		.user_buf = user_buf,
		.len = len,
		.off = off
	};
	struct pt_regs regs;
	ssize_t ret;
	int cpu;

	if (*off == 0) {
		crash_setup_regs(&regs, NULL);
		tkcore_save_cpu_state(&regs, smp_processor_id());
		tkcore_get_file_info(&pd->fi);
	}
	ret = __tkcore_read(fh, &pd->fi, &tmp);
	if (ret <= 0) {
		tkcore_cleanup_tasks();
		for_each_possible_cpu(cpu) {
			tkcore_clear_cpu_state(cpu);
		}
	}

	*off = *tmp.off;
	return ret;
}

/**
 * tkcore_open() - called upon opening procfs file
 * @in: inode associated with procfs file
 * @fh: procfs file handle
 *
 * Adds memory entries for all added allocation iterators.
 *
 * Return: 0 on success, < 0 on error.
 */
static int
tkcore_open(struct inode *in, struct file *fh)
{
	struct tkcore_proc_priv *pd;
	int i;

	if (!capable(CAP_SYS_RAWIO))
		return -EPERM;

	pd = kzalloc(sizeof(*pd), GFP_KERNEL);
	if (!pd)
		return -ENOMEM;

	fh->private_data = pd;

	for (i = 0; i < ARRAY_SIZE(tkc.alloc_iter); i++) {
		tkcore_run_alloc_iterator(i);
	}
	return 0;
}

/**
 * tkcore_release() - called upon closing procfs file
 * @in: inode associated with procfs file
 * @fh: procfs file handle
 *
 * Removes memory entries for all added allocation iterators.
 *
 * Return: 0 on success, < 0 on error.
 */
static int
tkcore_release(struct inode *in, struct file *fh)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(tkc.alloc_iter); i++) {
		tkcore_remove_alloc_iterator_entries(i);
	}
	kfree(fh->private_data);
	return 0;
}

static const struct file_operations proc_tkcore_ops = {
	.read		= tkcore_read,
	.open		= tkcore_open,
	.release	= tkcore_release,
	.llseek		= default_llseek,
};

/**
 * proc_tkcore_init() - initialize function
 *
 * Creates the procfs file and registers MTD notifier when a MTD is to be used. Also
 * add some initial memory entries for the IRQ stacks (one per CPU).
 *
 * Return: 0
 */
static int __init proc_tkcore_init(void)
{
	int cpu;

	tkc.root = proc_create(TKCORE_DUMPFILE, S_IRUSR, NULL, &proc_tkcore_ops);
	if (!tkc.root) {
		pr_err("failed to create /proc/%s\n", TKCORE_DUMPFILE);
		if (tkcore_mtddev < 0)
			return -EIO;
	}

	tkc.cpus = __alloc_percpu(sizeof(struct tkcore_cpu_state), TKCORE_SEG_ALIGN_SIZE);
	if (!tkc.cpus) {
		proc_remove(tkc.root);
		return -ENOMEM;
	}

	/* initalize dump lists */
	INIT_LIST_HEAD(&tkc.mem_list);
	INIT_LIST_HEAD(&tkc.tsk_list);

	/* add banner */
	target_banner = kstrdup(linux_banner, GFP_KERNEL);
	tkcore_add_mem_entry(&target_banner, sizeof(void *), -1, false);
	tkcore_add_mem_entry(target_banner, strlen(target_banner) + 1, -1, false);

	/* add dumper and printk log buffer */
	tkcore_add_mem_entry(&tkc.kmsg, sizeof(tkc.kmsg), -1, false);
	tkcore_add_mem_entry(log_buf_addr_get(), log_buf_len_get(), -1, false);

	for_each_possible_cpu(cpu) {
#if defined(CONFIG_ARM64)
		/* add irq stack entry */
		tkcore_add_mem_entry(per_cpu(irq_stack_ptr, cpu), IRQ_STACK_SIZE, -1, false);
#endif
		/* clear cpu state */
		tkcore_clear_cpu_state(cpu);
	}

	if (tkcore_mtddev < 0) {
		pr_info("no MTD specified so only support coredump through procfs\n");
		return 0;
	}
	register_mtd_user(&tkcore_mtd_notifier);
	return 0;
}
fs_initcall(proc_tkcore_init);

/**
 * tkcore_add_module() - add memory entry for module sections
 * @mod: the module to add
 *
 * Adds core_layout for the given module which contains .data, .bss, and .text sections.
 */
void tkcore_add_module(struct module *mod)
{
	pr_debug("adding %s module\n", mod->name);
	tkcore_add_mem_entry(mod->core_layout.base, mod->core_layout.size, -1, false);
}
EXPORT_SYMBOL(tkcore_add_module);

/**
 * tkcore_remove_module() - remove memory entry for given module
 * @mod: the module to remove
 *
 * removes the core_layout memory entry for the given module.
 */
void tkcore_remove_module(struct module *mod)
{
	struct tkcore_mem_entry tmp;
	struct tkcore_mem_entry *me;

	pr_debug("removing %s module\n", mod->name);
	tmp.addr = mod->core_layout.base;
	tmp.size = mod->core_layout.size;
	me = tkcore_remove_mem_entry(&tmp, NULL);
	kfree(me);
}
EXPORT_SYMBOL(tkcore_remove_module);

/**
 * tkcore_add_task() - add memory entries for given task
 * @tsk: task to add
 *
 * Adds a memory entry for the @tsk struct itself and the task stack and add
 * an entry in &tkcore_context.tsk_list.
 */
void tkcore_add_task(struct task_struct *tsk)
{
	__tkcore_add_task(tsk, 0);
}
EXPORT_SYMBOL(tkcore_add_task);

/**
 * tkcore_remove_task() - remove memory entries for given task
 * @tsk: task to remove
 *
 * Remove the memory entries for @tsk and remove entry in &tkcore_context.tsk_list.
 */
void tkcore_remove_task(struct task_struct *tsk)
{
	struct tkcore_tsk_entry *te, *tmp;

	list_for_each_entry_safe(te, tmp, &tkc.tsk_list, list) {
		if (te->tsk == tsk) {
			list_del(&te->list);
			tkcore_free_task_entry(te);
			return;
		}
	}
	WARN_ON(1);
}
EXPORT_SYMBOL(tkcore_remove_task);

/**
 * tkcore_add_alloc_iterator() - add allocation iterator
 * @ctx: context to be used calling the iterator callback function
 * @iter_fn: iterator callback function
 *
 * Used to obtain memory allocation through the @iter_fn for which memory
 * entries will be added to &tkcore_context.mem_list.
 */
void tkcore_add_alloc_iterator(void *ctx,
	int (*iter_fn)(void *ctx, struct tkcore_alloc_entry *entry))
{
	int i;

	for (i = 0; i < ARRAY_SIZE(tkc.alloc_iter); i++) {
		if (tkc.alloc_iter[i].iter_fn == iter_fn && tkc.alloc_iter[i].ctx == ctx) {
			pr_warn("iterator <%px %ps> already added\n", ctx, iter_fn);
			return;
		}
		if (!tkc.alloc_iter[i].ctx) {
			pr_debug("added iterator <%px %ps>\n", ctx, iter_fn);
			tkc.alloc_iter[i].ctx = ctx;
			tkc.alloc_iter[i].iter_fn = iter_fn;
			return;
		}
	}
	pr_warn("all iterator slots filled\n");
}
EXPORT_SYMBOL(tkcore_add_alloc_iterator);

/**
 * tkcore_remove_alloc_iterator() - remove allocation iterator
 * @ctx: context of the to be removed allocation iterator
 *
 * Removes all memory entries associated with the allocation iterator
 * with the given @ctx.
 */
void tkcore_remove_alloc_iterator(void *ctx)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(tkc.alloc_iter); i++) {
		if (tkc.alloc_iter[i].ctx == ctx) {
			pr_debug("removing iterator <%px %ps>\n", ctx,
				tkc.alloc_iter[i].iter_fn);
			tkcore_remove_alloc_iterator_entries(i);
			tkc.alloc_iter[i].ctx = NULL;
			tkc.alloc_iter[i].iter_fn = NULL;
			return;
		}
	}
}
EXPORT_SYMBOL(tkcore_remove_alloc_iterator);