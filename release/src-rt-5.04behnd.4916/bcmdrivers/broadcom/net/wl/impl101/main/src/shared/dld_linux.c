/*
 * dld - debugability support for dumping logs to file (debug log dump)
 *
 * Copyright (C) 2023, Broadcom. All Rights Reserved.
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
 * $Id$
 */
#include <osl.h>
#include <bcmutils.h>
#include <bcmendian.h>
#include <linux/rtc.h>
#include <dld.h>

#define DLD_ERROR(args) printf args

/*
 * Temporarily change log dump lock to spin_lock_irqsave as DHD_ERROR/DHD_LOG_MEM
 * are being called from dhdpcie_bus_isr.
 * This will be reverted after proper solution is implemented to handle isr prints
 */
#define DLD_BUF_LOCK(lock, flags)	spin_lock_irqsave((lock), (flags))
#define DLD_BUF_UNLOCK(lock, flags)	spin_unlock_irqrestore((lock), (flags))

#define DLD_MAX_TEMP_BUFFER_SIZE	512

/* Structure for debug log dump ring buffer */
typedef struct dld_buf {
	dld_buf_type_t type;
	spinlock_t lock;
	bool enable;
	unsigned int wraparound;
	unsigned int size;
	unsigned int remain;
	char* present;
	char* buffer;
	char* max;
} dld_buf_t;

#define DLD_MAX_BUFFER_SZ	(1024 * 1024)

/* Structure for debug log dump */
typedef struct dld_info {
	bool inited;
	osl_t *osh;
	struct mutex logdump_lock;
	uint buf_cnt;
	dld_buf_t buf[DLD_BUF_MAX];
} dld_info_t;

static void
dld_lock(void *info)
{
	dld_info_t *dld_info = (dld_info_t *)info;

	if (dld_info && dld_info->inited) {
		mutex_lock(&dld_info->logdump_lock);
	}
}

static void
dld_unlock(void *info)
{
	dld_info_t *dld_info = (dld_info_t *)info;

	if (dld_info && dld_info->inited) {
		mutex_unlock(&dld_info->logdump_lock);
	}
}

/* Init debug log dump */
void *
dld_init(osl_t *osh)
{
	dld_info_t *dld_info;

	if (osh == NULL) {
		DLD_ERROR(("%s invalid argument\n", __FUNCTION__));
		return NULL;
	}

	dld_info = MALLOCZ(osh, sizeof(*dld_info));
	if (!dld_info) {
		DLD_ERROR(("%s malloc failed\n", __FUNCTION__));
		return NULL;
	}

	mutex_init(&dld_info->logdump_lock);

	dld_info->osh = osh;
	dld_info->inited = TRUE;

	return dld_info;
}

/* Denit debug log dump */
void
dld_deinit(void *info)
{
	int i;
	dld_info_t *dld_info = (dld_info_t *)info;

	if (!dld_info || !dld_info->inited) {
		DLD_ERROR(("arg is invalid\n"));
		return;
	}

	for (i = 0; i < DLD_BUF_MAX; i++) {
		dld_buf_deinit(dld_info, i);
	}

	dld_info->inited = FALSE;
	MFREE(dld_info->osh, dld_info, sizeof(*dld_info));
}

/* Debug log dump buffer allocation and init */
int
dld_buf_init(void *info, const dld_buf_alloc_info_t *allocinfo, uint input_count)
{
	dld_info_t *dld_info = (dld_info_t *)info;
	dld_buf_t *dld_buf;
	unsigned int i;
	unsigned int size, type;
	int ret = BCME_OK;

	if (!dld_info || !allocinfo) {
		DLD_ERROR(("arg is null\n"));
		return BCME_BADARG;
	}

	if (!dld_info->inited) {
		DLD_ERROR(("Not initiailized\n"));
		return BCME_NOTREADY;
	}

	dld_lock(dld_info);

	if (input_count + dld_info->buf_cnt > DLD_BUF_MAX) {
		DLD_ERROR(("input buffer cnt is invalid\n"));
		ret = BCME_BADARG;
		goto exit;
	}

	for (i = 0; i < input_count; i++) {
		size = allocinfo[i].size;
		type = allocinfo[i].type;
		if (size > DLD_MAX_BUFFER_SZ || type > DLD_BUF_MAX) {
			DLD_ERROR(("input buffer param is invalid size %d, type %d\n",
				size, type));
			ret = BCME_ERROR;
			goto exit;
		}

		dld_buf = &dld_info->buf[type];
		if (dld_buf->enable == TRUE) {
			DLD_ERROR(("already enabled type %d\n", type));
			ret = BCME_BADARG;
			goto exit;
		}

		dld_buf->buffer = MALLOCZ(dld_info->osh, size);
		if (!dld_buf->buffer) {
			DLD_ERROR(("%s malloc failed\n", __FUNCTION__));
			ret = BCME_NOMEM;
			goto exit;
		}

		spin_lock_init(&dld_buf->lock);
		dld_buf->wraparound = 0;
		dld_buf->max = dld_buf->buffer + size;
		dld_buf->size = size;
		dld_buf->present = dld_buf->buffer;
		dld_buf->remain = size;
		dld_buf->type = type;

		dld_buf->enable = TRUE;
		dld_info->buf_cnt++;
	}

exit:
	dld_unlock(dld_info);

	return ret;
}

/* Debug log dump buffer deinit */
void
dld_buf_deinit(void *info, dld_buf_type_t type)
{
	dld_info_t *dld_info = (dld_info_t *)info;
	dld_buf_t *dld_buf;
	unsigned int size;
	unsigned long flags = 0;

	if (!dld_info || !dld_info->inited) {
		return;
	}

	if (type >= DLD_BUF_MAX) {
		return;
	}

	dld_lock(dld_info);

	dld_buf = &dld_info->buf[type];
	if (dld_buf->enable != TRUE) {
		goto exit;
	}
	size = dld_buf->max - dld_buf->buffer;
	DLD_BUF_LOCK(&dld_buf->lock, flags);
	MFREE(dld_info->osh, dld_buf->buffer, size);
	dld_buf->enable = FALSE;
	dld_buf->present = NULL;
	dld_buf->size = dld_buf->remain = 0;
	dld_buf->type = DLD_BUF_MAX;
	DLD_BUF_UNLOCK(&dld_buf->lock, flags);
	dld_info->buf_cnt--;

exit:
	dld_unlock(dld_info);

}

/* Write log to debug log dump memory */
void
dld_write(void *info, dld_buf_type_t type, const char *fmt, ...)
{
	int len = 0;
	char tmp_buf[DLD_MAX_TEMP_BUFFER_SIZE] = {0, };
	va_list args;
	unsigned long flags = 0;
	dld_buf_t *dld_buf = NULL;
	dld_info_t *dld_info = (dld_info_t *)info;
	char *src = tmp_buf;

	if (!dld_info || !dld_info->inited) {
		return;
	}

	if (type >= DLD_BUF_MAX) {
		DLD_ERROR(("%s: Unsupported BUF_TYPE(%d).\n",
			__FUNCTION__, type));
		return;
	}

	dld_buf = &dld_info->buf[type];
	if (dld_buf->enable != TRUE) {
		return;
	}

	va_start(args, fmt);
	len = vsnprintf(tmp_buf, DLD_MAX_TEMP_BUFFER_SIZE, fmt, args);
	/* Non ANSI C99 compliant returns -1,
	 * ANSI compliant return len >= DHD_LOG_DUMP_MAX_TEMP_BUFFER_SIZE
	 */
	va_end(args);
	if (len < 0) {
		return;
	}

	/* Fit the string to the buffer */
	if (len > DLD_MAX_TEMP_BUFFER_SIZE) {
		len = DLD_MAX_TEMP_BUFFER_SIZE;
	}

	/* make a critical section to eliminate race conditions */
	DLD_BUF_LOCK(&dld_buf->lock, flags);

	/* If there is not enough space, copy the string as much as possible */
	if (dld_buf->remain < len) {
		dld_buf->wraparound = 1;
		memcpy(dld_buf->present, src, dld_buf->remain);
		len -= dld_buf->remain;
		src += dld_buf->remain;
		dld_buf->present = dld_buf->buffer;
		dld_buf->remain = dld_buf->size;
	}

	/* Write the (remaining) string to debug log dump buffer */
	memcpy(dld_buf->present, src, len);

	/* Reduce the remaining buffer length */
	dld_buf->remain -= len;
	/* Move buffer pointer as much as the len of the string */
	dld_buf->present += len;
	DLD_BUF_UNLOCK(&dld_buf->lock, flags);

	/* double check invalid memory operation */
	ASSERT(dld_buf->present <= dld_buf->max);
}

/* Get debug log dump buffer */
int
dld_get_buf(void *info, dld_buf_type_t type, char *buf, uint *size)
{
	dld_info_t *dld_info = (dld_info_t *)info;
	dld_buf_t *dld_buf = NULL;
	unsigned long flags = 0;
	int ret = BCME_OK;

	if (!dld_info || !dld_info->inited || !buf || !size || !*size) {
		return BCME_BADARG;
	}

	if (type >= DLD_BUF_MAX) {
		DLD_ERROR(("%s: Unsupported BUF_TYPE(%d).\n",
			__FUNCTION__, type));
		return BCME_BADARG;
	}

	dld_lock(dld_info);

	dld_buf = &dld_info->buf[type];
	if (dld_buf->enable != TRUE) {
		ret = BCME_NORESOURCE;
		goto exit;
	}

	if (*size < dld_buf->size) {
		DLD_ERROR(("dld read buffer size %d is smaller than buf %d\n",
			*size, dld_buf->size));
		ret = BCME_BUFTOOSHORT;
		goto exit;
	}

	DLD_BUF_LOCK(&dld_buf->lock, flags);

	*size = 0;

	/* In case wraparound, start from the current position */
	if (dld_buf->wraparound) {
		memcpy(buf, dld_buf->present,
			(unsigned int)(dld_buf->max - dld_buf->present));
		*size += dld_buf->max - dld_buf->present;
	}
	/* Read from the start of the buffer */
	memcpy(buf + *size, dld_buf->buffer,
		(unsigned int)(dld_buf->present - dld_buf->buffer));
	*size += (unsigned int)(dld_buf->present - dld_buf->buffer);

	/* Null terminate buffer */
	if (*size < dld_buf->size) {
		buf[*size] = '\0';
	} else {
		buf[dld_buf->size - 1] = '\0';
	}

	/* Restore the pointers after uploading the buffer */
	dld_buf->wraparound = 0;
	dld_buf->present = dld_buf->buffer;
	dld_buf->remain = dld_buf->size;

	DLD_BUF_UNLOCK(&dld_buf->lock, flags);

exit:
	dld_unlock(dld_info);

	return ret;
}
