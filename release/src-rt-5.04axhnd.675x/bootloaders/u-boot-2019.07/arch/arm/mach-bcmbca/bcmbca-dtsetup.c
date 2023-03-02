#include <common.h>
#include <malloc.h>
#include <spl.h>

#if defined(CONFIG_MMC) && defined(CONFIG_SUPPORT_EMMC_BOOT)
#include <mmc.h>
#endif

#include "bcmbca-dtsetup.h"
#include "dt_helper.h"
#include "boot_flash.h"
#include "spl_env.h"

#define FDT_PAD_BYTES 0x1000

DECLARE_GLOBAL_DATA_PTR;

static int set_sec_exports(void *blob)
{
	int off_linux, ret=0;
	int  len = 0;	
	u8 * value;
	int trust_offset,exp_item_offset;
	char * exp_flag;
	char * node_name;
	int ndepth;

	off_linux=fdt_path_offset (blob, "/");
	if (off_linux < 0) {
		return ret;
	}

	trust_offset = fdt_path_offset(gd->fdt_blob, "/trust");
	if (trust_offset < 0) {
		debug("INFO: Didnt find /trust node in boot DTB\n");
		return ret;
	}

	for (ndepth = 0, exp_item_offset = fdt_next_node(gd->fdt_blob, trust_offset, &ndepth);
		(exp_item_offset >= 0) && (ndepth > 0);
		exp_item_offset = fdt_next_node(gd->fdt_blob, exp_item_offset, &ndepth)) {
		if (ndepth == 1) {
				/*
				 * Direct child node of the trust parent node,
				 * i.e. item node.
				 */
			node_name = (char*)fdt_get_name(gd->fdt_blob, exp_item_offset, &len);
			exp_flag = (char*)(fdt_getprop(gd->fdt_blob, exp_item_offset, "export", &len));
			value = (char*)(fdt_getprop(gd->fdt_blob, exp_item_offset, "value", &len));
		
			if( exp_flag && len && value && node_name && (strcasecmp(exp_flag, "yes") == 0) ) {
				ret = fdt_setprop(blob, off_linux, node_name, value, len);
				if(ret != 0 ) {
					printf("fdt_setprop failed for %s [%d]\n", node_name, ret);
					return ret;
				}
			}
		}
	}
	return ret;
}

static int set_uboot_version(void *blob)
{
	char version[64];
	int offset, ret=-1;

	display_options_get_banner(false, version, sizeof(version));
	offset=fdt_path_offset (blob, "/");
	if(offset >= 0 )
	{
		ret = fdt_setprop(blob, offset, "uboot-version", version, strlen(version));
		if(ret != 0 )
			printf("fdt_setprop failed [%d]\n", ret);
	}
	return ret;
}

#if defined(CONFIG_MMC) && defined(CONFIG_SUPPORT_EMMC_BOOT)
static int set_emmc_boot_partition(void *blob)
{

int mmc_boot_part = 0, ret=-1, offset;
char mmc_boot_part_str[10];
struct mmc *mmc = NULL;

	mmc = find_mmc_device(0);
	if( !mmc ) {
		printf("Error: Cannot access mmc device!\n");
		return ret;
	}

	/* Get active boot partition */
	mmc_boot_part = EXT_CSD_EXTRACT_BOOT_PART(mmc->part_config);

	/* Default to first boot partition if nothing is set */
	if( !mmc_boot_part )
		mmc_boot_part = 1;

	mmc_boot_part_str[0]='0'+mmc_boot_part;
	mmc_boot_part_str[1]='\0';
	
	offset=fdt_path_offset (blob, "/");
	if(offset >= 0 )
	{
		ret = fdt_setprop(blob, offset, "emmc_boot_part", mmc_boot_part_str, strlen(mmc_boot_part_str));
		if(ret != 0 )
			printf("fdt_setprop failed [%d]\n", ret);
	}
	return ret;
}
#endif

static int set_uboot_env(void *blob)
{
	env_t *ep;
	char *envbuf = NULL;
	int ret = -2;
	int elen;
	char *config;
	uint32_t new;
	int i;
	char *found = NULL;
	int offset;
	char * boot_magic_updated = 0;

	config = env_get("env_boot_magic");
	if (NULL == config)
	{
		printf("env_boot_magic missing in the env\n");
		return -1;
	}
	elen = simple_strtoul(config, NULL, 0);
	found = malloc(strlen(config) + 1);
	if (!found)
	{
		printf("memory allocation failed\n");
		return -2;
	}
	strcpy(found, config);
	envbuf = malloc(max(elen + 12, CONFIG_ENV_SIZE + 12));
	if (!envbuf)
	{
		printf("memory allocation failed\n");
		return -2;
	}
	ep = (env_t *) (envbuf + 8);

	/* Check if we need to save the environment to flash due to a env_boot_magic update */
	boot_magic_updated = env_get("env_boot_magic_updated");
	if (boot_magic_updated){
		run_command("env delete env_boot_magic_updated", 0);
		env_save();
	}

	ret = env_export(ep);
	for (i = CONFIG_ENV_SIZE; i < elen; i++) {
		envbuf[12 + i] = 0xff;
	}
	new = crc32(0, ep->data, elen - 4);
	memcpy(&ep->crc, &new, sizeof(new));

	offset=fdt_path_offset (blob, "/");
	if(offset >= 0 )
	{
		//add 0x1000 (4096) padding bytes, 12 bytes for env header
		ret=fdt_increase_size(blob, max(elen + 12, CONFIG_ENV_SIZE + 12)+FDT_PAD_BYTES);
		ret = fdt_setprop(blob, offset, "uboot_env", envbuf, max(elen + 12, CONFIG_ENV_SIZE + 12));
		if(ret != 0 )
			printf("fdt_setprop failed [%d]\n", ret);
	}
	else
	{
		printf("can't append uboot env to dtb, / node not found\n");
		ret=-4;
	}

	return ret;
}

/* Todo: port the remaining code from cfe dtb_set_bootcmd(void) */
static int set_bootargs(void *blob)
{
	char* boot_args;
	char* extra_args, *mtd_parts, *rootfs;
	int len = 0;

	boot_args = env_get("bootargs");
	if (boot_args) {
		/* fdt_chosen function already override the bootargs.. just print the msg */
		printf("linux boot command line overriden from bootargs env variable:\n");
		printf("   %s\n", boot_args);
	} else
	{
		boot_args = malloc(1024);
		if (boot_args == NULL) {
			printf("Failed to allocate memory for bootargs!\n");
			return -1;
		}

		memset(boot_args, 0x0, 1024);

		/* append the required bootargs saved in the env variables */
		mtd_parts = env_get("mtdparts");
		if (mtd_parts)
			len = snprintf(boot_args, 1024, "mtdparts=%s ", mtd_parts);

		rootfs = env_get("rootfs_opts");
		if (rootfs)
			strncat(boot_args, rootfs, 1024-len);

		strncat(boot_args, " cma=0M", 1024-strlen(boot_args));

		/* any other argment to append? */
		extra_args = env_get("bootargs_append");
		if (extra_args) {
			strncat(boot_args, " ", 1024-strlen(boot_args));
			strncat(boot_args, extra_args, 1024-strlen(boot_args));
		}

#if (defined(CONFIG_BCM4912) || defined(CONFIG_BCM6855)) && defined(CONFIG_BCM_JUMBO_FRAME)
{
        char *mtuSize;
        char mtuSizeBuf[16];
		memset(mtuSizeBuf, 0, sizeof(mtuSizeBuf));
        mtuSize = env_get("mtusize");
        if (mtuSize) {
            snprintf(mtuSizeBuf, sizeof(mtuSizeBuf), " mtusize=%s", mtuSize);
            strncat(boot_args, mtuSizeBuf, 1024-strlen(boot_args));
        }
}
#endif

		printf("appending extra boot args to linux boot command line:\n");
		printf("   %s\n", boot_args);
		dtb_set_bootargs(blob, boot_args, 1);

		free(boot_args);
	}

	return 0;
}

static int is_cma_rsvmem_enabled(void *fdt_ptr)
{
	char dt_node_name[64];
	int len;
	int ret = 0;

	sprintf(dt_node_name, "/%s/%s%s", DT_RSVD_NODE_STR, DT_RSVD_PREFIX_STR, CMA_BASE_ADDR_STR);
	if (dtb_get_prop(fdt_ptr, dt_node_name, "reg", &len)) {
		ret = 1;
	}
	return ret;
}

static void del_all_cma_nodes(void *dtb_ptr)
{
	char dt_node_name[64];

	sprintf(dt_node_name, "%s%s", DT_RSVD_PREFIX_STR, CMA_BASE_ADDR_STR);
	dtb_del_reserved_memory(dtb_ptr, dt_node_name);
	dtb_del_cma_rsvmem_device(dtb_ptr);

	return;
}

static int get_dsl_excl_range(void *dtb_ptr, char *name, uint64_t* low, uint64_t* high)
{
    int ret = -1;

    ret = dtb_getprop_cma_rsv_param(dtb_ptr, name, "excl-addr-low", low);
    if (ret != 0)
        return ret;
    ret = dtb_getprop_cma_rsv_param(dtb_ptr, name, "excl-addr-high", high);

    return ret;
}

static void set_reserved_memory(void *dtb_ptr, bd_t *bd)
{
	uint64_t mem_end = bd->bi_dram[0].size + bd->bi_dram[0].start;
	uint64_t rsrv_mem_required = 0;
	char dt_node_name[64];
	int use_max_from_env_and_dt = 0;
	uint64_t size;
	
#ifdef CONFIG_RSVD_USE_MAX_FROM_ENV_AND_DT
	use_max_from_env_and_dt = 1;
#endif
	
	struct mem_reserv_prm {
		uint64_t size;
		int      size_in_mb;
		char *   of_str;
		char *   env_str;
		int      from_dt;
	} params[] = {
				  /* DSL memory must be reserved first toward end of 256MB */
				 {0, 0, ADSL_BASE_ADDR_STR, NULL, 0},
				 {0, 1, PARAM1_BASE_ADDR_STR, ENV_RDP1, 0}, {0, 1, PARAM2_BASE_ADDR_STR, ENV_RDP2, 0}, 
				 {0, 1, BUFMEM_BASE_ADDR_STR, ENV_BUFMEM, 0}, {0, 1, RNRMEM_BASE_ADDR_STR, ENV_RNRMEM, 0},
				 {0, 1, DHD_BASE_ADDR_STR, ENV_DHD1, 0}, {0, 1, DHD_BASE_ADDR_STR_1, ENV_DHD2, 0},
				 {0, 1, DHD_BASE_ADDR_STR_2, ENV_DHD3, 0}, {0, 1, NULL, NULL, 0}}, *params_ptr = params;

	if(!dtb_ptr){
		printf("RSVD: ERROT  NULL dtb pointer!");
		return;
	}

#if defined(CONFIG_BCM63138) || defined(CONFIG_BCM63148)
	if (mem_end > 256 * SZ_1M)
		mem_end = 256 * SZ_1M;
#endif	

	/* We support only alocation using CMA */
	/* Some device like 63178, 47622 don't use CMA, just exit here */
	if (!is_cma_rsvmem_enabled(dtb_ptr)){
		return;
	}	

#if defined(XT8PRO) || defined(BM68) || defined(XT8_V2) || defined(ET8PRO) || defined(ET8_V2)
	printf("cathy force RSVD: dhd2=11\n");
	env_set_ulong("dhd2", 11);
	printf("cathy force RSVD: dhd0=0\n");
	env_set_ulong("dhd0", 0);
#endif

#if defined(RTAX58U_V2) || defined(TUFAX3000_V2) || defined(RTAXE7800) || defined(RTAX3000N) || defined(BR63) || defined(RTAX82U_V2) || defined(RPAX58) || defined(XD4PRO) || defined(TUFAX5400_V2) || defined(XD6_V2) || defined(RTAX5400)
	env_set_ulong("dhd0", 0);
#endif
#if defined(GT10)
	env_set_ulong("dhd0", 11);
#endif
#if defined(GTAX6000) || defined(RTAXE7800) || defined(GT10) || defined(RTAX82U_V2) || defined(TUFAX5400_V2) || defined(XD6_V2) || defined(RTAX5400)
	env_set_ulong("dhd1", 11);
#endif
#if defined(GT10)
	env_set_ulong("dhd2", 0);
#endif

	/*compute alocation memory*/
	while (params_ptr->of_str){
		size = 0;
		/* get size from environment */
		if (params_ptr->env_str){
			params_ptr->size = env_get_ulong(params_ptr->env_str, 0, 0);
			if (params_ptr->size_in_mb)
				params_ptr->size *= SZ_1M;
		}
		/* if no environment for the parameter or CONFIG_ENV_NOT_OVERRIDE defined,
		  	get size from DT */
		if (!params_ptr->size || use_max_from_env_and_dt){
			if (dtb_getprop_cma_rsvmem_size(dtb_ptr, params_ptr->of_str, &size) != 0){ 
				params_ptr->size = 0;
				printf("RSVD: not found enrty for %s\n", params_ptr->of_str);
			}
			else if (strcmp(params_ptr->of_str, ADSL_BASE_ADDR_STR) == 0) {
				uint64_t adsl_excl_low = 0, adsl_excl_high;
				if ((get_dsl_excl_range(dtb_ptr, params_ptr->of_str, &adsl_excl_low, &adsl_excl_high) == 0) && (adsl_excl_low != 0)) {
					printf("adsl_excl_memory_range: low=0x%X high=0x%X rsv_size=0x%X mem_end=0x%X\n", adsl_excl_low, adsl_excl_high, size, mem_end);
					if ((mem_end > adsl_excl_low) && ((mem_end - size) < adsl_excl_high))
						mem_end = adsl_excl_low;
				}
				else {
					if( mem_end > 256*SZ_1M )
						mem_end = 256*SZ_1M;
				}
			}
		}
	
		if(size && (!use_max_from_env_and_dt || params_ptr->size < size)){
				params_ptr->from_dt = 1;
				params_ptr->size = size;
		}
		
		if (!params_ptr->size){
			params_ptr++;
			continue;
		}		
			
		params_ptr->size = ALIGN(params_ptr->size, SZ_1M);

		/* make sure we don't reserve too much memory in total that can cause
		 * conflict with linux. Print big warining */
		if ((rsrv_mem_required + params_ptr->size) > (mem_end - (32 * SZ_1M))) {
			/* out of the range, worning and skip it */
			printf("RSVD: Not enough memory %s skipped!\n", params_ptr->of_str);
			printf("Requested 0x%llx already alocated 0x%llx, mem_end 0x%llx\n",
			       params_ptr->size, rsrv_mem_required, mem_end);
			dtb_del_cma_rsvmem(dtb_ptr, params_ptr->of_str);
			params_ptr->size = 0;
		}
		else
			rsrv_mem_required += params_ptr->size;
		
		params_ptr++;
	}
	
	if (rsrv_mem_required == 0 ) {
		/* remove the cma nodes if no rsvmem is needed */
		del_all_cma_nodes(dtb_ptr);
		printf("RSVD: No CMA memory reserved\n");
		return;
	}
		
	/* update DTB and environmet */
	params_ptr = params;
	
	while (params_ptr->of_str){	
		if (!params_ptr->size){
			params_ptr++;
			continue;
		}
		if(!params_ptr->from_dt){
			/* update DT */
			dtb_setprop_cma_rsvmem_size(dtb_ptr, params_ptr->of_str, params_ptr->size);
		}
		else if (params_ptr->env_str){
			/* propogate in env, make it visible in linux proc */
			env_set_ulong(params_ptr->env_str, params_ptr->size_in_mb?params_ptr->size/SZ_1M:params_ptr->size);
		}
		printf("RSVD: Allocated for %s    %uMB\n", params_ptr->of_str, (uint32_t)(params_ptr->size/SZ_1M));
		params_ptr++;
	}
	
	/* kernel want 4MB aligned in both start address and size
	   mem_end already aligned so need care only of rsrv_mem_required*/
	rsrv_mem_required = ALIGN(rsrv_mem_required, SZ_4M);
	
	sprintf(dt_node_name, "%s%s", DT_RSVD_PREFIX_STR, CMA_BASE_ADDR_STR);
	if (!dtb_set_reserved_memory(dtb_ptr, dt_node_name, mem_end - rsrv_mem_required, rsrv_mem_required))
		printf("RSVD: Total 0x%08x bytes CMA reserved memory @ 0x%08x\n",
		   (uint32_t)rsrv_mem_required, (uint32_t)(mem_end - rsrv_mem_required));
	else
		printf("RSVD: ERROR set CMA reserved memory!\n");
}

static void set_flash_node(void* fdt_addr)
{
	if (bcmbca_get_boot_device() != BOOT_DEVICE_SPI)
		fdt_status_disabled_by_alias(fdt_addr, "spinand0");

	if (bcmbca_get_boot_device() != BOOT_DEVICE_NAND)
		fdt_status_disabled_by_alias(fdt_addr, "nand0")	;  

	if (bcmbca_get_boot_device() != BOOT_DEVICE_NOR)
		fdt_status_disabled_by_alias(fdt_addr, "spinor0")	;  
}

int ft_system_setup(void *blob, bd_t *bd)
{
	int ret = 0;
#if !defined(CONFIG_BCMBCA_IKOS)
	set_flash_node(blob);
	ft_update_cpu_nodes(blob, bd);
	set_reserved_memory(blob, bd);
	ret = set_bootargs(blob);
	ret |= set_uboot_env(blob);
	ret |= set_uboot_version(blob);
#if defined(CONFIG_MMC) && defined(CONFIG_SUPPORT_EMMC_BOOT) && defined(CONFIG_BCMBCA_BOARD_SDK)
	if( strcasecmp(get_loader_media(), FLASH_DEV_STR_EMMC) == 0 )
		ret |= set_emmc_boot_partition(blob);
#endif
	ret |= set_sec_exports(blob);
#endif
	return ret;
}

static inline char * dev_to_string(int bdevice)
{
	char *device_str=NULL;
	switch(bdevice)
	{
		case BOOT_DEVICE_NAND:
		case BOOT_DEVICE_SPI:	/* SPI NAND */	  
			device_str=FLASH_DEV_STR_NAND;
			break;

		case BOOT_DEVICE_NOR:
			device_str=FLASH_DEV_STR_SPINOR;
			break;
		case BOOT_DEVICE_MMC1:
		case BOOT_DEVICE_MMC2:
		case BOOT_DEVICE_MMC2_2:
			device_str=FLASH_DEV_STR_EMMC;
			break;
		
	}
	return device_str;
}

void update_uboot_fdt(void *fdt_addr, tpl_params *tplp)
{
	int offset, regsize;
	uint64_t total_size;
#if defined(CONFIG_ARM64)
	uint64_t reg[4];
	uint64_t split_size = PHYS_SDRAM_1_SIZE;
#if defined(PHYS_SDRAM_2)	
	uint64_t second_base = PHYS_SDRAM_2;
#else
	uint64_t second_base = PHYS_SDRAM_1_SIZE;	
#endif
#else
	uint32_t reg[2];
#endif

	total_size = ((uint64_t)tplp->ddr_size)*SZ_1M;
	
	/* Fixup memory offsets */
	offset=fdt_path_offset (fdt_addr, "/memory");
	if(offset >= 0)
	{
#if defined(CONFIG_ARM64)
		reg[0] = cpu_to_fdt64(0);
		if (total_size <= split_size) {
			reg[1] = cpu_to_fdt64(total_size);
			regsize = 16;
		} else {
		  	reg[1] = cpu_to_fdt64(split_size);
			reg[2] = cpu_to_fdt64(second_base);
			reg[3] = cpu_to_fdt64(total_size-split_size);
			regsize = 32;
		}
#else
		reg[0] = cpu_to_fdt32(0);
		reg[1] = cpu_to_fdt32(total_size);
		regsize = sizeof(reg);
#endif
		if(fdt_setprop(fdt_addr, offset, "reg", reg, regsize))
		{
			printf("Could not set memory node in the fdt, device may not boot properly\n");
		}
	}

	/*get boot device from tplparams */
	offset=fdt_path_offset (fdt_addr, "/chosen");
	if(offset >= 0)
	{
		char *dev_string=dev_to_string(tplp->boot_device);
		if(dev_string != NULL)
		{
			if(fdt_setprop_string(fdt_addr, offset, "boot_device", dev_string))
			{
				printf("Could not set boot device node in the fdt, device may not boot properly\n");
			}
		}

#if defined(CONFIG_BCMBCA_PMC)
		char *avs_disable =
			find_spl_env_val(tplp->environment, ENV_AVS_DISABLE);
		if (avs_disable && *avs_disable == '1' &&
			fdt_setprop(fdt_addr, offset, ENV_AVS_DISABLE,
				avs_disable, sizeof(*avs_disable)))
		{
			printf("Could not set avs_disable=1 in the fdt, "
					"device may not boot properly\n");
		}
#endif
	}

  	//other parameters???
	set_flash_node(fdt_addr);
}

__attribute__ ((weak))
int get_nr_cpus()
{
	return -1;
}

#define CPU_STR "/cpus/cpu@"

void ft_update_cpu_nodes(void *blob, bd_t * bd)
{
	int nr_cpus=get_nr_cpus(), cpu=0;
	int offset;
	char cpu_node_str[30];

	if(nr_cpus >= 1)
	{
		for( cpu = QUAD_CPUS-1; cpu >= nr_cpus; cpu--)
		{
			if(snprintf(cpu_node_str, sizeof(cpu_node_str), "%s%d", CPU_STR, cpu) != -1)
			{
				offset = fdt_path_offset(blob, cpu_node_str);
				if (offset >= 0)
				{
					fdt_del_node(blob, offset);
				}
			}
		}
	}

	return;
}
