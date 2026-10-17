// SPDX-License-Identifier:    GPL-2.0
/*
 * Copyright (C) 2018 Marvell International Ltd.
 */

#include <dm.h>
#include <malloc.h>
#include <miiphy.h>
#include <misc.h>
#include <pci.h>
#include <pci_ids.h>
#include <phy.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <linux/ctype.h>
#include <linux/delay.h>

#define PCI_DEVICE_ID_OCTEONTX_SMI 0xA02B

DECLARE_GLOBAL_DATA_PTR;

enum octeontx_smi_mode {
	CLAUSE22 = 0,
	CLAUSE45 = 1,
};

enum {
	SMI_OP_C22_WRITE = 0,
	SMI_OP_C22_READ = 1,

	SMI_OP_C45_ADDR = 0,
	SMI_OP_C45_WRITE = 1,
	SMI_OP_C45_PRIA = 2,
	SMI_OP_C45_READ = 3,
};

union smi_x_clk {
	u64 u;
	struct smi_x_clk_s {
		int phase:8;
		int sample:4;
		int preamble:1;
		int clk_idle:1;
		int reserved_14_14:1;
		int sample_mode:1;
		int sample_hi:5;
		int reserved_21_23:3;
		int mode:1;
	} s;
};

union smi_x_cmd {
	u64 u;
	struct smi_x_cmd_s {
		int reg_adr:5;
		int reserved_5_7:3;
		int phy_adr:5;
		int reserved_13_15:3;
		int phy_op:2;
	} s;
};

union smi_x_wr_dat {
	u64 u;
	struct smi_x_wr_dat_s {
		unsigned int dat:16;
		int val:1;
		int pending:1;
	} s;
};

union smi_x_rd_dat {
	u64 u;
	struct smi_x_rd_dat_s {
		unsigned int dat:16;
		int val:1;
		int pending:1;
	} s;
};

union smi_x_en {
	u64 u;
	struct smi_x_en_s {
		int en:1;
	} s;
};

#define SMI_X_RD_DAT	0x10ull
#define SMI_X_WR_DAT	0x08ull
#define SMI_X_CMD	0x00ull
#define SMI_X_CLK	0x18ull
#define SMI_X_EN	0x20ull

struct octeontx_smi_priv {
	void __iomem *baseaddr;
	enum octeontx_smi_mode mode;
};

#define MDIO_TIMEOUT 10000

void octeontx_smi_setmode(struct mii_dev *bus, enum octeontx_smi_mode mode)
{
	struct octeontx_smi_priv *priv = bus->priv;
	union smi_x_clk smix_clk;

	smix_clk.u = readq(priv->baseaddr + SMI_X_CLK);
	smix_clk.s.mode = mode;
	smix_clk.s.preamble = mode == CLAUSE45;
	writeq(smix_clk.u, priv->baseaddr + SMI_X_CLK);

	priv->mode = mode;
}

int octeontx_c45_addr(struct mii_dev *bus, int addr, int devad, int regnum)
{
	struct octeontx_smi_priv *priv = bus->priv;

	union smi_x_cmd smix_cmd;
	union smi_x_wr_dat smix_wr_dat;
	unsigned long timeout = MDIO_TIMEOUT;

	smix_wr_dat.u = 0;
	smix_wr_dat.s.dat = regnum;

	writeq(smix_wr_dat.u, priv->baseaddr + SMI_X_WR_DAT);

	smix_cmd.u = 0;
	smix_cmd.s.phy_op = SMI_OP_C45_ADDR;
	smix_cmd.s.phy_adr = addr;
	smix_cmd.s.reg_adr = devad;

	writeq(smix_cmd.u, priv->baseaddr + SMI_X_CMD);

	do {
		smix_wr_dat.u = readq(priv->baseaddr + SMI_X_WR_DAT);
		udelay(100);
		timeout--;
	} while (smix_wr_dat.s.pending && timeout);

	return timeout == 0;
}

int octeontx_phy_read(struct mii_dev *bus, int addr, int devad, int regnum)
{
	struct octeontx_smi_priv *priv = bus->priv;
	union smi_x_cmd smix_cmd;
	union smi_x_rd_dat smix_rd_dat;
	unsigned long timeout = MDIO_TIMEOUT;
	int ret;

	enum octeontx_smi_mode mode = (devad < 0) ? CLAUSE22 : CLAUSE45;

	debug("RD: Mode: %u, baseaddr: %p, addr: %d, devad: %d, reg: %d\n",
	      mode, priv->baseaddr, addr, devad, regnum);

	octeontx_smi_setmode(bus, mode);

	if (mode == CLAUSE45) {
		ret = octeontx_c45_addr(bus, addr, devad, regnum);

		debug("RD: ret: %u\n", ret);

		if (ret)
			return 0;
	}

	smix_cmd.u = 0;
	smix_cmd.s.phy_adr = addr;

	if (mode == CLAUSE45) {
		smix_cmd.s.reg_adr = devad;
		smix_cmd.s.phy_op = SMI_OP_C45_READ;
	} else {
		smix_cmd.s.reg_adr = regnum;
		smix_cmd.s.phy_op = SMI_OP_C22_READ;
	}

	writeq(smix_cmd.u, priv->baseaddr + SMI_X_CMD);

	do {
		smix_rd_dat.u = readq(priv->baseaddr + SMI_X_RD_DAT);
		udelay(10);
		timeout--;
	} while (smix_rd_dat.s.pending && timeout);

	debug("SMIX_RD_DAT: %lx\n", (unsigned long)smix_rd_dat.u);

	return smix_rd_dat.s.dat;
}

int octeontx_phy_write(struct mii_dev *bus, int addr, int devad, int regnum,
		       u16 value)
{
	struct octeontx_smi_priv *priv = bus->priv;
	union smi_x_cmd smix_cmd;
	union smi_x_wr_dat smix_wr_dat;
	unsigned long timeout = MDIO_TIMEOUT;
	int ret;

	enum octeontx_smi_mode mode = (devad < 0) ? CLAUSE22 : CLAUSE45;

	debug("WR: Mode: %u, baseaddr: %p, addr: %d, devad: %d, reg: %d\n",
	      mode, priv->baseaddr, addr, devad, regnum);

	if (mode == CLAUSE45) {
		ret = octeontx_c45_addr(bus, addr, devad, regnum);

		debug("WR: ret: %u\n", ret);

		if (ret)
			return ret;
	}

	smix_wr_dat.u = 0;
	smix_wr_dat.s.dat = value;

	writeq(smix_wr_dat.u, priv->baseaddr + SMI_X_WR_DAT);

	smix_cmd.u = 0;
	smix_cmd.s.phy_adr = addr;

	if (mode == CLAUSE45) {
		smix_cmd.s.reg_adr = devad;
		smix_cmd.s.phy_op = SMI_OP_C45_WRITE;
	} else {
		smix_cmd.s.reg_adr = regnum;
		smix_cmd.s.phy_op = SMI_OP_C22_WRITE;
	}

	writeq(smix_cmd.u, priv->baseaddr + SMI_X_CMD);

	do {
		smix_wr_dat.u = readq(priv->baseaddr + SMI_X_WR_DAT);
		udelay(10);
		timeout--;
	} while (smix_wr_dat.s.pending && timeout);

	debug("SMIX_WR_DAT: %lx\n", (unsigned long)smix_wr_dat.u);

	return timeout == 0;
}

int octeontx_smi_reset(struct mii_dev *bus)
{
	struct octeontx_smi_priv *priv = bus->priv;

	union smi_x_en smi_en;

	smi_en.s.en = 0;
	writeq(smi_en.u, priv->baseaddr + SMI_X_EN);

	smi_en.s.en = 1;
	writeq(smi_en.u, priv->baseaddr + SMI_X_EN);

	octeontx_smi_setmode(bus, CLAUSE22);

	return 0;
}

/* PHY XS initialization, primarily for RXAUI
 *
 */
int rxaui_phy_xs_init(struct mii_dev *bus, int phy_addr)
{
	int reg;
	ulong start_time;
	int phy_id1, phy_id2;
	int oui, model_number;

	phy_id1 = octeontx_phy_read(bus, phy_addr, 1, 0x2);
	phy_id2 = octeontx_phy_read(bus, phy_addr, 1, 0x3);
	model_number = (phy_id2 >> 4) & 0x3F;
	debug("%s model %x\n", __func__, model_number);
	oui = phy_id1;
	oui <<= 6;
	oui |= (phy_id2 >> 10) & 0x3F;
	debug("%s oui %x\n", __func__, oui);
	switch (oui) {
	case 0x5016:
		if (model_number == 9) {
			debug("%s +\n", __func__);
			/* Perform hardware reset in XGXS control */
			reg = octeontx_phy_read(bus, phy_addr, 4, 0x0);
			if ((reg & 0xffff) < 0)
				goto read_error;
			reg |= 0x8000;
			octeontx_phy_write(bus, phy_addr, 4, 0x0, reg);

			start_time = get_timer(0);
			do {
				reg = octeontx_phy_read(bus, phy_addr, 4, 0x0);
				if ((reg & 0xffff) < 0)
					goto read_error;
			} while ((reg & 0x8000) && get_timer(start_time) < 500);
			if (reg & 0x8000) {
				printf("HW reset for M88X3120 PHY failed");
				printf("MII_BMCR: 0x%x\n", reg);
				return -1;
			}
			/* program 4.49155 with 0x5 */
			octeontx_phy_write(bus, phy_addr, 4, 0xc003, 0x5);
		}
		break;
	default:
		break;
	}

	return 0;

read_error:
	debug("M88X3120 PHY config read failed\n");
	return -1;
}

int octeontx_smi_probe(struct udevice *dev)
{
	pci_dev_t bdf = dm_pci_get_bdf(dev);
	struct octeontx_smi_priv *priv;
	struct mii_dev *bus;
	int ret, cnt = 0;
	ofnode subnode;
	u64 baseaddr;

	debug("SMI PCI device: %x\n", bdf);
	if (!dm_pci_map_bar(dev, PCI_BASE_ADDRESS_0, 0, 0, PCI_REGION_TYPE, PCI_REGION_MEM)) {
		printf("Failed to map PCI region for bdf %x\n", bdf);
		return -1;
	}

	dev_for_each_subnode(subnode, dev) {
		if (!ofnode_device_is_compatible(subnode,
						 "cavium,thunder-8890-mdio"))
			continue;
		if (ofnode_read_u64(subnode, "reg", &baseaddr))
			continue;
		bus = mdio_alloc();
		priv = malloc(sizeof(*priv));
		if (!bus || !priv) {
			printf("Failed to allocate OcteonTX MDIO bus # %u\n",
			       dev_seq(dev));
			return -1;
		}

		bus->read = octeontx_phy_read;
		bus->write = octeontx_phy_write;
		bus->reset = octeontx_smi_reset;
		bus->priv = priv;

		priv->mode = CLAUSE22;
		priv->baseaddr = (void __iomem *)baseaddr;
		debug("mdio base addr %p\n", priv->baseaddr);

		/* use given name or generate its own unique name */
		snprintf(bus->name, MDIO_NAME_LEN, "smi%d", cnt++);

		ret = mdio_register(bus);
		if (ret)
			return ret;
	}
	return 0;
}

static const struct udevice_id octeontx_smi_ids[] = {
	{ .compatible = "cavium,thunder-8890-mdio-nexus" },
	{}
};

U_BOOT_DRIVER(octeontx_smi) = {
	.name	= "octeontx_smi",
	.id	= UCLASS_MISC,
	.probe	= octeontx_smi_probe,
	.of_match = octeontx_smi_ids,
};

static struct pci_device_id octeontx_smi_supported[] = {
	{ PCI_VDEVICE(CAVIUM, PCI_DEVICE_ID_CAVIUM_SMI) },
	{}
};

U_BOOT_PCI_DEVICE(octeontx_smi, octeontx_smi_supported);
