// SPDX-License-Identifier: GPL-2.0+
/*
 *
 * Common security related functions for OMAP devices
 *
 * (C) Copyright 2016-2017
 * Texas Instruments, <www.ti.com>
 *
 * Daniel Allred <d-allred@ti.com>
 * Andreas Dannenberg <dannenberg@ti.com>
 * Harinarayan Bhatta <harinarayan@ti.com>
 * Andrew F. Davis <afd@ti.com>
 */

#include <common.h>
#include <stdarg.h>

#include <asm/arch/sys_proto.h>
#include <asm/cache.h>
#include <asm/omap_common.h>
#include <asm/omap_sec_common.h>
#include <asm/spl.h>
#include <asm/ti-common/sys_proto.h>
#include <mapmem.h>
#include <spl.h>
#include <tee/optee.h>

/* Index for signature verify ROM API */
#ifdef CONFIG_AM33XX
#define API_HAL_KM_VERIFYCERTIFICATESIGNATURE_INDEX	(0x0000000C)
#else
#define API_HAL_KM_VERIFYCERTIFICATESIGNATURE_INDEX	(0x0000000E)
#endif

/* Index for signature PPA-based TI HAL APIs */
#define PPA_HAL_SERVICES_START_INDEX        (0x200)
#define PPA_SERV_HAL_TEE_LOAD_MASTER        (PPA_HAL_SERVICES_START_INDEX + 23)
#define PPA_SERV_HAL_TEE_LOAD_SLAVE         (PPA_HAL_SERVICES_START_INDEX + 24)
#define PPA_SERV_HAL_SETUP_SEC_RESVD_REGION (PPA_HAL_SERVICES_START_INDEX + 25)
#define PPA_SERV_HAL_SETUP_EMIF_FW_REGION   (PPA_HAL_SERVICES_START_INDEX + 26)
#define PPA_SERV_HAL_LOCK_EMIF_FW           (PPA_HAL_SERVICES_START_INDEX + 27)

/* Offset of header size if image is signed as ISW */
#define HEADER_SIZE_OFFSET	(0x6D)

int tee_loaded = 0;

/* Argument for PPA_SERV_HAL_TEE_LOAD_MASTER */
struct ppa_tee_load_info {
	u32 tee_sec_mem_start; /* Physical start address reserved for TEE */
	u32 tee_sec_mem_size;  /* Size of the memory reserved for TEE */
	u32 tee_cert_start;    /* Address where signed TEE binary is loaded */
	u32 tee_cert_size;     /* Size of TEE certificate (signed binary) */
	u32 tee_jump_addr;     /* Address to jump to start TEE execution */
	u32 tee_arg0;          /* argument to TEE jump function, in r0 */
};

static uint32_t secure_rom_call_args[5] __aligned(ARCH_DMA_MINALIGN) __section(".data");

u32 secure_rom_call(u32 service, u32 proc_id, u32 flag, ...)
{
	int i;
	u32 num_args;
	va_list ap;

	va_start(ap, flag);

	num_args = va_arg(ap, u32);

	if (num_args > 4) {
		va_end(ap);
		return 1;
	}

	/* Copy args to aligned args structure */
	for (i = 0; i < num_args; i++)
		secure_rom_call_args[i + 1] = va_arg(ap, u32);

	secure_rom_call_args[0] = num_args;

	va_end(ap);

	/* if data cache is enabled, flush the aligned args structure */
	flush_dcache_range(
		(unsigned int)&secure_rom_call_args[0],
		(unsigned int)&secure_rom_call_args[0] +
		roundup(sizeof(secure_rom_call_args), ARCH_DMA_MINALIGN));

	return omap_smc_sec(service, proc_id, flag, secure_rom_call_args);
}

static u32 find_sig_start(char *image, size_t size)
{
	char *image_end = image + size;
	char *sig_start_magic = "CERT_";
	int magic_str_len = strlen(sig_start_magic);
	char *ch;

	while (--image_end > image) {
		if (*image_end == '_') {
			ch = image_end - magic_str_len + 1;
			if (!strncmp(ch, sig_start_magic, magic_str_len))
				return (u32)ch;
		}
	}
	return 0;
}

int secure_boot_verify_image(void **image, size_t *size)
{
	int result = 1;
	u32 cert_addr, sig_addr;
	size_t cert_size;

	/* Perform cache writeback on input buffer */
	flush_dcache_range(
		rounddown((u32)*image, ARCH_DMA_MINALIGN),
		roundup((u32)*image + *size, ARCH_DMA_MINALIGN));

	cert_addr = (uint32_t)*image;
	sig_addr = find_sig_start((char *)*image, *size);

	if (sig_addr == 0) {
		printf("No signature found in image!\n");
		result = 1;
		goto auth_exit;
	}

	*size = sig_addr - cert_addr;	/* Subtract out the signature size */
	/* Subtract header if present */
	if (strncmp((char *)sig_addr, "CERT_ISW_", 9) == 0)
		*size -= ((u32 *)*image)[HEADER_SIZE_OFFSET];
	cert_size = *size;

	/* Check if image load address is 32-bit aligned */
	if (!IS_ALIGNED(cert_addr, 4)) {
		printf("Image is not 4-byte aligned!\n");
		result = 1;
		goto auth_exit;
	}

	/* Image size also should be multiple of 4 */
	if (!IS_ALIGNED(cert_size, 4)) {
		printf("Image size is not 4-byte aligned!\n");
		result = 1;
		goto auth_exit;
	}

	/* Call ROM HAL API to verify certificate signature */
	debug("%s: load_addr = %x, size = %x, sig_addr = %x\n", __func__,
	      cert_addr, cert_size, sig_addr);

	result = secure_rom_call(
		API_HAL_KM_VERIFYCERTIFICATESIGNATURE_INDEX, 0, 0,
		4, cert_addr, cert_size, sig_addr, 0xFFFFFFFF);

	/* Perform cache writeback on output buffer */
	flush_dcache_range(
		rounddown((u32)*image, ARCH_DMA_MINALIGN),
		roundup((u32)*image + *size, ARCH_DMA_MINALIGN));

auth_exit:
	if (result != 0) {
		printf("Authentication failed!\n");
		printf("Return Value = %08X\n", result);
		hang();
	}

	/*
	 * Output notification of successful authentication to re-assure the
	 * user that the secure code is being processed as expected. However
	 * suppress any such log output in case of building for SPL and booting
	 * via YMODEM. This is done to avoid disturbing the YMODEM serial
	 * protocol transactions.
	 */
	if (!(IS_ENABLED(CONFIG_SPL_BUILD) &&
	      IS_ENABLED(CONFIG_SPL_YMODEM_SUPPORT) &&
	      spl_boot_device() == BOOT_DEVICE_UART))
		printf("Authentication passed\n");

	return result;
}

u32 get_sec_mem_start(void)
{
	u32 sec_mem_start = CONFIG_TI_SECURE_EMIF_REGION_START;
	u32 sec_mem_size = CONFIG_TI_SECURE_EMIF_TOTAL_REGION_SIZE;
	/*
	 * Total reserved region is all contiguous with protected
	 * region coming first, followed by the non-secure region.
	 * If 0x0 start address is given, we simply put the reserved
	 * region at the end of the external DRAM.
	 */
	if (sec_mem_start == 0)
		sec_mem_start =
			(CONFIG_SYS_SDRAM_BASE + (
#if defined(CONFIG_OMAP54XX)
			omap_sdram_size()
#else
			get_ram_size((void *)CONFIG_SYS_SDRAM_BASE,
				     CONFIG_MAX_RAM_BANK_SIZE)
#endif
			- sec_mem_size));
	return sec_mem_start;
}

int secure_emif_firewall_setup(uint8_t region_num, uint32_t start_addr,
			       uint32_t size, uint32_t access_perm,
			       uint32_t initiator_perm)
{
	int result = 1;

	/*
	 * Call PPA HAL API to do any other general firewall
	 * configuration for regions 1-6 of the EMIF firewall.
	 */
	debug("%s: regionNum = %x, startAddr = %x, size = %x", __func__,
	      region_num, start_addr, size);

	result = secure_rom_call(
			PPA_SERV_HAL_SETUP_EMIF_FW_REGION, 0, 0, 4,
			(start_addr & 0xFFFFFFF0) | (region_num & 0x0F),
			size, access_perm, initiator_perm);

	if (result != 0) {
		puts("Secure EMIF Firewall Setup failed!\n");
		debug("Return Value = %x\n", result);
	}

	return result;
}

#if	(CONFIG_TI_SECURE_EMIF_TOTAL_REGION_SIZE <  \
	CONFIG_TI_SECURE_EMIF_PROTECTED_REGION_SIZE)
#error	"TI Secure EMIF: Protected size cannot be larger than total size."
#endif
int secure_emif_reserve(void)
{
	int result = 1;
	u32 sec_mem_start = get_sec_mem_start();
	u32 sec_prot_size = CONFIG_TI_SECURE_EMIF_PROTECTED_REGION_SIZE;

	/* If there is no protected region, there is no reservation to make */
	if (sec_prot_size == 0)
		return 0;

	/*
	 * Call PPA HAL API to reserve a chunk of EMIF SDRAM
	 * for secure world use. This region should be carved out
	 * from use by any public code. EMIF firewall region 7
	 * will be used to protect this block of memory.
	 */
	result = secure_rom_call(
			PPA_SERV_HAL_SETUP_SEC_RESVD_REGION,
			0, 0, 2, sec_mem_start, sec_prot_size);

	if (result != 0) {
		puts("SDRAM Firewall: Secure memory reservation failed!\n");
		debug("Return Value = %x\n", result);
	}

	return result;
}

int secure_emif_firewall_lock(void)
{
	int result = 1;

	/*
	 * Call PPA HAL API to lock the EMIF firewall configurations.
	 * After this API is called, none of the PPA HAL APIs for
	 * configuring the EMIF firewalls will be usable again (that
	 * is, calls to those APIs will return failure and have no
	 * effect).
	 */

	result = secure_rom_call(
			PPA_SERV_HAL_LOCK_EMIF_FW,
			0, 0, 0);

	if (result != 0) {
		puts("Secure EMIF Firewall Lock failed!\n");
		debug("Return Value = %x\n", result);
	}

	return result;
}

static struct ppa_tee_load_info tee_info __aligned(ARCH_DMA_MINALIGN);

int secure_tee_install(u32 addr)
{
	struct optee_header *hdr;
	void *loadptr;
	u32 tee_file_size;
	u32 sec_mem_start = get_sec_mem_start();
	const u32 size = CONFIG_TI_SECURE_EMIF_PROTECTED_REGION_SIZE;
	u32 ret;

	/* If there is no protected region, there is no place to put the TEE */
	if (size == 0) {
		printf("Error loading TEE, no protected memory region available\n");
		return -ENOBUFS;
	}

	hdr = (struct optee_header *)map_sysmem(addr, sizeof(struct optee_header));
	/* 280 bytes = size of signature */
	tee_file_size = hdr->init_size + hdr->paged_size +
			sizeof(struct optee_header) + 280;

	if ((hdr->magic != OPTEE_MAGIC) ||
	    (hdr->version != OPTEE_VERSION) ||
	    (tee_file_size > size)) {
		printf("Error in TEE header. Check firewall and TEE sizes\n");
		unmap_sysmem(hdr);
		return CMD_RET_FAILURE;
	}

	tee_info.tee_sec_mem_start = sec_mem_start;
	tee_info.tee_sec_mem_size = size;
	tee_info.tee_jump_addr = hdr->init_load_addr_lo;
	tee_info.tee_cert_start = addr;
	tee_info.tee_cert_size = tee_file_size;
	tee_info.tee_arg0 = hdr->init_size + tee_info.tee_jump_addr;
	unmap_sysmem(hdr);
	loadptr = map_sysmem(addr, tee_file_size);

	debug("tee_info.tee_sec_mem_start= %08X\n", tee_info.tee_sec_mem_start);
	debug("tee_info.tee_sec_mem_size = %08X\n", tee_info.tee_sec_mem_size);
	debug("tee_info.tee_jump_addr = %08X\n", tee_info.tee_jump_addr);
	debug("tee_info.tee_cert_start = %08X\n", tee_info.tee_cert_start);
	debug("tee_info.tee_cert_size = %08X\n", tee_info.tee_cert_size);
	debug("tee_info.tee_arg0 = %08X\n", tee_info.tee_arg0);
	debug("tee_file_size = %d\n", tee_file_size);

#if !CONFIG_IS_ENABLED(SYS_DCACHE_OFF)
	flush_dcache_range(
		rounddown((u32)loadptr, ARCH_DMA_MINALIGN),
		roundup((u32)loadptr + tee_file_size, ARCH_DMA_MINALIGN));

	flush_dcache_range((u32)&tee_info, (u32)&tee_info +
			roundup(sizeof(tee_info), ARCH_DMA_MINALIGN));
#endif
	unmap_sysmem(loadptr);

	ret = secure_rom_call(PPA_SERV_HAL_TEE_LOAD_MASTER, 0, 0, 1, &tee_info);
	if (ret) {
		printf("TEE_LOAD_MASTER Failed\n");
		return ret;
	}
	printf("TEE_LOAD_MASTER Done\n");

#if defined(CONFIG_OMAP54XX)
	if (!is_dra72x()) {
		u32 *smc_cpu1_params;
		/* Reuse the tee_info buffer for SMC params */
		smc_cpu1_params = (u32 *)&tee_info;
		smc_cpu1_params[0] = 0;
#if !CONFIG_IS_ENABLED(SYS_DCACHE_OFF)
		flush_dcache_range((u32)smc_cpu1_params, (u32)smc_cpu1_params +
				roundup(sizeof(u32), ARCH_DMA_MINALIGN));
#endif
		ret = omap_smc_sec_cpu1(PPA_SERV_HAL_TEE_LOAD_SLAVE, 0, 0,
				smc_cpu1_params);
		if (ret) {
			printf("TEE_LOAD_SLAVE Failed\n");
			return ret;
		}
		printf("TEE_LOAD_SLAVE Done\n");
	}
#endif

	tee_loaded = 1;

	return 0;
}
