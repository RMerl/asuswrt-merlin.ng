// SPDX-License-Identifier: GPL-2.0+

#include <asm/global_data.h>
#include <asm/arch/sys_proto.h>
#include <asm/mach-imx/boot_mode.h>

DECLARE_GLOBAL_DATA_PTR;

u32 rom_api_download_image(u8 *dest, u32 offset, u32 size)
{
	u32 xor = (uintptr_t)dest ^ offset ^ size;
	volatile gd_t *sgd = gd;
	u32 ret;

	ret = g_rom_api->download_image(dest, offset, size, xor);
	set_gd(sgd);

	return ret;
}

u32 rom_api_query_boot_infor(u32 info_type, u32 *info)
{
	u32 xor = info_type ^ (uintptr_t)info;
	volatile gd_t *sgd = gd;
	u32 ret;

	ret = g_rom_api->query_boot_infor(info_type, info, xor);
	set_gd(sgd);

	return ret;
}

extern struct rom_api *g_rom_api;

enum boot_device get_boot_device(void)
{
	volatile gd_t *pgd = gd;
	int ret;
	u32 boot;
	u16 boot_type;
	u8 boot_instance;
	enum boot_device boot_dev = SD1_BOOT;

	ret = g_rom_api->query_boot_infor(QUERY_BT_DEV, &boot,
					  ((uintptr_t)&boot) ^ QUERY_BT_DEV);
	set_gd(pgd);

	if (ret != ROM_API_OKAY) {
		puts("ROMAPI: failure at query_boot_info\n");
		return -1;
	}

	boot_type = boot >> 16;
	boot_instance = (boot >> 8) & 0xff;

	switch (boot_type) {
	case BT_DEV_TYPE_SD:
		boot_dev = boot_instance + SD1_BOOT;
		break;
	case BT_DEV_TYPE_MMC:
		boot_dev = boot_instance + MMC1_BOOT;
		break;
	case BT_DEV_TYPE_NAND:
		boot_dev = NAND_BOOT;
		break;
	case BT_DEV_TYPE_FLEXSPINOR:
		boot_dev = QSPI_BOOT;
		break;
	case BT_DEV_TYPE_SPI_NOR:
		boot_dev = SPI_NOR_BOOT;
		break;
	case BT_DEV_TYPE_USB:
		if (!is_imx8ulp() && !is_imx9())
			boot_instance = 0;
		boot_dev = boot_instance + USB_BOOT;
		break;
	default:
		break;
	}

	return boot_dev;
}
