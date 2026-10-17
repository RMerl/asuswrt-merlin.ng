// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2011 The Chromium Authors
 */

#include <common.h>
#include <dm.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/mrc_common.h>
#include <asm/arch/iomap.h>
#include <asm/arch/pch.h>
#include <asm/arch/pei_data.h>

__weak asmlinkage void sdram_console_tx_byte(unsigned char byte)
{
#ifdef DEBUG
	putc(byte);
#endif
}

void broadwell_fill_pei_data(struct pei_data *pei_data)
{
	pei_data->pei_version = PEI_VERSION;
	pei_data->board_type = BOARD_TYPE_ULT;
	pei_data->pciexbar = MCFG_BASE_ADDRESS;
	pei_data->smbusbar = SMBUS_BASE_ADDRESS;
	pei_data->ehcibar = EARLY_EHCI_BAR;
	pei_data->xhcibar = EARLY_XHCI_BAR;
	pei_data->gttbar = EARLY_GTT_BAR;
	pei_data->pmbase = ACPI_BASE_ADDRESS;
	pei_data->gpiobase = GPIO_BASE_ADDRESS;
	pei_data->tseg_size = CONFIG_SMM_TSEG_SIZE;
	pei_data->temp_mmio_base = EARLY_TEMP_MMIO;
	pei_data->tx_byte = sdram_console_tx_byte;
	pei_data->ddr_refresh_2x = 1;
}

static void pei_data_usb2_port(struct pei_data *pei_data, int port, uint length,
			       uint enable, uint oc_pin, uint location)
{
	pei_data->usb2_ports[port].length   = length;
	pei_data->usb2_ports[port].enable   = enable;
	pei_data->usb2_ports[port].oc_pin   = oc_pin;
	pei_data->usb2_ports[port].location = location;
}

static void pei_data_usb3_port(struct pei_data *pei_data, int port, uint enable,
			       uint oc_pin, uint fixed_eq)
{
	pei_data->usb3_ports[port].enable   = enable;
	pei_data->usb3_ports[port].oc_pin   = oc_pin;
	pei_data->usb3_ports[port].fixed_eq = fixed_eq;
}

void mainboard_fill_pei_data(struct pei_data *pei_data)
{
	/* DQ byte map for Samus board */
	const u8 dq_map[2][6][2] = {
		{ { 0x0F, 0xF0 }, { 0x00, 0xF0 }, { 0x0F, 0xF0 },
		  { 0x0F, 0x00 }, { 0xFF, 0x00 }, { 0xFF, 0x00 } },
		{ { 0x0F, 0xF0 }, { 0x00, 0xF0 }, { 0x0F, 0xF0 },
		  { 0x0F, 0x00 }, { 0xFF, 0x00 }, { 0xFF, 0x00 } } };
	/* DQS CPU<>DRAM map for Samus board */
	const u8 dqs_map[2][8] = {
		{ 2, 0, 1, 3, 6, 4, 7, 5 },
		{ 2, 1, 0, 3, 6, 5, 4, 7 } };

	pei_data->ec_present = 1;

	/* One installed DIMM per channel */
	pei_data->dimm_channel0_disabled = 2;
	pei_data->dimm_channel1_disabled = 2;

	memcpy(pei_data->dq_map, dq_map, sizeof(dq_map));
	memcpy(pei_data->dqs_map, dqs_map, sizeof(dqs_map));

	/* P0: HOST PORT */
	pei_data_usb2_port(pei_data, 0, 0x0080, 1, 0,
			   USB_PORT_BACK_PANEL);
	/* P1: HOST PORT */
	pei_data_usb2_port(pei_data, 1, 0x0080, 1, 1,
			   USB_PORT_BACK_PANEL);
	/* P2: RAIDEN */
	pei_data_usb2_port(pei_data, 2, 0x0080, 1, USB_OC_PIN_SKIP,
			   USB_PORT_BACK_PANEL);
	/* P3: SD CARD */
	pei_data_usb2_port(pei_data, 3, 0x0040, 1, USB_OC_PIN_SKIP,
			   USB_PORT_INTERNAL);
	/* P4: RAIDEN */
	pei_data_usb2_port(pei_data, 4, 0x0080, 1, USB_OC_PIN_SKIP,
			   USB_PORT_BACK_PANEL);
	/* P5: WWAN (Disabled) */
	pei_data_usb2_port(pei_data, 5, 0x0000, 0, USB_OC_PIN_SKIP,
			   USB_PORT_SKIP);
	/* P6: CAMERA */
	pei_data_usb2_port(pei_data, 6, 0x0040, 1, USB_OC_PIN_SKIP,
			   USB_PORT_INTERNAL);
	/* P7: BT */
	pei_data_usb2_port(pei_data, 7, 0x0040, 1, USB_OC_PIN_SKIP,
			   USB_PORT_INTERNAL);

	/* P1: HOST PORT */
	pei_data_usb3_port(pei_data, 0, 1, 0, 0);
	/* P2: HOST PORT */
	pei_data_usb3_port(pei_data, 1, 1, 1, 0);
	/* P3: RAIDEN */
	pei_data_usb3_port(pei_data, 2, 1, USB_OC_PIN_SKIP, 0);
	/* P4: RAIDEN */
	pei_data_usb3_port(pei_data, 3, 1, USB_OC_PIN_SKIP, 0);
}

static int broadwell_northbridge_early_init(struct udevice *dev)
{
	/* Move earlier? */
	dm_pci_write_config32(dev, PCIEXBAR + 4, 0);
	/* 64MiB - 0-63 buses */
	dm_pci_write_config32(dev, PCIEXBAR, MCFG_BASE_ADDRESS | 4 | 1);

	dm_pci_write_config32(dev, MCHBAR, MCH_BASE_ADDRESS | 1);
	dm_pci_write_config32(dev, DMIBAR, DMI_BASE_ADDRESS | 1);
	dm_pci_write_config32(dev, EPBAR, EP_BASE_ADDRESS | 1);
	writel(EDRAM_BASE_ADDRESS | 1, MCH_BASE_ADDRESS + EDRAMBAR);
	writel(GDXC_BASE_ADDRESS | 1, MCH_BASE_ADDRESS + GDXCBAR);

	/* Set C0000-FFFFF to access RAM on both reads and writes */
	dm_pci_write_config8(dev, PAM0, 0x30);
	dm_pci_write_config8(dev, PAM1, 0x33);
	dm_pci_write_config8(dev, PAM2, 0x33);
	dm_pci_write_config8(dev, PAM3, 0x33);
	dm_pci_write_config8(dev, PAM4, 0x33);
	dm_pci_write_config8(dev, PAM5, 0x33);
	dm_pci_write_config8(dev, PAM6, 0x33);

	/* Device enable: IGD and Mini-HD */
	dm_pci_write_config32(dev, DEVEN, DEVEN_D0EN | DEVEN_D2EN | DEVEN_D3EN);

	return 0;
}

static int broadwell_northbridge_probe(struct udevice *dev)
{
	if (!(gd->flags & GD_FLG_RELOC))
		return broadwell_northbridge_early_init(dev);

	return 0;
}

static const struct udevice_id broadwell_northbridge_ids[] = {
	{ .compatible = "intel,broadwell-northbridge" },
	{ }
};

U_BOOT_DRIVER(broadwell_northbridge_drv) = {
	.name		= "broadwell_northbridge",
	.id		= UCLASS_NORTHBRIDGE,
	.of_match	= broadwell_northbridge_ids,
	.probe		= broadwell_northbridge_probe,
};
