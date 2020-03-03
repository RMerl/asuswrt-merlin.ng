/*
 * Copyright (c) 2014 Christoph Hellwig.
 */
#include <linux/sunrpc/svc.h>
#include <linux/exportfs.h>
#include <linux/nfs4.h>

#include "nfsd.h"
#include "blocklayoutxdr.h"

#define NFSDDBG_FACILITY	NFSDDBG_PNFS


__be32
nfsd4_block_encode_layoutget(struct xdr_stream *xdr,
		struct nfsd4_layoutget *lgp)
{
	struct pnfs_block_extent *b = lgp->lg_content;
	int len = sizeof(__be32) + 5 * sizeof(__be64) + sizeof(__be32);
	__be32 *p;

	p = xdr_reserve_space(xdr, sizeof(__be32) + len);
	if (!p)
		return nfserr_toosmall;

	*p++ = cpu_to_be32(len);
	*p++ = cpu_to_be32(1);		/* we always return a single extent */

	p = xdr_encode_opaque_fixed(p, &b->vol_id,
			sizeof(struct nfsd4_deviceid));
	p = xdr_encode_hyper(p, b->foff);
	p = xdr_encode_hyper(p, b->len);
	p = xdr_encode_hyper(p, b->soff);
	*p++ = cpu_to_be32(b->es);
	return 0;
}

static int
nfsd4_block_encode_volume(struct xdr_stream *xdr, struct pnfs_block_volume *b)
{
	__be32 *p;
	int len;

	switch (b->type) {
	case PNFS_BLOCK_VOLUME_SIMPLE:
		len = 4 + 4 + 8 + 4 + b->simple.sig_len;
		p = xdr_reserve_space(xdr, len);
		if (!p)
			return -ETOOSMALL;

		*p++ = cpu_to_be32(b->type);
		*p++ = cpu_to_be32(1);	/* single signature */
		p = xdr_encode_hyper(p, b->simple.offset);
		p = xdr_encode_opaque(p, b->simple.sig, b->simple.sig_len);
		break;
	default:
		return -ENOTSUPP;
	}

	return len;
}

__be32
nfsd4_block_encode_getdeviceinfo(struct xdr_stream *xdr,
		struct nfsd4_getdeviceinfo *gdp)
{
	struct pnfs_block_deviceaddr *dev = gdp->gd_device;
	int len = sizeof(__be32), ret, i;
	__be32 *p;

	p = xdr_reserve_space(xdr, len + sizeof(__be32));
	if (!p)
		return nfserr_resource;

	for (i = 0; i < dev->nr_volumes; i++) {
		ret = nfsd4_block_encode_volume(xdr, &dev->volumes[i]);
		if (ret < 0)
			return nfserrno(ret);
		len += ret;
	}

	/*
	 * Fill in the overall length and number of volumes at the beginning
	 * of the layout.
	 */
	*p++ = cpu_to_be32(len);
	*p++ = cpu_to_be32(dev->nr_volumes);
	return 0;
}

int
nfsd4_block_decode_layoutupdate(__be32 *p, u32 len, struct iomap **iomapp,
		u32 block_size)
{
	struct iomap *iomaps;
	u32 nr_iomaps, expected, i;

	if (len < sizeof(u32)) {
		dprintk("%s: extent array too small: %u\n", __func__, len);
		return -EINVAL;
	}

	nr_iomaps = be32_to_cpup(p++);
	expected = sizeof(__be32) + nr_iomaps * NFS4_BLOCK_EXTENT_SIZE;
	if (len != expected) {
		dprintk("%s: extent array size mismatch: %u/%u\n",
			__func__, len, expected);
		return -EINVAL;
	}

	iomaps = kcalloc(nr_iomaps, sizeof(*iomaps), GFP_KERNEL);
	if (!iomaps) {
		dprintk("%s: failed to allocate extent array\n", __func__);
		return -ENOMEM;
	}

	for (i = 0; i < nr_iomaps; i++) {
		struct pnfs_block_extent bex;

		memcpy(&bex.vol_id, p, sizeof(struct nfsd4_deviceid));
		p += XDR_QUADLEN(sizeof(struct nfsd4_deviceid));

		p = xdr_decode_hyper(p, &bex.foff);
		if (bex.foff & (block_size - 1)) {
			dprintk("%s: unaligned offset 0x%llx\n",
				__func__, bex.foff);
			goto fail;
		}
		p = xdr_decode_hyper(p, &bex.len);
		if (bex.len & (block_size - 1)) {
			dprintk("%s: unaligned length 0x%llx\n",
				__func__, bex.foff);
			goto fail;
		}
		p = xdr_decode_hyper(p, &bex.soff);
		if (bex.soff & (block_size - 1)) {
			dprintk("%s: unaligned disk offset 0x%llx\n",
				__func__, bex.soff);
			goto fail;
		}
		bex.es = be32_to_cpup(p++);
		if (bex.es != PNFS_BLOCK_READWRITE_DATA) {
			dprintk("%s: incorrect extent state %d\n",
				__func__, bex.es);
			goto fail;
		}

		iomaps[i].offset = bex.foff;
		iomaps[i].length = bex.len;
	}

	*iomapp = iomaps;
	return nr_iomaps;
fail:
	kfree(iomaps);
	return -EINVAL;
}
