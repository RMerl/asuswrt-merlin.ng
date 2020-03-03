/*
 * Copyright 2013 Red Hat Inc.
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
#include "priv.h"
#include "pad.h"

#include <core/device.h>
#include <core/notify.h>
#include <core/option.h>
#include <subdev/bios.h>
#include <subdev/bios/dcb.h>

/******************************************************************************
 * interface to linux i2c bit-banging algorithm
 *****************************************************************************/

#ifdef CONFIG_NOUVEAU_I2C_INTERNAL_DEFAULT
#define CSTMSEL true
#else
#define CSTMSEL false
#endif

static int
nvkm_i2c_pre_xfer(struct i2c_adapter *adap)
{
	struct i2c_algo_bit_data *bit = adap->algo_data;
	struct nvkm_i2c_port *port = bit->data;
	return nvkm_i2c(port)->acquire(port, bit->timeout);
}

static void
nvkm_i2c_post_xfer(struct i2c_adapter *adap)
{
	struct i2c_algo_bit_data *bit = adap->algo_data;
	struct nvkm_i2c_port *port = bit->data;
	return nvkm_i2c(port)->release(port);
}

static void
nvkm_i2c_setscl(void *data, int state)
{
	struct nvkm_i2c_port *port = data;
	port->func->drive_scl(port, state);
}

static void
nvkm_i2c_setsda(void *data, int state)
{
	struct nvkm_i2c_port *port = data;
	port->func->drive_sda(port, state);
}

static int
nvkm_i2c_getscl(void *data)
{
	struct nvkm_i2c_port *port = data;
	return port->func->sense_scl(port);
}

static int
nvkm_i2c_getsda(void *data)
{
	struct nvkm_i2c_port *port = data;
	return port->func->sense_sda(port);
}

/******************************************************************************
 * base i2c "port" class implementation
 *****************************************************************************/

int
_nvkm_i2c_port_fini(struct nvkm_object *object, bool suspend)
{
	struct nvkm_i2c_port *port = (void *)object;
	struct nvkm_i2c_pad *pad = nvkm_i2c_pad(port);
	nv_ofuncs(pad)->fini(nv_object(pad), suspend);
	return nvkm_object_fini(&port->base, suspend);
}

void
_nvkm_i2c_port_dtor(struct nvkm_object *object)
{
	struct nvkm_i2c_port *port = (void *)object;
	i2c_del_adapter(&port->adapter);
	nvkm_object_destroy(&port->base);
}

int
nvkm_i2c_port_create_(struct nvkm_object *parent, struct nvkm_object *engine,
		      struct nvkm_oclass *oclass, u8 index,
		      const struct i2c_algorithm *algo,
		      const struct nvkm_i2c_func *func,
		      int size, void **pobject)
{
	struct nvkm_device *device = nv_device(parent);
	struct nvkm_i2c *i2c = nvkm_i2c(parent);
	struct nvkm_i2c_port *port;
	int ret;

	ret = nvkm_object_create_(parent, engine, oclass, 0, size, pobject);
	port = *pobject;
	if (ret)
		return ret;

	snprintf(port->adapter.name, sizeof(port->adapter.name),
		 "nvkm-%s-%d", device->name, index);
	port->adapter.owner = THIS_MODULE;
	port->adapter.dev.parent = nv_device_base(device);
	port->index = index;
	port->aux = -1;
	port->func = func;
	mutex_init(&port->mutex);

	if ( algo == &nvkm_i2c_bit_algo &&
	    !nvkm_boolopt(device->cfgopt, "NvI2C", CSTMSEL)) {
		struct i2c_algo_bit_data *bit;

		bit = kzalloc(sizeof(*bit), GFP_KERNEL);
		if (!bit)
			return -ENOMEM;

		bit->udelay = 10;
		bit->timeout = usecs_to_jiffies(2200);
		bit->data = port;
		bit->pre_xfer = nvkm_i2c_pre_xfer;
		bit->post_xfer = nvkm_i2c_post_xfer;
		bit->setsda = nvkm_i2c_setsda;
		bit->setscl = nvkm_i2c_setscl;
		bit->getsda = nvkm_i2c_getsda;
		bit->getscl = nvkm_i2c_getscl;

		port->adapter.algo_data = bit;
		ret = i2c_bit_add_bus(&port->adapter);
	} else {
		port->adapter.algo_data = port;
		port->adapter.algo = algo;
		ret = i2c_add_adapter(&port->adapter);
	}

	if (ret == 0)
		list_add_tail(&port->head, &i2c->ports);
	return ret;
}

/******************************************************************************
 * base i2c subdev class implementation
 *****************************************************************************/

static struct nvkm_i2c_port *
nvkm_i2c_find(struct nvkm_i2c *i2c, u8 index)
{
	struct nvkm_bios *bios = nvkm_bios(i2c);
	struct nvkm_i2c_port *port;

	if (index == NV_I2C_DEFAULT(0) ||
	    index == NV_I2C_DEFAULT(1)) {
		u8  ver, hdr, cnt, len;
		u16 i2c = dcb_i2c_table(bios, &ver, &hdr, &cnt, &len);
		if (i2c && ver >= 0x30) {
			u8 auxidx = nv_ro08(bios, i2c + 4);
			if (index == NV_I2C_DEFAULT(0))
				index = (auxidx & 0x0f) >> 0;
			else
				index = (auxidx & 0xf0) >> 4;
		} else {
			index = 2;
		}
	}

	list_for_each_entry(port, &i2c->ports, head) {
		if (port->index == index)
			return port;
	}

	return NULL;
}

static struct nvkm_i2c_port *
nvkm_i2c_find_type(struct nvkm_i2c *i2c, u16 type)
{
	struct nvkm_i2c_port *port;

	list_for_each_entry(port, &i2c->ports, head) {
		if (nv_hclass(port) == type)
			return port;
	}

	return NULL;
}

static void
nvkm_i2c_release_pad(struct nvkm_i2c_port *port)
{
	struct nvkm_i2c_pad *pad = nvkm_i2c_pad(port);
	struct nvkm_i2c *i2c = nvkm_i2c(port);

	if (atomic_dec_and_test(&nv_object(pad)->usecount)) {
		nv_ofuncs(pad)->fini(nv_object(pad), false);
		wake_up_all(&i2c->wait);
	}
}

static int
nvkm_i2c_try_acquire_pad(struct nvkm_i2c_port *port)
{
	struct nvkm_i2c_pad *pad = nvkm_i2c_pad(port);

	if (atomic_add_return(1, &nv_object(pad)->usecount) != 1) {
		struct nvkm_object *owner = (void *)pad->port;
		do {
			if (owner == (void *)port)
				return 0;
			owner = owner->parent;
		} while(owner);
		nvkm_i2c_release_pad(port);
		return -EBUSY;
	}

	pad->next = port;
	nv_ofuncs(pad)->init(nv_object(pad));
	return 0;
}

static int
nvkm_i2c_acquire_pad(struct nvkm_i2c_port *port, unsigned long timeout)
{
	struct nvkm_i2c *i2c = nvkm_i2c(port);

	if (timeout) {
		if (wait_event_timeout(i2c->wait,
				       nvkm_i2c_try_acquire_pad(port) == 0,
				       timeout) == 0)
			return -EBUSY;
	} else {
		wait_event(i2c->wait, nvkm_i2c_try_acquire_pad(port) == 0);
	}

	return 0;
}

static void
nvkm_i2c_release(struct nvkm_i2c_port *port)
__releases(pad->mutex)
{
	nvkm_i2c(port)->release_pad(port);
	mutex_unlock(&port->mutex);
}

static int
nvkm_i2c_acquire(struct nvkm_i2c_port *port, unsigned long timeout)
__acquires(pad->mutex)
{
	int ret;
	mutex_lock(&port->mutex);
	if ((ret = nvkm_i2c(port)->acquire_pad(port, timeout)))
		mutex_unlock(&port->mutex);
	return ret;
}

static int
nvkm_i2c_identify(struct nvkm_i2c *i2c, int index, const char *what,
		  struct nvkm_i2c_board_info *info,
		  bool (*match)(struct nvkm_i2c_port *,
				struct i2c_board_info *, void *), void *data)
{
	struct nvkm_i2c_port *port = nvkm_i2c_find(i2c, index);
	int i;

	if (!port) {
		nv_debug(i2c, "no bus when probing %s on %d\n", what, index);
		return -ENODEV;
	}

	nv_debug(i2c, "probing %ss on bus: %d\n", what, port->index);
	for (i = 0; info[i].dev.addr; i++) {
		u8 orig_udelay = 0;

		if ((port->adapter.algo == &i2c_bit_algo) &&
		    (info[i].udelay != 0)) {
			struct i2c_algo_bit_data *algo = port->adapter.algo_data;
			nv_debug(i2c, "using custom udelay %d instead of %d\n",
			         info[i].udelay, algo->udelay);
			orig_udelay = algo->udelay;
			algo->udelay = info[i].udelay;
		}

		if (nv_probe_i2c(port, info[i].dev.addr) &&
		    (!match || match(port, &info[i].dev, data))) {
			nv_info(i2c, "detected %s: %s\n", what,
				info[i].dev.type);
			return i;
		}

		if (orig_udelay) {
			struct i2c_algo_bit_data *algo = port->adapter.algo_data;
			algo->udelay = orig_udelay;
		}
	}

	nv_debug(i2c, "no devices found.\n");
	return -ENODEV;
}

static void
nvkm_i2c_intr_fini(struct nvkm_event *event, int type, int index)
{
	struct nvkm_i2c *i2c = container_of(event, typeof(*i2c), event);
	struct nvkm_i2c_port *port = i2c->find(i2c, index);
	const struct nvkm_i2c_impl *impl = (void *)nv_object(i2c)->oclass;
	if (port && port->aux >= 0)
		impl->aux_mask(i2c, type, 1 << port->aux, 0);
}

static void
nvkm_i2c_intr_init(struct nvkm_event *event, int type, int index)
{
	struct nvkm_i2c *i2c = container_of(event, typeof(*i2c), event);
	struct nvkm_i2c_port *port = i2c->find(i2c, index);
	const struct nvkm_i2c_impl *impl = (void *)nv_object(i2c)->oclass;
	if (port && port->aux >= 0)
		impl->aux_mask(i2c, type, 1 << port->aux, 1 << port->aux);
}

static int
nvkm_i2c_intr_ctor(struct nvkm_object *object, void *data, u32 size,
		      struct nvkm_notify *notify)
{
	struct nvkm_i2c_ntfy_req *req = data;
	if (!WARN_ON(size != sizeof(*req))) {
		notify->size  = sizeof(struct nvkm_i2c_ntfy_rep);
		notify->types = req->mask;
		notify->index = req->port;
		return 0;
	}
	return -EINVAL;
}

static void
nvkm_i2c_intr(struct nvkm_subdev *subdev)
{
	struct nvkm_i2c_impl *impl = (void *)nv_oclass(subdev);
	struct nvkm_i2c *i2c = nvkm_i2c(subdev);
	struct nvkm_i2c_port *port;
	u32 hi, lo, rq, tx, e;

	if (impl->aux_stat) {
		impl->aux_stat(i2c, &hi, &lo, &rq, &tx);
		if (hi || lo || rq || tx) {
			list_for_each_entry(port, &i2c->ports, head) {
				if (e = 0, port->aux < 0)
					continue;

				if (hi & (1 << port->aux)) e |= NVKM_I2C_PLUG;
				if (lo & (1 << port->aux)) e |= NVKM_I2C_UNPLUG;
				if (rq & (1 << port->aux)) e |= NVKM_I2C_IRQ;
				if (tx & (1 << port->aux)) e |= NVKM_I2C_DONE;
				if (e) {
					struct nvkm_i2c_ntfy_rep rep = {
						.mask = e,
					};
					nvkm_event_send(&i2c->event, rep.mask,
							port->index, &rep,
							sizeof(rep));
				}
			}
		}
	}
}

static const struct nvkm_event_func
nvkm_i2c_intr_func = {
	.ctor = nvkm_i2c_intr_ctor,
	.init = nvkm_i2c_intr_init,
	.fini = nvkm_i2c_intr_fini,
};

int
_nvkm_i2c_fini(struct nvkm_object *object, bool suspend)
{
	struct nvkm_i2c_impl *impl = (void *)nv_oclass(object);
	struct nvkm_i2c *i2c = (void *)object;
	struct nvkm_i2c_port *port;
	u32 mask;
	int ret;

	list_for_each_entry(port, &i2c->ports, head) {
		ret = nv_ofuncs(port)->fini(nv_object(port), suspend);
		if (ret && suspend)
			goto fail;
	}

	if ((mask = (1 << impl->aux) - 1), impl->aux_stat) {
		impl->aux_mask(i2c, NVKM_I2C_ANY, mask, 0);
		impl->aux_stat(i2c, &mask, &mask, &mask, &mask);
	}

	return nvkm_subdev_fini(&i2c->base, suspend);
fail:
	list_for_each_entry_continue_reverse(port, &i2c->ports, head) {
		nv_ofuncs(port)->init(nv_object(port));
	}

	return ret;
}

int
_nvkm_i2c_init(struct nvkm_object *object)
{
	struct nvkm_i2c *i2c = (void *)object;
	struct nvkm_i2c_port *port;
	int ret;

	ret = nvkm_subdev_init(&i2c->base);
	if (ret == 0) {
		list_for_each_entry(port, &i2c->ports, head) {
			ret = nv_ofuncs(port)->init(nv_object(port));
			if (ret)
				goto fail;
		}
	}

	return ret;
fail:
	list_for_each_entry_continue_reverse(port, &i2c->ports, head) {
		nv_ofuncs(port)->fini(nv_object(port), false);
	}

	return ret;
}

void
_nvkm_i2c_dtor(struct nvkm_object *object)
{
	struct nvkm_i2c *i2c = (void *)object;
	struct nvkm_i2c_port *port, *temp;

	nvkm_event_fini(&i2c->event);

	list_for_each_entry_safe(port, temp, &i2c->ports, head) {
		nvkm_object_ref(NULL, (struct nvkm_object **)&port);
	}

	nvkm_subdev_destroy(&i2c->base);
}

static struct nvkm_oclass *
nvkm_i2c_extdev_sclass[] = {
	nvkm_anx9805_sclass,
};

static void
nvkm_i2c_create_port(struct nvkm_i2c *i2c, int index, u8 type,
		     struct dcb_i2c_entry *info)
{
	const struct nvkm_i2c_impl *impl = (void *)nv_oclass(i2c);
	struct nvkm_oclass *oclass;
	struct nvkm_object *parent;
	struct nvkm_object *object;
	int ret, pad;

	if (info->share != DCB_I2C_UNUSED) {
		pad    = info->share;
		oclass = impl->pad_s;
	} else {
		if (type != DCB_I2C_NVIO_AUX)
			pad = 0x100 + info->drive;
		else
			pad = 0x100 + info->auxch;
		oclass = impl->pad_x;
	}

	ret = nvkm_object_ctor(nv_object(i2c), NULL, oclass,
			       NULL, pad, &parent);
	if (ret < 0)
		return;

	oclass = impl->sclass;
	do {
		ret = -EINVAL;
		if (oclass->handle == type) {
			ret = nvkm_object_ctor(parent, NULL, oclass,
					       info, index, &object);
		}
	} while (ret && (++oclass)->handle);

	nvkm_object_ref(NULL, &parent);
}

int
nvkm_i2c_create_(struct nvkm_object *parent, struct nvkm_object *engine,
		 struct nvkm_oclass *oclass, int length, void **pobject)
{
	struct nvkm_bios *bios = nvkm_bios(parent);
	struct nvkm_i2c *i2c;
	struct nvkm_object *object;
	struct dcb_i2c_entry info;
	int ret, i, j, index = -1;
	struct dcb_output outp;
	u8  ver, hdr;
	u32 data;

	ret = nvkm_subdev_create(parent, engine, oclass, 0, "I2C", "i2c", &i2c);
	*pobject = nv_object(i2c);
	if (ret)
		return ret;

	nv_subdev(i2c)->intr = nvkm_i2c_intr;
	i2c->find = nvkm_i2c_find;
	i2c->find_type = nvkm_i2c_find_type;
	i2c->acquire_pad = nvkm_i2c_acquire_pad;
	i2c->release_pad = nvkm_i2c_release_pad;
	i2c->acquire = nvkm_i2c_acquire;
	i2c->release = nvkm_i2c_release;
	i2c->identify = nvkm_i2c_identify;
	init_waitqueue_head(&i2c->wait);
	INIT_LIST_HEAD(&i2c->ports);

	while (!dcb_i2c_parse(bios, ++index, &info)) {
		switch (info.type) {
		case DCB_I2C_NV04_BIT:
		case DCB_I2C_NV4E_BIT:
		case DCB_I2C_NVIO_BIT:
			nvkm_i2c_create_port(i2c, NV_I2C_PORT(index),
					     info.type, &info);
			break;
		case DCB_I2C_NVIO_AUX:
			nvkm_i2c_create_port(i2c, NV_I2C_AUX(index),
					     info.type, &info);
			break;
		case DCB_I2C_PMGR:
			if (info.drive != DCB_I2C_UNUSED) {
				nvkm_i2c_create_port(i2c, NV_I2C_PORT(index),
						     DCB_I2C_NVIO_BIT, &info);
			}
			if (info.auxch != DCB_I2C_UNUSED) {
				nvkm_i2c_create_port(i2c, NV_I2C_AUX(index),
						     DCB_I2C_NVIO_AUX, &info);
			}
			break;
		case DCB_I2C_UNUSED:
		default:
			continue;
		}
	}

	/* in addition to the busses specified in the i2c table, there
	 * may be ddc/aux channels hiding behind external tmds/dp/etc
	 * transmitters.
	 */
	index = NV_I2C_EXT(0);
	i = -1;
	while ((data = dcb_outp_parse(bios, ++i, &ver, &hdr, &outp))) {
		if (!outp.location || !outp.extdev)
			continue;

		switch (outp.type) {
		case DCB_OUTPUT_TMDS:
			info.type = NV_I2C_TYPE_EXTDDC(outp.extdev);
			break;
		case DCB_OUTPUT_DP:
			info.type = NV_I2C_TYPE_EXTAUX(outp.extdev);
			break;
		default:
			continue;
		}

		ret = -ENODEV;
		j = -1;
		while (ret && ++j < ARRAY_SIZE(nvkm_i2c_extdev_sclass)) {
			parent = nv_object(i2c->find(i2c, outp.i2c_index));
			oclass = nvkm_i2c_extdev_sclass[j];
			do {
				if (oclass->handle != info.type)
					continue;
				ret = nvkm_object_ctor(parent, NULL, oclass,
						       NULL, index++, &object);
			} while (ret && (++oclass)->handle);
		}
	}

	ret = nvkm_event_init(&nvkm_i2c_intr_func, 4, index, &i2c->event);
	if (ret)
		return ret;

	return 0;
}

int
_nvkm_i2c_ctor(struct nvkm_object *parent, struct nvkm_object *engine,
	       struct nvkm_oclass *oclass, void *data, u32 size,
	       struct nvkm_object **pobject)
{
	struct nvkm_i2c *i2c;
	int ret;

	ret = nvkm_i2c_create(parent, engine, oclass, &i2c);
	*pobject = nv_object(i2c);
	if (ret)
		return ret;

	return 0;
}
