// SPDX-License-Identifier: GPL-2.0+
/*
 * Some very basic tests for fdtdec, accessed through test_fdtdec command.
 * They are easiest to use with sandbox.
 *
 * Copyright (c) 2011 The Chromium OS Authors.
 */

#include <common.h>
#include <fdtdec.h>
#include <linux/libfdt.h>
#include <malloc.h>
#include <os.h>

/* The size of our test fdt blob */
#define FDT_SIZE	(16 * 1024)

#define CHECK(op) ({							\
		int err = op;						\
		if (err < 0) {						\
			printf("%s: %s: %s\n", __func__, #op,		\
			       fdt_strerror(err));			\
			return err;					\
		}							\
									\
		err;							\
	})

#define CHECKVAL(op, expected) ({					\
		int err = op;						\
		if (err != expected) {					\
			printf("%s: %s: expected %d, but returned %d\n",\
			       __func__, #op, expected, err);		\
			return err;					\
		}							\
									\
		err;							\
	})

#define CHECKOK(op)	CHECKVAL(op, 0)

/* maximum number of nodes / aliases to generate */
#define MAX_NODES	20

/*
 * Make a test fdt
 *
 * @param fdt		Device tree pointer
 * @param size		Size of device tree blob
 * @param aliases	Specifies alias assignments. Format is a list of items
 *			separated by space. Items are #a where
 *				# is the alias number
 *				a is the node to point to
 * @param nodes		Specifies nodes to generate (a=0, b=1), upper case
 *			means to create a disabled node
 */
static int make_fdt(void *fdt, int size, const char *aliases,
		    const char *nodes)
{
	char name[20], value[20];
	const char *s;
#if defined(DEBUG) && defined(CONFIG_SANDBOX)
	int fd;
#endif

	CHECK(fdt_create(fdt, size));
	CHECK(fdt_finish_reservemap(fdt));
	CHECK(fdt_begin_node(fdt, ""));

	CHECK(fdt_begin_node(fdt, "aliases"));
	for (s = aliases; *s;) {
		sprintf(name, "i2c%c", *s);
		sprintf(value, "/i2c%d@0", s[1] - 'a');
		CHECK(fdt_property_string(fdt, name, value));
		s += 2 + (s[2] != '\0');
	}
	CHECK(fdt_end_node(fdt));

	for (s = nodes; *s; s++) {
		sprintf(value, "i2c%d@0", (*s & 0xdf) - 'A');
		CHECK(fdt_begin_node(fdt, value));
		CHECK(fdt_property_string(fdt, "compatible",
			fdtdec_get_compatible(COMPAT_UNKNOWN)));
		if (*s <= 'Z')
			CHECK(fdt_property_string(fdt, "status", "disabled"));
		CHECK(fdt_end_node(fdt));
	}

	CHECK(fdt_end_node(fdt));
	CHECK(fdt_finish(fdt));
	CHECK(fdt_pack(fdt));
#if defined(DEBUG) && defined(CONFIG_SANDBOX)
	fd = os_open("/tmp/fdtdec-text.dtb", OS_O_CREAT | OS_O_WRONLY);
	if (fd == -1) {
		printf("Could not open .dtb file to write\n");
		return -1;
	}
	os_write(fd, fdt, size);
	os_close(fd);
#endif
	return 0;
}

static int run_test(const char *aliases, const char *nodes, const char *expect)
{
	int list[MAX_NODES];
	const char *s;
	void *blob;
	int i;

	blob = malloc(FDT_SIZE);
	if (!blob) {
		printf("%s: out of memory\n", __func__);
		return 1;
	}

	printf("aliases=%s, nodes=%s, expect=%s: ", aliases, nodes, expect);
	CHECKVAL(make_fdt(blob, FDT_SIZE, aliases, nodes), 0);
	CHECKVAL(fdtdec_find_aliases_for_id(blob, "i2c",
			COMPAT_UNKNOWN,
			list, ARRAY_SIZE(list)), (int)strlen(expect));

	/* Check we got the right ones */
	for (i = 0, s = expect; *s; s++, i++) {
		int want = *s;
		const char *name;
		int got = ' ';

		name = list[i] ? fdt_get_name(blob, list[i], NULL) : NULL;
		if (name)
			got = name[3] + 'a' - '0';

		if (got != want) {
			printf("Position %d: Expected '%c', got '%c' ('%s')\n",
			       i, want, got, name);
			return 1;
		}
	}

	printf("pass\n");
	return 0;
}

static int make_fdt_carveout_device(void *fdt, uint32_t na, uint32_t ns)
{
	const char *basename = "/display";
	struct fdt_memory carveout = {
#ifdef CONFIG_PHYS_64BIT
		.start = 0x180000000,
		.end = 0x18fffffff,
#else
		.start = 0x80000000,
		.end = 0x8fffffff,
#endif
	};
	fdt32_t cells[4], *ptr = cells;
	uint32_t upper, lower;
	fdt_size_t size;
	char name[32];
	int offset;

	/* store one or two address cells */
	upper = upper_32_bits(carveout.start);
	lower = lower_32_bits(carveout.start);

	if (na > 1 && upper > 0)
		snprintf(name, sizeof(name), "%s@%x,%x", basename, upper,
			 lower);
	else
		snprintf(name, sizeof(name), "%s@%x", basename, lower);

	if (na > 1)
		*ptr++ = cpu_to_fdt32(upper);

	*ptr++ = cpu_to_fdt32(lower);

	/* store one or two size cells */
	size = carveout.end - carveout.start + 1;
	upper = upper_32_bits(size);
	lower = lower_32_bits(size);

	if (ns > 1)
		*ptr++ = cpu_to_fdt32(upper);

	*ptr++ = cpu_to_fdt32(lower);

	offset = CHECK(fdt_add_subnode(fdt, 0, name + 1));
	CHECK(fdt_setprop(fdt, offset, "reg", cells, (na + ns) * sizeof(*cells)));

	return fdtdec_set_carveout(fdt, name, "memory-region", 0,
				   "framebuffer", &carveout);
}

static int check_fdt_carveout(void *fdt, uint32_t address_cells,
			      uint32_t size_cells)
{
#ifdef CONFIG_PHYS_64BIT
	const char *name = "/display@1,80000000";
	const struct fdt_memory expected = {
		.start = 0x180000000,
		.end = 0x18fffffff,
	};
#else
	const char *name = "/display@80000000";
	const struct fdt_memory expected = {
		.start = 0x80000000,
		.end = 0x8fffffff,
	};
#endif
	struct fdt_memory carveout;

	printf("carveout: %pap-%pap na=%u ns=%u: ", &expected.start,
	       &expected.end, address_cells, size_cells);

	CHECK(fdtdec_get_carveout(fdt, name, "memory-region", 0, &carveout));

	if ((carveout.start != expected.start) ||
	    (carveout.end != expected.end)) {
		printf("carveout: %pap-%pap, expected %pap-%pap\n",
		       &carveout.start, &carveout.end,
		       &expected.start, &expected.end);
		return 1;
	}

	printf("pass\n");
	return 0;
}

static int make_fdt_carveout(void *fdt, int size, uint32_t address_cells,
			     uint32_t size_cells)
{
	fdt32_t na = cpu_to_fdt32(address_cells);
	fdt32_t ns = cpu_to_fdt32(size_cells);
#if defined(DEBUG) && defined(CONFIG_SANDBOX)
	char filename[512];
	int fd;
#endif
	int err;

	CHECK(fdt_create(fdt, size));
	CHECK(fdt_finish_reservemap(fdt));
	CHECK(fdt_begin_node(fdt, ""));
	CHECK(fdt_property(fdt, "#address-cells", &na, sizeof(na)));
	CHECK(fdt_property(fdt, "#size-cells", &ns, sizeof(ns)));
	CHECK(fdt_end_node(fdt));
	CHECK(fdt_finish(fdt));
	CHECK(fdt_pack(fdt));

	CHECK(fdt_open_into(fdt, fdt, FDT_SIZE));

	err = make_fdt_carveout_device(fdt, address_cells, size_cells);

#if defined(DEBUG) && defined(CONFIG_SANDBOX)
	snprintf(filename, sizeof(filename), "/tmp/fdtdec-carveout-%u-%u.dtb",
		 address_cells, size_cells);

	fd = os_open(filename, OS_O_CREAT | OS_O_WRONLY);
	if (fd < 0) {
		printf("could not open .dtb file to write\n");
		goto out;
	}

	os_write(fd, fdt, size);
	os_close(fd);

out:
#endif
	return err;
}

static int check_carveout(void)
{
	void *fdt;

	fdt = malloc(FDT_SIZE);
	if (!fdt) {
		printf("%s: out of memory\n", __func__);
		return 1;
	}

#ifndef CONFIG_PHYS_64BIT
	CHECKVAL(make_fdt_carveout(fdt, FDT_SIZE, 1, 1), 0);
	CHECKOK(check_fdt_carveout(fdt, 1, 1));
	CHECKVAL(make_fdt_carveout(fdt, FDT_SIZE, 1, 2), 0);
	CHECKOK(check_fdt_carveout(fdt, 1, 2));
#else
	CHECKVAL(make_fdt_carveout(fdt, FDT_SIZE, 1, 1), -FDT_ERR_BADVALUE);
	CHECKVAL(make_fdt_carveout(fdt, FDT_SIZE, 1, 2), -FDT_ERR_BADVALUE);
#endif
	CHECKVAL(make_fdt_carveout(fdt, FDT_SIZE, 2, 1), 0);
	CHECKOK(check_fdt_carveout(fdt, 2, 1));
	CHECKVAL(make_fdt_carveout(fdt, FDT_SIZE, 2, 2), 0);
	CHECKOK(check_fdt_carveout(fdt, 2, 2));

	return 0;
}

static int do_test_fdtdec(cmd_tbl_t *cmdtp, int flag, int argc,
			  char * const argv[])
{
	/* basic tests */
	CHECKOK(run_test("", "", ""));
	CHECKOK(run_test("1e 3d", "", ""));

	/*
	 * 'a' represents 0, 'b' represents 1, etc.
	 * The first character is the alias number, the second is the node
	 * number. So the params mean:
	 * 0a 1b	: point alias 0 to node 0 (a), alias 1 to node 1(b)
	 * ab		: to create nodes 0 and 1 (a and b)
	 * ab		: we expect the function to return two nodes, in
	 *		  the order 0, 1
	 */
	CHECKOK(run_test("0a 1b", "ab", "ab"));

	CHECKOK(run_test("0a 1c", "ab", "ab"));
	CHECKOK(run_test("1c", "ab", "ab"));
	CHECKOK(run_test("1b", "ab", "ab"));
	CHECKOK(run_test("0b", "ab", "ba"));
	CHECKOK(run_test("0b 2d", "dbc", "bcd"));
	CHECKOK(run_test("0d 3a 1c 2b", "dbac", "dcba"));

	/* things with holes */
	CHECKOK(run_test("1b 3d", "dbc", "cb d"));
	CHECKOK(run_test("1e 3d", "dbc", "bc d"));

	/* no aliases */
	CHECKOK(run_test("", "dbac", "dbac"));

	/* disabled nodes */
	CHECKOK(run_test("0d 3a 1c 2b", "dBac", "dc a"));
	CHECKOK(run_test("0b 2d", "DBc", "c"));
	CHECKOK(run_test("0b 4d 2c", "DBc", "  c"));

	/* conflicting aliases - first one gets it */
	CHECKOK(run_test("2a 1a 0a", "a", "  a"));
	CHECKOK(run_test("0a 1a 2a", "a", "a"));

	CHECKOK(check_carveout());

	printf("Test passed\n");
	return 0;
}

U_BOOT_CMD(
	test_fdtdec, 3, 1, do_test_fdtdec,
	"test_fdtdec",
	"Run tests for fdtdec library");
