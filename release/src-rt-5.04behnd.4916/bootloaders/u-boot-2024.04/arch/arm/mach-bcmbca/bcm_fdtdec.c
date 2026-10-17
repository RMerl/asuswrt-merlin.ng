#include <asm/global_data.h>
#include <malloc.h>
#include <image.h>
#include <asm/sections.h>

DECLARE_GLOBAL_DATA_PTR;

#if defined(CONFIG_BCMBCA_IKOS) && !defined(CONFIG_SPL) && defined(CONFIG_BCM6888)
void *board_fdt_blob_setup(int *ret)
{
	/* DTB is backdoor loaded */
	*ret = 0;
	return (void *)0x03000000;
}
#else
/*
 * only need to provide override for TPL as we need to increase the dtb size
 * for secure boot dtb nodes. The standard impl in fdtdec_setup is good for
 * SPL and u-boot.
 */
#if defined(CONFIG_TPL_BUILD)
void  __attribute__((section(".data"))) * g_fdt_ptr = NULL;
/* reserved extra space for TPL fdt for secure exported items */
#define FDT_EXTRA_SPACE        0x2000
#endif

void *board_fdt_blob_setup(int *ret)
{
#if defined(CONFIG_TPL_BUILD)
	int fdt_size = 0;
	const struct fdt_header *hdr = (const struct fdt_header *)gd->fdt_blob;

	if (fdt_magic(hdr) == FDT_MAGIC) {
		fdt_size = fdt_totalsize(hdr) + FDT_EXTRA_SPACE;
		g_fdt_ptr = malloc(fdt_size);
		if (g_fdt_ptr) {
			if (fdt_open_into(gd->fdt_blob, g_fdt_ptr, fdt_size)) {
				free(g_fdt_ptr);
				g_fdt_ptr = NULL;
			}
		}
	}

	/* in case of oom or error occurs, restore the original fdt */
	if (!g_fdt_ptr)
		g_fdt_ptr = (void *)gd->fdt_blob;

	*ret = 0;
	return g_fdt_ptr;
#else
	*ret = -EEXIST;
	return (void *)gd->fdt_blob;
#endif
}
#endif
