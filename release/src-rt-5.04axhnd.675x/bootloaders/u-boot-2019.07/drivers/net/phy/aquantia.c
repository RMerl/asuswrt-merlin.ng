// SPDX-License-Identifier: GPL-2.0+
/*
 * Aquantia PHY drivers
 *
 * Copyright 2014 Freescale Semiconductor, Inc.
 * Copyright 2018 NXP
 */
#include <config.h>
#include <common.h>
#include <dm.h>
#include <phy.h>
#include <u-boot/crc.h>
#include <malloc.h>
#include <asm/byteorder.h>
#include <fs.h>

#define AQUNTIA_10G_CTL		0x20
#define AQUNTIA_VENDOR_P1	0xc400

#define AQUNTIA_SPEED_LSB_MASK	0x2000
#define AQUNTIA_SPEED_MSB_MASK	0x40

#define AQUANTIA_SYSTEM_INTERFACE_SR     0xe812
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
#define GLOBAL_FIRMWARE_ID 0x20
#define GLOBAL_FAULT 0xc850
#define GLOBAL_RSTATUS_1 0xc885

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

/* addresses of memory segments in the phy */
#define DRAM_BASE_ADDR 0x3FFE0000
#define IRAM_BASE_ADDR 0x40000000

/* firmware image format constants */
#define VERSION_STRING_SIZE 0x40
#define VERSION_STRING_OFFSET 0x0200
#define HEADER_OFFSET 0x300

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
	debug("Loading Acquantia microcode from %s %s\n",
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
	debug("Found Acquantia microcode.\n");

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
	printf("%s loading firmare version '%s'\n", phydev->dev->name, version);

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

	printf("%s firmare loading done.\n", phydev->dev->name);
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

int aquantia_config(struct phy_device *phydev)
{
	u32 val, id, rstatus, fault;
	u32 reg_val1 = 0;

	id = phy_read(phydev, MDIO_MMD_VEND1, GLOBAL_FIRMWARE_ID);
	rstatus = phy_read(phydev, MDIO_MMD_VEND1, GLOBAL_RSTATUS_1);
	fault = phy_read(phydev, MDIO_MMD_VEND1, GLOBAL_FAULT);

	if (id != 0)
		printf("%s running firmware version %X.%X.%X\n",
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

	val = phy_read(phydev, MDIO_MMD_PMAPMD, MII_BMCR);

	if (phydev->interface == PHY_INTERFACE_MODE_SGMII) {
		/* 1000BASE-T mode */
		phydev->advertising = SUPPORTED_1000baseT_Full;
		phydev->supported = phydev->advertising;

		val = (val & ~AQUNTIA_SPEED_LSB_MASK) | AQUNTIA_SPEED_MSB_MASK;
		phy_write(phydev, MDIO_MMD_PMAPMD, MII_BMCR, val);
	} else if (phydev->interface == PHY_INTERFACE_MODE_XGMII) {
		/* 10GBASE-T mode */
		phydev->advertising = SUPPORTED_10000baseT_Full;
		phydev->supported = phydev->advertising;

		if (!(val & AQUNTIA_SPEED_LSB_MASK) ||
		    !(val & AQUNTIA_SPEED_MSB_MASK))
			phy_write(phydev, MDIO_MMD_PMAPMD, MII_BMCR,
				  AQUNTIA_SPEED_LSB_MASK |
				  AQUNTIA_SPEED_MSB_MASK);

		val = phy_read(phydev, MDIO_MMD_PHYXS,
			       AQUANTIA_SYSTEM_INTERFACE_SR);
		/* If SI is USXGMII then start USXGMII autoneg */
		if ((val & AQUANTIA_SI_IN_USE_MASK) == AQUANTIA_SI_USXGMII) {
			reg_val1 =  phy_read(phydev, MDIO_MMD_PHYXS,
					     AQUANTIA_VENDOR_PROVISIONING_REG);

			reg_val1 |= AQUANTIA_USX_AUTONEG_CONTROL_ENA;

			phy_write(phydev, MDIO_MMD_PHYXS,
				  AQUANTIA_VENDOR_PROVISIONING_REG,
				  reg_val1);
			printf("%s: system interface USXGMII\n",
			       phydev->dev->name);
		} else {
			printf("%s: system interface XFI\n",
			       phydev->dev->name);
		}

	} else if (phydev->interface == PHY_INTERFACE_MODE_SGMII_2500) {
		/* 2.5GBASE-T mode */
		phydev->advertising = SUPPORTED_1000baseT_Full;
		phydev->supported = phydev->advertising;

		phy_write(phydev, MDIO_MMD_AN, AQUNTIA_10G_CTL, 1);
		phy_write(phydev, MDIO_MMD_AN, AQUNTIA_VENDOR_P1, 0x9440);
	} else if (phydev->interface == PHY_INTERFACE_MODE_MII) {
		/* 100BASE-TX mode */
		phydev->advertising = SUPPORTED_100baseT_Full;
		phydev->supported = phydev->advertising;

		val = (val & ~AQUNTIA_SPEED_MSB_MASK) | AQUNTIA_SPEED_LSB_MASK;
		phy_write(phydev, MDIO_MMD_PMAPMD, MII_BMCR, val);
	}

	val = phy_read(phydev, MDIO_MMD_VEND1, AQUANTIA_RESERVED_STATUS);
	reg_val1 = phy_read(phydev, MDIO_MMD_VEND1, AQUANTIA_FIRMWARE_ID);

	printf("%s: %s Firmware Version %x.%x.%x\n", phydev->dev->name,
	       phydev->drv->name,
	       (reg_val1 & AQUANTIA_FIRMWARE_MAJOR_MASK) >> 8,
	       reg_val1 & AQUANTIA_FIRMWARE_MINOR_MASK,
	       (val & AQUANTIA_FIRMWARE_BUILD_MASK) >> 4);

	return 0;
}

int aquantia_startup(struct phy_device *phydev)
{
	u32 reg, speed;
	int i = 0;

	phydev->duplex = DUPLEX_FULL;

	/* if the AN is still in progress, wait till timeout. */
	phy_read(phydev, MDIO_MMD_AN, MDIO_STAT1);
	reg = phy_read(phydev, MDIO_MMD_AN, MDIO_STAT1);
	if (!(reg & MDIO_AN_STAT1_COMPLETE)) {
		printf("%s Waiting for PHY auto negotiation to complete",
		       phydev->dev->name);
		do {
			udelay(1000);
			reg = phy_read(phydev, MDIO_MMD_AN, MDIO_STAT1);
			if ((i++ % 500) == 0)
				printf(".");
		} while (!(reg & MDIO_AN_STAT1_COMPLETE) &&
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

struct phy_driver aq1202_driver = {
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

struct phy_driver aq2104_driver = {
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

struct phy_driver aqr105_driver = {
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
};

struct phy_driver aqr106_driver = {
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

struct phy_driver aqr107_driver = {
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
};

struct phy_driver aqr405_driver = {
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
};

int phy_aquantia_init(void)
{
	phy_register(&aq1202_driver);
	phy_register(&aq2104_driver);
	phy_register(&aqr105_driver);
	phy_register(&aqr106_driver);
	phy_register(&aqr107_driver);
	phy_register(&aqr405_driver);

	return 0;
}
