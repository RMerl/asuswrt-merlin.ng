/* Copyright (c) 2020 Intel Corporation */
#include <stdio.h>
#include "internal.h"

#define RAH_RAH					0x0000FFFF
#define RAH_ASEL				0x00010000
#define RAH_QSEL				0x000C0000
#define RAH_QSEL_EN				0x10000000
#define RAH_AV					0x80000000
#define RCTL_RXEN				0x00000002
#define RCTL_SBP				0x00000004
#define RCTL_UPE				0x00000008
#define RCTL_MPE				0x00000010
#define RCTL_LPE				0x00000020
#define RCTL_LBM				0x000000C0
#define RCTL_LBM_PHY				0x00000000
#define RCTL_LBM_MAC				0x00000040
#define RCTL_HSEL				0x00000300
#define RCTL_HSEL_MULTICAST			0x00000000
#define RCTL_HSEL_UNICAST			0x00000100
#define RCTL_HSEL_BOTH				0x00000200
#define RCTL_MO					0x00003000
#define RCTL_MO_47_36				0x00000000
#define RCTL_MO_43_32				0x00001000
#define RCTL_MO_39_28				0x00002000
#define RCTL_BAM				0x00008000
#define RCTL_BSIZE				0x00030000
#define RCTL_BSIZE_2048				0x00000000
#define RCTL_BSIZE_1024				0x00010000
#define RCTL_BSIZE_512				0x00020000
#define RCTL_VFE				0x00040000
#define RCTL_CFIEN				0x00080000
#define RCTL_CFI				0x00100000
#define RCTL_PSP				0x00200000
#define RCTL_DPF				0x00400000
#define RCTL_PMCF				0x00800000
#define RCTL_SECRC				0x04000000
#define VLANPQF_VP0QSEL				0x00000003
#define VLANPQF_VP0PBSEL			0x00000004
#define VLANPQF_VLANP0V				0x00000008
#define VLANPQF_VP1QSEL				0x00000030
#define VLANPQF_VP1PBSEL			0x00000040
#define VLANPQF_VLANP1V				0x00000080
#define VLANPQF_VP2QSEL				0x00000300
#define VLANPQF_VP2PBSEL			0x00000400
#define VLANPQF_VLANP2V				0x00000800
#define VLANPQF_VP3QSEL				0x00003000
#define VLANPQF_VP3PBSEL			0x00004000
#define VLANPQF_VLANP3V				0x00008000
#define VLANPQF_VP4QSEL				0x00030000
#define VLANPQF_VP4PBSEL			0x00040000
#define VLANPQF_VLANP4V				0x00080000
#define VLANPQF_VP5QSEL				0x00300000
#define VLANPQF_VP5PBSEL			0x00400000
#define VLANPQF_VLANP5V				0x00800000
#define VLANPQF_VP6QSEL				0x03000000
#define VLANPQF_VP6PBSEL			0x04000000
#define VLANPQF_VLANP6V				0x08000000
#define VLANPQF_VP7QSEL				0x30000000
#define VLANPQF_VP7PBSEL			0x40000000
#define VLANPQF_VLANP7V				0x80000000
#define ETQF_ETYPE				0x0000FFFF
#define ETQF_QUEUE				0x00070000
#define ETQF_ETYPE_LEN				0x01F00000
#define ETQF_ETYPE_LEN_EN			0x02000000
#define ETQF_FILTER_EN				0x04000000
#define ETQF_IMMEDIATE_INTR			0x20000000
#define ETQF_1588_TIMESTAMP			0x40000000
#define ETQF_QUEUE_EN				0x80000000

#define RAH_QSEL_SHIFT				18
#define VLANPQF_VP1QSEL_SHIFT			4
#define VLANPQF_VP2QSEL_SHIFT			8
#define VLANPQF_VP3QSEL_SHIFT			12
#define VLANPQF_VP4QSEL_SHIFT			16
#define VLANPQF_VP5QSEL_SHIFT			20
#define VLANPQF_VP6QSEL_SHIFT			24
#define VLANPQF_VP7QSEL_SHIFT			28
#define ETQF_QUEUE_SHIFT			16
#define ETQF_ETYPE_LEN_SHIFT			20

static const char *bit_to_boolean(u32 val)
{
	return val ? "yes" : "no";
}

static const char *bit_to_enable(u32 val)
{
	return val ? "enabled" : "disabled";
}

static const char *bit_to_prio(u32 val)
{
	return val ? "low" : "high";
}

int igc_dump_regs(struct ethtool_drvinfo *info __maybe_unused,
		  struct ethtool_regs *regs)
{
	u32 reg;
	int offset, i;
	u32 *regs_buff = (u32 *)regs->data;
	u8 version = (u8)(regs->version >> 24);

	if (version != 2)
		return -1;

	for (offset = 0; offset < 24; offset++) {
		reg = regs_buff[offset];
		printf("%04d: 0x%08X\n", offset, reg);
	}

	offset = 24;

	reg = regs_buff[offset];
	printf("%04d: RCTL (Receive Control Register)              \n"
	       "    Receiver:                                    %s\n"
	       "    Stop Bad Packets:                            %s\n"
	       "    Unicast Promiscuous:                         %s\n"
	       "    Multicast Promiscuous:                       %s\n"
	       "    Long Packet Reception:                       %s\n"
	       "    Loopback Model:                              %s\n"
	       "    Hash Select for MTA:                         %s\n"
	       "    Multicast/Unicast Table Offset:              %s\n"
	       "    Broadcast Accept Mode:                       %s\n"
	       "    Receive Buffer Size:                         %s\n"
	       "    VLAN Filter:                                 %s\n"
	       "    Canonical Form Indicator:                    %s\n"
	       "    Canonical Form Indicator Bit:                %s\n"
	       "    Pad Small Receive Packets:                   %s\n"
	       "    Discard Pause Frames:                        %s\n"
	       "    Pass MAC Control Frames:                     %s\n"
	       "    Strip Ethernet CRC:                          %s\n",
	       offset,
	       bit_to_enable(reg & RCTL_RXEN),
	       bit_to_enable(reg & RCTL_SBP),
	       bit_to_enable(reg & RCTL_UPE),
	       bit_to_enable(reg & RCTL_MPE),
	       bit_to_enable(reg & RCTL_LPE),
	       (reg & RCTL_LBM) == RCTL_LBM_PHY ? "PHY" :
	       (reg & RCTL_LBM) == RCTL_LBM_MAC ? "MAC" :
	       "undefined",
	       (reg & RCTL_HSEL) == RCTL_HSEL_MULTICAST ? "multicast only" :
	       (reg & RCTL_HSEL) == RCTL_HSEL_UNICAST ? "unicast only" :
	       (reg & RCTL_HSEL) == RCTL_HSEL_BOTH ? "multicast and unicast" :
	       "reserved",
	       (reg & RCTL_MO) == RCTL_MO_47_36 ? "bits [47:36]" :
	       (reg & RCTL_MO) == RCTL_MO_43_32 ? "bits [43:32]" :
	       (reg & RCTL_MO) == RCTL_MO_39_28 ? "bits [39:28]" :
	       "bits [35:24]",
	       bit_to_enable(reg & RCTL_BAM),
	       (reg & RCTL_BSIZE) == RCTL_BSIZE_2048 ? "2048 bytes" :
	       (reg & RCTL_BSIZE) == RCTL_BSIZE_1024 ? "1024 bytes" :
	       (reg & RCTL_BSIZE) == RCTL_BSIZE_512 ? "512 bytes" :
	       "256 bytes",
	       bit_to_enable(reg & RCTL_VFE),
	       bit_to_enable(reg & RCTL_CFIEN),
	       reg & RCTL_CFI ? "discarded" : "accepted",
	       bit_to_enable(reg & RCTL_PSP),
	       bit_to_enable(reg & RCTL_DPF),
	       bit_to_enable(reg & RCTL_PMCF),
	       bit_to_enable(reg & RCTL_SECRC));

	for (offset = 25; offset < 172; offset++) {
		reg = regs_buff[offset];
		printf("%04d: 0x%08X\n", offset, reg);
	}

	offset = 172;

	for (i = 0; i < 16; i++) {
		reg = regs_buff[offset + i];
		printf("%04d: RAL (Receive Address Low %02d)               \n"
		       "    Receive Address Low:                       %08X\n",
		       offset + i, i,
		       reg);
	}

	offset = 188;

	for (i = 0; i < 16; i++) {
		reg = regs_buff[offset + i];
		printf("%04d: RAH (Receive Address High %02d)              \n"
		       "    Receive Address High:                      %04X\n"
		       "    Address Select:                            %s\n"
		       "    Queue Select:                              %d\n"
		       "    Queue Select Enable:                       %s\n"
		       "    Address Valid:                             %s\n",
		       offset + i, i,
		       reg & RAH_RAH,
		       reg & RAH_ASEL ? "source" : "destination",
		       (reg & RAH_QSEL) >> RAH_QSEL_SHIFT,
		       bit_to_boolean(reg & RAH_QSEL_EN),
		       bit_to_boolean(reg & RAH_AV));
	}

	offset = 204;

	reg = regs_buff[offset];
	printf("%04d: VLANPQF (VLAN Priority Queue Filter)       \n"
	       "    Priority 0                                   \n"
	       "        Queue:                                 %d\n"
	       "        Packet Buffer:                         %s\n"
	       "        Valid:                                 %s\n"
	       "    Priority 1                                   \n"
	       "        Queue:                                 %d\n"
	       "        Packet Buffer:                         %s\n"
	       "        Valid:                                 %s\n"
	       "    Priority 2                                   \n"
	       "        Queue:                                 %d\n"
	       "        Packet Buffer:                         %s\n"
	       "        Valid:                                 %s\n"
	       "    Priority 3                                   \n"
	       "        Queue:                                 %d\n"
	       "        Packet Buffer:                         %s\n"
	       "        Valid:                                 %s\n"
	       "    Priority 4                                   \n"
	       "        Queue:                                 %d\n"
	       "        Packet Buffer:                         %s\n"
	       "        Valid:                                 %s\n"
	       "    Priority 5                                   \n"
	       "        Queue:                                 %d\n"
	       "        Packet Buffer:                         %s\n"
	       "        Valid:                                 %s\n"
	       "    Priority 6                                   \n"
	       "        Queue:                                 %d\n"
	       "        Packet Buffer:                         %s\n"
	       "        Valid:                                 %s\n"
	       "    Priority 7                                   \n"
	       "        Queue:                                 %d\n"
	       "        Packet Buffer:                         %s\n"
	       "        Valid:                                 %s\n",
	       offset,
	       reg & VLANPQF_VP0QSEL,
	       bit_to_prio(reg & VLANPQF_VP0PBSEL),
	       bit_to_boolean(reg & VLANPQF_VLANP0V),
	       (reg & VLANPQF_VP1QSEL) >> VLANPQF_VP1QSEL_SHIFT,
	       bit_to_prio(reg & VLANPQF_VP1PBSEL),
	       bit_to_boolean(reg & VLANPQF_VLANP1V),
	       (reg & VLANPQF_VP2QSEL) >> VLANPQF_VP2QSEL_SHIFT,
	       bit_to_prio(reg & VLANPQF_VP2PBSEL),
	       bit_to_boolean(reg & VLANPQF_VLANP2V),
	       (reg & VLANPQF_VP3QSEL) >> VLANPQF_VP3QSEL_SHIFT,
	       bit_to_prio(reg & VLANPQF_VP3PBSEL),
	       bit_to_boolean(reg & VLANPQF_VLANP3V),
	       (reg & VLANPQF_VP4QSEL) >> VLANPQF_VP4QSEL_SHIFT,
	       bit_to_prio(reg & VLANPQF_VP4PBSEL),
	       bit_to_boolean(reg & VLANPQF_VLANP4V),
	       (reg & VLANPQF_VP5QSEL) >> VLANPQF_VP5QSEL_SHIFT,
	       bit_to_prio(reg & VLANPQF_VP5PBSEL),
	       bit_to_boolean(reg & VLANPQF_VLANP5V),
	       (reg & VLANPQF_VP6QSEL) >> VLANPQF_VP6QSEL_SHIFT,
	       bit_to_prio(reg & VLANPQF_VP6PBSEL),
	       bit_to_boolean(reg & VLANPQF_VLANP6V),
	       (reg & VLANPQF_VP7QSEL) >> VLANPQF_VP7QSEL_SHIFT,
	       bit_to_prio(reg & VLANPQF_VP7PBSEL),
	       bit_to_boolean(reg & VLANPQF_VLANP7V));

	offset = 205;

	for (i = 0; i < 8; i++) {
		reg = regs_buff[offset + i];
		printf("%04d: ETQF (EType Queue Filter %d)                 \n"
		       "    EType:                                     %04X\n"
		       "    EType Length:                              %d\n"
		       "    EType Length Enable:                       %s\n"
		       "    Queue:                                     %d\n"
		       "    Queue Enable:                              %s\n"
		       "    Immediate Interrupt:                       %s\n"
		       "    1588 Time Stamp:                           %s\n"
		       "    Filter Enable:                             %s\n",
		       offset + i, i,
		       reg & ETQF_ETYPE,
		       (reg & ETQF_ETYPE_LEN) >> ETQF_ETYPE_LEN_SHIFT,
		       bit_to_boolean(reg & ETQF_ETYPE_LEN_EN),
		       (reg & ETQF_QUEUE) >> ETQF_QUEUE_SHIFT,
		       bit_to_boolean(reg & ETQF_QUEUE_EN),
		       bit_to_enable(reg & ETQF_IMMEDIATE_INTR),
		       bit_to_enable(reg & ETQF_1588_TIMESTAMP),
		       bit_to_boolean(reg & ETQF_FILTER_EN));
	}

	return 0;
}
