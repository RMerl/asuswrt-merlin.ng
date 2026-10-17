// SPDX-License-Identifier: MIT
/*
 * The sart code is copied from m1n1 (https://github.com/AsahiLinux/m1n1) and
 * licensed as MIT.
 *
 * (C) Copyright 2022 The Asahi Linux Contributors
 */

#include <asm/io.h>
#include <asm/arch/sart.h>

#include <linux/bitfield.h>
#include <linux/types.h>

#include <malloc.h>

#define APPLE_SART_MAX_ENTRIES 16

/* This is probably a bitfield but the exact meaning of each bit is unknown. */
#define APPLE_SART_FLAGS_ALLOW 0xff

/* SARTv2 registers */
#define APPLE_SART2_CONFIG(idx)       (0x00 + 4 * (idx))
#define APPLE_SART2_CONFIG_FLAGS      GENMASK(31, 24)
#define APPLE_SART2_CONFIG_SIZE       GENMASK(23, 0)
#define APPLE_SART2_CONFIG_SIZE_SHIFT 12
#define APPLE_SART2_CONFIG_SIZE_MAX   GENMASK(23, 0)

#define APPLE_SART2_PADDR(idx)  (0x40 + 4 * (idx))
#define APPLE_SART2_PADDR_SHIFT 12

/* SARTv3 registers */
#define APPLE_SART3_CONFIG(idx) (0x00 + 4 * (idx))

#define APPLE_SART3_PADDR(idx)  (0x40 + 4 * (idx))
#define APPLE_SART3_PADDR_SHIFT 12

#define APPLE_SART3_SIZE(idx)  (0x80 + 4 * (idx))
#define APPLE_SART3_SIZE_SHIFT 12
#define APPLE_SART3_SIZE_MAX   GENMASK(29, 0)

struct apple_sart {
	uintptr_t base;
	u32 protected_entries;

	void (*get_entry)(struct apple_sart *sart, int index, u8 *flags, void **paddr,
			  size_t *size);
	bool (*set_entry)(struct apple_sart *sart, int index, u8 flags, void *paddr,
			  size_t size);
};

static void sart2_get_entry(struct apple_sart *sart, int index, u8 *flags, void **paddr,
			    size_t *size)
{
	u32 cfg = readl(sart->base + APPLE_SART2_CONFIG(index));
	*flags = FIELD_GET(APPLE_SART2_CONFIG_FLAGS, cfg);
	*size = (size_t)FIELD_GET(APPLE_SART2_CONFIG_SIZE, cfg) << APPLE_SART2_CONFIG_SIZE_SHIFT;
	*paddr = (void *)
		((u64)readl(sart->base + APPLE_SART2_PADDR(index)) << APPLE_SART2_PADDR_SHIFT);
}

static bool sart2_set_entry(struct apple_sart *sart, int index, u8 flags, void *paddr_,
			    size_t size)
{
	u32 cfg;
	u64 paddr = (u64)paddr_;

	if (size & ((1 << APPLE_SART2_CONFIG_SIZE_SHIFT) - 1))
		return false;
	if (paddr & ((1 << APPLE_SART2_PADDR_SHIFT) - 1))
		return false;

	size >>= APPLE_SART2_CONFIG_SIZE_SHIFT;
	paddr >>= APPLE_SART2_PADDR_SHIFT;

	if (size > APPLE_SART2_CONFIG_SIZE_MAX)
		return false;

	cfg = FIELD_PREP(APPLE_SART2_CONFIG_FLAGS, flags);
	cfg |= FIELD_PREP(APPLE_SART2_CONFIG_SIZE, size);

	writel(paddr, sart->base + APPLE_SART2_PADDR(index));
	writel(cfg, sart->base + APPLE_SART2_CONFIG(index));

	return true;
}

static void sart3_get_entry(struct apple_sart *sart, int index, u8 *flags, void **paddr,
			    size_t *size)
{
	*flags = readl(sart->base + APPLE_SART3_CONFIG(index));
	*size = (size_t)readl(sart->base + APPLE_SART3_SIZE(index)) << APPLE_SART3_SIZE_SHIFT;
	*paddr = (void *)
		((u64)readl(sart->base + APPLE_SART3_PADDR(index)) << APPLE_SART3_PADDR_SHIFT);
}

static bool sart3_set_entry(struct apple_sart *sart, int index, u8 flags, void *paddr_,
			    size_t size)
{
	u64 paddr = (u64)paddr_;

	if (size & ((1 << APPLE_SART3_SIZE_SHIFT) - 1))
		return false;
	if (paddr & ((1 << APPLE_SART3_PADDR_SHIFT) - 1))
		return false;

	paddr >>= APPLE_SART3_PADDR_SHIFT;
	size >>= APPLE_SART3_SIZE_SHIFT;

	if (size > APPLE_SART3_SIZE_MAX)
		return false;

	writel(paddr, sart->base + APPLE_SART3_PADDR(index));
	writel(size, sart->base + APPLE_SART3_SIZE(index));
	writel(flags, sart->base + APPLE_SART3_CONFIG(index));

	return true;
}

struct apple_sart *sart_init(ofnode node)
{
	phys_addr_t base;
	u32 sart_version;
	struct apple_sart *sart;

	base = ofnode_get_addr(node);
	if (base == FDT_ADDR_T_NONE)
		return NULL;

	if (ofnode_device_is_compatible(node, "apple,t8103-sart")) {
		sart_version = 2;
	} else if (ofnode_device_is_compatible(node, "apple,t6000-sart")) {
		sart_version = 3;
	} else {
		printf("sart: unknown SART compatible: %sd\n",
		       ofnode_read_string(node, "compatible"));
		return NULL;
	}

	sart = calloc(sizeof(*sart), 1);
	if (!sart)
		return NULL;

	sart->base = base;

	switch (sart_version) {
	case 2:
		sart->get_entry = sart2_get_entry;
		sart->set_entry = sart2_set_entry;
		break;
	case 3:
		sart->get_entry = sart3_get_entry;
		sart->set_entry = sart3_set_entry;
		break;
	default:
		printf("sart: SART has unknown version %d\n", sart_version);
		free(sart);
		return NULL;
	}

	sart->protected_entries = 0;
	for (unsigned int i = 0; i < APPLE_SART_MAX_ENTRIES; ++i) {
		void *paddr;
		u8 flags;
		size_t sz;

		sart->get_entry(sart, i, &flags, &paddr, &sz);
		if (flags)
			sart->protected_entries |= 1 << i;
	}

	return sart;
}

void sart_free(struct apple_sart *sart)
{
	for (unsigned int i = 0; i < APPLE_SART_MAX_ENTRIES; ++i) {
		if (sart->protected_entries & (1 << i))
			continue;
		sart->set_entry(sart, i, 0, NULL, 0);
	}

	free(sart);
}

bool sart_add_allowed_region(struct apple_sart *sart, void *paddr, size_t sz)
{
	for (unsigned int i = 0; i < APPLE_SART_MAX_ENTRIES; ++i) {
		void *e_paddr;
		u8 e_flags;
		size_t e_sz;

		if (sart->protected_entries & (1 << i))
			continue;

		sart->get_entry(sart, i, &e_flags, &e_paddr, &e_sz);
		if (e_flags)
			continue;

		return sart->set_entry(sart, i, APPLE_SART_FLAGS_ALLOW, paddr, sz);
	}

	printf("sart: no more free entries\n");
	return false;
}

bool sart_remove_allowed_region(struct apple_sart *sart, void *paddr, size_t sz)
{
	for (unsigned int i = 0; i < APPLE_SART_MAX_ENTRIES; ++i) {
		void *e_paddr;
		u8 e_flags;
		size_t e_sz;

		if (sart->protected_entries & (1 << i))
			continue;

		sart->get_entry(sart, i, &e_flags, &e_paddr, &e_sz);
		if (!e_flags)
			continue;
		if (e_paddr != paddr)
			continue;
		if (e_sz != sz)
			continue;

		return sart->set_entry(sart, i, 0, NULL, 0);
	}

	printf("sart: could not find entry to be removed\n");
	return false;
}
