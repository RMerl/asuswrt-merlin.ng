/*
 * Kexec bzImage loader
 *
 * Copyright (C) 2014 Red Hat Inc.
 * Authors:
 *      Vivek Goyal <vgoyal@redhat.com>
 *
 * This source code is licensed under the GNU General Public License,
 * Version 2.  See the file COPYING for more details.
 */

#define pr_fmt(fmt)	"kexec-bzImage64: " fmt

#include <linux/string.h>
#include <linux/printk.h>
#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/kexec.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/efi.h>
#include <linux/verify_pefile.h>
#include <keys/system_keyring.h>

#include <asm/bootparam.h>
#include <asm/setup.h>
#include <asm/crash.h>
#include <asm/efi.h>
#include <asm/kexec-bzimage64.h>

#define MAX_ELFCOREHDR_STR_LEN	30	/* elfcorehdr=0x<64bit-value> */

/*
 * Defines lowest physical address for various segments. Not sure where
 * exactly these limits came from. Current bzimage64 loader in kexec-tools
 * uses these so I am retaining it. It can be changed over time as we gain
 * more insight.
 */
#define MIN_PURGATORY_ADDR	0x3000
#define MIN_BOOTPARAM_ADDR	0x3000
#define MIN_KERNEL_LOAD_ADDR	0x100000
#define MIN_INITRD_LOAD_ADDR	0x1000000

/*
 * This is a place holder for all boot loader specific data structure which
 * gets allocated in one call but gets freed much later during cleanup
 * time. Right now there is only one field but it can grow as need be.
 */
struct bzimage64_data {
	/*
	 * Temporary buffer to hold bootparams buffer. This should be
	 * freed once the bootparam segment has been loaded.
	 */
	void *bootparams_buf;
};

static int setup_initrd(struct boot_params *params,
		unsigned long initrd_load_addr, unsigned long initrd_len)
{
	params->hdr.ramdisk_image = initrd_load_addr & 0xffffffffUL;
	params->hdr.ramdisk_size = initrd_len & 0xffffffffUL;

	params->ext_ramdisk_image = initrd_load_addr >> 32;
	params->ext_ramdisk_size = initrd_len >> 32;

	return 0;
}

static int setup_cmdline(struct kimage *image, struct boot_params *params,
			 unsigned long bootparams_load_addr,
			 unsigned long cmdline_offset, char *cmdline,
			 unsigned long cmdline_len)
{
	char *cmdline_ptr = ((char *)params) + cmdline_offset;
	unsigned long cmdline_ptr_phys, len;
	uint32_t cmdline_low_32, cmdline_ext_32;

	memcpy(cmdline_ptr, cmdline, cmdline_len);
	if (image->type == KEXEC_TYPE_CRASH) {
		len = sprintf(cmdline_ptr + cmdline_len - 1,
			" elfcorehdr=0x%lx", image->arch.elf_load_addr);
		cmdline_len += len;
	}
	cmdline_ptr[cmdline_len - 1] = '\0';

	pr_debug("Final command line is: %s\n", cmdline_ptr);
	cmdline_ptr_phys = bootparams_load_addr + cmdline_offset;
	cmdline_low_32 = cmdline_ptr_phys & 0xffffffffUL;
	cmdline_ext_32 = cmdline_ptr_phys >> 32;

	params->hdr.cmd_line_ptr = cmdline_low_32;
	if (cmdline_ext_32)
		params->ext_cmd_line_ptr = cmdline_ext_32;

	return 0;
}

static int setup_e820_entries(struct boot_params *params)
{
	unsigned int nr_e820_entries;

	nr_e820_entries = e820_saved.nr_map;

	/* TODO: Pass entries more than E820MAX in bootparams setup data */
	if (nr_e820_entries > E820MAX)
		nr_e820_entries = E820MAX;

	params->e820_entries = nr_e820_entries;
	memcpy(&params->e820_map, &e820_saved.map,
	       nr_e820_entries * sizeof(struct e820entry));

	return 0;
}

#ifdef CONFIG_EFI
static int setup_efi_info_memmap(struct boot_params *params,
				  unsigned long params_load_addr,
				  unsigned int efi_map_offset,
				  unsigned int efi_map_sz)
{
	void *efi_map = (void *)params + efi_map_offset;
	unsigned long efi_map_phys_addr = params_load_addr + efi_map_offset;
	struct efi_info *ei = &params->efi_info;

	if (!efi_map_sz)
		return 0;

	efi_runtime_map_copy(efi_map, efi_map_sz);

	ei->efi_memmap = efi_map_phys_addr & 0xffffffff;
	ei->efi_memmap_hi = efi_map_phys_addr >> 32;
	ei->efi_memmap_size = efi_map_sz;

	return 0;
}

static int
prepare_add_efi_setup_data(struct boot_params *params,
		       unsigned long params_load_addr,
		       unsigned int efi_setup_data_offset)
{
	unsigned long setup_data_phys;
	struct setup_data *sd = (void *)params + efi_setup_data_offset;
	struct efi_setup_data *esd = (void *)sd + sizeof(struct setup_data);

	esd->fw_vendor = efi.fw_vendor;
	esd->runtime = efi.runtime;
	esd->tables = efi.config_table;
	esd->smbios = efi.smbios;

	sd->type = SETUP_EFI;
	sd->len = sizeof(struct efi_setup_data);

	/* Add setup data */
	setup_data_phys = params_load_addr + efi_setup_data_offset;
	sd->next = params->hdr.setup_data;
	params->hdr.setup_data = setup_data_phys;

	return 0;
}

static int
setup_efi_state(struct boot_params *params, unsigned long params_load_addr,
		unsigned int efi_map_offset, unsigned int efi_map_sz,
		unsigned int efi_setup_data_offset)
{
	struct efi_info *current_ei = &boot_params.efi_info;
	struct efi_info *ei = &params->efi_info;

	if (!current_ei->efi_memmap_size)
		return 0;

	/*
	 * If 1:1 mapping is not enabled, second kernel can not setup EFI
	 * and use EFI run time services. User space will have to pass
	 * acpi_rsdp=<addr> on kernel command line to make second kernel boot
	 * without efi.
	 */
	if (efi_enabled(EFI_OLD_MEMMAP))
		return 0;

	ei->efi_loader_signature = current_ei->efi_loader_signature;
	ei->efi_systab = current_ei->efi_systab;
	ei->efi_systab_hi = current_ei->efi_systab_hi;

	ei->efi_memdesc_version = current_ei->efi_memdesc_version;
	ei->efi_memdesc_size = efi_get_runtime_map_desc_size();

	setup_efi_info_memmap(params, params_load_addr, efi_map_offset,
			      efi_map_sz);
	prepare_add_efi_setup_data(params, params_load_addr,
				   efi_setup_data_offset);
	return 0;
}
#endif /* CONFIG_EFI */

static int
setup_boot_parameters(struct kimage *image, struct boot_params *params,
		      unsigned long params_load_addr,
		      unsigned int efi_map_offset, unsigned int efi_map_sz,
		      unsigned int efi_setup_data_offset)
{
	unsigned int nr_e820_entries;
	unsigned long long mem_k, start, end;
	int i, ret = 0;

	/* Get subarch from existing bootparams */
	params->hdr.hardware_subarch = boot_params.hdr.hardware_subarch;

	/* Copying screen_info will do? */
	memcpy(&params->screen_info, &boot_params.screen_info,
				sizeof(struct screen_info));

	/* Fill in memsize later */
	params->screen_info.ext_mem_k = 0;
	params->alt_mem_k = 0;

	/* Default APM info */
	memset(&params->apm_bios_info, 0, sizeof(params->apm_bios_info));

	/* Default drive info */
	memset(&params->hd0_info, 0, sizeof(params->hd0_info));
	memset(&params->hd1_info, 0, sizeof(params->hd1_info));

	/* Default sysdesc table */
	params->sys_desc_table.length = 0;

	if (image->type == KEXEC_TYPE_CRASH) {
		ret = crash_setup_memmap_entries(image, params);
		if (ret)
			return ret;
	} else
		setup_e820_entries(params);

	nr_e820_entries = params->e820_entries;

	for (i = 0; i < nr_e820_entries; i++) {
		if (params->e820_map[i].type != E820_RAM)
			continue;
		start = params->e820_map[i].addr;
		end = params->e820_map[i].addr + params->e820_map[i].size - 1;

		if ((start <= 0x100000) && end > 0x100000) {
			mem_k = (end >> 10) - (0x100000 >> 10);
			params->screen_info.ext_mem_k = mem_k;
			params->alt_mem_k = mem_k;
			if (mem_k > 0xfc00)
				params->screen_info.ext_mem_k = 0xfc00; /* 64M*/
			if (mem_k > 0xffffffff)
				params->alt_mem_k = 0xffffffff;
		}
	}

#ifdef CONFIG_EFI
	/* Setup EFI state */
	setup_efi_state(params, params_load_addr, efi_map_offset, efi_map_sz,
			efi_setup_data_offset);
#endif

	/* Setup EDD info */
	memcpy(params->eddbuf, boot_params.eddbuf,
				EDDMAXNR * sizeof(struct edd_info));
	params->eddbuf_entries = boot_params.eddbuf_entries;

	memcpy(params->edd_mbr_sig_buffer, boot_params.edd_mbr_sig_buffer,
	       EDD_MBR_SIG_MAX * sizeof(unsigned int));

	return ret;
}

static int bzImage64_probe(const char *buf, unsigned long len)
{
	int ret = -ENOEXEC;
	struct setup_header *header;

	/* kernel should be atleast two sectors long */
	if (len < 2 * 512) {
		pr_err("File is too short to be a bzImage\n");
		return ret;
	}

	header = (struct setup_header *)(buf + offsetof(struct boot_params, hdr));
	if (memcmp((char *)&header->header, "HdrS", 4) != 0) {
		pr_err("Not a bzImage\n");
		return ret;
	}

	if (header->boot_flag != 0xAA55) {
		pr_err("No x86 boot sector present\n");
		return ret;
	}

	if (header->version < 0x020C) {
		pr_err("Must be at least protocol version 2.12\n");
		return ret;
	}

	if (!(header->loadflags & LOADED_HIGH)) {
		pr_err("zImage not a bzImage\n");
		return ret;
	}

	if (!(header->xloadflags & XLF_KERNEL_64)) {
		pr_err("Not a bzImage64. XLF_KERNEL_64 is not set.\n");
		return ret;
	}

	if (!(header->xloadflags & XLF_CAN_BE_LOADED_ABOVE_4G)) {
		pr_err("XLF_CAN_BE_LOADED_ABOVE_4G is not set.\n");
		return ret;
	}

	/*
	 * Can't handle 32bit EFI as it does not allow loading kernel
	 * above 4G. This should be handled by 32bit bzImage loader
	 */
	if (efi_enabled(EFI_RUNTIME_SERVICES) && !efi_enabled(EFI_64BIT)) {
		pr_debug("EFI is 32 bit. Can't load kernel above 4G.\n");
		return ret;
	}

	/* I've got a bzImage */
	pr_debug("It's a relocatable bzImage64\n");
	ret = 0;

	return ret;
}

static void *bzImage64_load(struct kimage *image, char *kernel,
			    unsigned long kernel_len, char *initrd,
			    unsigned long initrd_len, char *cmdline,
			    unsigned long cmdline_len)
{

	struct setup_header *header;
	int setup_sects, kern16_size, ret = 0;
	unsigned long setup_header_size, params_cmdline_sz, params_misc_sz;
	struct boot_params *params;
	unsigned long bootparam_load_addr, kernel_load_addr, initrd_load_addr;
	unsigned long purgatory_load_addr;
	unsigned long kernel_bufsz, kernel_memsz, kernel_align;
	char *kernel_buf;
	struct bzimage64_data *ldata;
	struct kexec_entry64_regs regs64;
	void *stack;
	unsigned int setup_hdr_offset = offsetof(struct boot_params, hdr);
	unsigned int efi_map_offset, efi_map_sz, efi_setup_data_offset;

	header = (struct setup_header *)(kernel + setup_hdr_offset);
	setup_sects = header->setup_sects;
	if (setup_sects == 0)
		setup_sects = 4;

	kern16_size = (setup_sects + 1) * 512;
	if (kernel_len < kern16_size) {
		pr_err("bzImage truncated\n");
		return ERR_PTR(-ENOEXEC);
	}

	if (cmdline_len > header->cmdline_size) {
		pr_err("Kernel command line too long\n");
		return ERR_PTR(-EINVAL);
	}

	/*
	 * In case of crash dump, we will append elfcorehdr=<addr> to
	 * command line. Make sure it does not overflow
	 */
	if (cmdline_len + MAX_ELFCOREHDR_STR_LEN > header->cmdline_size) {
		pr_debug("Appending elfcorehdr=<addr> to command line exceeds maximum allowed length\n");
		return ERR_PTR(-EINVAL);
	}

	/* Allocate and load backup region */
	if (image->type == KEXEC_TYPE_CRASH) {
		ret = crash_load_segments(image);
		if (ret)
			return ERR_PTR(ret);
	}

	/*
	 * Load purgatory. For 64bit entry point, purgatory  code can be
	 * anywhere.
	 */
	ret = kexec_load_purgatory(image, MIN_PURGATORY_ADDR, ULONG_MAX, 1,
				   &purgatory_load_addr);
	if (ret) {
		pr_err("Loading purgatory failed\n");
		return ERR_PTR(ret);
	}

	pr_debug("Loaded purgatory at 0x%lx\n", purgatory_load_addr);


	/*
	 * Load Bootparams and cmdline and space for efi stuff.
	 *
	 * Allocate memory together for multiple data structures so
	 * that they all can go in single area/segment and we don't
	 * have to create separate segment for each. Keeps things
	 * little bit simple
	 */
	efi_map_sz = efi_get_runtime_map_size();
	efi_map_sz = ALIGN(efi_map_sz, 16);
	params_cmdline_sz = sizeof(struct boot_params) + cmdline_len +
				MAX_ELFCOREHDR_STR_LEN;
	params_cmdline_sz = ALIGN(params_cmdline_sz, 16);
	params_misc_sz = params_cmdline_sz + efi_map_sz +
				sizeof(struct setup_data) +
				sizeof(struct efi_setup_data);

	params = kzalloc(params_misc_sz, GFP_KERNEL);
	if (!params)
		return ERR_PTR(-ENOMEM);
	efi_map_offset = params_cmdline_sz;
	efi_setup_data_offset = efi_map_offset + efi_map_sz;

	/* Copy setup header onto bootparams. Documentation/x86/boot.txt */
	setup_header_size = 0x0202 + kernel[0x0201] - setup_hdr_offset;

	/* Is there a limit on setup header size? */
	memcpy(&params->hdr, (kernel + setup_hdr_offset), setup_header_size);

	ret = kexec_add_buffer(image, (char *)params, params_misc_sz,
			       params_misc_sz, 16, MIN_BOOTPARAM_ADDR,
			       ULONG_MAX, 1, &bootparam_load_addr);
	if (ret)
		goto out_free_params;
	pr_debug("Loaded boot_param, command line and misc at 0x%lx bufsz=0x%lx memsz=0x%lx\n",
		 bootparam_load_addr, params_misc_sz, params_misc_sz);

	/* Load kernel */
	kernel_buf = kernel + kern16_size;
	kernel_bufsz =  kernel_len - kern16_size;
	kernel_memsz = PAGE_ALIGN(header->init_size);
	kernel_align = header->kernel_alignment;

	ret = kexec_add_buffer(image, kernel_buf,
			       kernel_bufsz, kernel_memsz, kernel_align,
			       MIN_KERNEL_LOAD_ADDR, ULONG_MAX, 1,
			       &kernel_load_addr);
	if (ret)
		goto out_free_params;

	pr_debug("Loaded 64bit kernel at 0x%lx bufsz=0x%lx memsz=0x%lx\n",
		 kernel_load_addr, kernel_memsz, kernel_memsz);

	/* Load initrd high */
	if (initrd) {
		ret = kexec_add_buffer(image, initrd, initrd_len, initrd_len,
				       PAGE_SIZE, MIN_INITRD_LOAD_ADDR,
				       ULONG_MAX, 1, &initrd_load_addr);
		if (ret)
			goto out_free_params;

		pr_debug("Loaded initrd at 0x%lx bufsz=0x%lx memsz=0x%lx\n",
				initrd_load_addr, initrd_len, initrd_len);

		setup_initrd(params, initrd_load_addr, initrd_len);
	}

	setup_cmdline(image, params, bootparam_load_addr,
		      sizeof(struct boot_params), cmdline, cmdline_len);

	/* bootloader info. Do we need a separate ID for kexec kernel loader? */
	params->hdr.type_of_loader = 0x0D << 4;
	params->hdr.loadflags = 0;

	/* Setup purgatory regs for entry */
	ret = kexec_purgatory_get_set_symbol(image, "entry64_regs", &regs64,
					     sizeof(regs64), 1);
	if (ret)
		goto out_free_params;

	regs64.rbx = 0; /* Bootstrap Processor */
	regs64.rsi = bootparam_load_addr;
	regs64.rip = kernel_load_addr + 0x200;
	stack = kexec_purgatory_get_symbol_addr(image, "stack_end");
	if (IS_ERR(stack)) {
		pr_err("Could not find address of symbol stack_end\n");
		ret = -EINVAL;
		goto out_free_params;
	}

	regs64.rsp = (unsigned long)stack;
	ret = kexec_purgatory_get_set_symbol(image, "entry64_regs", &regs64,
					     sizeof(regs64), 0);
	if (ret)
		goto out_free_params;

	ret = setup_boot_parameters(image, params, bootparam_load_addr,
				    efi_map_offset, efi_map_sz,
				    efi_setup_data_offset);
	if (ret)
		goto out_free_params;

	/* Allocate loader specific data */
	ldata = kzalloc(sizeof(struct bzimage64_data), GFP_KERNEL);
	if (!ldata) {
		ret = -ENOMEM;
		goto out_free_params;
	}

	/*
	 * Store pointer to params so that it could be freed after loading
	 * params segment has been loaded and contents have been copied
	 * somewhere else.
	 */
	ldata->bootparams_buf = params;
	return ldata;

out_free_params:
	kfree(params);
	return ERR_PTR(ret);
}

/* This cleanup function is called after various segments have been loaded */
static int bzImage64_cleanup(void *loader_data)
{
	struct bzimage64_data *ldata = loader_data;

	if (!ldata)
		return 0;

	kfree(ldata->bootparams_buf);
	ldata->bootparams_buf = NULL;

	return 0;
}

#ifdef CONFIG_KEXEC_BZIMAGE_VERIFY_SIG
static int bzImage64_verify_sig(const char *kernel, unsigned long kernel_len)
{
	bool trusted;
	int ret;

	ret = verify_pefile_signature(kernel, kernel_len,
				      system_trusted_keyring, &trusted);
	if (ret < 0)
		return ret;
	if (!trusted)
		return -EKEYREJECTED;
	return 0;
}
#endif

struct kexec_file_ops kexec_bzImage64_ops = {
	.probe = bzImage64_probe,
	.load = bzImage64_load,
	.cleanup = bzImage64_cleanup,
#ifdef CONFIG_KEXEC_BZIMAGE_VERIFY_SIG
	.verify_sig = bzImage64_verify_sig,
#endif
};
