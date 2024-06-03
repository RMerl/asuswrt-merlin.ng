/*
 * Simulate a SPI flash
 *
 * Copyright (c) 2011-2013 The Chromium OS Authors.
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * Licensed under the GPL-2 or later.
 */

#define LOG_CATEGORY UCLASS_SPI_FLASH

#include <common.h>
#include <dm.h>
#include <malloc.h>
#include <spi.h>
#include <os.h>

#include <spi_flash.h>
#include "sf_internal.h"

#include <asm/getopt.h>
#include <asm/spi.h>
#include <asm/state.h>
#include <dm/device-internal.h>
#include <dm/lists.h>
#include <dm/uclass-internal.h>

/*
 * The different states that our SPI flash transitions between.
 * We need to keep track of this across multiple xfer calls since
 * the SPI bus could possibly call down into us multiple times.
 */
enum sandbox_sf_state {
	SF_CMD,   /* default state -- we're awaiting a command */
	SF_ID,    /* read the flash's (jedec) ID code */
	SF_ADDR,  /* processing the offset in the flash to read/etc... */
	SF_READ,  /* reading data from the flash */
	SF_WRITE, /* writing data to the flash, i.e. page programming */
	SF_ERASE, /* erase the flash */
	SF_READ_STATUS, /* read the flash's status register */
	SF_READ_STATUS1, /* read the flash's status register upper 8 bits*/
	SF_WRITE_STATUS, /* write the flash's status register */
};

#if CONFIG_IS_ENABLED(LOG)
static const char *sandbox_sf_state_name(enum sandbox_sf_state state)
{
	static const char * const states[] = {
		"CMD", "ID", "ADDR", "READ", "WRITE", "ERASE", "READ_STATUS",
		"READ_STATUS1", "WRITE_STATUS",
	};
	return states[state];
}
#endif /* LOG */

/* Bits for the status register */
#define STAT_WIP	(1 << 0)
#define STAT_WEL	(1 << 1)
#define STAT_BP_SHIFT	2
#define STAT_BP_MASK	(7 << STAT_BP_SHIFT)

/* Assume all SPI flashes have 3 byte addresses since they do atm */
#define SF_ADDR_LEN	3

#define IDCODE_LEN 3

/* Used to quickly bulk erase backing store */
static u8 sandbox_sf_0xff[0x1000];

/* Internal state data for each SPI flash */
struct sandbox_spi_flash {
	unsigned int cs;	/* Chip select we are attached to */
	/*
	 * As we receive data over the SPI bus, our flash transitions
	 * between states.  For example, we start off in the SF_CMD
	 * state where the first byte tells us what operation to perform
	 * (such as read or write the flash).  But the operation itself
	 * can go through a few states such as first reading in the
	 * offset in the flash to perform the requested operation.
	 * Thus "state" stores the exact state that our machine is in
	 * while "cmd" stores the overall command we're processing.
	 */
	enum sandbox_sf_state state;
	uint cmd;
	/* Erase size of current erase command */
	uint erase_size;
	/* Current position in the flash; used when reading/writing/etc... */
	uint off;
	/* How many address bytes we've consumed */
	uint addr_bytes, pad_addr_bytes;
	/* The current flash status (see STAT_XXX defines above) */
	u16 status;
	/* Data describing the flash we're emulating */
	const struct flash_info *data;
	/* The file on disk to serv up data from */
	int fd;
};

struct sandbox_spi_flash_plat_data {
	const char *filename;
	const char *device_name;
	int bus;
	int cs;
};

void sandbox_sf_set_block_protect(struct udevice *dev, int bp_mask)
{
	struct sandbox_spi_flash *sbsf = dev_get_priv(dev);

	sbsf->status &= ~STAT_BP_MASK;
	sbsf->status |= bp_mask << STAT_BP_SHIFT;
}

/**
 * This is a very strange probe function. If it has platform data (which may
 * have come from the device tree) then this function gets the filename and
 * device type from there.
 */
static int sandbox_sf_probe(struct udevice *dev)
{
	/* spec = idcode:file */
	struct sandbox_spi_flash *sbsf = dev_get_priv(dev);
	size_t len, idname_len;
	const struct flash_info *data;
	struct sandbox_spi_flash_plat_data *pdata = dev_get_platdata(dev);
	struct sandbox_state *state = state_get_current();
	struct dm_spi_slave_platdata *slave_plat;
	struct udevice *bus = dev->parent;
	const char *spec = NULL;
	struct udevice *emul;
	int ret = 0;
	int cs = -1;

	debug("%s: bus %d, looking for emul=%p: ", __func__, bus->seq, dev);
	ret = sandbox_spi_get_emul(state, bus, dev, &emul);
	if (ret) {
		printf("Error: Unknown chip select for device '%s'\n",
			dev->name);
		return ret;
	}
	slave_plat = dev_get_parent_platdata(dev);
	cs = slave_plat->cs;
	debug("found at cs %d\n", cs);

	if (!pdata->filename) {
		printf("Error: No filename available\n");
		return -EINVAL;
	}
	spec = strchr(pdata->device_name, ',');
	if (spec)
		spec++;
	else
		spec = pdata->device_name;
	idname_len = strlen(spec);
	debug("%s: device='%s'\n", __func__, spec);

	for (data = spi_nor_ids; data->name; data++) {
		len = strlen(data->name);
		if (idname_len != len)
			continue;
		if (!strncasecmp(spec, data->name, len))
			break;
	}
	if (!data->name) {
		printf("%s: unknown flash '%*s'\n", __func__, (int)idname_len,
		       spec);
		ret = -EINVAL;
		goto error;
	}

	if (sandbox_sf_0xff[0] == 0x00)
		memset(sandbox_sf_0xff, 0xff, sizeof(sandbox_sf_0xff));

	sbsf->fd = os_open(pdata->filename, 02);
	if (sbsf->fd == -1) {
		printf("%s: unable to open file '%s'\n", __func__,
		       pdata->filename);
		ret = -EIO;
		goto error;
	}

	sbsf->data = data;
	sbsf->cs = cs;

	return 0;

 error:
	debug("%s: Got error %d\n", __func__, ret);
	return ret;
}

static int sandbox_sf_remove(struct udevice *dev)
{
	struct sandbox_spi_flash *sbsf = dev_get_priv(dev);

	os_close(sbsf->fd);

	return 0;
}

static void sandbox_sf_cs_activate(struct udevice *dev)
{
	struct sandbox_spi_flash *sbsf = dev_get_priv(dev);

	log_content("sandbox_sf: CS activated; state is fresh!\n");

	/* CS is asserted, so reset state */
	sbsf->off = 0;
	sbsf->addr_bytes = 0;
	sbsf->pad_addr_bytes = 0;
	sbsf->state = SF_CMD;
	sbsf->cmd = SF_CMD;
}

static void sandbox_sf_cs_deactivate(struct udevice *dev)
{
	log_content("sandbox_sf: CS deactivated; cmd done processing!\n");
}

/*
 * There are times when the data lines are allowed to tristate.  What
 * is actually sensed on the line depends on the hardware.  It could
 * always be 0xFF/0x00 (if there are pull ups/downs), or things could
 * float and so we'd get garbage back.  This func encapsulates that
 * scenario so we can worry about the details here.
 */
static void sandbox_spi_tristate(u8 *buf, uint len)
{
	/* XXX: make this into a user config option ? */
	memset(buf, 0xff, len);
}

/* Figure out what command this stream is telling us to do */
static int sandbox_sf_process_cmd(struct sandbox_spi_flash *sbsf, const u8 *rx,
				  u8 *tx)
{
	enum sandbox_sf_state oldstate = sbsf->state;

	/* We need to output a byte for the cmd byte we just ate */
	if (tx)
		sandbox_spi_tristate(tx, 1);

	sbsf->cmd = rx[0];
	switch (sbsf->cmd) {
	case SPINOR_OP_RDID:
		sbsf->state = SF_ID;
		sbsf->cmd = SF_ID;
		break;
	case SPINOR_OP_READ_FAST:
		sbsf->pad_addr_bytes = 1;
	case SPINOR_OP_READ:
	case SPINOR_OP_PP:
		sbsf->state = SF_ADDR;
		break;
	case SPINOR_OP_WRDI:
		debug(" write disabled\n");
		sbsf->status &= ~STAT_WEL;
		break;
	case SPINOR_OP_RDSR:
		sbsf->state = SF_READ_STATUS;
		break;
	case SPINOR_OP_RDSR2:
		sbsf->state = SF_READ_STATUS1;
		break;
	case SPINOR_OP_WREN:
		debug(" write enabled\n");
		sbsf->status |= STAT_WEL;
		break;
	case SPINOR_OP_WRSR:
		sbsf->state = SF_WRITE_STATUS;
		break;
	default: {
		int flags = sbsf->data->flags;

		/* we only support erase here */
		if (sbsf->cmd == SPINOR_OP_CHIP_ERASE) {
			sbsf->erase_size = sbsf->data->sector_size *
				sbsf->data->n_sectors;
		} else if (sbsf->cmd == SPINOR_OP_BE_4K && (flags & SECT_4K)) {
			sbsf->erase_size = 4 << 10;
		} else if (sbsf->cmd == SPINOR_OP_SE && !(flags & SECT_4K)) {
			sbsf->erase_size = 64 << 10;
		} else {
			debug(" cmd unknown: %#x\n", sbsf->cmd);
			return -EIO;
		}
		sbsf->state = SF_ADDR;
		break;
	}
	}

	if (oldstate != sbsf->state)
		log_content(" cmd: transition to %s state\n",
			    sandbox_sf_state_name(sbsf->state));

	return 0;
}

int sandbox_erase_part(struct sandbox_spi_flash *sbsf, int size)
{
	int todo;
	int ret;

	while (size > 0) {
		todo = min(size, (int)sizeof(sandbox_sf_0xff));
		ret = os_write(sbsf->fd, sandbox_sf_0xff, todo);
		if (ret != todo)
			return ret;
		size -= todo;
	}

	return 0;
}

static int sandbox_sf_xfer(struct udevice *dev, unsigned int bitlen,
			   const void *rxp, void *txp, unsigned long flags)
{
	struct sandbox_spi_flash *sbsf = dev_get_priv(dev);
	const uint8_t *rx = rxp;
	uint8_t *tx = txp;
	uint cnt, pos = 0;
	int bytes = bitlen / 8;
	int ret;

	log_content("sandbox_sf: state:%x(%s) bytes:%u\n", sbsf->state,
		    sandbox_sf_state_name(sbsf->state), bytes);

	if ((flags & SPI_XFER_BEGIN))
		sandbox_sf_cs_activate(dev);

	if (sbsf->state == SF_CMD) {
		/* Figure out the initial state */
		ret = sandbox_sf_process_cmd(sbsf, rx, tx);
		if (ret)
			return ret;
		++pos;
	}

	/* Process the remaining data */
	while (pos < bytes) {
		switch (sbsf->state) {
		case SF_ID: {
			u8 id;

			log_content(" id: off:%u tx:", sbsf->off);
			if (sbsf->off < IDCODE_LEN) {
				/* Extract correct byte from ID 0x00aabbcc */
				id = ((JEDEC_MFR(sbsf->data) << 16) |
					JEDEC_ID(sbsf->data)) >>
					(8 * (IDCODE_LEN - 1 - sbsf->off));
			} else {
				id = 0;
			}
			log_content("%d %02x\n", sbsf->off, id);
			tx[pos++] = id;
			++sbsf->off;
			break;
		}
		case SF_ADDR:
			log_content(" addr: bytes:%u rx:%02x ",
				    sbsf->addr_bytes, rx[pos]);

			if (sbsf->addr_bytes++ < SF_ADDR_LEN)
				sbsf->off = (sbsf->off << 8) | rx[pos];
			log_content("addr:%06x\n", sbsf->off);

			if (tx)
				sandbox_spi_tristate(&tx[pos], 1);
			pos++;

			/* See if we're done processing */
			if (sbsf->addr_bytes <
					SF_ADDR_LEN + sbsf->pad_addr_bytes)
				break;

			/* Next state! */
			if (os_lseek(sbsf->fd, sbsf->off, OS_SEEK_SET) < 0) {
				puts("sandbox_sf: os_lseek() failed");
				return -EIO;
			}
			switch (sbsf->cmd) {
			case SPINOR_OP_READ_FAST:
			case SPINOR_OP_READ:
				sbsf->state = SF_READ;
				break;
			case SPINOR_OP_PP:
				sbsf->state = SF_WRITE;
				break;
			default:
				/* assume erase state ... */
				sbsf->state = SF_ERASE;
				goto case_sf_erase;
			}
			log_content(" cmd: transition to %s state\n",
				    sandbox_sf_state_name(sbsf->state));
			break;
		case SF_READ:
			/*
			 * XXX: need to handle exotic behavior:
			 *      - reading past end of device
			 */

			cnt = bytes - pos;
			log_content(" tx: read(%u)\n", cnt);
			assert(tx);
			ret = os_read(sbsf->fd, tx + pos, cnt);
			if (ret < 0) {
				puts("sandbox_sf: os_read() failed\n");
				return -EIO;
			}
			pos += ret;
			break;
		case SF_READ_STATUS:
			log_content(" read status: %#x\n", sbsf->status);
			cnt = bytes - pos;
			memset(tx + pos, sbsf->status, cnt);
			pos += cnt;
			break;
		case SF_READ_STATUS1:
			log_content(" read status: %#x\n", sbsf->status);
			cnt = bytes - pos;
			memset(tx + pos, sbsf->status >> 8, cnt);
			pos += cnt;
			break;
		case SF_WRITE_STATUS:
			log_content(" write status: %#x (ignored)\n", rx[pos]);
			pos = bytes;
			break;
		case SF_WRITE:
			/*
			 * XXX: need to handle exotic behavior:
			 *      - unaligned addresses
			 *      - more than a page (256) worth of data
			 *      - reading past end of device
			 */
			if (!(sbsf->status & STAT_WEL)) {
				puts("sandbox_sf: write enable not set before write\n");
				goto done;
			}

			cnt = bytes - pos;
			log_content(" rx: write(%u)\n", cnt);
			if (tx)
				sandbox_spi_tristate(&tx[pos], cnt);
			ret = os_write(sbsf->fd, rx + pos, cnt);
			if (ret < 0) {
				puts("sandbox_spi: os_write() failed\n");
				return -EIO;
			}
			pos += ret;
			sbsf->status &= ~STAT_WEL;
			break;
		case SF_ERASE:
 case_sf_erase: {
			if (!(sbsf->status & STAT_WEL)) {
				puts("sandbox_sf: write enable not set before erase\n");
				goto done;
			}

			/* verify address is aligned */
			if (sbsf->off & (sbsf->erase_size - 1)) {
				log_content(" sector erase: cmd:%#x needs align:%#x, but we got %#x\n",
					    sbsf->cmd, sbsf->erase_size,
					    sbsf->off);
				sbsf->status &= ~STAT_WEL;
				goto done;
			}

			log_content(" sector erase addr: %u, size: %u\n",
				    sbsf->off, sbsf->erase_size);

			cnt = bytes - pos;
			if (tx)
				sandbox_spi_tristate(&tx[pos], cnt);
			pos += cnt;

			/*
			 * TODO(vapier@gentoo.org): latch WIP in status, and
			 * delay before clearing it ?
			 */
			ret = sandbox_erase_part(sbsf, sbsf->erase_size);
			sbsf->status &= ~STAT_WEL;
			if (ret) {
				log_content("sandbox_sf: Erase failed\n");
				goto done;
			}
			goto done;
		}
		default:
			log_content(" ??? no idea what to do ???\n");
			goto done;
		}
	}

 done:
	if (flags & SPI_XFER_END)
		sandbox_sf_cs_deactivate(dev);
	return pos == bytes ? 0 : -EIO;
}

int sandbox_sf_ofdata_to_platdata(struct udevice *dev)
{
	struct sandbox_spi_flash_plat_data *pdata = dev_get_platdata(dev);

	pdata->filename = dev_read_string(dev, "sandbox,filename");
	pdata->device_name = dev_read_string(dev, "compatible");
	if (!pdata->filename || !pdata->device_name) {
		debug("%s: Missing properties, filename=%s, device_name=%s\n",
		      __func__, pdata->filename, pdata->device_name);
		return -EINVAL;
	}

	return 0;
}

static const struct dm_spi_emul_ops sandbox_sf_emul_ops = {
	.xfer          = sandbox_sf_xfer,
};

#ifdef CONFIG_SPI_FLASH
int sandbox_sf_bind_emul(struct sandbox_state *state, int busnum, int cs,
			 struct udevice *bus, ofnode node, const char *spec)
{
	struct udevice *emul;
	char name[20], *str;
	struct driver *drv;
	int ret;

	/* now the emulator */
	strncpy(name, spec, sizeof(name) - 6);
	name[sizeof(name) - 6] = '\0';
	strcat(name, "-emul");
	drv = lists_driver_lookup_name("sandbox_sf_emul");
	if (!drv) {
		puts("Cannot find sandbox_sf_emul driver\n");
		return -ENOENT;
	}
	str = strdup(name);
	if (!str)
		return -ENOMEM;
	ret = device_bind_ofnode(bus, drv, str, NULL, node, &emul);
	if (ret) {
		free(str);
		printf("Cannot create emul device for spec '%s' (err=%d)\n",
		       spec, ret);
		return ret;
	}
	state->spi[busnum][cs].emul = emul;

	return 0;
}

void sandbox_sf_unbind_emul(struct sandbox_state *state, int busnum, int cs)
{
	struct udevice *dev;

	dev = state->spi[busnum][cs].emul;
	device_remove(dev, DM_REMOVE_NORMAL);
	device_unbind(dev);
	state->spi[busnum][cs].emul = NULL;
}

int sandbox_spi_get_emul(struct sandbox_state *state,
			 struct udevice *bus, struct udevice *slave,
			 struct udevice **emulp)
{
	struct sandbox_spi_info *info;
	int busnum = bus->seq;
	int cs = spi_chip_select(slave);
	int ret;

	info = &state->spi[busnum][cs];
	if (!info->emul) {
		/* Use the same device tree node as the SPI flash device */
		debug("%s: busnum=%u, cs=%u: binding SPI flash emulation: ",
		      __func__, busnum, cs);
		ret = sandbox_sf_bind_emul(state, busnum, cs, bus,
					   dev_ofnode(slave), slave->name);
		if (ret) {
			debug("failed (err=%d)\n", ret);
			return ret;
		}
		debug("OK\n");
	}
	*emulp = info->emul;

	return 0;
}
#endif

static const struct udevice_id sandbox_sf_ids[] = {
	{ .compatible = "sandbox,spi-flash" },
	{ }
};

U_BOOT_DRIVER(sandbox_sf_emul) = {
	.name		= "sandbox_sf_emul",
	.id		= UCLASS_SPI_EMUL,
	.of_match	= sandbox_sf_ids,
	.ofdata_to_platdata = sandbox_sf_ofdata_to_platdata,
	.probe		= sandbox_sf_probe,
	.remove		= sandbox_sf_remove,
	.priv_auto_alloc_size = sizeof(struct sandbox_spi_flash),
	.platdata_auto_alloc_size = sizeof(struct sandbox_spi_flash_plat_data),
	.ops		= &sandbox_sf_emul_ops,
};
