/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * dfu.h - DFU flashable area description
 *
 * Copyright (C) 2012 Samsung Electronics
 * authors: Andrzej Pietrasiewicz <andrzej.p@samsung.com>
 *	    Lukasz Majewski <l.majewski@samsung.com>
 */

#ifndef __DFU_ENTITY_H_
#define __DFU_ENTITY_H_

#include <linux/errno.h>
#include <linux/list.h>
#include <mmc.h>
#include <spi_flash.h>
#include <linux/usb/composite.h>

enum dfu_device_type {
	DFU_DEV_MMC = 1,
	DFU_DEV_ONENAND,
	DFU_DEV_NAND,
	DFU_DEV_RAM,
	DFU_DEV_SF,
	DFU_DEV_MTD,
	DFU_DEV_VIRT,
};

enum dfu_layout {
	DFU_RAW_ADDR = 1,
	DFU_FS_FAT,
	DFU_FS_EXT2,
	DFU_FS_EXT3,
	DFU_FS_EXT4,
	DFU_RAM_ADDR,
	DFU_SKIP,
	DFU_SCRIPT,
};

enum dfu_op {
	DFU_OP_READ = 1,
	DFU_OP_WRITE,
	DFU_OP_SIZE,
};

struct mmc_internal_data {
	int dev_num;

	/* RAW programming */
	unsigned int lba_start;
	unsigned int lba_size;
	unsigned int lba_blk_size;

	/* eMMC HW partition access */
	int hw_partition;

	/* FAT/EXT */
	unsigned int dev;
	unsigned int part;
};

struct mtd_internal_data {
	struct mtd_info *info;

	/* RAW programming */
	u64 start;
	u64 size;
	/* for ubi partition */
	unsigned int ubi;
};

struct nand_internal_data {
	/* RAW programming */
	u64 start;
	u64 size;

	unsigned int dev;
	unsigned int part;
	/* for nand/ubi use */
	unsigned int ubi;
};

struct ram_internal_data {
	unsigned long	start;
	unsigned int	size;
};

struct sf_internal_data {
	struct spi_flash *dev;

	/* RAW programming */
	u64 start;
	u64 size;
	/* for sf/ubi use */
	unsigned int ubi;
};

struct virt_internal_data {
	int dev_num;
};


#if defined(CONFIG_DFU_NAME_MAX_SIZE)
#define DFU_NAME_SIZE			CONFIG_DFU_NAME_MAX_SIZE
#else
#define DFU_NAME_SIZE			32
#endif
#ifndef DFU_DEFAULT_POLL_TIMEOUT
#define DFU_DEFAULT_POLL_TIMEOUT 0
#endif
#ifndef DFU_MANIFEST_POLL_TIMEOUT
#define DFU_MANIFEST_POLL_TIMEOUT	DFU_DEFAULT_POLL_TIMEOUT
#endif

struct dfu_entity {
	char			name[DFU_NAME_SIZE];
	int                     alt;
	void                    *dev_private;
	enum dfu_device_type    dev_type;
	enum dfu_layout         layout;
	unsigned long           max_buf_size;

	union {
		struct mmc_internal_data mmc;
		struct mtd_internal_data mtd;
		struct nand_internal_data nand;
		struct ram_internal_data ram;
		struct sf_internal_data sf;
		struct virt_internal_data virt;
	} data;

	int (*get_medium_size)(struct dfu_entity *dfu, u64 *size);

	int (*read_medium)(struct dfu_entity *dfu,
			u64 offset, void *buf, long *len);

	int (*write_medium)(struct dfu_entity *dfu,
			u64 offset, void *buf, long *len);

	int (*flush_medium)(struct dfu_entity *dfu);
	unsigned int (*poll_timeout)(struct dfu_entity *dfu);

	void (*free_entity)(struct dfu_entity *dfu);

	struct list_head list;

	/* on the fly state */
	u32 crc;
	u64 offset;
	int i_blk_seq_num;
	u8 *i_buf;
	u8 *i_buf_start;
	u8 *i_buf_end;
	u64 r_left;
	long b_left;

	u32 bad_skip;	/* for nand use */

	unsigned int inited:1;
};

struct list_head;
extern struct list_head dfu_list;

#ifdef CONFIG_SET_DFU_ALT_INFO
/**
 * set_dfu_alt_info() - set dfu_alt_info environment variable
 *
 * If CONFIG_SET_DFU_ALT_INFO=y, this board specific function is called to set
 * environment variable dfu_alt_info.
 *
 * @interface:	dfu interface, e.g. "mmc" or "nand"
 * @devstr:	device number as string
 */
void set_dfu_alt_info(char *interface, char *devstr);
#endif

/**
 * dfu_alt_init() - initialize buffer for dfu entities
 *
 * @num:	number of entities
 * @dfu:	on return allocated buffer
 * Return:	0 on success
 */
int dfu_alt_init(int num, struct dfu_entity **dfu);

/**
 * dfu_alt_add() - add alternate to dfu entity buffer
 *
 * @dfu:	dfu entity
 * @interface:	dfu interface, e.g. "mmc" or "nand"
 * @devstr:	device number as string
 * @s:		string description of alternate
 * Return:	0 on success
 */
int dfu_alt_add(struct dfu_entity *dfu, char *interface, char *devstr, char *s);

/**
 * dfu_config_entities() - initialize dfu entitities from envirionment
 *
 * Initialize the list of dfu entities from environment variable dfu_alt_info.
 * The list must be freed by calling dfu_free_entities(). This function bypasses
 * set_dfu_alt_info(). So typically you should use dfu_init_env_entities()
 * instead.
 *
 * See function :c:func:`dfu_free_entities`
 * See function :c:func:`dfu_init_env_entities`
 *
 * @s:		string with alternates
 * @interface:	interface, e.g. "mmc" or "nand"
 * @devstr:	device number as string
 * Return:	0 on success, a negative error code otherwise
 */
int dfu_config_entities(char *s, char *interface, char *devstr);

/**
 * dfu_free_entities() - free the list of dfu entities
 *
 * Free the internal list of dfu entities.
 *
 * See function :c:func:`dfu_init_env_entities`
 */
void dfu_free_entities(void);

/**
 * dfu_show_entities() - print DFU alt settings list
 */
void dfu_show_entities(void);

/**
 * dfu_get_alt_number() - get number of alternates
 *
 * Return: number of alternates in the dfu entities list
 */
int dfu_get_alt_number(void);

/**
 * dfu_get_dev_type() - get string representation for dfu device type
 *
 * @type:	device type
 * Return:	string representation for device type
 */
const char *dfu_get_dev_type(enum dfu_device_type type);

/**
 * dfu_get_layout() - get string describing layout
 *
 * Internally layouts are represented by enum dfu_device_type values. This
 * function translates an enum value to a human readable string, e.g. DFU_FS_FAT
 * is translated to "FAT".
 *
 * @layout:	layout
 * Result:	string representation for the layout
 */
const char *dfu_get_layout(enum dfu_layout layout);

/**
 * dfu_get_entity() - get dfu entity for an alternate id
 *
 * @alt:	alternate id
 * Return:	dfu entity
 */
struct dfu_entity *dfu_get_entity(int alt);

char *dfu_extract_token(char** e, int *n);

/**
 * dfu_get_alt() - get alternate id for filename
 *
 * Environment variable dfu_alt_info defines the write destinations (alternates)
 * for different filenames. This function get the index of the alternate for
 * a filename. If an absolute filename is provided (starting with '/'), the
 * directory path is ignored.
 *
 * @name:	filename
 * Return:	id of the alternate or negative error number (-ENODEV)
 */
int dfu_get_alt(char *name);

/**
 * dfu_init_env_entities() - initialize dfu entitities from envirionment
 *
 * Initialize the list of dfu entities from environment variable dfu_alt_info.
 * The list must be freed by calling dfu_free_entities().
 * @interface and @devstr are used to select the relevant set of alternates
 * from environment variable dfu_alt_info.
 *
 * If environment variable dfu_alt_info specifies the interface and the device,
 * use NULL for @interface and @devstr.
 *
 * See function :c:func:`dfu_free_entities`
 *
 * @interface:	interface, e.g. "mmc" or "nand"
 * @devstr:	device number as string
 * Return:	0 on success, a negative error code otherwise
 */
int dfu_init_env_entities(char *interface, char *devstr);

unsigned char *dfu_get_buf(struct dfu_entity *dfu);
unsigned char *dfu_free_buf(void);
unsigned long dfu_get_buf_size(void);
bool dfu_usb_get_reset(void);

#ifdef CONFIG_DFU_TIMEOUT
unsigned long dfu_get_timeout(void);
void dfu_set_timeout(unsigned long);
#endif

/**
 * dfu_read() - read from dfu entity
 *
 * The block sequence number @blk_seq_num is a 16 bit counter that must be
 * incremented with each call for the same dfu entity @de.
 *
 * @de:			dfu entity
 * @buf:		buffer
 * @size:		size of buffer
 * @blk_seq_num:	block sequence number
 * Return:		0 for success, -1 for error
 */
int dfu_read(struct dfu_entity *de, void *buf, int size, int blk_seq_num);

/**
 * dfu_write() - write to dfu entity
 *
 * Write the contents of a buffer @buf to the dfu entity @de. After writing
 * the last block call dfu_flush(). If a file is already loaded completely
 * into memory it is preferable to use dfu_write_from_mem_addr() which takes
 * care of blockwise transfer and flushing.
 *
 * The block sequence number @blk_seq_num is a 16 bit counter that must be
 * incremented with each call for the same dfu entity @de.
 *
 * See function :c:func:`dfu_flush`
 * See function :c:func:`dfu_write_from_mem_addr`
 *
 * @de:			dfu entity
 * @buf:		buffer
 * @size:		size of buffer
 * @blk_seq_num:	block sequence number
 * Return:		0 for success, -1 for error
 */
int dfu_write(struct dfu_entity *de, void *buf, int size, int blk_seq_num);

/**
 * dfu_flush() - flush to dfu entity
 *
 * This function has to be called after writing the last block to the dfu
 * entity @de.
 *
 * The block sequence number @blk_seq_num is a 16 bit counter that must be
 * incremented with each call for the same dfu entity @de.
 *
 * See function :c:func:`dfu_write`
 *
 * @de:			dfu entity
 * @buf:		ignored
 * @size:		ignored
 * @blk_seq_num:	block sequence number of last write - ignored
 * Return:		0 for success, -1 for error
 */
int dfu_flush(struct dfu_entity *de, void *buf, int size, int blk_seq_num);

/**
 * dfu_initiated_callback() - weak callback called on DFU transaction start
 *
 * It is a callback function called by DFU stack when a DFU transaction is
 * initiated. This function allows to manage some board specific behavior on
 * DFU targets.
 *
 * @dfu:	pointer to the dfu_entity, which should be initialized
 */
void dfu_initiated_callback(struct dfu_entity *dfu);

/**
 * dfu_flush_callback() - weak callback called at the end of the DFU write
 *
 * It is a callback function called by DFU stack after DFU manifestation.
 * This function allows to manage some board specific behavior on DFU targets
 *
 * @dfu:	pointer to the dfu_entity, which should be flushed
 */
void dfu_flush_callback(struct dfu_entity *dfu);

/**
 * dfu_error_callback() - weak callback called at the DFU write error
 *
 * It is a callback function called by DFU stack after DFU write error.
 * This function allows to manage some board specific behavior on DFU targets
 *
 * @dfu:	pointer to the dfu_entity which cause the error
 * @msg:	the message of the error
 */
void dfu_error_callback(struct dfu_entity *dfu, const char *msg);

int dfu_transaction_initiate(struct dfu_entity *dfu, bool read);
void dfu_transaction_cleanup(struct dfu_entity *dfu);

/*
 * dfu_defer_flush - pointer to store dfu_entity for deferred flashing.
 *		     It should be NULL when not used.
 */
extern struct dfu_entity *dfu_defer_flush;

/**
 * dfu_get_defer_flush() - get current value of dfu_defer_flush pointer
 *
 * Return:	value of the dfu_defer_flush pointer
 */
static inline struct dfu_entity *dfu_get_defer_flush(void)
{
	return dfu_defer_flush;
}

/**
 * dfu_set_defer_flush() - set the dfu_defer_flush pointer
 *
 * @dfu:	pointer to the dfu_entity, which should be written
 */
static inline void dfu_set_defer_flush(struct dfu_entity *dfu)
{
	dfu_defer_flush = dfu;
}

/**
 * dfu_write_from_mem_addr() - write data from memory to DFU managed medium
 *
 * This function adds support for writing data starting from fixed memory
 * address (like $loadaddr) to dfu managed medium (e.g. NAND, MMC, file system)
 *
 * @dfu:	dfu entity to which we want to store data
 * @buf:	fixed memory address from where data starts
 * @size:	number of bytes to write
 *
 * Return:	0 on success, other value on failure
 */
int dfu_write_from_mem_addr(struct dfu_entity *dfu, void *buf, int size);

/* Device specific */
/* Each entity has 5 arguments in maximum. */
#define DFU_MAX_ENTITY_ARGS	5

#if CONFIG_IS_ENABLED(DFU_MMC)
extern int dfu_fill_entity_mmc(struct dfu_entity *dfu, char *devstr,
			       char **argv, int argc);
#else
static inline int dfu_fill_entity_mmc(struct dfu_entity *dfu, char *devstr,
				      char **argv, int argc)
{
	puts("MMC support not available!\n");
	return -1;
}
#endif

#if CONFIG_IS_ENABLED(DFU_NAND)
extern int dfu_fill_entity_nand(struct dfu_entity *dfu, char *devstr,
				char **argv, int argc);
#else
static inline int dfu_fill_entity_nand(struct dfu_entity *dfu, char *devstr,
				       char **argv, int argc)
{
	puts("NAND support not available!\n");
	return -1;
}
#endif

#if CONFIG_IS_ENABLED(DFU_RAM)
extern int dfu_fill_entity_ram(struct dfu_entity *dfu, char *devstr,
			       char **argv, int argc);
#else
static inline int dfu_fill_entity_ram(struct dfu_entity *dfu, char *devstr,
				      char **argv, int argc)
{
	puts("RAM support not available!\n");
	return -1;
}
#endif

#if CONFIG_IS_ENABLED(DFU_SF)
extern int dfu_fill_entity_sf(struct dfu_entity *dfu, char *devstr,
			      char **argv, int argc);
#else
static inline int dfu_fill_entity_sf(struct dfu_entity *dfu, char *devstr,
				     char **argv, int argc)
{
	puts("SF support not available!\n");
	return -1;
}
#endif

#if CONFIG_IS_ENABLED(DFU_MTD)
extern int dfu_fill_entity_mtd(struct dfu_entity *dfu, char *devstr,
			       char **argv, int argc);
#else
static inline int dfu_fill_entity_mtd(struct dfu_entity *dfu, char *devstr,
				      char **argv, int argc)
{
	puts("MTD support not available!\n");
	return -1;
}
#endif

#if CONFIG_IS_ENABLED(DFU_VIRT)
int dfu_fill_entity_virt(struct dfu_entity *dfu, char *devstr,
			 char **argv, int argc);
int dfu_write_medium_virt(struct dfu_entity *dfu, u64 offset,
			  void *buf, long *len);
int dfu_get_medium_size_virt(struct dfu_entity *dfu, u64 *size);
int dfu_read_medium_virt(struct dfu_entity *dfu, u64 offset,
			 void *buf, long *len);
#else
static inline int dfu_fill_entity_virt(struct dfu_entity *dfu, char *devstr,
				       char **argv, int argc)
{
	puts("VIRT support not available!\n");
	return -1;
}
#endif

extern bool dfu_reinit_needed;

#if CONFIG_IS_ENABLED(DFU_WRITE_ALT)
/**
 * dfu_write_by_name() - write data to DFU medium
 * @dfu_entity_name:	Name of DFU entity to write
 * @addr:		Address of data buffer to write
 * @len:		Number of bytes
 * @interface:		Destination DFU medium (e.g. "mmc")
 * @devstring:		Instance number of destination DFU medium (e.g. "1")
 *
 * This function is storing data received on DFU supported medium which
 * is specified by @dfu_entity_name.
 *
 * Return:		0 - on success, error code - otherwise
 */
int dfu_write_by_name(char *dfu_entity_name, void *addr,
		      unsigned int len, char *interface, char *devstring);

/**
 * dfu_write_by_alt() - write data to DFU medium
 * @dfu_alt_num:	DFU alt setting number
 * @addr:		Address of data buffer to write
 * @len:		Number of bytes
 * @interface:		Destination DFU medium (e.g. "mmc")
 * @devstring:		Instance number of destination DFU medium (e.g. "1")
 *
 * This function is storing data received on DFU supported medium which
 * is specified by @dfu_alt_name.
 *
 * Return:		0 - on success, error code - otherwise
 */
int dfu_write_by_alt(int dfu_alt_num, void *addr, unsigned int len,
		     char *interface, char *devstring);
#else
static inline int dfu_write_by_name(char *dfu_entity_name, void *addr,
				    unsigned int len, char *interface,
				    char *devstring)
{
	puts("write support for DFU not available!\n");
	return -ENOSYS;
}

static inline int dfu_write_by_alt(int dfu_alt_num, void *addr,
				   unsigned int len, char *interface,
				   char *devstring)
{
	puts("write support for DFU not available!\n");
	return -ENOSYS;
}
#endif

int dfu_add(struct usb_configuration *c);
#endif /* __DFU_ENTITY_H_ */
