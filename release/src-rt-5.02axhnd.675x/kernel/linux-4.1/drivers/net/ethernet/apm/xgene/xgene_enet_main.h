/* Applied Micro X-Gene SoC Ethernet Driver
 *
 * Copyright (c) 2014, Applied Micro Circuits Corporation
 * Authors: Iyappan Subramanian <isubramanian@apm.com>
 *	    Ravi Patel <rapatel@apm.com>
 *	    Keyur Chudgar <kchudgar@apm.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __XGENE_ENET_MAIN_H__
#define __XGENE_ENET_MAIN_H__

#include <linux/acpi.h>
#include <linux/clk.h>
#include <linux/efi.h>
#include <linux/io.h>
#include <linux/of_platform.h>
#include <linux/of_net.h>
#include <linux/of_mdio.h>
#include <linux/module.h>
#include <net/ip.h>
#include <linux/prefetch.h>
#include <linux/if_vlan.h>
#include <linux/phy.h>
#include "xgene_enet_hw.h"

#define XGENE_DRV_VERSION	"v1.0"
#define XGENE_ENET_MAX_MTU	1536
#define SKB_BUFFER_SIZE		(XGENE_ENET_MAX_MTU - NET_IP_ALIGN)
#define NUM_PKT_BUF	64
#define NUM_BUFPOOL	32

#define START_CPU_BUFNUM_0	0
#define START_ETH_BUFNUM_0	2
#define START_BP_BUFNUM_0	0x22
#define START_RING_NUM_0	8
#define START_CPU_BUFNUM_1	12
#define START_ETH_BUFNUM_1	10
#define START_BP_BUFNUM_1	0x2A
#define START_RING_NUM_1	264

#define IRQ_ID_SIZE		16
#define XGENE_MAX_TXC_RINGS	1

#define PHY_POLL_LINK_ON	(10 * HZ)
#define PHY_POLL_LINK_OFF	(PHY_POLL_LINK_ON / 5)

/* software context of a descriptor ring */
struct xgene_enet_desc_ring {
	struct net_device *ndev;
	u16 id;
	u16 num;
	u16 head;
	u16 tail;
	u16 slots;
	u16 irq;
	char irq_name[IRQ_ID_SIZE];
	u32 size;
	u32 state[NUM_RING_CONFIG];
	void __iomem *cmd_base;
	void __iomem *cmd;
	dma_addr_t dma;
	u16 dst_ring_num;
	u8 nbufpool;
	struct sk_buff *(*rx_skb);
	struct sk_buff *(*cp_skb);
	enum xgene_enet_ring_cfgsize cfgsize;
	struct xgene_enet_desc_ring *cp_ring;
	struct xgene_enet_desc_ring *buf_pool;
	struct napi_struct napi;
	union {
		void *desc_addr;
		struct xgene_enet_raw_desc *raw_desc;
		struct xgene_enet_raw_desc16 *raw_desc16;
	};
};

struct xgene_mac_ops {
	void (*init)(struct xgene_enet_pdata *pdata);
	void (*reset)(struct xgene_enet_pdata *pdata);
	void (*tx_enable)(struct xgene_enet_pdata *pdata);
	void (*rx_enable)(struct xgene_enet_pdata *pdata);
	void (*tx_disable)(struct xgene_enet_pdata *pdata);
	void (*rx_disable)(struct xgene_enet_pdata *pdata);
	void (*set_mac_addr)(struct xgene_enet_pdata *pdata);
	void (*link_state)(struct work_struct *work);
};

struct xgene_port_ops {
	int (*reset)(struct xgene_enet_pdata *pdata);
	void (*cle_bypass)(struct xgene_enet_pdata *pdata,
			   u32 dst_ring_num, u16 bufpool_id);
	void (*shutdown)(struct xgene_enet_pdata *pdata);
};

/* ethernet private data */
struct xgene_enet_pdata {
	struct net_device *ndev;
	struct mii_bus *mdio_bus;
	struct phy_device *phy_dev;
	int phy_speed;
	struct clk *clk;
	struct platform_device *pdev;
	struct xgene_enet_desc_ring *tx_ring;
	struct xgene_enet_desc_ring *rx_ring;
	char *dev_name;
	u32 rx_buff_cnt;
	u32 tx_qcnt_hi;
	u32 cp_qcnt_hi;
	u32 cp_qcnt_low;
	u32 rx_irq;
	u32 txc_irq;
	u8 cq_cnt;
	void __iomem *eth_csr_addr;
	void __iomem *eth_ring_if_addr;
	void __iomem *eth_diag_csr_addr;
	void __iomem *mcx_mac_addr;
	void __iomem *mcx_mac_csr_addr;
	void __iomem *base_addr;
	void __iomem *ring_csr_addr;
	void __iomem *ring_cmd_addr;
	int phy_mode;
	enum xgene_enet_rm rm;
	struct rtnl_link_stats64 stats;
	struct xgene_mac_ops *mac_ops;
	struct xgene_port_ops *port_ops;
	struct delayed_work link_work;
	u32 port_id;
	u8 cpu_bufnum;
	u8 eth_bufnum;
	u8 bp_bufnum;
	u16 ring_num;
};

struct xgene_indirect_ctl {
	void __iomem *addr;
	void __iomem *ctl;
	void __iomem *cmd;
	void __iomem *cmd_done;
};

/* Set the specified value into a bit-field defined by its starting position
 * and length within a single u64.
 */
static inline u64 xgene_enet_set_field_value(int pos, int len, u64 val)
{
	return (val & ((1ULL << len) - 1)) << pos;
}

#define SET_VAL(field, val) \
		xgene_enet_set_field_value(field ## _POS, field ## _LEN, val)

#define SET_BIT(field) \
		xgene_enet_set_field_value(field ## _POS, 1, 1)

/* Get the value from a bit-field defined by its starting position
 * and length within the specified u64.
 */
static inline u64 xgene_enet_get_field_value(int pos, int len, u64 src)
{
	return (src >> pos) & ((1ULL << len) - 1);
}

#define GET_VAL(field, src) \
		xgene_enet_get_field_value(field ## _POS, field ## _LEN, src)

static inline struct device *ndev_to_dev(struct net_device *ndev)
{
	return ndev->dev.parent;
}

void xgene_enet_set_ethtool_ops(struct net_device *netdev);

#endif /* __XGENE_ENET_MAIN_H__ */
