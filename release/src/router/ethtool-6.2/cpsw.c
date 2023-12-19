// SPDX-License-Identifier: GPL-2.0
/* Code to dump registers for TI CPSW switch devices.
 *
 * Copyright (c) 2022 Linutronix GmbH
 * Author: Benedikt Spranger <b.spranger@linutronix.de>
 */

#include <stdio.h>
#include <string.h>

#include "internal.h"

#define ALE_ENTRY_BITS		68
#define ALE_ENTRY_WORDS DIV_ROUND_UP(ALE_ENTRY_BITS, 32)
#define ALE_ENTRY_BYTES (ALE_ENTRY_WORDS * 4)

struct address_table_entry
{
	u8 port;
	u8 reserved1;
	u8 reserved2;
	u8 reserved3;
	u8 addr2;
	u8 addr1;
	u16 vlan;
	u8 addr6;
	u8 addr5;
	u8 addr4;
	u8 addr3;
} __attribute__((packed));

struct vlan_table_entry
{
	u8 reserved1;
	u8 reserved2;
	u8 reserved3;
	u8 reserved4;
	u8 reserved5;
	u8 reserved6;
	u16 vlan;
	u8 member;
	u8 mc_unreg;
	u8 mc_reg;
	u8 untag;
} __attribute__((packed));

union ale_entry {
	struct address_table_entry addr;
	struct vlan_table_entry vlan;
	u32 val[3];
	u8 byte[12];
};

enum entry_type {
	FREE_ENTRY = 0,
	ADDR_ENTRY,
	VLAN_ENTRY,
	VLAN_ADDR_ENTRY,
	LAST_ENTRY
};

static char *fwd_state_name[] = {
	"Forwarding",
	"Blocking/Forwarding/Learning",
	"Forwarding/Learning",
	"Forwarding",
};

static char *type_name[] = {
	"free entry",
	"address entry",
	"VLAN entry",
	"VLAN address entry",
	"invalid"
};

enum entry_type decode_type(union ale_entry *entry)
{
	/* Entry Type (61:60) */
	return (entry->byte[7] >> 4) & 0x3;
}

static void print_addr(u8 *data)
{
	printf("%02x:%02x:%02x:%02x:%02x:%02x",
	       data[5], data[4], data[11], data[10], data[9], data[8]);
}

static void decode_multi_addr(union ale_entry *entry, int vlan)
{
	printf("      MULTI: ");
	print_addr(entry->byte);
	printf(" %s", fwd_state_name[entry->addr.vlan >> 14]);
	printf("%s", (entry->addr.port & 0x02) ? " Super" : "");
	printf(" Ports: 0x%x", (entry->addr.port >> 2) & 0x3);
	if (vlan)
		printf(" VLAN: %04d", entry->addr.vlan & 0x0fff);
	printf("\n");
}

static void decode_uni_addr(union ale_entry *entry, int vlan)
{
	printf("      UNI  : ");
	print_addr(entry->byte);
	printf("%s", (entry->addr.port & 0x01) ? " Secure" : "");
	printf("%s", (entry->addr.port & 0x02) ? " Block" : "");
	printf("%s", (entry->addr.port & 0x20) ? " DLR" : "");
	printf(" Ports: 0x%x", (entry->addr.port >> 2) & 0x3);
	if (vlan)
		printf(" VLAN: %04d", entry->addr.vlan & 0x0fff);
	printf("\n");
}

static void decode_oui_addr(union ale_entry *entry)
{
	printf("      OUI  : ");
	print_addr(entry->byte);
	printf("\n");
}

static void decode_vlan(union ale_entry *entry)
{
	printf("      VLAN ");
	printf("%04d: ", entry->vlan.vlan & 0x0fff);
	printf("member: 0x%x ", entry->vlan.member & 0x7);
	printf("mc flood unreg: 0x%x ", entry->vlan.mc_unreg & 0x7);
	printf("mc flood reg: 0x%x ", entry->vlan.mc_reg & 0x7);
	printf("untag: 0x%x\n", entry->vlan.untag & 0x7);
}

static enum entry_type decode_ale_entry(unsigned int idx, const u8 *data,
					bool last_was_free)
{
	union ale_entry *entry = (union ale_entry *) data;
	enum entry_type type;

	entry = entry + idx;
	type = decode_type(entry);

	if (!last_was_free || type != FREE_ENTRY)
		printf("%04d: %s\n", idx, type_name[type]);

	switch (type)
	{
	case FREE_ENTRY:
		goto out;
		break;

	case ADDR_ENTRY:
		/* Multicast: OUI 01:00:5e:xx:xx:xx */
		if (entry->addr.addr1 == 0x01)
			decode_multi_addr(entry, 0);
		else
			if ((entry->addr.vlan >> 14) == 0x2)
				decode_oui_addr(entry);
			else
				decode_uni_addr(entry, 0);
		break;

	case VLAN_ENTRY:
		decode_vlan(entry);
		break;

	case VLAN_ADDR_ENTRY:
		/* Check for Individual/Group bit */
		if (entry->addr.addr1 & 0x01)
			decode_multi_addr(entry, 1);
		else
			decode_uni_addr(entry, 1);
		break;

	default:
		printf("internal failure.\n");
	}

out:
	return type;
}

int cpsw_dump_regs(struct ethtool_drvinfo *info __maybe_unused,
		   struct ethtool_regs *regs)
{
	unsigned int entries = regs->len/ALE_ENTRY_BYTES;
	enum entry_type type = LAST_ENTRY;
	unsigned int i;

	printf("ALE Entries (%d):\n", entries);

	for (i = 0; i < entries; i++)
		type = decode_ale_entry(i, regs->data, (type == FREE_ENTRY));

	return 0;
}
