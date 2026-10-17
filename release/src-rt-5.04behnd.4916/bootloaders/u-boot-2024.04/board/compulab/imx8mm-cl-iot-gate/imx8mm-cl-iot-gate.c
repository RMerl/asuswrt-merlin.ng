// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2018 NXP
 * Copyright 2020 Linaro
 */

#include <common.h>
#include <efi.h>
#include <efi_loader.h>
#include <env.h>
#include <extension_board.h>
#include <hang.h>
#include <i2c.h>
#include <init.h>
#include <miiphy.h>
#include <netdev.h>
#include <i2c_eeprom.h>
#include <i2c.h>

#include <asm/arch/clock.h>
#include <asm/arch/imx8mm_pins.h>
#include <asm/arch/sys_proto.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/mach-imx/gpio.h>
#include <asm/mach-imx/mxc_i2c.h>
#include <asm/sections.h>
#include <linux/kernel.h>

#include "ddr/ddr.h"

DECLARE_GLOBAL_DATA_PTR;

#if IS_ENABLED(CONFIG_EFI_HAVE_CAPSULE_SUPPORT)
struct efi_fw_image fw_images[] = {
#if defined(CONFIG_TARGET_IMX8MM_CL_IOT_GATE)
	{
		.image_type_id = IMX8MM_CL_IOT_GATE_FIT_IMAGE_GUID,
		.fw_name = u"IMX8MM-CL-IOT-GATE-FIT",
		.image_index = 1,
	},
#elif defined(CONFIG_TARGET_IMX8MM_CL_IOT_GATE_OPTEE)
	{
		.image_type_id = IMX8MM_CL_IOT_GATE_OPTEE_FIT_IMAGE_GUID,
		.fw_name = u"IMX8MM-CL-IOT-GATE-FIT",
		.image_index = 1,
	},
#endif
};

struct efi_capsule_update_info update_info = {
	.dfu_string = "mmc 2=flash-bin raw 0x42 0x1D00 mmcpart 1",
	.num_images = ARRAY_SIZE(fw_images),
	.images = fw_images,
};

#endif /* EFI_HAVE_CAPSULE_SUPPORT */

int board_phys_sdram_size(phys_size_t *size)
{
	struct lpddr4_tcm_desc *lpddr4_tcm_desc =
		(struct lpddr4_tcm_desc *)TCM_DATA_CFG;

	switch (lpddr4_tcm_desc->size) {
	case 4096:
	case 2048:
	case 1024:
		*size = (1L << 20) * lpddr4_tcm_desc->size;
		break;
	default:
		printf("%s: DRAM size %uM is not supported\n",
		       __func__,
		       lpddr4_tcm_desc->size);
		hang();
		break;
	};

	return 0;
}

/* IOT_GATE-iMX8 extension boards ID */
typedef enum {
	IOT_GATE_EXT_EMPTY, /* No extension */
	IOT_GATE_EXT_CAN,   /* CAN bus */
	IOT_GATE_EXT_IED,   /* Bridge */
	IOT_GATE_EXT_POE,   /* POE */
	IOT_GATE_EXT_POEV2, /* POEv2 */
} iot_gate_imx8_ext;

typedef enum {
	IOT_GATE_IMX8_CARD_ID_EMPTY = 0,  /* card id - uninhabited */
	IOT_GATE_IMX8_CARD_ID_DI4O4 = 1,  /* Card ID - IED-DI4O4   */
	IOT_GATE_IMX8_CARD_ID_RS_485 = 2, /* Card ID - IED-RS485   */
	IOT_GATE_IMX8_CARD_ID_TPM = 3,    /* Card ID - IED-TPM     */
	IOT_GATE_IMX8_CARD_ID_CAN = 4,    /* Card ID - IED-CAN     */
	IOT_GATE_IMX8_CARD_ID_CL420 = 5,  /* Card ID - IED-CL420   */
	IOT_GATE_IMX8_CARD_ID_RS_232 = 6, /* Card ID - IED-RS232   */
} iot_gate_imx8_ied_ext;

static int setup_fec(void)
{
	if (IS_ENABLED(CONFIG_FEC_MXC)) {
		struct iomuxc_gpr_base_regs *gpr =
			(struct iomuxc_gpr_base_regs *)IOMUXC_GPR_BASE_ADDR;

		/* Use 125M anatop REF_CLK1 for ENET1, not from external */
		clrsetbits_le32(&gpr->gpr[1], 0x2000, 0);
	}

	return 0;
}

int board_phy_config(struct phy_device *phydev)
{
	if (IS_ENABLED(CONFIG_FEC_MXC)) {
		/* enable rgmii rxc skew and phy mode select to RGMII copper */
		phy_write(phydev, MDIO_DEVAD_NONE, 0x1d, 0x1f);
		phy_write(phydev, MDIO_DEVAD_NONE, 0x1e, 0x8);

		phy_write(phydev, MDIO_DEVAD_NONE, 0x1d, 0x00);
		phy_write(phydev, MDIO_DEVAD_NONE, 0x1e, 0x82ee);
		phy_write(phydev, MDIO_DEVAD_NONE, 0x1d, 0x05);
		phy_write(phydev, MDIO_DEVAD_NONE, 0x1e, 0x100);

		if (phydev->drv->config)
			phydev->drv->config(phydev);
	}
	return 0;
}

int board_init(void)
{
	if (IS_ENABLED(CONFIG_FEC_MXC))
		setup_fec();

	return 0;
}

int board_mmc_get_env_dev(int devno)
{
	return devno;
}

#define IOT_GATE_IMX8_EXT_I2C 3 /* I2C ID of the extension board */
#define IOT_GATE_IMX8_EXT_I2C_ADDR_EEPROM 0x54 /* I2C address of the EEPROM */

/* I2C address of the EEPROM in the POE extension */
#define IOT_GATE_IMX8_EXT_I2C_ADDR_EEPROM_POE 0x50
#define IOT_GATE_IMX8_EXT_I2C_ADDR_EEPROM_POEV2 0x51
#define IOT_GATE_IMX8_EXT_I2C_ADDR_GPIO 0x22 /* I2C address of the GPIO
						extender */

static int iot_gate_imx8_ext_id = IOT_GATE_EXT_EMPTY; /* Extension board ID */
static int iot_gate_imx8_ext_ied_id [3] = {
	IOT_GATE_IMX8_CARD_ID_EMPTY,
	IOT_GATE_IMX8_CARD_ID_EMPTY,
	IOT_GATE_IMX8_CARD_ID_EMPTY };

/*
 * iot_gate_imx8_detect_ext() - extended board detection
 * The detection is done according to the detected I2C devices.
 */
static void iot_gate_imx8_detect_ext(void)
{
	int ret;
	struct udevice *i2c_bus, *i2c_dev;

	ret = uclass_get_device_by_seq(UCLASS_I2C, IOT_GATE_IMX8_EXT_I2C,
				       &i2c_bus);
	if (ret) {
		printf("%s: Failed getting i2c device\n", __func__);
		return;
	}

	ret = dm_i2c_probe(i2c_bus, IOT_GATE_IMX8_EXT_I2C_ADDR_EEPROM_POE, 0,
			   &i2c_dev);
	if (!ret) {
		iot_gate_imx8_ext_id = IOT_GATE_EXT_POE;
		return;
	}

	ret = dm_i2c_probe(i2c_bus, IOT_GATE_IMX8_EXT_I2C_ADDR_EEPROM_POEV2, 0,
			   &i2c_dev);
	if (!ret) {
		iot_gate_imx8_ext_id = IOT_GATE_EXT_POEV2;
		return;
	}

	ret = dm_i2c_probe(i2c_bus, IOT_GATE_IMX8_EXT_I2C_ADDR_EEPROM, 0,
			   &i2c_dev);
	if (ret){
		iot_gate_imx8_ext_id = IOT_GATE_EXT_EMPTY;
		return;
	}
	/* Only the bridge extension includes the GPIO extender */
	ret = dm_i2c_probe(i2c_bus, IOT_GATE_IMX8_EXT_I2C_ADDR_GPIO, 0,
			   &i2c_dev);
	if (ret) /* GPIO extender not detected */
		iot_gate_imx8_ext_id = IOT_GATE_EXT_CAN;
	else /* GPIO extender detected */
		iot_gate_imx8_ext_id = IOT_GATE_EXT_IED;
}

static iomux_v3_cfg_t const iot_gate_imx8_ext_ied_pads[] = {
	IMX8MM_PAD_NAND_ALE_GPIO3_IO0	 | MUX_PAD_CTRL(PAD_CTL_PE),
	IMX8MM_PAD_NAND_CE0_B_GPIO3_IO1	 | MUX_PAD_CTRL(PAD_CTL_PE),
	IMX8MM_PAD_NAND_DATA00_GPIO3_IO6 | MUX_PAD_CTRL(PAD_CTL_PE),
	IMX8MM_PAD_NAND_DATA01_GPIO3_IO7 | MUX_PAD_CTRL(PAD_CTL_PE),
	IMX8MM_PAD_NAND_DATA02_GPIO3_IO8 | MUX_PAD_CTRL(PAD_CTL_PE),
	IMX8MM_PAD_NAND_DATA03_GPIO3_IO9 | MUX_PAD_CTRL(PAD_CTL_PE),
};

static iomux_v3_cfg_t const iot_gate_imx8_ext_poev2_pads[] = {
	IMX8MM_PAD_SAI3_TXD_GPIO5_IO1	 | MUX_PAD_CTRL(PAD_CTL_PE |
							PAD_CTL_PUE),
};

/* Extension board bridge GPIOs */
#define IOT_GATE_IMX8_GPIO_EXT_IED_I0 IMX_GPIO_NR(3, 0) /* IN 0 */
#define IOT_GATE_IMX8_GPIO_EXT_IED_I1 IMX_GPIO_NR(3, 1) /* IN 1 */
#define IOT_GATE_IMX8_GPIO_EXT_IED_I2 IMX_GPIO_NR(3, 6) /* IN 2 */
#define IOT_GATE_IMX8_GPIO_EXT_IED_I3 IMX_GPIO_NR(3, 7) /* IN 3 */
#define IOT_GATE_IMX8_GPIO_EXT_IED_O0 IMX_GPIO_NR(3, 8) /* OUT 0 */
#define IOT_GATE_IMX8_GPIO_EXT_IED_O1 IMX_GPIO_NR(3, 9) /* OUT 1 */
#define IOT_GATE_IMX8_GPIO_EXT_IED_O2 IMX_GPIO_NR(6, 9) /* OUT 2 */
#define IOT_GATE_IMX8_GPIO_EXT_IED_O3 IMX_GPIO_NR(6, 10)/* OUT 3 */

/* Extension board POE GPIOs */
#define IOT_GATE_IMX8_GPIO_EXT_POE_MUX IMX_GPIO_NR(5, 1)/* USB_MUX */

/*
 * iot_gate_imx8_update_pinmux() - update the pinmux
 * Update the pinmux according to the detected extended board.
 */
static void iot_gate_imx8_update_pinmux(void)
{
	if (iot_gate_imx8_ext_id == IOT_GATE_EXT_POEV2) {
		imx_iomux_v3_setup_multiple_pads(iot_gate_imx8_ext_poev2_pads,
				ARRAY_SIZE(iot_gate_imx8_ext_poev2_pads));
		gpio_request(IOT_GATE_IMX8_GPIO_EXT_POE_MUX, "poev2_usb-mux");
		/* Update USB MUX state */
		gpio_direction_output(IOT_GATE_IMX8_GPIO_EXT_POE_MUX, 1);

		return;
	}
	if (iot_gate_imx8_ext_id != IOT_GATE_EXT_IED)
		return;

	imx_iomux_v3_setup_multiple_pads(iot_gate_imx8_ext_ied_pads,
					 ARRAY_SIZE(iot_gate_imx8_ext_ied_pads));

	gpio_request(IOT_GATE_IMX8_GPIO_EXT_IED_I0, "ied-di4o4_i0");
	gpio_direction_input(IOT_GATE_IMX8_GPIO_EXT_IED_I0);
	gpio_request(IOT_GATE_IMX8_GPIO_EXT_IED_I1, "ied-di4o4_i1");
	gpio_direction_input(IOT_GATE_IMX8_GPIO_EXT_IED_I1);
	gpio_request(IOT_GATE_IMX8_GPIO_EXT_IED_I2, "ied-di4o4_i2");
	gpio_direction_input(IOT_GATE_IMX8_GPIO_EXT_IED_I2);
	gpio_request(IOT_GATE_IMX8_GPIO_EXT_IED_I3, "ied-di4o4_i3");
	gpio_direction_input(IOT_GATE_IMX8_GPIO_EXT_IED_I3);
	gpio_request(IOT_GATE_IMX8_GPIO_EXT_IED_O0, "ied-di4o4_o0");
	gpio_direction_output(IOT_GATE_IMX8_GPIO_EXT_IED_O0, 0);
	gpio_request(IOT_GATE_IMX8_GPIO_EXT_IED_O1, "ied-di4o4_o1");
	gpio_direction_output(IOT_GATE_IMX8_GPIO_EXT_IED_O1, 0);
	gpio_request(IOT_GATE_IMX8_GPIO_EXT_IED_O2, "ied-di4o4_o2");
	gpio_direction_output(IOT_GATE_IMX8_GPIO_EXT_IED_O2, 0);
	gpio_request(IOT_GATE_IMX8_GPIO_EXT_IED_O3, "ied-di4o4_o3");
	gpio_direction_output(IOT_GATE_IMX8_GPIO_EXT_IED_O3, 0);
}

#define IOT_GATE_IMX8_GPIO_S0B0 IMX_GPIO_NR(6, 0) /* Slot ID slot 0 bit 0 */
#define IOT_GATE_IMX8_GPIO_S0B1 IMX_GPIO_NR(6, 1) /* Slot ID slot 0 bit 1 */
#define IOT_GATE_IMX8_GPIO_S0B2 IMX_GPIO_NR(6, 2) /* Slot ID slot 0 bit 2 */
#define IOT_GATE_IMX8_GPIO_S1B0 IMX_GPIO_NR(6, 3) /* Slot ID slot 1 bit 0 */
#define IOT_GATE_IMX8_GPIO_S1B1 IMX_GPIO_NR(6, 4) /* Slot ID slot 1 bit 1 */
#define IOT_GATE_IMX8_GPIO_S1B2 IMX_GPIO_NR(6, 5) /* Slot ID slot 1 bit 2 */
#define IOT_GATE_IMX8_GPIO_S2B0 IMX_GPIO_NR(6, 6) /* Slot ID slot 2 bit 0 */
#define IOT_GATE_IMX8_GPIO_S2B1 IMX_GPIO_NR(6, 7) /* Slot ID slot 2 bit 1 */
#define IOT_GATE_IMX8_GPIO_S2B2 IMX_GPIO_NR(6, 8) /* Slot ID slot 2 bit 2 */

/*
 * iot_gate_imx8_update_ext_ied()
 * Update device tree of the extended board IED-BASE.
 * The device tree is updated according to the detected sub modules.
 *
 * Return 0 for success, 1 for failure.
 */
static int iot_gate_imx8_update_ext_ied(void)
{
	int revision;

	if (iot_gate_imx8_ext_id != IOT_GATE_EXT_IED)
		return 0;

	/* ID GPIO initializations */
	if (gpio_request(IOT_GATE_IMX8_GPIO_S0B0, "id_s0b0") ||
	    gpio_request(IOT_GATE_IMX8_GPIO_S0B1, "id_s0b1") ||
	    gpio_request(IOT_GATE_IMX8_GPIO_S0B2, "id_s0b2") ||
	    gpio_request(IOT_GATE_IMX8_GPIO_S1B0, "id_s1b0") ||
	    gpio_request(IOT_GATE_IMX8_GPIO_S1B1, "id_s1b1") ||
	    gpio_request(IOT_GATE_IMX8_GPIO_S1B2, "id_s1b2") ||
	    gpio_request(IOT_GATE_IMX8_GPIO_S2B0, "id_s2b0") ||
	    gpio_request(IOT_GATE_IMX8_GPIO_S2B1, "id_s2b1") ||
	    gpio_request(IOT_GATE_IMX8_GPIO_S2B2, "id_s2b2")) {
		printf("%s: ID GPIO request failure\n", __func__);
		return 1;
	}
	gpio_direction_input(IOT_GATE_IMX8_GPIO_S0B0);
	gpio_direction_input(IOT_GATE_IMX8_GPIO_S0B1);
	gpio_direction_input(IOT_GATE_IMX8_GPIO_S0B2);
	gpio_direction_input(IOT_GATE_IMX8_GPIO_S1B0);
	gpio_direction_input(IOT_GATE_IMX8_GPIO_S1B1);
	gpio_direction_input(IOT_GATE_IMX8_GPIO_S1B2);
	gpio_direction_input(IOT_GATE_IMX8_GPIO_S2B0);
	gpio_direction_input(IOT_GATE_IMX8_GPIO_S2B1);
	gpio_direction_input(IOT_GATE_IMX8_GPIO_S2B2);

	/* Get slot 0 card ID */
	revision =	gpio_get_value(IOT_GATE_IMX8_GPIO_S0B0)		|
			gpio_get_value(IOT_GATE_IMX8_GPIO_S0B1) << 1	|
			gpio_get_value(IOT_GATE_IMX8_GPIO_S0B2) << 2;
	iot_gate_imx8_ext_ied_id[0] = revision;

	/* Get slot 1 card ID */
	revision =	gpio_get_value(IOT_GATE_IMX8_GPIO_S1B0)		|
			gpio_get_value(IOT_GATE_IMX8_GPIO_S1B1) << 1	|
			gpio_get_value(IOT_GATE_IMX8_GPIO_S1B2) << 2;
	iot_gate_imx8_ext_ied_id[1] = revision;

	/* Get slot 2 card ID */
	revision =	gpio_get_value(IOT_GATE_IMX8_GPIO_S2B0)		|
			gpio_get_value(IOT_GATE_IMX8_GPIO_S2B1) << 1	|
			gpio_get_value(IOT_GATE_IMX8_GPIO_S2B2) << 2;
	iot_gate_imx8_ext_ied_id[2] = revision;

	return 0;
}

int extension_board_scan(struct list_head *extension_list)
{
	struct extension *extension = NULL;
	int i;
	int ret = 0;

	iot_gate_imx8_detect_ext(); /* Extended board detection */

	switch(iot_gate_imx8_ext_id) {
	case IOT_GATE_EXT_EMPTY:
		break;
	case IOT_GATE_EXT_CAN:
		extension = calloc(1, sizeof(struct extension));
		snprintf(extension->name, sizeof(extension->name),
			 "IOT_GATE_EXT_CAN");
		break;
	case IOT_GATE_EXT_IED:
		extension = calloc(1, sizeof(struct extension));
		snprintf(extension->name, sizeof(extension->name),
			 "IOT_GATE_EXT_IED");
		snprintf(extension->overlay, sizeof(extension->overlay),
			 "imx8mm-cl-iot-gate-ied.dtbo");
		break;
	case IOT_GATE_EXT_POE:
		extension = calloc(1, sizeof(struct extension));
		snprintf(extension->name, sizeof(extension->name),
			 "IOT_GATE_EXT_POE");
		break;
	case IOT_GATE_EXT_POEV2:
		extension = calloc(1, sizeof(struct extension));
		snprintf(extension->name, sizeof(extension->name),
			 "IOT_GATE_EXT_POEV2");
		break;
	default:
		printf("IOT_GATE-iMX8 extension board: unknown\n");
		break;
	}

	if (extension) {
		snprintf(extension->owner, sizeof(extension->owner),
			 "Compulab");
		list_add_tail(&extension->list, extension_list);
		ret = 1;
	} else
		return ret;

	iot_gate_imx8_update_pinmux();

	iot_gate_imx8_update_ext_ied();
	for (i=0; i<ARRAY_SIZE(iot_gate_imx8_ext_ied_id); i++) {
		extension = NULL;
		switch (iot_gate_imx8_ext_ied_id[i]) {
		case IOT_GATE_IMX8_CARD_ID_EMPTY:
			break;
		case IOT_GATE_IMX8_CARD_ID_RS_485:
			extension = calloc(1, sizeof(struct extension));
			snprintf(extension->name, sizeof(extension->name),
				 "IOT_GATE_IMX8_CARD_ID_RS_485");
			break;
		case IOT_GATE_IMX8_CARD_ID_RS_232:
			extension = calloc(1, sizeof(struct extension));
			snprintf(extension->name, sizeof(extension->name),
				 "IOT_GATE_IMX8_CARD_ID_RS_232");
			break;
		case IOT_GATE_IMX8_CARD_ID_CAN:
			extension = calloc(1, sizeof(struct extension));
			snprintf(extension->name, sizeof(extension->name),
				 "IOT_GATE_IMX8_CARD_ID_CAN");
			snprintf(extension->overlay, sizeof(extension->overlay),
				 "imx8mm-cl-iot-gate-ied-can%d.dtbo", i);
			break;
		case IOT_GATE_IMX8_CARD_ID_TPM:
			extension = calloc(1, sizeof(struct extension));
			snprintf(extension->name, sizeof(extension->name),
				 "IOT_GATE_IMX8_CARD_ID_TPM");
			snprintf(extension->overlay, sizeof(extension->overlay),
				 "imx8mm-cl-iot-gate-ied-tpm%d.dtbo", i);
			break;
		case IOT_GATE_IMX8_CARD_ID_CL420:
			extension = calloc(1, sizeof(struct extension));
			snprintf(extension->name, sizeof(extension->name),
				 "IOT_GATE_IMX8_CARD_ID_CL420");
			snprintf(extension->overlay, sizeof(extension->overlay),
				 "imx8mm-cl-iot-gate-ied-can%d.dtbo", i);
			break;
		case IOT_GATE_IMX8_CARD_ID_DI4O4:
			extension = calloc(1, sizeof(struct extension));
			snprintf(extension->name, sizeof(extension->name),
				 "IOT_GATE_IMX8_CARD_ID_DI4O4");
			break;
		default:
			printf("%s: invalid slot %d card ID: %d\n",
			       __func__, i, iot_gate_imx8_ext_ied_id[i]);
			break;
		}
		if (extension) {
			snprintf(extension->owner, sizeof(extension->owner),
				 "Compulab");
			snprintf(extension->other, sizeof(extension->other),
				 "On slot %d", i);
			list_add_tail(&extension->list, extension_list);
			ret = ret + 1;
		}
	}

        return ret;
}

static int setup_mac_address(void)
{
	unsigned char enetaddr[6];
	struct udevice *dev;
	int ret, off;

	ret = eth_env_get_enetaddr("ethaddr", enetaddr);
	if (ret)
		return 0;

	off = fdt_path_offset(gd->fdt_blob, "eeprom1");
	if (off < 0) {
		printf("No eeprom0 path offset found in DT\n");
		return off;
	}

	ret = uclass_get_device_by_of_offset(UCLASS_I2C_EEPROM, off, &dev);
	if (ret) {
		printf("%s: Could not find EEPROM\n", __func__);
		return ret;
	}

	ret = i2c_set_chip_offset_len(dev, 1);
	if (ret)
		return ret;

	ret = i2c_eeprom_read(dev, 4, enetaddr, sizeof(enetaddr));
	if (ret) {
		printf("%s: Could not read EEPROM\n", __func__);
		return ret;
	}

	ret = is_valid_ethaddr(enetaddr);
	if (!ret)
		return -EINVAL;

	ret = eth_env_set_enetaddr("ethaddr", enetaddr);
	if (ret)
		return ret;

	return 0;
}

static int read_serial_number(void)
{
	unsigned char serialnumber[6];
	unsigned char reversed[6];
	char serial_string[12];
	struct udevice *dev;
	int ret, off, i;

	off = fdt_path_offset(gd->fdt_blob, "eeprom0");
	if (off < 0) {
		printf("No eeprom0 path offset found in DT\n");
		return off;
	}

	ret = uclass_get_device_by_of_offset(UCLASS_I2C_EEPROM, off, &dev);
	if (ret) {
		printf("%s: Could not find EEPROM\n", __func__);
		return ret;
	}

	ret = i2c_set_chip_offset_len(dev, 1);
	if (ret)
		return ret;

	ret = i2c_eeprom_read(dev, 0x14, serialnumber, sizeof(serialnumber));
	if (ret) {
		printf("%s: Could not read EEPROM\n", __func__);
		return ret;
	}

	for (i = sizeof(serialnumber) - 1; i >= 0; i--)
		reversed[i] = serialnumber[sizeof(serialnumber) - 1 - i];

	for (i = 0; i < sizeof(reversed); i++) {
		serial_string[i * 2] = (reversed[i] >> 4) & 0xf;
		serial_string[i * 2 + 1] = reversed[i] & 0xf;
	}

	for (i = 0; i < sizeof(serial_string); i++)
		serial_string[i] += '0';

	env_set("serial#", serial_string);

	return 0;
}

int board_late_init(void)
{
	int ret;

	if (IS_ENABLED(CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG)) {
		env_set("board_name", "IOT-GATE-IMX8");
		env_set("board_rev", "SBC-IOTMX8");
	}

	ret = setup_mac_address();
	if (ret < 0)
		printf("Cannot set MAC address from EEPROM\n");

	ret = read_serial_number();
	if (ret < 0)
		printf("Cannot read serial number from EEPROM\n");

	return 0;
}
