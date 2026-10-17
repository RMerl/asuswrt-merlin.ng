// SPDX-License-Identifier: GPL-2.0
/*
 * mtu3_dr.c - dual role switch and host glue layer
 *
 * Copyright (C) 2016 MediaTek Inc.
 *
 * Author: Chunfeng Yun <chunfeng.yun@mediatek.com>
 */

#include <dm/lists.h>
#include <linux/iopoll.h>

#include "mtu3.h"
#include "mtu3_dr.h"

static void host_ports_num_get(struct mtu3_host *u3h)
{
	u32 xhci_cap;

	xhci_cap = mtu3_readl(u3h->ippc_base, U3D_SSUSB_IP_XHCI_CAP);
	u3h->u2_ports = SSUSB_IP_XHCI_U2_PORT_NUM(xhci_cap);
	u3h->u3_ports = SSUSB_IP_XHCI_U3_PORT_NUM(xhci_cap);

	dev_dbg(u3h->dev, "host - u2_ports:%d, u3_ports:%d\n",
		u3h->u2_ports, u3h->u3_ports);
}

/* only configure ports will be used later */
static int ssusb_host_enable(struct mtu3_host *u3h)
{
	void __iomem *ibase = u3h->ippc_base;
	int num_u3p = u3h->u3_ports;
	int num_u2p = u3h->u2_ports;
	int u3_ports_disabed;
	u32 check_clk;
	u32 value;
	int i;

	/* power on host ip */
	mtu3_clrbits(ibase, U3D_SSUSB_IP_PW_CTRL1, SSUSB_IP_HOST_PDN);

	/* power on and enable u3 ports except skipped ones */
	u3_ports_disabed = 0;
	for (i = 0; i < num_u3p; i++) {
		if ((0x1 << i) & u3h->u3p_dis_msk) {
			u3_ports_disabed++;
			continue;
		}

		value = mtu3_readl(ibase, SSUSB_U3_CTRL(i));
		value &= ~(SSUSB_U3_PORT_PDN | SSUSB_U3_PORT_DIS);
		value |= SSUSB_U3_PORT_HOST_SEL;
		mtu3_writel(ibase, SSUSB_U3_CTRL(i), value);
	}

	/* power on and enable all u2 ports */
	for (i = 0; i < num_u2p; i++) {
		value = mtu3_readl(ibase, SSUSB_U2_CTRL(i));
		value &= ~(SSUSB_U2_PORT_PDN | SSUSB_U2_PORT_DIS);
		value |= SSUSB_U2_PORT_HOST_SEL;
		mtu3_writel(ibase, SSUSB_U2_CTRL(i), value);
	}

	check_clk = SSUSB_XHCI_RST_B_STS;
	if (num_u3p > u3_ports_disabed)
		check_clk = SSUSB_U3_MAC_RST_B_STS;

	return ssusb_check_clocks(u3h->ssusb, check_clk);
}

static void ssusb_host_disable(struct mtu3_host *u3h)
{
	void __iomem *ibase = u3h->ippc_base;
	int num_u3p = u3h->u3_ports;
	int num_u2p = u3h->u2_ports;
	u32 value;
	int i;

	/* power down and disable u3 ports except skipped ones */
	for (i = 0; i < num_u3p; i++) {
		if ((0x1 << i) & u3h->u3p_dis_msk)
			continue;

		value = mtu3_readl(ibase, SSUSB_U3_CTRL(i));
		value |= SSUSB_U3_PORT_PDN | SSUSB_U3_PORT_DIS;
		mtu3_writel(ibase, SSUSB_U3_CTRL(i), value);
	}

	/* power down and disable all u2 ports */
	for (i = 0; i < num_u2p; i++) {
		value = mtu3_readl(ibase, SSUSB_U2_CTRL(i));
		value |= SSUSB_U2_PORT_PDN | SSUSB_U2_PORT_DIS;
		mtu3_writel(ibase, SSUSB_U2_CTRL(i), value);
	}

	/* power down host ip */
	mtu3_setbits(ibase, U3D_SSUSB_IP_PW_CTRL1, SSUSB_IP_HOST_PDN);
}

/*
 * If host supports multiple ports, the VBUSes(5V) of ports except port0
 * which supports OTG are better to be enabled by default in DTS.
 * Because the host driver will keep link with devices attached when system
 * enters suspend mode, so no need to control VBUSes after initialization.
 */
int ssusb_host_init(struct ssusb_mtk *ssusb)
{
	struct mtu3_host *u3h = ssusb->u3h;
	struct udevice *dev = u3h->dev;
	int ret;

	u3h->ssusb = ssusb;
	u3h->hcd = ssusb->mac_base;
	u3h->ippc_base = ssusb->ippc_base;

	/* optional property, ignore the error */
	dev_read_u32(dev, "mediatek,u3p-dis-msk", &u3h->u3p_dis_msk);

	host_ports_num_get(u3h);
	ret = ssusb_host_enable(u3h);
	if (ret)
		return ret;

	ssusb_set_force_mode(ssusb, MTU3_DR_FORCE_HOST);

	ret = regulator_set_enable(ssusb->vbus_supply, true);
	if (ret < 0 && ret != -ENOSYS) {
		dev_err(dev, "failed to enable vbus %d!\n", ret);
		return ret;
	}

	dev_info(dev, "%s done...\n", __func__);

	return 0;
}

void ssusb_host_exit(struct ssusb_mtk *ssusb)
{
	regulator_set_enable(ssusb->vbus_supply, false);
	ssusb_host_disable(ssusb->u3h);
}
