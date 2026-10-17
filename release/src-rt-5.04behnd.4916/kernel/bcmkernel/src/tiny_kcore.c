/*
* <:copyright-BRCM:2023:DUAL/GPL:standard
*
*    Copyright (c) 2023 Broadcom
*    All Rights Reserved
*
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed
* to you under the terms of the GNU General Public License version 2
* (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
* with the following added to such license:
*
*    As a special exception, the copyright holders of this software give
*    you permission to link this software with independent modules, and
*    to copy and distribute the resulting executable under terms of your
*    choice, provided that you also meet, for each linked independent
*    module, the terms and conditions of the license of that module.
*    An independent module is a module which is not derived from this
*    software.  The special exception does not apply to any modifications
*    of the software.
*
* Not withstanding the above, under no circumstances may you combine
* this software in any way with any other Broadcom software provided
* under a license other than the GPL, without Broadcom's express prior
* written consent.
*
:>
*/

/* needed before including tiny_kcore.h */
#define TINY_KCORE_ACTIVE
#define TKCORE_API_SELECT	(2)

#define pr_fmt(_fmt)	"[%s] %s: " _fmt, KBUILD_MODNAME, __func__

#include <linux/kmsg_dump.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/poison.h>
#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/mtd/mtd.h>
#include <linux/elfcore.h>
#include <linux/elf.h>
#include <linux/nmi.h>
#include <linux/lzo.h>
#include <linux/jiffies.h>
#include <linux/crash_core.h>
#include <linux/version.h>
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

/* ======================= kernel compatibility tricks ======================= */
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 8, 0)
#define COPY_KERNEL_BOUNCE	probe_kernel_read
#else
#define COPY_KERNEL_BOUNCE	copy_from_kernel_nofault
#endif
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 12, 0)
#define ELF_PRSTAT_PID(_elf_prstat)	((_elf_prstat)->pr_pid)
#else
#define ELF_PRSTAT_PID(_elf_prstat)	((_elf_prstat)->common.pr_pid)
#endif
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 6, 0)
#define PROC_ENTRY_OPS		struct file_operations
#define PROC_READ(_fn)		read = _fn
#define PROC_OPEN(_fn)		open = _fn
#define PROC_RELEASE(_fn)	release = _fn
#define PROC_LSEEK(_fn)		llseek = _fn
#else
#define PROC_ENTRY_OPS		struct proc_ops
#define PROC_READ(_fn)		proc_read = _fn
#define PROC_OPEN(_fn)		proc_open = _fn
#define PROC_RELEASE(_fn)	proc_release = _fn
#define PROC_LSEEK(_fn)		proc_lseek = _fn
#endif
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 14, 0)
#define TASK_STATE(_tsk)	((_tsk)->state)
#else
#define TASK_STATE(_tsk)	((_tsk)->__state)
#endif
/* =========================================================================== */

#define TKCORE_DUMPFILE			"tinykcore"
#define TKCORE_MEMPOOL_SIZE		(1024 * 1024)
#define TKCORE_MTD_MIN_SIZE		(8 * 1024 * 1024)
#define TKCORE_MTD_WRITE_BLOCKSIZE	(PAGE_SIZE * 16)
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

/* regdump mtd header byte offsets */
#define REGDUMP_MTDHDR_INST	(0)
#define REGDUMP_MTDHDR_TYPE	(1)
#define REGDUMP_MTDHDR_MORE	(2)
#define REGDUMP_MTDHDR_IDLEN	(3)
#define REGDUMP_MTDHDR_ID	(4)

#define TKCORE_INITIAL_SCRATCH_BUFSIZE \
	(1 << (20 + CONFIG_BCM_TINY_KCORE_SCRATCH_BUFSZ))

/**
 * struct tkcore_mem_entry - memory entry to be included in coredump file
 *
 * @list: linked list
 * @addr: address of the memory entry
 * @size: size of the memory entry
 * @offset: file offset where this segment will be placed
 * @iter: index of the alloc iterator by which the entry was added; -1 if not applicable
 * @cpuid: indicates memory content was saved in cpu_state; -1 if not applicable
 * @flags: used to indicate special handling.
 */
struct tkcore_mem_entry {
	struct list_head list;
	const void *addr;
	size_t size;
	size_t offset;
	int iter;
	int cpuid;
	enum {
		TKCORE_MEM_NEED_BOUNCE   = 0x01, /* use bounce buffer for user-space exposure */
		TKCORE_MEM_DISABLE_KASAN = 0x02, /* disable KASAN report when writing to dump */
	} flags:8;
};

/**
 * struct tkcore_file_info - ELF core file information
 *
 * @segs: ELF segment headers to be stored
 * @n_segs: number of memory segments to be stored in ELF core file
 * @segs_len: length of ELF segment headers
 * @notes: ELF note segment to be stored
 * @notes_len: length of ELF notes segment
 * @offset: file offset at which the memory segments will be written
 * @core_len: total length of the ELF core file
 */
struct tkcore_file_info {
	struct elf_phdr *segs;
	int n_segs;
	size_t segs_len;
	char *notes;
	size_t notes_len;
	size_t offset;
	u32 core_len;
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
 * @me: memory entry for the task stack
 * @flags: additional info for handling task entry
 *
 * For each task a NOTE section is added and the task structure and task stack
 * are included in the ELF core file.
 */
struct tkcore_tsk_entry {
	struct list_head list;
	struct task_struct *tsk;
	struct tkcore_mem_entry *me;
	enum {
		TKCORE_TSK_INTERNAL	= 0x01,	/* entry for internal use */
		TKCORE_TSK_SKIP_NOTE	= 0x02,	/* skip note creation iterating task list */
	} flags:8;
};

/**
 * struct tkcore_cpu_state - per-cpu state to include in coredump
 *
 * @curr_tsk: active task for the CPU
 * @prstatus: NT_PRSTATUS content for @curr_tsk
 * @me: memory entry holding the stack for @curr_tsk
 * @saved_stack: copy of the @curr_tsk stack made upon saving cpu state
 *
 * The per-cpu entry is filled in tkcore_save_cpu_state() and used
 * when creating the coredump.
 */
struct tkcore_cpu_state {
	struct task_struct *curr_tsk;
	struct elf_prstatus prstatus;
	struct tkcore_mem_entry *me;
	u8 *saved_stack;
};

/**
 * struct tkcore_regdump_offset - register dump offset on MTD
 *
 * @version: version for the register dump
 * @compression: type of compression used
 * @blocksize: size of uncompressed dump buffer in power-of-two MB
 *	0 = 1MB, 1 = 2MB, 2 = 4MB, etc.
 * @start_offset: offset on MTD where register dumps start
 * @elf_magic: %ELFMAG value to mark MTD holds coredump data
 */
struct tkcore_regdump_offset {
	u16 version;
	u8 compression;
	u8 blocksize;
	u32 start_offset;
	u32 elf_magic;
};

/**
 * struct tkcore_dev_regdump - callback to retrieve register dumps
 *
 * @list: list_head for register dump list
 * @ctx: context to be used in callback
 * @inst: instance id stored with register dump, ie. 0 for wl0
 * @dev_regdump_fn: callback function for register dump retrieval
 * @iter: current iter value for dump retrieval
 * @mtd_pos: offset on mtd where current register dump starts
 *
 * The entry stores callback to be called during panic to obtain
 * register dumps from the device. The callback will be called
 * until it returns -ENOENT.
 */
struct tkcore_dev_regdump {
	struct list_head list;
	void *ctx;
	u8 inst;
	int (*dev_regdump_fn)(void *ctx, uint iter, struct tkcore_regdump_info *rdi);
};

/**
 * struct tkcore_curr_regdump - active dump info used in tkcore_regdump_read()
 *
 * @rd_curr: current regdump callback instance being processed
 * @iter: iter value of current regdump
 * @mtd_pos: offset on MTD where current regdump starts
 * @lzo_size: size of current compressed regdump
 * @lzo_remain: remaining bytes of current compressed regdump to be stored
 * @more: indicates active dump is fragmented
 * @offset: offset where the next dump fragment starts
 * @mtd_hdr: header for the current regdump
 * @mtd_hdr_len: header length for current regdump depending on dump identifier
 */
struct tkcore_curr_regdump {
	struct tkcore_dev_regdump *rd_curr;
	int iter;
	loff_t mtd_pos;
	size_t lzo_size;
	size_t lzo_remain;
	u8 more;
	off_t offset;
	u8 mtd_hdr[27];
	u8 mtd_hdr_len;
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
module_param_named(mtddev, tkcore_mtddev, int, 0400);
MODULE_PARM_DESC(mtddev, "index of the MTD device to store coredump");
static char tkcore_mtdname[80] = "tinykcore";
module_param_string(mtdname, tkcore_mtdname, sizeof(tkcore_mtdname), 0400);
MODULE_PARM_DESC(mtdname, "name of the MTD device to store coredump");
static bool tkcore_minimal;
module_param_named(minimal, tkcore_minimal, bool, 0600);
MODULE_PARM_DESC(minimal, "capture only ELF coredump and dmesg");

const char *target_banner;

/**
 * struct tkcore_context - global context
 *
 * @root: procfs entry
 * @mtd: MTD device used for persistant coredump storage
 * @regs: CPU registers of the running process
 * @kmsg: kmsg dumper handle
 * @mimic_mtdoops: only write kernel log to MTD.
 * @panic: indicates that coredump is created in panic context
 * @panic_buf: buffer for writing blocks to MTD device during panic
 * @mem_list: list with &struct tkcore_mem_entry instances
 * @file_info: &struct tkcore_file_info used during panic
 * @tsk_list: list with &struct tkcore_tsk_entry instances
 * @alloc_iter: array holding limited amount of &struct tkcore_alloc_iter_info instances
 * @cpu_state_saved: bitmask indicating CPU state has been saved
 * @cpus: state of each cpu in the system
 * @regdump_list: list with &struct tkcore_dev_regdump instances
 * @rd_active: information about active regdump being written to MTD
 * @regdump_end_pos: offset pointing after the end marker
 * @regdump_bufsz: size of @regdump_buf
 * @regdump_buf: buffer used to collect register dumps
 * @regdump_lzo: buffer containing compressed @regdump_buf
 * @lzo_workmem: buffer used for in-memory LZO compression
 * @scratch_buf: buffer used for dump data and compressed result
 * @mempool: pool for allocation during panic
 * @mempool_alloced: allocated amount of bytes from @mempool
 */
static struct tkcore_context {
	struct proc_dir_entry *root;
	struct mtd_info *mtd;
	struct pt_regs regs;
	struct kmsg_dumper kmsg;
	bool mimic_mtdoops;
	bool panic;
	void *panic_buf;
	struct list_head mem_list;
	struct tkcore_file_info file_info;
	struct list_head tsk_list;
	struct tkcore_alloc_iter_info alloc_iter[8];
	ulong cpu_state_saved;
	struct tkcore_cpu_state __percpu *cpus;
	struct list_head regdump_list;
	struct tkcore_curr_regdump rd_active;
	loff_t regdump_end_pos;
	size_t regdump_bufsz;
	char *regdump_buf;
	char *regdump_lzo;
	char *lzo_workmem;
	char *scratch_buf;
	char *mempool;
	size_t mempool_alloced;
} tkc;

/*
 * mutex to avoid concurrent access to the procfs file.
 */
static DEFINE_MUTEX(tkcore_proc_lock);

/*
 * mutex to avoid concurrent invocation of smp_call_function().
 */
static DEFINE_MUTEX(tkcore_smp_call_lock);

/**
 * tkcore_alloc_zero() - allocate zeroed memory with requested size
 * @size: requested size
 *
 * When handling procfs access it does a regular kzalloc() but
 * in panic context it will use the scratch buffer for allocations.
 * This buffer is already zero allocated upon hooking up the MTD
 * and we simply hand out pieces of it until it is depleted.
 */
static void *
tkcore_alloc_zero(size_t size)
{
	size_t mempool_free = 0;
	void *ptr;

	if (!tkc.panic)
		return kzalloc(size, GFP_KERNEL);

	if (tkc.mempool_alloced >= TKCORE_MEMPOOL_SIZE) {
		goto fail;
	}
	mempool_free = TKCORE_MEMPOOL_SIZE - tkc.mempool_alloced;
	if (size > mempool_free) {
		goto fail;
	}
	ptr = tkc.mempool + tkc.mempool_alloced;
	tkc.mempool_alloced += ALIGN(size, 4);
	pr_debug("allocated %zu bytes (total=%u free=%zu)\n", size,
		TKCORE_MEMPOOL_SIZE, TKCORE_MEMPOOL_SIZE - tkc.mempool_alloced);
	return ptr;

fail:
	pr_err("pool alloc failed: requested %zu free %zu\n", size, mempool_free);
 	return NULL;
}

/**
 * tkcore_copy_to() - generic copy function for coredump
 * @dest: destination buffer info
 * @src: source buffer to copy from
 * @size: number of bytes to copy
 *
 * The function will use the appropriate copy function based on
 * flag &struct tkcore_context.panic.
 *
 * Return: 0 if OK, otherwise it is non-zero.
 */
static long
tkcore_copy_to(struct tkcore_dest_buf *dest, char *src, size_t size)
{
	unsigned long ret = 0;

	pr_debug("copying 0x%zx@0x%px\n", size, src);
	if (!tkc.panic) {
		ret = copy_to_user_nofault(dest->user_buf, src, size);
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
 * @dest: destination buffer info
 * @size: number of bytes to clear
 * @val: fill bytes with this value
 *
 * The function will use the appropriate clear function based on the
 * flag &struct tkcore_context.panic.
 *
 * Return: 0 if OK, otherwise it is non-zero.
 */
static unsigned long
tkcore_clear(struct tkcore_dest_buf *dest, size_t size, u8 val)
{
	unsigned long ret = 0;

	if (!tkc.panic) {
		ret = clear_user(dest->user_buf, size);
		if (ret)
			return ret;
		dest->user_buf += size;
	} else {
		memset(dest->kern_buf, val, size);
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
static bool tkcore_valid_addr(struct tkcore_mem_entry *me, const void *addr, size_t size)
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
			continue;
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
 * _tkcore_add_mem_entry() - add a new memory entry into the list
 * @addr: address for the new entry
 * @size: size of the new entry
 * @iter: index number of allocation iterator which adds the entry
 * @flags: special handling requested
 *
 * The new memory entry is added to the memory list &tkc_context.mem_list, which is ordered
 * by the address. It also avoids adding duplicate entries.
 *
 * Return: new entry if successfully added; or existing one if duplicate is found. %NULL
 *	if new entry could not be allocated.
 */
static struct tkcore_mem_entry *
_tkcore_add_mem_entry(const void *addr, size_t size, int iter, u8 flags)
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
			return prev;

		if (prev->addr > addr)
			prev = NULL;
		else
			next = NULL;
		goto alloc_new;

	}

	while (prev != next) {
		/* validate new entry */
		if (!tkcore_valid_addr(prev, addr, size))
			return prev;
		if (!tkcore_valid_addr(next, addr, size))
			return next;

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
	new = tkcore_alloc_zero(sizeof(*new));
	if (!new)
		return NULL;

	pr_debug("entry: addr 0x%px size %zu iter %d flags 0x%02x\n",
		 addr, size, iter, flags);
	new->addr = addr;
	new->size = size;
	new->iter = iter;
	new->cpuid = -1;
	new->flags = flags;

	__list_add(&new->list, prev ? &prev->list : &tkc.mem_list,
		   next ? &next->list : &tkc.mem_list);
	return new;
}

void tkcore_add_mem_entry(const void *addr, size_t size)
{
	if (!tkc.root)
		return;

	_tkcore_add_mem_entry(addr, size, -1, 0);
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
	struct tkcore_alloc_iter_info *iter = &tkc.alloc_iter[iter_index];
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

		_tkcore_add_mem_entry(ae.addr, ae.size, iter_index, 0);
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


#if defined(CONFIG_ARM64)
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
}

/**
 * tkcore_is_ext_abort() - indicates dump was triggered by external abort
 *
 * Architecture specific code to check ESR register.
 */
static bool tkcore_is_ext_abort(void)
{
	unsigned long esr = read_sysreg(esr_el1);

	pr_info("esr = 0x%lx\n", esr);
	return esr & BIT(ESR_ELx_EA_SHIFT);
}
#elif defined(CONFIG_ARM)
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
}

#define DFSR		__ACCESS_CP15(c5, 0, c0, 0)
#define DFSR_ExT	BIT(12)

/**
 * tkcore_is_ext_abort() - indicates dump was triggered by external abort
 *
 * Architecture specific code to check DFSR register.
 */
static bool tkcore_is_ext_abort(void)
{
	unsigned long dfsr = read_sysreg(DFSR);

	pr_info("dfsr = 0x%lx\n", dfsr);
	return dfsr & DFSR_ExT;
}
#endif

/**
 * __tkcore_add_task() - create internal bookkeeping for given task
 * @tsk: task to be tracked
 * @flags: flags for new task entry
 *
 * Creates an entry on the task list for the given task and sets
 * the flags as requested which determines how it is handled in
 * dump creation and cleanup. Returns the task entry.
 */
static
struct tkcore_tsk_entry *__tkcore_add_task(struct task_struct *tsk, u8 flags)
{
	struct tkcore_tsk_entry *te;

	if (!tsk->stack) {
		pr_err("task %s (%d) has no stack!!\n", tsk->comm, tsk->pid);
		return NULL;
	}
	te = tkcore_alloc_zero(sizeof(*te));
	if (!te)
		return NULL;

	pr_debug("adding task %s (%d)\n", tsk->comm, tsk->pid);
	te->tsk = tsk;
	te->flags = flags;
	tkcore_add_mem_entry(te, sizeof(*te));
	_tkcore_add_mem_entry(tsk, arch_task_struct_size, -1, TKCORE_MEM_NEED_BOUNCE);
	te->me = _tkcore_add_mem_entry(tsk->stack, THREAD_SIZE, -1, TKCORE_MEM_DISABLE_KASAN);
	list_add_tail(&te->list, &tkc.tsk_list);
	return te;
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
 * tkcore_fill_segment_headers() - alloc and fill segment headers
 * @fi: file info needed to fill the segment headers
 */
static struct elf_phdr *
tkcore_fill_segment_headers(struct tkcore_file_info *fi)
{
	struct tkcore_mem_entry *me;
	struct elf_phdr *segs, *seg;

	segs = tkcore_alloc_zero(fi->segs_len);
	if (!segs)
		goto out;

	seg = &segs[0];
	seg->p_type = PT_NOTE;
	seg->p_offset = sizeof(struct elfhdr) + fi->segs_len;
	seg->p_filesz = fi->notes_len;

	list_for_each_entry(me, &tkc.mem_list, list) {
		seg++;
		if (WARN_ON(seg >= segs + fi->segs_len)) {
			break;
		}
		seg->p_type = PT_LOAD;
		seg->p_flags = PF_R | PF_W | PF_X;
		seg->p_offset = fi->offset + me->offset;
		seg->p_vaddr = (size_t)me->addr;
		seg->p_filesz = seg->p_memsz = me->size;
		seg->p_align = TKCORE_SEG_ALIGN_SIZE;
	}
out:
	return segs;
}

/**
 * tkcore_fill_segment_headers() - alloc and fill segment headers
 * @fi: file info needed to fill the segment headers
 */
static char *
tkcore_fill_note_segment(size_t len, struct cpumask *running)
{
	struct tkcore_note_info vmcore = {
		0, VMCOREINFO_NOTE_NAME, vmcoreinfo_size
	};
	struct tkcore_tsk_entry *te;
	struct elf_prpsinfo prpsinfo = {
		.pr_sname = 'R',
		.pr_fname = "vmlinux",
	};
	struct tkcore_cpu_state *cpu_state;
	char *notes, *p_note;
	int cpu;

	notes = tkcore_alloc_zero(len);
	if (!notes)
		goto out;

	strlcpy(prpsinfo.pr_psargs, saved_command_line, sizeof(prpsinfo.pr_psargs));

	p_note = notes;

	for_each_cpu_wrap(cpu, running, smp_processor_id()) {
		cpu_state = per_cpu_ptr(tkc.cpus, cpu);
		if (ELF_PRSTAT_PID(&cpu_state->prstatus) == -1 ||
		    !cpu_state->curr_tsk || !cpu_state->me)
			continue;

		p_note = tkcore_fill_note(p_note, &TKCORE_PRSTATUS_INFO, &cpu_state->prstatus);
	}

	p_note = tkcore_fill_note(p_note, &note_info[1], &prpsinfo);
	p_note = tkcore_fill_note(p_note, &note_info[2], current);

	list_for_each_entry(te, &tkc.tsk_list, list) {
		struct elf_prstatus tskstatus;

		if (te->flags & TKCORE_TSK_SKIP_NOTE) {
			pr_debug("skip running task %s\n", te->tsk->comm);
			continue;
		}
		/* avoid out-of-bounds access */
		if (p_note >= notes + len) {
			break;
		}
		memset(&tskstatus, 0, sizeof(tskstatus));
		ELF_PRSTAT_PID(&tskstatus) = te->tsk->pid;
		tkcore_copy_task_regs(te->tsk, &tskstatus);
		p_note = tkcore_fill_note(p_note, &TKCORE_PRSTATUS_INFO, &tskstatus);
	}
	if (p_note < notes + len && vmcoreinfo_size <= (notes + len - p_note)) {
		p_note = tkcore_fill_note(p_note, &vmcore, vmcoreinfo_data);
	}
	WARN_ON(p_note != notes + len);
out:
	return notes;
}

/**
 * tkcore_get_file_info() - prepare ELF corefile creation
 * @fi: file info to fill
 *
 * Fill file info depending on &tkcore_context.mem_list and
 * &tkcore_context.tsk_list.
 *
 * Return: size of the ELF corefile; 0 when this fails.
 */
static size_t
tkcore_get_file_info(struct tkcore_file_info *fi)
{
	struct tkcore_file_info tmp = { 0 };
	struct tkcore_cpu_state *cpu_state;
	struct tkcore_mem_entry *me;
	struct tkcore_tsk_entry *te;
	struct task_struct *tsk;
	struct cpumask running = *cpu_possible_mask;
	size_t data_size = 0;
	int i, cpu;

	/* add tasks for cpu state */
	for_each_possible_cpu(cpu) {
		u8 flags = TKCORE_TSK_SKIP_NOTE;

		cpu_state = per_cpu_ptr(tkc.cpus, cpu);
		if (ELF_PRSTAT_PID(&cpu_state->prstatus) == -1)
			goto clear;
		if (WARN_ON(!cpu_state->curr_tsk))
			goto clear;

		tsk = cpu_state->curr_tsk;
		te = tkcore_find_task_entry(tsk);
		if (te) {
			te->flags |= flags;
		} else {
			flags |= TKCORE_TSK_INTERNAL;
			te = __tkcore_add_task(tsk, flags);
			if (!te) {
				pr_debug("CPU%d: no task stored\n", cpu);
				put_task_stack(tsk);
				put_task_struct(tsk);
				cpu_state->curr_tsk = NULL;
				goto clear;
			}
		}
		cpu_state->me = te->me;
		te->me->cpuid = cpu;
		continue;
clear:
		cpumask_clear_cpu(cpu, &running);
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
	/* add VMCORE_INFO note entry */
	tmp.notes_len += ALIGN(sizeof(struct elf_note), sizeof(Elf_Word));
	tmp.notes_len += VMCOREINFO_NOTE_NAME_BYTES;
	tmp.notes_len += ALIGN(vmcoreinfo_size, sizeof(Elf_Word));

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
	tmp.core_len = tmp.offset + data_size;

	tmp.segs = tkcore_fill_segment_headers(&tmp);
	tmp.notes = tkcore_fill_note_segment(tmp.notes_len, &running);
	*fi = tmp;
	if (tmp.segs && tmp.notes) {
		pr_err("size=%u (%x) n_segs=%d(len=%zu) notes_len=%zu data=0x%zx@0x%zx\n",
			 tmp.core_len, tmp.core_len, tmp.n_segs, tmp.segs_len, tmp.notes_len, data_size, tmp.offset);
		return tmp.core_len;
	}
	return 0;
}

/**
 * tkcore_free_task_entry() - free the memory associated with given task
 * @te: task entry to free
 *
 * Free everything associated with given task entry. The task entry itself
 * should already have been removed from the task list.
 */
static void
tkcore_free_task_entry(struct tkcore_tsk_entry *te)
{
	struct tkcore_mem_entry *me, rem;
	struct task_struct *tsk = te->tsk;

	pr_debug("removing task %s\n", tsk->comm);
	rem.addr = tsk;
	rem.size = arch_task_struct_size;
	me = tkcore_remove_mem_entry(&rem, NULL);
	kfree(me);

	me = te->me;
	list_del(&me->list);
	kfree(me);

	rem.addr = te;
	rem.size = sizeof(*te);
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
static void
tkcore_cleanup_tasks(void)
{
	struct tkcore_tsk_entry *te, *tmp;

	list_for_each_entry_safe(te, tmp, &tkc.tsk_list, list) {
		if (te->flags & TKCORE_TSK_SKIP_NOTE) {
			te->flags &= ~TKCORE_TSK_SKIP_NOTE;
			put_task_stack(te->tsk);
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

	/* initialization failed so bail out */
	if (!tkc.root)
		return;

	/* idle cpu has nothing interesting to show for */
	if (idle_cpu(cpu)) {
		pr_debug("CPU%d: idle\n", cpu);
		return;
	}

	cpu_state = per_cpu_ptr(tkc.cpus, cpu);
	if (WARN_ON(!cpu_state))
		return;

	/* already saved so ignore now */
	if (test_and_set_bit(cpu, &tkc.cpu_state_saved)) {
		return;
	}

	/*
	 * store the task reference and increment usage count
	 * for the task and its stack. This may fail for the stack
	 * if the task is about to be removed although unlikely as
	 * this is the current task.
	 */
	if (WARN_ON(!try_get_task_stack(tsk))) {
		clear_bit(cpu, &tkc.cpu_state_saved);
		return;
	}
	pr_debug("CPU%d: task %s (%d)\n", cpu, tsk->comm, tsk->pid);
	get_task_struct(tsk);

	/*
	 * Simply copy the entire stack. By doing so we are
	 * likely to get a KASAN report hence we disable it
	 * during the copy.
	 */
	kasan_disable_current();
	memcpy(cpu_state->saved_stack, task_stack_page(tsk), THREAD_SIZE);
	kasan_enable_current();

	ELF_PRSTAT_PID(&cpu_state->prstatus) = tsk->pid;
	cpu_state->curr_tsk = tsk;
	if (!regs) {
		crash_setup_regs(&cpu_regs, NULL);
		regs = &cpu_regs;
	}
	elf_core_copy_regs(&cpu_state->prstatus.pr_reg, regs);
}

/**
 * tkcore_smp_save_cpu_state() - IPI callback to save cpu state
 *
 * @info: unused
 *
 */
static void tkcore_smp_save_cpu_state(void *info)
{
	ulong flags;
	int cpu = smp_processor_id();

	local_irq_save(flags);
	tkcore_save_cpu_state(get_irq_regs(), cpu);
	local_irq_restore(flags);
}

/**
 * tkcore_can_save_cpu_state() - check condition to invoke smp call or not
 *
 * @cpu: cpu to check
 * @info: unused
 */
static bool tkcore_can_save_cpu_state(int cpu, void *info)
{
	return !test_bit(cpu, &tkc.cpu_state_saved);
}

/**
 * tkcore_save_all_cpu_state() - store state of all non-idle cpus
 *
 * saving the state starting with the current CPU so it will not
 * be called by on_each_cpu_cond() which is needed to get the proper
 * registers saved. on_each_cpu_cond() can only be used in thread
 * context as it might end up in deadlock otherwise.
 */
void tkcore_save_all_cpu_state(void)
{
	ulong flags;

	/* initialization failed so bail out */
	if (!tkc.root)
		return;

	might_sleep();

	local_irq_save(flags);
	tkcore_save_cpu_state(NULL, smp_processor_id());
	local_irq_restore(flags);
	if (in_task()) {
		mutex_lock(&tkcore_smp_call_lock);
		on_each_cpu_cond(tkcore_can_save_cpu_state, tkcore_smp_save_cpu_state, NULL, true);
		mutex_unlock(&tkcore_smp_call_lock);
	}
}
EXPORT_SYMBOL(tkcore_save_all_cpu_state);

/**
 * tkcore_clear_cpu_state() - reinitializes the cpu state
 * @cpu: given cpu id
 */
static void
tkcore_clear_cpu_state(int cpu)
{
	struct tkcore_cpu_state *cpu_state;

	cpu_state = per_cpu_ptr(tkc.cpus, cpu);
	if (!cpu_state)
		return;

	if (cpu_state->me)
		cpu_state->me->cpuid = -1;
	memset(cpu_state, 0, sizeof(*cpu_state));
	ELF_PRSTAT_PID(&cpu_state->prstatus) = -1;
	cpu_state->curr_tsk = NULL;
	/* allocate saved stack once when called from proc_tkcore_init() */
	if (!cpu_state->saved_stack)
		cpu_state->saved_stack = kzalloc(THREAD_SIZE, GFP_KERNEL);
	clear_bit(cpu, &tkc.cpu_state_saved);
}

/**
 * tkcore_cpu_state_filled() - indicate if state of given cpu is stored
 * @cpu: given cpu id
 */
static bool
tkcore_cpu_state_filled(int cpu)
{
	struct tkcore_cpu_state *cpu_state;

	cpu_state = per_cpu_ptr(tkc.cpus, cpu);
	if (!cpu_state)
		return false;

	return (ELF_PRSTAT_PID(&cpu_state->prstatus) != -1);
}

static void
tkcore_enable_kasan(struct tkcore_mem_entry *me)
{
	if (!(me->flags & TKCORE_MEM_DISABLE_KASAN))
		return;

	kasan_enable_current();
}

static void
tkcore_disable_kasan(struct tkcore_mem_entry *me)
{
	if (!(me->flags & TKCORE_MEM_DISABLE_KASAN))
		return;

	kasan_disable_current();
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
	struct tkcore_cpu_state *cpu_state;
	struct tkcore_mem_entry *me;
	size_t segs_offset, notes_offset, copy_size;
	size_t orig_len = dest->len;
	ssize_t ret = 0;

	segs_offset = sizeof(struct elfhdr);
	notes_offset = segs_offset + fi->segs_len;

	/* file header */
	if (dest->len && *dest->off < segs_offset) {
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
		copy_size = min_t(size_t, dest->len, segs_offset - *dest->off);
		if (tkcore_copy_to(dest, (char *)&eh + *dest->off, copy_size)) {
			ret = -EFAULT;
			goto out;
		}
	}

	/* segment headers */
	if (dest->len && *dest->off < notes_offset) {
		copy_size = min_t(size_t, dest->len, notes_offset - *dest->off);
		if (tkcore_copy_to(dest, (char *)fi->segs + *dest->off - segs_offset,
				   copy_size)) {
			ret = -EFAULT;
			goto out;
		}
	}

	/* notes */
	if (dest->len && *dest->off < notes_offset + fi->notes_len) {
		copy_size = min_t(size_t, dest->len, notes_offset + fi->notes_len - *dest->off);
		if (tkcore_copy_to(dest, fi->notes + *dest->off - notes_offset, copy_size)) {
			ret = -EFAULT;
			goto out;
		}
	}

	list_for_each_entry(me, &tkc.mem_list, list) {
		size_t seg_offset = fi->offset + me->offset;

		/* fill page alignment space */
		if (dest->len && *dest->off < seg_offset) {
			copy_size = min_t(size_t, dest->len, seg_offset - *dest->off);
			if (tkcore_clear(dest, copy_size, 0)) {
				ret = -EFAULT;
				goto out;
			}
		}
		/* now copy actual data */
		if (dest->len && *dest->off < seg_offset + me->size) {
			copy_size = min_t(size_t, dest->len, seg_offset + me->size - *dest->off);
			if (!tkc.panic && (me->flags & TKCORE_MEM_NEED_BOUNCE)) {
				struct tkcore_proc_priv *pd = fh->private_data;
				bounce = pd->bounce;
				/* limit copy size to bounce buffer size */
				copy_size = min_t(size_t, copy_size, PAGE_SIZE);
				if (COPY_KERNEL_BOUNCE(bounce,
						       (void *)(me->addr + *dest->off - seg_offset),
						       copy_size)) {
					if (tkcore_clear(dest, copy_size, 0)) {
						ret = -EFAULT;
						goto out;
					}
				} else {
					if (tkcore_copy_to(dest, bounce, copy_size)) {
						ret = -EFAULT;
						goto out;
					}
				}
			} else {
				const void *addr;

				if (me->cpuid >= 0) {
					cpu_state = per_cpu_ptr(tkc.cpus, me->cpuid);
					addr = cpu_state->saved_stack;
					pr_debug("storing saved stack %px for CPU%d: task %s (%d)\n", addr,
						 me->cpuid, cpu_state->curr_tsk->comm, cpu_state->curr_tsk->pid);
				} else {
					addr = me->addr;
				}
				tkcore_disable_kasan(me);
				if (tkcore_copy_to(dest,
					(void *)(addr + *dest->off - seg_offset),
					copy_size)) {
					if (tkcore_clear(dest, copy_size, POISON_FREE)) {
						ret = -EFAULT;
						tkcore_enable_kasan(me);
						goto out;
					}
				}
				tkcore_enable_kasan(me);
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
 * tkcore_regdump_info_is_valid() - validate register dump info
 * @rdi: register dump info to validate
 *
 * Validates the register dump info after calling a register dump
 * callback, which may have modified the fields.
 */
static bool tkcore_regdump_info_is_valid(struct tkcore_regdump_info *rdi)
{
	/* 1. bufsz may not exceed allocated buffer size */
	if (rdi->bufsz > tkc.regdump_bufsz)
		return false;

	return true;
}

/**
 * tkcore_regdump_read() - reading register dump info in blocks
 *
 * @dest: destination buffer information
 *
 * Will invoke all registered regdump callbacks and store the obtained
 * dumps in lzo compressed format in the destination buffer which will
 * be writted to the MTD partition. Please note that a register dump
 * callback can provide multiple dumps.
 */
static ssize_t tkcore_regdump_read(struct tkcore_dest_buf *dest, bool ext_abort,
	struct list_head *regdump_list)
{
	struct tkcore_curr_regdump *rda = &tkc.rd_active;
	struct tkcore_regdump_info rdi = {
		.buf = tkc.regdump_buf,
		.bufsz = tkc.regdump_bufsz,
		.ext_abort = ext_abort,
	};
	u16 end_marker = 0xFFFF;
	loff_t start_offset = *dest->off;
	size_t dump_buf_size = 0;
	struct tkcore_dev_regdump *tmp, *rd = rda->rd_curr;
	size_t copy_size;
	bool new_rd = false;
	int err;

	rda->rd_curr = NULL;
	if (!rd) {
		rd = list_first_entry_or_null(regdump_list, typeof(*rd), list);
		new_rd = true;
	} else {
		rdi.more = rda->more;
		rdi.offset = rda->offset;
	}

	while (rd && dest->len) {
		u32 data_len;
		u8 idlen;

		if (new_rd) {
			new_rd = false;
			pr_debug("handle [%u] %ps\n", rd->inst, rd->dev_regdump_fn);
			rda->mtd_pos = *dest->off;
			rda->iter = -1;
		}
		if (!rda->lzo_remain) {
			rda->lzo_size = 0;
			rda->mtd_pos = *dest->off;
			if (!rdi.more) {
				rdi.offset = 0;
				rda->iter++;
			}
			err = rd->dev_regdump_fn(rd->ctx, rda->iter, &rdi);
			if (err < 0 || !tkcore_regdump_info_is_valid(&rdi)) {
				tmp = rd;
				if (list_is_last(&tmp->list, regdump_list)) {
					tkc.regdump_end_pos = *dest->off + 2;
					rd = NULL;
				} else {
					rd = list_next_entry(tmp, list);
				}
				list_del(&tmp->list);
				kfree(tmp);
				new_rd = true;
				continue;
			}

			dump_buf_size = rdi.bufsz;
			rdi.bufsz = tkc.regdump_bufsz;

			if (!dump_buf_size)
				continue;

			pr_debug("dump %s collected (%zu)\n", rdi.id, dump_buf_size);
			err = lzo1x_1_compress(tkc.regdump_buf, dump_buf_size,
					       tkc.regdump_lzo, &rda->lzo_size,
					       tkc.lzo_workmem);
			if (err < 0) {
				pr_err("compression dump %s failed (err=%d)\n",
					rdi.id, err);
				return -EFAULT;
			}

			pr_debug("LZO compress: %zu -> %zu\n", dump_buf_size,
			       rda->lzo_size);

			rda->lzo_remain = rda->lzo_size;
			idlen = strnlen(rdi.id, 20);
			data_len = rda->lzo_size;
			rda->mtd_hdr[REGDUMP_MTDHDR_INST] = rd->inst;
			rda->mtd_hdr[REGDUMP_MTDHDR_TYPE] = rdi.type;
			rda->mtd_hdr[REGDUMP_MTDHDR_MORE] = !!rdi.more;
			rda->mtd_hdr[REGDUMP_MTDHDR_IDLEN] = idlen;
			memcpy(&rda->mtd_hdr[REGDUMP_MTDHDR_ID], rdi.id, idlen);
			memcpy(&rda->mtd_hdr[REGDUMP_MTDHDR_ID] + idlen, &data_len, sizeof(data_len));
			rda->mtd_hdr_len = idlen + REGDUMP_MTDHDR_ID + sizeof(data_len);
		}
		/* write regdump header */
		if (dest->len && *dest->off < (rda->mtd_pos + rda->mtd_hdr_len)) {
			copy_size = min_t(size_t, dest->len, rda->mtd_pos + rda->mtd_hdr_len - *dest->off);
			if (tkcore_copy_to(dest, &rda->mtd_hdr[*dest->off - rda->mtd_pos], copy_size))
				return -EFAULT;
		}
		/* write compressed regdump data */
		if (dest->len && *dest->off < (rda->mtd_pos + rda->mtd_hdr_len + rda->lzo_size)) {
			copy_size = min_t(size_t, dest->len, rda->lzo_remain);
			if (tkcore_copy_to(dest, tkc.regdump_lzo + rda->lzo_size - rda->lzo_remain, copy_size))
				return -EFAULT;
			rda->lzo_remain -= copy_size;
		}
	}
	/* register dumps completed? */
	if (rd) {
		rda->rd_curr = rd;
		rda->more = rdi.more;
		rda->offset = rdi.offset;
	}

	if (dest->len && *dest->off < tkc.regdump_end_pos) {
		copy_size = min_t(size_t, dest->len, tkc.regdump_end_pos - *dest->off);
		if (tkcore_copy_to(dest, (char *)&end_marker + 2 - copy_size, copy_size))
			return -EFAULT;
	}
	return *dest->off - start_offset;
}

/**
 * tkcore_elf_core_dump() - obtain ELF coredump in chunks
 *
 * @ctx: context which is verified to be same as static global tkc variable.
 * @rdi: register dump related information.
 *
 * invoked by dump callback providing ELF coredump in chunks which are
 * compressed and stored on MTD partition.
 */
static int
tkcore_elf_core_dump(void *ctx, struct tkcore_regdump_info *rdi)
{
	loff_t newpos = rdi->offset;
	struct tkcore_dest_buf dest = {
		.kern_buf = rdi->buf,
		.len = rdi->bufsz,
		.off = &newpos
	};
	ssize_t ret;

	rdi->id = "tkcore";
	rdi->type = TKCORE_REGDUMP_BINARY;

	if (!rdi->offset) {
		if (!tkcore_get_file_info(&tkc.file_info))
			return -EIO;
	}

	ret = __tkcore_read(NULL, &tkc.file_info, &dest);
	if (ret < 0)
		return ret;

	rdi->bufsz = ret;
	rdi->offset = newpos;
	rdi->more = !!(tkc.file_info.core_len - rdi->offset);
	pr_debug("offset %lu more %d\n", rdi->offset, rdi->more);
	return 0;
}

/**
 * tkcore_printk_log_dump() - obtain kernel log in chunks
 *
 * @ctx: context which is verified to be same as static global tkc variable.
 * @rdi: register dump related information.
 *
 * invoked by dump callback providing kernel log in chunks which are
 * compressed and stored on MTD partition, but dump buffer exceeds the
 * kernel log buffer size so there will be just one part compressed and
 * stored.
 */
static int
tkcore_printk_log_dump(void *ctx, struct tkcore_regdump_info *rdi)
{
	struct kmsg_dump_iter dump_iter;
	size_t retlen;

	rdi->id = "dmesg";
	rdi->type = TKCORE_REGDUMP_TEXT;
	kmsg_dump_rewind(&dump_iter);
	kmsg_dump_get_buffer(&dump_iter, false, rdi->buf, rdi->bufsz, &retlen);

	pr_debug("collected %zu bytes\n", retlen);
	rdi->bufsz = retlen;
	rdi->more = false;
	return 0;
}

/**
 * tkcore_regdump_collect() - invoke individual dump producers
 *
 * @ctx: context which is verified to be same as static global tkc variable.
 * @iter: iteration identifier.
 * @rdi: register dump related information.
 *
 * dump callback providing ELF coredump (when space requirement is
 * met) and kernel log.
 */
static int tkcore_regdump_collect(void *ctx, uint iter, struct tkcore_regdump_info *rdi)
{
	uint printk_log_iter = 0;
	
	/* something really wrong here so bail out */
	if (WARN_ON(ctx != &tkc)) {
		pr_err("invalid context\n");
		return -ENOENT;
	}

	if (!tkc.mimic_mtdoops) {
		printk_log_iter++;
		if (iter == 0)
			return tkcore_elf_core_dump(ctx, rdi);
	}
	if (iter == printk_log_iter)
		return tkcore_printk_log_dump(ctx, rdi);

	pr_debug("done\n");
	return -ENOENT;
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
	loff_t curpos, newpos = 0;
	struct tkcore_regdump_offset rd_start = {
		.version = TKCORE_REGDUMP_VERSION,
		.compression = TKCORE_REGDUMP_COMPRESS_LZO,
		.blocksize = tkc.regdump_bufsz >> 20,
		.start_offset = 0,
	};
	struct tkcore_dest_buf dest = {
		.kern_buf = tkc.panic_buf,
		.len = TKCORE_MTD_WRITE_BLOCKSIZE,
		.off = &newpos
	};
	struct list_head regdump_list, *regdump_ref;
	struct tkcore_dev_regdump *tmp;
	struct pt_regs regs;
	ssize_t ret = 1;
	struct mtd_info *mtd;
	size_t mtd_retlen, len;
	size_t bytes_written = 0;
	size_t copy_size;
	bool ext_abort;
	int mtd_ret, i, cpu = smp_processor_id();

	if (reason != KMSG_DUMP_PANIC)
		return;

	pr_debug("panic in task %s stack 0x%px\n", current->comm, current->stack);

	/*
	 * if tkc.panic is already set this means a panic
	 * within a panic occurred so better bail out here
	 * and also bail out when there is no MTD to write to.
	 */
	if (tkc.panic || !tkc.mtd)
		return;

	/* executing in panic context */
	tkc.panic = true;

	tkcore_watchdog_ping();

	for (i = 0; i < ARRAY_SIZE(tkc.alloc_iter); i++) {
		tkcore_run_alloc_iterator(i);
	}

	/* only current CPU running when we end up here */
	if (!tkcore_cpu_state_filled(cpu)) {
		crash_setup_regs(&regs, NULL);
		tkcore_save_cpu_state(&regs, cpu);
	}


	pr_info("creating MTD dumps\n");
	memset(tkc.regdump_buf, 0, tkc.regdump_bufsz);
	rd_start.blocksize = ffs(rd_start.blocksize) - 1;
	rd_start.start_offset = ALIGN(sizeof(rd_start), 8);
	memcpy(&rd_start.elf_magic, ELFMAG, sizeof(rd_start.elf_magic));
	mtd = tkc.mtd;

	/* pass limited list of dump producers (only one) */
	if (tkc.mimic_mtdoops || tkcore_minimal) {
		INIT_LIST_HEAD(&regdump_list);
		tmp = list_first_entry_or_null(&tkc.regdump_list, typeof(*tmp), list);
		list_del(&tmp->list);
		list_add(&tmp->list, &regdump_list);
		regdump_ref = &regdump_list;
	} else {
		regdump_ref = &tkc.regdump_list;
	}
	
	ext_abort = tkcore_is_ext_abort();
	while (ret > 0) {
		tkcore_watchdog_ping();

		/* reset buffer info */
		dest.kern_buf = tkc.panic_buf;
		dest.len = TKCORE_MTD_WRITE_BLOCKSIZE;
		curpos = newpos;

		/* register dump start header */
		if (dest.len && *dest.off < sizeof(rd_start)) {
			copy_size = min_t(size_t, dest.len, sizeof(rd_start) - *dest.off);
			if (tkcore_copy_to(&dest, (char *)&rd_start + *dest.off, copy_size)) {
				ret = -EFAULT;
				break;
			}
		}

		if (dest.len && *dest.off < rd_start.start_offset) {
			copy_size = min_t(size_t, dest.len, rd_start.start_offset - *dest.off);
			if (tkcore_clear(&dest, copy_size, 0)) {
				ret = -EFAULT;
				break;
			}
		}

		/* read chunk of data and update newpos */
		ret = tkcore_regdump_read(&dest, ext_abort, regdump_ref);
		if (ret <= 0)
			break;

		len = PAGE_ALIGN(newpos - curpos);
		pr_debug("write to mtd: 0x%zx bytes @ 0x%llx\n", len, curpos);
		mtd_ret = mtd_panic_write(mtd, curpos, len, &mtd_retlen, tkc.panic_buf);
		if (mtd_retlen != len || mtd_ret < 0) {
			pr_err("write failure (ret=%d) @ 0x%llx (0x%zx of 0x%zx written)\n",
			       mtd_ret, curpos, mtd_retlen, len);
			ret = -EIO;
		} else {
			bytes_written += mtd_retlen;
		}
	}
	pr_info("coredump complete: size 0x%zx mtd%d (%s)\n", bytes_written,
		mtd->index, mtd->name);
}

/**
 * tkcore_mtd_match() - given MTD can be used as name matches
 * @mtd: MTD partition to verify
 */
static bool tkcore_mtd_match(struct mtd_info *mtd)
{
	static const char crash_mtd[] = "crashdump";

	if (!strncmp(mtd->name, tkcore_mtdname, sizeof(tkcore_mtdname)))
		return true;

	/* if MTD_OOPS is disabled we may try to claim "crashdump" partition */
	if (IS_ENABLED(CONFIG_MTD_OOPS))
		return false;

	if (!strncmp(mtd->name, crash_mtd, sizeof(crash_mtd))) {
		strncpy(tkcore_mtdname, crash_mtd, sizeof(tkcore_mtdname));
		return true;
	}

	return false;
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
	struct tkcore_regdump_offset rd_start;
	u32 signature, scratch_bufsz;
	size_t mtd_retlen;
	int err;

	BUILD_BUG_ON(TKCORE_MTD_WRITE_BLOCKSIZE & 0xFFF);
	if (!tkcore_mtd_match(mtd))
		return;

	tkc.mempool = kzalloc(TKCORE_MEMPOOL_SIZE, GFP_KERNEL);
	if (!tkc.mempool)
		return;

	tkc.panic_buf = kzalloc(TKCORE_MTD_WRITE_BLOCKSIZE, GFP_KERNEL);
	if (!tkc.panic_buf)
		goto fail;

	/*
	 * LZO compression may result in longer buffer than the input when
	 * it is uncompressible so need to take worst case into account.
	 */
	tkc.regdump_bufsz = TKCORE_INITIAL_SCRATCH_BUFSIZE >> 1;
	scratch_bufsz = tkc.regdump_bufsz + lzo1x_worst_compress(tkc.regdump_bufsz);
	tkc.scratch_buf = kzalloc(scratch_bufsz, GFP_KERNEL);
	if (!tkc.scratch_buf)
		goto fail;

	tkc.regdump_buf = tkc.scratch_buf;
	tkc.regdump_lzo = tkc.scratch_buf + tkc.regdump_bufsz;

	err = mtd_read(mtd, sizeof(rd_start), sizeof(signature),
		       &mtd_retlen, (u8 *)&signature);
	if (!err && mtd_retlen == sizeof(signature) && signature != 0xffffffff) {
		pr_warn(">>>> MTD NOT EMPTY (%sELF magic signature found) <<<<\n",
			memcmp(&signature, ELFMAG, sizeof(signature)) ? "no " : "");
	} else {
		pr_info(">>>>>>> MTD IS EMPTY <<<<<<<\n");
	}

	tkc.lzo_workmem = kzalloc(LZO1X_1_MEM_COMPRESS, GFP_KERNEL);
	if (!tkc.lzo_workmem)
		goto fail;

	tkc.kmsg.max_reason = KMSG_DUMP_PANIC;
	tkc.kmsg.dump = tkcore_do_dump;
	err = kmsg_dump_register(&tkc.kmsg);
	if (err) {
		pr_warn("dumper registration failed: err=%d\n", err);
		goto fail;
	}

	/* self register for dump creation */
	tkcore_add_dev_register_dumper(&tkc, log_buf_len_get(), -1, tkcore_regdump_collect);

	pr_info("using mtd%d (%s) for coredump storage\n", mtd->index, mtd->name);
	tkcore_mtddev = mtd->index;
	tkc.mtd = mtd;
	tkc.mimic_mtdoops = (mtd->size < TKCORE_MTD_MIN_SIZE);
	return;

fail:
	kfree(tkc.scratch_buf);
	kfree(tkc.panic_buf);
	kfree(tkc.mempool);
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
	ssize_t ret;

	if (*off == 0) {
		tkcore_save_all_cpu_state();
		if (!tkcore_get_file_info(&pd->fi)) {
			return -ENOMEM;
		}
	}

	ret = __tkcore_read(fh, &pd->fi, &tmp);

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

	mutex_lock(&tkcore_proc_lock);

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
	struct tkcore_proc_priv *pd;
	struct tkcore_alloc_iter_info *iter;
	int i;

	for_each_possible_cpu(i) {
		tkcore_clear_cpu_state(i);
	}
	tkcore_cleanup_tasks();
	for (i = 0; i < ARRAY_SIZE(tkc.alloc_iter); i++) {
		tkcore_remove_alloc_iterator_entries(i);
		iter = &tkc.alloc_iter[i];
		if (iter->ctx && iter->done_fn)
			iter->done_fn(iter->ctx);
	}

	mutex_unlock(&tkcore_proc_lock);

	pd = fh->private_data;
	kfree(pd->fi.segs);
	kfree(pd->fi.notes);
	kfree(pd);
	return 0;
}

static const PROC_ENTRY_OPS proc_tkcore_ops = {
	.PROC_READ(tkcore_read),
	.PROC_OPEN(tkcore_open),
	.PROC_RELEASE(tkcore_release),
	.PROC_LSEEK(default_llseek),
};

#ifdef CONFIG_IKCONFIG
/* globals related to included config.gz binary */
extern char kernel_config_data;
extern char kernel_config_data_end;
#endif

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

	tkc.root = proc_create(TKCORE_DUMPFILE, 0444, NULL, &proc_tkcore_ops);
	if (!tkc.root) {
		pr_err("failed to create /proc/%s\n", TKCORE_DUMPFILE);
		return -EIO;
	}

	tkc.cpus = __alloc_percpu(sizeof(struct tkcore_cpu_state), TKCORE_SEG_ALIGN_SIZE);
	if (!tkc.cpus) {
		proc_remove(tkc.root);
		tkc.root = NULL;
		return -ENOMEM;
	}

	/* initalize dump lists */
	INIT_LIST_HEAD(&tkc.mem_list);
	INIT_LIST_HEAD(&tkc.tsk_list);
	INIT_LIST_HEAD(&tkc.regdump_list);

	/* add task list */
	tkcore_add_mem_entry(&tkc, sizeof(tkc));
	tkcore_add_mem_entry(&tkc.tsk_list, sizeof(tkc.tsk_list));

	/* add banner */
	target_banner = kstrdup(linux_banner, GFP_KERNEL);
	tkcore_add_mem_entry(&target_banner, sizeof(void *));
	tkcore_add_mem_entry(target_banner, strlen(target_banner) + 1);

	/* add printk log buffer */
	tkcore_add_mem_entry(log_buf_addr_get(), log_buf_len_get());

#ifdef CONFIG_IKCONFIG
	/* add in-kernel config data */
	tkcore_add_mem_entry(&kernel_config_data,
			     &kernel_config_data_end - &kernel_config_data);
#endif

	for_each_possible_cpu(cpu) {
#if defined(CONFIG_ARM64)
		/* add irq stack entry */
		tkcore_add_mem_entry(per_cpu(irq_stack_ptr, cpu), IRQ_STACK_SIZE);
#endif
		/* clear cpu state */
		tkcore_clear_cpu_state(cpu);
	}

	tkcore_mtddev = -1;
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
	const void *addr;
	size_t size;

	if (!tkc.root)
		return;

	pr_debug("adding %s module\n", mod->name);
	addr = mod->core_layout.base;
	size = mod->core_layout.size;
	_tkcore_add_mem_entry(addr, size, -1, TKCORE_MEM_DISABLE_KASAN);
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

	if (!tkc.root)
		return;

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
	if (!tkc.root)
		return;

	/* increase refcount */
	get_task_struct(tsk);
	try_get_task_stack(tsk);

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

	if (!tkc.root)
		return;

	list_for_each_entry_safe(te, tmp, &tkc.tsk_list, list) {
		if (te->tsk == tsk) {
			list_del(&te->list);
			tkcore_free_task_entry(te);

			/* decrease refcount */
			put_task_stack(tsk);
			put_task_struct(tsk);
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
void tkcore_add_alloc_iterator(struct tkcore_alloc_iter_info *aii)
{
	int i;

	if (!tkc.root)
		return;

	pr_debug("adding iterator: ctx=%px\n", aii->ctx);
	for (i = 0; i < ARRAY_SIZE(tkc.alloc_iter); i++) {
		if (!memcmp(&tkc.alloc_iter[i], aii, sizeof(*aii))) {
			pr_warn("iterator <%px> already added\n", aii->ctx);
			return;
		}
		if (!aii->iter_fn) {
			pr_err("no iterator callback given\n");
			return;
		}
		if (!aii->done_fn) {
			pr_warn("no iteration done callback given\n");
		}
		if (!tkc.alloc_iter[i].ctx) {
			pr_debug("added iterator <%px>\n", aii->ctx);
			tkc.alloc_iter[i] = *aii;
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
	struct tkcore_alloc_iter_info *iter;
	int i;

	if (!tkc.root)
		return;

	for (i = 0; i < ARRAY_SIZE(tkc.alloc_iter); i++) {
		iter = &tkc.alloc_iter[i];
		if (iter->ctx == ctx) {
			pr_debug("removing iterator <%px %ps>\n", ctx,
				iter->iter_fn);
			tkcore_remove_alloc_iterator_entries(i);
			memset(iter, 0, sizeof(*iter));
			return;
		}
	}
}
EXPORT_SYMBOL(tkcore_remove_alloc_iterator);

/**
 * tkcore_add_dev_register_dumper() - add register dump callback
 *
 * @ctx: opaque context associated with the register dump callback
 * @dump_buf_size: buffer size required by register dump callback
 * @inst_id: unique identifier for the register dump
 * @dev_regdump_fn: the callback function being added
 */
void tkcore_add_dev_register_dumper(void *ctx,
	size_t dump_buf_size, u8 inst_id,
	int (*dev_regdump_fn)(void *ctx, uint iter, struct tkcore_regdump_info *rdi))
{
	struct tkcore_dev_regdump *rd;

	if (!tkc.root)
		return;

	rd = kzalloc(sizeof(*rd), GFP_KERNEL);
	if (!rd)
		return;

	rd->ctx = ctx;
	rd->inst = inst_id;
	rd->dev_regdump_fn = dev_regdump_fn;
	if (dump_buf_size > tkc.regdump_bufsz) {
		char *tmp;

		tmp = kzalloc(dump_buf_size + lzo1x_worst_compress(dump_buf_size), GFP_KERNEL);
		if (!tmp) {
			kfree(rd);
			return;
		}

		kfree(tkc.scratch_buf);
		tkc.regdump_bufsz = dump_buf_size;
		tkc.scratch_buf = tmp;
		tkc.regdump_buf = tkc.scratch_buf;
		tkc.regdump_lzo = tkc.scratch_buf + tkc.regdump_bufsz;
	}
	pr_debug("adding %ps\n", dev_regdump_fn);
	list_add_tail(&rd->list, &tkc.regdump_list);
}
EXPORT_SYMBOL(tkcore_add_dev_register_dumper);

/**
 * tkcore_remove_dev_register_dumper() - remove register dump callback
 *
 * @ctx: opaque context associated with the register dump callback
 */
void tkcore_remove_dev_register_dumper(void *ctx)
{
	struct tkcore_dev_regdump *rd, *tmp;

	if (!tkc.root)
		return;

	list_for_each_entry_safe(rd, tmp, &tkc.regdump_list, list) {
		if (rd->ctx == ctx) {
			list_del(&rd->list);
			pr_debug("removing %ps\n", rd->dev_regdump_fn);
			kfree(rd);
			break;
		}
	}
}
EXPORT_SYMBOL(tkcore_remove_dev_register_dumper);
