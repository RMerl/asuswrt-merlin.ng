// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2019-2022 Intel Corporation <www.intel.com>
 *
 */
#include <dm.h>
#include <hang.h>
#include <wait_bit.h>

#include <asm/io.h>
#include <linux/bitops.h>

/* Directory */
#define DIRUSFER		0x80010
#define DIRUCASER0		0x80040
#define DIRUSFMCR		0x80080
#define DIRUSFMAR		0x80084

#define DIRUSFMCR_SFID_SHIFT	16

/* Coherent cache agent interface */
#define CAIUIDR			0x00ffc

#define CAIUIDR_CA_GET(v)	(((v) & 0x00008000) >> 15)
#define CAIUIDR_TYPE_GET(v)	(((v) & 0x000f0000) >> 16)
#define CAIUIDR_TYPE_ACE_CAI_DVM_SUPPORT	0
#define CAIUIDR_TYPE_ACELITE_CAI_DVM_SUPPORT	1

/* Coherent subsystem */
#define CSADSER0		0xff040
#define CSUIDR			0xffff8
#define CSIDR			0xffffc

#define CSUIDR_NUMCAIUS_GET(v)	(((v) & 0x0000007f) >> 0)
#define CSUIDR_NUMDIRUS_GET(v)	(((v) & 0x003f0000) >> 16)
#define CSUIDR_NUMCMIUS_GET(v)	(((v) & 0x3f000000) >> 24)

#define CSIDR_NUMSFS_GET(v)	(((v) & 0x007c0000) >> 18)

#define DIR_REG_SZ		0x1000
#define CAIU_REG_SZ		0x1000

#define CCU_DIR_REG_ADDR(base, reg, dir)	\
		((base) + (reg) + ((dir) * DIR_REG_SZ))

/* OCRAM firewall register */
#define OCRAM_FW_01			0x100204
#define OCRAM_SECURE_REGIONS		4

#define OCRAM_PRIVILEGED_MASK		BIT(29)
#define OCRAM_SECURE_MASK		BIT(30)

static void ncore_ccu_init_dirs(void __iomem *base)
{
	ulong i, f;
	int ret;
	u32 num_of_dirs;
	u32 num_of_snoop_filters;
	u32 reg;

	num_of_dirs = CSUIDR_NUMDIRUS_GET(readl(base + CSUIDR));
	num_of_snoop_filters =
		CSIDR_NUMSFS_GET(readl(base + CSIDR)) + 1;

	/* Initialize each snoop filter in each directory */
	for (f = 0; f < num_of_snoop_filters; f++) {
		reg = f << DIRUSFMCR_SFID_SHIFT;
		for (i = 0; i < num_of_dirs; i++) {
			/* Initialize all entries */
			writel(reg, CCU_DIR_REG_ADDR(base, DIRUSFMCR, i));

			/* Poll snoop filter maintenance operation active
			 * bit become 0.
			 */
			ret = wait_for_bit_le32((const void *)
						CCU_DIR_REG_ADDR(base,
								 DIRUSFMAR, i),
						BIT(0), false, 1000, false);
			if (ret) {
				puts("CCU: Directory initialization failed!\n");
				hang();
			}

			/* Disable snoop filter, a bit per snoop filter */
			clrbits_le32((ulong)CCU_DIR_REG_ADDR(base, DIRUSFER, i),
				     BIT(f));
		}
	}
}

static void ncore_ccu_init_coh_agent(void __iomem *base)
{
	u32 num_of_coh_agent_intf;
	u32 num_of_dirs;
	u32 reg;
	u32 type;
	u32 i, dir;

	num_of_coh_agent_intf =
		CSUIDR_NUMCAIUS_GET(readl(base + CSUIDR));
	num_of_dirs = CSUIDR_NUMDIRUS_GET(readl(base + CSUIDR));

	for (i = 0; i < num_of_coh_agent_intf; i++) {
		reg = readl(base + CAIUIDR + (i * CAIU_REG_SZ));
		if (CAIUIDR_CA_GET(reg)) {
			/* Caching agent bit is enabled, enable caching agent
			 * snoop in each directory
			 */
			for (dir = 0; dir < num_of_dirs; dir++) {
				setbits_le32((ulong)
					     CCU_DIR_REG_ADDR(base, DIRUCASER0,
							      dir),
					     BIT(i));
			}
		}

		type = CAIUIDR_TYPE_GET(reg);
		if (type == CAIUIDR_TYPE_ACE_CAI_DVM_SUPPORT ||
		    type == CAIUIDR_TYPE_ACELITE_CAI_DVM_SUPPORT) {
			/* DVM support is enabled, enable ACE DVM snoop*/
			setbits_le32((ulong)(base + CSADSER0),
				     BIT(i));
		}
	}
}

static void ocram_bypass_firewall(void __iomem *base)
{
	int i;

	for (i = 0; i < OCRAM_SECURE_REGIONS; i++) {
		clrbits_le32(base + OCRAM_FW_01 + (i * sizeof(u32)),
			     OCRAM_PRIVILEGED_MASK | OCRAM_SECURE_MASK);
	}
}

static int ncore_ccu_probe(struct udevice *dev)
{
	void __iomem *base;
	fdt_addr_t addr;

	addr = dev_read_addr(dev);
	if (addr == FDT_ADDR_T_NONE)
		return -EINVAL;

	base = (void __iomem *)addr;

	ncore_ccu_init_dirs(base);
	ncore_ccu_init_coh_agent(base);
	ocram_bypass_firewall(base);

	return 0;
}

static const struct udevice_id ncore_ccu_ids[] = {
	{ .compatible = "arteris,ncore-ccu" },
	{}
};

U_BOOT_DRIVER(ncore_ccu) = {
	.name   = "ncore_ccu",
	.id     = UCLASS_CACHE,
	.of_match = ncore_ccu_ids,
	.probe	= ncore_ccu_probe,
	.flags  = DM_FLAG_PRE_RELOC,
};
