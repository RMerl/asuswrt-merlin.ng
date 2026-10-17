// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) Sean Anderson <seanga2@gmail.com>
 */

#define LOG_CATEGORY UCLASS_MTD
#include <errno.h>
#include <hexdump.h>
#include <log.h>
#include <nand.h>
#include <os.h>
#include <rand.h>
#include <spl.h>
#include <system-constants.h>
#include <dm/device_compat.h>
#include <dm/read.h>
#include <dm/uclass.h>
#include <asm/bitops.h>
#include <linux/bitmap.h>
#include <linux/mtd/rawnand.h>
#include <linux/sizes.h>

enum sand_nand_state {
	STATE_READY,
	STATE_IDLE,
	STATE_READ,
	STATE_READ_ID,
	STATE_READ_ONFI,
	STATE_PARAM_ONFI,
	STATE_STATUS,
	STATE_PROG,
	STATE_ERASE,
};

static const char *const state_name[] = {
	[STATE_READY] = "READY",
	[STATE_IDLE] = "IDLE",
	[STATE_READ] = "READ",
	[STATE_READ_ID] = "READ_ID",
	[STATE_READ_ONFI] = "READ_ONFI",
	[STATE_PARAM_ONFI] = "PARAM_ONFI",
	[STATE_STATUS] = "STATUS",
	[STATE_PROG] = "PROG",
	[STATE_ERASE] = "ERASE",
};

/**
 * struct sand_nand_chip - Per-device private data
 * @nand: The nand chip
 * @node: The next device in this controller
 * @programmed: Bitmap of whether sectors are programmed
 * @id: ID to report for NAND_CMD_READID
 * @id_len: Length of @id
 * @onfi: Three copies of ONFI parameter page
 * @status: Status to report for NAND_CMD_STATUS
 * @chunksize: Size of one "chunk" (page + oob) in bytes
 * @pageize: Size of one page in bytes
 * @pages: Total number of pages
 * @pages_per_erase: Number of pages per eraseblock
 * @err_count: Number of errors to inject per @err_step_bits of data
 * @err_step_bits: Number of data bits per error "step"
 * @err_steps: Number of err steps in a page
 * @cs: Chip select for this device
 * @state: Current state of the device
 * @column: Column of the most-recent command
 * @page_addr: Page address of the most-recent command
 * @fd: File descriptor for the backing data
 * @fd_page_addr: Page address that @fd is seek'd to
 * @selected: Whether this device is selected
 * @tmp: "Cache" buffer used to store transferred data before committing it
 * @tmp_dirty: Whether @tmp is dirty (modified) or clean (all ones)
 *
 * Data is stored with the OOB area in-line. For example, with 512-byte pages
 * and and 16-byte OOB areas, the first page would start at offset 0, the second
 * at offset 528, the third at offset 1056, and so on
 */
struct sand_nand_chip {
	struct nand_chip nand;
	struct list_head node;
	long *programmed;
	const u8 *id;
	u32 chunksize, pagesize, pages, pages_per_erase;
	u32 err_count, err_step_bits, err_steps, ecc_bits;
	unsigned int cs;
	enum sand_nand_state state;
	int column, page_addr, fd, fd_page_addr;
	bool selected, tmp_dirty;
	u8 status;
	u8 id_len;
	u8 tmp[NAND_MAX_PAGESIZE + NAND_MAX_OOBSIZE];
	u8 onfi[sizeof(struct nand_onfi_params) * 3];
};

#define SAND_DEBUG(chip, fmt, ...) \
	dev_dbg((chip)->nand.mtd.dev, "%u (%s): " fmt, (chip)->cs, \
		state_name[(chip)->state], ##__VA_ARGS__)

static inline void to_state(struct sand_nand_chip *chip,
			    enum sand_nand_state new_state)
{
	if (new_state != chip->state)
		SAND_DEBUG(chip, "to state %s\n", state_name[new_state]);
	chip->state = new_state;
}

static inline struct sand_nand_chip *to_sand_nand(struct nand_chip *nand)
{
	return container_of(nand, struct sand_nand_chip, nand);
}

struct sand_nand_priv {
	struct list_head chips;
};

static int sand_nand_dev_ready(struct mtd_info *mtd)
{
	return 1;
}

static int sand_nand_wait(struct mtd_info *mtd, struct nand_chip *chip)
{
	u8 status;

	return nand_status_op(chip, &status) ?: status;
}

static int sand_nand_seek(struct sand_nand_chip *chip)
{
	if (chip->fd_page_addr == chip->page_addr)
		return 0;

	if (os_lseek(chip->fd, (off_t)chip->page_addr * chip->chunksize,
		     OS_SEEK_SET) < 0) {
		SAND_DEBUG(chip, "could not seek: %d\n", errno);
		return -EIO;
	}

	chip->fd_page_addr = chip->page_addr;
	return 0;
}

static void sand_nand_inject_error(struct sand_nand_chip *chip,
				   unsigned int step, unsigned int pos)
{
	int byte, index;

	if (pos < chip->err_step_bits) {
		__change_bit(step * chip->err_step_bits + pos, chip->tmp);
		return;
	}

	/*
	 * Only ECC bytes are covered in the OOB area, so
	 * pretend that those are the only bytes which can have
	 * errors.
	 */
	byte = (pos - chip->err_step_bits + step * chip->ecc_bits) / 8;
	index = chip->nand.ecc.layout->eccpos[byte];
	/* Avoid endianness issues by working with bytes */
	chip->tmp[chip->pagesize + index] ^= BIT(pos & 0x7);
}

static int sand_nand_read(struct sand_nand_chip *chip)
{
	unsigned int i, stop = 0;

	if (chip->column == chip->pagesize)
		stop = chip->err_step_bits;

	if (test_bit(chip->page_addr, chip->programmed)) {
		if (sand_nand_seek(chip))
			return -EIO;

		if (os_read(chip->fd, chip->tmp, chip->chunksize) !=
		    chip->chunksize) {
			SAND_DEBUG(chip, "could not read: %d\n", errno);
			return -EIO;
		}
		chip->fd_page_addr++;
	} else if (chip->tmp_dirty) {
		memset(chip->tmp + chip->column, 0xff,
		       chip->chunksize - chip->column);
	}

	/*
	 * Inject some errors; this is Method A from "An Efficient Algorithm for
	 * Sequential Random Sampling" (Vitter 87). This is still slow when
	 * generating a lot (dozens) of ECC errors.
	 *
	 * To avoid generating too many errors in any one ECC step, we separate
	 * our error generation by ECC step.
	 */
	chip->tmp_dirty = true;
	for (i = 0; i < chip->err_steps; i++) {
		u32 bit_errors = chip->err_count;
		unsigned int j = chip->err_step_bits + chip->ecc_bits;

		while (bit_errors) {
			unsigned int u = rand();
			float quot = 1ULL << 32;

			do {
				quot *= j - bit_errors;
				quot /= j;
				j--;

				if (j < stop)
					goto next;
			} while (u < quot);

			sand_nand_inject_error(chip, i, j);
			bit_errors--;
		}
next:
		;
	}

	return 0;
}

static void sand_nand_command(struct mtd_info *mtd, unsigned int command,
			      int column, int page_addr)
{
	struct nand_chip *nand = mtd_to_nand(mtd);
	struct sand_nand_chip *chip = to_sand_nand(nand);
	enum sand_nand_state new_state = chip->state;

	SAND_DEBUG(chip, "command=%02x column=%d page_addr=%d\n", command,
		   column, page_addr);

	if (!chip->selected)
		return;

	switch (chip->state) {
	case STATE_READY:
		if (command == NAND_CMD_RESET)
			goto reset;
		break;
	case STATE_PROG:
		new_state = STATE_IDLE;
		if (command != NAND_CMD_PAGEPROG ||
		    test_and_set_bit(chip->page_addr, chip->programmed)) {
			chip->status |= NAND_STATUS_FAIL;
			break;
		}

		if (sand_nand_seek(chip)) {
			chip->status |= NAND_STATUS_FAIL;
			break;
		}

		if (os_write(chip->fd, chip->tmp, chip->chunksize) !=
		    chip->chunksize) {
			SAND_DEBUG(chip, "could not write: %d\n", errno);
			chip->status |= NAND_STATUS_FAIL;
			break;
		}

		chip->fd_page_addr++;
		break;
	case STATE_ERASE:
		new_state = STATE_IDLE;
		if (command != NAND_CMD_ERASE2) {
			chip->status |= NAND_STATUS_FAIL;
			break;
		}

		if (chip->page_addr < 0 ||
		    chip->page_addr >= chip->pages ||
		    chip->page_addr % chip->pages_per_erase)
			chip->status |= NAND_STATUS_FAIL;
		else
			bitmap_clear(chip->programmed, chip->page_addr,
				     chip->pages_per_erase);
		break;
	default:
		chip->column = column;
		chip->page_addr = page_addr;
		switch (command) {
		case NAND_CMD_READOOB:
			if (column >= 0)
				chip->column += chip->pagesize;
			fallthrough;
		case NAND_CMD_READ0:
			new_state = STATE_IDLE;
			if (page_addr < 0 || page_addr >= chip->pages)
				break;

			if (chip->column < 0 || chip->column >= chip->chunksize)
				break;

			if (sand_nand_read(chip))
				break;

			chip->page_addr = page_addr;
			new_state = STATE_READ;
			break;
		case NAND_CMD_ERASE1:
			new_state = STATE_ERASE;
			chip->status = ~NAND_STATUS_FAIL;
			break;
		case NAND_CMD_STATUS:
			new_state = STATE_STATUS;
			chip->column = 0;
			break;
		case NAND_CMD_SEQIN:
			new_state = STATE_PROG;
			chip->status = ~NAND_STATUS_FAIL;
			if (page_addr < 0 || page_addr >= chip->pages ||
			    chip->column < 0 ||
			    chip->column >= chip->chunksize) {
				chip->status |= NAND_STATUS_FAIL;
			} else if (chip->tmp_dirty) {
				memset(chip->tmp, 0xff, chip->chunksize);
				chip->tmp_dirty = false;
			}
			break;
		case NAND_CMD_READID:
			if (chip->onfi[0] && column == 0x20)
				new_state = STATE_READ_ONFI;
			else
				new_state = STATE_READ_ID;
			chip->column = 0;
			break;
		case NAND_CMD_PARAM:
			if (chip->onfi[0] && !column)
				new_state = STATE_PARAM_ONFI;
			else
				new_state = STATE_IDLE;
			break;
		case NAND_CMD_RESET:
reset:
			new_state = STATE_IDLE;
			chip->column = -1;
			chip->page_addr = -1;
			chip->status = ~NAND_STATUS_FAIL;
			break;
		default:
			new_state = STATE_IDLE;
			SAND_DEBUG(chip, "Unsupported command %02x\n", command);
		}
	}

	to_state(chip, new_state);
}

static void sand_nand_select_chip(struct mtd_info *mtd, int n)
{
	struct nand_chip *nand = mtd_to_nand(mtd);
	struct sand_nand_chip *chip = to_sand_nand(nand);

	chip->selected = !n;
}

static void sand_nand_read_buf(struct mtd_info *mtd, u8 *buf, int len)
{
	struct nand_chip *nand = mtd_to_nand(mtd);
	struct sand_nand_chip *chip = to_sand_nand(nand);
	unsigned int to_copy;
	int src_len = 0;
	const u8 *src = NULL;

	if (!chip->selected)
		goto copy;

	switch (chip->state) {
	case STATE_READ:
		src = chip->tmp;
		src_len = chip->chunksize;
		break;
	case STATE_READ_ID:
		src = chip->id;
		src_len = chip->id_len;
		break;
	case STATE_READ_ONFI:
		src = "ONFI";
		src_len = 4;
		break;
	case STATE_PARAM_ONFI:
		src = chip->onfi;
		src_len = sizeof(chip->onfi);
		break;
	case STATE_STATUS:
		src = &chip->status;
		src_len = 1;
		break;
	default:
		break;
	}

copy:
	if (chip->column >= 0)
		to_copy = max(min(len, src_len - chip->column), 0);
	else
		to_copy = 0;
	memcpy(buf, src + chip->column, to_copy);
	memset(buf + to_copy, 0xff, len - to_copy);
	chip->column += to_copy;

	if (len == 1) {
		SAND_DEBUG(chip, "read [ %02x ]\n", buf[0]);
	} else if (src_len) {
		SAND_DEBUG(chip, "read %d bytes\n", len);
#ifdef VERBOSE_DEBUG
		print_hex_dump_bytes("", DUMP_PREFIX_OFFSET, buf, len);
#endif
	}

	if (src_len && chip->column == src_len)
		to_state(chip, STATE_IDLE);
}

static u8 sand_nand_read_byte(struct mtd_info *mtd)
{
	u8 ret;

	sand_nand_read_buf(mtd, &ret, 1);
	return ret;
}

static u16 sand_nand_read_word(struct mtd_info *mtd)
{
	struct nand_chip *nand = mtd_to_nand(mtd);
	struct sand_nand_chip *chip = to_sand_nand(nand);

	SAND_DEBUG(chip, "16-bit access unsupported\n");
	return sand_nand_read_byte(mtd) | 0xff00;
}

static void sand_nand_write_buf(struct mtd_info *mtd, const u8 *buf, int len)
{
	struct nand_chip *nand = mtd_to_nand(mtd);
	struct sand_nand_chip *chip = to_sand_nand(nand);

	SAND_DEBUG(chip, "write %d bytes\n", len);
#ifdef VERBOSE_DEBUG
	print_hex_dump_bytes("", DUMP_PREFIX_OFFSET, buf, len);
#endif

	if (chip->state != STATE_PROG || chip->status & NAND_STATUS_FAIL)
		return;

	chip->tmp_dirty = true;
	len = min((unsigned int)len, chip->chunksize - chip->column);
	memcpy(chip->tmp + chip->column, buf, len);
	chip->column += len;
}

static struct nand_chip *nand_chip;

int sand_nand_remove(struct udevice *dev)
{
	struct sand_nand_priv *priv = dev_get_priv(dev);
	struct sand_nand_chip *chip;

	list_for_each_entry(chip, &priv->chips, node) {
		struct nand_chip *nand = &chip->nand;

		if (nand_chip == nand)
			nand_chip = NULL;

		nand_unregister(nand_to_mtd(nand));
		free(chip->programmed);
		os_close(chip->fd);
		free(chip);
	}

	return 0;
}

static int sand_nand_probe(struct udevice *dev)
{
	struct sand_nand_priv *priv = dev_get_priv(dev);
	struct sand_nand_chip *chip;
	int ret, devnum = 0;
	ofnode np;

	INIT_LIST_HEAD(&priv->chips);

	dev_for_each_subnode(np, dev) {
		struct nand_chip *nand;
		struct mtd_info *mtd;
		u32 erasesize, oobsize, pagesize, pages;
		u32 err_count, err_step_size;
		off_t expected_size;
		char filename[30];
		fdt_addr_t cs;
		const u8 *id, *onfi;
		int id_len, onfi_len;

		cs = ofnode_get_addr_size_index_notrans(np, 0, NULL);
		if (cs == FDT_ADDR_T_NONE) {
			dev_dbg(dev, "Invalid cs for chip %s\n",
				ofnode_get_name(np));
			ret = -ENOENT;
			goto err;
		}

		id = ofnode_read_prop(np, "sandbox,id", &id_len);
		if (!id) {
			dev_dbg(dev, "No sandbox,id property for chip %s\n",
				ofnode_get_name(np));
			ret = -EINVAL;
			goto err;
		}

		onfi = ofnode_read_prop(np, "sandbox,onfi", &onfi_len);
		if (onfi && onfi_len != sizeof(struct nand_onfi_params)) {
			dev_dbg(dev, "Invalid length %d for onfi params\n",
				onfi_len);
			ret = -EINVAL;
			goto err;
		}

		ret = ofnode_read_u32(np, "sandbox,erasesize", &erasesize);
		if (ret) {
			dev_dbg(dev, "No sandbox,erasesize property for chip %s",
				ofnode_get_name(np));
			goto err;
		}

		ret = ofnode_read_u32(np, "sandbox,oobsize", &oobsize);
		if (ret) {
			dev_dbg(dev, "No sandbox,oobsize property for chip %s",
				ofnode_get_name(np));
			goto err;
		}

		ret = ofnode_read_u32(np, "sandbox,pagesize", &pagesize);
		if (ret) {
			dev_dbg(dev, "No sandbox,pagesize property for chip %s",
				ofnode_get_name(np));
			goto err;
		}

		ret = ofnode_read_u32(np, "sandbox,pages", &pages);
		if (ret) {
			dev_dbg(dev, "No sandbox,pages property for chip %s",
				ofnode_get_name(np));
			goto err;
		}

		ret = ofnode_read_u32(np, "sandbox,err-count", &err_count);
		if (ret) {
			dev_dbg(dev,
				"No sandbox,err-count property for chip %s",
				ofnode_get_name(np));
			goto err;
		}

		ret = ofnode_read_u32(np, "sandbox,err-step-size",
				      &err_step_size);
		if (ret) {
			dev_dbg(dev,
				"No sandbox,err-step-size property for chip %s",
				ofnode_get_name(np));
			goto err;
		}

		chip = calloc(sizeof(*chip), 1);
		if (!chip) {
			ret = -ENOMEM;
			goto err;
		}

		chip->cs = cs;
		chip->id = id;
		chip->id_len = id_len;
		chip->chunksize = pagesize + oobsize;
		chip->pagesize = pagesize;
		chip->pages = pages;
		chip->pages_per_erase = erasesize / pagesize;
		memset(chip->tmp, 0xff, chip->chunksize);

		chip->err_count = err_count;
		chip->err_step_bits = err_step_size * 8;
		chip->err_steps = pagesize / err_step_size;

		expected_size = (off_t)pages * chip->chunksize;
		snprintf(filename, sizeof(filename),
			 "/tmp/u-boot.nand%d.XXXXXX", devnum);
		chip->fd = os_mktemp(filename, expected_size);
		if (chip->fd < 0) {
			dev_dbg(dev, "Could not create temp file %s\n",
				filename);
			ret = chip->fd;
			goto err_chip;
		}

		chip->programmed = calloc(sizeof(long),
					  BITS_TO_LONGS(pages));
		if (!chip->programmed) {
			ret = -ENOMEM;
			goto err_fd;
		}

		if (onfi) {
			memcpy(chip->onfi, onfi, onfi_len);
			memcpy(chip->onfi + onfi_len, onfi, onfi_len);
			memcpy(chip->onfi + 2 * onfi_len, onfi, onfi_len);
		}

		nand = &chip->nand;
		nand->options = spl_in_proper() ? 0 : NAND_SKIP_BBTSCAN;
		nand->flash_node = np;
		nand->dev_ready = sand_nand_dev_ready;
		nand->cmdfunc = sand_nand_command;
		nand->waitfunc = sand_nand_wait;
		nand->select_chip = sand_nand_select_chip;
		nand->read_byte = sand_nand_read_byte;
		nand->read_word = sand_nand_read_word;
		nand->read_buf = sand_nand_read_buf;
		nand->write_buf = sand_nand_write_buf;
		nand->ecc.options = NAND_ECC_GENERIC_ERASED_CHECK;

		mtd = nand_to_mtd(nand);
		mtd->dev = dev;

		ret = nand_scan(mtd, CONFIG_SYS_NAND_MAX_CHIPS);
		if (ret) {
			dev_dbg(dev, "Could not scan chip %s: %d\n",
				ofnode_get_name(np), ret);
			goto err_prog;
		}
		chip->ecc_bits = nand->ecc.layout->eccbytes * 8 /
				 chip->err_steps;

		ret = nand_register(devnum, mtd);
		if (ret) {
			dev_dbg(dev, "Could not register nand %d: %d\n", devnum,
				ret);
			goto err_prog;
		}

		if (!nand_chip)
			nand_chip = nand;

		list_add_tail(&chip->node, &priv->chips);
		devnum++;
		continue;

err_prog:
		free(chip->programmed);
err_fd:
		os_close(chip->fd);
err_chip:
		free(chip);
		goto err;
	}

	return 0;

err:
	sand_nand_remove(dev);
	return ret;
}

static const struct udevice_id sand_nand_ids[] = {
	{ .compatible = "sandbox,nand" },
	{ }
};

U_BOOT_DRIVER(sand_nand) = {
	.name           = "sand-nand",
	.id             = UCLASS_MTD,
	.of_match       = sand_nand_ids,
	.probe          = sand_nand_probe,
	.remove		= sand_nand_remove,
	.priv_auto	= sizeof(struct sand_nand_priv),
};

void board_nand_init(void)
{
	struct udevice *dev;
	int err;

	err = uclass_get_device_by_driver(UCLASS_MTD, DM_DRIVER_REF(sand_nand),
					  &dev);
	if (err && err != -ENODEV)
		log_info("Failed to get sandbox NAND: %d\n", err);
}

#if IS_ENABLED(CONFIG_SPL_BUILD) && IS_ENABLED(CONFIG_SPL_NAND_INIT)
void nand_deselect(void)
{
	nand_chip->select_chip(nand_to_mtd(nand_chip), -1);
}

static int nand_is_bad_block(int block)
{
	struct mtd_info *mtd = nand_to_mtd(nand_chip);

	return mtd_block_isbad(mtd, block << mtd->erasesize_shift);
}

static int nand_read_page(int block, int page, uchar *dst)
{
	struct mtd_info *mtd = nand_to_mtd(nand_chip);
	loff_t ofs = ((loff_t)block << mtd->erasesize_shift) +
		     ((loff_t)page << mtd->writesize_shift);
	size_t len = mtd->writesize;

	return nand_read(mtd, ofs, &len, dst);
}

#include "nand_spl_loaders.c"
#endif /* CONFIG_SPL_NAND_INIT */
