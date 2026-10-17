// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) 2007-2008 Samuel Thibault.
 * (C) Copyright 2020 EPAM Systems Inc.
 */

#define LOG_CATEGORY UCLASS_PVBLOCK

#include <blk.h>
#include <common.h>
#include <dm.h>
#include <dm/device-internal.h>
#include <malloc.h>
#include <part.h>

#include <asm/armv8/mmu.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/xen/system.h>

#include <linux/bug.h>
#include <linux/compat.h>

#include <xen/events.h>
#include <xen/gnttab.h>
#include <xen/hvm.h>
#include <xen/xenbus.h>

#include <xen/interface/io/ring.h>
#include <xen/interface/io/blkif.h>
#include <xen/interface/io/protocols.h>

#define DRV_NAME	"pvblock"
#define DRV_NAME_BLK	"pvblock_blk"

#define O_RDONLY	00
#define O_RDWR		02
#define WAIT_RING_TO_MS	10

struct blkfront_info {
	u64 sectors;
	unsigned int sector_size;
	int mode;
	int info;
	int barrier;
	int flush;
};

/**
 * struct blkfront_dev - Struct representing blkfront device
 * @dom: Domain id
 * @ring: Front_ring structure
 * @ring_ref: The grant reference, allowing us to grant access
 *	      to the ring to the other end/domain
 * @evtchn: Event channel used to signal ring events
 * @handle: Events handle
 * @nodename: Device XenStore path in format "device/vbd/" + @devid
 * @backend: Backend XenStore path
 * @info: Private data
 * @devid: Device id
 */
struct blkfront_dev {
	domid_t dom;

	struct blkif_front_ring ring;
	grant_ref_t ring_ref;
	evtchn_port_t evtchn;
	blkif_vdev_t handle;

	char *nodename;
	char *backend;
	struct blkfront_info info;
	unsigned int devid;
	u8 *bounce_buffer;
};

struct blkfront_plat {
	unsigned int devid;
};

/**
 * struct blkfront_aiocb - AIO сontrol block
 * @aio_dev: Blockfront device
 * @aio_buf: Memory buffer, which must be sector-aligned for
 *	     @aio_dev sector
 * @aio_nbytes: Size of AIO, which must be less than @aio_dev
 *		sector-sized amounts
 * @aio_offset: Offset, which must not go beyond @aio_dev
 *		sector-aligned location
 * @data: Data used to receiving response from ring
 * @gref: Array of grant references
 * @n: Number of segments
 * @aio_cb: Represents one I/O request.
 */
struct blkfront_aiocb {
	struct blkfront_dev *aio_dev;
	u8 *aio_buf;
	size_t aio_nbytes;
	off_t aio_offset;
	void *data;

	grant_ref_t gref[BLKIF_MAX_SEGMENTS_PER_REQUEST];
	int n;

	void (*aio_cb)(struct blkfront_aiocb *aiocb, int ret);
};

static void blkfront_sync(struct blkfront_dev *dev);

static void free_blkfront(struct blkfront_dev *dev)
{
	mask_evtchn(dev->evtchn);
	free(dev->backend);

	gnttab_end_access(dev->ring_ref);
	free(dev->ring.sring);

	unbind_evtchn(dev->evtchn);

	free(dev->bounce_buffer);
	free(dev->nodename);
	free(dev);
}

static int init_blkfront(unsigned int devid, struct blkfront_dev *dev)
{
	xenbus_transaction_t xbt;
	char *err = NULL;
	char *message = NULL;
	struct blkif_sring *s;
	int retry = 0;
	char *msg = NULL;
	char *c;
	char nodename[32];
	char path[ARRAY_SIZE(nodename) + strlen("/backend-id") + 1];

	sprintf(nodename, "device/vbd/%d", devid);

	memset(dev, 0, sizeof(*dev));
	dev->nodename = strdup(nodename);
	dev->devid = devid;

	snprintf(path, sizeof(path), "%s/backend-id", nodename);
	dev->dom = xenbus_read_integer(path);
	evtchn_alloc_unbound(dev->dom, NULL, dev, &dev->evtchn);

	s = (struct blkif_sring *)memalign(PAGE_SIZE, PAGE_SIZE);
	if (!s) {
		printf("Failed to allocate shared ring\n");
		goto error;
	}

	SHARED_RING_INIT(s);
	FRONT_RING_INIT(&dev->ring, s, PAGE_SIZE);

	dev->ring_ref = gnttab_grant_access(dev->dom, virt_to_pfn(s), 0);

again:
	err = xenbus_transaction_start(&xbt);
	if (err) {
		printf("starting transaction\n");
		free(err);
	}

	err = xenbus_printf(xbt, nodename, "ring-ref", "%u", dev->ring_ref);
	if (err) {
		message = "writing ring-ref";
		goto abort_transaction;
	}
	err = xenbus_printf(xbt, nodename, "event-channel", "%u", dev->evtchn);
	if (err) {
		message = "writing event-channel";
		goto abort_transaction;
	}
	err = xenbus_printf(xbt, nodename, "protocol", "%s",
			    XEN_IO_PROTO_ABI_NATIVE);
	if (err) {
		message = "writing protocol";
		goto abort_transaction;
	}

	snprintf(path, sizeof(path), "%s/state", nodename);
	err = xenbus_switch_state(xbt, path, XenbusStateConnected);
	if (err) {
		message = "switching state";
		goto abort_transaction;
	}

	err = xenbus_transaction_end(xbt, 0, &retry);
	free(err);
	if (retry) {
		goto again;
		printf("completing transaction\n");
	}

	goto done;

abort_transaction:
	free(err);
	err = xenbus_transaction_end(xbt, 1, &retry);
	printf("Abort transaction %s\n", message);
	goto error;

done:
	snprintf(path, sizeof(path), "%s/backend", nodename);
	msg = xenbus_read(XBT_NIL, path, &dev->backend);
	if (msg) {
		printf("Error %s when reading the backend path %s\n",
		       msg, path);
		goto error;
	}

	dev->handle = strtoul(strrchr(nodename, '/') + 1, NULL, 0);

	{
		XenbusState state;
		char path[strlen(dev->backend) +
			strlen("/feature-flush-cache") + 1];

		snprintf(path, sizeof(path), "%s/mode", dev->backend);
		msg = xenbus_read(XBT_NIL, path, &c);
		if (msg) {
			printf("Error %s when reading the mode\n", msg);
			goto error;
		}
		if (*c == 'w')
			dev->info.mode = O_RDWR;
		else
			dev->info.mode = O_RDONLY;
		free(c);

		snprintf(path, sizeof(path), "%s/state", dev->backend);

		msg = NULL;
		state = xenbus_read_integer(path);
		while (!msg && state < XenbusStateConnected)
			msg = xenbus_wait_for_state_change(path, &state);
		if (msg || state != XenbusStateConnected) {
			printf("backend not available, state=%d\n", state);
			goto error;
		}

		snprintf(path, sizeof(path), "%s/info", dev->backend);
		dev->info.info = xenbus_read_integer(path);

		snprintf(path, sizeof(path), "%s/sectors", dev->backend);
		/*
		 * FIXME: read_integer returns an int, so disk size
		 * limited to 1TB for now
		 */
		dev->info.sectors = xenbus_read_integer(path);

		snprintf(path, sizeof(path), "%s/sector-size", dev->backend);
		dev->info.sector_size = xenbus_read_integer(path);

		snprintf(path, sizeof(path), "%s/feature-barrier",
			 dev->backend);
		dev->info.barrier = xenbus_read_integer(path);

		snprintf(path, sizeof(path), "%s/feature-flush-cache",
			 dev->backend);
		dev->info.flush = xenbus_read_integer(path);
	}
	unmask_evtchn(dev->evtchn);

	dev->bounce_buffer = memalign(dev->info.sector_size,
				      dev->info.sector_size);
	if (!dev->bounce_buffer) {
		printf("Failed to allocate bouncing buffer\n");
		goto error;
	}

	debug("%llu sectors of %u bytes, bounce buffer at %p\n",
	      dev->info.sectors, dev->info.sector_size,
	      dev->bounce_buffer);

	return 0;

error:
	free(msg);
	free(err);
	free_blkfront(dev);
	return -ENODEV;
}

static void shutdown_blkfront(struct blkfront_dev *dev)
{
	char *err = NULL, *err2;
	XenbusState state;

	char path[strlen(dev->backend) + strlen("/state") + 1];
	char nodename[strlen(dev->nodename) + strlen("/event-channel") + 1];

	debug("Close " DRV_NAME ", device ID %d\n", dev->devid);

	blkfront_sync(dev);

	snprintf(path, sizeof(path), "%s/state", dev->backend);
	snprintf(nodename, sizeof(nodename), "%s/state", dev->nodename);

	err = xenbus_switch_state(XBT_NIL, nodename, XenbusStateClosing);
	if (err) {
		printf("%s: error changing state to %d: %s\n", __func__,
		       XenbusStateClosing, err);
		goto close;
	}

	state = xenbus_read_integer(path);
	while (!err && state < XenbusStateClosing)
		err = xenbus_wait_for_state_change(path, &state);
	free(err);

	err = xenbus_switch_state(XBT_NIL, nodename, XenbusStateClosed);
	if (err) {
		printf("%s: error changing state to %d: %s\n", __func__,
		       XenbusStateClosed, err);
		goto close;
	}

	state = xenbus_read_integer(path);
	while (state < XenbusStateClosed) {
		err = xenbus_wait_for_state_change(path, &state);
		free(err);
	}

	err = xenbus_switch_state(XBT_NIL, nodename, XenbusStateInitialising);
	if (err) {
		printf("%s: error changing state to %d: %s\n", __func__,
		       XenbusStateInitialising, err);
		goto close;
	}

	state = xenbus_read_integer(path);
	while (!err &&
	       (state < XenbusStateInitWait || state >= XenbusStateClosed))
		err = xenbus_wait_for_state_change(path, &state);

close:
	free(err);

	snprintf(nodename, sizeof(nodename), "%s/ring-ref", dev->nodename);
	err2 = xenbus_rm(XBT_NIL, nodename);
	free(err2);
	snprintf(nodename, sizeof(nodename), "%s/event-channel", dev->nodename);
	err2 = xenbus_rm(XBT_NIL, nodename);
	free(err2);

	if (!err)
		free_blkfront(dev);
}

/**
 * blkfront_aio_poll() - AIO polling function.
 * @dev: Blkfront device
 *
 * Here we receive response from the ring and check its status. This happens
 * until we read all data from the ring. We read the data from consumed pointer
 * to the response pointer. Then increase consumed pointer to make it clear that
 * the data has been read.
 *
 * Return: Number of consumed bytes.
 */
static int blkfront_aio_poll(struct blkfront_dev *dev)
{
	RING_IDX rp, cons;
	struct blkif_response *rsp;
	int more;
	int nr_consumed;

moretodo:
	rp = dev->ring.sring->rsp_prod;
	rmb(); /* Ensure we see queued responses up to 'rp'. */
	cons = dev->ring.rsp_cons;

	nr_consumed = 0;
	while ((cons != rp)) {
		struct blkfront_aiocb *aiocbp;
		int status;

		rsp = RING_GET_RESPONSE(&dev->ring, cons);
		nr_consumed++;

		aiocbp = (void *)(uintptr_t)rsp->id;
		status = rsp->status;

		switch (rsp->operation) {
		case BLKIF_OP_READ:
		case BLKIF_OP_WRITE:
		{
			int j;

			if (status != BLKIF_RSP_OKAY)
				printf("%s error %d on %s at offset %llu, num bytes %llu\n",
				       rsp->operation == BLKIF_OP_READ ?
				       "read" : "write",
				       status, aiocbp->aio_dev->nodename,
				       (unsigned long long)aiocbp->aio_offset,
				       (unsigned long long)aiocbp->aio_nbytes);

			for (j = 0; j < aiocbp->n; j++)
				gnttab_end_access(aiocbp->gref[j]);

			break;
		}

		case BLKIF_OP_WRITE_BARRIER:
			if (status != BLKIF_RSP_OKAY)
				printf("write barrier error %d\n", status);
			break;
		case BLKIF_OP_FLUSH_DISKCACHE:
			if (status != BLKIF_RSP_OKAY)
				printf("flush error %d\n", status);
			break;

		default:
			printf("unrecognized block operation %d response (status %d)\n",
			       rsp->operation, status);
			break;
		}

		dev->ring.rsp_cons = ++cons;
		/* Nota: callback frees aiocbp itself */
		if (aiocbp && aiocbp->aio_cb)
			aiocbp->aio_cb(aiocbp, status ? -EIO : 0);
		if (dev->ring.rsp_cons != cons)
			/* We reentered, we must not continue here */
			break;
	}

	RING_FINAL_CHECK_FOR_RESPONSES(&dev->ring, more);
	if (more)
		goto moretodo;

	return nr_consumed;
}

static void blkfront_wait_slot(struct blkfront_dev *dev)
{
	/* Wait for a slot */
	if (RING_FULL(&dev->ring)) {
		while (true) {
			blkfront_aio_poll(dev);
			if (!RING_FULL(&dev->ring))
				break;
			wait_event_timeout(NULL, !RING_FULL(&dev->ring),
					   WAIT_RING_TO_MS);
		}
	}
}

/**
 * blkfront_aio_poll() - Issue an aio.
 * @aiocbp: AIO control block structure
 * @write: Describes is it read or write operation
 *	   0 - read
 *	   1 - write
 *
 * We check whether the AIO parameters meet the requirements of the device.
 * Then receive request from ring and define its arguments. After this we
 * grant access to the grant references. The last step is notifying about AIO
 * via event channel.
 */
static void blkfront_aio(struct blkfront_aiocb *aiocbp, int write)
{
	struct blkfront_dev *dev = aiocbp->aio_dev;
	struct blkif_request *req;
	RING_IDX i;
	int notify;
	int n, j;
	uintptr_t start, end;

	/* Can't io at non-sector-aligned location */
	BUG_ON(aiocbp->aio_offset & (dev->info.sector_size - 1));
	/* Can't io non-sector-sized amounts */
	BUG_ON(aiocbp->aio_nbytes & (dev->info.sector_size - 1));
	/* Can't io non-sector-aligned buffer */
	BUG_ON(((uintptr_t)aiocbp->aio_buf & (dev->info.sector_size - 1)));

	start = (uintptr_t)aiocbp->aio_buf & PAGE_MASK;
	end = ((uintptr_t)aiocbp->aio_buf + aiocbp->aio_nbytes +
	       PAGE_SIZE - 1) & PAGE_MASK;
	n = (end - start) / PAGE_SIZE;
	aiocbp->n = n;

	BUG_ON(n > BLKIF_MAX_SEGMENTS_PER_REQUEST);

	blkfront_wait_slot(dev);
	i = dev->ring.req_prod_pvt;
	req = RING_GET_REQUEST(&dev->ring, i);

	req->operation = write ? BLKIF_OP_WRITE : BLKIF_OP_READ;
	req->nr_segments = n;
	req->handle = dev->handle;
	req->id = (uintptr_t)aiocbp;
	req->sector_number = aiocbp->aio_offset / dev->info.sector_size;

	for (j = 0; j < n; j++) {
		req->seg[j].first_sect = 0;
		req->seg[j].last_sect = PAGE_SIZE / dev->info.sector_size - 1;
	}
	req->seg[0].first_sect = ((uintptr_t)aiocbp->aio_buf & ~PAGE_MASK) /
		dev->info.sector_size;
	req->seg[n - 1].last_sect = (((uintptr_t)aiocbp->aio_buf +
		aiocbp->aio_nbytes - 1) & ~PAGE_MASK) / dev->info.sector_size;
	for (j = 0; j < n; j++) {
		uintptr_t data = start + j * PAGE_SIZE;

		if (!write) {
			/* Trigger CoW if needed */
			*(char *)(data + (req->seg[j].first_sect *
					  dev->info.sector_size)) = 0;
			barrier();
		}
		req->seg[j].gref = gnttab_grant_access(dev->dom,
						       virt_to_pfn((void *)data),
						       write);
		aiocbp->gref[j] = req->seg[j].gref;
	}

	dev->ring.req_prod_pvt = i + 1;

	wmb();
	RING_PUSH_REQUESTS_AND_CHECK_NOTIFY(&dev->ring, notify);

	if (notify)
		notify_remote_via_evtchn(dev->evtchn);
}

static void blkfront_aio_cb(struct blkfront_aiocb *aiocbp, int ret)
{
	aiocbp->data = (void *)1;
	aiocbp->aio_cb = NULL;
}

static void blkfront_io(struct blkfront_aiocb *aiocbp, int write)
{
	aiocbp->aio_cb = blkfront_aio_cb;
	blkfront_aio(aiocbp, write);
	aiocbp->data = NULL;

	while (true) {
		blkfront_aio_poll(aiocbp->aio_dev);
		if (aiocbp->data)
			break;
		cpu_relax();
	}
}

static void blkfront_push_operation(struct blkfront_dev *dev, u8 op,
				    uint64_t id)
{
	struct blkif_request *req;
	int notify, i;

	blkfront_wait_slot(dev);
	i = dev->ring.req_prod_pvt;
	req = RING_GET_REQUEST(&dev->ring, i);
	req->operation = op;
	req->nr_segments = 0;
	req->handle = dev->handle;
	req->id = id;
	req->sector_number = 0;
	dev->ring.req_prod_pvt = i + 1;
	wmb();
	RING_PUSH_REQUESTS_AND_CHECK_NOTIFY(&dev->ring, notify);
	if (notify)
		notify_remote_via_evtchn(dev->evtchn);
}

static void blkfront_sync(struct blkfront_dev *dev)
{
	if (dev->info.mode == O_RDWR) {
		if (dev->info.barrier == 1)
			blkfront_push_operation(dev,
						BLKIF_OP_WRITE_BARRIER, 0);

		if (dev->info.flush == 1)
			blkfront_push_operation(dev,
						BLKIF_OP_FLUSH_DISKCACHE, 0);
	}

	while (true) {
		blkfront_aio_poll(dev);
		if (RING_FREE_REQUESTS(&dev->ring) == RING_SIZE(&dev->ring))
			break;
		cpu_relax();
	}
}

/**
 * pvblock_iop() - Issue an aio.
 * @udev: Pvblock device
 * @blknr: Block number to read from / write to
 * @blkcnt: Amount of blocks to read / write
 * @buffer: Memory buffer with data to be read / write
 * @write: Describes is it read or write operation
 *	   0 - read
 *	   1 - write
 *
 * Depending on the operation - reading or writing, data is read / written from the
 * specified address (@buffer) to the sector (@blknr).
 */
static ulong pvblock_iop(struct udevice *udev, lbaint_t blknr,
			 lbaint_t blkcnt, void *buffer, int write)
{
	struct blkfront_dev *blk_dev = dev_get_priv(udev);
	struct blk_desc *desc = dev_get_uclass_plat(udev);
	struct blkfront_aiocb aiocb;
	lbaint_t blocks_todo;
	bool unaligned;

	if (blkcnt == 0)
		return 0;

	if ((blknr + blkcnt) > desc->lba) {
		printf(DRV_NAME ": block number 0x" LBAF " exceeds max(0x" LBAF ")\n",
		       blknr + blkcnt, desc->lba);
		return 0;
	}

	unaligned = (uintptr_t)buffer & (blk_dev->info.sector_size - 1);

	aiocb.aio_dev = blk_dev;
	aiocb.aio_offset = blknr * desc->blksz;
	aiocb.aio_cb = NULL;
	aiocb.data = NULL;
	blocks_todo = blkcnt;
	do {
		aiocb.aio_buf = unaligned ? blk_dev->bounce_buffer : buffer;

		if (write && unaligned)
			memcpy(blk_dev->bounce_buffer, buffer, desc->blksz);

		aiocb.aio_nbytes = unaligned ? desc->blksz :
			min((size_t)((BLKIF_MAX_SEGMENTS_PER_REQUEST - 1)
					* PAGE_SIZE),
			    (size_t)(blocks_todo * desc->blksz));

		blkfront_io(&aiocb, write);

		if (!write && unaligned)
			memcpy(buffer, blk_dev->bounce_buffer, desc->blksz);

		aiocb.aio_offset += aiocb.aio_nbytes;
		buffer += aiocb.aio_nbytes;
		blocks_todo -= aiocb.aio_nbytes / desc->blksz;
	} while (blocks_todo > 0);

	return blkcnt;
}

ulong pvblock_blk_read(struct udevice *udev, lbaint_t blknr, lbaint_t blkcnt,
		       void *buffer)
{
	return pvblock_iop(udev, blknr, blkcnt, buffer, 0);
}

ulong pvblock_blk_write(struct udevice *udev, lbaint_t blknr, lbaint_t blkcnt,
			const void *buffer)
{
	return pvblock_iop(udev, blknr, blkcnt, (void *)buffer, 1);
}

static int pvblock_blk_bind(struct udevice *udev)
{
	struct blk_desc *desc = dev_get_uclass_plat(udev);
	int devnum;

	desc->uclass_id = UCLASS_PVBLOCK;
	/*
	 * Initialize the devnum to -ENODEV. This is to make sure that
	 * blk_next_free_devnum() works as expected, since the default
	 * value 0 is a valid devnum.
	 */
	desc->devnum = -ENODEV;
	devnum = blk_next_free_devnum(UCLASS_PVBLOCK);
	if (devnum < 0)
		return devnum;
	desc->devnum = devnum;
	desc->part_type = PART_TYPE_UNKNOWN;
	desc->bdev = udev;

	strncpy(desc->vendor, "Xen", sizeof(desc->vendor));
	strncpy(desc->revision, "1", sizeof(desc->revision));
	strncpy(desc->product, "Virtual disk", sizeof(desc->product));

	return 0;
}

static int pvblock_blk_probe(struct udevice *udev)
{
	struct blkfront_dev *blk_dev = dev_get_priv(udev);
	struct blkfront_plat *plat = dev_get_plat(udev);
	struct blk_desc *desc = dev_get_uclass_plat(udev);
	int ret, devid;

	devid = plat->devid;
	free(plat);

	ret = init_blkfront(devid, blk_dev);
	if (ret < 0)
		return ret;

	desc->blksz = blk_dev->info.sector_size;
	desc->lba = blk_dev->info.sectors;
	desc->log2blksz = LOG2(blk_dev->info.sector_size);

	return 0;
}

static int pvblock_blk_remove(struct udevice *udev)
{
	struct blkfront_dev *blk_dev = dev_get_priv(udev);

	shutdown_blkfront(blk_dev);
	return 0;
}

static const struct blk_ops pvblock_blk_ops = {
	.read	= pvblock_blk_read,
	.write	= pvblock_blk_write,
};

U_BOOT_DRIVER(pvblock_blk) = {
	.name			= DRV_NAME_BLK,
	.id			= UCLASS_BLK,
	.ops			= &pvblock_blk_ops,
	.bind			= pvblock_blk_bind,
	.probe			= pvblock_blk_probe,
	.remove			= pvblock_blk_remove,
	.priv_auto	= sizeof(struct blkfront_dev),
	.flags			= DM_FLAG_OS_PREPARE,
};

/*******************************************************************************
 * Para-virtual block device class
 *******************************************************************************/

typedef int (*enum_vbd_callback)(struct udevice *parent, unsigned int devid);

static int on_new_vbd(struct udevice *parent, unsigned int devid)
{
	struct driver_info info;
	struct udevice *udev;
	struct blkfront_plat *plat;
	int ret;

	debug("New " DRV_NAME_BLK ", device ID %d\n", devid);

	plat = malloc(sizeof(struct blkfront_plat));
	if (!plat) {
		printf("Failed to allocate platform data\n");
		return -ENOMEM;
	}

	plat->devid = devid;

	info.name = DRV_NAME_BLK;
	info.plat = plat;

	ret = device_bind_by_name(parent, false, &info, &udev);
	if (ret < 0) {
		printf("Failed to bind " DRV_NAME_BLK " to device with ID %d, ret: %d\n",
		       devid, ret);
		free(plat);
	}
	return ret;
}

static int xenbus_enumerate_vbd(struct udevice *udev, enum_vbd_callback clb)
{
	char **dirs, *msg;
	int i, ret;

	msg = xenbus_ls(XBT_NIL, "device/vbd", &dirs);
	if (msg) {
		printf("Failed to read device/vbd directory: %s\n", msg);
		free(msg);
		return -ENODEV;
	}

	for (i = 0; dirs[i]; i++) {
		int devid;

		sscanf(dirs[i], "%d", &devid);
		ret = clb(udev, devid);
		if (ret < 0)
			goto fail;

		free(dirs[i]);
	}
	ret = 0;

fail:
	for (; dirs[i]; i++)
		free(dirs[i]);
	free(dirs);
	return ret;
}

static void print_pvblock_devices(void)
{
	struct udevice *udev;
	bool first = true;
	const char *class_name;

	class_name = uclass_get_name(UCLASS_PVBLOCK);
	for (blk_first_device(UCLASS_PVBLOCK, &udev); udev;
	     blk_next_device(&udev), first = false) {
		struct blk_desc *desc = dev_get_uclass_plat(udev);

		if (!first)
			puts(", ");
		printf("%s: %d", class_name, desc->devnum);
	}
	printf("\n");
}

void pvblock_init(void)
{
	struct driver_info info;
	int ret;

	/*
	 * At this point Xen drivers have already initialized,
	 * so we can instantiate the class driver and enumerate
	 * virtual block devices.
	 */
	info.name = DRV_NAME;
	ret = device_bind_by_name(gd->dm_root, false, &info, NULL);
	if (ret < 0)
		printf("Failed to bind " DRV_NAME ", ret: %d\n", ret);

	/* Bootstrap virtual block devices class driver */
	uclass_probe_all(UCLASS_PVBLOCK);

	print_pvblock_devices();
}

static int pvblock_probe(struct udevice *udev)
{
	struct uclass *uc;
	int ret;

	if (xenbus_enumerate_vbd(udev, on_new_vbd) < 0)
		return -ENODEV;

	ret = uclass_get(UCLASS_BLK, &uc);
	if (ret)
		return ret;
	uclass_foreach_dev_probe(UCLASS_BLK, udev);
	return 0;
}

U_BOOT_DRIVER(pvblock_drv) = {
	.name		= DRV_NAME,
	.id		= UCLASS_PVBLOCK,
	.probe		= pvblock_probe,
};

UCLASS_DRIVER(pvblock) = {
	.name		= DRV_NAME,
	.id		= UCLASS_PVBLOCK,
};
