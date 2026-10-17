// SPDX-License-Identifier: GPL-2.0+
/*
 * Aquantia PHY drivers
 *
 * Copyright 2014 Freescale Semiconductor, Inc.
 * Copyright 2018, 2021 NXP
 */
#include <config.h>
#include <common.h>
#include <dm.h>
#include <log.h>
#include <net.h>
#include <phy.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <u-boot/crc.h>
#include <malloc.h>
#include <asm/byteorder.h>
#include <fs.h>

#define AQUNTIA_10G_CTL		0x20
#define AQUNTIA_VENDOR_P1	0xc400

#define AQUNTIA_SPEED_LSB_MASK	0x2000
#define AQUNTIA_SPEED_MSB_MASK	0x40

#define AQUANTIA_SYSTEM_INTERFACE_SR     0xe812
#define  AQUANTIA_SYSTEM_INTERFACE_SR_READY	BIT(0)
#define AQUANTIA_VENDOR_PROVISIONING_REG 0xC441
#define AQUANTIA_FIRMWARE_ID		 0x20
#define AQUANTIA_RESERVED_STATUS	 0xc885
#define AQUANTIA_FIRMWARE_MAJOR_MASK	 0xff00
#define AQUANTIA_FIRMWARE_MINOR_MASK	 0xff
#define AQUANTIA_FIRMWARE_BUILD_MASK	 0xf0

#define AQUANTIA_USX_AUTONEG_CONTROL_ENA 0x0008
#define AQUANTIA_SI_IN_USE_MASK          0x0078
#define AQUANTIA_SI_USXGMII              0x0018

/* registers in MDIO_MMD_VEND1 region */
#define AQUANTIA_VND1_GLOBAL_SC			0x000
#define  AQUANTIA_VND1_GLOBAL_SC_LP		BIT(0xb)

#define GLOBAL_FIRMWARE_ID 0x20
#define GLOBAL_FAULT 0xc850
#define GLOBAL_RSTATUS_1 0xc885

#define GLOBAL_ALARM_1 0xcc00
#define SYSTEM_READY_BIT 0x40

#define GLOBAL_STANDARD_CONTROL 0x0
#define SOFT_RESET BIT(15)
#define LOW_POWER BIT(11)

#define MAILBOX_CONTROL 0x0200
#define MAILBOX_EXECUTE BIT(15)
#define MAILBOX_WRITE BIT(14)
#define MAILBOX_RESET_CRC BIT(12)
#define MAILBOX_BUSY BIT(8)

#define MAILBOX_CRC 0x0201

#define MAILBOX_ADDR_MSW 0x0202
#define MAILBOX_ADDR_LSW 0x0203

#define MAILBOX_DATA_MSW 0x0204
#define MAILBOX_DATA_LSW 0x0205

#define UP_CONTROL 0xc001
#define UP_RESET BIT(15)
#define UP_RUN_STALL_OVERRIDE BIT(6)
#define UP_RUN_STALL BIT(0)

#define AQUANTIA_PMA_RX_VENDOR_P1		0xe400
#define  AQUANTIA_PMA_RX_VENDOR_P1_MDI_MSK	GENMASK(1, 0)
/* MDI reversal configured through registers */
#define  AQUANTIA_PMA_RX_VENDOR_P1_MDI_CFG	BIT(1)
/* MDI reversal enabled */
#define  AQUANTIA_PMA_RX_VENDOR_P1_MDI_REV	BIT(0)

/*
 * global start rate, the protocol associated with this speed is used by default
 * on SI.
 */
#define AQUANTIA_VND1_GSTART_RATE		0x31a
#define  AQUANTIA_VND1_GSTART_RATE_OFF		0
#define  AQUANTIA_VND1_GSTART_RATE_100M		1
#define  AQUANTIA_VND1_GSTART_RATE_1G		2
#define  AQUANTIA_VND1_GSTART_RATE_10G		3
#define  AQUANTIA_VND1_GSTART_RATE_2_5G		4
#define  AQUANTIA_VND1_GSTART_RATE_5G		5

/* SYSCFG registers for 100M, 1G, 2.5G, 5G, 10G */
#define AQUANTIA_VND1_GSYSCFG_BASE		0x31b
#define AQUANTIA_VND1_GSYSCFG_100M		0
#define AQUANTIA_VND1_GSYSCFG_1G		1
#define AQUANTIA_VND1_GSYSCFG_2_5G		2
#define AQUANTIA_VND1_GSYSCFG_5G		3
#define AQUANTIA_VND1_GSYSCFG_10G		4

#define AQUANTIA_VND1_SMBUS0			0xc485
#define AQUANTIA_VND1_SMBUS1			0xc495

/* addresses of memory segments in the phy */
#define DRAM_BASE_ADDR 0x3FFE0000
#define IRAM_BASE_ADDR 0x40000000

/* firmware image format constants */
#define VERSION_STRING_SIZE 0x40
#define VERSION_STRING_OFFSET 0x0200
#define HEADER_OFFSET 0x300

/* driver private data */
#define AQUANTIA_NA		0
#define AQUANTIA_GEN1		1
#define AQUANTIA_GEN2		2
#define AQUANTIA_GEN3		3

#pragma pack(1)
struct fw_header {
	u8 padding[4];
	u8 iram_offset[3];
	u8 iram_size[3];
	u8 dram_offset[3];
	u8 dram_size[3];
};

#pragma pack()

#if defined(CONFIG_PHY_AQUANTIA_UPLOAD_FW)
static int aquantia_read_fw(u8 **fw_addr, size_t *fw_length)
{
	loff_t length, read;
	int ret;
	void *addr = NULL;

	*fw_addr = NULL;
	*fw_length = 0;
	debug("Loading Aquantia microcode from %s %s\n",
	      CONFIG_PHY_AQUANTIA_FW_PART, CONFIG_PHY_AQUANTIA_FW_NAME);
	ret = fs_set_blk_dev("mmc", CONFIG_PHY_AQUANTIA_FW_PART, FS_TYPE_ANY);
	if (ret < 0)
		goto cleanup;

	ret = fs_size(CONFIG_PHY_AQUANTIA_FW_NAME, &length);
	if (ret < 0)
		goto cleanup;

	addr = malloc(length);
	if (!addr) {
		ret = -ENOMEM;
		goto cleanup;
	}

	ret = fs_set_blk_dev("mmc", CONFIG_PHY_AQUANTIA_FW_PART, FS_TYPE_ANY);
	if (ret < 0)
		goto cleanup;

	ret = fs_read(CONFIG_PHY_AQUANTIA_FW_NAME, (ulong)addr, 0, length,
		      &read);
	if (ret < 0)
		goto cleanup;

	*fw_addr = addr;
	*fw_length = length;
	debug("Found Aquantia microcode.\n");

cleanup:
	if (ret < 0) {
		printf("loading firmware file %s %s failed with error %d\n",
		       CONFIG_PHY_AQUANTIA_FW_PART,
		       CONFIG_PHY_AQUANTIA_FW_NAME, ret);
		free(addr);
	}
	return ret;
}

/* load data into the phy's memory */
static int aquantia_load_memory(struct phy_device *phydev, u32 addr,
				const u8 *data, size_t len)
{
	size_t pos;
	u16 crc = 0, up_crc;

	phy_write(phydev, MDIO_MMD_VEND1, MAILBOX_CONTROL, MAILBOX_RESET_CRC);
	phy_write(phydev, MDIO_MMD_VEND1, MAILBOX_ADDR_MSW, addr >> 16);
	phy_write(phydev, MDIO_MMD_VEND1, MAILBOX_ADDR_LSW, addr & 0xfffc);

	for (pos = 0; pos < len; pos += min(sizeof(u32), len - pos)) {
		u32 word = 0;

		memcpy(&word, &data[pos], min(sizeof(u32), len - pos));

		phy_write(phydev, MDIO_MMD_VEND1, MAILBOX_DATA_MSW,
			  (word >> 16));
		phy_write(phydev, MDIO_MMD_VEND1, MAILBOX_DATA_LSW,
			  word & 0xffff);

		phy_write(phydev, MDIO_MMD_VEND1, MAILBOX_CONTROL,
			  MAILBOX_EXECUTE | MAILBOX_WRITE);

		/* keep a big endian CRC to match the phy processor */
		word = cpu_to_be32(word);
		crc = crc16_ccitt(crc, (u8 *)&word, sizeof(word));
	}

	up_crc = phy_read(phydev, MDIO_MMD_VEND1, MAILBOX_CRC);
	if (crc != up_crc) {
		printf("%s crc mismatch: calculated 0x%04hx phy 0x%04hx\n",
		       phydev->dev->name, crc, up_crc);
		return -EINVAL;
	}
	return 0;
}

static u32 unpack_u24(const u8 *data)
{
	return (data[2] << 16) + (data[1] << 8) + data[0];
}

static int aquantia_upload_firmware(struct phy_device *phydev)
{
	int ret;
	u8 *addr = NULL;
	size_t fw_length = 0;
	u16 calculated_crc, read_crc;
	char version[VERSION_STRING_SIZE];
	u32 primary_offset, iram_offset, iram_size, dram_offset, dram_size;
	const struct fw_header *header;

	ret = aquantia_read_fw(&addr, &fw_length);
	if (ret != 0)
		return ret;

	read_crc = (addr[fw_length - 2] << 8)  | addr[fw_length - 1];
	calculated_crc = crc16_ccitt(0, addr, fw_length - 2);
	if (read_crc != calculated_crc) {
		printf("%s bad firmware crc: file 0x%04x calculated 0x%04x\n",
		       phydev->dev->name, read_crc, calculated_crc);
		ret = -EINVAL;
		goto done;
	}

	/* Find the DRAM and IRAM sections within the firmware file. */
	primary_offset = ((addr[9] & 0xf) << 8 | addr[8]) << 12;

	header = (struct fw_header *)&addr[primary_offset + HEADER_OFFSET];

	iram_offset = primary_offset + unpack_u24(header->iram_offset);
	iram_size = unpack_u24(header->iram_size);

	dram_offset = primary_offset + unpack_u24(header->dram_offset);
	dram_size = unpack_u24(header->dram_size);

	debug("primary %d iram offset=%d size=%d dram offset=%d size=%d\n",
	      primary_offset, iram_offset, iram_size, dram_offset, dram_size);

	strlcpy(version, (char *)&addr[dram_offset + VERSION_STRING_OFFSET],
		VERSION_STRING_SIZE);
	printf("%s loading firmware version '%s'\n", phydev->dev->name, version);

	/* stall the microcprocessor */
	phy_write(phydev, MDIO_MMD_VEND1, UP_CONTROL,
		  UP_RUN_STALL | UP_RUN_STALL_OVERRIDE);

	debug("loading dram 0x%08x from offset=%d size=%d\n",
	      DRAM_BASE_ADDR, dram_offset, dram_size);
	ret = aquantia_load_memory(phydev, DRAM_BASE_ADDR, &addr[dram_offset],
				   dram_size);
	if (ret != 0)
		goto done;

	debug("loading iram 0x%08x from offset=%d size=%d\n",
	      IRAM_BASE_ADDR, iram_offset, iram_size);
	ret = aquantia_load_memory(phydev, IRAM_BASE_ADDR, &addr[iram_offset],
				   iram_size);
	if (ret != 0)
		goto done;

	/* make sure soft reset and low power mode are clear */
	phy_write(phydev, MDIO_MMD_VEND1, GLOBAL_STANDARD_CONTROL, 0);

	/* Release the microprocessor. UP_RESET must be held for 100 usec. */
	phy_write(phydev, MDIO_MMD_VEND1, UP_CONTROL,
		  UP_RUN_STALL | UP_RUN_STALL_OVERRIDE | UP_RESET);

	udelay(100);

	phy_write(phydev, MDIO_MMD_VEND1, UP_CONTROL, UP_RUN_STALL_OVERRIDE);

	printf("%s firmware loading done.\n", phydev->dev->name);
done:
	free(addr);
	return ret;
}
#else
static int aquantia_upload_firmware(struct phy_device *phydev)
{
	printf("ERROR %s firmware loading disabled.\n", phydev->dev->name);
	return -1;
}
#endif

struct {
	u16 syscfg;
	int cnt;
	u16 start_rate;
} aquantia_syscfg[PHY_INTERFACE_MODE_MAX] = {
	[PHY_INTERFACE_MODE_SGMII] =      {0x04b, AQUANTIA_VND1_GSYSCFG_1G,
					   AQUANTIA_VND1_GSTART_RATE_1G},
	[PHY_INTERFACE_MODE_2500BASEX]  = {0x144, AQUANTIA_VND1_GSYSCFG_2_5G,
					   AQUANTIA_VND1_GSTART_RATE_2_5G},
	[PHY_INTERFACE_MODE_10GBASER] =   {0x100, AQUANTIA_VND1_GSYSCFG_10G,
					   AQUANTIA_VND1_GSTART_RATE_10G},
	[PHY_INTERFACE_MODE_USXGMII] =    {0x080, AQUANTIA_VND1_GSYSCFG_10G,
					   AQUANTIA_VND1_GSTART_RATE_10G},
};

static int aquantia_set_proto(struct phy_device *phydev,
			      phy_interface_t interface)
{
	int i;

	if (!aquantia_syscfg[interface].cnt)
		return 0;

	/* set the default rate to enable the SI link */
	phy_write(phydev, MDIO_MMD_VEND1, AQUANTIA_VND1_GSTART_RATE,
		  aquantia_syscfg[interface].start_rate);

	/* set selected protocol for all relevant line side link speeds */
	for (i = 0; i <= aquantia_syscfg[interface].cnt; i++)
		phy_write(phydev, MDIO_MMD_VEND1,
			  AQUANTIA_VND1_GSYSCFG_BASE + i,
			  aquantia_syscfg[interface].syscfg);
	return 0;
}

static int aquantia_dts_config(struct phy_device *phydev)
{
	ofnode node = phydev->node;
	u32 prop;
	u16 reg;

	/* this code only works on gen2 and gen3 PHYs */
	if (phydev->drv->data != AQUANTIA_GEN2 &&
	    phydev->drv->data != AQUANTIA_GEN3)
		return -ENOTSUPP;

	if (!ofnode_valid(node))
		return 0;

	if (!ofnode_read_u32(node, "mdi-reversal", &prop)) {
		debug("mdi-reversal = %d\n", (int)prop);
		reg =  phy_read(phydev, MDIO_MMD_PMAPMD,
				AQUANTIA_PMA_RX_VENDOR_P1);
		reg &= ~AQUANTIA_PMA_RX_VENDOR_P1_MDI_MSK;
		reg |= AQUANTIA_PMA_RX_VENDOR_P1_MDI_CFG;
		reg |= prop ? AQUANTIA_PMA_RX_VENDOR_P1_MDI_REV : 0;
		phy_write(phydev, MDIO_MMD_PMAPMD, AQUANTIA_PMA_RX_VENDOR_P1,
			  reg);
	}
	if (!ofnode_read_u32(node, "smb-addr", &prop)) {
		debug("smb-addr = %x\n", (int)prop);
		/*
		 * there are two addresses here, normally just one bus would
		 * be in use so we're setting both regs using the same DT
		 * property.
		 */
		phy_write(phydev, MDIO_MMD_VEND1, AQUANTIA_VND1_SMBUS0,
			  (u16)(prop << 1));
		phy_write(phydev, MDIO_MMD_VEND1, AQUANTIA_VND1_SMBUS1,
			  (u16)(prop << 1));
	}

	return 0;
}

static bool aquantia_link_is_up(struct phy_device *phydev)
{
	u16 reg, regmask;
	int devad, regnum;

	/*
	 * On Gen 2 and 3 we have a bit that indicates that both system and
	 * line side are ready for data, use that if possible.
	 */
	if (phydev->drv->data == AQUANTIA_GEN2 ||
	    phydev->drv->data == AQUANTIA_GEN3) {
		devad = MDIO_MMD_PHYXS;
		regnum = AQUANTIA_SYSTEM_INTERFACE_SR;
		regmask = AQUANTIA_SYSTEM_INTERFACE_SR_READY;
	} else {
		devad = MDIO_MMD_AN;
		regnum = MDIO_STAT1;
		regmask = MDIO_AN_STAT1_COMPLETE;
	}
	/* the register should be latched, do a double read */
	phy_read(phydev, devad, regnum);
	reg = phy_read(phydev, devad, regnum);

	return !!(reg & regmask);
}

int aquantia_config(struct phy_device *phydev)
{
	int interface = phydev->interface;
	u32 val, id, rstatus, fault;
	u32 reg_val1 = 0;
	int num_retries = 5;
	int usx_an = 0;

	/*
	 * check if the system is out of reset and init sequence completed.
	 * chip-wide reset for gen1 quad phys takes longer
	 */
	while (--num_retries) {
		rstatus = phy_read(phydev, MDIO_MMD_VEND1, GLOBAL_ALARM_1);
		if (rstatus & SYSTEM_READY_BIT)
			break;
		mdelay(10);
	}

	id = phy_read(phydev, MDIO_MMD_VEND1, GLOBAL_FIRMWARE_ID);
	rstatus = phy_read(phydev, MDIO_MMD_VEND1, GLOBAL_RSTATUS_1);
	fault = phy_read(phydev, MDIO_MMD_VEND1, GLOBAL_FAULT);

	if (id != 0)
		debug("%s running firmware version %X.%X.%X\n",
		      phydev->dev->name, (id >> 8), id & 0xff,
		      (rstatus >> 4) & 0xf);

	if (fault != 0)
		printf("%s fault 0x%04x detected\n", phydev->dev->name, fault);

	if (id == 0 || fault != 0) {
		int ret;

		ret = aquantia_upload_firmware(phydev);
		if (ret != 0)
			return ret;
	}
	/*
	 * for backward compatibility convert XGMII into either 10GBase-R or
	 * USXGMII based on FW config
	 */
	if (interface == PHY_INTERFACE_MODE_XGMII) {
		debug("use 10GBase-R or USXGMII SI protos, XGMII is not valid\n");

		reg_val1 = phy_read(phydev, MDIO_MMD_PHYXS,
				    AQUANTIA_SYSTEM_INTERFACE_SR);
		if ((reg_val1 & AQUANTIA_SI_IN_USE_MASK) == AQUANTIA_SI_USXGMII)
			interface = PHY_INTERFACE_MODE_USXGMII;
		else
			interface = PHY_INTERFACE_MODE_10GBASER;
	}

	/*
	 * if link is up already we can just use it, otherwise configure
	 * the protocols in the PHY.  If link is down set the system
	 * interface protocol to use based on phydev->interface
	 */
	if (!aquantia_link_is_up(phydev) &&
	    (phydev->drv->data == AQUANTIA_GEN2 ||
	     phydev->drv->data == AQUANTIA_GEN3)) {
		/* set PHY in low power mode so we can configure protocols */
		phy_write(phydev, MDIO_MMD_VEND1, AQUANTIA_VND1_GLOBAL_SC,
			  AQUANTIA_VND1_GLOBAL_SC_LP);
		mdelay(10);

		/* configure protocol based on phydev->interface */
		aquantia_set_proto(phydev, interface);
		/* apply custom configuration based on DT */
		aquantia_dts_config(phydev);

		/* wake PHY back up */
		phy_write(phydev, MDIO_MMD_VEND1, AQUANTIA_VND1_GLOBAL_SC, 0);
		mdelay(10);
	}

	val = phy_read(phydev, MDIO_MMD_PMAPMD, MII_BMCR);

	switch (interface) {
	case PHY_INTERFACE_MODE_SGMII:
		/* 1000BASE-T mode */
		phydev->advertising = SUPPORTED_1000baseT_Full;
		phydev->supported = phydev->advertising;

		val = (val & ~AQUNTIA_SPEED_LSB_MASK) | AQUNTIA_SPEED_MSB_MASK;
		phy_write(phydev, MDIO_MMD_PMAPMD, MII_BMCR, val);
		break;
	case PHY_INTERFACE_MODE_USXGMII:
		usx_an = 1;
		/* FALLTHROUGH */
	case PHY_INTERFACE_MODE_10GBASER:
		/* 10GBASE-T mode */
		phydev->advertising = SUPPORTED_10000baseT_Full;
		phydev->supported = phydev->advertising;

		if (!(val & AQUNTIA_SPEED_LSB_MASK) ||
		    !(val & AQUNTIA_SPEED_MSB_MASK))
			phy_write(phydev, MDIO_MMD_PMAPMD, MII_BMCR,
				  AQUNTIA_SPEED_LSB_MASK |
				  AQUNTIA_SPEED_MSB_MASK);

		/* If SI is USXGMII then start USXGMII autoneg */
		reg_val1 =  phy_read(phydev, MDIO_MMD_PHYXS,
				     AQUANTIA_VENDOR_PROVISIONING_REG);

		if (usx_an) {
			reg_val1 |= AQUANTIA_USX_AUTONEG_CONTROL_ENA;
			debug("%s: system interface USXGMII\n",
			      phydev->dev->name);
		} else {
			reg_val1 &= ~AQUANTIA_USX_AUTONEG_CONTROL_ENA;
			debug("%s: system interface 10GBase-R\n",
			      phydev->dev->name);
		}

		phy_write(phydev, MDIO_MMD_PHYXS,
			  AQUANTIA_VENDOR_PROVISIONING_REG, reg_val1);
		break;
	case PHY_INTERFACE_MODE_2500BASEX:
		/* 2.5GBASE-T mode */
		phydev->advertising = SUPPORTED_1000baseT_Full;
		phydev->supported = phydev->advertising;

		phy_write(phydev, MDIO_MMD_AN, AQUNTIA_10G_CTL, 1);
		phy_write(phydev, MDIO_MMD_AN, AQUNTIA_VENDOR_P1, 0x9440);
		break;
	case PHY_INTERFACE_MODE_MII:
		/* 100BASE-TX mode */
		phydev->advertising = SUPPORTED_100baseT_Full;
		phydev->supported = phydev->advertising;

		val = (val & ~AQUNTIA_SPEED_MSB_MASK) | AQUNTIA_SPEED_LSB_MASK;
		phy_write(phydev, MDIO_MMD_PMAPMD, MII_BMCR, val);
		break;
	};

	val = phy_read(phydev, MDIO_MMD_VEND1, AQUANTIA_RESERVED_STATUS);
	reg_val1 = phy_read(phydev, MDIO_MMD_VEND1, AQUANTIA_FIRMWARE_ID);

	debug("%s: %s Firmware Version %x.%x.%x\n", phydev->dev->name,
	      phydev->drv->name,
	      (reg_val1 & AQUANTIA_FIRMWARE_MAJOR_MASK) >> 8,
	      reg_val1 & AQUANTIA_FIRMWARE_MINOR_MASK,
	      (val & AQUANTIA_FIRMWARE_BUILD_MASK) >> 4);

	return 0;
}

int aquantia_startup(struct phy_device *phydev)
{
	u32 speed;
	int i = 0;
	int reg;

	phydev->duplex = DUPLEX_FULL;

	/* if the AN is still in progress, wait till timeout. */
	if (!aquantia_link_is_up(phydev)) {
		printf("%s Waiting for PHY auto negotiation to complete",
		       phydev->dev->name);
		do {
			udelay(1000);
			if ((i++ % 500) == 0)
				printf(".");
		} while (!aquantia_link_is_up(phydev) &&
			 i < (4 * PHY_ANEG_TIMEOUT));

		if (i > PHY_ANEG_TIMEOUT)
			printf(" TIMEOUT !\n");
	}

	/* Read twice because link state is latched and a
	 * read moves the current state into the register */
	phy_read(phydev, MDIO_MMD_AN, MDIO_STAT1);
	reg = phy_read(phydev, MDIO_MMD_AN, MDIO_STAT1);
	if (reg < 0 || !(reg & MDIO_STAT1_LSTATUS))
		phydev->link = 0;
	else
		phydev->link = 1;

	speed = phy_read(phydev, MDIO_MMD_PMAPMD, MII_BMCR);
	if (speed & AQUNTIA_SPEED_MSB_MASK) {
		if (speed & AQUNTIA_SPEED_LSB_MASK)
			phydev->speed = SPEED_10000;
		else
			phydev->speed = SPEED_1000;
	} else {
		if (speed & AQUNTIA_SPEED_LSB_MASK)
			phydev->speed = SPEED_100;
		else
			phydev->speed = SPEED_10;
	}

	return 0;
}

U_BOOT_PHY_DRIVER(aq1202) = {
	.name = "Aquantia AQ1202",
	.uid = 0x3a1b445,
	.mask = 0xfffffff0,
	.features = PHY_10G_FEATURES,
	.mmds = (MDIO_MMD_PMAPMD | MDIO_MMD_PCS|
			MDIO_MMD_PHYXS | MDIO_MMD_AN |
			MDIO_MMD_VEND1),
	.config = &aquantia_config,
	.startup = &aquantia_startup,
	.shutdown = &gen10g_shutdown,
};

U_BOOT_PHY_DRIVER(aq2104) = {
	.name = "Aquantia AQ2104",
	.uid = 0x3a1b460,
	.mask = 0xfffffff0,
	.features = PHY_10G_FEATURES,
	.mmds = (MDIO_MMD_PMAPMD | MDIO_MMD_PCS|
			MDIO_MMD_PHYXS | MDIO_MMD_AN |
			MDIO_MMD_VEND1),
	.config = &aquantia_config,
	.startup = &aquantia_startup,
	.shutdown = &gen10g_shutdown,
};

U_BOOT_PHY_DRIVER(aqr105) = {
	.name = "Aquantia AQR105",
	.uid = 0x3a1b4a2,
	.mask = 0xfffffff0,
	.features = PHY_10G_FEATURES,
	.mmds = (MDIO_MMD_PMAPMD | MDIO_MMD_PCS|
			MDIO_MMD_PHYXS | MDIO_MMD_AN |
			MDIO_MMD_VEND1),
	.config = &aquantia_config,
	.startup = &aquantia_startup,
	.shutdown = &gen10g_shutdown,
	.data = AQUANTIA_GEN1,
};

U_BOOT_PHY_DRIVER(aqr106) = {
	.name = "Aquantia AQR106",
	.uid = 0x3a1b4d0,
	.mask = 0xfffffff0,
	.features = PHY_10G_FEATURES,
	.mmds = (MDIO_MMD_PMAPMD | MDIO_MMD_PCS|
			MDIO_MMD_PHYXS | MDIO_MMD_AN |
			MDIO_MMD_VEND1),
	.config = &aquantia_config,
	.startup = &aquantia_startup,
	.shutdown = &gen10g_shutdown,
};

U_BOOT_PHY_DRIVER(aqr107) = {
	.name = "Aquantia AQR107",
	.uid = 0x3a1b4e0,
	.mask = 0xfffffff0,
	.features = PHY_10G_FEATURES,
	.mmds = (MDIO_MMD_PMAPMD | MDIO_MMD_PCS|
			MDIO_MMD_PHYXS | MDIO_MMD_AN |
			MDIO_MMD_VEND1),
	.config = &aquantia_config,
	.startup = &aquantia_startup,
	.shutdown = &gen10g_shutdown,
	.data = AQUANTIA_GEN2,
};

U_BOOT_PHY_DRIVER(aqr112) = {
	.name = "Aquantia AQR112",
	.uid = 0x3a1b660,
	.mask = 0xfffffff0,
	.features = PHY_10G_FEATURES,
	.mmds = (MDIO_MMD_PMAPMD | MDIO_MMD_PCS |
		 MDIO_MMD_PHYXS | MDIO_MMD_AN |
		 MDIO_MMD_VEND1),
	.config = &aquantia_config,
	.startup = &aquantia_startup,
	.shutdown = &gen10g_shutdown,
	.data = AQUANTIA_GEN3,
};

U_BOOT_PHY_DRIVER(aqr113c) = {
	.name = "Aquantia AQR113C",
	.uid = 0x31c31c12,
	.mask = 0xfffffff0,
	.features = PHY_10G_FEATURES,
	.mmds = (MDIO_MMD_PMAPMD | MDIO_MMD_PCS |
		 MDIO_MMD_PHYXS | MDIO_MMD_AN |
		 MDIO_MMD_VEND1),
	.config = &aquantia_config,
	.startup = &aquantia_startup,
	.shutdown = &gen10g_shutdown,
	.data = AQUANTIA_GEN3,
};

U_BOOT_PHY_DRIVER(aqr405) = {
	.name = "Aquantia AQR405",
	.uid = 0x3a1b4b2,
	.mask = 0xfffffff0,
	.features = PHY_10G_FEATURES,
	.mmds = (MDIO_MMD_PMAPMD | MDIO_MMD_PCS|
		 MDIO_MMD_PHYXS | MDIO_MMD_AN |
		 MDIO_MMD_VEND1),
	.config = &aquantia_config,
	.startup = &aquantia_startup,
	.shutdown = &gen10g_shutdown,
	.data = AQUANTIA_GEN1,
};

U_BOOT_PHY_DRIVER(aqr412) = {
	.name = "Aquantia AQR412",
	.uid = 0x3a1b710,
	.mask = 0xfffffff0,
	.features = PHY_10G_FEATURES,
	.mmds = (MDIO_MMD_PMAPMD | MDIO_MMD_PCS |
		 MDIO_MMD_PHYXS | MDIO_MMD_AN |
		 MDIO_MMD_VEND1),
	.config = &aquantia_config,
	.startup = &aquantia_startup,
	.shutdown = &gen10g_shutdown,
	.data = AQUANTIA_GEN3,
};
