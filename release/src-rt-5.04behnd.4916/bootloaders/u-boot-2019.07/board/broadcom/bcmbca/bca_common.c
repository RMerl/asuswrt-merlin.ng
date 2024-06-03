/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2019 Broadcom Ltd.
 */

#include <common.h>
#include <linux/ctype.h>
#include <stdlib.h>
#include <string.h>
#include <mmc.h>
#include <mtd.h>
#include <spl.h>
#include "bca_common.h"
#include "boot_flash.h"
#include "bcm_strap_drv.h"

#define CLI_CB_NUM 8

struct cli_job_cb
{
    unsigned long last_time;
    unsigned long time_period;
    void (*job_cb)(void);
};

static unsigned long        registred_cb_count = 0;
static struct cli_job_cb    cli_job_cb_arr[CLI_CB_NUM];

void init_cli_cb_arr(void)
{
    memset(cli_job_cb_arr, 0, sizeof(struct cli_job_cb)*CLI_CB_NUM);
}

void register_cli_job_cb(unsigned long time_period, void (*job_cb)(void))
{
    int i;
    for(i=0; i<CLI_CB_NUM; i++) 
    {
        if (!cli_job_cb_arr[i].job_cb)
        {
            cli_job_cb_arr[i].job_cb = job_cb;
            cli_job_cb_arr[i].time_period = time_period;
            cli_job_cb_arr[i].last_time = get_timer(0);
            registred_cb_count++;
            break;
        }
    }
}

void unregister_cli_job_cb(void (*job_cb)(void))
{
    int i,j;
    for(i=0; i<registred_cb_count; i++) 
    {
        if (cli_job_cb_arr[i].job_cb == job_cb)
        {
            for(j=i; j<registred_cb_count; j++)
            {
                if(j+1 == registred_cb_count)
                {
                    memset(&cli_job_cb_arr[j], 0, sizeof(struct cli_job_cb));
                    break;
                }
                memcpy(&cli_job_cb_arr[j], &cli_job_cb_arr[j+1], sizeof(struct cli_job_cb));
            }
            registred_cb_count--;
            break;
        }
    }
}

void run_cli_jobs(void)
{
    int i;
    for(i=0; i<registred_cb_count; i++) 
    {
        if(cli_job_cb_arr[i].job_cb)
        {
            if(!cli_job_cb_arr[i].time_period)
            {
                cli_job_cb_arr[i].job_cb();
            }
            else
            {
                if(get_timer(cli_job_cb_arr[i].last_time) >= cli_job_cb_arr[i].time_period)
                {
                    cli_job_cb_arr[i].job_cb();
                    cli_job_cb_arr[i].last_time = get_timer(0);
                }
            }
        }
    }
}

int suffix2shift(char suffix)
{
	if (suffix == 'K')
	{
		return(10);
	} else if (suffix == 'M')
	{
		return(20);
	} else if (suffix == 'G')
	{
		return(30);
	}
	return(0);
}

/**
 * parse_env_nums - get named environment value of format FIELD=n1,n2,n3....
 * @buffer: pointer environment variable to parse
 * @maxargs: max number of arguments to parse
 * @args: pointer to array of unsigned longs for numeric arguments
 * @suffixes: pointer to array of chars for suffix characters
 *
 * returns:
 *     number of arguments total if successful
 *     0 if not found
 */
int parse_env_nums(const char *buffer, const int maxargs, unsigned long *args, char *suffixes)
{
	int ret = 0;
	char *b = NULL;
	char *p;
	int i;
	int l;
	char *tok;
	if (NULL != buffer)
	{
		l = strlen(buffer);
		b = malloc(l+1); 
		strncpy(b, buffer, l+1);
		p = b;
		for (i = 0 ; i < maxargs ; i++)
		{
			tok = strtok(p,",");
			p = NULL;
			if (NULL != tok)
			{
				char *cp = NULL;
				args[i] = simple_strtoul(tok, &cp, 0);
				suffixes[i] = '\0';
				if (NULL != cp)
				{
					if (isalpha(*cp) && (NULL != suffixes))
					{
						suffixes[i] = toupper(*cp);
					} 
				}
				ret++;
			}
			else
			{
				break;
			}
		
		}
	}
	if (b) free(b);
	return(ret); 
}


/**
 * parse_env_string_plus_nums - get named environment value of format FIELD=name:n1,n2,n3....
 * @buffer: pointer environment variable to parse
 * @name: pointer to buffer to which name will be copied [ caller is required to free this pointer ]
 * @maxargs: max number of arguments to parse
 * @args: pointer to array of unsigned longs for numeric arguments
 * @suffixes: pointer to array of chars for suffix characters
 *
 * returns:
 *     number of arguments total  (name + args) if successful
 *     0 if not found
 */
int parse_env_string_plus_nums(const char *buffer, char **name, const int maxargs, unsigned long *args, char *suffixes)
{
	int ret = 0;
	char *b;
	int i;
	int l;
	char *tok;
	if (NULL != buffer)
	{
		l = strlen(buffer);
		b = malloc(l+1); 
		if (NULL == b) {
			return(0);
		}
		strncpy(b, buffer, l+1);
		tok = strtok(b,":");
		*name = b; 
		ret++;
		for (i = 0 ; i < maxargs ; i++)
		{
			tok = strtok(NULL,",");
			if (NULL != tok)
			{
				char *cp = NULL;
				args[i] = simple_strtoul(tok, &cp, 0);
				suffixes[i] = '\0';
				if (NULL != cp)
				{
					if (isalpha(*cp) && (NULL != suffixes))
					{
						suffixes[i] = toupper(*cp);
					} 
				}
				ret++;
			}
			else
			{
				break;
			}
		
		}
	}
	return(ret); 
}

#ifdef CONFIG_MTD
#define UBOOT_NAND_MTD_ID		"nand%d"
#define BRCM_NAND_DRV_NAME		"brcm-nand"
#define SPI_NAND_DRV_NAME		"spi_nand"

static struct mtd_info *check_nand_driver(char* drv_name)
{
	struct mtd_info *mtd = NULL;
#ifdef CONFIG_NAND
	int i;
	char nand_mtd[16];

	for (i = 0; i < CONFIG_SYS_MAX_NAND_DEVICE; i++) {
		sprintf(nand_mtd, UBOOT_NAND_MTD_ID, i);
		mtd = get_mtd_device_nm(nand_mtd);
		if (!IS_ERR_OR_NULL(mtd)) {
			if (strcmp(mtd->dev->driver->name, drv_name) == 0) {
				break;
			} else {
				put_mtd_device(mtd);
				mtd = NULL;
			}
		}
	}
#endif
	return mtd;
}

struct mtd_info *bcmbca_get_nand_mtd_dev(void)
{
	return check_nand_driver(BRCM_NAND_DRV_NAME);
}

struct mtd_info *bcmbca_get_spinand_mtd_dev(void)
{
	return check_nand_driver(SPI_NAND_DRV_NAME);
}

struct mtd_info * bcmbca_get_spinor_mtd_dev(void)
{
	return get_mtd_device_nm(SPIFLASH_MTDNAME);
}

int bcmbca_is_nand_detected(void)
{
	struct mtd_info *mtd = NULL;
	int ret = 0;

	mtd = check_nand_driver(BRCM_NAND_DRV_NAME);
	if (!IS_ERR_OR_NULL(mtd)) {
		put_mtd_device(mtd);
		ret = 1;
	}
	debug("bcmbca_is_nand_detected %p\n", mtd);
	return ret;
}

int bcmbca_is_spinand_detected(void)
{
	struct mtd_info *mtd = NULL;
	int ret = 0;

	mtd = check_nand_driver(SPI_NAND_DRV_NAME);
	if (!IS_ERR_OR_NULL(mtd)) {
		put_mtd_device(mtd);
		ret = 1;
	}

	debug("bcmbca_is_spinand_detected %p\n", mtd);
	return ret;
}

int bcmbca_is_spinor_detected(void)
{
	struct mtd_info *mtd = NULL;
	int ret = 0;

	mtd = get_mtd_device_nm(SPIFLASH_MTDNAME);
	if (!IS_ERR_OR_NULL(mtd)) {
		put_mtd_device(mtd);
		ret = 1;
	}

	debug("bcmbca_is_spinor_detected %p\n", mtd);
	return ret;
}

int bcmbca_is_emmc_detected(void)
{
#ifdef CONFIG_DM_MMC
	debug("bcmbca_is_emmc detected %d\n", get_mmc_num());
	return get_mmc_num();
#else
	return 0;
#endif
}

/* currently we only support image and loader mtds that are on the same flash device
   so look for image mtd that comes from the boot device */
struct mtd_info *bcmbca_get_image_mtd_device(void)
{
	struct mtd_info *mtd = NULL;
	int bdev;

	bdev = bcm_get_boot_device();
	if (bdev == BOOT_DEVICE_NAND)
		mtd = bcmbca_get_nand_mtd_dev();
	else if (bdev == BOOT_DEVICE_SPI)
		mtd = bcmbca_get_spinand_mtd_dev();
	else if (bdev == BOOT_DEVICE_NOR)
		mtd = bcmbca_get_spinor_mtd_dev();

	return mtd;
}
#endif

int get_binary_from_bundle(ulong bundle_addr, const char * conf_name, const char * name, char ** bin_name, ulong * addr, ulong * size)
{
	char path[128];
	int conf_nodeoffset = 0;
	int nodeoffset = 0;
	int len;
	int ret = -1;

	if ( !conf_name || !name || !bin_name ) {
		printf("ERROR: Invalid conf_name %p, bin_name %p, name %p\n",
			conf_name, bin_name, name);
		return ret;
	}

	/* retrieve configuration node */
	sprintf(path, "/configurations/%s", conf_name);
	conf_nodeoffset = fdt_path_offset((void *)bundle_addr, path);
	if( conf_nodeoffset < 0 ) {
		printf("ERROR: %s node not found in bundle!\n", conf_name);
		return ret;
	}

	/* Get actual name of fit node for bundle component */
	*bin_name = (char *)fdt_getprop( (void *)bundle_addr, conf_nodeoffset, name, &len);
	if( !*bin_name ) {
		printf("INFO: %s not found in configuration %s ...skipping!\n", name, conf_name);
		return ret;
	}
	
	/* Retrieve node offset for bundle component */
	sprintf(path, "/images/%s", *bin_name);
	nodeoffset = fdt_path_offset((void *)bundle_addr, path);
	if( nodeoffset < 0 ) {
		printf("ERROR: %s node not found in bundle!\n", path);
		return ret;
	}
	
	/* Get location and size of binary's data */
	ret=fit_image_get_data_and_size((void *)bundle_addr, nodeoffset, (const void**)addr, (size_t*)size);
	if( ret != 0 ||  !*size ) {
		printf("ERROR: %s data not found in bundle!\n", *bin_name);
		return ret;
	}
	return 0;
}
