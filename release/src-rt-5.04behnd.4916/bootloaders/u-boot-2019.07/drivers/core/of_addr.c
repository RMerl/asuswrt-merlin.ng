// SPDX-License-Identifier: GPL-2.0+
/*
 * Taken from Linux v4.9 drivers/of/address.c
 *
 * Modified for U-Boot
 * Copyright (c) 2017 Google, Inc
 */

#include <common.h>
#include <linux/libfdt.h>
#include <dm/of_access.h>
#include <dm/of_addr.h>
#include <linux/err.h>
#include <linux/ioport.h>

/* Max address size we deal with */
#define OF_MAX_ADDR_CELLS	4
#define OF_CHECK_ADDR_COUNT(na)	((na) > 0 && (na) <= OF_MAX_ADDR_CELLS)
#define OF_CHECK_COUNTS(na, ns)	(OF_CHECK_ADDR_COUNT(na) && (ns) > 0)

static struct of_bus *of_match_bus(struct device_node *np);

/* Debug utility */
#ifdef DEBUG
static void of_dump_addr(const char *s, const __be32 *addr, int na)
{
	debug("%s", s);
	while (na--)
		pr_cont(" %08x", be32_to_cpu(*(addr++)));
	pr_cont("\n");
}
#else
static void of_dump_addr(const char *s, const __be32 *addr, int na) { }
#endif

/* Callbacks for bus specific translators */
struct of_bus {
	const char *name;
	const char *addresses;
	int (*match)(struct device_node *parent);
	void (*count_cells)(const struct device_node *child, int *addrc,
			    int *sizec);
	u64 (*map)(__be32 *addr, const __be32 *range, int na, int ns, int pna);
	int (*translate)(__be32 *addr, u64 offset, int na);
	unsigned int (*get_flags)(const __be32 *addr);
};

static void of_bus_default_count_cells(const struct device_node *np,
				       int *addrc, int *sizec)
{
	if (addrc)
		*addrc = of_n_addr_cells(np);
	if (sizec)
		*sizec = of_n_size_cells(np);
}

static u64 of_bus_default_map(__be32 *addr, const __be32 *range,
		int na, int ns, int pna)
{
	u64 cp, s, da;

	cp = of_read_number(range, na);
	s  = of_read_number(range + na + pna, ns);
	da = of_read_number(addr, na);

	debug("default map, cp=%llx, s=%llx, da=%llx\n",
	      (unsigned long long)cp, (unsigned long long)s,
	      (unsigned long long)da);

	if (da < cp || da >= (cp + s))
		return OF_BAD_ADDR;
	return da - cp;
}

static int of_bus_default_translate(__be32 *addr, u64 offset, int na)
{
	u64 a = of_read_number(addr, na);
	memset(addr, 0, na * 4);
	a += offset;
	if (na > 1)
		addr[na - 2] = cpu_to_be32(a >> 32);
	addr[na - 1] = cpu_to_be32(a & 0xffffffffu);

	return 0;
}

static unsigned int of_bus_default_get_flags(const __be32 *addr)
{
	return IORESOURCE_MEM;
}

/*
 * Array of bus-specific translators
 */
static struct of_bus of_busses[] = {
	/* Default */
	{
		.name = "default",
		.addresses = "reg",
		.match = NULL,
		.count_cells = of_bus_default_count_cells,
		.map = of_bus_default_map,
		.translate = of_bus_default_translate,
		.get_flags = of_bus_default_get_flags,
	},
};

static struct of_bus *of_match_bus(struct device_node *np)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(of_busses); i++)
		if (!of_busses[i].match || of_busses[i].match(np))
			return &of_busses[i];
	BUG();
	return NULL;
}

static void dev_count_cells(const struct device_node *np, int *nap, int *nsp)
{
	of_bus_default_count_cells(np, nap, nsp);
}

const __be32 *of_get_address(const struct device_node *dev, int index,
			     u64 *size, unsigned int *flags)
{
	const __be32 *prop;
	int psize;
	struct device_node *parent;
	struct of_bus *bus;
	int onesize, i, na, ns;

	/* Get parent & match bus type */
	parent = of_get_parent(dev);
	if (parent == NULL)
		return NULL;
	dev_count_cells(dev, &na, &ns);
	bus = of_match_bus(parent);
	bus->count_cells(dev, &na, &ns);
	of_node_put(parent);
	if (!OF_CHECK_ADDR_COUNT(na))
		return NULL;

	/* Get "reg" or "assigned-addresses" property */
	prop = of_get_property(dev, "reg", &psize);
	if (prop == NULL)
		return NULL;
	psize /= 4;

	onesize = na + ns;
	for (i = 0; psize >= onesize; psize -= onesize, prop += onesize, i++)
		if (i == index) {
			if (size)
				*size = of_read_number(prop + na, ns);
			if (flags)
				*flags = bus->get_flags(prop);
			return prop;
		}
	return NULL;
}
EXPORT_SYMBOL(of_get_address);

static int of_empty_ranges_quirk(const struct device_node *np)
{
	return false;
}

static int of_translate_one(const struct device_node *parent,
			    struct of_bus *bus, struct of_bus *pbus,
			    __be32 *addr, int na, int ns, int pna,
			    const char *rprop)
{
	const __be32 *ranges;
	int rlen;
	int rone;
	u64 offset = OF_BAD_ADDR;

	/*
	 * Normally, an absence of a "ranges" property means we are
	 * crossing a non-translatable boundary, and thus the addresses
	 * below the current cannot be converted to CPU physical ones.
	 * Unfortunately, while this is very clear in the spec, it's not
	 * what Apple understood, and they do have things like /uni-n or
	 * /ht nodes with no "ranges" property and a lot of perfectly
	 * useable mapped devices below them. Thus we treat the absence of
	 * "ranges" as equivalent to an empty "ranges" property which means
	 * a 1:1 translation at that level. It's up to the caller not to try
	 * to translate addresses that aren't supposed to be translated in
	 * the first place. --BenH.
	 *
	 * As far as we know, this damage only exists on Apple machines, so
	 * This code is only enabled on powerpc. --gcl
	 */
	ranges = of_get_property(parent, rprop, &rlen);
	if (ranges == NULL && !of_empty_ranges_quirk(parent)) {
		debug("no ranges; cannot translate\n");
		return 1;
	}
	if (ranges == NULL || rlen == 0) {
		offset = of_read_number(addr, na);
		memset(addr, 0, pna * 4);
		debug("empty ranges; 1:1 translation\n");
		goto finish;
	}

	debug("walking ranges...\n");

	/* Now walk through the ranges */
	rlen /= 4;
	rone = na + pna + ns;
	for (; rlen >= rone; rlen -= rone, ranges += rone) {
		offset = bus->map(addr, ranges, na, ns, pna);
		if (offset != OF_BAD_ADDR)
			break;
	}
	if (offset == OF_BAD_ADDR) {
		debug("not found !\n");
		return 1;
	}
	memcpy(addr, ranges + na, 4 * pna);

 finish:
	of_dump_addr("parent translation for:", addr, pna);
	debug("with offset: %llx\n", (unsigned long long)offset);

	/* Translate it into parent bus space */
	return pbus->translate(addr, offset, pna);
}

/*
 * Translate an address from the device-tree into a CPU physical address,
 * this walks up the tree and applies the various bus mappings on the
 * way.
 *
 * Note: We consider that crossing any level with #size-cells == 0 to mean
 * that translation is impossible (that is we are not dealing with a value
 * that can be mapped to a cpu physical address). This is not really specified
 * that way, but this is traditionally the way IBM at least do things
 */
static u64 __of_translate_address(const struct device_node *dev,
				  const __be32 *in_addr, const char *rprop)
{
	struct device_node *parent = NULL;
	struct of_bus *bus, *pbus;
	__be32 addr[OF_MAX_ADDR_CELLS];
	int na, ns, pna, pns;
	u64 result = OF_BAD_ADDR;

	debug("** translation for device %s **\n", of_node_full_name(dev));

	/* Increase refcount at current level */
	(void)of_node_get(dev);

	/* Get parent & match bus type */
	parent = of_get_parent(dev);
	if (parent == NULL)
		goto bail;
	bus = of_match_bus(parent);

	/* Count address cells & copy address locally */
	bus->count_cells(dev, &na, &ns);
	if (!OF_CHECK_COUNTS(na, ns)) {
		debug("Bad cell count for %s\n", of_node_full_name(dev));
		goto bail;
	}
	memcpy(addr, in_addr, na * 4);

	debug("bus is %s (na=%d, ns=%d) on %s\n", bus->name, na, ns,
	      of_node_full_name(parent));
	of_dump_addr("translating address:", addr, na);

	/* Translate */
	for (;;) {
		/* Switch to parent bus */
		of_node_put(dev);
		dev = parent;
		parent = of_get_parent(dev);

		/* If root, we have finished */
		if (parent == NULL) {
			debug("reached root node\n");
			result = of_read_number(addr, na);
			break;
		}

		/* Get new parent bus and counts */
		pbus = of_match_bus(parent);
		pbus->count_cells(dev, &pna, &pns);
		if (!OF_CHECK_COUNTS(pna, pns)) {
			debug("Bad cell count for %s\n",
			      of_node_full_name(dev));
			break;
		}

		debug("parent bus is %s (na=%d, ns=%d) on %s\n", pbus->name,
		      pna, pns, of_node_full_name(parent));

		/* Apply bus translation */
		if (of_translate_one(dev, bus, pbus, addr, na, ns, pna, rprop))
			break;

		/* Complete the move up one level */
		na = pna;
		ns = pns;
		bus = pbus;

		of_dump_addr("one level translation:", addr, na);
	}
 bail:
	of_node_put(parent);
	of_node_put(dev);

	return result;
}

u64 of_translate_address(const struct device_node *dev, const __be32 *in_addr)
{
	return __of_translate_address(dev, in_addr, "ranges");
}


static int __of_address_to_resource(const struct device_node *dev,
		const __be32 *addrp, u64 size, unsigned int flags,
		const char *name, struct resource *r)
{
	u64 taddr;

	if ((flags & (IORESOURCE_IO | IORESOURCE_MEM)) == 0)
		return -EINVAL;
	taddr = of_translate_address(dev, addrp);
	if (taddr == OF_BAD_ADDR)
		return -EINVAL;
	memset(r, 0, sizeof(struct resource));
	r->start = taddr;
	r->end = taddr + size - 1;
	r->flags = flags;
	r->name = name ? name : dev->full_name;

	return 0;
}

int of_address_to_resource(const struct device_node *dev, int index,
			   struct resource *r)
{
	const __be32	*addrp;
	u64		size;
	unsigned int	flags;
	const char	*name = NULL;

	addrp = of_get_address(dev, index, &size, &flags);
	if (addrp == NULL)
		return -EINVAL;

	/* Get optional "reg-names" property to add a name to a resource */
	of_property_read_string_index(dev, "reg-names", index, &name);

	return __of_address_to_resource(dev, addrp, size, flags, name, r);
}
