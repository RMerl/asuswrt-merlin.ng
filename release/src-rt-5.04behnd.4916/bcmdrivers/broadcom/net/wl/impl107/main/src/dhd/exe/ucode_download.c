/*
 * Ucode download related utility functions
 *
 * Copyright (C) 2025, Broadcom. All Rights Reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: ucode_download.c 841294 2024-06-14 16:25:45Z $
 */

#include <unistd.h>
#include <errno.h>
#include <trxhdr.h>
#include <bcmendian.h>
#include <wlu_common.h>

#define DEVPRESENT_DELAY  10000	/* in microsecs */
#define DEVPRESENT_RETRIES   100

extern int wl_validatedev(void *dev_handle);

int
dload_generic_data(void *wl, unsigned char *dload_buf, int len)
{
	struct wl_dload_data *dload_ptr = (struct wl_dload_data *)dload_buf;
	int err = 0;
	int actual_data_offset;
	char *buf;

	actual_data_offset = OFFSETOF(struct wl_dload_data, data);
	dload_ptr->flag = (DLOAD_HANDLER_VER << DLOAD_FLAG_VER_SHIFT);
	dload_ptr->flag |= DL_CRC_NOT_INUSE;
	dload_ptr->len = htod32(len - actual_data_offset);
	dload_ptr->crc = 0;

	len = len + 8 - (len%8);

	buf = malloc(WLC_IOCTL_MEDLEN);
	if (buf) {
		bzero(buf, WLC_IOCTL_MEDLEN);
		err = wlu_iovar_setbuf(wl, "generic_dload", dload_buf, len, buf,
			WLC_IOCTL_MEDLEN);
	}
	free(buf);
	return err;
}

int
dload_ucode_part(void *wl, uint8 ucode_type, uint32 datalen, unsigned char *org_buf)
{
	int num_chunks, chunk_len, cumulative_len = 0;
	int size2alloc, ucode_chunk_len = 0;
	unsigned char *new_buf;
	struct wl_ucode_info *ucode_ptr;
	int err = 0, ucode_offset, chunk_offset;

	ucode_offset = OFFSETOF(wl_dload_data_t, data);
	chunk_offset = OFFSETOF(wl_ucode_info_t, data_chunk);

	err = wlu_iovar_getint(wl, "ucdload_chunk_len",
		&ucode_chunk_len);
	if (err) {
		printf("err in getting ucode chunk len, exiting\n");
		return err;
	}

	num_chunks = datalen/ucode_chunk_len;
	if (datalen % ucode_chunk_len != 0)
		num_chunks++;
	size2alloc = ucode_offset + chunk_offset + ucode_chunk_len;

	/* single chunk buffer */
	new_buf = (unsigned char *)malloc(size2alloc);
	memset(new_buf, 0, size2alloc);
	ucode_ptr = (struct wl_ucode_info *)((uint8 *)new_buf+ucode_offset);
	ucode_ptr->ucode_type = ucode_type;
	ucode_ptr->num_chunks = num_chunks;
	do {
		if (datalen >= ucode_chunk_len)
			chunk_len = ucode_chunk_len;
		else
			chunk_len = datalen;
		memset(new_buf+ucode_offset+chunk_offset, 0, size2alloc-ucode_offset-chunk_offset);
		ucode_ptr->chunk_len = htod32(chunk_len);
		ucode_ptr->chunk_num++;
		memcpy(&ucode_ptr->data_chunk[0], org_buf + cumulative_len, chunk_len);
		cumulative_len += chunk_len;
		err = dload_generic_data(wl, new_buf, size2alloc);
		if (err) {
			printf("error while writing %s to the memory\n",
				(ucode_type == UCODE_FW)? "ucode" : "initvals");
			break;
		}
		datalen = datalen - chunk_len;
	} while (datalen > 0);
	free(new_buf);

	return err;
}

static int
check_ucode_file(unsigned char *headers)
{
	struct trx_header *trx;
	int actual_data_len = -1;

	/* Extract trx header */
	trx = (struct trx_header *)headers;
	if (trx->magic != TRX_MAGIC) {
		printf("Error: trx bad hdr\n");
		goto err;
	}
	actual_data_len = ROUNDUP(trx->offsets[0], 4) + ROUNDUP(trx->offsets[1], 4);
err:
	return actual_data_len;
}

int
proc_ucode_download(char* fw_filename, void *dev_handle)
{
	FILE *fp = NULL;
	int ret = 0, loopcnt = 0;
	struct trx_header main_trx_hdr, *ucode_trx_hdr = NULL;
	uint32 maintrx_hdr_len, tmp_len;
	uint32 fw_size, second_offset, ucode_trx_offset;
	long ucode_pos;
	int ucode_info_len = 0, status;
	unsigned char *ucodetrx_buf = NULL, *initvals_ptr = NULL;
	int ucode_len, initvals_len;
	int ucdload_status = 0;
	int is_devpresent;

	/* read the file and push blocks down to memory */
	if ((fp = fopen(fw_filename, "rb")) == NULL) {
		fprintf(stderr, "%s: unable to open %s: %s\n",
		        __FUNCTION__, fw_filename, strerror(errno));
		ret = -1;
		goto exit;
	}

	maintrx_hdr_len = sizeof(struct trx_header);
	tmp_len = fread(&main_trx_hdr, sizeof(uint8), maintrx_hdr_len, fp);

	if (tmp_len == maintrx_hdr_len) {
		if (main_trx_hdr.magic == TRX_MAGIC) {
			fw_size = main_trx_hdr.offsets[0];
			second_offset = main_trx_hdr.offsets[2];

			if (second_offset == maintrx_hdr_len) {
				second_offset = 0;
			}
			ucode_trx_offset = maintrx_hdr_len +
				ROUNDUP(fw_size, 4) + ROUNDUP(second_offset, 4);
			ucode_pos = fseek(fp, ucode_trx_offset, SEEK_SET);
			BCM_REFERENCE(ucode_pos);

			if ((ucode_trx_hdr = malloc(sizeof(struct trx_header)))
				== NULL) {
				printf("Unable to allocate %d bytes!\n", maintrx_hdr_len);
				ret = -ENOMEM;
				goto exit;
			}

			/* Read ONLY the firmware-file-header into the new_buffer */
			status = fread(ucode_trx_hdr, sizeof(uint8),
				maintrx_hdr_len, fp);
			if (status < sizeof(struct trx_header)) {
				printf("Short read in hdr read for %s!\n", fw_filename);
				ret = -EINVAL;
				goto exit;
			}

			if ((ucode_info_len = check_ucode_file(
				(unsigned char *)ucode_trx_hdr)) <= 0) {
				printf("not a valid ucode.trx\n");
				ret = -1;
				goto exit;
			}

			if ((ucodetrx_buf = (unsigned char *)malloc(ucode_info_len *
				sizeof(char))) == NULL) {
				printf("Unable to allocate ucodetrx_buf\n");
				ret = -ENOMEM;
				goto exit;
			}
			tmp_len = fread(ucodetrx_buf, sizeof(uint8),
				ucode_info_len, fp);
			ucode_len = ucode_trx_hdr->offsets[0];
			initvals_ptr = ucodetrx_buf +
				ROUNDUP(ucode_trx_hdr->offsets[0], 4);
			initvals_len = ucode_trx_hdr->offsets[1];

			init_cmd_batchingmode();
			do {
				is_devpresent = wl_validatedev(dev_handle);
				loopcnt++;
				/* in USB after dongle fw starts running wl interface
				might not appear in the list of interfaces immediately, hence try
				after some delay of 10ms
				*/
				if (!is_devpresent)
					usleep(DEVPRESENT_DELAY);
				else {
						/* below iovar to verify if the for foundout
						interface has already ucode been downloaded
						*/
						ret = wlu_iovar_getint(dev_handle, "ucdload_status",
							&ucdload_status);
						if (ret) {
							printf("err in ucdload_status, exiting\n");
							goto exit;
						}
						if (ucdload_status) {
							/* Number of 'wl' interfaces to skip
							in the next round of going thru wl_find
							*/
							printf("ucode is already downloaded\n");
						}
				}
			/* usb seems to take some time to come up, hence the
			loop value of 100
			*/
			} while (loopcnt < DEVPRESENT_RETRIES && !is_devpresent);

			if (loopcnt < DEVPRESENT_RETRIES) {
				/* download the ucode fw */
				ret = dload_ucode_part(dev_handle, UCODE_FW, ucode_len,
					ucodetrx_buf);
				if (ret) {
					printf("error while downloading ucode, exiting\n");
					goto exit;
				}
				/* download the initvals to the dongle */
				ret = dload_ucode_part(dev_handle, INIT_VALS,
					initvals_len, initvals_ptr);

				if (ret) {
					printf("error while downloading initvals, exiting\n");
					goto exit;
				}
			}
			else {
				printf("wl device is not present\n");
			}
		}
	}

exit:
	if (ucode_trx_hdr)
		free(ucode_trx_hdr);
	if (ucodetrx_buf)
		free(ucodetrx_buf);

	if (fp)
		fclose(fp);
	return ret;
}
