/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2019-2021 Broadcom.
 */

#ifndef _BXNT_DBG_H_
#define _BXNT_DBG_H_

/* Adjust commented out lines below to enable debug. */
/* #define DEBUG_PCI */
/* #define DEBUG_MEMORY */
/* #define DEBUG_LINK */
/* #define DEBUG_CHIP */
/* #define DEBUG_FAIL */
/* #define DEBUG_HWRM_CMDS */
/* #define DEBUG_HWRM_DUMP */
/* #define DEBUG_CQ */
/* #define DEBUG_CQ_DUMP */
/* #define DEBUG_TX */
/* #define DEBUG_TX_DUMP */
/* #define DEBUG_RX */
/* #define DEBUG_RX_DUMP */

#if \
	defined(DEBUG_PCI) || \
	defined(DEBUG_MEMORY) || \
	defined(DEBUG_LINK) || \
	defined(DEBUG_CHIP) || \
	defined(DEBUG_FAIL) || \
	defined(DEBUG_HWRM_CMDS) || \
	defined(DEBUG_HWRM_DUMP) || \
	defined(DEBUG_CQ) || \
	defined(DEBUG_CQ_DUMP) || \
	defined(DEBUG_TX) || \
	defined(DEBUG_TX_DUMP) || \
	defined(DEBUG_RX) || \
	defined(DEBUG_RX_DUMP)
#define DEBUG_DEFAULT
#endif

#if defined(DEBUG_DEFAULT)
#define dbg_prn          printf
#define MAX_CHAR_SIZE(a) (u32)((1 << (a)) - 1)
#define DISP_U8          0x00
#define DISP_U16         0x01
#define DISP_U32         0x02
#define DISP_U64         0x03

void dumpmemory1(u8 *buffer, u32 length, u8 flag)
{
	u32 jj = 0;
	u8  i, c;

	printf("\n  %p:", buffer);
	for (jj = 0; jj < 16; jj++) {
		if (!(jj & MAX_CHAR_SIZE(flag)))
			printf(" ");
		if (jj < length)
			printf("%02x", buffer[jj]);
		else
			printf("  ");
		if ((jj & 0xF) == 0xF) {
			printf(" ");
			for (i = 0; i < 16; i++) {
				if (i < length) {
					c = buffer[jj + i - 15];
					if (c >= 0x20 && c < 0x7F)
						;
					else
						c = '.';
					printf("%c", c);
				}
			}
		}
	}
}

void dump_mem(u8 *buffer, u32 length, u8 flag)
{
	u32 length16, remlen, jj;

	length16 = length & 0xFFFFFFF0;
	remlen   = length & 0xF;
	for (jj = 0; jj < length16; jj += 16)
		dumpmemory1((u8 *)&buffer[jj], 16, flag);
	if (remlen)
		dumpmemory1((u8 *)&buffer[length16], remlen, flag);
	if (length16 || remlen)
		printf("\n");
}
#endif

#if defined(DEBUG_PCI)
void dbg_pci(struct bnxt *bp, const char *func, u16 cmd_reg)
{
	printf("- %s()\n", func);
	printf("  Vendor id          : %04X\n", bp->vendor_id);
	printf("  Device id          : %04X\n", bp->device_id);
	printf("  Irq                : %d\n", bp->irq);
	printf("  PCI Command Reg    : %04X  %04X\n", bp->cmd_reg, cmd_reg);
	printf("  Sub Vendor id      : %04X\n", bp->subsystem_vendor);
	printf("  Sub Device id      : %04X\n", bp->subsystem_device);
	printf("  BAR (0)            : %p\n", bp->bar0);
	printf("  BAR (1)            : %p\n", bp->bar1);
	printf("  BAR (2)            : %p\n", bp->bar2);
}
#else
#define dbg_pci(bp, func, creg)
#endif

#if defined(DEBUG_MEMORY)
void dbg_mem(struct bnxt *bp, const char *func)
{
	printf("- %s()\n", func);
	printf("  bp Addr            : %p", bp);
	printf(" Len %4d", (u16)sizeof(struct bnxt));
	printf(" phy %llx\n", virt_to_bus(bp));
	printf("  bp->hwrm_req_addr  : %p", bp->hwrm_addr_req);
	printf(" Len %4d", (u16)REQ_BUFFER_SIZE);
	printf(" phy %llx\n", bp->req_addr_mapping);
	printf("  bp->hwrm_resp_addr : %p", bp->hwrm_addr_resp);
	printf(" Len %4d", (u16)RESP_BUFFER_SIZE);
	printf(" phy %llx\n", bp->resp_addr_mapping);
	printf("  bp->tx.bd_virt     : %p", bp->tx.bd_virt);
	printf(" Len %4d", (u16)TX_RING_DMA_BUFFER_SIZE);
	printf(" phy %llx\n", virt_to_bus(bp->tx.bd_virt));
	printf("  bp->rx.bd_virt     : %p", bp->rx.bd_virt);
	printf(" Len %4d", (u16)RX_RING_DMA_BUFFER_SIZE);
	printf(" phy %llx\n", virt_to_bus(bp->rx.bd_virt));
	printf("  bp->cq.bd_virt     : %p", bp->cq.bd_virt);
	printf(" Len %4d", (u16)CQ_RING_DMA_BUFFER_SIZE);
	printf(" phy %llx\n", virt_to_bus(bp->cq.bd_virt));
}
#else
#define dbg_mem(bp, func)
#endif

#if defined(DEBUG_CHIP)
void print_fw_ver(struct hwrm_ver_get_output *resp, u32 tmo)
{
	if (resp->hwrm_intf_maj_8b < 1) {
		dbg_prn("  HWRM interface %d.%d.%d is older than 1.0.0.\n",
			resp->hwrm_intf_maj_8b, resp->hwrm_intf_min_8b,
			resp->hwrm_intf_upd_8b);
		dbg_prn("  Update FW with HWRM interface 1.0.0 or newer.\n");
	}
	dbg_prn("  FW Version         : %d.%d.%d.%d\n",
		resp->hwrm_fw_maj_8b, resp->hwrm_fw_min_8b,
		resp->hwrm_fw_bld_8b, resp->hwrm_fw_rsvd_8b);
	printf("  cmd timeout        : %d\n", tmo);
}

void dbg_func_resource_qcaps(struct bnxt *bp)
{
	/* Ring Groups */
	printf("  min_hw_ring_grps   : %d\n", bp->min_hw_ring_grps);
	printf("  max_hw_ring_grps   : %d\n", bp->max_hw_ring_grps);
	/* TX Rings */
	printf("  min_tx_rings       : %d\n", bp->min_tx_rings);
	printf("  max_tx_rings       : %d\n", bp->max_tx_rings);
	/* RX Rings */
	printf("  min_rx_rings       : %d\n", bp->min_rx_rings);
	printf("  max_rx_rings       : %d\n", bp->max_rx_rings);
	/* Completion Rings */
	printf("  min_cq_rings       : %d\n", bp->min_cp_rings);
	printf("  max_cq_rings       : %d\n", bp->max_cp_rings);
	/* Statistic Contexts */
	printf("  min_stat_ctxs      : %d\n", bp->min_stat_ctxs);
	printf("  max_stat_ctxs      : %d\n", bp->max_stat_ctxs);
}

void print_func_qcaps(struct bnxt *bp)
{
	printf("  Port Number        : %d\n", bp->port_idx);
	printf("  fid                : 0x%04x\n", bp->fid);
	dbg_prn("  PF MAC             : %02x:%02x:%02x:%02x:%02x:%02x\n",
		bp->mac_addr[0],
		bp->mac_addr[1],
		bp->mac_addr[2],
		bp->mac_addr[3],
		bp->mac_addr[4],
		bp->mac_addr[5]);
}

void print_func_qcfg(struct bnxt *bp)
{
	printf("  ordinal_value      : %d\n", bp->ordinal_value);
	printf("  stat_ctx_id        : %x\n", bp->stat_ctx_id);
	dbg_prn("  FW MAC             : %02x:%02x:%02x:%02x:%02x:%02x\n",
		bp->mac_addr[0],
		bp->mac_addr[1],
		bp->mac_addr[2],
		bp->mac_addr[3],
		bp->mac_addr[4],
		bp->mac_addr[5]);
}

void dbg_set_speed(u32 speed)
{
	u32 speed1 = ((speed & LINK_SPEED_DRV_MASK) >> LINK_SPEED_DRV_SHIFT);

	printf("  Set Link Speed     : ");
	switch (speed & LINK_SPEED_DRV_MASK) {
	case LINK_SPEED_DRV_1G:
		printf("1 GBPS");
		break;
	case LINK_SPEED_DRV_10G:
		printf("10 GBPS");
		break;
	case LINK_SPEED_DRV_25G:
		printf("25 GBPS");
		break;
	case LINK_SPEED_DRV_40G:
		printf("40 GBPS");
		break;
	case LINK_SPEED_DRV_50G:
		printf("50 GBPS");
		break;
	case LINK_SPEED_DRV_100G:
		printf("100 GBPS");
		break;
	case LINK_SPEED_DRV_AUTONEG:
		printf("AUTONEG");
		break;
	default:
		printf("%x", speed1);
		break;
	}
	printf("\n");
}

void dbg_chip_info(struct bnxt *bp)
{
	printf("  Stat Ctx ID        : %d\n", bp->stat_ctx_id);
	printf("  Grp ID             : %d\n", bp->ring_grp_id);
	printf("  CQ Ring Id         : %d\n", bp->cq_ring_id);
	printf("  Tx Ring Id         : %d\n", bp->tx_ring_id);
	printf("  Rx ring Id         : %d\n", bp->rx_ring_id);
}

void print_num_rings(struct bnxt *bp)
{
	printf("  num_cmpl_rings     : %d\n", bp->num_cmpl_rings);
	printf("  num_tx_rings       : %d\n", bp->num_tx_rings);
	printf("  num_rx_rings       : %d\n", bp->num_rx_rings);
	printf("  num_ring_grps      : %d\n", bp->num_hw_ring_grps);
	printf("  num_stat_ctxs      : %d\n", bp->num_stat_ctxs);
}

void dbg_flags(const char *func, u32 flags)
{
	printf("- %s()\n", func);
	printf("  bp->flags          : 0x%04x\n", flags);
}
#else
#define print_fw_ver(resp, tmo)
#define dbg_func_resource_qcaps(bp)
#define print_func_qcaps(bp)
#define print_func_qcfg(bp)
#define dbg_set_speed(speed)
#define dbg_chip_info(bp)
#define print_num_rings(bp)
#define dbg_flags(func, flags)
#endif

#if defined(DEBUG_HWRM_CMDS) || defined(DEBUG_FAIL)
void dump_hwrm_req(struct bnxt *bp, const char *func, u32 len, u32 tmo)
{
	dbg_prn("- %s(0x%04x) cmd_len %d cmd_tmo %d",
		func, (u16)((struct input *)bp->hwrm_addr_req)->req_type,
		len, tmo);
#if defined(DEBUG_HWRM_DUMP)
	dump_mem((u8 *)bp->hwrm_addr_req, len, DISP_U8);
#else
	printf("\n");
#endif
}

void debug_resp(struct bnxt *bp, const char *func, u32 resp_len, u16 err)
{
	dbg_prn("- %s(0x%04x) - ",
		func, (u16)((struct input *)bp->hwrm_addr_req)->req_type);
	if (err == STATUS_SUCCESS)
		printf("Done");
	else if (err != STATUS_TIMEOUT)
		printf("Fail err 0x%04x", err);
	else
		printf("timedout");
#if defined(DEBUG_HWRM_DUMP)
	if (err != STATUS_TIMEOUT)
		dump_mem((u8 *)bp->hwrm_addr_resp, resp_len, DISP_U8);
	else
		printf("\n");
#else
	printf("\n");
#endif
}

void dbg_hw_cmd(struct bnxt *bp,
		const char *func, u16 cmd_len,
		u16 resp_len, u32 cmd_tmo, u16 err)
{
#if !defined(DEBUG_HWRM_CMDS)
	if (err && err != STATUS_TIMEOUT)
#endif
	{
		dump_hwrm_req(bp, func, cmd_len, cmd_tmo);
		debug_resp(bp, func, resp_len, err);
	}
}
#else
#define dbg_hw_cmd(bp, func, cmd_len, resp_len, cmd_tmo, err)
#endif

#if defined(DEBUG_HWRM_CMDS)
void dbg_short_cmd(u8 *req, const char *func, u32 len)
{
	struct hwrm_short_input *sreq;

	sreq = (struct hwrm_short_input *)req;
	dbg_prn("- %s(0x%04x) short_cmd_len %d",
		func,
		sreq->req_type,
		(int)len);
#if defined(DEBUG_HWRM_DUMP)
	dump_mem((u8 *)sreq, len, DISP_U8);
#else
	printf("\n");
#endif
}
#else
#define dbg_short_cmd(sreq, func, len)
#endif

#if defined(DEBUG_RX)
void dump_rx_bd(struct rx_pkt_cmpl *rx_cmp,
		struct rx_pkt_cmpl_hi *rx_cmp_hi,
		u32 desc_idx)
{
	printf("  RX desc_idx %d\n", desc_idx);
	printf("- rx_cmp    %llx", virt_to_bus(rx_cmp));
#if defined(DEBUG_RX_DUMP)
	dump_mem((u8 *)rx_cmp, (u32)sizeof(struct rx_pkt_cmpl), DISP_U8);
#else
	printf("\n");
#endif
	printf("- rx_cmp_hi %llx", virt_to_bus(rx_cmp_hi));
#if defined(DEBUG_RX_DUMP)
	dump_mem((u8 *)rx_cmp_hi, (u32)sizeof(struct rx_pkt_cmpl_hi), DISP_U8);
#else
	printf("\n");
#endif
}

void dbg_rxp(u8 *iob, u16 rx_len, u16 flag)
{
	printf("- RX iob %llx Len %d ", virt_to_bus(iob), rx_len);
	if (flag == PKT_RECEIVED)
		printf(" PKT RECEIVED");
	else if (flag == PKT_DROPPED)
		printf(" PKT DROPPED");
#if defined(DEBUG_RX_DUMP)
	dump_mem(iob, (u32)rx_len, DISP_U8);
#else
	printf("\n");
#endif
}

void dbg_rx_cid(u16 idx, u16 cid)
{
	dbg_prn("- RX old cid %d new cid %d\n", idx, cid);
}

void dbg_rx_alloc_iob_fail(u16 idx, u16 cid)
{
	dbg_prn("  Rx alloc_iob (%d) failed", idx);
	dbg_prn(" for cons_id %d\n", cid);
}

void dbg_rx_iob(void *iob, u16 idx, u16 cid)
{
	dbg_prn("  Rx alloc_iob (%d) %p bd_virt (%d)\n",
		idx, iob, cid);
}

void dbg_rx_pkt(struct bnxt *bp, const char *func, uchar *pkt, int len)
{
	if (bp->rx.iob_recv == PKT_RECEIVED) {
		dbg_prn("- %s: %llx %d\n", func,
			virt_to_bus(pkt), len);
	}
}
#else
#define dump_rx_bd(rx_cmp, rx_cmp_hi, desc_idx)
#define dbg_rxp(iob, rx_len, flag)
#define dbg_rx_cid(idx, cid)
#define dbg_rx_alloc_iob_fail(idx, cid)
#define dbg_rx_iob(iob, idx, cid)
#define dbg_rx_pkt(bp, func, pkt, len)
#endif

#if defined(DEBUG_CQ)
void dump_CQ(struct cmpl_base *cmp, u16 cons_idx)
{
	printf("- CQ Type ");

	switch (cmp->type & CMPL_BASE_TYPE_MASK) {
	case CMPL_BASE_TYPE_STAT_EJECT:
		printf("(se)");
		break;
	case CMPL_BASE_TYPE_HWRM_ASYNC_EVENT:
		printf("(ae)");
		break;
	case CMPL_BASE_TYPE_TX_L2:
		printf("(tx)");
		break;
	case CMPL_BASE_TYPE_RX_L2:
		printf("(rx)");
		break;
	default:
		printf("%04x", (u16)(cmp->type & CMPL_BASE_TYPE_MASK));
		break;
	}
	printf(" cid %d", cons_idx);
#if defined(DEBUG_CQ_DUMP)
	dump_mem((u8 *)cmp, (u32)sizeof(struct cmpl_base), DISP_U8);
#else
	printf("\n");
#endif
}
#else
#define dump_CQ(cq, id)
#endif

#if defined(DEBUG_TX)
void dump_tx_stat(struct bnxt *bp)
{
	printf("  TX stats cnt %d req_cnt %d", bp->tx.cnt, bp->tx.cnt_req);
	printf(" prod_id %d cons_id %d\n", bp->tx.prod_id, bp->tx.cons_id);
}

void dump_tx_pkt(void *packet, dma_addr_t mapping, int len)
{
	printf("  TX Addr %llx Size %d", mapping, len);
#if defined(DEBUG_TX_DUMP)
	dump_mem((u8 *)packet, len, DISP_U8);
#else
	printf("\n");
#endif
}

void dump_tx_bd(struct tx_bd_short *tx_bd, u16 len)
{
	printf("  Tx BD Addr %llx Size %d", virt_to_bus(tx_bd), len);
#if defined(DEBUG_TX_DUMP)
	dump_mem((u8 *)tx_bd, (u32)len, DISP_U8);
#else
	printf("\n");
#endif
}

void dbg_no_tx_bd(void)
{
	printf("  Tx ring full\n");
}
#else
#define dump_tx_stat(bp)
#define dump_tx_pkt(packet, mapping, len)
#define dump_tx_bd(prod_bd, len)
#define dbg_no_tx_bd()
#endif

#if defined(DEBUG_MEMORY)
void dbg_mem_free_done(const char *func)
{
	printf("- %s - Done\n", func);
}
#else
#define dbg_mem_free_done(func)
#endif

#if defined(DEBUG_FAIL)
void dbg_mem_alloc_fail(const char *func)
{
	printf("- %s() Fail\n", func);
}
#else
#define dbg_mem_alloc_fail(func)
#endif

#if defined(DEBUG_LINK)
static void dump_evt(u8 *cmp, u32 type, u16 cid)
{
	u32 size = sizeof(struct cmpl_base);
	u8  c = 'C';

	switch (type) {
	case CMPL_BASE_TYPE_HWRM_ASYNC_EVENT:
		break;
	default:
		return;
	}
	dbg_prn("- %cQ Type (ae)  cid %d", c, cid);
	dump_mem(cmp, size, DISP_U8);
}

void dbg_link_status(struct bnxt *bp)
{
	dbg_prn("  Port(%d)            : Link", bp->port_idx);
	if (bp->link_status == STATUS_LINK_ACTIVE) {
		dbg_prn("Up");
	} else {
		dbg_prn("Down\n");
		dbg_prn("  media_detect       : %x", bp->media_detect);
	}
	dbg_prn("\n");
}

void dbg_link_state(struct bnxt *bp, u32 tmo)
{
	if (bp->link_status == STATUS_LINK_ACTIVE)
		printf("  Link wait time     : %d ms\n", tmo);
}

void dbg_phy_speed(struct bnxt *bp, char *name)
{
	printf("  Current Speed      : %s\n", name);
}
#else
#define dump_evt(cmp, ty, cid)
#define dbg_link_status(bp)
#define dbg_link_state(bp, tmo)
#define dbg_phy_speed(bp, name)
#endif

#endif /* _BXNT_DBG_H_ */
