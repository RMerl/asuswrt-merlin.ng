/*
 * Copyright 2012 Red Hat Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE COPYRIGHT HOLDER(S) OR AUTHOR(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * Authors: Ben Skeggs
 */
#include "nv50.h"

void
nv50_i2c_drive_scl(struct nvkm_i2c_port *base, int state)
{
	struct nv50_i2c_priv *priv = (void *)nvkm_i2c(base);
	struct nv50_i2c_port *port = (void *)base;
	if (state) port->state |= 0x01;
	else	   port->state &= 0xfe;
	nv_wr32(priv, port->addr, port->state);
}

void
nv50_i2c_drive_sda(struct nvkm_i2c_port *base, int state)
{
	struct nv50_i2c_priv *priv = (void *)nvkm_i2c(base);
	struct nv50_i2c_port *port = (void *)base;
	if (state) port->state |= 0x02;
	else	   port->state &= 0xfd;
	nv_wr32(priv, port->addr, port->state);
}

int
nv50_i2c_sense_scl(struct nvkm_i2c_port *base)
{
	struct nv50_i2c_priv *priv = (void *)nvkm_i2c(base);
	struct nv50_i2c_port *port = (void *)base;
	return !!(nv_rd32(priv, port->addr) & 0x00000001);
}

int
nv50_i2c_sense_sda(struct nvkm_i2c_port *base)
{
	struct nv50_i2c_priv *priv = (void *)nvkm_i2c(base);
	struct nv50_i2c_port *port = (void *)base;
	return !!(nv_rd32(priv, port->addr) & 0x00000002);
}

static const struct nvkm_i2c_func
nv50_i2c_func = {
	.drive_scl = nv50_i2c_drive_scl,
	.drive_sda = nv50_i2c_drive_sda,
	.sense_scl = nv50_i2c_sense_scl,
	.sense_sda = nv50_i2c_sense_sda,
};

const u32 nv50_i2c_addr[] = {
	0x00e138, 0x00e150, 0x00e168, 0x00e180,
	0x00e254, 0x00e274, 0x00e764, 0x00e780,
	0x00e79c, 0x00e7b8
};
const int nv50_i2c_addr_nr = ARRAY_SIZE(nv50_i2c_addr);

static int
nv50_i2c_port_ctor(struct nvkm_object *parent, struct nvkm_object *engine,
		   struct nvkm_oclass *oclass, void *data, u32 index,
		   struct nvkm_object **pobject)
{
	struct dcb_i2c_entry *info = data;
	struct nv50_i2c_port *port;
	int ret;

	ret = nvkm_i2c_port_create(parent, engine, oclass, index,
				   &nvkm_i2c_bit_algo, &nv50_i2c_func, &port);
	*pobject = nv_object(port);
	if (ret)
		return ret;

	if (info->drive >= nv50_i2c_addr_nr)
		return -EINVAL;

	port->state = 0x00000007;
	port->addr = nv50_i2c_addr[info->drive];
	return 0;
}

int
nv50_i2c_port_init(struct nvkm_object *object)
{
	struct nv50_i2c_priv *priv = (void *)nvkm_i2c(object);
	struct nv50_i2c_port *port = (void *)object;
	nv_wr32(priv, port->addr, port->state);
	return nvkm_i2c_port_init(&port->base);
}

static struct nvkm_oclass
nv50_i2c_sclass[] = {
	{ .handle = NV_I2C_TYPE_DCBI2C(DCB_I2C_NVIO_BIT),
	  .ofuncs = &(struct nvkm_ofuncs) {
		  .ctor = nv50_i2c_port_ctor,
		  .dtor = _nvkm_i2c_port_dtor,
		  .init = nv50_i2c_port_init,
		  .fini = _nvkm_i2c_port_fini,
	  },
	},
	{}
};

struct nvkm_oclass *
nv50_i2c_oclass = &(struct nvkm_i2c_impl) {
	.base.handle = NV_SUBDEV(I2C, 0x50),
	.base.ofuncs = &(struct nvkm_ofuncs) {
		.ctor = _nvkm_i2c_ctor,
		.dtor = _nvkm_i2c_dtor,
		.init = _nvkm_i2c_init,
		.fini = _nvkm_i2c_fini,
	},
	.sclass = nv50_i2c_sclass,
	.pad_x = &nv04_i2c_pad_oclass,
}.base;
