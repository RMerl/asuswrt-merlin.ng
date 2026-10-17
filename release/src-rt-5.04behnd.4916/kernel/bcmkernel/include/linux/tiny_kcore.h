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
#ifndef __TINY_KCORE_H
#define __TINY_KCORE_H

/*
 * Define for the API version which is supported. If not defined
 * the API version is 1. The API user shall define TKCORE_API_SELECT
 * with the requested API version. If not defined version 1 is assumed.
 */
#define TKCORE_API_VERSION		(2)
#ifndef TKCORE_API_SELECT
#define TKCORE_API_SELECT		(1)
#endif
#if TKCORE_API_SELECT > TKCORE_API_VERSION
#error "unsupported API selected by TKCORE_API_SELECT"
#endif

/* register dump defines */
#define TKCORE_REGDUMP_VERSION		(2)
#define TKCORE_REGDUMP_COMPRESS_LZO	(0)

/**
 * struct tkcore_alloc_entry - memory allocation information
 *
 * @addr: address of allocated memory
 * @size: size of allocated memory
 *
 * used by tkcore as argument calling the alloc iterator callback
 * function which should fill in the fields.
 */
struct tkcore_alloc_entry {
	void *addr;
	size_t size;
};

/**
 * enum tkcore_regdump_type - type of register dump
 *
 * @TKCORE_REGDUMP_TEXT: text based register dump
 * @TKCORE_REGDUMP_BINARY: binary register dump
 */
enum tkcore_regdump_type {
	TKCORE_REGDUMP_TEXT,
	TKCORE_REGDUMP_BINARY
};

/**
 * struct tkcore_regdump_info - register dump information
 *
 * @dump_id: identifier the dump obtained from register dump callback
 * @type: indicates whether register dump is binary or text
 * @ext_abort: indicates panic was due to external abort
 * @more: dump callback indicating current dump is fragmented.
 * @buf: buffer provided to the register dump callback
 * @bufzs: size of @buf that should not be exceeded by register dump callback
 * @offset: offset to start the dump from.
 *
 * passed calling register dump callback in which the caller provides the buffer
 * and consequently the size of the buffer and possibly offset if the register
 * dump is fragmented. The callback must fill the dump id, type of dump, change
 * the @bufsz with actual number of bytes that were stored in @buf, indicate
 * if there are more fragments and set the offset for next fragment. The iter
 * parameter will remain the same for subsequent fragments of the same dump, ie.
 * when for the callback sets @more as true for @iter 2, the following callback
 * will also have iter 2. When the callback sets @more as false the iter will
 * increment and offset is reset to zero. The callback is called until it
 * returns -ENOENT.
 */
struct tkcore_regdump_info {
	const char *id;
	enum tkcore_regdump_type type;
	bool ext_abort;
	bool more;
	u8 *buf;
	size_t bufsz;
	off_t offset;
};

/**
 * struct tkcore_alloc_iter_info - alloc iterator callback information
 *
 * @ctx: context to be used in callbacks
 * @iter_fn: iterator callback for retrieving allocation entries
 * @done_fn: callback to inform that caller is done using the entries
 *
 * After obtaining all entries the alloc iterator provider must guarantee
 * that the allocation entries are not freed. They can be freed after the
 * @done_fn callback has been used.
 */
struct tkcore_alloc_iter_info {
	void *ctx;
	int (*iter_fn)(void *ctx, struct tkcore_alloc_entry *entry);
	void (*done_fn)(void *ctx);
};

/* forward declarations */
struct module;
struct task_struct;
struct pt_regs;
struct printk_ringbuffer;

#if defined(CONFIG_BCM_TINY_KCORE)
void tkcore_add_mem_entry(const void *addr, size_t size);
void tkcore_save_cpu_state(struct pt_regs *regs, uint cpu);
void tkcore_save_all_cpu_state(void);
void tkcore_add_module(struct module *mod);
void tkcore_remove_module(struct module *mod);
void tkcore_add_task(struct task_struct *tsk);
void tkcore_remove_task(struct task_struct *tsk);
void tkcore_add_alloc_iterator(struct tkcore_alloc_iter_info *aii);
#if TKCORE_API_SELECT == 1
static inline
void tkcore_add_alloc_iter_v1(void *ctx,
	int (*iter_fn)(void *ctx, struct tkcore_alloc_entry *entry))
{
	struct tkcore_alloc_iter_info aii;

	aii.ctx = ctx;
	aii.iter_fn = iter_fn;
	aii.done_fn = NULL;
	tkcore_add_alloc_iterator(&aii);
}
#define tkcore_add_alloc_iterator(_ctx, _iter_fn) \
	tkcore_add_alloc_iter_v1((_ctx), (_iter_fn))
#endif
void tkcore_remove_alloc_iterator(void *ctx);
void tkcore_add_dev_register_dumper(void *ctx, size_t dump_buf_size, u8 inst_id,
	int (*dev_regdump_fn)(void *ctx, uint iter, struct tkcore_regdump_info *rdi));
void tkcore_remove_dev_register_dumper(void *ctx);
#else
static inline void tkcore_add_mem_entry(const void *addr, size_t size) {}
static inline void tkcore_save_cpu_state(struct pt_regs *regs, uint cpu) {}
static inline void tkcore_save_all_cpu_state(void) {}
static inline void tkcore_add_module(struct module *mod) {}
static inline void tkcore_remove_module(struct module *mod) {}
static inline void tkcore_add_task(struct task_struct *tsk) {}
static inline void tkcore_remove_task(struct task_struct *tsk) {}
static inline void tkcore_add_alloc_iterator(void *ctx,
	int (*alloc_iter_fn)(void *ctx, struct tkcore_alloc_entry *entry)) {}
static inline void tkcore_remove_alloc_iterator(void *ctx) {}
static inline void tkcore_add_dev_register_dumper(void *ctx,
	size_t dump_buf_size, u8 inst_id,
	int (*dev_regdump_fn)(void *ctx, uint iter, struct tkcore_regdump_info *rdi))
{
}
static inline void tkcore_remove_dev_register_dumper(void *ctx) { }
#endif /* CONFIG_BCM_TINY_KCORE */

#endif /* __TINY_KCORE_H */