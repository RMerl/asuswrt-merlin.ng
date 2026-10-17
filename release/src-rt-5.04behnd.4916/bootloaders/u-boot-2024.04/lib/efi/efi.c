// SPDX-License-Identifier: GPL-2.0+
/*
 * Functions shared by the app and stub
 *
 * Copyright (c) 2015 Google, Inc
 *
 * EFI information obtained here:
 * http://wiki.phoenix.com/wiki/index.php/EFI_BOOT_SERVICES
 *
 * Common EFI functions
 */

#include <debug_uart.h>
#include <errno.h>
#include <malloc.h>
#include <linux/err.h>
#include <linux/types.h>
#include <efi.h>
#include <efi_api.h>

static struct efi_priv *global_priv;

struct efi_priv *efi_get_priv(void)
{
	return global_priv;
}

void efi_set_priv(struct efi_priv *priv)
{
	global_priv = priv;
}

struct efi_system_table *efi_get_sys_table(void)
{
	return global_priv->sys_table;
}

struct efi_boot_services *efi_get_boot(void)
{
	return global_priv->boot;
}

unsigned long efi_get_ram_base(void)
{
	return global_priv->ram_base;
}

/*
 * Global declaration of gd.
 *
 * As we write to it before relocation we have to make sure it is not put into
 * a .bss section which may overlap a .rela section. Initialization forces it
 * into a .data section which cannot overlap any .rela section.
 */
struct global_data *global_data_ptr = (struct global_data *)~0;

/*
 * Unfortunately we cannot access any code outside what is built especially
 * for the stub. lib/string.c is already being built for the U-Boot payload
 * so it uses the wrong compiler flags. Add our own memset() here.
 */
static void efi_memset(void *ptr, int ch, int size)
{
	char *dest = ptr;

	while (size-- > 0)
		*dest++ = ch;
}

/*
 * Since the EFI stub cannot access most of the U-Boot code, add our own
 * simple console output functions here. The EFI app will not use these since
 * it can use the normal console.
 */
void efi_putc(struct efi_priv *priv, const char ch)
{
	struct efi_simple_text_output_protocol *con = priv->sys_table->con_out;
	uint16_t ucode[2];

	ucode[0] = ch;
	ucode[1] = '\0';
	con->output_string(con, ucode);
}

void efi_puts(struct efi_priv *priv, const char *str)
{
	while (*str)
		efi_putc(priv, *str++);
}

int efi_init(struct efi_priv *priv, const char *banner, efi_handle_t image,
	     struct efi_system_table *sys_table)
{
	efi_guid_t loaded_image_guid = EFI_LOADED_IMAGE_PROTOCOL_GUID;
	struct efi_boot_services *boot = sys_table->boottime;
	struct efi_loaded_image *loaded_image;
	int ret;

	efi_memset(priv, '\0', sizeof(*priv));
	priv->sys_table = sys_table;
	priv->boot = sys_table->boottime;
	priv->parent_image = image;
	priv->run = sys_table->runtime;

	efi_puts(priv, "U-Boot EFI ");
	efi_puts(priv, banner);
	efi_putc(priv, ' ');

	ret = boot->open_protocol(priv->parent_image, &loaded_image_guid,
				  (void **)&loaded_image, priv->parent_image,
				  NULL, EFI_OPEN_PROTOCOL_GET_PROTOCOL);
	if (ret) {
		efi_puts(priv, "Failed to get loaded image protocol\n");
		return ret;
	}
	priv->image_data_type = loaded_image->image_data_type;

	return 0;
}

void *efi_malloc(struct efi_priv *priv, int size, efi_status_t *retp)
{
	struct efi_boot_services *boot = priv->boot;
	void *buf = NULL;

	*retp = boot->allocate_pool(priv->image_data_type, size, &buf);

	return buf;
}

void efi_free(struct efi_priv *priv, void *ptr)
{
	struct efi_boot_services *boot = priv->boot;

	boot->free_pool(ptr);
}

int efi_store_memory_map(struct efi_priv *priv)
{
	struct efi_boot_services *boot = priv->sys_table->boottime;
	efi_uintn_t size, desc_size;
	efi_status_t ret;

	/* Get the memory map so we can switch off EFI */
	size = 0;
	ret = boot->get_memory_map(&size, NULL, &priv->memmap_key,
				   &priv->memmap_desc_size,
				   &priv->memmap_version);
	if (ret != EFI_BUFFER_TOO_SMALL) {
		/*
		 * Note this function avoids using printf() since it is not
		 * available in the stub
		 */
		printhex2(EFI_BITS_PER_LONG);
		putc(' ');
		printhex2(ret);
		puts(" No memory map\n");
		return ret;
	}
	/*
	 * Since doing a malloc() may change the memory map and also we want to
	 * be able to read the memory map in efi_call_exit_boot_services()
	 * below, after more changes have happened
	 */
	priv->memmap_alloc = size + 1024;
	priv->memmap_size = priv->memmap_alloc;
	priv->memmap_desc = efi_malloc(priv, size, &ret);
	if (!priv->memmap_desc) {
		printhex2(ret);
		puts(" No memory for memory descriptor\n");
		return ret;
	}

	ret = boot->get_memory_map(&priv->memmap_size, priv->memmap_desc,
				   &priv->memmap_key, &desc_size,
				   &priv->memmap_version);
	if (ret) {
		printhex2(ret);
		puts(" Can't get memory map\n");
		return ret;
	}

	return 0;
}

int efi_call_exit_boot_services(void)
{
	struct efi_priv *priv = efi_get_priv();
	const struct efi_boot_services *boot = priv->boot;
	efi_uintn_t size;
	u32 version;
	efi_status_t ret;

	size = priv->memmap_alloc;
	ret = boot->get_memory_map(&size, priv->memmap_desc,
				   &priv->memmap_key,
				   &priv->memmap_desc_size, &version);
	if (ret) {
		printhex2(ret);
		puts(" Can't get memory map\n");
		return ret;
	}
	ret = boot->exit_boot_services(priv->parent_image, priv->memmap_key);
	if (ret)
		return ret;

	return 0;
}
