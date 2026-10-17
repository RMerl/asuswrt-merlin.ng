// SPDX-License-Identifier: GPL-2.0+
/*
 * Simple API for configuring TrustZone memory restrictions for TZC400
 */

#define LOG_CATEGORY LOGC_ARCH

#include <linux/iopoll.h>
#include <mach/tzc.h>

#define TZC_TIMEOUT_US		100

#define TZC_BUILD_CONFIG	0x00
#define TZC_ACTION		0x04
#define TZC_ACTION_NONE		0
#define TZC_ACTION_ERR		1
#define TZC_ACTION_INT		2
#define TZC_ACTION_INT_ERR	3
#define TZC_GATE_KEEPER		0x08

#define TZC_REGION0_OFFSET	0x100
#define TZC_REGION_CFG_SIZE	0x20
#define TZC_REGION1_OFFSET	0x120
#define TZC_REGION_BASE		0x00
#define TZC_REGION_TOP		0x08
#define TZC_REGION_ATTRIBUTE	0x10
#define TZC_REGION_ACCESS	0x14

static uint32_t tzc_read(uintptr_t tzc, size_t reg)
{
	return readl(tzc + reg);
}

static void tzc_write(uintptr_t tzc, size_t reg, uint32_t val)
{
	writel(val, tzc + reg);
}

static uint16_t tzc_config_get_active_filters(const struct tzc_region *cfg)
{
	uint16_t active_filters = 0;

	for ( ; cfg->top != 0; cfg++)
		active_filters |= cfg->filters_mask;

	return active_filters;
}

int tzc_configure(uintptr_t tzc, const struct tzc_region *cfg)
{
	uintptr_t region = tzc + TZC_REGION1_OFFSET;
	uint32_t nsid, attr_reg, active_filters;
	int ret;

	active_filters = tzc_config_get_active_filters(cfg);
	if (active_filters == 0)
		return -EINVAL;

	ret = tzc_disable_filters(tzc, active_filters);
	if (ret < 0)
		return ret;

	for ( ; cfg->top != 0; cfg++, region += TZC_REGION_CFG_SIZE) {
		attr_reg = (cfg->sec_mode & 0x03) << 30;
		attr_reg |= (cfg->filters_mask & 0x03) << 0;
		nsid = cfg->nsec_id & 0xffff;
		nsid |= nsid << 16;

		tzc_write(region, TZC_REGION_BASE, cfg->base);
		tzc_write(region, TZC_REGION_TOP, cfg->top);
		tzc_write(region, TZC_REGION_ACCESS, nsid);
		tzc_write(region, TZC_REGION_ATTRIBUTE, attr_reg);
	}

	tzc_write(tzc, TZC_ACTION, TZC_ACTION_ERR);
	return tzc_enable_filters(tzc, active_filters);
}

int tzc_disable_filters(uintptr_t tzc, uint16_t filters_mask)
{
	uint32_t gate = tzc_read(tzc, TZC_GATE_KEEPER);
	uint32_t filter_status = filters_mask << 16;

	gate &= ~filters_mask;
	tzc_write(tzc, TZC_GATE_KEEPER, gate);

	return readl_poll_timeout(tzc + TZC_GATE_KEEPER, gate,
				 (gate & filter_status) == 0, TZC_TIMEOUT_US);
}

int tzc_enable_filters(uintptr_t tzc, uint16_t filters_mask)
{
	uint32_t gate = tzc_read(tzc, TZC_GATE_KEEPER);
	uint32_t filter_status = filters_mask << 16;

	gate |= filters_mask;
	tzc_write(tzc, TZC_GATE_KEEPER, gate);

	return readl_poll_timeout(tzc + TZC_GATE_KEEPER, gate,
				 (gate & filter_status) == filter_status,
				 TZC_TIMEOUT_US);
}

static const char *sec_access_str_from_attr(uint32_t attr)
{
	const char *const sec_mode[] = { "none", "RO  ", "WO  ", "RW  " };

	return sec_mode[(attr >> 30) & 0x03];
}

void tzc_dump_config(uintptr_t tzc)
{
	uint32_t build_config, base, top, attr, nsaid;
	int num_regions, i;
	uintptr_t region;

	build_config = tzc_read(tzc, TZC_BUILD_CONFIG);
	num_regions = ((build_config >> 0) & 0x1f) + 1;

	for (i = 0; i < num_regions; i++) {
		region = tzc + TZC_REGION0_OFFSET + i * TZC_REGION_CFG_SIZE;

		base = tzc_read(region, TZC_REGION_BASE);
		top = tzc_read(region, TZC_REGION_TOP);
		attr = tzc_read(region, TZC_REGION_ATTRIBUTE);
		nsaid = tzc_read(region, TZC_REGION_ACCESS);

		if (attr == 0 && nsaid == 0)
			continue;

		log_info("TZC region %u: %08x->%08x - filters 0x%x\n",
			 i, base, top, (attr >> 0) & 0xf);
		log_info("\t Secure access %s NSAID %08x\n",
			 sec_access_str_from_attr(attr), nsaid);
	}
}
