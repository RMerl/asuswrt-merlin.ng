// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2011 Sebastian Andrzej Siewior <bigeasy@linutronix.de>
 */

#include <common.h>
#include <env.h>
#include <image.h>
#include <image-android-dt.h>
#include <android_image.h>
#include <malloc.h>
#include <errno.h>
#include <asm/unaligned.h>
#include <mapmem.h>
#include <linux/libfdt.h>

#define ANDROID_IMAGE_DEFAULT_KERNEL_ADDR	0x10008000

static char andr_tmp_str[ANDR_BOOT_ARGS_SIZE + 1];

static ulong checksum(const unsigned char *buffer, ulong size)
{
	ulong sum = 0;

	for (ulong i = 0; i < size; i++)
		sum += buffer[i];
	return sum;
}

static bool is_trailer_present(ulong bootconfig_end_addr)
{
	return !strncmp((char *)(bootconfig_end_addr - BOOTCONFIG_MAGIC_SIZE),
			BOOTCONFIG_MAGIC, BOOTCONFIG_MAGIC_SIZE);
}

static ulong add_trailer(ulong bootconfig_start_addr, ulong bootconfig_size)
{
	ulong end;
	ulong sum;

	if (!bootconfig_start_addr)
		return -1;
	if (!bootconfig_size)
		return 0;

	end = bootconfig_start_addr + bootconfig_size;
	if (is_trailer_present(end))
		return 0;

	memcpy((void *)(end), &bootconfig_size, BOOTCONFIG_SIZE_SIZE);
	sum = checksum((unsigned char *)bootconfig_start_addr, bootconfig_size);
	memcpy((void *)(end + BOOTCONFIG_SIZE_SIZE), &sum,
	       BOOTCONFIG_CHECKSUM_SIZE);
	memcpy((void *)(end + BOOTCONFIG_SIZE_SIZE + BOOTCONFIG_CHECKSUM_SIZE),
	       BOOTCONFIG_MAGIC, BOOTCONFIG_MAGIC_SIZE);

	return BOOTCONFIG_TRAILER_SIZE;
}

static void android_boot_image_v3_v4_parse_hdr(const struct andr_boot_img_hdr_v3 *hdr,
					       struct andr_image_data *data)
{
	ulong end;

	data->kcmdline = hdr->cmdline;
	data->header_version = hdr->header_version;
	data->ramdisk_ptr = env_get_ulong("ramdisk_addr_r", 16, 0);

	/*
	 * The header takes a full page, the remaining components are aligned
	 * on page boundary.
	 */
	end = (ulong)hdr;
	end += ANDR_GKI_PAGE_SIZE;
	data->kernel_ptr = end;
	data->kernel_size = hdr->kernel_size;
	end += ALIGN(hdr->kernel_size, ANDR_GKI_PAGE_SIZE);
	data->ramdisk_size = hdr->ramdisk_size;
	data->boot_ramdisk_size = hdr->ramdisk_size;
	end += ALIGN(hdr->ramdisk_size, ANDR_GKI_PAGE_SIZE);

	if (hdr->header_version > 3)
		end += ALIGN(hdr->signature_size, ANDR_GKI_PAGE_SIZE);

	data->boot_img_total_size = end - (ulong)hdr;
}

static void android_vendor_boot_image_v3_v4_parse_hdr(const struct andr_vnd_boot_img_hdr
						      *hdr, struct andr_image_data *data)
{
	ulong end;

	/*
	 * The header takes a full page, the remaining components are aligned
	 * on page boundary.
	 */
	data->kcmdline_extra = hdr->cmdline;
	data->tags_addr = hdr->tags_addr;
	data->image_name = hdr->name;
	data->kernel_addr = hdr->kernel_addr;
	data->ramdisk_addr = hdr->ramdisk_addr;
	data->dtb_load_addr = hdr->dtb_addr;
	data->bootconfig_size = hdr->bootconfig_size;
	end = (ulong)hdr;
	end += hdr->page_size;
	if (hdr->vendor_ramdisk_size) {
		data->vendor_ramdisk_ptr = end;
		data->vendor_ramdisk_size = hdr->vendor_ramdisk_size;
		data->ramdisk_size += hdr->vendor_ramdisk_size;
		end += ALIGN(hdr->vendor_ramdisk_size, hdr->page_size);
	}

	data->dtb_ptr = end;
	data->dtb_size = hdr->dtb_size;

	end += ALIGN(hdr->dtb_size, hdr->page_size);
	end += ALIGN(hdr->vendor_ramdisk_table_size, hdr->page_size);
	data->bootconfig_addr = end;
	if (hdr->bootconfig_size) {
		data->bootconfig_size += add_trailer(data->bootconfig_addr,
						     data->bootconfig_size);
		data->ramdisk_size += data->bootconfig_size;
	}
	end += ALIGN(data->bootconfig_size, hdr->page_size);
	data->vendor_boot_img_total_size = end - (ulong)hdr;
}

static void android_boot_image_v0_v1_v2_parse_hdr(const struct andr_boot_img_hdr_v0 *hdr,
						  struct andr_image_data *data)
{
	ulong end;

	data->image_name = hdr->name;
	data->kcmdline = hdr->cmdline;
	data->kernel_addr = hdr->kernel_addr;
	data->ramdisk_addr = hdr->ramdisk_addr;
	data->header_version = hdr->header_version;
	data->dtb_load_addr = hdr->dtb_addr;

	end = (ulong)hdr;

	/*
	 * The header takes a full page, the remaining components are aligned
	 * on page boundary
	 */

	end += hdr->page_size;

	data->kernel_ptr = end;
	data->kernel_size = hdr->kernel_size;
	end += ALIGN(hdr->kernel_size, hdr->page_size);

	data->ramdisk_ptr = end;
	data->ramdisk_size = hdr->ramdisk_size;
	end += ALIGN(hdr->ramdisk_size, hdr->page_size);

	data->second_ptr = end;
	data->second_size = hdr->second_size;
	end += ALIGN(hdr->second_size, hdr->page_size);

	if (hdr->header_version >= 1) {
		data->recovery_dtbo_ptr = end;
		data->recovery_dtbo_size = hdr->recovery_dtbo_size;
		end += ALIGN(hdr->recovery_dtbo_size, hdr->page_size);
	}

	if (hdr->header_version >= 2) {
		data->dtb_ptr = end;
		data->dtb_size = hdr->dtb_size;
		end += ALIGN(hdr->dtb_size, hdr->page_size);
	}

	data->boot_img_total_size = end - (ulong)hdr;
}

bool android_image_get_data(const void *boot_hdr, const void *vendor_boot_hdr,
			    struct andr_image_data *data)
{
	if (!boot_hdr || !data) {
		printf("boot_hdr or data params can't be NULL\n");
		return false;
	}

	if (!is_android_boot_image_header(boot_hdr)) {
		printf("Incorrect boot image header\n");
		return false;
	}

	if (((struct andr_boot_img_hdr_v0 *)boot_hdr)->header_version > 2) {
		if (!vendor_boot_hdr) {
			printf("For boot header v3+ vendor boot image has to be provided\n");
			return false;
		}
		if (!is_android_vendor_boot_image_header(vendor_boot_hdr)) {
			printf("Incorrect vendor boot image header\n");
			return false;
		}
		android_boot_image_v3_v4_parse_hdr(boot_hdr, data);
		android_vendor_boot_image_v3_v4_parse_hdr(vendor_boot_hdr, data);
	} else {
		android_boot_image_v0_v1_v2_parse_hdr(boot_hdr, data);
	}

	return true;
}

static ulong android_image_get_kernel_addr(struct andr_image_data *img_data)
{
	/*
	 * All the Android tools that generate a boot.img use this
	 * address as the default.
	 *
	 * Even though it doesn't really make a lot of sense, and it
	 * might be valid on some platforms, we treat that adress as
	 * the default value for this field, and try to execute the
	 * kernel in place in such a case.
	 *
	 * Otherwise, we will return the actual value set by the user.
	 */
	if (img_data->kernel_addr  == ANDROID_IMAGE_DEFAULT_KERNEL_ADDR)
		return img_data->kernel_ptr;

	/*
	 * abootimg creates images where all load addresses are 0
	 * and we need to fix them.
	 */
	if (img_data->kernel_addr == 0 && img_data->ramdisk_addr == 0)
		return env_get_ulong("kernel_addr_r", 16, 0);

	return img_data->kernel_addr;
}

/**
 * android_image_get_kernel() - processes kernel part of Android boot images
 * @hdr:	Pointer to boot image header, which is at the start
 *			of the image.
 * @vendor_boot_img:	Pointer to vendor boot image header, which is at the
 *				start of the image.
 * @verify:	Checksum verification flag. Currently unimplemented.
 * @os_data:	Pointer to a ulong variable, will hold os data start
 *			address.
 * @os_len:	Pointer to a ulong variable, will hold os data length.
 *
 * This function returns the os image's start address and length. Also,
 * it appends the kernel command line to the bootargs env variable.
 *
 * Return: Zero, os start address and length on success,
 *		otherwise on failure.
 */
int android_image_get_kernel(const void *hdr,
			     const void *vendor_boot_img, int verify,
			     ulong *os_data, ulong *os_len)
{
	struct andr_image_data img_data = {0};
	u32 kernel_addr;
	const struct legacy_img_hdr *ihdr;

	if (!android_image_get_data(hdr, vendor_boot_img, &img_data))
		return -EINVAL;

	kernel_addr = android_image_get_kernel_addr(&img_data);
	ihdr = (const struct legacy_img_hdr *)img_data.kernel_ptr;

	/*
	 * Not all Android tools use the id field for signing the image with
	 * sha1 (or anything) so we don't check it. It is not obvious that the
	 * string is null terminated so we take care of this.
	 */
	strlcpy(andr_tmp_str, img_data.image_name, ANDR_BOOT_NAME_SIZE);
	andr_tmp_str[ANDR_BOOT_NAME_SIZE] = '\0';
	if (strlen(andr_tmp_str))
		printf("Android's image name: %s\n", andr_tmp_str);

	printf("Kernel load addr 0x%08x size %u KiB\n",
	       kernel_addr, DIV_ROUND_UP(img_data.kernel_size, 1024));

	int len = 0;
	if (*img_data.kcmdline) {
		printf("Kernel command line: %s\n", img_data.kcmdline);
		len += strlen(img_data.kcmdline);
	}

	if (img_data.kcmdline_extra) {
		printf("Kernel extra command line: %s\n", img_data.kcmdline_extra);
		len += strlen(img_data.kcmdline_extra);
	}

	char *bootargs = env_get("bootargs");
	if (bootargs)
		len += strlen(bootargs);

	char *newbootargs = malloc(len + 2);
	if (!newbootargs) {
		puts("Error: malloc in android_image_get_kernel failed!\n");
		return -ENOMEM;
	}
	*newbootargs = '\0';

	if (bootargs) {
		strcpy(newbootargs, bootargs);
		strcat(newbootargs, " ");
	}

	if (*img_data.kcmdline)
		strcat(newbootargs, img_data.kcmdline);

	if (img_data.kcmdline_extra) {
		strcat(newbootargs, " ");
		strcat(newbootargs, img_data.kcmdline_extra);
	}

	env_set("bootargs", newbootargs);

	if (os_data) {
		if (image_get_magic(ihdr) == IH_MAGIC) {
			*os_data = image_get_data(ihdr);
		} else {
			*os_data = img_data.kernel_ptr;
		}
	}
	if (os_len) {
		if (image_get_magic(ihdr) == IH_MAGIC)
			*os_len = image_get_data_size(ihdr);
		else
			*os_len = img_data.kernel_size;
	}
	return 0;
}

bool is_android_vendor_boot_image_header(const void *vendor_boot_img)
{
	return !memcmp(VENDOR_BOOT_MAGIC, vendor_boot_img, ANDR_VENDOR_BOOT_MAGIC_SIZE);
}

bool is_android_boot_image_header(const void *hdr)
{
	return !memcmp(ANDR_BOOT_MAGIC, hdr, ANDR_BOOT_MAGIC_SIZE);
}

ulong android_image_get_end(const struct andr_boot_img_hdr_v0 *hdr,
			    const void *vendor_boot_img)
{
	struct andr_image_data img_data;

	if (!android_image_get_data(hdr, vendor_boot_img, &img_data))
		return -EINVAL;

	if (img_data.header_version > 2)
		return 0;

	return img_data.boot_img_total_size;
}

ulong android_image_get_kload(const void *hdr,
			      const void *vendor_boot_img)
{
	struct andr_image_data img_data;

	if (!android_image_get_data(hdr, vendor_boot_img, &img_data))
		return -EINVAL;

	return android_image_get_kernel_addr(&img_data);
}

ulong android_image_get_kcomp(const void *hdr,
			      const void *vendor_boot_img)
{
	struct andr_image_data img_data;
	const void *p;

	if (!android_image_get_data(hdr, vendor_boot_img, &img_data))
		return -EINVAL;

	p = (const void *)img_data.kernel_ptr;
	if (image_get_magic((struct legacy_img_hdr *)p) == IH_MAGIC)
		return image_get_comp((struct legacy_img_hdr *)p);
	else if (get_unaligned_le32(p) == LZ4F_MAGIC)
		return IH_COMP_LZ4;
	else
		return image_decomp_type(p, sizeof(u32));
}

int android_image_get_ramdisk(const void *hdr, const void *vendor_boot_img,
			      ulong *rd_data, ulong *rd_len)
{
	struct andr_image_data img_data = {0};
	ulong ramdisk_ptr;

	if (!android_image_get_data(hdr, vendor_boot_img, &img_data))
		return -EINVAL;

	if (!img_data.ramdisk_size) {
		*rd_data = *rd_len = 0;
		return -1;
	}
	if (img_data.header_version > 2) {
		ramdisk_ptr = img_data.ramdisk_ptr;
		memcpy((void *)(ramdisk_ptr), (void *)img_data.vendor_ramdisk_ptr,
		       img_data.vendor_ramdisk_size);
		memcpy((void *)(ramdisk_ptr + img_data.vendor_ramdisk_size),
		       (void *)img_data.ramdisk_ptr,
		       img_data.boot_ramdisk_size);
		if (img_data.bootconfig_size) {
			memcpy((void *)
			       (ramdisk_ptr + img_data.vendor_ramdisk_size +
			       img_data.boot_ramdisk_size),
			       (void *)img_data.bootconfig_addr,
			       img_data.bootconfig_size);
		}
	}

	printf("RAM disk load addr 0x%08lx size %u KiB\n",
	       img_data.ramdisk_ptr, DIV_ROUND_UP(img_data.ramdisk_size, 1024));

	*rd_data = img_data.ramdisk_ptr;

	*rd_len = img_data.ramdisk_size;
	return 0;
}

int android_image_get_second(const void *hdr, ulong *second_data, ulong *second_len)
{
	struct andr_image_data img_data;

	if (!android_image_get_data(hdr, NULL, &img_data))
		return -EINVAL;

	if (img_data.header_version > 2) {
		printf("Second stage bootloader is only supported for boot image version <= 2\n");
		return -EOPNOTSUPP;
	}

	if (!img_data.second_size) {
		*second_data = *second_len = 0;
		return -1;
	}

	*second_data = img_data.second_ptr;

	printf("second address is 0x%lx\n",*second_data);

	*second_len = img_data.second_size;
	return 0;
}

/**
 * android_image_get_dtbo() - Get address and size of recovery DTBO image.
 * @hdr_addr: Boot image header address
 * @addr: If not NULL, will contain address of recovery DTBO image
 * @size: If not NULL, will contain size of recovery DTBO image
 *
 * Get the address and size of DTBO image in "Recovery DTBO" area of Android
 * Boot Image in RAM. The format of this image is Android DTBO (see
 * corresponding "DTB/DTBO Partitions" AOSP documentation for details). Once
 * the address is obtained from this function, one can use 'adtimg' U-Boot
 * command or android_dt_*() functions to extract desired DTBO blob.
 *
 * This DTBO (included in boot image) is only needed for non-A/B devices, and it
 * only can be found in recovery image. On A/B devices we can always rely on
 * "dtbo" partition. See "Including DTBO in Recovery for Non-A/B Devices" in
 * AOSP documentation for details.
 *
 * Return: true on success or false on error.
 */
bool android_image_get_dtbo(ulong hdr_addr, ulong *addr, u32 *size)
{
	const struct andr_boot_img_hdr_v0 *hdr;
	ulong dtbo_img_addr;
	bool ret = true;

	hdr = map_sysmem(hdr_addr, sizeof(*hdr));
	if (!is_android_boot_image_header(hdr)) {
		printf("Error: Boot Image header is incorrect\n");
		ret = false;
		goto exit;
	}

	if (hdr->header_version != 1 && hdr->header_version != 2) {
		printf("Error: header version must be >= 1 and <= 2 to get dtbo\n");
		ret = false;
		goto exit;
	}

	if (hdr->recovery_dtbo_size == 0) {
		printf("Error: recovery_dtbo_size is 0\n");
		ret = false;
		goto exit;
	}

	/* Calculate the address of DTB area in boot image */
	dtbo_img_addr = hdr_addr;
	dtbo_img_addr += hdr->page_size;
	dtbo_img_addr += ALIGN(hdr->kernel_size, hdr->page_size);
	dtbo_img_addr += ALIGN(hdr->ramdisk_size, hdr->page_size);
	dtbo_img_addr += ALIGN(hdr->second_size, hdr->page_size);

	if (addr)
		*addr = dtbo_img_addr;
	if (size)
		*size = hdr->recovery_dtbo_size;

exit:
	unmap_sysmem(hdr);
	return ret;
}

/**
 * android_image_get_dtb_img_addr() - Get the address of DTB area in boot image.
 * @hdr_addr: Boot image header address
 * @vhdr_addr: Vendor Boot image header address
 * @addr: Will contain the address of DTB area in boot image
 *
 * Return: true on success or false on fail.
 */
static bool android_image_get_dtb_img_addr(ulong hdr_addr, ulong vhdr_addr, ulong *addr)
{
	const struct andr_boot_img_hdr_v0 *hdr;
	const struct andr_vnd_boot_img_hdr *v_hdr;
	ulong dtb_img_addr;
	bool ret = true;

	hdr = map_sysmem(hdr_addr, sizeof(*hdr));
	if (!is_android_boot_image_header(hdr)) {
		printf("Error: Boot Image header is incorrect\n");
		ret = false;
		goto exit;
	}

	if (hdr->header_version < 2) {
		printf("Error: header_version must be >= 2 to get dtb\n");
		ret = false;
		goto exit;
	}

	if (hdr->header_version == 2) {
		if (!hdr->dtb_size) {
			printf("Error: dtb_size is 0\n");
			ret = false;
			goto exit;
		}
		/* Calculate the address of DTB area in boot image */
		dtb_img_addr = hdr_addr;
		dtb_img_addr += hdr->page_size;
		dtb_img_addr += ALIGN(hdr->kernel_size, hdr->page_size);
		dtb_img_addr += ALIGN(hdr->ramdisk_size, hdr->page_size);
		dtb_img_addr += ALIGN(hdr->second_size, hdr->page_size);
		dtb_img_addr += ALIGN(hdr->recovery_dtbo_size, hdr->page_size);

		*addr = dtb_img_addr;
	}

	if (hdr->header_version > 2) {
		v_hdr = map_sysmem(vhdr_addr, sizeof(*v_hdr));
		if (!v_hdr->dtb_size) {
			printf("Error: dtb_size is 0\n");
			ret = false;
			unmap_sysmem(v_hdr);
			goto exit;
		}
		/* Calculate the address of DTB area in boot image */
		dtb_img_addr = vhdr_addr;
		dtb_img_addr += v_hdr->page_size;
		if (v_hdr->vendor_ramdisk_size)
			dtb_img_addr += ALIGN(v_hdr->vendor_ramdisk_size, v_hdr->page_size);
		*addr = dtb_img_addr;
		unmap_sysmem(v_hdr);
		goto exit;
	}
exit:
	unmap_sysmem(hdr);
	return ret;
}

/**
 * android_image_get_dtb_by_index() - Get address and size of blob in DTB area.
 * @hdr_addr: Boot image header address
 * @vendor_boot_img: Pointer to vendor boot image header, which is at the start of the image.
 * @index: Index of desired DTB in DTB area (starting from 0)
 * @addr: If not NULL, will contain address to specified DTB
 * @size: If not NULL, will contain size of specified DTB
 *
 * Get the address and size of DTB blob by its index in DTB area of Android
 * Boot Image in RAM.
 *
 * Return: true on success or false on error.
 */
bool android_image_get_dtb_by_index(ulong hdr_addr, ulong vendor_boot_img,
				    u32 index, ulong *addr, u32 *size)
{
	struct andr_image_data img_data;
	const struct andr_boot_img_hdr_v0 *hdr;
	const struct andr_vnd_boot_img_hdr *vhdr;

	hdr = map_sysmem(hdr_addr, sizeof(*hdr));
	if (vendor_boot_img != -1)
		vhdr = map_sysmem(vendor_boot_img, sizeof(*vhdr));
	if (!android_image_get_data(hdr, vhdr, &img_data)) {
		if (vendor_boot_img != -1)
			unmap_sysmem(vhdr);
		unmap_sysmem(hdr);
		return false;
	}
	if (vendor_boot_img != -1)
		unmap_sysmem(vhdr);
	unmap_sysmem(hdr);

	ulong dtb_img_addr;	/* address of DTB part in boot image */
	u32 dtb_img_size;	/* size of DTB payload in boot image */
	ulong dtb_addr;		/* address of DTB blob with specified index  */
	u32 i;			/* index iterator */

	android_image_get_dtb_img_addr(hdr_addr, vendor_boot_img, &dtb_img_addr);
	/* Check if DTB area of boot image is in DTBO format */
	if (android_dt_check_header(dtb_img_addr)) {
		return android_dt_get_fdt_by_index(dtb_img_addr, index, addr,
						   size);
	}

	/* Find out the address of DTB with specified index in concat blobs */
	dtb_img_size = img_data.dtb_size;
	i = 0;
	dtb_addr = dtb_img_addr;
	while (dtb_addr < dtb_img_addr + dtb_img_size) {
		const struct fdt_header *fdt;
		u32 dtb_size;

		fdt = map_sysmem(dtb_addr, sizeof(*fdt));
		if (fdt_check_header(fdt) != 0) {
			unmap_sysmem(fdt);
			printf("Error: Invalid FDT header for index %u\n", i);
			return false;
		}

		dtb_size = fdt_totalsize(fdt);
		unmap_sysmem(fdt);

		if (i == index) {
			if (size)
				*size = dtb_size;
			if (addr)
				*addr = dtb_addr;
			return true;
		}

		dtb_addr += dtb_size;
		++i;
	}

	printf("Error: Index is out of bounds (%u/%u)\n", index, i);
	return false;
}

#if !defined(CONFIG_SPL_BUILD)
/**
 * android_print_contents - prints out the contents of the Android format image
 * @hdr: pointer to the Android format image header
 *
 * android_print_contents() formats a multi line Android image contents
 * description.
 * The routine prints out Android image properties
 *
 * returns:
 *     no returned results
 */
void android_print_contents(const struct andr_boot_img_hdr_v0 *hdr)
{
	if (hdr->header_version >= 3) {
		printf("Content print is not supported for boot image header version > 2");
		return;
	}
	const char * const p = IMAGE_INDENT_STRING;
	/* os_version = ver << 11 | lvl */
	u32 os_ver = hdr->os_version >> 11;
	u32 os_lvl = hdr->os_version & ((1U << 11) - 1);

	printf("%skernel size:          %x\n", p, hdr->kernel_size);
	printf("%skernel address:       %x\n", p, hdr->kernel_addr);
	printf("%sramdisk size:         %x\n", p, hdr->ramdisk_size);
	printf("%sramdisk address:      %x\n", p, hdr->ramdisk_addr);
	printf("%ssecond size:          %x\n", p, hdr->second_size);
	printf("%ssecond address:       %x\n", p, hdr->second_addr);
	printf("%stags address:         %x\n", p, hdr->tags_addr);
	printf("%spage size:            %x\n", p, hdr->page_size);
	/* ver = A << 14 | B << 7 | C         (7 bits for each of A, B, C)
	 * lvl = ((Y - 2000) & 127) << 4 | M  (7 bits for Y, 4 bits for M) */
	printf("%sos_version:           %x (ver: %u.%u.%u, level: %u.%u)\n",
	       p, hdr->os_version,
	       (os_ver >> 7) & 0x7F, (os_ver >> 14) & 0x7F, os_ver & 0x7F,
	       (os_lvl >> 4) + 2000, os_lvl & 0x0F);
	printf("%sname:                 %s\n", p, hdr->name);
	printf("%scmdline:              %s\n", p, hdr->cmdline);
	printf("%sheader_version:       %d\n", p, hdr->header_version);

	if (hdr->header_version >= 1) {
		printf("%srecovery dtbo size:   %x\n", p,
		       hdr->recovery_dtbo_size);
		printf("%srecovery dtbo offset: %llx\n", p,
		       hdr->recovery_dtbo_offset);
		printf("%sheader size:          %x\n", p,
		       hdr->header_size);
	}

	if (hdr->header_version == 2) {
		printf("%sdtb size:             %x\n", p, hdr->dtb_size);
		printf("%sdtb addr:             %llx\n", p, hdr->dtb_addr);
	}
}

/**
 * android_image_print_dtb_info - Print info for one DTB blob in DTB area.
 * @fdt: DTB header
 * @index: Number of DTB blob in DTB area.
 *
 * Return: true on success or false on error.
 */
static bool android_image_print_dtb_info(const struct fdt_header *fdt,
					 u32 index)
{
	int root_node_off;
	u32 fdt_size;
	const char *model;
	const char *compatible;

	root_node_off = fdt_path_offset(fdt, "/");
	if (root_node_off < 0) {
		printf("Error: Root node not found\n");
		return false;
	}

	fdt_size = fdt_totalsize(fdt);
	compatible = fdt_getprop(fdt, root_node_off, "compatible",
				 NULL);
	model = fdt_getprop(fdt, root_node_off, "model", NULL);

	printf(" - DTB #%u:\n", index);
	printf("           (DTB)size = %d\n", fdt_size);
	printf("          (DTB)model = %s\n", model ? model : "(unknown)");
	printf("     (DTB)compatible = %s\n",
	       compatible ? compatible : "(unknown)");

	return true;
}

/**
 * android_image_print_dtb_contents() - Print info for DTB blobs in DTB area.
 * @hdr_addr: Boot image header address
 *
 * DTB payload in Android Boot Image v2+ can be in one of following formats:
 *   1. Concatenated DTB blobs
 *   2. Android DTBO format (see CONFIG_CMD_ADTIMG for details)
 *
 * This function does next:
 *   1. Prints out the format used in DTB area
 *   2. Iterates over all DTB blobs in DTB area and prints out the info for
 *      each blob.
 *
 * Return: true on success or false on error.
 */
bool android_image_print_dtb_contents(ulong hdr_addr)
{
	const struct andr_boot_img_hdr_v0 *hdr;
	bool res;
	ulong dtb_img_addr;	/* address of DTB part in boot image */
	u32 dtb_img_size;	/* size of DTB payload in boot image */
	ulong dtb_addr;		/* address of DTB blob with specified index  */
	u32 i;			/* index iterator */

	res = android_image_get_dtb_img_addr(hdr_addr, 0, &dtb_img_addr);
	if (!res)
		return false;

	/* Check if DTB area of boot image is in DTBO format */
	if (android_dt_check_header(dtb_img_addr)) {
		printf("## DTB area contents (DTBO format):\n");
		android_dt_print_contents(dtb_img_addr);
		return true;
	}

	printf("## DTB area contents (concat format):\n");

	/* Iterate over concatenated DTB blobs */
	hdr = map_sysmem(hdr_addr, sizeof(*hdr));
	dtb_img_size = hdr->dtb_size;
	unmap_sysmem(hdr);
	i = 0;
	dtb_addr = dtb_img_addr;
	while (dtb_addr < dtb_img_addr + dtb_img_size) {
		const struct fdt_header *fdt;
		u32 dtb_size;

		fdt = map_sysmem(dtb_addr, sizeof(*fdt));
		if (fdt_check_header(fdt) != 0) {
			unmap_sysmem(fdt);
			printf("Error: Invalid FDT header for index %u\n", i);
			return false;
		}

		res = android_image_print_dtb_info(fdt, i);
		if (!res) {
			unmap_sysmem(fdt);
			return false;
		}

		dtb_size = fdt_totalsize(fdt);
		unmap_sysmem(fdt);
		dtb_addr += dtb_size;
		++i;
	}

	return true;
}
#endif
