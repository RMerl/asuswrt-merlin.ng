#include <stdio.h>
#include <string.h>

#include "internal.h"

/* Macros and dump functions for the 32-bit "fec" driver registers */

#define REG(_reg, _name, _val) \
	printf("0x%.03x: %-44.44s 0x%.8x\n", _reg, _name, _val)

#define FIELD(_name, _fmt, ...) \
	printf("    %-47.47s " _fmt "\n", _name, ##__VA_ARGS__)

static void fec_dump_reg_v1(int reg, u32 val)
{
	switch (reg) {
	case 0x000: /* FEC_ECNTRL */
	case 0x004: /* FEC_IEVENT */
	case 0x008: /* FEC_IMASK */
	case 0x00c: /* FEC_IVEC */
	case 0x010: /* FEC_R_DES_ACTIVE_0 */
	case 0x014: /* FEC_X_DES_ACTIVE_0 */
	case 0x040: /* FEC_MII_DATA */
	case 0x044: /* FEC_MII_SPEED */
	case 0x08c: /* FEC_R_BOUND */
	case 0x090: /* FEC_R_FSTART */
	case 0x0a4: /* FEC_X_WMRK */
	case 0x0ac: /* FEC_X_FSTART */
	case 0x104: /* FEC_R_CNTRL */
	case 0x108: /* FEC_MAX_FRM_LEN */
	case 0x144: /* FEC_X_CNTRL */
	case 0x3c0: /* FEC_ADDR_LOW */
	case 0x3c4: /* FEC_ADDR_HIGH */
	case 0x3c8: /* FEC_GRP_HASH_TABLE_HIGH */
	case 0x3cc: /* FEC_GRP_HASH_TABLE_LOW */
	case 0x3d0: /* FEC_R_DES_START_0 */
	case 0x3d4: /* FEC_X_DES_START_0 */
	case 0x3d8: /* FEC_R_BUFF_SIZE_0 */
		REG(reg, "", val);
		break;
	}
}

static void fec_dump_reg_v2(int reg, u32 val)
{
	switch (reg) {
	case 0x084: /* FEC_R_CNTRL */
		REG(reg, "RCR (Receive Control Register)", val);
		FIELD("MAX_FL (Maximum frame length)", "%u", (val & 0x07ff0000) >> 16);
		FIELD("FCE (Flow control enable)", "%u", !!(val & 0x00000020));
		FIELD("BC_REJ (Broadcast frame reject)", "%u", !!(val & 0x00000010));
		FIELD("PROM (Promiscuous mode)", "%u", !!(val & 0x00000008));
		FIELD("DRT (Disable receive on transmit)", "%u", !!(val & 0x00000002));
		FIELD("LOOP (Internal loopback)", "%u", !!(val & 0x00000001));
		break;
	case 0x0c4: /* FEC_X_CNTRL */
		REG(reg, "TCR (Transmit Control Register)", val);
		FIELD("RFC_PAUSE (Receive frame control pause)", "%u", !!(val & 0x00000010));
		FIELD("TFC_PAUSE (Transmit frame control pause)", "%u", !!(val & 0x00000008));
		FIELD("FDEN (Full duplex enable)", "%u", !!(val & 0x00000004));
		FIELD("HBC (Heartbeat control)", "%u", !!(val & 0x00000002));
		FIELD("GTS (Graceful transmit stop)", "%u", !!(val & 0x00000001));
		break;
	case 0x118: /* FEC_HASH_TABLE_HIGH */
		REG(reg, "IAUR (Individual Address Upper Register)", val);
		FIELD("IADDR1", "0x%.16llx", (u64)((u64)val) << 32);
		break;
	case 0x11c: /* FEC_HASH_TABLE_LOW */
		REG(reg, "IALR (Individual Address Lower Register)", val);
		FIELD("IADDR2", "0x%.16x", val);
		break;
	case 0x120: /* FEC_GRP_HASH_TABLE_HIGH */
		REG(reg, "GAUR (Group Address Upper Register)", val);
		FIELD("GADDR1", "0x%.16llx", (u64)((u64)val) << 32);
		break;
	case 0x124: /* FEC_GRP_HASH_TABLE_LOW */
		REG(reg, "GALR (Group Address Lower Register)", val);
		FIELD("GADDR2", "0x%.16x", val);
		break;
	case 0x144: /* FEC_X_WMRK */
		REG(reg, "TFWR (Transmit FIFO Watermark Register)", val);
		FIELD("X_WMRK", "%s",
			(val & 0x00000003) == 0x00000000 ? "64 bytes" :
			(val & 0x00000003) == 0x00000002 ? "128 bytes" :
			(val & 0x00000003) == 0x00000003 ? "192 bytes" : "?");
		break;
	case 0x14c: /* FEC_R_BOUND */
		REG(reg, "FRBR (FIFO Receive Bound Register)", val);
		FIELD("R_BOUND (Highest valid FIFO RAM address)", "0x%.2x", (val & 0x000003fc) >> 2);
		break;
	case 0x188: /* FEC_R_BUFF_SIZE_0 */
		REG(reg, "EMRBR (Maximum Receive Buffer Size)", val);
		FIELD("R_BUF_SIZE (Receive buffer size)", "%u", (val & 0x000007f0) >> 4);
		break;
	case 0x004: /* FEC_IEVENT */
	case 0x008: /* FEC_IMASK */
	case 0x010: /* FEC_R_DES_ACTIVE_0 */
	case 0x014: /* FEC_X_DES_ACTIVE_0 */
	case 0x024: /* FEC_ECNTRL */
	case 0x040: /* FEC_MII_DATA */
	case 0x044: /* FEC_MII_SPEED */
	case 0x064: /* FEC_MIB_CTRLSTAT */
	case 0x0e4: /* FEC_ADDR_LOW */
	case 0x0e8: /* FEC_ADDR_HIGH */
	case 0x0ec: /* FEC_OPD */
	case 0x0f0: /* FEC_TXIC0 */
	case 0x0f4: /* FEC_TXIC1 */
	case 0x0f8: /* FEC_TXIC2 */
	case 0x100: /* FEC_RXIC0 */
	case 0x104: /* FEC_RXIC1 */
	case 0x108: /* FEC_RXIC2 */
	case 0x150: /* FEC_R_FSTART */
	case 0x160: /* FEC_R_DES_START_1 */
	case 0x164: /* FEC_X_DES_START_1 */
	case 0x168: /* FEC_R_BUFF_SIZE_1 */
	case 0x16c: /* FEC_R_DES_START_2 */
	case 0x170: /* FEC_X_DES_START_2 */
	case 0x174: /* FEC_R_BUFF_SIZE_2 */
	case 0x180: /* FEC_R_DES_START_0 */
	case 0x184: /* FEC_X_DES_START_0 */
	case 0x190: /* FEC_R_FIFO_RSFL */
	case 0x194: /* FEC_R_FIFO_RSEM */
	case 0x198: /* FEC_R_FIFO_RAEM */
	case 0x19c: /* FEC_R_FIFO_RAFL */
	case 0x1c4: /* FEC_RACC */
	case 0x1c8: /* FEC_RCMR_1 */
	case 0x1cc: /* FEC_RCMR_2 */
	case 0x1d8: /* FEC_DMA_CFG_1 */
	case 0x1dc: /* FEC_DMA_CFG_2 */
	case 0x1e0: /* FEC_R_DES_ACTIVE_1 */
	case 0x1e4: /* FEC_X_DES_ACTIVE_1 */
	case 0x1e8: /* FEC_R_DES_ACTIVE_2 */
	case 0x1ec: /* FEC_X_DES_ACTIVE_2 */
	case 0x1f0: /* FEC_QOS_SCHEME */
	case 0x200: /* RMON_T_DROP */
	case 0x204: /* RMON_T_PACKETS */
	case 0x208: /* RMON_T_BC_PKT */
	case 0x20c: /* RMON_T_MC_PKT */
	case 0x210: /* RMON_T_CRC_ALIGN */
	case 0x214: /* RMON_T_UNDERSIZE */
	case 0x218: /* RMON_T_OVERSIZE */
	case 0x21c: /* RMON_T_FRAG */
	case 0x220: /* RMON_T_JAB */
	case 0x224: /* RMON_T_COL */
	case 0x228: /* RMON_T_P64 */
	case 0x22c: /* RMON_T_P65TO127 */
	case 0x230: /* RMON_T_P128TO255 */
	case 0x234: /* RMON_T_P256TO511 */
	case 0x238: /* RMON_T_P512TO1023 */
	case 0x23c: /* RMON_T_P1024TO2047 */
	case 0x240: /* RMON_T_P_GTE2048 */
	case 0x244: /* RMON_T_OCTETS */
	case 0x248: /* IEEE_T_DROP */
	case 0x24c: /* IEEE_T_FRAME_OK */
	case 0x250: /* IEEE_T_1COL */
	case 0x254: /* IEEE_T_MCOL */
	case 0x258: /* IEEE_T_DEF */
	case 0x25c: /* IEEE_T_LCOL */
	case 0x260: /* IEEE_T_EXCOL */
	case 0x264: /* IEEE_T_MACERR */
	case 0x268: /* IEEE_T_CSERR */
	case 0x26c: /* IEEE_T_SQE */
	case 0x270: /* IEEE_T_FDXFC */
	case 0x274: /* IEEE_T_OCTETS_OK */
	case 0x284: /* RMON_R_PACKETS */
	case 0x288: /* RMON_R_BC_PKT */
	case 0x28c: /* RMON_R_MC_PKT */
	case 0x290: /* RMON_R_CRC_ALIGN */
	case 0x294: /* RMON_R_UNDERSIZE */
	case 0x298: /* RMON_R_OVERSIZE */
	case 0x29c: /* RMON_R_FRAG */
	case 0x2a0: /* RMON_R_JAB */
	case 0x2a4: /* RMON_R_RESVD_O */
	case 0x2a8: /* RMON_R_P64 */
	case 0x2ac: /* RMON_R_P65TO127 */
	case 0x2b0: /* RMON_R_P128TO255 */
	case 0x2b4: /* RMON_R_P256TO511 */
	case 0x2b8: /* RMON_R_P512TO1023 */
	case 0x2bc: /* RMON_R_P1024TO2047 */
	case 0x2c0: /* RMON_R_P_GTE2048 */
	case 0x2c4: /* RMON_R_OCTETS */
	case 0x2c8: /* IEEE_R_DROP */
	case 0x2cc: /* IEEE_R_FRAME_OK */
	case 0x2d0: /* IEEE_R_CRC */
	case 0x2d4: /* IEEE_R_ALIGN */
	case 0x2d8: /* IEEE_R_MACERR */
	case 0x2dc: /* IEEE_R_FDXFC */
	case 0x2e0: /* IEEE_R_OCTETS_OK */
		REG(reg, "", val);
		break;
	}
}

#undef FIELD
#undef REG

int fec_dump_regs(struct ethtool_drvinfo *info __maybe_unused,
		  struct ethtool_regs *regs)
{
	const u32 *data = (u32 *)regs->data;
	unsigned int offset;
	u32 val;

	for (offset = 0; offset < regs->len; offset += 4) {
		val = data[offset / 4];

		switch (regs->version) {
		case 1:
			fec_dump_reg_v1(offset, val);
			break;
		case 2:
			fec_dump_reg_v2(offset, val);
			break;
		default:
			return 1;
		}
	}

	return 0;
}
