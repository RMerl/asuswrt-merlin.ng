// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2018
 * Mario Six, Guntermann & Drunck GmbH, mario.six@gdsys.cc
 */

#include <common.h>
#include <hexdump.h>
#include <test/lib.h>
#include <test/test.h>
#include <test/ut.h>

static int lib_test_hex_to_bin(struct unit_test_state *uts)
{
	ut_asserteq(0x0, hex_to_bin('0'));
	ut_asserteq(0x1, hex_to_bin('1'));
	ut_asserteq(0x2, hex_to_bin('2'));
	ut_asserteq(0x3, hex_to_bin('3'));
	ut_asserteq(0x4, hex_to_bin('4'));
	ut_asserteq(0x5, hex_to_bin('5'));
	ut_asserteq(0x6, hex_to_bin('6'));
	ut_asserteq(0x7, hex_to_bin('7'));
	ut_asserteq(0x8, hex_to_bin('8'));
	ut_asserteq(0x9, hex_to_bin('9'));
	ut_asserteq(0xa, hex_to_bin('a'));
	ut_asserteq(0xb, hex_to_bin('b'));
	ut_asserteq(0xc, hex_to_bin('c'));
	ut_asserteq(0xd, hex_to_bin('d'));
	ut_asserteq(0xe, hex_to_bin('e'));
	ut_asserteq(0xf, hex_to_bin('f'));
	ut_asserteq(-1, hex_to_bin('g'));

	return 0;
}

LIB_TEST(lib_test_hex_to_bin, 0);

static int lib_test_hex2bin(struct unit_test_state *uts)
{
	u8 dst[4];

	hex2bin(dst, "649421de", 4);
	ut_asserteq_mem("\x64\x94\x21\xde", dst, 4);
	hex2bin(dst, "aa2e7545", 4);
	ut_asserteq_mem("\xaa\x2e\x75\x45", dst, 4);
	hex2bin(dst, "75453bc5", 4);
	ut_asserteq_mem("\x75\x45\x3b\xc5", dst, 4);
	hex2bin(dst, "a16884c3", 4);
	ut_asserteq_mem("\xa1\x68\x84\xc3", dst, 4);
	hex2bin(dst, "156b2e5e", 4);
	ut_asserteq_mem("\x15\x6b\x2e\x5e", dst, 4);
	hex2bin(dst, "2e035fff", 4);
	ut_asserteq_mem("\x2e\x03\x5f\xff", dst, 4);
	hex2bin(dst, "0ffce99f", 4);
	ut_asserteq_mem("\x0f\xfc\xe9\x9f", dst, 4);
	hex2bin(dst, "d3999443", 4);
	ut_asserteq_mem("\xd3\x99\x94\x43", dst, 4);
	hex2bin(dst, "91dd87bc", 4);
	ut_asserteq_mem("\x91\xdd\x87\xbc", dst, 4);
	hex2bin(dst, "7fec8963", 4);
	ut_asserteq_mem("\x7f\xec\x89\x63", dst, 4);

	return 0;
}

LIB_TEST(lib_test_hex2bin, 0);

static int lib_test_bin2hex(struct unit_test_state *uts)
{
	char dst[8 + 1] = "\0";

	bin2hex(dst, "\x64\x94\x21\xde", 4);
	ut_asserteq_str("649421de", dst);
	bin2hex(dst, "\xaa\x2e\x75\x45", 4);
	ut_asserteq_str("aa2e7545", dst);
	bin2hex(dst, "\x75\x45\x3b\xc5", 4);
	ut_asserteq_str("75453bc5", dst);
	bin2hex(dst, "\xa1\x68\x84\xc3", 4);
	ut_asserteq_str("a16884c3", dst);
	bin2hex(dst, "\x15\x6b\x2e\x5e", 4);
	ut_asserteq_str("156b2e5e", dst);
	bin2hex(dst, "\x2e\x03\x5f\xff", 4);
	ut_asserteq_str("2e035fff", dst);
	bin2hex(dst, "\x0f\xfc\xe9\x9f", 4);
	ut_asserteq_str("0ffce99f", dst);
	bin2hex(dst, "\xd3\x99\x94\x43", 4);
	ut_asserteq_str("d3999443", dst);
	bin2hex(dst, "\x91\xdd\x87\xbc", 4);
	ut_asserteq_str("91dd87bc", dst);
	bin2hex(dst, "\x7f\xec\x89\x63", 4);
	ut_asserteq_str("7fec8963", dst);

	return 0;
}

LIB_TEST(lib_test_bin2hex, 0);
