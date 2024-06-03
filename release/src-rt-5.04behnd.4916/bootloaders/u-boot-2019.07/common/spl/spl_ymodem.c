// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000-2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2011
 * Texas Instruments, <www.ti.com>
 *
 * Matt Porter <mporter@ti.com>
 */
#include <common.h>
#include <spl.h>
#include <xyzModem.h>
#include <asm/u-boot.h>
#include <linux/libfdt.h>

#define BUF_SIZE 1024

/*
 * Information required to load image using ymodem.
 *
 * @image_read: Now of bytes read from the image.
 * @buf: pointer to the previous read block.
 */
struct ymodem_fit_info {
	int image_read;
	char *buf;
};

static int getcymodem(void) {
	if (tstc())
		return (getc());
	return -1;
}

static ulong ymodem_read_fit(struct spl_load_info *load, ulong offset,
			     ulong size, void *addr)
{
	int res, err;
	struct ymodem_fit_info *info = load->priv;
	char *buf = info->buf;

	while (info->image_read < offset) {
		res = xyzModem_stream_read(buf, BUF_SIZE, &err);
		if (res <= 0)
			return res;
		info->image_read += res;
	}

	if (info->image_read > offset) {
		res = info->image_read - offset;
		memcpy(addr, &buf[BUF_SIZE - res], res);
		addr = addr + res;
	}

	while (info->image_read < offset + size) {
		res = xyzModem_stream_read(buf, BUF_SIZE, &err);
		if (res <= 0)
			return res;

		memcpy(addr, buf, res);
		info->image_read += res;
		addr += res;
	}

	return size;
}

static int spl_ymodem_load_image(struct spl_image_info *spl_image,
				 struct spl_boot_device *bootdev)
{
	ulong size = 0;
	int err;
	int res;
	int ret;
	connection_info_t info;
	char buf[BUF_SIZE];
	struct image_header *ih = NULL;
	ulong addr = 0;

	info.mode = xyzModem_ymodem;
	ret = xyzModem_stream_open(&info, &err);
	if (ret) {
		printf("spl: ymodem err - %s\n", xyzModem_error(err));
		return ret;
	}

	res = xyzModem_stream_read(buf, BUF_SIZE, &err);
	if (res <= 0)
		goto end_stream;

	if (IS_ENABLED(CONFIG_SPL_LOAD_FIT_FULL) &&
	    image_get_magic((struct image_header *)buf) == FDT_MAGIC) {
		addr = CONFIG_SYS_LOAD_ADDR;
		ih = (struct image_header *)addr;

		memcpy((void *)addr, buf, res);
		size += res;
		addr += res;

		while ((res = xyzModem_stream_read(buf, BUF_SIZE, &err)) > 0) {
			memcpy((void *)addr, buf, res);
			size += res;
			addr += res;
		}

		ret = spl_parse_image_header(spl_image, ih);
		if (ret)
			return ret;
	} else if (CONFIG_IS_ENABLED(LOAD_FIT) &&
	    image_get_magic((struct image_header *)buf) == FDT_MAGIC) {
		struct spl_load_info load;
		struct ymodem_fit_info info;

		debug("Found FIT\n");
		load.dev = NULL;
		load.priv = (void *)&info;
		load.filename = NULL;
		load.bl_len = 1;
		info.buf = buf;
		info.image_read = BUF_SIZE;
		load.read = ymodem_read_fit;
		ret = spl_load_simple_fit(spl_image, &load, 0, (void *)buf);
		size = info.image_read;

		while ((res = xyzModem_stream_read(buf, BUF_SIZE, &err)) > 0)
			size += res;
	} else {
		ih = (struct image_header *)buf;
		ret = spl_parse_image_header(spl_image, ih);
		if (ret)
			goto end_stream;
#ifdef CONFIG_SPL_GZIP
		if (ih->ih_comp == IH_COMP_GZIP)
			addr = CONFIG_SYS_LOAD_ADDR;
		else
#endif
			addr = spl_image->load_addr;
		memcpy((void *)addr, buf, res);
		ih = (struct image_header *)addr;
		size += res;
		addr += res;

		while ((res = xyzModem_stream_read(buf, BUF_SIZE, &err)) > 0) {
			memcpy((void *)addr, buf, res);
			size += res;
			addr += res;
		}
	}

end_stream:
	xyzModem_stream_close(&err);
	xyzModem_stream_terminate(false, &getcymodem);

	printf("Loaded %lu bytes\n", size);

#ifdef CONFIG_SPL_GZIP
	if (!(IS_ENABLED(CONFIG_SPL_LOAD_FIT) &&
	      image_get_magic((struct image_header *)buf) == FDT_MAGIC) &&
	    (ih->ih_comp == IH_COMP_GZIP)) {
		if (gunzip((void *)(spl_image->load_addr + sizeof(*ih)),
			   CONFIG_SYS_BOOTM_LEN,
			   (void *)(CONFIG_SYS_LOAD_ADDR + sizeof(*ih)),
			   &size)) {
			puts("Uncompressing error\n");
			return -EIO;
		}
	}
#endif

	return ret;
}
SPL_LOAD_IMAGE_METHOD("UART", 0, BOOT_DEVICE_UART, spl_ymodem_load_image);
