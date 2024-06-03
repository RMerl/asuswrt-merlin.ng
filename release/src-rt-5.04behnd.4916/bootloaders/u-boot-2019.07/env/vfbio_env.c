/* SPDX-License-Identifier: GPL-2.0+
 *
 *  Copyright 2019 Broadcom Ltd.
 */

#include <environment.h>
#include <memalign.h>
#include "vfbio.h"
#include "bca_sdk.h"

DECLARE_GLOBAL_DATA_PTR;

static int vfbio_save(void)
{
	int ret, i, id;
	uint blk_size = 0, lun_size = 0;
	char name[16];
	ALLOC_CACHE_ALIGN_BUFFER(char, env_new, CONFIG_ENV_SIZE + SZ_4K);

	*(uint32_t *)env_new = BOOT_MAGIC_MAGIC;
	*(uint32_t *)(env_new+4) = CONFIG_ENV_SIZE;
	ret = env_export((env_t *)(env_new+8));
	if(ret)
		return ret;

	for(i = 1; i <= 2; i++, id++)
	{
		sprintf(name, "ubootenv%d", i);
		ret = vfbio_lun_get_id(name, &id);
		if(ret < 0)
		{
			printf("%s lun was not found on flash\n", name);
			return -1;
		}
		
		vfbio_lun_get_size(id, &lun_size);
		vfbio_lun_get_blk_size(id, &blk_size);
		if((CONFIG_ENV_SIZE + blk_size) > lun_size)
		{
			printf("Not enough space to burn %s image - available %u, needed %u\n", name, lun_size, CONFIG_ENV_SIZE);
			return -1;
		}
		
		if(vfbio_lun_write(id, 0, CONFIG_ENV_SIZE/blk_size + 1, env_new))
			printf("\n** Unable to write env to LUN %s (%d) **\n", name, id);
	}

	puts("done\n");
	return 0;
}

static int vfbio_load(void)
{
	int i, id;
	uint32_t blk_s;
	char name[16];
	ALLOC_CACHE_ALIGN_BUFFER(char, buf, CONFIG_ENV_SIZE + SZ_4K);

	for(i = 1; i <= 2; i++, id++)
	{
		sprintf(name, "ubootenv%d", i);
		if(vfbio_lun_get_id(name, &id) || vfbio_lun_get_blk_size(id, &blk_s))
			 continue;
		/*
		* In case we have restarted u-boot there is a chance that buffer
		* contains old environment (from the previous boot).
		* If VFBIO LUN is zero size, read doesn't modify the
		* buffer.
		* We need to clear buffer manually here, so the invalid CRC will
		* cause setting default environment as expected.
		*/
		memset(buf+8, 0x0, CONFIG_ENV_SIZE);

		if(vfbio_lun_read(id, 0, CONFIG_ENV_SIZE/blk_s + 1, buf))
		{
			printf("Failed to read LUN %s(%d)\n", name, id);
			continue;
		}
		if(!env_import(buf+8, 1))
			return 0;
	}

	set_default_env("failed to load env from VFBIO device", 0);
	return -1;
}

U_BOOT_ENV_LOCATION(ubi) = {
	.location	= ENVL_VFBIO,
	ENV_NAME("VFBIO")
	.load		= vfbio_load,
	.save		= env_save_ptr(vfbio_save),
};
