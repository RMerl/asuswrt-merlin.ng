// SPDX-License-Identifier: GPL-2.0+
/*
 * Faraday FTPCI100 PCI Bridge Controller Device Driver Implementation
 *
 * Copyright (C) 2011 Andes Technology Corporation
 * Gavin Guo, Andes Technology Corporation <gavinguo@andestech.com>
 * Macpaul Lin, Andes Technology Corporation <macpaul@andestech.com>
 */
#include <common.h>
#include <malloc.h>
#include <pci.h>

#include <faraday/ftpci100.h>

#include <asm/io.h>
#include <asm/types.h> /* u32, u16.... used by pci.h */

struct ftpci100_data {
	unsigned int reg_base;
	unsigned int io_base;
	unsigned int mem_base;
	unsigned int mmio_base;
	unsigned int ndevs;
};

static struct pci_config devs[FTPCI100_MAX_FUNCTIONS];
static struct pci_controller local_hose;

static void setup_pci_bar(unsigned int bus, unsigned int dev, unsigned func,
		unsigned char header, struct ftpci100_data *priv)
{
	struct pci_controller *hose = (struct pci_controller *)&local_hose;
	unsigned int i, tmp32, bar_no, iovsmem = 1;
	pci_dev_t dev_nu;

	/* A device is present, add an entry to the array */
	devs[priv->ndevs].bus = bus;
	devs[priv->ndevs].dev = dev;
	devs[priv->ndevs].func = func;

	dev_nu = PCI_BDF(bus, dev, func);

	if ((header & 0x7f) == 0x01)
		/* PCI-PCI Bridge */
		bar_no = 2;
	else
		bar_no = 6;

	/* Allocate address spaces by configuring BARs */
	for (i = 0; i < bar_no; i++) {
		pci_hose_write_config_dword(hose, dev_nu,
					PCI_BASE_ADDRESS_0 + i * 4, 0xffffffff);
		pci_hose_read_config_dword(hose, dev_nu,
					PCI_BASE_ADDRESS_0 + i * 4, &tmp32);

		if (tmp32 == 0x0)
			continue;

		/* IO space */
		if (tmp32 & 0x1) {
			iovsmem = 0;
			unsigned int size_mask = ~(tmp32 & 0xfffffffc);

			if (priv->io_base & size_mask)
				priv->io_base = (priv->io_base & ~size_mask) + \
						 size_mask + 1;

			devs[priv->ndevs].bar[i].addr = priv->io_base;
			devs[priv->ndevs].bar[i].size = size_mask + 1;

			pci_hose_write_config_dword(hose, dev_nu,
					PCI_BASE_ADDRESS_0 + i * 4,
					priv->io_base);

			debug("Allocated IO address 0x%X-" \
				"0x%X for Bus %d, Device %d, Function %d\n",
				priv->io_base,
				priv->io_base + size_mask, bus, dev, func);

			priv->io_base += size_mask + 1;
		} else {
			/* Memory space */
			unsigned int is_64bit = ((tmp32 & 0x6) == 0x4);
			unsigned int is_pref = tmp32 & 0x8;
			unsigned int size_mask = ~(tmp32 & 0xfffffff0);
			unsigned int alloc_base;
			unsigned int *addr_mem_base;

			if (is_pref)
				addr_mem_base = &priv->mem_base;
			else
				addr_mem_base = &priv->mmio_base;

			alloc_base = *addr_mem_base;

			if (alloc_base & size_mask)
				alloc_base = (alloc_base & ~size_mask) \
						+ size_mask + 1;

			pci_hose_write_config_dword(hose, dev_nu,
					PCI_BASE_ADDRESS_0 + i * 4, alloc_base);

			debug("Allocated %s address 0x%X-" \
				"0x%X for Bus %d, Device %d, Function %d\n",
				is_pref ? "MEM" : "MMIO", alloc_base,
				alloc_base + size_mask, bus, dev, func);

			devs[priv->ndevs].bar[i].addr = alloc_base;
			devs[priv->ndevs].bar[i].size = size_mask + 1;

			debug("BAR address  BAR size\n");
			debug("%010x  %08d\n",
				devs[priv->ndevs].bar[0].addr,
				devs[priv->ndevs].bar[0].size);

			alloc_base += size_mask + 1;
			*addr_mem_base = alloc_base;

			if (is_64bit) {
				i++;
				pci_hose_write_config_dword(hose, dev_nu,
					PCI_BASE_ADDRESS_0 + i * 4, 0x0);
			}
		}
	}

	/* Enable Bus Master, Memory Space, and IO Space */
	pci_hose_read_config_dword(hose, dev_nu, PCI_CACHE_LINE_SIZE, &tmp32);
	pci_hose_write_config_dword(hose, dev_nu, PCI_CACHE_LINE_SIZE, 0x08);
	pci_hose_read_config_dword(hose, dev_nu, PCI_CACHE_LINE_SIZE, &tmp32);

	pci_hose_read_config_dword(hose, dev_nu, PCI_COMMAND, &tmp32);

	tmp32 &= 0xffff;

	if (iovsmem == 0)
		tmp32 |= 0x5;
	else
		tmp32 |= 0x6;

	pci_hose_write_config_dword(hose, dev_nu, PCI_COMMAND, tmp32);
}

static void pci_bus_scan(struct ftpci100_data *priv)
{
	struct pci_controller *hose = (struct pci_controller *)&local_hose;
	unsigned int bus, dev, func;
	pci_dev_t dev_nu;
	unsigned int data32;
	unsigned int tmp;
	unsigned char header;
	unsigned char int_pin;
	unsigned int niobars;
	unsigned int nmbars;

	priv->ndevs = 1;

	nmbars = 0;
	niobars = 0;

	for (bus = 0; bus < MAX_BUS_NUM; bus++)
		for (dev = 0; dev < MAX_DEV_NUM; dev++)
			for (func = 0; func < MAX_FUN_NUM; func++) {
				dev_nu = PCI_BDF(bus, dev, func);
				pci_hose_read_config_dword(hose, dev_nu,
							PCI_VENDOR_ID, &data32);

				/*
				 * some broken boards return 0 or ~0,
				 * if a slot is empty.
				 */
				if (data32 == 0xffffffff ||
					data32 == 0x00000000 ||
					data32 == 0x0000ffff ||
					data32 == 0xffff0000)
					continue;

				pci_hose_read_config_dword(hose, dev_nu,
							PCI_HEADER_TYPE, &tmp);
				header = (unsigned char)tmp;
				setup_pci_bar(bus, dev, func, header, priv);

				devs[priv->ndevs].v_id = (u16)(data32 & \
								0x0000ffff);

				devs[priv->ndevs].d_id = (u16)((data32 & \
							0xffff0000) >> 16);

				/* Figure out what INTX# line the card uses */
				pci_hose_read_config_byte(hose, dev_nu,
						PCI_INTERRUPT_PIN, &int_pin);

				/* assign the appropriate irq line */
				if (int_pin > PCI_IRQ_LINES) {
					printf("more irq lines than expect\n");
				} else if (int_pin != 0) {
					/* This device uses an interrupt line */
					devs[priv->ndevs].pin = int_pin;
				}

				pci_hose_read_config_dword(hose, dev_nu,
						PCI_CLASS_DEVICE, &data32);

				debug("%06d  %03d  %03d  " \
					"%04d  %08x  %08x  " \
					"%03d  %08x  %06d  %08x\n",
					priv->ndevs, devs[priv->ndevs].bus,
					devs[priv->ndevs].dev,
					devs[priv->ndevs].func,
					devs[priv->ndevs].d_id,
					devs[priv->ndevs].v_id,
					devs[priv->ndevs].pin,
					devs[priv->ndevs].bar[0].addr,
					devs[priv->ndevs].bar[0].size,
					data32 >> 8);

				priv->ndevs++;
			}
}

static void ftpci_preinit(struct ftpci100_data *priv)
{
	struct ftpci100_ahbc *ftpci100;
	struct pci_controller *hose = (struct pci_controller *)&local_hose;
	u32 pci_config_addr;
	u32 pci_config_data;

	priv->reg_base = CONFIG_FTPCI100_BASE;
	priv->io_base = CONFIG_FTPCI100_BASE + CONFIG_FTPCI100_IO_SIZE;
	priv->mmio_base = CONFIG_FTPCI100_MEM_BASE;
	priv->mem_base = CONFIG_FTPCI100_MEM_BASE + CONFIG_FTPCI100_MEM_SIZE;

	ftpci100 = (struct ftpci100_ahbc *)priv->reg_base;

	pci_config_addr = (u32) &ftpci100->conf;
	pci_config_data = (u32) &ftpci100->data;

	/* print device name */
	printf("FTPCI100\n");

	/* dump basic configuration */
	debug("%s: Config addr is %08X, data port is %08X\n",
		__func__, pci_config_addr, pci_config_data);

	/* PCI memory space */
	pci_set_region(hose->regions + 0,
		CONFIG_PCI_MEM_BUS,
		CONFIG_PCI_MEM_PHYS,
		CONFIG_PCI_MEM_SIZE,
		PCI_REGION_MEM);
	hose->region_count++;

	/* PCI IO space */
	pci_set_region(hose->regions + 1,
		CONFIG_PCI_IO_BUS,
		CONFIG_PCI_IO_PHYS,
		CONFIG_PCI_IO_SIZE,
		PCI_REGION_IO);
	hose->region_count++;

#if defined(CONFIG_PCI_SYS_BUS)
	/* PCI System Memory space */
	pci_set_region(hose->regions + 2,
		CONFIG_PCI_SYS_BUS,
		CONFIG_PCI_SYS_PHYS,
		CONFIG_PCI_SYS_SIZE,
		PCI_REGION_MEM | PCI_REGION_SYS_MEMORY);
	hose->region_count++;
#endif

	/* setup indirect read/write function */
	pci_setup_indirect(hose, pci_config_addr, pci_config_data);

	/* register hose */
	pci_register_hose(hose);
}

void pci_ftpci_init(void)
{
	struct ftpci100_data *priv = NULL;
	struct pci_controller *hose = (struct pci_controller *)&local_hose;
	pci_dev_t bridge_num;

	struct pci_device_id bridge_ids[] = {
		{FTPCI100_BRIDGE_VENDORID, FTPCI100_BRIDGE_DEVICEID},
		{0, 0}
	};

	priv = malloc(sizeof(struct ftpci100_data));

	if (!priv) {
		printf("%s(): failed to malloc priv\n", __func__);
		return;
	}

	memset(priv, 0, sizeof(struct ftpci100_data));

	ftpci_preinit(priv);

	debug("Device  bus  dev  func  deviceID  vendorID  pin  address" \
		"   size    class\n");

	pci_bus_scan(priv);

	/*
	 * Setup the PCI Bridge Window to 1GB,
	 * it will cause USB OHCI Host controller Unrecoverable Error
	 * if it is not set.
	 */
	bridge_num = pci_find_devices(bridge_ids, 0);
	if (bridge_num == -1) {
		printf("PCI Bridge not found\n");
		return;
	}
	pci_hose_write_config_dword(hose, bridge_num, PCI_MEM_BASE_SIZE1,
					FTPCI100_BASE_ADR_SIZE(1024));
}
