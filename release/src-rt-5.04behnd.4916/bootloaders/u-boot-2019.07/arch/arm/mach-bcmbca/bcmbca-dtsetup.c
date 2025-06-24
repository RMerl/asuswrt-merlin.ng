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
#include "bca_common.h"
#include "early_abort.h"
#include "bcm_strap_drv.h"
#include <rng.h>
#include <version.h>
#include <dm/uclass.h>
#if defined(CONFIG_BCMBCA_VFBIO)
#include "itc_rpc.h"
#include "vfbio.h"
#endif
#include "bcm_otp.h"

char * get_loader_media(void);

#define FDT_PAD_BYTES 0x1000

DECLARE_GLOBAL_DATA_PTR;

#if defined(CONFIG_DM_RNG)	
static int set_kaslr_seed(void *blob)
{
	int off_chosen, ret=0;
	uint64_t value;
	char * node_name = "kaslr-seed";
	struct udevice *dev;

	if( env_get_hex("kaslr_enabled", 0) )
	{
		if (uclass_get_device(UCLASS_RNG, 0, &dev) || !dev) {
			printf("No RNG device\n");
			return CMD_RET_FAILURE;
		}

		if (dm_rng_read(dev, (void*)&value, sizeof(uint64_t))) {
			printf("Reading RNG failed\n");
			ret = CMD_RET_FAILURE;
			return ret;
		} 

		off_chosen=fdt_path_offset (blob, "/chosen");
		if (off_chosen < 0) {
			return ret;
		}
		ret = fdt_setprop(blob, off_chosen, node_name, (u8*)&value, sizeof(uint64_t));
		if(ret != 0 ) {
			printf("fdt_setprop failed for %s [%d]\n", node_name, ret);
			return ret;
		}
	}

	return ret;
}
#endif

#if !defined(CONFIG_BCMBCA_IKOS)
/* set chip manufacture process info */
static int set_chip_info(void *blob)
{
	int ret = -1;
	int root_node, chip_node;
	u32 val;

	root_node = fdt_path_offset (blob, "/");
	if (root_node < 0)
		return ret;

	/* Add chip_info node in linux dtb */
	chip_node = fdt_add_subnode(blob, root_node, "chip_info");
	if (chip_node < 0) {
		printf("ERROR: Could not create chip info node!\n");
		return ret;
	}

#if defined(CONFIG_BCM6765)
	/* workaround the OTP error on certain 6765 wafers */
	if (bcm_otp_get(OTP_MAP_MFG_PROG, &val))
		return ret;

	if (val == 0x2e69) {
		ret = fdt_setprop_u32(blob, chip_node, "process", 0x4);
		ret |= fdt_setprop_u32(blob, chip_node, "substrate", 0x1);
		ret |= fdt_setprop_u32(blob, chip_node, "foundry", 0x3);
		return ret;
	}
#endif

	/* Add chip serial number */
	ret = bcm_otp_get_chip_ser_num(&val);
	if (ret == 0) {
		if(val) {
			char value[64];
			sprintf(value,"%08x", val);
			ret = fdt_setprop_string(blob, chip_node, "serial-number", value);
			if (ret) {
				printf("fdt_setprop failed for serial-number %d\n", ret);
				return ret;
			}
		}
	}

	ret = bcm_otp_get_mfg_process(&val);
	if (ret == 0) {
		ret = fdt_setprop_u32(blob, chip_node, "process", val);
		if (ret) {
			printf("fdt_setprop failed for mfg process %d\n", ret);
			return ret;
		}
	}

	ret = bcm_otp_get_mfg_substrate(&val);
	if (ret == 0) {
		ret = fdt_setprop_u32(blob, chip_node, "substrate", val);
		if (ret) {
			printf("fdt_setprop failed for mfg substrate %d\n", ret);
			return ret;
		}
	}

	ret = bcm_otp_get_mfg_foundry(&val);
	if (ret == 0) {
		ret = fdt_setprop_u32(blob, chip_node, "foundry", val);
		if (ret) {
			printf("fdt_setprop failed for mfg foundry %d\n", ret);
			return ret;
		}
	}

	return 0;
}

static int set_loader_info(void *blob)
{
	int off_linux, ret=0;
	int  len = 0;	
	int loader_info_offs, chosen_offs;
	int  compat_ver = 0;
	int loader_img_idx = 0;
	char * value = NULL;
	int loader_info_node;

	off_linux=fdt_path_offset (blob, "/");
	if (off_linux < 0) {
		return ret;
	}

 	chosen_offs = fdt_path_offset(gd->fdt_blob, "/chosen");
 	if (chosen_offs < 0) {
 	        debug("INFO: Didnt find /chosen node in uboot DTB\n");
 	        return ret;
 	}

	loader_info_offs = fdt_path_offset(gd->fdt_blob, "/chosen/loader_info");
	if (loader_info_offs < 0) {
		debug("INFO: Didnt find /chosen/loader_info node in uboot DTB\n");
		return ret;
	}

	/* Add loader_info node in linux dtb */
	loader_info_node = fdt_add_subnode(blob, off_linux, "loader_info");
	if( loader_info_node < 0) {
		printf("ERROR: Could not create %s node!\n", "loader_info");
		return ret;
	}

	value = (char*)(fdt_getprop(gd->fdt_blob, chosen_offs, "active_image", &len));
	if( value ) {
		loader_img_idx = be32_to_cpu(*(int*)value);
		ret = fdt_setprop_u32(blob, loader_info_node, "loader_img_idx", loader_img_idx);
		if(ret != 0 )
			printf("fdt_setprop failed for %s [%d]\n", "loader_img_idx", ret);
	}

	value = (char*)(fdt_getprop(gd->fdt_blob, loader_info_offs, "tpl_min_compat", &len));
	if( value ) {
		compat_ver = be32_to_cpu(*(int*)value);
		ret = fdt_setprop_u32(blob, loader_info_node, "tpl_min_compat", compat_ver);
		if(ret != 0 )
			printf("fdt_setprop failed for %s [%d]\n", "tpl_min_compat", ret);
	}

	value = (char*)(fdt_getprop(gd->fdt_blob, loader_info_offs, "build_date", &len));
	if( value ) {
		ret = fdt_setprop_string(blob, loader_info_node, "build_date", value);
		if(ret != 0 )
			printf("fdt_setprop failed for %s [%d]\n", "build_date", ret);
	}

	value = (char*)(fdt_getprop(gd->fdt_blob, loader_info_offs, "build_tag", &len));
	if( value ) {
		ret = fdt_setprop_string(blob, loader_info_node, "build_tag", value);
		if(ret != 0 )
			printf("fdt_setprop failed for %s [%d]\n", "build_tag", ret);
	}

	return ret;
}

static int set_sec_state(void *blob)
{
	int off_linux, ret=0;
	int  len = 0;	
	int chosen_offset;
	int  sec_state = 0;
	int * value = NULL;

	off_linux=fdt_path_offset (blob, "/");
	if (off_linux < 0) {
		return ret;
	}

	chosen_offset = fdt_path_offset(gd->fdt_blob, "/chosen");
	if (chosen_offset < 0) {
		debug("INFO: Didnt find /chosen node in boot DTB\n");
		return ret;
	}

	value = (int*)(fdt_getprop(gd->fdt_blob, chosen_offset, "brom_sec_state", &len));

	if( value )
		sec_state = be32_to_cpu(*value);

	ret = fdt_setprop_u32(blob, off_linux, "brom_sec_state", sec_state);
	if(ret != 0 ) {
		printf("fdt_setprop failed for %s [%d]\n", "brom_sec_state", ret);
	}

	return ret;
}

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
			value = (u8*)(fdt_getprop(gd->fdt_blob, exp_item_offset, "value", &len));
		
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

#define U_BOOT_TERSE_VERSION_STRING "U-Boot2019.07(" U_BOOT_DMI_DATE "-" \
U_BOOT_TIME "" U_BOOT_TZ ")"

#ifndef BUILD_TAG
#define BUILD_TAG "\0"
#endif


static int set_uboot_version(void *blob)
{
	char version[64];
	int offset, ret=-1;

	snprintf(version,sizeof(version),"%s%s", U_BOOT_TERSE_VERSION_STRING, BUILD_TAG);
	version[sizeof(version)-1] = '\0';
	offset=fdt_path_offset (blob, "/");
	if(offset >= 0 )
	{
		ret = fdt_setprop(blob, offset, "uboot-version", version, strlen(version));
		if(ret != 0 )
			printf("fdt_setprop failed [%d]\n", ret);
	}
	return ret;
}
#endif

#if !defined(CONFIG_BCMBCA_IKOS)
#if defined(CONFIG_MMC) && defined(CONFIG_SUPPORT_EMMC_BOOT) && defined(CONFIG_BCMBCA_BOARD_SDK)
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
	int elen = CONFIG_ENV_SIZE;
	uint32_t new;
	int i;
	int offset;
#if !defined(CONFIG_BCMBCA_VFBIO)
	char *config;
	char * boot_magic_updated = 0;

	config = env_get("env_boot_magic");
	if (NULL == config)
	{
		printf("env_boot_magic missing in the env\n");
		return -1;
	}
	elen = simple_strtoul(config, NULL, 0);

	/* Check if we need to save the environment to flash due to a env_boot_magic update */
	boot_magic_updated = env_get("env_boot_magic_updated");
	if (boot_magic_updated){
		run_command("env delete env_boot_magic_updated", 0);
		env_save();
	}
#endif
	envbuf = malloc(max(elen + 12, CONFIG_ENV_SIZE + 12));
	if (!envbuf)
	{
		printf("memory allocation failed\n");
		return -2;
	}
	ep = (env_t *) (envbuf + 8);

	ret = env_export(ep);
	for (i = CONFIG_ENV_SIZE; i < elen; i++) {
		envbuf[12 + i] = 0xff;
	}
	new = crc32(0, ep->data, elen - 4);
	memcpy(&ep->crc, &new, sizeof(new));

	offset=fdt_path_offset (blob, "/");
	if(offset >= 0 )
	{
		((int*)envbuf)[1]=elen;
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

	free(envbuf);

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
		if (rootfs) {
			strncat(boot_args, rootfs, 1024-len);
			if (strlen(rootfs) > 128) {
				env_set("rootfs_opts","");
			}
		}

		/* If rootfs is on eMMC, add 'gpt' kernel option which forces 
		 * use of backup GPT header incase the primary header is corrupted */
		if (strstr(rootfs, "mmc")) {
			strncat(boot_args, " gpt", 1024-strlen(boot_args));
		}

		/* any other argment to append? */
		extra_args = env_get("bootargs_append");
		if (extra_args) {
			strncat(boot_args, " ", 1024-strlen(boot_args));
			strncat(boot_args, extra_args, 1024-strlen(boot_args));
			printf("appending extra boot args to linux boot command line:\n");
			printf("   %s\n", extra_args);
		}
		
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

static int get_cma_alignment_from_dt(void *dtb_ptr)
{
	char dt_node_name[64];
	const uint32_t *val;

	sprintf(dt_node_name, "/%s/%s%s", DT_RSVD_NODE_STR, DT_RSVD_PREFIX_STR, CMA_BASE_ADDR_STR);
	val = dtb_get_prop(dtb_ptr, dt_node_name, "align", NULL);

	return val ? be32_to_cpup(val) : SZ_1M;
}

static void set_reserved_memory(void *dtb_ptr, bd_t *bd)
{
	uint64_t rsrv_mem_end = bd->bi_dram[0].size + bd->bi_dram[0].start;
	uint64_t mem_size = rsrv_mem_end;
	uint64_t rsrv_mem_required = 0, rsrv_mem_start;
	uint64_t cma_required = 0, cma_start = (uint64_t)(-1);
	char dt_node_name[64];
	int use_max_from_env_and_dt = 0;
	uint64_t size, ba_mem = 0;
	char *bootargs, *pmem, *endp;
	int hw_req = 0;
	
#ifdef CONFIG_RSVD_USE_MAX_FROM_ENV_AND_DT
	use_max_from_env_and_dt = 1;
#endif
	
#if defined(BQ16) || defined(BQ16_PRO) || defined(BT10) || defined(RTBE82U) || defined(RTBE82M) || defined(GSBE18000) || defined(GSBE12000) || defined(GS7_PRO) || defined(GT7)
	char cma_str[30] = {0};
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
				 {0, 1, DDOSMEM_BASE_ADDR_STR, ENV_DDOSMEM, 0},
				 {0, 1, DHD_BASE_ADDR_STR, ENV_DHD0, 0}, {0, 1, DHD_BASE_ADDR_STR_1, ENV_DHD1, 0},
				 {0, 1, DHD_BASE_ADDR_STR_2, ENV_DHD2, 0}, {0, 1, DHD_BASE_ADDR_STR_3, ENV_DHD3, 0},
				 {0, 1, NULL, NULL, 0}}, *params_ptr = params;

	if(!dtb_ptr){
		printf("RSVD: ERROR  NULL dtb pointer!");
		return;
	}

	if ((bootargs = env_get("bootargs_append"))) {
		/* check if bootargs has mem arg that override the total memory for linux */
		if ((pmem = strstr(bootargs, "mem="))) {
			pmem += 4;
			ba_mem = ustrtoull(pmem, &endp, 0);
			if (ba_mem != 0 && ba_mem < rsrv_mem_end)
				mem_size = rsrv_mem_end = ba_mem;
		}

		/* check if system cma is reserved */
		if ((pmem = strstr(bootargs, "cma="))) {
			pmem += 4;
			cma_required = ustrtoull(pmem, &endp, 0);
			if (*endp == '@') {
				cma_start = ustrtoull(endp + 1, &endp, 0);
			}
		}
	}
#if defined(CONFIG_BCM63138) || defined(CONFIG_BCM63148)
	if (rsrv_mem_end > 256 * SZ_1M) {
		rsrv_mem_end = 256 * SZ_1M;
		hw_req = 1;
	}
#elif defined(CONFIG_BCM6855)
	if (rsrv_mem_end > 1024 * SZ_1M) {
		rsrv_mem_end = 1024 * SZ_1M;
		hw_req = 1;
	}
#elif defined(CONFIG_BCM6766)
	if (rsrv_mem_end > 768 * SZ_1M) {
		rsrv_mem_end = 768 * SZ_1M;
		hw_req = 1;
	}
#endif	

	/* We support only alocation using CMA */
	/* Some device like 63178, 47622 don't use CMA, just exit here */
	if (!is_cma_rsvmem_enabled(dtb_ptr)){
		return;
	}	

#if defined(BQ16) || defined(BQ16_PRO)
	snprintf(cma_str, sizeof(cma_str), "%s", env_get("bootargs_append"));
	if(strcmp(cma_str, "cma=168M") == 0){
		printf("BQ16 and BQ16_PRO  CMA is already:168M\n");
	}else{
		printf("BQ16 and BQ16_PRO  CMA update:168M\n");
		env_set("bootargs_append","cma=168M");
	}
#endif

#if defined(BT10)
	// BT10 is BCM6766 and has CMA
	snprintf(cma_str, sizeof(cma_str), "%s", env_get("bootargs_append"));
	if(strcmp(cma_str, "cma=64M vmalloc=384") == 0){
		printf("BT10 is already: CMA is 64M and vmalloc is 384M\n");
	}else{
		printf("BT10 overwrites: CMA is 64M and vmalloc is 384M\n");
		env_set("bootargs_append","cma=64M vmalloc=384M");
	}
#endif

#if defined(RTBE82U) || defined(RTBE82M)
	// RT-BE82U is BCM6766 but has no CMA
	snprintf(cma_str, sizeof(cma_str), "%s", env_get("bootargs_append"));
	if(strcmp(cma_str, "vmalloc=384") == 0){
		printf("RT-BE82U is already: vmalloc is 384M\n");
	}else{
		printf("RT-BE82U overwrites: vmalloc is 384M\n");
		env_set("bootargs_append","vmalloc=384M");
	}
#endif

#if defined(GSBE18000) || defined(GSBE12000) || defined(GS7_PRO) || defined(GT7)
	// GS-BE18000 is BCM6766 and has CMA
	snprintf(cma_str, sizeof(cma_str), "%s", env_get("bootargs_append"));
	if(strcmp(cma_str, "cma=64M vmalloc=384") == 0){
		printf("GS-BE18000 is already: CMA is 64M and vmalloc is 384M\n");
	}else{
		printf("GS-BE18000 overwrites: CMA is 64M and vmalloc is 384M\n");
		env_set("bootargs_append","cma=64M vmalloc=384M");
	}
#endif

#ifdef RTBE86U
	env_set_ulong("dhd0", 0);
#endif
#if defined(GSBE18000) || defined(GSBE12000) || defined(GS7_PRO) || defined(GT7)
	env_set_ulong("dhd0", 11);
#endif
#if defined(RTBE58U) || defined(TUFBE3600) || defined(RTBE92U) || defined(RTBE82U) || defined(TUFBE82) || defined(RTBE82M) || defined(RTBE58U_PRO) || defined(RTBE58U_V2) || defined(TUFBE3600_V2) || defined(RTBE55)
	env_set_ulong("dhd0", 0);
	env_set_ulong("dhd1", 0);
#endif
#ifdef RTBE95U
	env_set_ulong("dhd0", 11);
	env_set("bootargs_append","cma=64M");
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
				hw_req = 1;
				if ((get_dsl_excl_range(dtb_ptr, params_ptr->of_str, &adsl_excl_low, &adsl_excl_high) == 0) && (adsl_excl_low != 0)) {
					printf("adsl_excl_memory_range: low=0x%llX high=0x%llX rsv_size=0x%llX rsrv_mem_end=0x%llX\n", adsl_excl_low, adsl_excl_high, size, rsrv_mem_end);
					if ((rsrv_mem_end > adsl_excl_low) && ((rsrv_mem_end - size) < adsl_excl_high))
						rsrv_mem_end = adsl_excl_low;
				}
				else {
					if( rsrv_mem_end > 256*SZ_1M )
						rsrv_mem_end = 256*SZ_1M;
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
			
		params_ptr->size = ALIGN(params_ptr->size, get_cma_alignment_from_dt(dtb_ptr));

		/* make sure we don't reserve too much memory in total that can cause
		 * conflict with linux. Print big warining */
		if ((rsrv_mem_required + params_ptr->size) > (rsrv_mem_end - (32 * SZ_1M))) {
			/* out of the range, worning and skip it */
			printf("RSVD: Not enough memory %s skipped!\n", params_ptr->of_str);
			printf("Requested 0x%llx already alocated 0x%llx, rsrv_mem_end 0x%llx\n",
			       params_ptr->size, rsrv_mem_required, rsrv_mem_end);
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
			env_set_ulong(params_ptr->env_str, params_ptr->size_in_mb ? params_ptr->size/SZ_1M : params_ptr->size);
		}
		printf("RSVD: Allocated for %s    %uMB\n", params_ptr->of_str, (uint32_t)(params_ptr->size/SZ_1M));
		params_ptr++;
	}
	
	/* kernel want 4MB aligned in both start address and size
	   rsrv_mem_end already aligned so need care only of rsrv_mem_required*/
	rsrv_mem_required = ALIGN(rsrv_mem_required, SZ_4M);
	rsrv_mem_start = rsrv_mem_end - rsrv_mem_required;

	if (cma_required) {
		printf("System CMA memory 0x%08x bytes", (uint32_t)cma_required);
		if (cma_start != (uint64_t)(-1))
			printf(" @ 0x%08x", (uint32_t)cma_start);
		printf("\n");
	}
	/* check if there is any conflict with system cma reserveation at specific location */
	if (cma_start != (uint64_t)(-1)) {
		if (rsrv_mem_end > cma_start && rsrv_mem_start < (cma_start + cma_required)) {
			/* 
			  If brcm mem reservation is required by hw, nothing we can do here. Let kernel
			  fail the system cma allocation and user has to change the cma setting. Otherwise
			  try to move the brcm mem reservation location
			*/
			if (!hw_req) {
				if (cma_start - rsrv_mem_required > (32 * SZ_1M))
					rsrv_mem_start = cma_start - rsrv_mem_required;
				else if (cma_start + cma_required < mem_size -rsrv_mem_required)
					rsrv_mem_start  = mem_size -rsrv_mem_required;
				else
					hw_req = 1;
			}
			if (hw_req) {
				printf("System CMA overlap with device CMA! System CMA allocation will fail!\n"
					"Adjust system CMA location if possible\n");
			}
		}
	}

	sprintf(dt_node_name, "%s%s", DT_RSVD_PREFIX_STR, CMA_BASE_ADDR_STR);
	if (!dtb_set_reserved_memory(dtb_ptr, dt_node_name, rsrv_mem_start, rsrv_mem_required))
		printf("RSVD: Total 0x%08x bytes Device CMA reserved memory @ 0x%08x\n",
		   (uint32_t)rsrv_mem_required, (uint32_t)rsrv_mem_start);
	else
		printf("RSVD: ERROR set CMA reserved memory!\n");
}

static void set_avs_mode(void *dtb_ptr)
{
    char *env_var = env_get("avs_mode");
    char *node_path = "/cpufreq";
	int offset;

    if (!dtb_ptr)
    {
        printf("AVS_MODE: ERROR NULL dtb pointer\n");
        return;
    }

    if (!env_var)
        return;
        
    if ((strcmp(env_var, "dvfs") != 0) &&
        (strcmp(env_var, "dfs") != 0) &&
        (strcmp(env_var, "avs") != 0)) 
    {
        printf("AVS_MODE: ERROR Wrong avs_mode = %s\n", env_var);
        return;
    }

	offset = fdt_path_offset(dtb_ptr, node_path);
    if (offset == -FDT_ERR_NOTFOUND)
    {
        printf("AVS_MODE: ERROR Wrong node = %s\n", node_path);
        return;
    }
    if (strcmp(env_var, "avs") == 0)
    {
        fdt_setprop_string(dtb_ptr, offset, "compatible", "brcm,donotrun");
    }
    else
    {
        fdt_setprop_string(dtb_ptr, offset, "op-mode", env_var);
    }
}
#endif

#if !defined(CONFIG_BCMBCA_IKOS)
static int set_flash_node(void* fdt_addr)
{
	int rc = 0;
#if defined(CONFIG_BCMBCA_VFBIO)
	char *name;
	uint32_t rpc_ph = 0;
	char str[80];
	uint32_t blk_s, blk_n;
	int node, subnode, id;

	rc = rpc_tunnel_get_name(RPC_TUNNEL_VFLASH_SMC_NS, &name);
	if(rc)
		return -1;
	sprintf(str, "/rpc/%s", name);
	node = fdt_path_offset(fdt_addr, str);
	if(node > 0)
		rpc_ph = fdt_get_phandle(fdt_addr, node);
	if(!rpc_ph)
	{
		printf("Failed to find VFLASH->SMC RPC channel, device may not boot properly\n");
		return -1;
	}
	
	node = fdt_path_offset(fdt_addr, "/flash");
	if(node < 0) {
		printf("Failed to set flash node properties, device may not boot properly\n");
		return -1;
	}

	rc = vfbio_lun_get_next(-1, &id);
	for(; rc == 0; rc = vfbio_lun_get_next(id, &id))
	{
		
		rc = vfbio_lun_get_blk_size(id, &blk_s);
		if(!rc)
			rc = vfbio_lun_get_blk_num(id, &blk_n);
		if(rc)
		{
			printf("Failed to get LUN info, device may not boot properly\n");
			return -1;
		}
		
		sprintf(str, "lun@%d", id);
		subnode = fdt_add_subnode(fdt_addr, node, str);
		if(subnode < 0)
		{
			printf("Failed to create LUN node, device may not boot properly\n");
			return -1;
		}
		rc = fdt_setprop_u32(fdt_addr, subnode, "rpc-channel", rpc_ph);
		if(!rc)
			rc = fdt_setprop_string(fdt_addr, subnode, "lun-name", vfbio_lun_get_name(id));
		if(!rc)
			rc = fdt_setprop_u32(fdt_addr, subnode, "reg", id);
		if(!rc)
			rc = fdt_setprop_string(fdt_addr, subnode, "compatible", "brcm,vfbio");
		if(rc)
		{
			printf("Failed to set LUN node properties, device may not boot properly\n");
			return -1;
		}
	}
#else
	/* 
	 * Spi nand and spi nor can not coexist on cs0. Only one of them can be
	 * physically connected to cs0. Remove the non-connected node in case both
	 * flashes are enabled in dts
	 */
	if (!bcmbca_is_spinor_detected())
		fdt_status_disabled_by_alias(fdt_addr, "spinor0");

	if (!bcmbca_is_spinand_detected())
		fdt_status_disabled_by_alias(fdt_addr, "spinand0");

	/* emmc and nand can not coexit as they share the same pins in 63138*/
#if defined(CONFIG_BCM63138)
	if (bcmbca_is_nand_detected())
		fdt_status_disabled_by_alias(fdt_addr, "/sdhci");
#else
	if (!bcmbca_is_nand_detected())
		fdt_status_disabled_by_alias(fdt_addr, "nand0");

	if (!bcmbca_is_emmc_detected())
		fdt_status_disabled_by_alias(fdt_addr, "/sdhci");
#endif
#endif

	return rc;
}
#endif

int ft_system_setup(void *blob, bd_t *bd)
{
	int ret = 0;
#if !defined(CONFIG_BCMBCA_IKOS)
	ret |= set_flash_node(blob);
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
	ret |= set_sec_state(blob);
	ret |= set_loader_info(blob);
	ret |= set_chip_info(blob);
#if defined(CONFIG_DM_RNG)	
	ret |= set_kaslr_seed(blob);
#endif

    set_avs_mode(blob);
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
#if defined(CONFIG_ARM64) || defined(CONFIG_ARMV7_LPAE)
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
#if defined(CONFIG_ARM64) || defined(CONFIG_ARMV7_LPAE)
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
		if (tplp->early_flags&SPL_EA_DDR_SAFE_MODE_MASK) {
			if(fdt_setprop_u32(fdt_addr, offset, "safemode", 1))
				printf("Could not set ddr safe mode in the fdt, "
					"device may not boot properly\n");
		}
	}
}

__attribute__ ((weak))
int get_nr_cpus(void)
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
