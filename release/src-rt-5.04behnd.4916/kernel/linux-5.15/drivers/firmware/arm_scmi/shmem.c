// SPDX-License-Identifier: GPL-2.0
/*
 * For transport using shared mem structure.
 *
 * Copyright (C) 2019 ARM Ltd.
 */

#include <linux/io.h>
#include <linux/of.h>
#include <linux/processor.h>
#include <linux/types.h>

#include "common.h"

/*
 * SCMI specification requires all parameters, message headers, return
 * arguments or any protocol data to be expressed in little endian
 * format only.
 */
struct scmi_shared_mem {
	__le32 reserved;
	__le32 channel_status;
#define SCMI_SHMEM_CHAN_STAT_CHANNEL_ERROR	BIT(1)
#define SCMI_SHMEM_CHAN_STAT_CHANNEL_FREE	BIT(0)
	__le32 reserved1[2];
	__le32 flags;
#define SCMI_SHMEM_FLAG_INTR_ENABLED	BIT(0)
	__le32 length;
	__le32 msg_header;
	u8 msg_payload[];
};

static inline void shmem_memcpy_fromio32(void *to,
					 const void __iomem *from,
					 size_t count)
{
	WARN_ON(!IS_ALIGNED((unsigned long)from, 4) ||
		!IS_ALIGNED((unsigned long)to, 4) ||
		count % 4);

	__ioread32_copy(to, from, count / 4);
}

static inline void shmem_memcpy_toio32(void __iomem *to,
				       const void *from,
				       size_t count)
{
	WARN_ON(!IS_ALIGNED((unsigned long)from, 4) ||
		!IS_ALIGNED((unsigned long)to, 4) ||
		count % 4);

	__iowrite32_copy(to, from, count / 4);
}

static struct scmi_shmem_io_ops shmem_io_ops32 = {
	.fromio	= shmem_memcpy_fromio32,
	.toio	= shmem_memcpy_toio32,
};

/* Wrappers are needed for proper memcpy_{from,to}_io expansion by the
 * pre-processor.
 */
static inline void shmem_memcpy_fromio(void *to,
				       const void __iomem *from,
				       size_t count)
{
	memcpy_fromio(to, from, count);
}

static inline void shmem_memcpy_toio(void __iomem *to,
				     const void *from,
				     size_t count)
{
	memcpy_toio(to, from, count);
}

static struct scmi_shmem_io_ops shmem_io_ops_default = {
	.fromio = shmem_memcpy_fromio,
	.toio	= shmem_memcpy_toio,
};

void shmem_tx_prepare(struct scmi_shared_mem __iomem *shmem,
		      struct scmi_xfer *xfer, shmem_copy_toio_t copy_toio)
{
	/*
	 * Ideally channel must be free by now unless OS timeout last
	 * request and platform continued to process the same, wait
	 * until it releases the shared memory, otherwise we may endup
	 * overwriting its response with new message payload or vice-versa
	 */
	spin_until_cond(ioread32(&shmem->channel_status) &
			SCMI_SHMEM_CHAN_STAT_CHANNEL_FREE);
	/* Mark channel busy + clear error */
	iowrite32(0x0, &shmem->channel_status);
	iowrite32(xfer->hdr.poll_completion ? 0 : SCMI_SHMEM_FLAG_INTR_ENABLED,
		  &shmem->flags);
	iowrite32(sizeof(shmem->msg_header) + xfer->tx.len, &shmem->length);
	iowrite32(pack_scmi_header(&xfer->hdr), &shmem->msg_header);
	if (xfer->tx.buf)
		copy_toio(shmem->msg_payload, xfer->tx.buf, xfer->tx.len);
}

u32 shmem_read_header(struct scmi_shared_mem __iomem *shmem)
{
	return ioread32(&shmem->msg_header);
}

void shmem_fetch_response(struct scmi_shared_mem __iomem *shmem,
			  struct scmi_xfer *xfer,
			  shmem_copy_fromio_t copy_fromio)
{
	size_t len = ioread32(&shmem->length);

	xfer->hdr.status = ioread32(shmem->msg_payload);
	/* Skip the length of header and status in shmem area i.e 8 bytes */
	xfer->rx.len = min_t(size_t, xfer->rx.len, len > 8 ? len - 8 : 0);

	/* Take a copy to the rx buffer.. */
	copy_fromio(xfer->rx.buf, shmem->msg_payload + 4, xfer->rx.len);
}

void shmem_fetch_notification(struct scmi_shared_mem __iomem *shmem,
			      size_t max_len, struct scmi_xfer *xfer,
			      shmem_copy_fromio_t copy_fromio)
{
	size_t len = ioread32(&shmem->length);

	/* Skip only the length of header in shmem area i.e 4 bytes */
	xfer->rx.len = min_t(size_t, max_len, len > 4 ? len - 4 : 0);

	/* Take a copy to the rx buffer.. */
	copy_fromio(xfer->rx.buf, shmem->msg_payload, xfer->rx.len);
}

void shmem_clear_channel(struct scmi_shared_mem __iomem *shmem)
{
	iowrite32(SCMI_SHMEM_CHAN_STAT_CHANNEL_FREE, &shmem->channel_status);
}

bool shmem_poll_done(struct scmi_shared_mem __iomem *shmem,
		     struct scmi_xfer *xfer)
{
	u16 xfer_id;

	xfer_id = MSG_XTRACT_TOKEN(ioread32(&shmem->msg_header));

	if (xfer->hdr.seq != xfer_id)
		return false;

	return ioread32(&shmem->channel_status) &
		(SCMI_SHMEM_CHAN_STAT_CHANNEL_ERROR |
		 SCMI_SHMEM_CHAN_STAT_CHANNEL_FREE);
}

bool shmem_channel_free(struct scmi_shared_mem __iomem *shmem)
{
	return (ioread32(&shmem->channel_status) &
			SCMI_SHMEM_CHAN_STAT_CHANNEL_FREE);
}

struct scmi_shmem_io_ops *shmem_get_io_ops(struct device_node *shmem)
{
	u32 reg_io_width;

	of_property_read_u32(shmem, "reg-io-width", &reg_io_width);
	switch (reg_io_width) {
	case 4:
		return &shmem_io_ops32;
	}

	return &shmem_io_ops_default;
}
