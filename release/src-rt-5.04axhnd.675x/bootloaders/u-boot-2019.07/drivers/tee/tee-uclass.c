// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2018 Linaro Limited
 */

#include <common.h>
#include <dm.h>
#include <dm/device-internal.h>
#include <dm/uclass-internal.h>
#include <tee.h>

/**
 * struct tee_uclass_priv - information of a TEE, stored by the uclass
 *
 * @list_shm:	list of structe tee_shm representing memory blocks shared
 *		with the TEE.
 */
struct tee_uclass_priv {
	struct list_head list_shm;
};

static const struct tee_driver_ops *tee_get_ops(struct udevice *dev)
{
	return device_get_ops(dev);
}

void tee_get_version(struct udevice *dev, struct tee_version_data *vers)
{
	tee_get_ops(dev)->get_version(dev, vers);
}

int tee_open_session(struct udevice *dev, struct tee_open_session_arg *arg,
		     uint num_param, struct tee_param *param)
{
	return tee_get_ops(dev)->open_session(dev, arg, num_param, param);
}

int tee_close_session(struct udevice *dev, u32 session)
{
	return tee_get_ops(dev)->close_session(dev, session);
}

int tee_invoke_func(struct udevice *dev, struct tee_invoke_arg *arg,
		    uint num_param, struct tee_param *param)
{
	return tee_get_ops(dev)->invoke_func(dev, arg, num_param, param);
}

int __tee_shm_add(struct udevice *dev, ulong align, void *addr, ulong size,
		  u32 flags, struct tee_shm **shmp)
{
	struct tee_shm *shm;
	void *p = addr;
	int rc;

	if (flags & TEE_SHM_ALLOC) {
		if (align)
			p = memalign(align, size);
		else
			p = malloc(size);
	}
	if (!p)
		return -ENOMEM;

	shm = calloc(1, sizeof(*shm));
	if (!shm) {
		rc = -ENOMEM;
		goto err;
	}

	shm->dev = dev;
	shm->addr = p;
	shm->size = size;
	shm->flags = flags;

	if (flags & TEE_SHM_SEC_REGISTER) {
		rc = tee_get_ops(dev)->shm_register(dev, shm);
		if (rc)
			goto err;
	}

	if (flags & TEE_SHM_REGISTER) {
		struct tee_uclass_priv *priv = dev_get_uclass_priv(dev);

		list_add(&shm->link, &priv->list_shm);
	}

	*shmp = shm;

	return 0;
err:
	free(shm);
	if (flags & TEE_SHM_ALLOC)
		free(p);

	return rc;
}

int tee_shm_alloc(struct udevice *dev, ulong size, u32 flags,
		  struct tee_shm **shmp)
{
	u32 f = flags;

	f |= TEE_SHM_SEC_REGISTER | TEE_SHM_REGISTER | TEE_SHM_ALLOC;

	return __tee_shm_add(dev, 0, NULL, size, f, shmp);
}

int tee_shm_register(struct udevice *dev, void *addr, ulong size, u32 flags,
		     struct tee_shm **shmp)
{
	u32 f = flags & ~TEE_SHM_ALLOC;

	f |= TEE_SHM_SEC_REGISTER | TEE_SHM_REGISTER;

	return __tee_shm_add(dev, 0, addr, size, f, shmp);
}

void tee_shm_free(struct tee_shm *shm)
{
	if (!shm)
		return;

	if (shm->flags & TEE_SHM_SEC_REGISTER)
		tee_get_ops(shm->dev)->shm_unregister(shm->dev, shm);

	if (shm->flags & TEE_SHM_REGISTER)
		list_del(&shm->link);

	if (shm->flags & TEE_SHM_ALLOC)
		free(shm->addr);

	free(shm);
}

bool tee_shm_is_registered(struct tee_shm *shm, struct udevice *dev)
{
	struct tee_uclass_priv *priv = dev_get_uclass_priv(dev);
	struct tee_shm *s;

	list_for_each_entry(s, &priv->list_shm, link)
		if (s == shm)
			return true;

	return false;
}

struct udevice *tee_find_device(struct udevice *start,
				int (*match)(struct tee_version_data *vers,
					     const void *data),
				const void *data,
				struct tee_version_data *vers)
{
	struct udevice *dev = start;
	struct tee_version_data lv;
	struct tee_version_data *v = vers ? vers : &lv;

	if (!dev)
		uclass_find_first_device(UCLASS_TEE, &dev);
	else
		uclass_find_next_device(&dev);

	for (; dev; uclass_find_next_device(&dev)) {
		if (device_probe(dev))
			continue;
		tee_get_ops(dev)->get_version(dev, v);
		if (!match || match(v, data))
			return dev;
	}

	return NULL;
}

static int tee_pre_probe(struct udevice *dev)
{
	struct tee_uclass_priv *priv = dev_get_uclass_priv(dev);

	INIT_LIST_HEAD(&priv->list_shm);

	return 0;
}

static int tee_pre_remove(struct udevice *dev)
{
	struct tee_uclass_priv *priv = dev_get_uclass_priv(dev);
	struct tee_shm *shm;

	/*
	 * Any remaining shared memory must be unregistered now as U-Boot
	 * is about to hand over to the next stage and that memory will be
	 * reused.
	 */
	while (!list_empty(&priv->list_shm)) {
		shm = list_first_entry(&priv->list_shm, struct tee_shm, link);
		debug("%s: freeing leftover shm %p (size %lu, flags %#x)\n",
		      __func__, (void *)shm, shm->size, shm->flags);
		tee_shm_free(shm);
	}

	return 0;
}

UCLASS_DRIVER(tee) = {
	.id = UCLASS_TEE,
	.name = "tee",
	.per_device_auto_alloc_size = sizeof(struct tee_uclass_priv),
	.pre_probe = tee_pre_probe,
	.pre_remove = tee_pre_remove,
};

void tee_optee_ta_uuid_from_octets(struct tee_optee_ta_uuid *d,
				   const u8 s[TEE_UUID_LEN])
{
	d->time_low = ((u32)s[0] << 24) | ((u32)s[1] << 16) |
		      ((u32)s[2] << 8) | s[3],
	d->time_mid = ((u32)s[4] << 8) | s[5];
	d->time_hi_and_version = ((u32)s[6] << 8) | s[7];
	memcpy(d->clock_seq_and_node, s + 8, sizeof(d->clock_seq_and_node));
}

void tee_optee_ta_uuid_to_octets(u8 d[TEE_UUID_LEN],
				 const struct tee_optee_ta_uuid *s)
{
	d[0] = s->time_low >> 24;
	d[1] = s->time_low >> 16;
	d[2] = s->time_low >> 8;
	d[3] = s->time_low;
	d[4] = s->time_mid >> 8;
	d[5] = s->time_mid;
	d[6] = s->time_hi_and_version >> 8;
	d[7] = s->time_hi_and_version;
	memcpy(d + 8, s->clock_seq_and_node, sizeof(s->clock_seq_and_node));
}
