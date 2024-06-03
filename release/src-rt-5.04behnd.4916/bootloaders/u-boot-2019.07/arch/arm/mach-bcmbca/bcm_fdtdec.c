#include <common.h>
#include <fdt.h>
#include <malloc.h>
#include <image.h>
#include <asm/sections.h>

#if defined(CONFIG_ARM64)

#if defined(CONFIG_BCMBCA_IKOS) && !defined(CONFIG_SPL) && defined(CONFIG_BCM6888)
void *board_fdt_blob_setup(void)
{
	/* DTB is backdoor loaded */
	return (void *)0x03000000;
}
#else
void  __attribute__((section(".data"))) * g_fdt_ptr = NULL;
void *board_fdt_blob_setup(void)
{
	struct fdt_header * fdt_blob = (struct fdt_header *)&_end;
	if(  uimage_to_cpu(fdt_blob->magic) == FDT_MAGIC )
	{
		g_fdt_ptr = malloc(  uimage_to_cpu(fdt_blob->totalsize) );
		if( g_fdt_ptr )
			memcpy( (char*)g_fdt_ptr, (char*)fdt_blob, uimage_to_cpu(fdt_blob->totalsize) );
	}

	return g_fdt_ptr;
}
#endif
#endif

