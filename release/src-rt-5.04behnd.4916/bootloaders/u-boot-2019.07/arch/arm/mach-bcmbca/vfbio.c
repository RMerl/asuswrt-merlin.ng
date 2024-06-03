/* SPDX-License-Identifier: GPL-2.0+
 *
 *  Copyright 2019 Broadcom Ltd.
 */

#include <stdio.h>
#include <blk.h>
#include <dm.h>
#include <linux/log2.h>
#include <dm/device.h>
#include <dm/device-internal.h>
#include <common.h>
#include <command.h>
#include "vfbio.h"

static unsigned long vfbio_blk_read(struct udevice *udev, lbaint_t start, lbaint_t blkcnt, void *buffer)
{
	struct blk_desc *desc = dev_get_uclass_platdata(udev);
	uint32_t cnt = blkcnt;
	int ret;
	
	ret = vfbio_rpc_read(desc->devnum, (ulong)buffer, start, desc->blksz, &cnt);

	return !ret ? cnt : ret;
}

static unsigned long vfbio_blk_write(struct udevice *udev, lbaint_t start, lbaint_t blkcnt, const void *buffer)
{
	struct blk_desc *desc = dev_get_uclass_platdata(udev);
	uint32_t cnt = blkcnt;
	int ret;
	
	ret = vfbio_rpc_write(desc->devnum, (ulong)buffer, start, desc->blksz, &cnt);

	return !ret ? cnt : ret;
}

static int vfbio_blk_probe(struct udevice *udev)
{
	struct blk_desc *desc = dev_get_uclass_platdata(udev);

	desc->log2blksz = ilog2(desc->blksz);

	return 0;
}

static const struct blk_ops vfbio_blk_ops =
{
	.read	= vfbio_blk_read,
	.write	= vfbio_blk_write,
};

U_BOOT_DRIVER(vfbio_blk) =
{
	.name	= "vfbio-blk",
	.id	= UCLASS_BLK,
	.probe	= vfbio_blk_probe,
	.ops	= &vfbio_blk_ops,
};

static int get_desc(enum if_type if_type, int devnum, struct blk_desc **descp)
{
	bool found_more = false;
	struct udevice *dev;
	struct uclass *uc;
	int ret;

	*descp = NULL;
	ret = uclass_get(UCLASS_BLK, &uc);
	if(ret)
		return ret;
	uclass_foreach_dev(dev, uc)
	{
		struct blk_desc *desc = dev_get_uclass_platdata(dev);

		debug("%s: if_type=%d, devnum=%d: %s, %d, %d\n", __func__,
		      if_type, devnum, dev->name, desc->if_type, desc->devnum);
		if(desc->if_type == if_type)
		{
			if(desc->devnum == devnum)
			{
				ret = device_probe(dev);
				if(ret)
					return ret;

				*descp = desc;
				return 0;
			} else if(desc->devnum > devnum)
			{
				found_more = true;
			}
		}
	}

	return found_more ? -ENOENT : -ENODEV;
}

static int create_vfbio_blk(int id, char *name, unsigned int size_blk, unsigned int n_blk)
{
	struct udevice *blk_dev;
	char *str;
	int ret;
	
	str = strdup(name);
	if(!str)
		return -ENOMEM;
	
	ret = blk_create_device(gd->dm_root, "vfbio-blk", str, IF_TYPE_VFBIO, id, size_blk, n_blk, &blk_dev);
	if(!ret)
		ret = device_probe(blk_dev);
	
	return ret;
}

static int delete_vfbio_blk(int id)
{
	struct blk_desc *desc;
	int ret;
	
	ret = get_desc(IF_TYPE_VFBIO, id, &desc);
	if(!ret)
	{
		free((void *)desc->bdev->name);
		ret = device_remove(desc->bdev, DM_REMOVE_NORMAL);
		if(!ret)
			ret = device_unbind(desc->bdev);
	}
	
	return ret;
}

static int vfbio_probe(struct udevice *dev)
{
	int id, ret = -1;
	struct vfbio_lun_info info;

	for(id = 0; id <= VFBIO_LUN_MAX ; id++)
	{
		ret = vfbio_rpc_lun_info(id, &info);
		if (!ret && info.n_blks)
		{
			ret = create_vfbio_blk(id, info.name, info.blk_sz, info.n_blks);
			if(ret)
				break;
		}
	}
	
	return ret;
}

static const struct udevice_id vfbio_ids[] =
{
        { .compatible = "brcm,vflash" },
        { }
};

U_BOOT_DRIVER(vfbio_drv) =
{
        .name = "bcm_vfbio",
        .id = UCLASS_VFBIO,
        .of_match = vfbio_ids,
        .probe = vfbio_probe,
};

UCLASS_DRIVER(vfbio) =
{
	.name	= "vfbio",
	.id	= UCLASS_VFBIO,
	.flags	= DM_UC_FLAG_SEQ_ALIAS,
};

void vfbio_init(void)
{
	struct udevice *dev;
	uclass_get_device_by_driver(UCLASS_VFBIO, DM_GET_DRIVER(vfbio_drv), &dev);
}

int vfbio_finish_first_boot(void)
{
	return vfbio_rpc_finish_first_boot();
}

int vfbio_lun_create(char *name, unsigned int size, int *id)
{	
	struct vfbio_lun_info info;
	int ret;

	*id = vfbio_rpc_lun_create(*id, name, size);
	if(*id < 0)
		return -1;
	
	ret = vfbio_rpc_lun_info(*id, &info);
	if(ret || !info.blk_sz || !info.n_blks)
		return -1;

	ret = create_vfbio_blk(*id, name, info.blk_sz, info.n_blks);
	if(ret)
		vfbio_rpc_lun_delete(*id);
	
	return ret;
}

int vfbio_lun_delete(int id)
{
	int ret = vfbio_rpc_lun_delete(id);
	
	if(!ret)
		 delete_vfbio_blk(id);
	
	return ret;
}

const char *vfbio_lun_get_name(int id)
{
	struct blk_desc *desc;
	
	if(get_desc(IF_TYPE_VFBIO, id, &desc) == 0)
		return desc->bdev->name;
	
	return NULL;
}

int vfbio_lun_get_blk_size(int id, unsigned int *size)
{
	struct blk_desc *desc;
	
	if(get_desc(IF_TYPE_VFBIO, id, &desc) == 0)
	{
		*size = desc->blksz;
		return 0;
	}
	
	*size = 0;
	return -1;
}

int vfbio_lun_get_blk_num(int id, unsigned int *num)
{
	struct blk_desc *desc;
	
	if(get_desc(IF_TYPE_VFBIO, id, &desc) == 0)
	{
		*num = desc->lba;
		return 0;
	}
	
	*num = 0;
	return -1;
}

int vfbio_lun_get_size(int id, unsigned int *size)
{
	unsigned int blk_s, blk_n;
	
	*size = 0;
	if(!vfbio_lun_get_blk_size(id, &blk_s) && !vfbio_lun_get_blk_num(id, &blk_n))
	{
		*size = blk_s * blk_n;
		return 0;
	}
	
	return -1;
}

int vfbio_lun_read(int id, unsigned int blk, unsigned int cnt, void *buffer)
{
	struct blk_desc *desc;
	
	if(get_desc(IF_TYPE_VFBIO, id, &desc))
		return -1;
	
	if(cnt == UINT_MAX)
	{
		if(blk <= desc->lba)
			cnt = desc->lba - blk;
	}
	
	return vfbio_rpc_read(desc->devnum, (ulong)buffer, blk, desc->blksz, &cnt);
}

int vfbio_lun_write(int id, unsigned int blk, unsigned int cnt, void *buffer)
{
	struct blk_desc *desc;
	
	if(get_desc(IF_TYPE_VFBIO, id, &desc))
		return -1;
	
	return vfbio_rpc_write(desc->devnum, (ulong)buffer, blk, desc->blksz, &cnt);
}

int vfbio_lun_resize(int id, unsigned int size)
{
	char name[VFBIO_LUN_INFO_NAME_MAX];
	unsigned int size_blk;
	int ret = -1;
	
	vfbio_lun_get_blk_size(id, &size_blk);
	if((size_blk > 0) && (size > 0))
	{
		strcpy(name, vfbio_lun_get_name(id));
		
		ret = vfbio_rpc_lun_resize(id, size);
		if(!ret)
		{
			if(delete_vfbio_blk(id) || create_vfbio_blk(id, name, size_blk, (size + size_blk -1)/size_blk))
			{
				printf("vfbio blk device settings could be inconsistent,try reset to recover\n");
				ret = -2;
			}
		}
	}
	
	return ret;
}

int vfbio_lun_rename(uint8_t num_luns, struct vfbio_lun_id_name id_name[])
{
	unsigned int blk_s, blk_n;
	int ret = 0;
	int i;
	
	ret = vfbio_rpc_lun_rename(num_luns, id_name);
	if(!ret)
	{
		for(i=0; i<num_luns; i++)
		{
			int id = id_name[i].id;
			vfbio_lun_get_blk_size(id, &blk_s);
			vfbio_lun_get_blk_num(id, &blk_n);
			if(delete_vfbio_blk(id) || create_vfbio_blk(id, id_name[i].name, blk_s, blk_n))
			{
				if(ret == 0) // print only once
					printf("vfbio blk device settings could be inconsistent,try reset to recover\n"); 
				ret = -2;
			}
		}
	}
	
	return ret;
}

int vfbio_lun_get_id(char *name, int *id)
{
	struct blk_desc *desc;
	int ret;

	*id = 0;
	while(1)
	{
		ret = get_desc(IF_TYPE_VFBIO, *id, &desc);
		if(ret == -ENODEV)
		{
			break;
		}
		else if(ret == 0)
		{
			if(strcmp(name, desc->bdev->name) == 0)
			{
				ret = 0;
				break;
			}
		}
		(*id)++;
	}
	
	return ret;
}

int vfbio_lun_get_next(int prev_id, int *id)
{
	struct blk_desc *desc;
	int ret;

	while(1)
	{
		ret = get_desc(IF_TYPE_VFBIO, ++prev_id, &desc);
		if(ret == -ENODEV)
		{
			break;
		}
		else if(ret == 0)
		{
			*id = prev_id;
			break;
		}
	}
	
	return ret;
}

int vfbio_device_get_info(uint64_t *total_size, uint64_t *free_size)
{
	return vfbio_rpc_device_get_info(total_size, free_size);
}
