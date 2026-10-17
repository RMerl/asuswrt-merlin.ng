// SPDX-License-Identifier: GPL-2.0+
/*
 * Tests for fdt command
 *
 * Copyright 2022 Google LLC
 */

#include <common.h>
#include <console.h>
#include <fdt_support.h>
#include <mapmem.h>
#include <asm/global_data.h>
#include <linux/libfdt.h>
#include <test/suites.h>
#include <test/ut.h>

DECLARE_GLOBAL_DATA_PTR;
/*
 * Missing tests:
 * fdt boardsetup         - Do board-specific set up
 * fdt checksign [<addr>] - check FIT signature
 *                          <addr> - address of key blob
 *                                   default gd->fdt_blob
 */

/* Declare a new fdt test */
#define FDT_TEST(_name, _flags)	UNIT_TEST(_name, _flags, fdt_test)

/**
 * make_test_fdt() - Create an FDT with just a root node
 *
 * The size is set to the minimum needed
 *
 * @uts: Test state
 * @fdt: Place to write FDT
 * @size: Maximum size of space for fdt
 */
static int make_test_fdt(struct unit_test_state *uts, void *fdt, int size)
{
	ut_assertok(fdt_create(fdt, size));
	ut_assertok(fdt_finish_reservemap(fdt));
	ut_assert(fdt_begin_node(fdt, "") >= 0);
	ut_assertok(fdt_end_node(fdt));
	ut_assertok(fdt_finish(fdt));

	return 0;
}

/**
 * make_fuller_fdt() - Create an FDT with root node and properties
 *
 * The size is set to the minimum needed
 *
 * @uts: Test state
 * @fdt: Place to write FDT
 * @size: Maximum size of space for fdt
 */
static int make_fuller_fdt(struct unit_test_state *uts, void *fdt, int size)
{
	fdt32_t regs[2] = { cpu_to_fdt32(0x1234), cpu_to_fdt32(0x1000) };

	/*
	 * Assemble the following DT for test purposes:
	 *
	 * / {
	 * 	#address-cells = <0x00000001>;
	 * 	#size-cells = <0x00000001>;
	 * 	compatible = "u-boot,fdt-test";
	 * 	model = "U-Boot FDT test";
	 *
	 *	aliases {
	 *		badalias = "/bad/alias";
	 *		subnodealias = "/test-node@1234/subnode";
	 *		testnodealias = "/test-node@1234";
	 *	};
	 *
	 * 	test-node@1234 {
	 * 		#address-cells = <0x00000000>;
	 * 		#size-cells = <0x00000000>;
	 * 		compatible = "u-boot,fdt-test-device1";
	 * 		clock-names = "fixed", "i2c", "spi", "uart2", "uart1";
	 * 		u-boot,empty-property;
	 * 		clock-frequency = <0x00fde800>;
	 * 		regs = <0x00001234 0x00001000>;
	 *
	 * 		subnode {
	 * 			#address-cells = <0x00000000>;
	 * 			#size-cells = <0x00000000>;
	 * 			compatible = "u-boot,fdt-subnode-test-device";
	 * 		};
	 * 	};
	 * };
	 */

	ut_assertok(fdt_create(fdt, size));
	ut_assertok(fdt_finish_reservemap(fdt));
	ut_assert(fdt_begin_node(fdt, "") >= 0);

	ut_assertok(fdt_property_u32(fdt, "#address-cells", 1));
	ut_assertok(fdt_property_u32(fdt, "#size-cells", 1));
	/* <string> */
	ut_assertok(fdt_property_string(fdt, "compatible", "u-boot,fdt-test"));
	/* <string> */
	ut_assertok(fdt_property_string(fdt, "model", "U-Boot FDT test"));

	ut_assert(fdt_begin_node(fdt, "aliases") >= 0);
	/* <string> */
	ut_assertok(fdt_property_string(fdt, "badalias", "/bad/alias"));
	/* <string> */
	ut_assertok(fdt_property_string(fdt, "subnodealias", "/test-node@1234/subnode"));
	/* <string> */
	ut_assertok(fdt_property_string(fdt, "testnodealias", "/test-node@1234"));
	ut_assertok(fdt_end_node(fdt));

	ut_assert(fdt_begin_node(fdt, "test-node@1234") >= 0);
	ut_assertok(fdt_property_cell(fdt, "#address-cells", 0));
	ut_assertok(fdt_property_cell(fdt, "#size-cells", 0));
	/* <string> */
	ut_assertok(fdt_property_string(fdt, "compatible", "u-boot,fdt-test-device1"));
	/* <stringlist> */
	ut_assertok(fdt_property(fdt, "clock-names", "fixed\0i2c\0spi\0uart2\0uart1\0", 26));
	/* <empty> */
	ut_assertok(fdt_property(fdt, "u-boot,empty-property", NULL, 0));
	/*
	 * <u32>
	 * This value is deliberate as it used to break cmd/fdt.c
	 * is_printable_string() implementation.
	 */
	ut_assertok(fdt_property_u32(fdt, "clock-frequency", 16640000));
	/* <prop-encoded-array> */
	ut_assertok(fdt_property(fdt, "regs", &regs, sizeof(regs)));
	ut_assert(fdt_begin_node(fdt, "subnode") >= 0);
	ut_assertok(fdt_property_cell(fdt, "#address-cells", 0));
	ut_assertok(fdt_property_cell(fdt, "#size-cells", 0));
	ut_assertok(fdt_property_string(fdt, "compatible", "u-boot,fdt-subnode-test-device"));
	ut_assertok(fdt_end_node(fdt));
	ut_assertok(fdt_end_node(fdt));

	ut_assertok(fdt_end_node(fdt));
	ut_assertok(fdt_finish(fdt));

	return 0;
}

/* Test 'fdt addr' getting/setting address */
static int fdt_test_addr(struct unit_test_state *uts)
{
	const void *fdt_blob, *new_fdt;
	char fdt[256];
	ulong addr;
	int ret;

	ut_assertok(console_record_reset_enable());
	ut_assertok(run_command("fdt addr -c", 0));
	ut_assert_nextline("Control fdt: %08lx",
			   (ulong)map_to_sysmem(gd->fdt_blob));
	ut_assertok(ut_check_console_end(uts));

	/* The working fdt is not set, so this should fail */
	set_working_fdt_addr(0);
	ut_assert_nextline("Working FDT set to 0");
	ut_asserteq(CMD_RET_FAILURE, run_command("fdt addr", 0));

	/*
	 * sandbox fails the check for !blob since the 0 pointer is mapped to
	 * memory somewhere other than at 0x0
	 */
	if (IS_ENABLED(CONFIG_SANDBOX))
		ut_assert_nextline("libfdt fdt_check_header(): FDT_ERR_BADMAGIC");
	ut_assertok(ut_check_console_end(uts));

	/* Set up a working FDT and try again */
	ut_assertok(make_test_fdt(uts, fdt, sizeof(fdt)));
	addr = map_to_sysmem(fdt);
	set_working_fdt_addr(addr);
	ut_assert_nextline("Working FDT set to %lx", addr);
	ut_assertok(run_command("fdt addr", 0));
	ut_assert_nextline("Working fdt: %08lx", (ulong)map_to_sysmem(fdt));
	ut_assertok(ut_check_console_end(uts));

	/* Set the working FDT */
	set_working_fdt_addr(0);
	ut_assert_nextline("Working FDT set to 0");
	ut_assertok(run_commandf("fdt addr %08lx", addr));
	ut_assert_nextline("Working FDT set to %lx", addr);
	ut_asserteq(addr, map_to_sysmem(working_fdt));
	ut_assertok(ut_check_console_end(uts));
	set_working_fdt_addr(0);
	ut_assert_nextline("Working FDT set to 0");

	/* Set the control FDT */
	fdt_blob = gd->fdt_blob;
	gd->fdt_blob = NULL;
	ret = run_commandf("fdt addr -c %08lx", addr);
	new_fdt = gd->fdt_blob;
	gd->fdt_blob = fdt_blob;
	ut_assertok(ret);
	ut_asserteq(addr, map_to_sysmem(new_fdt));
	ut_assertok(ut_check_console_end(uts));

	/* Test setting an invalid FDT */
	fdt[0] = 123;
	ut_asserteq(1, run_commandf("fdt addr %08lx", addr));
	ut_assert_nextline("libfdt fdt_check_header(): FDT_ERR_BADMAGIC");
	ut_assertok(ut_check_console_end(uts));

	/* Test detecting an invalid FDT */
	fdt[0] = 123;
	set_working_fdt_addr(addr);
	ut_assert_nextline("Working FDT set to %lx", addr);
	ut_asserteq(1, run_commandf("fdt addr"));
	ut_assert_nextline("libfdt fdt_check_header(): FDT_ERR_BADMAGIC");
	ut_assertok(ut_check_console_end(uts));

	return 0;
}
FDT_TEST(fdt_test_addr, UT_TESTF_CONSOLE_REC);

/* Test 'fdt addr' resizing an fdt */
static int fdt_test_addr_resize(struct unit_test_state *uts)
{
	char fdt[256];
	const int newsize = sizeof(fdt) / 2;
	ulong addr;

	ut_assertok(make_test_fdt(uts, fdt, sizeof(fdt)));
	addr = map_to_sysmem(fdt);
	set_working_fdt_addr(addr);

	/* Test setting and resizing the working FDT to a larger size */
	ut_assertok(console_record_reset_enable());
	ut_assertok(run_commandf("fdt addr %08lx %x", addr, newsize));
	ut_assert_nextline("Working FDT set to %lx", addr);
	ut_assertok(ut_check_console_end(uts));

	/* Try shrinking it */
	ut_assertok(run_commandf("fdt addr %08lx %zx", addr, sizeof(fdt) / 4));
	ut_assert_nextline("Working FDT set to %lx", addr);
	ut_assert_nextline("New length %d < existing length %d, ignoring",
			   (int)sizeof(fdt) / 4, newsize);
	ut_assertok(ut_check_console_end(uts));

	/* ...quietly */
	ut_assertok(run_commandf("fdt addr -q %08lx %zx", addr, sizeof(fdt) / 4));
	ut_assertok(ut_check_console_end(uts));

	/* We cannot easily provoke errors in fdt_open_into(), so ignore that */

	return 0;
}
FDT_TEST(fdt_test_addr_resize, UT_TESTF_CONSOLE_REC);

static int fdt_test_move(struct unit_test_state *uts)
{
	char fdt[256];
	ulong addr, newaddr = 0x10000;
	const int size = sizeof(fdt);
	uint32_t ts;
	void *buf;

	/* Original source DT */
	ut_assertok(make_test_fdt(uts, fdt, size));
	ts = fdt_totalsize(fdt);
	addr = map_to_sysmem(fdt);
	set_working_fdt_addr(addr);

	/* Moved target DT location */
	buf = map_sysmem(newaddr, size);
	memset(buf, 0, size);

	/* Test moving the working FDT to a new location */
	ut_assertok(console_record_reset_enable());
	ut_assertok(run_commandf("fdt move %08lx %08lx %x", addr, newaddr, ts));
	ut_assert_nextline("Working FDT set to %lx", newaddr);
	ut_assertok(ut_check_console_end(uts));

	/* Compare the source and destination DTs */
	ut_assertok(console_record_reset_enable());
	ut_assertok(run_commandf("cmp.b %08lx %08lx %x", addr, newaddr, ts));
	ut_assert_nextline("Total of %d byte(s) were the same", ts);
	ut_assertok(ut_check_console_end(uts));

	return 0;
}
FDT_TEST(fdt_test_move, UT_TESTF_CONSOLE_REC);

static int fdt_test_resize(struct unit_test_state *uts)
{
	char fdt[256];
	const unsigned int newsize = 0x2000;
	uint32_t ts;
	ulong addr;

	/* Original source DT */
	ut_assertok(make_test_fdt(uts, fdt, sizeof(fdt)));
	fdt_shrink_to_minimum(fdt, 0);	/* Resize with 0 extra bytes */
	ts = fdt_totalsize(fdt);
	addr = map_to_sysmem(fdt);
	set_working_fdt_addr(addr);

	/* Test resizing the working FDT and verify the new space was added */
	ut_assertok(console_record_reset_enable());
	ut_assertok(run_commandf("fdt resize %x", newsize));
	ut_asserteq(ts + newsize, fdt_totalsize(fdt));
	ut_assertok(ut_check_console_end(uts));

	return 0;
}
FDT_TEST(fdt_test_resize, UT_TESTF_CONSOLE_REC);

static int fdt_test_print_list_common(struct unit_test_state *uts,
				      const char *opc, const char *node)
{
	/*
	 * Test printing/listing the working FDT
	 * subnode $node/subnode
	 */
	ut_assertok(console_record_reset_enable());
	ut_assertok(run_commandf("fdt %s %s/subnode", opc, node));
	ut_assert_nextline("subnode {");
	ut_assert_nextline("\t#address-cells = <0x00000000>;");
	ut_assert_nextline("\t#size-cells = <0x00000000>;");
	ut_assert_nextline("\tcompatible = \"u-boot,fdt-subnode-test-device\";");
	ut_assert_nextline("};");
	ut_assertok(ut_check_console_end(uts));

	/*
	 * Test printing/listing the working FDT
	 * path / string property model
	 */
	ut_assertok(console_record_reset_enable());
	ut_assertok(run_commandf("fdt %s / model", opc));
	ut_assert_nextline("model = \"U-Boot FDT test\"");
	ut_assertok(ut_check_console_end(uts));

	/*
	 * Test printing/listing the working FDT
	 * path $node string property compatible
	 */
	ut_assertok(console_record_reset_enable());
	ut_assertok(run_commandf("fdt %s %s compatible", opc, node));
	ut_assert_nextline("compatible = \"u-boot,fdt-test-device1\"");
	ut_assertok(ut_check_console_end(uts));

	/*
	 * Test printing/listing the working FDT
	 * path $node stringlist property clock-names
	 */
	ut_assertok(console_record_reset_enable());
	ut_assertok(run_commandf("fdt %s %s clock-names", opc, node));
	ut_assert_nextline("clock-names = \"fixed\", \"i2c\", \"spi\", \"uart2\", \"uart1\"");
	ut_assertok(ut_check_console_end(uts));

	/*
	 * Test printing/listing the working FDT
	 * path $node u32 property clock-frequency
	 */
	ut_assertok(console_record_reset_enable());
	ut_assertok(run_commandf("fdt %s %s clock-frequency", opc, node));
	ut_assert_nextline("clock-frequency = <0x00fde800>");
	ut_assertok(ut_check_console_end(uts));

	/*
	 * Test printing/listing the working FDT
	 * path $node empty property u-boot,empty-property
	 */
	ut_assertok(console_record_reset_enable());
	ut_assertok(run_commandf("fdt %s %s u-boot,empty-property", opc, node));
	/*
	 * This is the only 'fdt print' / 'fdt list' incantation which
	 * prefixes the property with node path. This has been in U-Boot
	 * since the beginning of the command 'fdt', keep it.
	 */
	ut_assert_nextline("%s u-boot,empty-property", node);
	ut_assertok(ut_check_console_end(uts));

	/*
	 * Test printing/listing the working FDT
	 * path $node prop-encoded array property regs
	 */
	ut_assertok(console_record_reset_enable());
	ut_assertok(run_commandf("fdt %s %s regs", opc, node));
	ut_assert_nextline("regs = <0x00001234 0x00001000>");
	ut_assertok(ut_check_console_end(uts));

	return 0;
}

static int fdt_test_print_list(struct unit_test_state *uts, bool print)
{
	const char *opc = print ? "print" : "list";
	char fdt[4096];
	ulong addr;
	int ret;

	/* Original source DT */
	ut_assertok(make_fuller_fdt(uts, fdt, sizeof(fdt)));
	addr = map_to_sysmem(fdt);
	set_working_fdt_addr(addr);

	/* Test printing/listing the working FDT -- node / */
	ut_assertok(console_record_reset_enable());
	ut_assertok(run_commandf("fdt %s", opc));
	ut_assert_nextline("/ {");
	ut_assert_nextline("\t#address-cells = <0x00000001>;");
	ut_assert_nextline("\t#size-cells = <0x00000001>;");
	ut_assert_nextline("\tcompatible = \"u-boot,fdt-test\";");
	ut_assert_nextline("\tmodel = \"U-Boot FDT test\";");
	ut_assert_nextline("\taliases {");
	if (print) {
		ut_assert_nextline("\t\tbadalias = \"/bad/alias\";");
		ut_assert_nextline("\t\tsubnodealias = \"/test-node@1234/subnode\";");
		ut_assert_nextline("\t\ttestnodealias = \"/test-node@1234\";");
	}
	ut_assert_nextline("\t};");
	ut_assert_nextline("\ttest-node@1234 {");
	if (print) {
		ut_assert_nextline("\t\t#address-cells = <0x00000000>;");
		ut_assert_nextline("\t\t#size-cells = <0x00000000>;");
		ut_assert_nextline("\t\tcompatible = \"u-boot,fdt-test-device1\";");
		ut_assert_nextline("\t\tclock-names = \"fixed\", \"i2c\", \"spi\", \"uart2\", \"uart1\";");
		ut_assert_nextline("\t\tu-boot,empty-property;");
		ut_assert_nextline("\t\tclock-frequency = <0x00fde800>;");
		ut_assert_nextline("\t\tregs = <0x00001234 0x00001000>;");
		ut_assert_nextline("\t\tsubnode {");
		ut_assert_nextline("\t\t\t#address-cells = <0x00000000>;");
		ut_assert_nextline("\t\t\t#size-cells = <0x00000000>;");
		ut_assert_nextline("\t\t\tcompatible = \"u-boot,fdt-subnode-test-device\";");
		ut_assert_nextline("\t\t};");
	}
	ut_assert_nextline("\t};");
	ut_assert_nextline("};");
	ut_assertok(ut_check_console_end(uts));

	ret = fdt_test_print_list_common(uts, opc, "/test-node@1234");
	if (!ret)
		ret = fdt_test_print_list_common(uts, opc, "testnodealias");

	return 0;
}

static int fdt_test_print(struct unit_test_state *uts)
{
	return fdt_test_print_list(uts, true);
}
FDT_TEST(fdt_test_print, UT_TESTF_CONSOLE_REC);

static int fdt_test_list(struct unit_test_state *uts)
{
	return fdt_test_print_list(uts, false);
}
FDT_TEST(fdt_test_list, UT_TESTF_CONSOLE_REC);

/* Test 'fdt get value' reading an fdt */
static int fdt_test_get_value_string(struct unit_test_state *uts,
				     const char *node, const char *prop,
				     const char *idx,  const char *strres,
				     const int intres)
{
	ut_assertok(console_record_reset_enable());
	ut_assertok(run_commandf("fdt get value var %s %s %s",
				 node, prop, idx ? : ""));
	if (strres) {
		ut_asserteq_str(strres, env_get("var"));
	} else {
		ut_asserteq(intres, env_get_hex("var", 0x1234));
	}
	ut_assertok(ut_check_console_end(uts));

	return 0;
}

static int fdt_test_get_value_common(struct unit_test_state *uts,
				     const char *node)
{
	/* Test getting default element of $node node clock-names property */
	fdt_test_get_value_string(uts, node, "clock-names", NULL, "fixed", 0);

	/* Test getting 0th element of $node node clock-names property */
	fdt_test_get_value_string(uts, node, "clock-names", "0", "fixed", 0);

	/* Test getting 1st element of $node node clock-names property */
	fdt_test_get_value_string(uts, node, "clock-names", "1", "i2c", 0);

	/* Test getting 2nd element of $node node clock-names property */
	fdt_test_get_value_string(uts, node, "clock-names", "2", "spi", 0);

	/*
	 * Test getting default element of $node node regs property.
	 * The result here is highly unusual, the non-index value read from
	 * integer array is a string of concatenated values from the array,
	 * but only if the array is shorter than 40 characters. Anything
	 * longer is an error. This is a special case for handling hashes.
	 */
	fdt_test_get_value_string(uts, node, "regs", NULL, "3412000000100000", 0);

	/* Test getting 0th element of $node node regs property */
	fdt_test_get_value_string(uts, node, "regs", "0", NULL, 0x1234);

	/* Test getting 1st element of $node node regs property */
	fdt_test_get_value_string(uts, node, "regs", "1", NULL, 0x1000);

	/* Test missing 10th element of $node node clock-names property */
	ut_assertok(console_record_reset_enable());
	ut_asserteq(1, run_commandf("fdt get value ften %s clock-names 10", node));
	ut_assertok(ut_check_console_end(uts));

	/* Test missing 10th element of $node node regs property */
	ut_assertok(console_record_reset_enable());
	ut_asserteq(1, run_commandf("fdt get value ften %s regs 10", node));
	ut_assertok(ut_check_console_end(uts));

	/* Test getting default element of $node node nonexistent property */
	ut_assertok(console_record_reset_enable());
	ut_asserteq(1, run_commandf("fdt get value fnone %s nonexistent", node));
	ut_assert_nextline("libfdt fdt_getprop(): FDT_ERR_NOTFOUND");
	ut_assertok(ut_check_console_end(uts));

	return 0;
}

static int fdt_test_get_value(struct unit_test_state *uts)
{
	char fdt[4096];
	ulong addr;
	int ret;

	ut_assertok(make_fuller_fdt(uts, fdt, sizeof(fdt)));
	addr = map_to_sysmem(fdt);
	set_working_fdt_addr(addr);

	ret = fdt_test_get_value_common(uts, "/test-node@1234");
	if (!ret)
		ret = fdt_test_get_value_common(uts, "testnodealias");
	if (ret)
		return ret;

	/* Test getting default element of /nonexistent node */
	ut_assertok(console_record_reset_enable());
	ut_asserteq(1, run_command("fdt get value fnode /nonexistent nonexistent", 1));
	ut_assert_nextline("libfdt fdt_path_offset() returned FDT_ERR_NOTFOUND");
	ut_assertok(ut_check_console_end(uts));

	/* Test getting default element of bad alias */
	ut_assertok(console_record_reset_enable());
	ut_asserteq(1, run_command("fdt get value vbadalias badalias nonexistent", 1));
	ut_assert_nextline("libfdt fdt_path_offset() returned FDT_ERR_NOTFOUND");
	ut_assertok(ut_check_console_end(uts));

	/* Test getting default element of nonexistent alias */
	ut_assertok(console_record_reset_enable());
	ut_asserteq(1, run_command("fdt get value vnoalias noalias nonexistent", 1));
	ut_assert_nextline("libfdt fdt_path_offset() returned FDT_ERR_BADPATH");
	ut_assertok(ut_check_console_end(uts));

	return 0;
}
FDT_TEST(fdt_test_get_value, UT_TESTF_CONSOLE_REC);

static int fdt_test_get_name(struct unit_test_state *uts)
{
	char fdt[4096];
	ulong addr;

	ut_assertok(make_fuller_fdt(uts, fdt, sizeof(fdt)));
	addr = map_to_sysmem(fdt);
	set_working_fdt_addr(addr);

	/* Test getting name of node 0 in /, which is /aliases node */
	ut_assertok(console_record_reset_enable());
	ut_assertok(run_command("fdt get name nzero / 0", 0));
	ut_asserteq_str("aliases", env_get("nzero"));
	ut_assertok(ut_check_console_end(uts));

	/* Test getting name of node 1 in /, which is /test-node@1234 node */
	ut_assertok(console_record_reset_enable());
	ut_assertok(run_command("fdt get name none / 1", 0));
	ut_asserteq_str("test-node@1234", env_get("none"));
	ut_assertok(ut_check_console_end(uts));

	/* Test getting name of node -1 in /, which is /aliases node, same as 0 */
	ut_assertok(console_record_reset_enable());
	ut_assertok(run_command("fdt get name nmone / -1", 0));
	ut_asserteq_str("aliases", env_get("nmone"));
	ut_assertok(ut_check_console_end(uts));

	/* Test getting name of node 2 in /, which does not exist */
	ut_assertok(console_record_reset_enable());
	ut_asserteq(1, run_command("fdt get name ntwo / 2", 1));
	ut_assert_nextline("libfdt node not found");
	ut_assertok(ut_check_console_end(uts));

	/* Test getting name of node 0 in /test-node@1234, which is /subnode node */
	ut_assertok(console_record_reset_enable());
	ut_assertok(run_command("fdt get name snzero /test-node@1234 0", 0));
	ut_asserteq_str("subnode", env_get("snzero"));
	ut_assertok(run_command("fdt get name asnzero testnodealias 0", 0));
	ut_asserteq_str("subnode", env_get("asnzero"));
	ut_assertok(ut_check_console_end(uts));

	/* Test getting name of node 1 in /test-node@1234, which does not exist */
	ut_assertok(console_record_reset_enable());
	ut_asserteq(1, run_command("fdt get name snone /test-node@1234 1", 1));
	ut_assert_nextline("libfdt node not found");
	ut_asserteq(1, run_command("fdt get name asnone testnodealias 1", 1));
	ut_assert_nextline("libfdt node not found");
	ut_assertok(ut_check_console_end(uts));

	/* Test getting name of node -1 in /test-node@1234, which is /subnode node, same as 0 */
	ut_assertok(console_record_reset_enable());
	ut_assertok(run_command("fdt get name snmone /test-node@1234 -1", 0));
	ut_asserteq_str("subnode", env_get("snmone"));
	ut_assertok(run_command("fdt get name asnmone testnodealias -1", 0));
	ut_asserteq_str("subnode", env_get("asnmone"));
	ut_assertok(ut_check_console_end(uts));

	/* Test getting name of nonexistent node */
	ut_assertok(console_record_reset_enable());
	ut_asserteq(1, run_command("fdt get name nonode /nonexistent 0", 1));
	ut_assert_nextline("libfdt fdt_path_offset() returned FDT_ERR_NOTFOUND");
	ut_assertok(ut_check_console_end(uts));

	/* Test getting name of bad alias */
	ut_assertok(console_record_reset_enable());
	ut_asserteq(1, run_command("fdt get name vbadalias badalias 0", 1));
	ut_assert_nextline("libfdt fdt_path_offset() returned FDT_ERR_NOTFOUND");
	ut_assertok(ut_check_console_end(uts));

	/* Test getting name of nonexistent alias */
	ut_assertok(console_record_reset_enable());
	ut_asserteq(1, run_command("fdt get name vnoalias noalias 0", 1));
	ut_assert_nextline("libfdt fdt_path_offset() returned FDT_ERR_BADPATH");
	ut_assertok(ut_check_console_end(uts));

	return 0;
}
FDT_TEST(fdt_test_get_name, UT_TESTF_CONSOLE_REC);

static int fdt_test_get_addr_common(struct unit_test_state *uts, char *fdt,
				    const char *path, const char *prop)
{
	unsigned int offset;
	int path_offset;
	void *prop_ptr;
	int len = 0;

	path_offset = fdt_path_offset(fdt, path);
	ut_assert(path_offset >= 0);
	prop_ptr = (void *)fdt_getprop(fdt, path_offset, prop, &len);
	ut_assertnonnull(prop_ptr);
	offset = (char *)prop_ptr - fdt;

	ut_assertok(console_record_reset_enable());
	ut_assertok(run_commandf("fdt get addr pstr %s %s", path, prop));
	ut_asserteq((ulong)map_sysmem(env_get_hex("fdtaddr", 0x1234), 0),
		    (ulong)(map_sysmem(env_get_hex("pstr", 0x1234), 0) - offset));
	ut_assertok(ut_check_console_end(uts));

	return 0;
}

static int fdt_test_get_addr(struct unit_test_state *uts)
{
	char fdt[4096];
	ulong addr;

	ut_assertok(make_fuller_fdt(uts, fdt, sizeof(fdt)));
	addr = map_to_sysmem(fdt);
	set_working_fdt_addr(addr);

	/* Test getting address of root node / string property "compatible" */
	fdt_test_get_addr_common(uts, fdt, "/", "compatible");

	/* Test getting address of node /test-node@1234 stringlist property "clock-names" */
	fdt_test_get_addr_common(uts, fdt, "/test-node@1234", "clock-names");
	fdt_test_get_addr_common(uts, fdt, "testnodealias", "clock-names");

	/* Test getting address of node /test-node@1234 u32 property "clock-frequency" */
	fdt_test_get_addr_common(uts, fdt, "/test-node@1234", "clock-frequency");
	fdt_test_get_addr_common(uts, fdt, "testnodealias", "clock-frequency");

	/* Test getting address of node /test-node@1234 empty property "u-boot,empty-property" */
	fdt_test_get_addr_common(uts, fdt, "/test-node@1234", "u-boot,empty-property");
	fdt_test_get_addr_common(uts, fdt, "testnodealias", "u-boot,empty-property");

	/* Test getting address of node /test-node@1234 array property "regs" */
	fdt_test_get_addr_common(uts, fdt, "/test-node@1234", "regs");
	fdt_test_get_addr_common(uts, fdt, "testnodealias", "regs");

	/* Test getting address of node /test-node@1234/subnode non-existent property "noprop" */
	ut_assertok(console_record_reset_enable());
	ut_asserteq(1, run_command("fdt get addr pnoprop /test-node@1234/subnode noprop", 1));
	ut_assert_nextline("libfdt fdt_getprop(): FDT_ERR_NOTFOUND");
	ut_assertok(ut_check_console_end(uts));

	/* Test getting address of non-existent node /test-node@1234/nonode@1 property "noprop" */
	ut_assertok(console_record_reset_enable());
	ut_asserteq(1, run_command("fdt get addr pnonode /test-node@1234/nonode@1 noprop", 1));
	ut_assert_nextline("libfdt fdt_path_offset() returned FDT_ERR_NOTFOUND");
	ut_assertok(ut_check_console_end(uts));

	return 0;
}
FDT_TEST(fdt_test_get_addr, UT_TESTF_CONSOLE_REC);

static int fdt_test_get_size_common(struct unit_test_state *uts,
				     const char *path, const char *prop,
				     const unsigned int val)
{
	ut_assertok(console_record_reset_enable());
	if (prop) {
		ut_assertok(run_commandf("fdt get size sstr %s %s", path, prop));
	} else {
		ut_assertok(run_commandf("fdt get size sstr %s", path));
	}
	ut_asserteq(val, env_get_hex("sstr", 0x1234));
	ut_assertok(ut_check_console_end(uts));

	return 0;
}

static int fdt_test_get_size(struct unit_test_state *uts)
{
	char fdt[4096];
	ulong addr;

	ut_assertok(make_fuller_fdt(uts, fdt, sizeof(fdt)));
	addr = map_to_sysmem(fdt);
	set_working_fdt_addr(addr);

	/* Test getting size of root node / string property "compatible" */
	fdt_test_get_size_common(uts, "/", "compatible", 16);

	/* Test getting size of node /test-node@1234 stringlist property "clock-names" */
	fdt_test_get_size_common(uts, "/test-node@1234", "clock-names", 26);
	fdt_test_get_size_common(uts, "testnodealias", "clock-names", 26);

	/* Test getting size of node /test-node@1234 u32 property "clock-frequency" */
	fdt_test_get_size_common(uts, "/test-node@1234", "clock-frequency", 4);
	fdt_test_get_size_common(uts, "testnodealias", "clock-frequency", 4);

	/* Test getting size of node /test-node@1234 empty property "u-boot,empty-property" */
	fdt_test_get_size_common(uts, "/test-node@1234", "u-boot,empty-property", 0);
	fdt_test_get_size_common(uts, "testnodealias", "u-boot,empty-property", 0);

	/* Test getting size of node /test-node@1234 array property "regs" */
	fdt_test_get_size_common(uts, "/test-node@1234", "regs", 8);
	fdt_test_get_size_common(uts, "testnodealias", "regs", 8);

	/* Test getting node count of node / */
	fdt_test_get_size_common(uts, "/", NULL, 2);

	/* Test getting node count of node /test-node@1234/subnode */
	fdt_test_get_size_common(uts, "/test-node@1234/subnode", NULL, 0);
	fdt_test_get_size_common(uts, "subnodealias", NULL, 0);

	/* Test getting size of node /test-node@1234/subnode non-existent property "noprop" */
	ut_assertok(console_record_reset_enable());
	ut_asserteq(1, run_command("fdt get size pnoprop /test-node@1234/subnode noprop", 1));
	ut_assert_nextline("libfdt fdt_getprop(): FDT_ERR_NOTFOUND");
	ut_asserteq(1, run_command("fdt get size pnoprop subnodealias noprop", 1));
	ut_assert_nextline("libfdt fdt_getprop(): FDT_ERR_NOTFOUND");
	ut_assertok(ut_check_console_end(uts));

	/* Test getting size of non-existent node /test-node@1234/nonode@1 property "noprop" */
	ut_assertok(console_record_reset_enable());
	ut_asserteq(1, run_command("fdt get size pnonode /test-node@1234/nonode@1 noprop", 1));
	ut_assert_nextline("libfdt fdt_path_offset() returned FDT_ERR_NOTFOUND");
	ut_assertok(ut_check_console_end(uts));

	/* Test getting node count of non-existent node /test-node@1234/nonode@1 */
	ut_assertok(console_record_reset_enable());
	ut_asserteq(1, run_command("fdt get size pnonode /test-node@1234/nonode@1", 1));
	ut_assert_nextline("libfdt fdt_path_offset() returned FDT_ERR_NOTFOUND");
	ut_assertok(ut_check_console_end(uts));

	/* Test getting node count of bad alias badalias */
	ut_assertok(console_record_reset_enable());
	ut_asserteq(1, run_command("fdt get size pnonode badalias noprop", 1));
	ut_assert_nextline("libfdt fdt_path_offset() returned FDT_ERR_NOTFOUND");
	ut_assertok(ut_check_console_end(uts));

	/* Test getting node count of non-existent alias noalias */
	ut_assertok(console_record_reset_enable());
	ut_asserteq(1, run_command("fdt get size pnonode noalias", 1));
	ut_assert_nextline("libfdt fdt_path_offset() returned FDT_ERR_BADPATH");
	ut_assertok(ut_check_console_end(uts));

	return 0;
}
FDT_TEST(fdt_test_get_size, UT_TESTF_CONSOLE_REC);

static int fdt_test_set_single(struct unit_test_state *uts,
			       const char *path, const char *prop,
			       const char *sval, int ival, bool integer)
{
	/*
	 * Set single element string/integer/<empty> property into DT, that is:
	 * => fdt set /path property string
	 * => fdt set /path property integer
	 * => fdt set /path property
	 */
	ut_assertok(console_record_reset_enable());
	if (sval)
		ut_assertok(run_commandf("fdt set %s %s %s", path, prop, sval));
	else if (integer)
		ut_assertok(run_commandf("fdt set %s %s <%d>", path, prop, ival));
	else
		ut_assertok(run_commandf("fdt set %s %s", path, prop));

	/* Validate the property is present and has correct value. */
	ut_assertok(run_commandf("fdt get value svar %s %s", path, prop));
	if (sval)
		ut_asserteq_str(sval, env_get("svar"));
	else if (integer)
		ut_asserteq(ival, env_get_hex("svar", 0x1234));
	else
		ut_assertnull(env_get("svar"));
	ut_assertok(ut_check_console_end(uts));

	return 0;
}

static int fdt_test_set_multi(struct unit_test_state *uts,
			      const char *path, const char *prop,
			      const char *sval1, const char *sval2,
			      int ival1, int ival2)
{
	/*
	 * Set multi element string/integer array property in DT, that is:
	 * => fdt set /path property <string1 string2>
	 * => fdt set /path property <integer1 integer2>
	 *
	 * The set is done twice in here deliberately, The first set adds
	 * the property with an extra trailing element in its array to make
	 * the array longer, the second set is the expected final content of
	 * the array property. The longer array is used to verify that the
	 * new array is correctly sized and read past the new array length
	 * triggers failure.
	 */
	ut_assertok(console_record_reset_enable());
	if (sval1 && sval2) {
		ut_assertok(run_commandf("fdt set %s %s %s %s end", path, prop, sval1, sval2));
		ut_assertok(run_commandf("fdt set %s %s %s %s", path, prop, sval1, sval2));
	} else {
		ut_assertok(run_commandf("fdt set %s %s <%d %d 10>", path, prop, ival1, ival2));
		ut_assertok(run_commandf("fdt set %s %s <%d %d>", path, prop, ival1, ival2));
	}

	/*
	 * Validate the property is present and has correct value.
	 *
	 * The "end/10" above and "svarn" below is used to validate that
	 * previous 'fdt set' to longer array does not polute newly set
	 * shorter array.
	 */
	ut_assertok(run_commandf("fdt get value svar1 %s %s 0", path, prop));
	ut_assertok(run_commandf("fdt get value svar2 %s %s 1", path, prop));
	ut_asserteq(1, run_commandf("fdt get value svarn %s %s 2", path, prop));
	if (sval1 && sval2) {
		ut_asserteq_str(sval1, env_get("svar1"));
		ut_asserteq_str(sval2, env_get("svar2"));
		ut_assertnull(env_get("svarn"));
	} else {
		ut_asserteq(ival1, env_get_hex("svar1", 0x1234));
		ut_asserteq(ival2, env_get_hex("svar2", 0x1234));
		ut_assertnull(env_get("svarn"));
	}
	ut_assertok(ut_check_console_end(uts));

	return 0;
}

static int fdt_test_set_node(struct unit_test_state *uts,
			     const char *path, const char *prop)
{
	fdt_test_set_single(uts, path, prop, "new", 0, false);
	fdt_test_set_single(uts, path, prop, "rewrite", 0, false);
	fdt_test_set_single(uts, path, prop, NULL, 42, true);
	fdt_test_set_single(uts, path, prop, NULL, 0, false);
	fdt_test_set_multi(uts, path, prop, NULL, NULL, 42, 1701);
	fdt_test_set_multi(uts, path, prop, NULL, NULL, 74656, 9);
	fdt_test_set_multi(uts, path, prop, "42", "1701", 0, 0);
	fdt_test_set_multi(uts, path, prop, "74656", "9", 0, 0);

	return 0;
}

static int fdt_test_set(struct unit_test_state *uts)
{
	char fdt[8192];
	ulong addr;

	ut_assertok(make_fuller_fdt(uts, fdt, sizeof(fdt)));
	fdt_shrink_to_minimum(fdt, 4096);	/* Resize with 4096 extra bytes */
	addr = map_to_sysmem(fdt);
	set_working_fdt_addr(addr);

	/* Test setting of root node / existing property "compatible" */
	fdt_test_set_node(uts, "/", "compatible");

	/* Test setting of root node / new property "newproperty" */
	fdt_test_set_node(uts, "/", "newproperty");

	/* Test setting of subnode existing property "compatible" */
	fdt_test_set_node(uts, "/test-node@1234/subnode", "compatible");
	fdt_test_set_node(uts, "subnodealias", "compatible");

	/* Test setting of subnode new property "newproperty" */
	fdt_test_set_node(uts, "/test-node@1234/subnode", "newproperty");
	fdt_test_set_node(uts, "subnodealias", "newproperty");

	/* Test setting property of non-existent node */
	ut_assertok(console_record_reset_enable());
	ut_asserteq(1, run_command("fdt set /no-node noprop", 1));
	ut_assert_nextline("libfdt fdt_path_offset() returned FDT_ERR_NOTFOUND");
	ut_assertok(ut_check_console_end(uts));

	/* Test setting property of non-existent alias */
	ut_assertok(console_record_reset_enable());
	ut_asserteq(1, run_command("fdt set noalias noprop", 1));
	ut_assert_nextline("libfdt fdt_path_offset() returned FDT_ERR_BADPATH");
	ut_assertok(ut_check_console_end(uts));

	/* Test setting property of bad alias */
	ut_assertok(console_record_reset_enable());
	ut_asserteq(1, run_command("fdt set badalias noprop", 1));
	ut_assert_nextline("libfdt fdt_path_offset() returned FDT_ERR_NOTFOUND");
	ut_assertok(ut_check_console_end(uts));

	return 0;
}
FDT_TEST(fdt_test_set, UT_TESTF_CONSOLE_REC);

static int fdt_test_mknode(struct unit_test_state *uts)
{
	char fdt[8192];
	ulong addr;

	ut_assertok(make_fuller_fdt(uts, fdt, sizeof(fdt)));
	fdt_shrink_to_minimum(fdt, 4096);	/* Resize with 4096 extra bytes */
	addr = map_to_sysmem(fdt);
	set_working_fdt_addr(addr);

	/* Test creation of new node in / */
	ut_assertok(console_record_reset_enable());
	ut_assertok(run_commandf("fdt mknode / newnode"));
	ut_assertok(run_commandf("fdt list /newnode"));
	ut_assert_nextline("newnode {");
	ut_assert_nextline("};");
	ut_assertok(ut_check_console_end(uts));

	/* Test creation of new node in /test-node@1234 */
	ut_assertok(console_record_reset_enable());
	ut_assertok(run_commandf("fdt mknode /test-node@1234 newsubnode"));
	ut_assertok(run_commandf("fdt list /test-node@1234/newsubnode"));
	ut_assert_nextline("newsubnode {");
	ut_assert_nextline("};");
	ut_assertok(ut_check_console_end(uts));

	/* Test creation of new node in /test-node@1234 by alias */
	ut_assertok(console_record_reset_enable());
	ut_assertok(run_commandf("fdt mknode testnodealias newersubnode"));
	ut_assertok(run_commandf("fdt list testnodealias/newersubnode"));
	ut_assert_nextline("newersubnode {");
	ut_assert_nextline("};");
	ut_assertok(ut_check_console_end(uts));

	/* Test creation of new node in /test-node@1234 over existing node */
	ut_assertok(console_record_reset_enable());
	ut_asserteq(1, run_commandf("fdt mknode testnodealias newsubnode"));
	ut_assert_nextline("libfdt fdt_add_subnode(): FDT_ERR_EXISTS");
	ut_assertok(ut_check_console_end(uts));

	/* Test creation of new node in /test-node@1234 by alias over existing node */
	ut_assertok(console_record_reset_enable());
	ut_asserteq(1, run_commandf("fdt mknode testnodealias newersubnode"));
	ut_assert_nextline("libfdt fdt_add_subnode(): FDT_ERR_EXISTS");
	ut_assertok(ut_check_console_end(uts));

	/* Test creation of new node in non-existent node */
	ut_assertok(console_record_reset_enable());
	ut_asserteq(1, run_commandf("fdt mknode /no-node newnosubnode"));
	ut_assert_nextline("libfdt fdt_path_offset() returned FDT_ERR_NOTFOUND");
	ut_assertok(ut_check_console_end(uts));

	/* Test creation of new node in non-existent alias */
	ut_assertok(console_record_reset_enable());
	ut_asserteq(1, run_commandf("fdt mknode noalias newfailsubnode"));
	ut_assert_nextline("libfdt fdt_path_offset() returned FDT_ERR_BADPATH");
	ut_assertok(ut_check_console_end(uts));

	/* Test creation of new node in bad alias */
	ut_assertok(console_record_reset_enable());
	ut_asserteq(1, run_commandf("fdt mknode badalias newbadsubnode"));
	ut_assert_nextline("libfdt fdt_path_offset() returned FDT_ERR_NOTFOUND");
	ut_assertok(ut_check_console_end(uts));

	return 0;
}
FDT_TEST(fdt_test_mknode, UT_TESTF_CONSOLE_REC);

static int fdt_test_rm(struct unit_test_state *uts)
{
	char fdt[4096];
	ulong addr;

	ut_assertok(make_fuller_fdt(uts, fdt, sizeof(fdt)));
	addr = map_to_sysmem(fdt);
	set_working_fdt_addr(addr);

	/* Test removal of property in root node / */
	ut_assertok(console_record_reset_enable());
	ut_assertok(run_commandf("fdt print / compatible"));
	ut_assert_nextline("compatible = \"u-boot,fdt-test\"");
	ut_assertok(run_commandf("fdt rm / compatible"));
	ut_asserteq(1, run_commandf("fdt print / compatible"));
	ut_assert_nextline("libfdt fdt_getprop(): FDT_ERR_NOTFOUND");
	ut_assertok(ut_check_console_end(uts));

	/* Test removal of property clock-names in subnode /test-node@1234 */
	ut_assertok(console_record_reset_enable());
	ut_assertok(run_commandf("fdt print /test-node@1234 clock-names"));
	ut_assert_nextline("clock-names = \"fixed\", \"i2c\", \"spi\", \"uart2\", \"uart1\"");
	ut_assertok(run_commandf("fdt rm /test-node@1234 clock-names"));
	ut_asserteq(1, run_commandf("fdt print /test-node@1234 clock-names"));
	ut_assert_nextline("libfdt fdt_getprop(): FDT_ERR_NOTFOUND");
	ut_assertok(ut_check_console_end(uts));

	/* Test removal of property u-boot,empty-property in subnode /test-node@1234 by alias */
	ut_assertok(console_record_reset_enable());
	ut_assertok(run_commandf("fdt print testnodealias u-boot,empty-property"));
	ut_assert_nextline("testnodealias u-boot,empty-property");
	ut_assertok(run_commandf("fdt rm testnodealias u-boot,empty-property"));
	ut_asserteq(1, run_commandf("fdt print testnodealias u-boot,empty-property"));
	ut_assert_nextline("libfdt fdt_getprop(): FDT_ERR_NOTFOUND");
	ut_assertok(ut_check_console_end(uts));

	/* Test removal of non-existent property noprop in subnode /test-node@1234 */
	ut_assertok(console_record_reset_enable());
	ut_asserteq(1, run_commandf("fdt rm /test-node@1234 noprop"));
	ut_assert_nextline("libfdt fdt_delprop(): FDT_ERR_NOTFOUND");
	ut_assertok(ut_check_console_end(uts));

	/* Test removal of non-existent node /no-node@5678 */
	ut_assertok(console_record_reset_enable());
	ut_asserteq(1, run_commandf("fdt rm /no-node@5678"));
	ut_assert_nextline("libfdt fdt_path_offset() returned FDT_ERR_NOTFOUND");
	ut_assertok(ut_check_console_end(uts));

	/* Test removal of subnode /test-node@1234/subnode by alias */
	ut_assertok(console_record_reset_enable());
	ut_assertok(run_commandf("fdt rm subnodealias"));
	ut_asserteq(1, run_commandf("fdt print /test-node@1234/subnode"));
	ut_assert_nextline("libfdt fdt_path_offset() returned FDT_ERR_NOTFOUND");
	ut_assertok(ut_check_console_end(uts));

	/* Test removal of node by non-existent alias */
	ut_assertok(console_record_reset_enable());
	ut_asserteq(1, run_commandf("fdt rm noalias"));
	ut_assert_nextline("libfdt fdt_path_offset() returned FDT_ERR_BADPATH");
	ut_assertok(ut_check_console_end(uts));

	/* Test removal of node by bad alias */
	ut_assertok(console_record_reset_enable());
	ut_asserteq(1, run_commandf("fdt rm noalias"));
	ut_assert_nextline("libfdt fdt_path_offset() returned FDT_ERR_BADPATH");
	ut_assertok(ut_check_console_end(uts));

	/* Test removal of node /test-node@1234 */
	ut_assertok(console_record_reset_enable());
	ut_assertok(run_commandf("fdt rm /test-node@1234"));
	ut_asserteq(1, run_commandf("fdt print /test-node@1234"));
	ut_assert_nextline("libfdt fdt_path_offset() returned FDT_ERR_NOTFOUND");
	ut_assertok(ut_check_console_end(uts));

	/* Test removal of node / */
	ut_assertok(console_record_reset_enable());
	ut_assertok(run_commandf("fdt rm /"));
	ut_asserteq(1, run_commandf("fdt print /"));
	ut_assertok(ut_check_console_end(uts));

	return 0;
}
FDT_TEST(fdt_test_rm, UT_TESTF_CONSOLE_REC);

static int fdt_test_bootcpu(struct unit_test_state *uts)
{
	char fdt[256];
	ulong addr;
	int i;

	ut_assertok(make_test_fdt(uts, fdt, sizeof(fdt)));
	addr = map_to_sysmem(fdt);
	set_working_fdt_addr(addr);

	/* Test getting default bootcpu entry */
	ut_assertok(console_record_reset_enable());
	ut_assertok(run_commandf("fdt header get bootcpu boot_cpuid_phys"));
	ut_asserteq(0, env_get_ulong("bootcpu", 10, 0x1234));
	ut_assertok(ut_check_console_end(uts));

	/* Test setting and getting new bootcpu entry, twice, to test overwrite */
	for (i = 42; i <= 43; i++) {
		ut_assertok(console_record_reset_enable());
		ut_assertok(run_commandf("fdt bootcpu %d", i));
		ut_assertok(ut_check_console_end(uts));

		/* Test getting new bootcpu entry */
		ut_assertok(console_record_reset_enable());
		ut_assertok(run_commandf("fdt header get bootcpu boot_cpuid_phys"));
		ut_asserteq(i, env_get_ulong("bootcpu", 10, 0x1234));
		ut_assertok(ut_check_console_end(uts));
	}

	return 0;
}
FDT_TEST(fdt_test_bootcpu, UT_TESTF_CONSOLE_REC);

static int fdt_test_header_get(struct unit_test_state *uts,
			       const char *field, const unsigned long val)
{
	/* Test getting valid header entry */
	ut_assertok(console_record_reset_enable());
	ut_assertok(run_commandf("fdt header get fvar %s", field));
	ut_asserteq(val, env_get_hex("fvar", 0x1234));
	ut_assertok(ut_check_console_end(uts));

	/* Test getting malformed header entry */
	ut_assertok(console_record_reset_enable());
	ut_asserteq(1, run_commandf("fdt header get fvar typo%stypo", field));
	ut_assertok(ut_check_console_end(uts));

	return 0;
}

static int fdt_test_header(struct unit_test_state *uts)
{
	char fdt[256];
	ulong addr;

	ut_assertok(make_test_fdt(uts, fdt, sizeof(fdt)));
	addr = map_to_sysmem(fdt);
	set_working_fdt_addr(addr);

	/* Test header print */
	ut_assertok(console_record_reset_enable());
	ut_assertok(run_commandf("fdt header"));
	ut_assert_nextline("magic:\t\t\t0x%x", fdt_magic(fdt));
	ut_assert_nextline("totalsize:\t\t0x%x (%d)", fdt_totalsize(fdt), fdt_totalsize(fdt));
	ut_assert_nextline("off_dt_struct:\t\t0x%x", fdt_off_dt_struct(fdt));
	ut_assert_nextline("off_dt_strings:\t\t0x%x", fdt_off_dt_strings(fdt));
	ut_assert_nextline("off_mem_rsvmap:\t\t0x%x", fdt_off_mem_rsvmap(fdt));
	ut_assert_nextline("version:\t\t%d", fdt_version(fdt));
	ut_assert_nextline("last_comp_version:\t%d", fdt_last_comp_version(fdt));
	ut_assert_nextline("boot_cpuid_phys:\t0x%x", fdt_boot_cpuid_phys(fdt));
	ut_assert_nextline("size_dt_strings:\t0x%x", fdt_size_dt_strings(fdt));
	ut_assert_nextline("size_dt_struct:\t\t0x%x", fdt_size_dt_struct(fdt));
	ut_assert_nextline("number mem_rsv:\t\t0x%x", fdt_num_mem_rsv(fdt));
	ut_assert_nextline_empty();
	ut_assertok(ut_check_console_end(uts));

	/* Test header get */
	fdt_test_header_get(uts, "magic", fdt_magic(fdt));
	fdt_test_header_get(uts, "totalsize", fdt_totalsize(fdt));
	fdt_test_header_get(uts, "off_dt_struct", fdt_off_dt_struct(fdt));
	fdt_test_header_get(uts, "off_dt_strings", fdt_off_dt_strings(fdt));
	fdt_test_header_get(uts, "off_mem_rsvmap", fdt_off_mem_rsvmap(fdt));
	fdt_test_header_get(uts, "version", fdt_version(fdt));
	fdt_test_header_get(uts, "last_comp_version", fdt_last_comp_version(fdt));
	fdt_test_header_get(uts, "boot_cpuid_phys", fdt_boot_cpuid_phys(fdt));
	fdt_test_header_get(uts, "size_dt_strings", fdt_size_dt_strings(fdt));
	fdt_test_header_get(uts, "size_dt_struct", fdt_size_dt_struct(fdt));

	return 0;
}
FDT_TEST(fdt_test_header, UT_TESTF_CONSOLE_REC);

static int fdt_test_memory_cells(struct unit_test_state *uts,
				 const unsigned int cells)
{
	unsigned char *pada, *pads;
	unsigned char *seta, *sets;
	char fdt[8192];
	const int size = sizeof(fdt);
	fdt32_t *regs;
	ulong addr;
	char *spc;
	int i;

	/* Create DT with node /memory { regs = <0x100 0x200>; } and #*cells */
	ut_assertnonnull(regs = calloc(2 * cells, sizeof(*regs)));
	ut_assertnonnull(pada = calloc(12, cells));
	ut_assertnonnull(pads = calloc(12, cells));
	ut_assertnonnull(seta = calloc(12, cells));
	ut_assertnonnull(sets = calloc(12, cells));
	for (i = cells; i >= 1; i--) {
		regs[cells - 1] = cpu_to_fdt32(i * 0x10000);
		regs[(cells * 2) - 1] = cpu_to_fdt32(~i);
		snprintf(seta + (8 * (cells - i)), 9, "%08x", i * 0x10000);
		snprintf(sets + (8 * (cells - i)), 9, "%08x", ~i);
		spc = (i != 1) ? " " : "";
		snprintf(pada + (11 * (cells - i)), 12, "0x%08x%s", i * 0x10000, spc);
		snprintf(pads + (11 * (cells - i)), 12, "0x%08x%s", ~i, spc);
	}

	ut_assertok(fdt_create(fdt, size));
	ut_assertok(fdt_finish_reservemap(fdt));
	ut_assert(fdt_begin_node(fdt, "") >= 0);
	ut_assertok(fdt_property_u32(fdt, "#address-cells", cells));
	ut_assertok(fdt_property_u32(fdt, "#size-cells", cells));
	ut_assert(fdt_begin_node(fdt, "memory") >= 0);
	ut_assertok(fdt_property_string(fdt, "device_type", "memory"));
	ut_assertok(fdt_property(fdt, "reg", &regs, cells * 2));
	ut_assertok(fdt_end_node(fdt));
	ut_assertok(fdt_end_node(fdt));
	ut_assertok(fdt_finish(fdt));
	fdt_shrink_to_minimum(fdt, 4096);	/* Resize with 4096 extra bytes */
	addr = map_to_sysmem(fdt);
	set_working_fdt_addr(addr);

	/* Test updating the memory node */
	ut_assertok(console_record_reset_enable());
	ut_assertok(run_commandf("fdt memory 0x%s 0x%s", seta, sets));
	ut_assertok(run_commandf("fdt print /memory"));
	ut_assert_nextline("memory {");
	ut_assert_nextline("\tdevice_type = \"memory\";");
	ut_assert_nextline("\treg = <%s %s>;", pada, pads);
	ut_assert_nextline("};");
	ut_assertok(ut_check_console_end(uts));

	free(sets);
	free(seta);
	free(pads);
	free(pada);
	free(regs);

	return 0;
}

static int fdt_test_memory(struct unit_test_state *uts)
{
	/*
	 * Test memory fixup for 32 and 64 bit systems, anything bigger is
	 * so far unsupported and fails because of simple_stroull() being
	 * 64bit tops in the 'fdt memory' command implementation.
	 */
	fdt_test_memory_cells(uts, 1);
	fdt_test_memory_cells(uts, 2);

	/*
	 * The 'fdt memory' command is limited to /memory node, it does
	 * not support any other valid DT memory node format, which is
	 * either one or multiple /memory@adresss nodes. Therefore, this
	 * DT variant is not tested here.
	 */

	return 0;
}
FDT_TEST(fdt_test_memory, UT_TESTF_CONSOLE_REC);

static int fdt_test_rsvmem(struct unit_test_state *uts)
{
	char fdt[8192];
	ulong addr;

	ut_assertok(make_test_fdt(uts, fdt, sizeof(fdt)));
	fdt_shrink_to_minimum(fdt, 4096);	/* Resize with 4096 extra bytes */
	fdt_add_mem_rsv(fdt, 0x42, 0x1701);
	fdt_add_mem_rsv(fdt, 0x74656, 0x9);
	addr = map_to_sysmem(fdt);
	set_working_fdt_addr(addr);

	/* Test default reserved memory node presence */
	ut_assertok(console_record_reset_enable());
	ut_assertok(run_commandf("fdt rsvmem print"));
	ut_assert_nextline("index\t\t   start\t\t    size");
	ut_assert_nextline("------------------------------------------------");
	ut_assert_nextline("    %x\t%016x\t%016x", 0, 0x42, 0x1701);
	ut_assert_nextline("    %x\t%016x\t%016x", 1, 0x74656, 0x9);
	ut_assertok(ut_check_console_end(uts));

	/* Test add new reserved memory node */
	ut_assertok(console_record_reset_enable());
	ut_assertok(run_commandf("fdt rsvmem add 0x1234 0x5678"));
	ut_assertok(run_commandf("fdt rsvmem print"));
	ut_assert_nextline("index\t\t   start\t\t    size");
	ut_assert_nextline("------------------------------------------------");
	ut_assert_nextline("    %x\t%016x\t%016x", 0, 0x42, 0x1701);
	ut_assert_nextline("    %x\t%016x\t%016x", 1, 0x74656, 0x9);
	ut_assert_nextline("    %x\t%016x\t%016x", 2, 0x1234, 0x5678);
	ut_assertok(ut_check_console_end(uts));

	/* Test delete reserved memory node */
	ut_assertok(console_record_reset_enable());
	ut_assertok(run_commandf("fdt rsvmem delete 0"));
	ut_assertok(run_commandf("fdt rsvmem print"));
	ut_assert_nextline("index\t\t   start\t\t    size");
	ut_assert_nextline("------------------------------------------------");
	ut_assert_nextline("    %x\t%016x\t%016x", 0, 0x74656, 0x9);
	ut_assert_nextline("    %x\t%016x\t%016x", 1, 0x1234, 0x5678);
	ut_assertok(ut_check_console_end(uts));

	/* Test re-add new reserved memory node */
	ut_assertok(console_record_reset_enable());
	ut_assertok(run_commandf("fdt rsvmem add 0x42 0x1701"));
	ut_assertok(run_commandf("fdt rsvmem print"));
	ut_assert_nextline("index\t\t   start\t\t    size");
	ut_assert_nextline("------------------------------------------------");
	ut_assert_nextline("    %x\t%016x\t%016x", 0, 0x74656, 0x9);
	ut_assert_nextline("    %x\t%016x\t%016x", 1, 0x1234, 0x5678);
	ut_assert_nextline("    %x\t%016x\t%016x", 2, 0x42, 0x1701);
	ut_assertok(ut_check_console_end(uts));

	/* Test delete nonexistent reserved memory node */
	ut_assertok(console_record_reset_enable());
	ut_asserteq(1, run_commandf("fdt rsvmem delete 10"));
	ut_assert_nextline("libfdt fdt_del_mem_rsv(): FDT_ERR_NOTFOUND");
	ut_assertok(ut_check_console_end(uts));

	return 0;
}
FDT_TEST(fdt_test_rsvmem, UT_TESTF_CONSOLE_REC);

static int fdt_test_chosen(struct unit_test_state *uts)
{
	const char *env_bootargs = env_get("bootargs");
	char fdt[8192];
	ulong addr;

	ut_assertok(make_test_fdt(uts, fdt, sizeof(fdt)));
	fdt_shrink_to_minimum(fdt, 4096);	/* Resize with 4096 extra bytes */
	addr = map_to_sysmem(fdt);
	set_working_fdt_addr(addr);

	/* Test default chosen node presence, fail as there is no /chosen node */
	ut_assertok(console_record_reset_enable());
	ut_asserteq(1, run_commandf("fdt print /chosen"));
	ut_assert_nextline("libfdt fdt_path_offset() returned FDT_ERR_NOTFOUND");
	ut_assertok(ut_check_console_end(uts));

	/* Test add new chosen node without initrd */
	ut_assertok(console_record_reset_enable());
	ut_assertok(run_commandf("fdt chosen"));
	ut_assertok(run_commandf("fdt print /chosen"));
	ut_assert_nextline("chosen {");
	ut_assert_nextlinen("\tu-boot,version = "); /* Ignore the version string */
	if (env_bootargs)
		ut_assert_nextline("\tbootargs = \"%s\";", env_bootargs);
	ut_assert_nextline("};");
	ut_assertok(ut_check_console_end(uts));

	/* Test add new chosen node with initrd */
	ut_assertok(console_record_reset_enable());
	ut_assertok(run_commandf("fdt chosen 0x1234 0x5678"));
	ut_assertok(run_commandf("fdt print /chosen"));
	ut_assert_nextline("chosen {");
	ut_assert_nextline("\tlinux,initrd-end = <0x%08x 0x%08x>;",
			   upper_32_bits(0x1234 + 0x5678 - 1),
			   lower_32_bits(0x1234 + 0x5678 - 1));
	ut_assert_nextline("\tlinux,initrd-start = <0x%08x 0x%08x>;",
			   upper_32_bits(0x1234), lower_32_bits(0x1234));
	ut_assert_nextlinen("\tu-boot,version = "); /* Ignore the version string */
	if (env_bootargs)
		ut_assert_nextline("\tbootargs = \"%s\";", env_bootargs);
	ut_assert_nextline("};");
	ut_assertok(ut_check_console_end(uts));

	return 0;
}
FDT_TEST(fdt_test_chosen, UT_TESTF_CONSOLE_REC);

static int fdt_test_apply(struct unit_test_state *uts)
{
	char fdt[8192], fdto[8192];
	ulong addr, addro;

	/* Create base DT with __symbols__ node */
	ut_assertok(fdt_create(fdt, sizeof(fdt)));
	ut_assertok(fdt_finish_reservemap(fdt));
	ut_assert(fdt_begin_node(fdt, "") >= 0);
	ut_assert(fdt_begin_node(fdt, "__symbols__") >= 0);
	ut_assertok(fdt_end_node(fdt));
	ut_assertok(fdt_end_node(fdt));
	ut_assertok(fdt_finish(fdt));
	fdt_shrink_to_minimum(fdt, 4096);	/* Resize with 4096 extra bytes */
	addr = map_to_sysmem(fdt);
	set_working_fdt_addr(addr);

	/* Create DTO which adds single property to root node / */
	ut_assertok(fdt_create(fdto, sizeof(fdto)));
	ut_assertok(fdt_finish_reservemap(fdto));
	ut_assert(fdt_begin_node(fdto, "") >= 0);
	ut_assert(fdt_begin_node(fdto, "fragment") >= 0);
	ut_assertok(fdt_property_string(fdto, "target-path", "/"));
	ut_assert(fdt_begin_node(fdto, "__overlay__") >= 0);
	ut_assertok(fdt_property_string(fdto, "newstring", "newvalue"));
	ut_assertok(fdt_end_node(fdto));
	ut_assertok(fdt_end_node(fdto));
	ut_assertok(fdt_finish(fdto));
	addro = map_to_sysmem(fdto);

	/* Test default DT print */
	ut_assertok(console_record_reset_enable());
	ut_assertok(run_commandf("fdt print /"));
	ut_assert_nextline("/ {");
	ut_assert_nextline("\t__symbols__ {");
	ut_assert_nextline("\t};");
	ut_assert_nextline("};");
	ut_assertok(ut_check_console_end(uts));

	/* Test simple DTO application */
	ut_assertok(console_record_reset_enable());
	ut_assertok(run_commandf("fdt apply 0x%08lx", addro));
	ut_assertok(run_commandf("fdt print /"));
	ut_assert_nextline("/ {");
	ut_assert_nextline("\tnewstring = \"newvalue\";");
	ut_assert_nextline("\t__symbols__ {");
	ut_assert_nextline("\t};");
	ut_assert_nextline("};");
	ut_assertok(ut_check_console_end(uts));

	/*
	 * Create complex DTO which:
	 * - modifies newstring property in root node /
	 * - adds new properties to root node /
	 * - adds new subnode with properties to root node /
	 * - adds phandle to the subnode and therefore __symbols__ node
	 */
	ut_assertok(fdt_create(fdto, sizeof(fdto)));
	ut_assertok(fdt_finish_reservemap(fdto));
	ut_assert(fdt_begin_node(fdto, "") >= 0);
	ut_assertok(fdt_property_cell(fdto, "#address-cells", 1));
	ut_assertok(fdt_property_cell(fdto, "#size-cells", 0));

	ut_assert(fdt_begin_node(fdto, "fragment@0") >= 0);
	ut_assertok(fdt_property_string(fdto, "target-path", "/"));
	ut_assert(fdt_begin_node(fdto, "__overlay__") >= 0);
	ut_assertok(fdt_property_string(fdto, "newstring", "newervalue"));
	ut_assertok(fdt_property_u32(fdto, "newu32", 0x12345678));
	ut_assertok(fdt_property(fdto, "empty-property", NULL, 0));
	ut_assert(fdt_begin_node(fdto, "subnode") >= 0);
	ut_assertok(fdt_property_string(fdto, "subnewstring", "newervalue"));
	ut_assertok(fdt_property_u32(fdto, "subnewu32", 0x12345678));
	ut_assertok(fdt_property(fdto, "subempty-property", NULL, 0));
	ut_assertok(fdt_property_u32(fdto, "phandle", 0x01));
	ut_assertok(fdt_end_node(fdto));
	ut_assertok(fdt_end_node(fdto));
	ut_assertok(fdt_end_node(fdto));

	ut_assert(fdt_begin_node(fdto, "__symbols__") >= 0);
	ut_assertok(fdt_property_string(fdto, "subnodephandle", "/fragment@0/__overlay__/subnode"));
	ut_assertok(fdt_end_node(fdto));
	ut_assertok(fdt_finish(fdto));
	addro = map_to_sysmem(fdto);

	/* Test complex DTO application */
	ut_assertok(console_record_reset_enable());
	ut_assertok(run_commandf("fdt apply 0x%08lx", addro));
	ut_assertok(run_commandf("fdt print /"));
	ut_assert_nextline("/ {");
	ut_assert_nextline("\tempty-property;");
	ut_assert_nextline("\tnewu32 = <0x12345678>;");
	ut_assert_nextline("\tnewstring = \"newervalue\";");
	ut_assert_nextline("\tsubnode {");
	ut_assert_nextline("\t\tphandle = <0x00000001>;");
	ut_assert_nextline("\t\tsubempty-property;");
	ut_assert_nextline("\t\tsubnewu32 = <0x12345678>;");
	ut_assert_nextline("\t\tsubnewstring = \"newervalue\";");
	ut_assert_nextline("\t};");
	ut_assert_nextline("\t__symbols__ {");
	ut_assert_nextline("\t\tsubnodephandle = \"/subnode\";");
	ut_assert_nextline("\t};");
	ut_assert_nextline("};");
	ut_assertok(ut_check_console_end(uts));

	/*
	 * Create complex DTO which:
	 * - modifies subnewu32 property in subnode via phandle and uses __fixups__ node
	 */
	ut_assertok(fdt_create(fdto, sizeof(fdto)));
	ut_assertok(fdt_finish_reservemap(fdto));
	ut_assert(fdt_begin_node(fdto, "") >= 0);
	ut_assertok(fdt_property_cell(fdto, "#address-cells", 1));
	ut_assertok(fdt_property_cell(fdto, "#size-cells", 0));

	ut_assert(fdt_begin_node(fdto, "fragment@0") >= 0);
	ut_assertok(fdt_property_u32(fdto, "target", 0xffffffff));
	ut_assert(fdt_begin_node(fdto, "__overlay__") >= 0);
	ut_assertok(fdt_property_u32(fdto, "subnewu32", 0xabcdef01));
	ut_assertok(fdt_end_node(fdto));
	ut_assertok(fdt_end_node(fdto));

	ut_assert(fdt_begin_node(fdto, "__fixups__") >= 0);
	ut_assertok(fdt_property_string(fdto, "subnodephandle", "/fragment@0:target:0"));
	ut_assertok(fdt_end_node(fdto));
	ut_assertok(fdt_end_node(fdto));
	ut_assertok(fdt_finish(fdto));
	addro = map_to_sysmem(fdto);

	/* Test complex DTO application */
	ut_assertok(console_record_reset_enable());
	ut_assertok(run_commandf("fdt apply 0x%08lx", addro));
	ut_assertok(run_commandf("fdt print /"));
	ut_assert_nextline("/ {");
	ut_assert_nextline("\tempty-property;");
	ut_assert_nextline("\tnewu32 = <0x12345678>;");
	ut_assert_nextline("\tnewstring = \"newervalue\";");
	ut_assert_nextline("\tsubnode {");
	ut_assert_nextline("\t\tphandle = <0x00000001>;");
	ut_assert_nextline("\t\tsubempty-property;");
	ut_assert_nextline("\t\tsubnewu32 = <0xabcdef01>;");
	ut_assert_nextline("\t\tsubnewstring = \"newervalue\";");
	ut_assert_nextline("\t};");
	ut_assert_nextline("\t__symbols__ {");
	ut_assert_nextline("\t\tsubnodephandle = \"/subnode\";");
	ut_assert_nextline("\t};");
	ut_assert_nextline("};");
	ut_assertok(ut_check_console_end(uts));

	return 0;
}
FDT_TEST(fdt_test_apply, UT_TESTF_CONSOLE_REC);

int do_ut_fdt(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	struct unit_test *tests = UNIT_TEST_SUITE_START(fdt_test);
	const int n_ents = UNIT_TEST_SUITE_COUNT(fdt_test);

	return cmd_ut_category("fdt", "fdt_test_", tests, n_ents, argc, argv);
}
