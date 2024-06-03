/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright © 1999-2010 David Woodhouse <dwmw2@infradead.org> et al.
 *
 */

#ifndef __MTD_MTD_H__
#define __MTD_MTD_H__

#ifndef __UBOOT__
#include <linux/types.h>
#include <linux/uio.h>
#include <linux/notifier.h>
#include <linux/device.h>

#include <mtd/mtd-abi.h>

#include <asm/div64.h>
#else
#include <linux/compat.h>
#include <mtd/mtd-abi.h>
#include <linux/errno.h>
#include <linux/list.h>
#include <div64.h>
#if IS_ENABLED(CONFIG_DM)
#include <dm/device.h>
#endif

#define MAX_MTD_DEVICES 32
#endif

#define MTD_ERASE_PENDING	0x01
#define MTD_ERASING		0x02
#define MTD_ERASE_SUSPEND	0x04
#define MTD_ERASE_DONE		0x08
#define MTD_ERASE_FAILED	0x10

#define MTD_FAIL_ADDR_UNKNOWN -1LL

/*
 * If the erase fails, fail_addr might indicate exactly which block failed. If
 * fail_addr = MTD_FAIL_ADDR_UNKNOWN, the failure was not at the device level
 * or was not specific to any particular block.
 */
struct erase_info {
	struct mtd_info *mtd;
	uint64_t addr;
	uint64_t len;
	uint64_t fail_addr;
	u_long time;
	u_long retries;
	unsigned dev;
	unsigned cell;
	void (*callback) (struct erase_info *self);
	u_long priv;
	u_char state;
	struct erase_info *next;
	int scrub;
};

struct mtd_erase_region_info {
	uint64_t offset;		/* At which this region starts, from the beginning of the MTD */
	uint32_t erasesize;		/* For this region */
	uint32_t numblocks;		/* Number of blocks of erasesize in this region */
	unsigned long *lockmap;		/* If keeping bitmap of locks */
};

/**
 * struct mtd_oob_ops - oob operation operands
 * @mode:	operation mode
 *
 * @len:	number of data bytes to write/read
 *
 * @retlen:	number of data bytes written/read
 *
 * @ooblen:	number of oob bytes to write/read
 * @oobretlen:	number of oob bytes written/read
 * @ooboffs:	offset of oob data in the oob area (only relevant when
 *		mode = MTD_OPS_PLACE_OOB or MTD_OPS_RAW)
 * @datbuf:	data buffer - if NULL only oob data are read/written
 * @oobbuf:	oob data buffer
 */
struct mtd_oob_ops {
	unsigned int	mode;
	size_t		len;
	size_t		retlen;
	size_t		ooblen;
	size_t		oobretlen;
	uint32_t	ooboffs;
	uint8_t		*datbuf;
	uint8_t		*oobbuf;
};

#ifdef CONFIG_SYS_NAND_MAX_OOBFREE
#define MTD_MAX_OOBFREE_ENTRIES_LARGE	CONFIG_SYS_NAND_MAX_OOBFREE
#else
#define MTD_MAX_OOBFREE_ENTRIES_LARGE	32
#endif

#ifdef CONFIG_SYS_NAND_MAX_ECCPOS
#define MTD_MAX_ECCPOS_ENTRIES_LARGE	CONFIG_SYS_NAND_MAX_ECCPOS
#else
#define MTD_MAX_ECCPOS_ENTRIES_LARGE	680
#endif
/**
 * struct mtd_oob_region - oob region definition
 * @offset: region offset
 * @length: region length
 *
 * This structure describes a region of the OOB area, and is used
 * to retrieve ECC or free bytes sections.
 * Each section is defined by an offset within the OOB area and a
 * length.
 */
struct mtd_oob_region {
	u32 offset;
	u32 length;
};

/*
 * struct mtd_ooblayout_ops - NAND OOB layout operations
 * @ecc: function returning an ECC region in the OOB area.
 *	 Should return -ERANGE if %section exceeds the total number of
 *	 ECC sections.
 * @free: function returning a free region in the OOB area.
 *	  Should return -ERANGE if %section exceeds the total number of
 *	  free sections.
 */
struct mtd_ooblayout_ops {
	int (*ecc)(struct mtd_info *mtd, int section,
		   struct mtd_oob_region *oobecc);
	int (*free)(struct mtd_info *mtd, int section,
		    struct mtd_oob_region *oobfree);
};

/*
 * Internal ECC layout control structure. For historical reasons, there is a
 * similar, smaller struct nand_ecclayout_user (in mtd-abi.h) that is retained
 * for export to user-space via the ECCGETLAYOUT ioctl.
 * nand_ecclayout should be expandable in the future simply by the above macros.
 */
struct nand_ecclayout {
	__u32 eccbytes;
	__u32 eccpos[MTD_MAX_ECCPOS_ENTRIES_LARGE];
	__u32 oobavail;
	struct nand_oobfree oobfree[MTD_MAX_OOBFREE_ENTRIES_LARGE];
};

struct module;	/* only needed for owner field in mtd_info */

struct mtd_info {
	u_char type;
	uint32_t flags;
	uint64_t size;	 // Total size of the MTD

	/* "Major" erase size for the device. Naïve users may take this
	 * to be the only erase size available, or may use the more detailed
	 * information below if they desire
	 */
	uint32_t erasesize;
	/* Minimal writable flash unit size. In case of NOR flash it is 1 (even
	 * though individual bits can be cleared), in case of NAND flash it is
	 * one NAND page (or half, or one-fourths of it), in case of ECC-ed NOR
	 * it is of ECC block size, etc. It is illegal to have writesize = 0.
	 * Any driver registering a struct mtd_info must ensure a writesize of
	 * 1 or larger.
	 */
	uint32_t writesize;

	/*
	 * Size of the write buffer used by the MTD. MTD devices having a write
	 * buffer can write multiple writesize chunks at a time. E.g. while
	 * writing 4 * writesize bytes to a device with 2 * writesize bytes
	 * buffer the MTD driver can (but doesn't have to) do 2 writesize
	 * operations, but not 4. Currently, all NANDs have writebufsize
	 * equivalent to writesize (NAND page size). Some NOR flashes do have
	 * writebufsize greater than writesize.
	 */
	uint32_t writebufsize;

	uint32_t oobsize;   // Amount of OOB data per block (e.g. 16)
	uint32_t oobavail;  // Available OOB bytes per block

	/*
	 * If erasesize is a power of 2 then the shift is stored in
	 * erasesize_shift otherwise erasesize_shift is zero. Ditto writesize.
	 */
	unsigned int erasesize_shift;
	unsigned int writesize_shift;
	/* Masks based on erasesize_shift and writesize_shift */
	unsigned int erasesize_mask;
	unsigned int writesize_mask;

	/*
	 * read ops return -EUCLEAN if max number of bitflips corrected on any
	 * one region comprising an ecc step equals or exceeds this value.
	 * Settable by driver, else defaults to ecc_strength.  User can override
	 * in sysfs.  N.B. The meaning of the -EUCLEAN return code has changed;
	 * see Documentation/ABI/testing/sysfs-class-mtd for more detail.
	 */
	unsigned int bitflip_threshold;

	// Kernel-only stuff starts here.
#ifndef __UBOOT__
	const char *name;
#else
	char *name;
#endif
	int index;

	/* OOB layout description */
	const struct mtd_ooblayout_ops *ooblayout;

	/* ECC layout structure pointer - read only! */
	struct nand_ecclayout *ecclayout;

	/* the ecc step size. */
	unsigned int ecc_step_size;

	/* max number of correctible bit errors per ecc step */
	unsigned int ecc_strength;

	/* Data for variable erase regions. If numeraseregions is zero,
	 * it means that the whole device has erasesize as given above.
	 */
	int numeraseregions;
	struct mtd_erase_region_info *eraseregions;

	/*
	 * Do not call via these pointers, use corresponding mtd_*()
	 * wrappers instead.
	 */
	int (*_erase) (struct mtd_info *mtd, struct erase_info *instr);
#ifndef __UBOOT__
	int (*_point) (struct mtd_info *mtd, loff_t from, size_t len,
		       size_t *retlen, void **virt, resource_size_t *phys);
	int (*_unpoint) (struct mtd_info *mtd, loff_t from, size_t len);
#endif
	unsigned long (*_get_unmapped_area) (struct mtd_info *mtd,
					     unsigned long len,
					     unsigned long offset,
					     unsigned long flags);
	int (*_read) (struct mtd_info *mtd, loff_t from, size_t len,
		      size_t *retlen, u_char *buf);
	int (*_write) (struct mtd_info *mtd, loff_t to, size_t len,
		       size_t *retlen, const u_char *buf);
	int (*_panic_write) (struct mtd_info *mtd, loff_t to, size_t len,
			     size_t *retlen, const u_char *buf);
	int (*_read_oob) (struct mtd_info *mtd, loff_t from,
			  struct mtd_oob_ops *ops);
	int (*_write_oob) (struct mtd_info *mtd, loff_t to,
			   struct mtd_oob_ops *ops);
	int (*_get_fact_prot_info) (struct mtd_info *mtd, size_t len,
				    size_t *retlen, struct otp_info *buf);
	int (*_read_fact_prot_reg) (struct mtd_info *mtd, loff_t from,
				    size_t len, size_t *retlen, u_char *buf);
	int (*_get_user_prot_info) (struct mtd_info *mtd, size_t len,
				    size_t *retlen, struct otp_info *buf);
	int (*_read_user_prot_reg) (struct mtd_info *mtd, loff_t from,
				    size_t len, size_t *retlen, u_char *buf);
	int (*_write_user_prot_reg) (struct mtd_info *mtd, loff_t to,
				     size_t len, size_t *retlen, u_char *buf);
	int (*_lock_user_prot_reg) (struct mtd_info *mtd, loff_t from,
				    size_t len);
#ifndef __UBOOT__
	int (*_writev) (struct mtd_info *mtd, const struct kvec *vecs,
			unsigned long count, loff_t to, size_t *retlen);
#endif
	void (*_sync) (struct mtd_info *mtd);
	int (*_lock) (struct mtd_info *mtd, loff_t ofs, uint64_t len);
	int (*_unlock) (struct mtd_info *mtd, loff_t ofs, uint64_t len);
	int (*_is_locked) (struct mtd_info *mtd, loff_t ofs, uint64_t len);
	int (*_block_isreserved) (struct mtd_info *mtd, loff_t ofs);
	int (*_block_isbad) (struct mtd_info *mtd, loff_t ofs);
	int (*_block_markbad) (struct mtd_info *mtd, loff_t ofs);
#ifndef __UBOOT__
	int (*_suspend) (struct mtd_info *mtd);
	void (*_resume) (struct mtd_info *mtd);
	void (*_reboot) (struct mtd_info *mtd);
#endif
	/*
	 * If the driver is something smart, like UBI, it may need to maintain
	 * its own reference counting. The below functions are only for driver.
	 */
	int (*_get_device) (struct mtd_info *mtd);
	void (*_put_device) (struct mtd_info *mtd);

#ifndef __UBOOT__
	/* Backing device capabilities for this device
	 * - provides mmap capabilities
	 */
	struct backing_dev_info *backing_dev_info;

	struct notifier_block reboot_notifier;  /* default mode before reboot */
#endif

	/* ECC status information */
	struct mtd_ecc_stats ecc_stats;
	/* Subpage shift (NAND) */
	int subpage_sft;

	void *priv;

	struct module *owner;
#ifndef __UBOOT__
	struct device dev;
#else
	struct udevice *dev;
#endif
	int usecount;

	/* MTD devices do not have any parent. MTD partitions do. */
	struct mtd_info *parent;

	/*
	 * Offset of the partition relatively to the parent offset.
	 * Is 0 for real MTD devices (ie. not partitions).
	 */
	u64 offset;

	/*
	 * List node used to add an MTD partition to the parent
	 * partition list.
	 */
	struct list_head node;

	/*
	 * List of partitions attached to this MTD device (the parent
	 * MTD device can itself be a partition).
	 */
	struct list_head partitions;
};

#if IS_ENABLED(CONFIG_DM)
static inline void mtd_set_of_node(struct mtd_info *mtd,
				   const struct device_node *np)
{
	mtd->dev->node.np = np;
}

static inline const struct device_node *mtd_get_of_node(struct mtd_info *mtd)
{
	return mtd->dev->node.np;
}
#else
struct device_node;

static inline void mtd_set_of_node(struct mtd_info *mtd,
				   const struct device_node *np)
{
}

static inline const struct device_node *mtd_get_of_node(struct mtd_info *mtd)
{
	return NULL;
}
#endif

static inline bool mtd_is_partition(const struct mtd_info *mtd)
{
	return mtd->parent;
}

static inline bool mtd_has_partitions(const struct mtd_info *mtd)
{
	return !list_empty(&mtd->partitions);
}

bool mtd_partitions_used(struct mtd_info *master);

int mtd_ooblayout_ecc(struct mtd_info *mtd, int section,
		      struct mtd_oob_region *oobecc);
int mtd_ooblayout_find_eccregion(struct mtd_info *mtd, int eccbyte,
				 int *section,
				 struct mtd_oob_region *oobregion);
int mtd_ooblayout_get_eccbytes(struct mtd_info *mtd, u8 *eccbuf,
			       const u8 *oobbuf, int start, int nbytes);
int mtd_ooblayout_set_eccbytes(struct mtd_info *mtd, const u8 *eccbuf,
			       u8 *oobbuf, int start, int nbytes);
int mtd_ooblayout_free(struct mtd_info *mtd, int section,
		       struct mtd_oob_region *oobfree);
int mtd_ooblayout_get_databytes(struct mtd_info *mtd, u8 *databuf,
				const u8 *oobbuf, int start, int nbytes);
int mtd_ooblayout_set_databytes(struct mtd_info *mtd, const u8 *databuf,
				u8 *oobbuf, int start, int nbytes);
int mtd_ooblayout_count_freebytes(struct mtd_info *mtd);
int mtd_ooblayout_count_eccbytes(struct mtd_info *mtd);

static inline void mtd_set_ooblayout(struct mtd_info *mtd,
				     const struct mtd_ooblayout_ops *ooblayout)
{
	mtd->ooblayout = ooblayout;
}

static inline u32 mtd_oobavail(struct mtd_info *mtd, struct mtd_oob_ops *ops)
{
	return ops->mode == MTD_OPS_AUTO_OOB ? mtd->oobavail : mtd->oobsize;
}

int mtd_erase(struct mtd_info *mtd, struct erase_info *instr);
#ifndef __UBOOT__
int mtd_point(struct mtd_info *mtd, loff_t from, size_t len, size_t *retlen,
	      void **virt, resource_size_t *phys);
int mtd_unpoint(struct mtd_info *mtd, loff_t from, size_t len);
#endif
unsigned long mtd_get_unmapped_area(struct mtd_info *mtd, unsigned long len,
				    unsigned long offset, unsigned long flags);
int mtd_read(struct mtd_info *mtd, loff_t from, size_t len, size_t *retlen,
	     u_char *buf);
int mtd_write(struct mtd_info *mtd, loff_t to, size_t len, size_t *retlen,
	      const u_char *buf);
int mtd_panic_write(struct mtd_info *mtd, loff_t to, size_t len, size_t *retlen,
		    const u_char *buf);

int mtd_read_oob(struct mtd_info *mtd, loff_t from, struct mtd_oob_ops *ops);
int mtd_write_oob(struct mtd_info *mtd, loff_t to, struct mtd_oob_ops *ops);

int mtd_get_fact_prot_info(struct mtd_info *mtd, size_t len, size_t *retlen,
			   struct otp_info *buf);
int mtd_read_fact_prot_reg(struct mtd_info *mtd, loff_t from, size_t len,
			   size_t *retlen, u_char *buf);
int mtd_get_user_prot_info(struct mtd_info *mtd, size_t len, size_t *retlen,
			   struct otp_info *buf);
int mtd_read_user_prot_reg(struct mtd_info *mtd, loff_t from, size_t len,
			   size_t *retlen, u_char *buf);
int mtd_write_user_prot_reg(struct mtd_info *mtd, loff_t to, size_t len,
			    size_t *retlen, u_char *buf);
int mtd_lock_user_prot_reg(struct mtd_info *mtd, loff_t from, size_t len);

#ifndef __UBOOT__
int mtd_writev(struct mtd_info *mtd, const struct kvec *vecs,
	       unsigned long count, loff_t to, size_t *retlen);
#endif

static inline void mtd_sync(struct mtd_info *mtd)
{
	if (mtd->_sync)
		mtd->_sync(mtd);
}

int mtd_lock(struct mtd_info *mtd, loff_t ofs, uint64_t len);
int mtd_unlock(struct mtd_info *mtd, loff_t ofs, uint64_t len);
int mtd_is_locked(struct mtd_info *mtd, loff_t ofs, uint64_t len);
int mtd_block_isreserved(struct mtd_info *mtd, loff_t ofs);
int mtd_block_isbad(struct mtd_info *mtd, loff_t ofs);
int mtd_block_markbad(struct mtd_info *mtd, loff_t ofs);

#ifndef __UBOOT__
static inline int mtd_suspend(struct mtd_info *mtd)
{
	return mtd->_suspend ? mtd->_suspend(mtd) : 0;
}

static inline void mtd_resume(struct mtd_info *mtd)
{
	if (mtd->_resume)
		mtd->_resume(mtd);
}
#endif

static inline uint32_t mtd_div_by_eb(uint64_t sz, struct mtd_info *mtd)
{
	if (mtd->erasesize_shift)
		return sz >> mtd->erasesize_shift;
	do_div(sz, mtd->erasesize);
	return sz;
}

static inline uint32_t mtd_mod_by_eb(uint64_t sz, struct mtd_info *mtd)
{
	if (mtd->erasesize_shift)
		return sz & mtd->erasesize_mask;
	return do_div(sz, mtd->erasesize);
}

static inline uint32_t mtd_div_by_ws(uint64_t sz, struct mtd_info *mtd)
{
	if (mtd->writesize_shift)
		return sz >> mtd->writesize_shift;
	do_div(sz, mtd->writesize);
	return sz;
}

static inline uint32_t mtd_mod_by_ws(uint64_t sz, struct mtd_info *mtd)
{
	if (mtd->writesize_shift)
		return sz & mtd->writesize_mask;
	return do_div(sz, mtd->writesize);
}

static inline int mtd_has_oob(const struct mtd_info *mtd)
{
	return mtd->_read_oob && mtd->_write_oob;
}

static inline int mtd_type_is_nand(const struct mtd_info *mtd)
{
	return mtd->type == MTD_NANDFLASH || mtd->type == MTD_MLCNANDFLASH;
}

static inline int mtd_can_have_bb(const struct mtd_info *mtd)
{
	return !!mtd->_block_isbad;
}

	/* Kernel-side ioctl definitions */

struct mtd_partition;
struct mtd_part_parser_data;

extern int mtd_device_parse_register(struct mtd_info *mtd,
				     const char * const *part_probe_types,
				     struct mtd_part_parser_data *parser_data,
				     const struct mtd_partition *defparts,
				     int defnr_parts);
#define mtd_device_register(master, parts, nr_parts)	\
	mtd_device_parse_register(master, NULL, NULL, parts, nr_parts)
extern int mtd_device_unregister(struct mtd_info *master);
extern struct mtd_info *get_mtd_device(struct mtd_info *mtd, int num);
extern int __get_mtd_device(struct mtd_info *mtd);
extern void __put_mtd_device(struct mtd_info *mtd);
extern struct mtd_info *get_mtd_device_nm(const char *name);
extern void put_mtd_device(struct mtd_info *mtd);


#ifndef __UBOOT__
struct mtd_notifier {
	void (*add)(struct mtd_info *mtd);
	void (*remove)(struct mtd_info *mtd);
	struct list_head list;
};


extern void register_mtd_user (struct mtd_notifier *new);
extern int unregister_mtd_user (struct mtd_notifier *old);
#endif
void *mtd_kmalloc_up_to(const struct mtd_info *mtd, size_t *size);

#ifdef CONFIG_MTD_PARTITIONS
void mtd_erase_callback(struct erase_info *instr);
#else
static inline void mtd_erase_callback(struct erase_info *instr)
{
	if (instr->callback)
		instr->callback(instr);
}
#endif

static inline int mtd_is_bitflip(int err) {
	return err == -EUCLEAN;
}

static inline int mtd_is_eccerr(int err) {
	return err == -EBADMSG;
}

static inline int mtd_is_bitflip_or_eccerr(int err) {
	return mtd_is_bitflip(err) || mtd_is_eccerr(err);
}

unsigned mtd_mmap_capabilities(struct mtd_info *mtd);

#ifdef __UBOOT__
/* drivers/mtd/mtdcore.h */
int add_mtd_device(struct mtd_info *mtd);
int del_mtd_device(struct mtd_info *mtd);

#ifdef CONFIG_MTD_PARTITIONS
int add_mtd_partitions(struct mtd_info *, const struct mtd_partition *, int);
int del_mtd_partitions(struct mtd_info *);
#else
static inline int add_mtd_partitions(struct mtd_info *mtd,
				     const struct mtd_partition *parts,
				     int nparts)
{
	return 0;
}

static inline int del_mtd_partitions(struct mtd_info *mtd)
{
	return 0;
}
#endif

struct mtd_info *__mtd_next_device(int i);
#define mtd_for_each_device(mtd)			\
	for ((mtd) = __mtd_next_device(0);		\
	     (mtd) != NULL;				\
	     (mtd) = __mtd_next_device(mtd->index + 1))

int mtd_arg_off(const char *arg, int *idx, loff_t *off, loff_t *size,
		loff_t *maxsize, int devtype, uint64_t chipsize);
int mtd_arg_off_size(int argc, char *const argv[], int *idx, loff_t *off,
		     loff_t *size, loff_t *maxsize, int devtype,
		     uint64_t chipsize);

/* drivers/mtd/mtdcore.c */
void mtd_get_len_incl_bad(struct mtd_info *mtd, uint64_t offset,
			  const uint64_t length, uint64_t *len_incl_bad,
			  int *truncated);
bool mtd_dev_list_updated(void);

/* drivers/mtd/mtd_uboot.c */
int mtd_search_alternate_name(const char *mtdname, char *altname,
			      unsigned int max_len);

#endif
#endif /* __MTD_MTD_H__ */
