/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2003-2010  Marcel Holtmann <marcel@holtmann.org>
 *
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/socket.h>

#include "lib/bluetooth.h"
#include "lib/hci.h"
#include "lib/hci_lib.h"

#include "csr.h"

struct psr_data {
	uint16_t pskey;
	uint8_t *value;
	uint8_t size;
	struct psr_data *next;
};

static struct psr_data *head = NULL, *tail = NULL;

static struct {
	uint16_t id;
	char *str;
} csr_map[] = {
	{   66, "HCI 9.8"	},
	{   97, "HCI 10.3"	},
	{  101, "HCI 10.5"	},
	{  111,	"HCI 11.0"	},
	{  112,	"HCI 11.1"	},
	{  114,	"HCI 11.2"	},
	{  115,	"HCI 11.3"	},
	{  117,	"HCI 12.0"	},
	{  119,	"HCI 12.1"	},
	{  133,	"HCI 12.2"	},
	{  134,	"HCI 12.3"	},
	{  162,	"HCI 12.4"	},
	{  165,	"HCI 12.5"	},
	{  169,	"HCI 12.6"	},
	{  188,	"HCI 12.7"	},
	{  218,	"HCI 12.8"	},
	{  283,	"HCI 12.9"	},
	{  203,	"HCI 13.2"	},
	{  204,	"HCI 13.2"	},
	{  210,	"HCI 13.3"	},
	{  211,	"HCI 13.3"	},
	{  213,	"HCI 13.4"	},
	{  214,	"HCI 13.4"	},
	{  225,	"HCI 13.5"	},
	{  226,	"HCI 13.5"	},
	{  237,	"HCI 13.6"	},
	{  238,	"HCI 13.6"	},
	{  242,	"HCI 14.0"	},
	{  243,	"HCI 14.0"	},
	{  244,	"HCI 14.0"	},
	{  245,	"HCI 14.0"	},
	{  254,	"HCI 13.7"	},
	{  255,	"HCI 13.7"	},
	{  264,	"HCI 14.1"	},
	{  265,	"HCI 14.1"	},
	{  267,	"HCI 14.2"	},
	{  268,	"HCI 14.2"	},
	{  272,	"HCI 14.3"	},
	{  273,	"HCI 14.3"	},
	{  274,	"HCI 13.8"	},
	{  275,	"HCI 13.8"	},
	{  286,	"HCI 13.9"	},
	{  287,	"HCI 13.9"	},
	{  309,	"HCI 13.10"	},
	{  310,	"HCI 13.10"	},
	{  313,	"HCI 14.4"	},
	{  314,	"HCI 14.4"	},
	{  323,	"HCI 14.5"	},
	{  324,	"HCI 14.5"	},
	{  336,	"HCI 14.6"	},
	{  337,	"HCI 14.6"	},
	{  351,	"HCI 13.11"	},
	{  352,	"HCI 13.11"	},
	{  362,	"HCI 15.0"	},
	{  363,	"HCI 15.0"	},
	{  364,	"HCI 15.0"	},
	{  365,	"HCI 15.0"	},
	{  373,	"HCI 14.7"	},
	{  374,	"HCI 14.7"	},
	{  379,	"HCI 15.1"	},
	{  380,	"HCI 15.1"	},
	{  381,	"HCI 15.1"	},
	{  382,	"HCI 15.1"	},
	{  392,	"HCI 15.2"	},
	{  393,	"HCI 15.2"	},
	{  394,	"HCI 15.2"	},
	{  395,	"HCI 15.2"	},
	{  436,	"HCI 16.0"	},
	{  437,	"HCI 16.0"	},
	{  438,	"HCI 16.0"	},
	{  439,	"HCI 16.0"	},
	{  443,	"HCI 15.3"	},
	{  444,	"HCI 15.3"	},
	{  465,	"HCI 16.1"	},
	{  466,	"HCI 16.1"	},
	{  467,	"HCI 16.1"	},
	{  468,	"HCI 16.1"	},
	{  487,	"HCI 14.8"	},
	{  488,	"HCI 14.8"	},
	{  492,	"HCI 16.2"	},
	{  493,	"HCI 16.2"	},
	{  495,	"HCI 16.2"	},
	{  496,	"HCI 16.2"	},
	{  502,	"HCI 16.1.1"	},
	{  503,	"HCI 16.1.1"	},
	{  504,	"HCI 16.1.1"	},
	{  505,	"HCI 16.1.1"	},
	{  506,	"HCI 16.1.2"	},
	{  507,	"HCI 16.1.2"	},
	{  508,	"HCI 16.1.2"	},
	{  509,	"HCI 16.1.2"	},
	{  516,	"HCI 16.3"	},
	{  517,	"HCI 16.3"	},
	{  518,	"HCI 16.3"	},
	{  519,	"HCI 16.3"	},
	{  523,	"HCI 16.4"	},
	{  524,	"HCI 16.4"	},
	{  525,	"HCI 16.4"	},
	{  526,	"HCI 16.4"	},
	{  553,	"HCI 15.3"	},
	{  554,	"HCI 15.3"	},
	{  562,	"HCI 16.5"	},
	{  563,	"HCI 16.5"	},
	{  564,	"HCI 16.5"	},
	{  565,	"HCI 16.5"	},
	{  593,	"HCI 17.0"	},
	{  594,	"HCI 17.0"	},
	{  595,	"HCI 17.0"	},
	{  599,	"HCI 17.0"	},
	{  600,	"HCI 17.0"	},
	{  608,	"HCI 13.10.1"	},
	{  609,	"HCI 13.10.1"	},
	{  613,	"HCI 17.1"	},
	{  614,	"HCI 17.1"	},
	{  615,	"HCI 17.1"	},
	{  616,	"HCI 17.1"	},
	{  618,	"HCI 17.1"	},
	{  624,	"HCI 17.2"	},
	{  625,	"HCI 17.2"	},
	{  626,	"HCI 17.2"	},
	{  627,	"HCI 17.2"	},
	{  637,	"HCI 16.6"	},
	{  638,	"HCI 16.6"	},
	{  639,	"HCI 16.6"	},
	{  640,	"HCI 16.6"	},
	{  642,	"HCI 13.10.2"	},
	{  643,	"HCI 13.10.2"	},
	{  644,	"HCI 13.10.3"	},
	{  645,	"HCI 13.10.3"	},
	{  668,	"HCI 13.10.4"	},
	{  669,	"HCI 13.10.4"	},
	{  681,	"HCI 16.7"	},
	{  682,	"HCI 16.7"	},
	{  683,	"HCI 16.7"	},
	{  684,	"HCI 16.7"	},
	{  704,	"HCI 16.8"	},
	{  718,	"HCI 16.4.1"	},
	{  719,	"HCI 16.4.1"	},
	{  720,	"HCI 16.4.1"	},
	{  721,	"HCI 16.4.1"	},
	{  722,	"HCI 16.7.1"	},
	{  723,	"HCI 16.7.1"	},
	{  724,	"HCI 16.7.1"	},
	{  725,	"HCI 16.7.1"	},
	{  731,	"HCI 16.7.2"	},
	{  732,	"HCI 16.7.2"	},
	{  733,	"HCI 16.7.2"	},
	{  734,	"HCI 16.7.2"	},
	{  735,	"HCI 16.4.2"	},
	{  736,	"HCI 16.4.2"	},
	{  737,	"HCI 16.4.2"	},
	{  738,	"HCI 16.4.2"	},
	{  750,	"HCI 16.7.3"	},
	{  751,	"HCI 16.7.3"	},
	{  752,	"HCI 16.7.3"	},
	{  753,	"HCI 16.7.3"	},
	{  760,	"HCI 16.7.4"	},
	{  761,	"HCI 16.7.4"	},
	{  762,	"HCI 16.7.4"	},
	{  763,	"HCI 16.7.4"	},
	{  770,	"HCI 16.9"	},
	{  771,	"HCI 16.9"	},
	{  772,	"HCI 16.9"	},
	{  773,	"HCI 16.9"	},
	{  774,	"HCI 17.3"	},
	{  775,	"HCI 17.3"	},
	{  776,	"HCI 17.3"	},
	{  777,	"HCI 17.3"	},
	{  781,	"HCI 16.7.5"	},
	{  786,	"HCI 16.10"	},
	{  787,	"HCI 16.10"	},
	{  788,	"HCI 16.10"	},
	{  789,	"HCI 16.10"	},
	{  791,	"HCI 16.4.3"	},
	{  792,	"HCI 16.4.3"	},
	{  793,	"HCI 16.4.3"	},
	{  794,	"HCI 16.4.3"	},
	{  798,	"HCI 16.11"	},
	{  799,	"HCI 16.11"	},
	{  800,	"HCI 16.11"	},
	{  801,	"HCI 16.11"	},
	{  806,	"HCI 16.7.5"	},
	{  807,	"HCI 16.12"	},
	{  808,	"HCI 16.12"	},
	{  809,	"HCI 16.12"	},
	{  810,	"HCI 16.12"	},
	{  817,	"HCI 16.13"	},
	{  818,	"HCI 16.13"	},
	{  819,	"HCI 16.13"	},
	{  820,	"HCI 16.13"	},
	{  823,	"HCI 13.10.5"	},
	{  824,	"HCI 13.10.5"	},
	{  826,	"HCI 16.14"	},
	{  827,	"HCI 16.14"	},
	{  828,	"HCI 16.14"	},
	{  829,	"HCI 16.14"	},
	{  843,	"HCI 17.3.1"	},
	{  856,	"HCI 17.3.2"	},
	{  857,	"HCI 17.3.2"	},
	{  858,	"HCI 17.3.2"	},
	{ 1120, "HCI 17.11"	},
	{ 1168, "HCI 18.1"	},
	{ 1169, "HCI 18.1"	},
	{ 1241, "HCI 18.x"	},
	{ 1298, "HCI 18.2"	},
	{ 1354, "HCI 18.2"	},
	{ 1392, "HCI 18.2"	},
	{ 1393, "HCI 18.2"	},
	{ 1501, "HCI 18.2"	},
	{ 1503, "HCI 18.2"	},
	{ 1504, "HCI 18.2"	},
	{ 1505, "HCI 18.2"	},
	{ 1506, "HCI 18.2"	},
	{ 1520, "HCI 18.2"	},
	{ 1586, "HCI 18.2"	},
	{ 1591, "HCI 18.2"	},
	{ 1592, "HCI 18.2"	},
	{ 1593, "HCI 18.2.1"	},
	{ 1733, "HCI 18.3"	},
	{ 1734, "HCI 18.3"	},
	{ 1735, "HCI 18.3"	},
	{ 1737, "HCI 18.3"	},
	{ 1915, "HCI 19.2"	},
	{ 1916, "HCI 19.2"	},
	{ 1958, "HCI 19.2"	},
	{ 1981, "Unified 20a"	},
	{ 1982, "Unified 20a"	},
	{ 1989, "HCI 18.4"	},
	{ 2062, "Unified 20a1"	},
	{ 2063, "Unified 20a1"	},
	{ 2067, "Unified 18f"	},
	{ 2068, "Unified 18f"	},
	{ 2243, "Unified 18e"	},
	{ 2244, "Unified 18e"	},
	{ 2258, "Unified 20d"	},
	{ 2259, "Unified 20d"	},
	{ 2361, "Unified 20e"	},
	{ 2362, "Unified 20e"	},
	{ 2386, "Unified 21a"	},
	{ 2387, "Unified 21a"	},
	{ 2423, "Unified 21a"	},
	{ 2424, "Unified 21a"	},
	{ 2623, "Unified 21c"	},
	{ 2624, "Unified 21c"	},
	{ 2625, "Unified 21c"	},
	{ 2626, "Unified 21c"	},
	{ 2627, "Unified 21c"	},
	{ 2628, "Unified 21c"	},
	{ 2629, "Unified 21c"	},
	{ 2630, "Unified 21c"	},
	{ 2631, "Unified 21c"	},
	{ 2632, "Unified 21c"	},
	{ 2633, "Unified 21c"	},
	{ 2634, "Unified 21c"	},
	{ 2635, "Unified 21c"	},
	{ 2636, "Unified 21c"	},
	{ 2649, "Unified 21c"	},
	{ 2650, "Unified 21c"	},
	{ 2651, "Unified 21c"	},
	{ 2652, "Unified 21c"	},
	{ 2653, "Unified 21c"	},
	{ 2654, "Unified 21c"	},
	{ 2655, "Unified 21c"	},
	{ 2656, "Unified 21c"	},
	{ 2658, "Unified 21c"	},
	{ 3057, "Unified 21d"	},
	{ 3058, "Unified 21d"	},
	{ 3059, "Unified 21d"	},
	{ 3060, "Unified 21d"	},
	{ 3062, "Unified 21d"	},
	{ 3063, "Unified 21d"	},
	{ 3064, "Unified 21d"	},
	{ 3164, "Unified 21e"	},
	{ 3413, "Unified 21f"	},
	{ 3414, "Unified 21f"	},
	{ 3415, "Unified 21f"	},
	{ 3424, "Unified 21f"	},
	{ 3454, "Unified 21f"	},
	{ 3684, "Unified 21f"	},
	{ 3764, "Unified 21f"	},
	{ 4276, "Unified 22b"	},
	{ 4277, "Unified 22b"	},
	{ 4279, "Unified 22b"	},
	{ 4281, "Unified 22b"	},
	{ 4282, "Unified 22b"	},
	{ 4283, "Unified 22b"	},
	{ 4284, "Unified 22b"	},
	{ 4285, "Unified 22b"	},
	{ 4289, "Unified 22b"	},
	{ 4290, "Unified 22b"	},
	{ 4291, "Unified 22b"	},
	{ 4292, "Unified 22b"	},
	{ 4293, "Unified 22b"	},
	{ 4294, "Unified 22b"	},
	{ 4295, "Unified 22b"	},
	{ 4363, "Unified 22c"	},
	{ 4373, "Unified 22c"	},
	{ 4374, "Unified 22c"	},
	{ 4532, "Unified 22d"	},
	{ 4533, "Unified 22d"	},
	{ 4698, "Unified 23c"	},
	{ 4839, "Unified 23c"	},
	{ 4841, "Unified 23c"	},
	{ 4866, "Unified 23c"	},
	{ 4867, "Unified 23c"	},
	{ 4868, "Unified 23c"	},
	{ 4869, "Unified 23c"	},
	{ 4870, "Unified 23c"	},
	{ 4871, "Unified 23c"	},
	{ 4872, "Unified 23c"	},
	{ 4874, "Unified 23c"	},
	{ 4875, "Unified 23c"	},
	{ 4876, "Unified 23c"	},
	{ 4877, "Unified 23c"	},
	{ 2526, "Marcel 1 (2005-09-26)"	},
	{ 2543, "Marcel 2 (2005-09-28)"	},
	{ 2622, "Marcel 3 (2005-10-27)"	},
	{ 3326, "Marcel 4 (2006-06-16)"	},
	{ 3612, "Marcel 5 (2006-10-24)"	},
	{ 4509, "Marcel 6 (2007-06-11)"	},
	{ 5417, "Marcel 7 (2008-08-26)" },
	{  195, "Sniff 1 (2001-11-27)"	},
	{  220, "Sniff 2 (2002-01-03)"	},
	{  269, "Sniff 3 (2002-02-22)"	},
	{  270, "Sniff 4 (2002-02-26)"	},
	{  284, "Sniff 5 (2002-03-12)"	},
	{  292, "Sniff 6 (2002-03-20)"	},
	{  305, "Sniff 7 (2002-04-12)"	},
	{  306, "Sniff 8 (2002-04-12)"	},
	{  343, "Sniff 9 (2002-05-02)"	},
	{  346, "Sniff 10 (2002-05-03)"	},
	{  355, "Sniff 11 (2002-05-16)"	},
	{  256, "Sniff 11 (2002-05-16)"	},
	{  390, "Sniff 12 (2002-06-26)"	},
	{  450, "Sniff 13 (2002-08-16)"	},
	{  451, "Sniff 13 (2002-08-16)"	},
	{  533, "Sniff 14 (2002-10-11)"	},
	{  580, "Sniff 15 (2002-11-14)"	},
	{  623, "Sniff 16 (2002-12-12)"	},
	{  678, "Sniff 17 (2003-01-29)"	},
	{  847, "Sniff 18 (2003-04-17)"	},
	{  876, "Sniff 19 (2003-06-10)"	},
	{  997, "Sniff 22 (2003-09-05)"	},
	{ 1027, "Sniff 23 (2003-10-03)"	},
	{ 1029, "Sniff 24 (2003-10-03)"	},
	{ 1112, "Sniff 25 (2003-12-03)"	},
	{ 1113, "Sniff 25 (2003-12-03)"	},
	{ 1133, "Sniff 26 (2003-12-18)"	},
	{ 1134, "Sniff 26 (2003-12-18)"	},
	{ 1223, "Sniff 27 (2004-03-08)"	},
	{ 1224, "Sniff 27 (2004-03-08)"	},
	{ 1319, "Sniff 31 (2004-04-22)"	},
	{ 1320, "Sniff 31 (2004-04-22)"	},
	{ 1427, "Sniff 34 (2004-06-16)"	},
	{ 1508, "Sniff 35 (2004-07-19)"	},
	{ 1509, "Sniff 35 (2004-07-19)"	},
	{ 1587, "Sniff 36 (2004-08-18)"	},
	{ 1588, "Sniff 36 (2004-08-18)"	},
	{ 1641, "Sniff 37 (2004-09-16)"	},
	{ 1642, "Sniff 37 (2004-09-16)"	},
	{ 1699, "Sniff 38 (2004-10-07)"	},
	{ 1700, "Sniff 38 (2004-10-07)"	},
	{ 1752, "Sniff 39 (2004-11-02)"	},
	{ 1753, "Sniff 39 (2004-11-02)"	},
	{ 1759, "Sniff 40 (2004-11-03)"	},
	{ 1760, "Sniff 40 (2004-11-03)"	},
	{ 1761, "Sniff 40 (2004-11-03)"	},
	{ 2009, "Sniff 41 (2005-04-06)"	},
	{ 2010, "Sniff 41 (2005-04-06)"	},
	{ 2011, "Sniff 41 (2005-04-06)"	},
	{ 2016, "Sniff 42 (2005-04-11)"	},
	{ 2017, "Sniff 42 (2005-04-11)"	},
	{ 2018, "Sniff 42 (2005-04-11)"	},
	{ 2023, "Sniff 43 (2005-04-14)"	},
	{ 2024, "Sniff 43 (2005-04-14)"	},
	{ 2025, "Sniff 43 (2005-04-14)"	},
	{ 2032, "Sniff 44 (2005-04-18)"	},
	{ 2033, "Sniff 44 (2005-04-18)"	},
	{ 2034, "Sniff 44 (2005-04-18)"	},
	{ 2288, "Sniff 45 (2005-07-08)"	},
	{ 2289, "Sniff 45 (2005-07-08)"	},
	{ 2290, "Sniff 45 (2005-07-08)"	},
	{ 2388, "Sniff 46 (2005-08-17)"	},
	{ 2389, "Sniff 46 (2005-08-17)"	},
	{ 2390, "Sniff 46 (2005-08-17)"	},
	{ 2869, "Sniff 47 (2006-02-15)"	},
	{ 2870, "Sniff 47 (2006-02-15)"	},
	{ 2871, "Sniff 47 (2006-02-15)"	},
	{ 3214, "Sniff 48 (2006-05-16)"	},
	{ 3215, "Sniff 48 (2006-05-16)"	},
	{ 3216, "Sniff 48 (2006-05-16)"	},
	{ 3356, "Sniff 49 (2006-07-17)"	},
	{ 3529, "Sniff 50 (2006-09-21)"	},
	{ 3546, "Sniff 51 (2006-09-29)"	},
	{ 3683, "Sniff 52 (2006-11-03)"	},
	{    0, }
};

char *csr_builddeftostr(uint16_t def)
{
	switch (def) {
	case 0x0000:
		return "NONE";
	case 0x0001:
		return "CHIP_BASE_BC01";
	case 0x0002:
		return "CHIP_BASE_BC02";
	case 0x0003:
		return "CHIP_BC01B";
	case 0x0004:
		return "CHIP_BC02_EXTERNAL";
	case 0x0005:
		return "BUILD_HCI";
	case 0x0006:
		return "BUILD_RFCOMM";
	case 0x0007:
		return "BT_VER_1_1";
	case 0x0008:
		return "TRANSPORT_ALL";
	case 0x0009:
		return "TRANSPORT_BCSP";
	case 0x000a:
		return "TRANSPORT_H4";
	case 0x000b:
		return "TRANSPORT_USB";
	case 0x000c:
		return "MAX_CRYPT_KEY_LEN_56";
	case 0x000d:
		return "MAX_CRYPT_KEY_LEN_128";
	case 0x000e:
		return "TRANSPORT_USER";
	case 0x000f:
		return "CHIP_BC02_KATO";
	case 0x0010:
		return "TRANSPORT_NONE";
	case 0x0012:
		return "REQUIRE_8MBIT";
	case 0x0013:
		return "RADIOTEST";
	case 0x0014:
		return "RADIOTEST_LITE";
	case 0x0015:
		return "INSTALL_FLASH";
	case 0x0016:
		return "INSTALL_EEPROM";
	case 0x0017:
		return "INSTALL_COMBO_DOT11";
	case 0x0018:
		return "LOWPOWER_TX";
	case 0x0019:
		return "TRANSPORT_TWUTL";
	case 0x001a:
		return "COMPILER_GCC";
	case 0x001b:
		return "CHIP_BC02_CLOUSEAU";
	case 0x001c:
		return "CHIP_BC02_TOULOUSE";
	case 0x001d:
		return "CHIP_BASE_BC3";
	case 0x001e:
		return "CHIP_BC3_NICKNACK";
	case 0x001f:
		return "CHIP_BC3_KALIMBA";
	case 0x0020:
		return "INSTALL_HCI_MODULE";
	case 0x0021:
		return "INSTALL_L2CAP_MODULE";
	case 0x0022:
		return "INSTALL_DM_MODULE";
	case 0x0023:
		return "INSTALL_SDP_MODULE";
	case 0x0024:
		return "INSTALL_RFCOMM_MODULE";
	case 0x0025:
		return "INSTALL_HIDIO_MODULE";
	case 0x0026:
		return "INSTALL_PAN_MODULE";
	case 0x0027:
		return "INSTALL_IPV4_MODULE";
	case 0x0028:
		return "INSTALL_IPV6_MODULE";
	case 0x0029:
		return "INSTALL_TCP_MODULE";
	case 0x002a:
		return "BT_VER_1_2";
	case 0x002b:
		return "INSTALL_UDP_MODULE";
	case 0x002c:
		return "REQUIRE_0_WAIT_STATES";
	case 0x002d:
		return "CHIP_BC3_PADDYWACK";
	case 0x002e:
		return "CHIP_BC4_COYOTE";
	case 0x002f:
		return "CHIP_BC4_ODDJOB";
	case 0x0030:
		return "TRANSPORT_H4DS";
	case 0x0031:
		return "CHIP_BASE_BC4";
	default:
		return "UNKNOWN";
	}
}

char *csr_buildidtostr(uint16_t id)
{
	static char str[12];
	int i;

	for (i = 0; csr_map[i].id; i++)
		if (csr_map[i].id == id)
			return csr_map[i].str;

	snprintf(str, 11, "Build %d", id);
	return str;
}

char *csr_chipvertostr(uint16_t ver, uint16_t rev)
{
	switch (ver) {
	case 0x00:
		return "BlueCore01a";
	case 0x01:
		switch (rev) {
		case 0x64:
			return "BlueCore01b (ES)";
		case 0x65:
		default:
			return "BlueCore01b";
		}
	case 0x02:
		switch (rev) {
		case 0x89:
			return "BlueCore02-External (ES2)";
		case 0x8a:
			return "BlueCore02-External";
		case 0x28:
			return "BlueCore02-ROM/Audio/Flash";
		default:
			return "BlueCore02";
		}
	case 0x03:
		switch (rev) {
		case 0x43:
			return "BlueCore3-MM";
		case 0x15:
			return "BlueCore3-ROM";
		case 0xe2:
			return "BlueCore3-Flash";
		case 0x26:
			return "BlueCore4-External";
		case 0x30:
			return "BlueCore4-ROM";
		default:
			return "BlueCore3 or BlueCore4";
		}
	default:
		return "Unknown";
	}
}

char *csr_pskeytostr(uint16_t pskey)
{
	switch (pskey) {
	case CSR_PSKEY_BDADDR:
		return "Bluetooth address";
	case CSR_PSKEY_COUNTRYCODE:
		return "Country code";
	case CSR_PSKEY_CLASSOFDEVICE:
		return "Class of device";
	case CSR_PSKEY_DEVICE_DRIFT:
		return "Device drift";
	case CSR_PSKEY_DEVICE_JITTER:
		return "Device jitter";
	case CSR_PSKEY_MAX_ACLS:
		return "Maximum ACL links";
	case CSR_PSKEY_MAX_SCOS:
		return "Maximum SCO links";
	case CSR_PSKEY_MAX_REMOTE_MASTERS:
		return "Maximum remote masters";
	case CSR_PSKEY_ENABLE_MASTERY_WITH_SLAVERY:
		return "Support master and slave roles simultaneously";
	case CSR_PSKEY_H_HC_FC_MAX_ACL_PKT_LEN:
		return "Maximum HCI ACL packet length";
	case CSR_PSKEY_H_HC_FC_MAX_SCO_PKT_LEN:
		return "Maximum HCI SCO packet length";
	case CSR_PSKEY_H_HC_FC_MAX_ACL_PKTS:
		return "Maximum number of HCI ACL packets";
	case CSR_PSKEY_H_HC_FC_MAX_SCO_PKTS:
		return "Maximum number of HCI SCO packets";
	case CSR_PSKEY_LC_FC_BUFFER_LOW_WATER_MARK:
		return "Flow control low water mark";
	case CSR_PSKEY_LC_MAX_TX_POWER:
		return "Maximum transmit power";
	case CSR_PSKEY_TX_GAIN_RAMP:
		return "Transmit gain ramp rate";
	case CSR_PSKEY_LC_POWER_TABLE:
		return "Radio power table";
	case CSR_PSKEY_LC_PEER_POWER_PERIOD:
		return "Peer transmit power control interval";
	case CSR_PSKEY_LC_FC_POOLS_LOW_WATER_MARK:
		return "Flow control pool low water mark";
	case CSR_PSKEY_LC_DEFAULT_TX_POWER:
		return "Default transmit power";
	case CSR_PSKEY_LC_RSSI_GOLDEN_RANGE:
		return "RSSI at bottom of golden receive range";
	case CSR_PSKEY_LC_COMBO_DISABLE_PIO_MASK:
		return "Combo: PIO lines and logic to disable transmit";
	case CSR_PSKEY_LC_COMBO_PRIORITY_PIO_MASK:
		return "Combo: priority activity PIO lines and logic";
	case CSR_PSKEY_LC_COMBO_DOT11_CHANNEL_PIO_BASE:
		return "Combo: 802.11b channel number base PIO line";
	case CSR_PSKEY_LC_COMBO_DOT11_BLOCK_CHANNELS:
		return "Combo: channels to block either side of 802.11b";
	case CSR_PSKEY_LC_MAX_TX_POWER_NO_RSSI:
		return "Maximum transmit power when peer has no RSSI";
	case CSR_PSKEY_LC_CONNECTION_RX_WINDOW:
		return "Receive window size during connections";
	case CSR_PSKEY_LC_COMBO_DOT11_TX_PROTECTION_MODE:
		return "Combo: which TX packets shall we protect";
	case CSR_PSKEY_LC_ENHANCED_POWER_TABLE:
		return "Radio power table";
	case CSR_PSKEY_LC_WIDEBAND_RSSI_CONFIG:
		return "RSSI configuration for use with wideband RSSI";
	case CSR_PSKEY_LC_COMBO_DOT11_PRIORITY_LEAD:
		return "Combo: How much notice will we give the Combo Card";
	case CSR_PSKEY_BT_CLOCK_INIT:
		return "Initial value of Bluetooth clock";
	case CSR_PSKEY_TX_MR_MOD_DELAY:
		return "TX Mod delay";
	case CSR_PSKEY_RX_MR_SYNC_TIMING:
		return "RX MR Sync Timing";
	case CSR_PSKEY_RX_MR_SYNC_CONFIG:
		return "RX MR Sync Configuration";
	case CSR_PSKEY_LC_LOST_SYNC_SLOTS:
		return "Time in ms for lost sync in low power modes";
	case CSR_PSKEY_RX_MR_SAMP_CONFIG:
		return "RX MR Sync Configuration";
	case CSR_PSKEY_AGC_HYST_LEVELS:
		return "AGC hysteresis levels";
	case CSR_PSKEY_RX_LEVEL_LOW_SIGNAL:
		return "ANA_RX_LVL at low signal strengths";
	case CSR_PSKEY_AGC_IQ_LVL_VALUES:
		return "ANA_IQ_LVL values for AGC algorithmn";
	case CSR_PSKEY_MR_FTRIM_OFFSET_12DB:
		return "ANA_RX_FTRIM offset when using 12 dB IF atten ";
	case CSR_PSKEY_MR_FTRIM_OFFSET_6DB:
		return "ANA_RX_FTRIM offset when using 6 dB IF atten ";
	case CSR_PSKEY_NO_CAL_ON_BOOT:
		return "Do not calibrate radio on boot";
	case CSR_PSKEY_RSSI_HI_TARGET:
		return "RSSI high target";
	case CSR_PSKEY_PREFERRED_MIN_ATTENUATION:
		return "Preferred minimum attenuator setting";
	case CSR_PSKEY_LC_COMBO_DOT11_PRIORITY_OVERRIDE:
		return "Combo: Treat all packets as high priority";
	case CSR_PSKEY_LC_MULTISLOT_HOLDOFF:
		return "Time till single slot packets are used for resync";
	case CSR_PSKEY_FREE_KEY_PIGEON_HOLE:
		return "Link key store bitfield";
	case CSR_PSKEY_LINK_KEY_BD_ADDR0:
		return "Bluetooth address + link key 0";
	case CSR_PSKEY_LINK_KEY_BD_ADDR1:
		return "Bluetooth address + link key 1";
	case CSR_PSKEY_LINK_KEY_BD_ADDR2:
		return "Bluetooth address + link key 2";
	case CSR_PSKEY_LINK_KEY_BD_ADDR3:
		return "Bluetooth address + link key 3";
	case CSR_PSKEY_LINK_KEY_BD_ADDR4:
		return "Bluetooth address + link key 4";
	case CSR_PSKEY_LINK_KEY_BD_ADDR5:
		return "Bluetooth address + link key 5";
	case CSR_PSKEY_LINK_KEY_BD_ADDR6:
		return "Bluetooth address + link key 6";
	case CSR_PSKEY_LINK_KEY_BD_ADDR7:
		return "Bluetooth address + link key 7";
	case CSR_PSKEY_LINK_KEY_BD_ADDR8:
		return "Bluetooth address + link key 8";
	case CSR_PSKEY_LINK_KEY_BD_ADDR9:
		return "Bluetooth address + link key 9";
	case CSR_PSKEY_LINK_KEY_BD_ADDR10:
		return "Bluetooth address + link key 10";
	case CSR_PSKEY_LINK_KEY_BD_ADDR11:
		return "Bluetooth address + link key 11";
	case CSR_PSKEY_LINK_KEY_BD_ADDR12:
		return "Bluetooth address + link key 12";
	case CSR_PSKEY_LINK_KEY_BD_ADDR13:
		return "Bluetooth address + link key 13";
	case CSR_PSKEY_LINK_KEY_BD_ADDR14:
		return "Bluetooth address + link key 14";
	case CSR_PSKEY_LINK_KEY_BD_ADDR15:
		return "Bluetooth address + link key 15";
	case CSR_PSKEY_ENC_KEY_LMIN:
		return "Minimum encryption key length";
	case CSR_PSKEY_ENC_KEY_LMAX:
		return "Maximum encryption key length";
	case CSR_PSKEY_LOCAL_SUPPORTED_FEATURES:
		return "Local supported features block";
	case CSR_PSKEY_LM_USE_UNIT_KEY:
		return "Allow use of unit key for authentication?";
	case CSR_PSKEY_HCI_NOP_DISABLE:
		return "Disable the HCI Command_Status event on boot";
	case CSR_PSKEY_LM_MAX_EVENT_FILTERS:
		return "Maximum number of event filters";
	case CSR_PSKEY_LM_USE_ENC_MODE_BROADCAST:
		return "Allow LM to use enc_mode=2";
	case CSR_PSKEY_LM_TEST_SEND_ACCEPTED_TWICE:
		return "LM sends two LMP_accepted messages in test mode";
	case CSR_PSKEY_LM_MAX_PAGE_HOLD_TIME:
		return "Maximum time we hold a device around page";
	case CSR_PSKEY_AFH_ADAPTATION_RESPONSE_TIME:
		return "LM period for AFH adaption";
	case CSR_PSKEY_AFH_OPTIONS:
		return "Options to configure AFH";
	case CSR_PSKEY_AFH_RSSI_RUN_PERIOD:
		return "AFH RSSI reading period";
	case CSR_PSKEY_AFH_REENABLE_CHANNEL_TIME:
		return "AFH good channel adding time";
	case CSR_PSKEY_NO_DROP_ON_ACR_MS_FAIL:
		return "Complete link if acr barge-in role switch refused";
	case CSR_PSKEY_MAX_PRIVATE_KEYS:
		return "Max private link keys stored";
	case CSR_PSKEY_PRIVATE_LINK_KEY_BD_ADDR0:
		return "Bluetooth address + link key 0";
	case CSR_PSKEY_PRIVATE_LINK_KEY_BD_ADDR1:
		return "Bluetooth address + link key 1";
	case CSR_PSKEY_PRIVATE_LINK_KEY_BD_ADDR2:
		return "Bluetooth address + link key 2";
	case CSR_PSKEY_PRIVATE_LINK_KEY_BD_ADDR3:
		return "Bluetooth address + link key 3";
	case CSR_PSKEY_PRIVATE_LINK_KEY_BD_ADDR4:
		return "Bluetooth address + link key 4";
	case CSR_PSKEY_PRIVATE_LINK_KEY_BD_ADDR5:
		return "Bluetooth address + link key 5";
	case CSR_PSKEY_PRIVATE_LINK_KEY_BD_ADDR6:
		return "Bluetooth address + link key 6";
	case CSR_PSKEY_PRIVATE_LINK_KEY_BD_ADDR7:
		return "Bluetooth address + link key 7";
	case CSR_PSKEY_LOCAL_SUPPORTED_COMMANDS:
		return "Local supported commands";
	case CSR_PSKEY_LM_MAX_ABSENCE_INDEX:
		return "Maximum absence index allowed";
	case CSR_PSKEY_DEVICE_NAME:
		return "Local device's \"user friendly\" name";
	case CSR_PSKEY_AFH_RSSI_THRESHOLD:
		return "AFH RSSI threshold";
	case CSR_PSKEY_LM_CASUAL_SCAN_INTERVAL:
		return "Scan interval in slots for casual scanning";
	case CSR_PSKEY_AFH_MIN_MAP_CHANGE:
		return "The minimum amount to change an AFH map by";
	case CSR_PSKEY_AFH_RSSI_LP_RUN_PERIOD:
		return "AFH RSSI reading period when in low power mode";
	case CSR_PSKEY_HCI_LMP_LOCAL_VERSION:
		return "The HCI and LMP version reported locally";
	case CSR_PSKEY_LMP_REMOTE_VERSION:
		return "The LMP version reported remotely";
	case CSR_PSKEY_HOLD_ERROR_MESSAGE_NUMBER:
		return "Maximum number of queued HCI Hardware Error Events";
	case CSR_PSKEY_DFU_ATTRIBUTES:
		return "DFU attributes";
	case CSR_PSKEY_DFU_DETACH_TO:
		return "DFU detach timeout";
	case CSR_PSKEY_DFU_TRANSFER_SIZE:
		return "DFU transfer size";
	case CSR_PSKEY_DFU_ENABLE:
		return "DFU enable";
	case CSR_PSKEY_DFU_LIN_REG_ENABLE:
		return "Linear Regulator enabled at boot in DFU mode";
	case CSR_PSKEY_DFUENC_VMAPP_PK_MODULUS_MSB:
		return "DFU encryption VM application public key MSB";
	case CSR_PSKEY_DFUENC_VMAPP_PK_MODULUS_LSB:
		return "DFU encryption VM application public key LSB";
	case CSR_PSKEY_DFUENC_VMAPP_PK_M_DASH:
		return "DFU encryption VM application M dash";
	case CSR_PSKEY_DFUENC_VMAPP_PK_R2N_MSB:
		return "DFU encryption VM application public key R2N MSB";
	case CSR_PSKEY_DFUENC_VMAPP_PK_R2N_LSB:
		return "DFU encryption VM application public key R2N LSB";
	case CSR_PSKEY_BCSP_LM_PS_BLOCK:
		return "BCSP link establishment block";
	case CSR_PSKEY_HOSTIO_FC_PS_BLOCK:
		return "HCI flow control block";
	case CSR_PSKEY_HOSTIO_PROTOCOL_INFO0:
		return "Host transport channel 0 settings (BCSP ACK)";
	case CSR_PSKEY_HOSTIO_PROTOCOL_INFO1:
		return "Host transport channel 1 settings (BCSP-LE)";
	case CSR_PSKEY_HOSTIO_PROTOCOL_INFO2:
		return "Host transport channel 2 settings (BCCMD)";
	case CSR_PSKEY_HOSTIO_PROTOCOL_INFO3:
		return "Host transport channel 3 settings (HQ)";
	case CSR_PSKEY_HOSTIO_PROTOCOL_INFO4:
		return "Host transport channel 4 settings (DM)";
	case CSR_PSKEY_HOSTIO_PROTOCOL_INFO5:
		return "Host transport channel 5 settings (HCI CMD/EVT)";
	case CSR_PSKEY_HOSTIO_PROTOCOL_INFO6:
		return "Host transport channel 6 settings (HCI ACL)";
	case CSR_PSKEY_HOSTIO_PROTOCOL_INFO7:
		return "Host transport channel 7 settings (HCI SCO)";
	case CSR_PSKEY_HOSTIO_PROTOCOL_INFO8:
		return "Host transport channel 8 settings (L2CAP)";
	case CSR_PSKEY_HOSTIO_PROTOCOL_INFO9:
		return "Host transport channel 9 settings (RFCOMM)";
	case CSR_PSKEY_HOSTIO_PROTOCOL_INFO10:
		return "Host transport channel 10 settings (SDP)";
	case CSR_PSKEY_HOSTIO_PROTOCOL_INFO11:
		return "Host transport channel 11 settings (TEST)";
	case CSR_PSKEY_HOSTIO_PROTOCOL_INFO12:
		return "Host transport channel 12 settings (DFU)";
	case CSR_PSKEY_HOSTIO_PROTOCOL_INFO13:
		return "Host transport channel 13 settings (VM)";
	case CSR_PSKEY_HOSTIO_PROTOCOL_INFO14:
		return "Host transport channel 14 settings";
	case CSR_PSKEY_HOSTIO_PROTOCOL_INFO15:
		return "Host transport channel 15 settings";
	case CSR_PSKEY_HOSTIO_UART_RESET_TIMEOUT:
		return "UART reset counter timeout";
	case CSR_PSKEY_HOSTIO_USE_HCI_EXTN:
		return "Use hci_extn to route non-hci channels";
	case CSR_PSKEY_HOSTIO_USE_HCI_EXTN_CCFC:
		return "Use command-complete flow control for hci_extn";
	case CSR_PSKEY_HOSTIO_HCI_EXTN_PAYLOAD_SIZE:
		return "Maximum hci_extn payload size";
	case CSR_PSKEY_BCSP_LM_CNF_CNT_LIMIT:
		return "BCSP link establishment conf message count";
	case CSR_PSKEY_HOSTIO_MAP_SCO_PCM:
		return "Map SCO over PCM";
	case CSR_PSKEY_HOSTIO_AWKWARD_PCM_SYNC:
		return "PCM interface synchronisation is difficult";
	case CSR_PSKEY_HOSTIO_BREAK_POLL_PERIOD:
		return "Break poll period (microseconds)";
	case CSR_PSKEY_HOSTIO_MIN_UART_HCI_SCO_SIZE:
		return "Minimum SCO packet size sent to host over UART HCI";
	case CSR_PSKEY_HOSTIO_MAP_SCO_CODEC:
		return "Map SCO over the built-in codec";
	case CSR_PSKEY_PCM_CVSD_TX_HI_FREQ_BOOST:
		return "High frequency boost for PCM when transmitting CVSD";
	case CSR_PSKEY_PCM_CVSD_RX_HI_FREQ_BOOST:
		return "High frequency boost for PCM when receiving CVSD";
	case CSR_PSKEY_PCM_CONFIG32:
		return "PCM interface settings bitfields";
	case CSR_PSKEY_USE_OLD_BCSP_LE:
		return "Use the old version of BCSP link establishment";
	case CSR_PSKEY_PCM_CVSD_USE_NEW_FILTER:
		return "CVSD uses the new filter if available";
	case CSR_PSKEY_PCM_FORMAT:
		return "PCM data format";
	case CSR_PSKEY_CODEC_OUT_GAIN:
		return "Audio output gain when using built-in codec";
	case CSR_PSKEY_CODEC_IN_GAIN:
		return "Audio input gain when using built-in codec";
	case CSR_PSKEY_CODEC_PIO:
		return "PIO to enable when built-in codec is enabled";
	case CSR_PSKEY_PCM_LOW_JITTER_CONFIG:
		return "PCM interface settings for low jitter master mode";
	case CSR_PSKEY_HOSTIO_SCO_PCM_THRESHOLDS:
		return "Thresholds for SCO PCM buffers";
	case CSR_PSKEY_HOSTIO_SCO_HCI_THRESHOLDS:
		return "Thresholds for SCO HCI buffers";
	case CSR_PSKEY_HOSTIO_MAP_SCO_PCM_SLOT:
		return "Route SCO data to specified slot in pcm frame";
	case CSR_PSKEY_UART_BAUDRATE:
		return "UART Baud rate";
	case CSR_PSKEY_UART_CONFIG_BCSP:
		return "UART configuration when using BCSP";
	case CSR_PSKEY_UART_CONFIG_H4:
		return "UART configuration when using H4";
	case CSR_PSKEY_UART_CONFIG_H5:
		return "UART configuration when using H5";
	case CSR_PSKEY_UART_CONFIG_USR:
		return "UART configuration when under VM control";
	case CSR_PSKEY_UART_TX_CRCS:
		return "Use CRCs for BCSP or H5";
	case CSR_PSKEY_UART_ACK_TIMEOUT:
		return "Acknowledgement timeout for BCSP and H5";
	case CSR_PSKEY_UART_TX_MAX_ATTEMPTS:
		return "Max times to send reliable BCSP or H5 message";
	case CSR_PSKEY_UART_TX_WINDOW_SIZE:
		return "Transmit window size for BCSP and H5";
	case CSR_PSKEY_UART_HOST_WAKE:
		return "UART host wakeup";
	case CSR_PSKEY_HOSTIO_THROTTLE_TIMEOUT:
		return "Host interface performance control.";
	case CSR_PSKEY_PCM_ALWAYS_ENABLE:
		return "PCM port is always enable when chip is running";
	case CSR_PSKEY_UART_HOST_WAKE_SIGNAL:
		return "Signal to use for uart host wakeup protocol";
	case CSR_PSKEY_UART_CONFIG_H4DS:
		return "UART configuration when using H4DS";
	case CSR_PSKEY_H4DS_WAKE_DURATION:
		return "How long to spend waking the host when using H4DS";
	case CSR_PSKEY_H4DS_MAXWU:
		return "Maximum number of H4DS Wake-Up messages to send";
	case CSR_PSKEY_H4DS_LE_TIMER_PERIOD:
		return "H4DS Link Establishment Tsync and Tconf period";
	case CSR_PSKEY_H4DS_TWU_TIMER_PERIOD:
		return "H4DS Twu timer period";
	case CSR_PSKEY_H4DS_UART_IDLE_TIMER_PERIOD:
		return "H4DS Tuart_idle timer period";
	case CSR_PSKEY_ANA_FTRIM:
		return "Crystal frequency trim";
	case CSR_PSKEY_WD_TIMEOUT:
		return "Watchdog timeout (microseconds)";
	case CSR_PSKEY_WD_PERIOD:
		return "Watchdog period (microseconds)";
	case CSR_PSKEY_HOST_INTERFACE:
		return "Host interface";
	case CSR_PSKEY_HQ_HOST_TIMEOUT:
		return "HQ host command timeout";
	case CSR_PSKEY_HQ_ACTIVE:
		return "Enable host query task?";
	case CSR_PSKEY_BCCMD_SECURITY_ACTIVE:
		return "Enable configuration security";
	case CSR_PSKEY_ANA_FREQ:
		return "Crystal frequency";
	case CSR_PSKEY_PIO_PROTECT_MASK:
		return "Access to PIO pins";
	case CSR_PSKEY_PMALLOC_SIZES:
		return "pmalloc sizes array";
	case CSR_PSKEY_UART_BAUD_RATE:
		return "UART Baud rate (pre 18)";
	case CSR_PSKEY_UART_CONFIG:
		return "UART configuration bitfield";
	case CSR_PSKEY_STUB:
		return "Stub";
	case CSR_PSKEY_TXRX_PIO_CONTROL:
		return "TX and RX PIO control";
	case CSR_PSKEY_ANA_RX_LEVEL:
		return "ANA_RX_LVL register initial value";
	case CSR_PSKEY_ANA_RX_FTRIM:
		return "ANA_RX_FTRIM register initial value";
	case CSR_PSKEY_PSBC_DATA_VERSION:
		return "Persistent store version";
	case CSR_PSKEY_PCM0_ATTENUATION:
		return "Volume control on PCM channel 0";
	case CSR_PSKEY_LO_LVL_MAX:
		return "Maximum value of LO level control register";
	case CSR_PSKEY_LO_ADC_AMPL_MIN:
		return "Minimum value of the LO amplitude measured on the ADC";
	case CSR_PSKEY_LO_ADC_AMPL_MAX:
		return "Maximum value of the LO amplitude measured on the ADC";
	case CSR_PSKEY_IQ_TRIM_CHANNEL:
		return "IQ calibration channel";
	case CSR_PSKEY_IQ_TRIM_GAIN:
		return "IQ calibration gain";
	case CSR_PSKEY_IQ_TRIM_ENABLE:
		return "IQ calibration enable";
	case CSR_PSKEY_TX_OFFSET_HALF_MHZ:
		return "Transmit offset";
	case CSR_PSKEY_GBL_MISC_ENABLES:
		return "Global miscellaneous hardware enables";
	case CSR_PSKEY_UART_SLEEP_TIMEOUT:
		return "Time in ms to deep sleep if nothing received";
	case CSR_PSKEY_DEEP_SLEEP_STATE:
		return "Deep sleep state usage";
	case CSR_PSKEY_IQ_ENABLE_PHASE_TRIM:
		return "IQ phase enable";
	case CSR_PSKEY_HCI_HANDLE_FREEZE_PERIOD:
		return "Time for which HCI handle is frozen after link removal";
	case CSR_PSKEY_MAX_FROZEN_HCI_HANDLES:
		return "Maximum number of frozen HCI handles";
	case CSR_PSKEY_PAGETABLE_DESTRUCTION_DELAY:
		return "Delay from freezing buf handle to deleting page table";
	case CSR_PSKEY_IQ_TRIM_PIO_SETTINGS:
		return "IQ PIO settings";
	case CSR_PSKEY_USE_EXTERNAL_CLOCK:
		return "Device uses an external clock";
	case CSR_PSKEY_DEEP_SLEEP_WAKE_CTS:
		return "Exit deep sleep on CTS line activity";
	case CSR_PSKEY_FC_HC2H_FLUSH_DELAY:
		return "Delay from disconnect to flushing HC->H FC tokens";
	case CSR_PSKEY_RX_HIGHSIDE:
		return "Disable the HIGHSIDE bit in ANA_CONFIG";
	case CSR_PSKEY_TX_PRE_LVL:
		return "TX pre-amplifier level";
	case CSR_PSKEY_RX_SINGLE_ENDED:
		return "RX single ended";
	case CSR_PSKEY_TX_FILTER_CONFIG:
		return "TX filter configuration";
	case CSR_PSKEY_CLOCK_REQUEST_ENABLE:
		return "External clock request enable";
	case CSR_PSKEY_RX_MIN_ATTEN:
		return "Minimum attenuation allowed for receiver";
	case CSR_PSKEY_XTAL_TARGET_AMPLITUDE:
		return "Crystal target amplitude";
	case CSR_PSKEY_PCM_MIN_CPU_CLOCK:
		return "Minimum CPU clock speed with PCM port running";
	case CSR_PSKEY_HOST_INTERFACE_PIO_USB:
		return "USB host interface selection PIO line";
	case CSR_PSKEY_CPU_IDLE_MODE:
		return "CPU idle mode when radio is active";
	case CSR_PSKEY_DEEP_SLEEP_CLEAR_RTS:
		return "Deep sleep clears the UART RTS line";
	case CSR_PSKEY_RF_RESONANCE_TRIM:
		return "Frequency trim for IQ and LNA resonant circuits";
	case CSR_PSKEY_DEEP_SLEEP_PIO_WAKE:
		return "PIO line to wake the chip from deep sleep";
	case CSR_PSKEY_DRAIN_BORE_TIMERS:
		return "Energy consumption measurement settings";
	case CSR_PSKEY_DRAIN_TX_POWER_BASE:
		return "Energy consumption measurement settings";
	case CSR_PSKEY_MODULE_ID:
		return "Module serial number";
	case CSR_PSKEY_MODULE_DESIGN:
		return "Module design ID";
	case CSR_PSKEY_MODULE_SECURITY_CODE:
		return "Module security code";
	case CSR_PSKEY_VM_DISABLE:
		return "VM disable";
	case CSR_PSKEY_MOD_MANUF0:
		return "Module manufactuer data 0";
	case CSR_PSKEY_MOD_MANUF1:
		return "Module manufactuer data 1";
	case CSR_PSKEY_MOD_MANUF2:
		return "Module manufactuer data 2";
	case CSR_PSKEY_MOD_MANUF3:
		return "Module manufactuer data 3";
	case CSR_PSKEY_MOD_MANUF4:
		return "Module manufactuer data 4";
	case CSR_PSKEY_MOD_MANUF5:
		return "Module manufactuer data 5";
	case CSR_PSKEY_MOD_MANUF6:
		return "Module manufactuer data 6";
	case CSR_PSKEY_MOD_MANUF7:
		return "Module manufactuer data 7";
	case CSR_PSKEY_MOD_MANUF8:
		return "Module manufactuer data 8";
	case CSR_PSKEY_MOD_MANUF9:
		return "Module manufactuer data 9";
	case CSR_PSKEY_DUT_VM_DISABLE:
		return "VM disable when entering radiotest modes";
	case CSR_PSKEY_USR0:
		return "User configuration data 0";
	case CSR_PSKEY_USR1:
		return "User configuration data 1";
	case CSR_PSKEY_USR2:
		return "User configuration data 2";
	case CSR_PSKEY_USR3:
		return "User configuration data 3";
	case CSR_PSKEY_USR4:
		return "User configuration data 4";
	case CSR_PSKEY_USR5:
		return "User configuration data 5";
	case CSR_PSKEY_USR6:
		return "User configuration data 6";
	case CSR_PSKEY_USR7:
		return "User configuration data 7";
	case CSR_PSKEY_USR8:
		return "User configuration data 8";
	case CSR_PSKEY_USR9:
		return "User configuration data 9";
	case CSR_PSKEY_USR10:
		return "User configuration data 10";
	case CSR_PSKEY_USR11:
		return "User configuration data 11";
	case CSR_PSKEY_USR12:
		return "User configuration data 12";
	case CSR_PSKEY_USR13:
		return "User configuration data 13";
	case CSR_PSKEY_USR14:
		return "User configuration data 14";
	case CSR_PSKEY_USR15:
		return "User configuration data 15";
	case CSR_PSKEY_USR16:
		return "User configuration data 16";
	case CSR_PSKEY_USR17:
		return "User configuration data 17";
	case CSR_PSKEY_USR18:
		return "User configuration data 18";
	case CSR_PSKEY_USR19:
		return "User configuration data 19";
	case CSR_PSKEY_USR20:
		return "User configuration data 20";
	case CSR_PSKEY_USR21:
		return "User configuration data 21";
	case CSR_PSKEY_USR22:
		return "User configuration data 22";
	case CSR_PSKEY_USR23:
		return "User configuration data 23";
	case CSR_PSKEY_USR24:
		return "User configuration data 24";
	case CSR_PSKEY_USR25:
		return "User configuration data 25";
	case CSR_PSKEY_USR26:
		return "User configuration data 26";
	case CSR_PSKEY_USR27:
		return "User configuration data 27";
	case CSR_PSKEY_USR28:
		return "User configuration data 28";
	case CSR_PSKEY_USR29:
		return "User configuration data 29";
	case CSR_PSKEY_USR30:
		return "User configuration data 30";
	case CSR_PSKEY_USR31:
		return "User configuration data 31";
	case CSR_PSKEY_USR32:
		return "User configuration data 32";
	case CSR_PSKEY_USR33:
		return "User configuration data 33";
	case CSR_PSKEY_USR34:
		return "User configuration data 34";
	case CSR_PSKEY_USR35:
		return "User configuration data 35";
	case CSR_PSKEY_USR36:
		return "User configuration data 36";
	case CSR_PSKEY_USR37:
		return "User configuration data 37";
	case CSR_PSKEY_USR38:
		return "User configuration data 38";
	case CSR_PSKEY_USR39:
		return "User configuration data 39";
	case CSR_PSKEY_USR40:
		return "User configuration data 40";
	case CSR_PSKEY_USR41:
		return "User configuration data 41";
	case CSR_PSKEY_USR42:
		return "User configuration data 42";
	case CSR_PSKEY_USR43:
		return "User configuration data 43";
	case CSR_PSKEY_USR44:
		return "User configuration data 44";
	case CSR_PSKEY_USR45:
		return "User configuration data 45";
	case CSR_PSKEY_USR46:
		return "User configuration data 46";
	case CSR_PSKEY_USR47:
		return "User configuration data 47";
	case CSR_PSKEY_USR48:
		return "User configuration data 48";
	case CSR_PSKEY_USR49:
		return "User configuration data 49";
	case CSR_PSKEY_USB_VERSION:
		return "USB specification version number";
	case CSR_PSKEY_USB_DEVICE_CLASS_CODES:
		return "USB device class codes";
	case CSR_PSKEY_USB_VENDOR_ID:
		return "USB vendor identifier";
	case CSR_PSKEY_USB_PRODUCT_ID:
		return "USB product identifier";
	case CSR_PSKEY_USB_MANUF_STRING:
		return "USB manufacturer string";
	case CSR_PSKEY_USB_PRODUCT_STRING:
		return "USB product string";
	case CSR_PSKEY_USB_SERIAL_NUMBER_STRING:
		return "USB serial number string";
	case CSR_PSKEY_USB_CONFIG_STRING:
		return "USB configuration string";
	case CSR_PSKEY_USB_ATTRIBUTES:
		return "USB attributes bitmap";
	case CSR_PSKEY_USB_MAX_POWER:
		return "USB device maximum power consumption";
	case CSR_PSKEY_USB_BT_IF_CLASS_CODES:
		return "USB Bluetooth interface class codes";
	case CSR_PSKEY_USB_LANGID:
		return "USB language strings supported";
	case CSR_PSKEY_USB_DFU_CLASS_CODES:
		return "USB DFU class codes block";
	case CSR_PSKEY_USB_DFU_PRODUCT_ID:
		return "USB DFU product ID";
	case CSR_PSKEY_USB_PIO_DETACH:
		return "USB detach/attach PIO line";
	case CSR_PSKEY_USB_PIO_WAKEUP:
		return "USB wakeup PIO line";
	case CSR_PSKEY_USB_PIO_PULLUP:
		return "USB D+ pullup PIO line";
	case CSR_PSKEY_USB_PIO_VBUS:
		return "USB VBus detection PIO Line";
	case CSR_PSKEY_USB_PIO_WAKE_TIMEOUT:
		return "Timeout for assertion of USB PIO wake signal";
	case CSR_PSKEY_USB_PIO_RESUME:
		return "PIO signal used in place of bus resume";
	case CSR_PSKEY_USB_BT_SCO_IF_CLASS_CODES:
		return "USB Bluetooth SCO interface class codes";
	case CSR_PSKEY_USB_SUSPEND_PIO_LEVEL:
		return "USB PIO levels to set when suspended";
	case CSR_PSKEY_USB_SUSPEND_PIO_DIR:
		return "USB PIO I/O directions to set when suspended";
	case CSR_PSKEY_USB_SUSPEND_PIO_MASK:
		return "USB PIO lines to be set forcibly in suspend";
	case CSR_PSKEY_USB_ENDPOINT_0_MAX_PACKET_SIZE:
		return "The maxmimum packet size for USB endpoint 0";
	case CSR_PSKEY_USB_CONFIG:
		return "USB config params for new chips (>bc2)";
	case CSR_PSKEY_RADIOTEST_ATTEN_INIT:
		return "Radio test initial attenuator";
	case CSR_PSKEY_RADIOTEST_FIRST_TRIM_TIME:
		return "IQ first calibration period in test";
	case CSR_PSKEY_RADIOTEST_SUBSEQUENT_TRIM_TIME:
		return "IQ subsequent calibration period in test";
	case CSR_PSKEY_RADIOTEST_LO_LVL_TRIM_ENABLE:
		return "LO_LVL calibration enable";
	case CSR_PSKEY_RADIOTEST_DISABLE_MODULATION:
		return "Disable modulation during radiotest transmissions";
	case CSR_PSKEY_RFCOMM_FCON_THRESHOLD:
		return "RFCOMM aggregate flow control on threshold";
	case CSR_PSKEY_RFCOMM_FCOFF_THRESHOLD:
		return "RFCOMM aggregate flow control off threshold";
	case CSR_PSKEY_IPV6_STATIC_ADDR:
		return "Static IPv6 address";
	case CSR_PSKEY_IPV4_STATIC_ADDR:
		return "Static IPv4 address";
	case CSR_PSKEY_IPV6_STATIC_PREFIX_LEN:
		return "Static IPv6 prefix length";
	case CSR_PSKEY_IPV6_STATIC_ROUTER_ADDR:
		return "Static IPv6 router address";
	case CSR_PSKEY_IPV4_STATIC_SUBNET_MASK:
		return "Static IPv4 subnet mask";
	case CSR_PSKEY_IPV4_STATIC_ROUTER_ADDR:
		return "Static IPv4 router address";
	case CSR_PSKEY_MDNS_NAME:
		return "Multicast DNS name";
	case CSR_PSKEY_FIXED_PIN:
		return "Fixed PIN";
	case CSR_PSKEY_MDNS_PORT:
		return "Multicast DNS port";
	case CSR_PSKEY_MDNS_TTL:
		return "Multicast DNS TTL";
	case CSR_PSKEY_MDNS_IPV4_ADDR:
		return "Multicast DNS IPv4 address";
	case CSR_PSKEY_ARP_CACHE_TIMEOUT:
		return "ARP cache timeout";
	case CSR_PSKEY_HFP_POWER_TABLE:
		return "HFP power table";
	case CSR_PSKEY_DRAIN_BORE_TIMER_COUNTERS:
		return "Energy consumption estimation timer counters";
	case CSR_PSKEY_DRAIN_BORE_COUNTERS:
		return "Energy consumption estimation counters";
	case CSR_PSKEY_LOOP_FILTER_TRIM:
		return "Trim value to optimise loop filter";
	case CSR_PSKEY_DRAIN_BORE_CURRENT_PEAK:
		return "Energy consumption estimation current peak";
	case CSR_PSKEY_VM_E2_CACHE_LIMIT:
		return "Maximum RAM for caching EEPROM VM application";
	case CSR_PSKEY_FORCE_16MHZ_REF_PIO:
		return "PIO line to force 16 MHz reference to be assumed";
	case CSR_PSKEY_CDMA_LO_REF_LIMITS:
		return "Local oscillator frequency reference limits for CDMA";
	case CSR_PSKEY_CDMA_LO_ERROR_LIMITS:
		return "Local oscillator frequency error limits for CDMA";
	case CSR_PSKEY_CLOCK_STARTUP_DELAY:
		return "Clock startup delay in milliseconds";
	case CSR_PSKEY_DEEP_SLEEP_CORRECTION_FACTOR:
		return "Deep sleep clock correction factor";
	case CSR_PSKEY_TEMPERATURE_CALIBRATION:
		return "Temperature in deg C for a given internal setting";
	case CSR_PSKEY_TEMPERATURE_VS_DELTA_INTERNAL_PA:
		return "Temperature for given internal PA adjustment";
	case CSR_PSKEY_TEMPERATURE_VS_DELTA_TX_PRE_LVL:
		return "Temperature for a given TX_PRE_LVL adjustment";
	case CSR_PSKEY_TEMPERATURE_VS_DELTA_TX_BB:
		return "Temperature for a given TX_BB adjustment";
	case CSR_PSKEY_TEMPERATURE_VS_DELTA_ANA_FTRIM:
		return "Temperature for given crystal trim adjustment";
	case CSR_PSKEY_TEST_DELTA_OFFSET:
		return "Frequency offset applied to synthesiser in test mode";
	case CSR_PSKEY_RX_DYNAMIC_LVL_OFFSET:
		return "Receiver dynamic level offset depending on channel";
	case CSR_PSKEY_TEST_FORCE_OFFSET:
		return "Force use of exact value in PSKEY_TEST_DELTA_OFFSET";
	case CSR_PSKEY_RF_TRAP_BAD_DIVISION_RATIOS:
		return "Trap bad division ratios in radio frequency tables";
	case CSR_PSKEY_RADIOTEST_CDMA_LO_REF_LIMITS:
		return "LO frequency reference limits for CDMA in radiotest";
	case CSR_PSKEY_INITIAL_BOOTMODE:
		return "Initial device bootmode";
	case CSR_PSKEY_ONCHIP_HCI_CLIENT:
		return "HCI traffic routed internally";
	case CSR_PSKEY_RX_ATTEN_BACKOFF:
		return "Receiver attenuation back-off";
	case CSR_PSKEY_RX_ATTEN_UPDATE_RATE:
		return "Receiver attenuation update rate";
	case CSR_PSKEY_SYNTH_TXRX_THRESHOLDS:
		return "Local oscillator tuning voltage limits for tx and rx";
	case CSR_PSKEY_MIN_WAIT_STATES:
		return "Flash wait state indicator";
	case CSR_PSKEY_RSSI_CORRECTION:
		return "RSSI correction factor.";
	case CSR_PSKEY_SCHED_THROTTLE_TIMEOUT:
		return "Scheduler performance control.";
	case CSR_PSKEY_DEEP_SLEEP_USE_EXTERNAL_CLOCK:
		return "Deep sleep uses external 32 kHz clock source";
	case CSR_PSKEY_TRIM_RADIO_FILTERS:
		return "Trim rx and tx radio filters if true.";
	case CSR_PSKEY_TRANSMIT_OFFSET:
		return "Transmit offset in units of 62.5 kHz";
	case CSR_PSKEY_USB_VM_CONTROL:
		return "VM application will supply USB descriptors";
	case CSR_PSKEY_MR_ANA_RX_FTRIM:
		return "Medium rate value for the ANA_RX_FTRIM register";
	case CSR_PSKEY_I2C_CONFIG:
		return "I2C configuration";
	case CSR_PSKEY_IQ_LVL_RX:
		return "IQ demand level for reception";
	case CSR_PSKEY_MR_TX_FILTER_CONFIG:
		return "TX filter configuration used for enhanced data rate";
	case CSR_PSKEY_MR_TX_CONFIG2:
		return "TX filter configuration used for enhanced data rate";
	case CSR_PSKEY_USB_DONT_RESET_BOOTMODE_ON_HOST_RESET:
		return "Don't reset bootmode if USB host resets";
	case CSR_PSKEY_LC_USE_THROTTLING:
		return "Adjust packet selection on packet error rate";
	case CSR_PSKEY_CHARGER_TRIM:
		return "Trim value for the current charger";
	case CSR_PSKEY_CLOCK_REQUEST_FEATURES:
		return "Clock request is tristated if enabled";
	case CSR_PSKEY_TRANSMIT_OFFSET_CLASS1:
		return "Transmit offset / 62.5 kHz for class 1 radios";
	case CSR_PSKEY_TX_AVOID_PA_CLASS1_PIO:
		return "PIO line asserted in class1 operation to avoid PA";
	case CSR_PSKEY_MR_PIO_CONFIG:
		return "PIO line asserted in class1 operation to avoid PA";
	case CSR_PSKEY_UART_CONFIG2:
		return "The UART Sampling point";
	case CSR_PSKEY_CLASS1_IQ_LVL:
		return "IQ demand level for class 1 power level";
	case CSR_PSKEY_CLASS1_TX_CONFIG2:
		return "TX filter configuration used for class 1 tx power";
	case CSR_PSKEY_TEMPERATURE_VS_DELTA_INTERNAL_PA_CLASS1:
		return "Temperature for given internal PA adjustment";
	case CSR_PSKEY_TEMPERATURE_VS_DELTA_EXTERNAL_PA_CLASS1:
		return "Temperature for given internal PA adjustment";
	case CSR_PSKEY_TEMPERATURE_VS_DELTA_TX_PRE_LVL_MR:
		return "Temperature adjustment for TX_PRE_LVL in EDR";
	case CSR_PSKEY_TEMPERATURE_VS_DELTA_TX_BB_MR_HEADER:
		return "Temperature for a given TX_BB in EDR header";
	case CSR_PSKEY_TEMPERATURE_VS_DELTA_TX_BB_MR_PAYLOAD:
		return "Temperature for a given TX_BB in EDR payload";
	case CSR_PSKEY_RX_MR_EQ_TAPS:
		return "Adjust receiver configuration for EDR";
	case CSR_PSKEY_TX_PRE_LVL_CLASS1:
		return "TX pre-amplifier level in class 1 operation";
	case CSR_PSKEY_ANALOGUE_ATTENUATOR:
		return "TX analogue attenuator setting";
	case CSR_PSKEY_MR_RX_FILTER_TRIM:
		return "Trim for receiver used in EDR.";
	case CSR_PSKEY_MR_RX_FILTER_RESPONSE:
		return "Filter response for receiver used in EDR.";
	case CSR_PSKEY_PIO_WAKEUP_STATE:
		return "PIO deep sleep wake up state ";
	case CSR_PSKEY_MR_TX_IF_ATTEN_OFF_TEMP:
		return "TX IF atten off temperature when using EDR.";
	case CSR_PSKEY_LO_DIV_LATCH_BYPASS:
		return "Bypass latch for LO dividers";
	case CSR_PSKEY_LO_VCO_STANDBY:
		return "Use standby mode for the LO VCO";
	case CSR_PSKEY_SLOW_CLOCK_FILTER_SHIFT:
		return "Slow clock sampling filter constant";
	case CSR_PSKEY_SLOW_CLOCK_FILTER_DIVIDER:
		return "Slow clock filter fractional threshold";
	case CSR_PSKEY_USB_ATTRIBUTES_POWER:
		return "USB self powered";
	case CSR_PSKEY_USB_ATTRIBUTES_WAKEUP:
		return "USB responds to wake-up";
	case CSR_PSKEY_DFU_ATTRIBUTES_MANIFESTATION_TOLERANT:
		return "DFU manifestation tolerant";
	case CSR_PSKEY_DFU_ATTRIBUTES_CAN_UPLOAD:
		return "DFU can upload";
	case CSR_PSKEY_DFU_ATTRIBUTES_CAN_DOWNLOAD:
		return "DFU can download";
	case CSR_PSKEY_UART_CONFIG_STOP_BITS:
		return "UART: stop bits";
	case CSR_PSKEY_UART_CONFIG_PARITY_BIT:
		return "UART: parity bit";
	case CSR_PSKEY_UART_CONFIG_FLOW_CTRL_EN:
		return "UART: hardware flow control";
	case CSR_PSKEY_UART_CONFIG_RTS_AUTO_EN:
		return "UART: RTS auto-enabled";
	case CSR_PSKEY_UART_CONFIG_RTS:
		return "UART: RTS asserted";
	case CSR_PSKEY_UART_CONFIG_TX_ZERO_EN:
		return "UART: TX zero enable";
	case CSR_PSKEY_UART_CONFIG_NON_BCSP_EN:
		return "UART: enable BCSP-specific hardware";
	case CSR_PSKEY_UART_CONFIG_RX_RATE_DELAY:
		return "UART: RX rate delay";
	case CSR_PSKEY_UART_SEQ_TIMEOUT:
		return "UART: BCSP ack timeout";
	case CSR_PSKEY_UART_SEQ_RETRIES:
		return "UART: retry limit in sequencing layer";
	case CSR_PSKEY_UART_SEQ_WINSIZE:
		return "UART: BCSP transmit window size";
	case CSR_PSKEY_UART_USE_CRC_ON_TX:
		return "UART: use BCSP CRCs";
	case CSR_PSKEY_UART_HOST_INITIAL_STATE:
		return "UART: initial host state";
	case CSR_PSKEY_UART_HOST_ATTENTION_SPAN:
		return "UART: host attention span";
	case CSR_PSKEY_UART_HOST_WAKEUP_TIME:
		return "UART: host wakeup time";
	case CSR_PSKEY_UART_HOST_WAKEUP_WAIT:
		return "UART: host wakeup wait";
	case CSR_PSKEY_BCSP_LM_MODE:
		return "BCSP link establishment mode";
	case CSR_PSKEY_BCSP_LM_SYNC_RETRIES:
		return "BCSP link establishment sync retries";
	case CSR_PSKEY_BCSP_LM_TSHY:
		return "BCSP link establishment Tshy";
	case CSR_PSKEY_UART_DFU_CONFIG_STOP_BITS:
		return "DFU mode UART: stop bits";
	case CSR_PSKEY_UART_DFU_CONFIG_PARITY_BIT:
		return "DFU mode UART: parity bit";
	case CSR_PSKEY_UART_DFU_CONFIG_FLOW_CTRL_EN:
		return "DFU mode UART: hardware flow control";
	case CSR_PSKEY_UART_DFU_CONFIG_RTS_AUTO_EN:
		return "DFU mode UART: RTS auto-enabled";
	case CSR_PSKEY_UART_DFU_CONFIG_RTS:
		return "DFU mode UART: RTS asserted";
	case CSR_PSKEY_UART_DFU_CONFIG_TX_ZERO_EN:
		return "DFU mode UART: TX zero enable";
	case CSR_PSKEY_UART_DFU_CONFIG_NON_BCSP_EN:
		return "DFU mode UART: enable BCSP-specific hardware";
	case CSR_PSKEY_UART_DFU_CONFIG_RX_RATE_DELAY:
		return "DFU mode UART: RX rate delay";
	case CSR_PSKEY_AMUX_AIO0:
		return "Multiplexer for AIO 0";
	case CSR_PSKEY_AMUX_AIO1:
		return "Multiplexer for AIO 1";
	case CSR_PSKEY_AMUX_AIO2:
		return "Multiplexer for AIO 2";
	case CSR_PSKEY_AMUX_AIO3:
		return "Multiplexer for AIO 3";
	case CSR_PSKEY_LOCAL_NAME_SIMPLIFIED:
		return "Local Name (simplified)";
	case CSR_PSKEY_EXTENDED_STUB:
		return "Extended stub";
	default:
		return "Unknown";
	}
}

char *csr_pskeytoval(uint16_t pskey)
{
	switch (pskey) {
	case CSR_PSKEY_BDADDR:
		return "BDADDR";
	case CSR_PSKEY_COUNTRYCODE:
		return "COUNTRYCODE";
	case CSR_PSKEY_CLASSOFDEVICE:
		return "CLASSOFDEVICE";
	case CSR_PSKEY_DEVICE_DRIFT:
		return "DEVICE_DRIFT";
	case CSR_PSKEY_DEVICE_JITTER:
		return "DEVICE_JITTER";
	case CSR_PSKEY_MAX_ACLS:
		return "MAX_ACLS";
	case CSR_PSKEY_MAX_SCOS:
		return "MAX_SCOS";
	case CSR_PSKEY_MAX_REMOTE_MASTERS:
		return "MAX_REMOTE_MASTERS";
	case CSR_PSKEY_ENABLE_MASTERY_WITH_SLAVERY:
		return "ENABLE_MASTERY_WITH_SLAVERY";
	case CSR_PSKEY_H_HC_FC_MAX_ACL_PKT_LEN:
		return "H_HC_FC_MAX_ACL_PKT_LEN";
	case CSR_PSKEY_H_HC_FC_MAX_SCO_PKT_LEN:
		return "H_HC_FC_MAX_SCO_PKT_LEN";
	case CSR_PSKEY_H_HC_FC_MAX_ACL_PKTS:
		return "H_HC_FC_MAX_ACL_PKTS";
	case CSR_PSKEY_H_HC_FC_MAX_SCO_PKTS:
		return "H_HC_FC_MAX_SCO_PKTS";
	case CSR_PSKEY_LC_FC_BUFFER_LOW_WATER_MARK:
		return "LC_FC_BUFFER_LOW_WATER_MARK";
	case CSR_PSKEY_LC_MAX_TX_POWER:
		return "LC_MAX_TX_POWER";
	case CSR_PSKEY_TX_GAIN_RAMP:
		return "TX_GAIN_RAMP";
	case CSR_PSKEY_LC_POWER_TABLE:
		return "LC_POWER_TABLE";
	case CSR_PSKEY_LC_PEER_POWER_PERIOD:
		return "LC_PEER_POWER_PERIOD";
	case CSR_PSKEY_LC_FC_POOLS_LOW_WATER_MARK:
		return "LC_FC_POOLS_LOW_WATER_MARK";
	case CSR_PSKEY_LC_DEFAULT_TX_POWER:
		return "LC_DEFAULT_TX_POWER";
	case CSR_PSKEY_LC_RSSI_GOLDEN_RANGE:
		return "LC_RSSI_GOLDEN_RANGE";
	case CSR_PSKEY_LC_COMBO_DISABLE_PIO_MASK:
		return "LC_COMBO_DISABLE_PIO_MASK";
	case CSR_PSKEY_LC_COMBO_PRIORITY_PIO_MASK:
		return "LC_COMBO_PRIORITY_PIO_MASK";
	case CSR_PSKEY_LC_COMBO_DOT11_CHANNEL_PIO_BASE:
		return "LC_COMBO_DOT11_CHANNEL_PIO_BASE";
	case CSR_PSKEY_LC_COMBO_DOT11_BLOCK_CHANNELS:
		return "LC_COMBO_DOT11_BLOCK_CHANNELS";
	case CSR_PSKEY_LC_MAX_TX_POWER_NO_RSSI:
		return "LC_MAX_TX_POWER_NO_RSSI";
	case CSR_PSKEY_LC_CONNECTION_RX_WINDOW:
		return "LC_CONNECTION_RX_WINDOW";
	case CSR_PSKEY_LC_COMBO_DOT11_TX_PROTECTION_MODE:
		return "LC_COMBO_DOT11_TX_PROTECTION_MODE";
	case CSR_PSKEY_LC_ENHANCED_POWER_TABLE:
		return "LC_ENHANCED_POWER_TABLE";
	case CSR_PSKEY_LC_WIDEBAND_RSSI_CONFIG:
		return "LC_WIDEBAND_RSSI_CONFIG";
	case CSR_PSKEY_LC_COMBO_DOT11_PRIORITY_LEAD:
		return "LC_COMBO_DOT11_PRIORITY_LEAD";
	case CSR_PSKEY_BT_CLOCK_INIT:
		return "BT_CLOCK_INIT";
	case CSR_PSKEY_TX_MR_MOD_DELAY:
		return "TX_MR_MOD_DELAY";
	case CSR_PSKEY_RX_MR_SYNC_TIMING:
		return "RX_MR_SYNC_TIMING";
	case CSR_PSKEY_RX_MR_SYNC_CONFIG:
		return "RX_MR_SYNC_CONFIG";
	case CSR_PSKEY_LC_LOST_SYNC_SLOTS:
		return "LC_LOST_SYNC_SLOTS";
	case CSR_PSKEY_RX_MR_SAMP_CONFIG:
		return "RX_MR_SAMP_CONFIG";
	case CSR_PSKEY_AGC_HYST_LEVELS:
		return "AGC_HYST_LEVELS";
	case CSR_PSKEY_RX_LEVEL_LOW_SIGNAL:
		return "RX_LEVEL_LOW_SIGNAL";
	case CSR_PSKEY_AGC_IQ_LVL_VALUES:
		return "AGC_IQ_LVL_VALUES";
	case CSR_PSKEY_MR_FTRIM_OFFSET_12DB:
		return "MR_FTRIM_OFFSET_12DB";
	case CSR_PSKEY_MR_FTRIM_OFFSET_6DB:
		return "MR_FTRIM_OFFSET_6DB";
	case CSR_PSKEY_NO_CAL_ON_BOOT:
		return "NO_CAL_ON_BOOT";
	case CSR_PSKEY_RSSI_HI_TARGET:
		return "RSSI_HI_TARGET";
	case CSR_PSKEY_PREFERRED_MIN_ATTENUATION:
		return "PREFERRED_MIN_ATTENUATION";
	case CSR_PSKEY_LC_COMBO_DOT11_PRIORITY_OVERRIDE:
		return "LC_COMBO_DOT11_PRIORITY_OVERRIDE";
	case CSR_PSKEY_LC_MULTISLOT_HOLDOFF:
		return "LC_MULTISLOT_HOLDOFF";
	case CSR_PSKEY_FREE_KEY_PIGEON_HOLE:
		return "FREE_KEY_PIGEON_HOLE";
	case CSR_PSKEY_LINK_KEY_BD_ADDR0:
		return "LINK_KEY_BD_ADDR0";
	case CSR_PSKEY_LINK_KEY_BD_ADDR1:
		return "LINK_KEY_BD_ADDR1";
	case CSR_PSKEY_LINK_KEY_BD_ADDR2:
		return "LINK_KEY_BD_ADDR2";
	case CSR_PSKEY_LINK_KEY_BD_ADDR3:
		return "LINK_KEY_BD_ADDR3";
	case CSR_PSKEY_LINK_KEY_BD_ADDR4:
		return "LINK_KEY_BD_ADDR4";
	case CSR_PSKEY_LINK_KEY_BD_ADDR5:
		return "LINK_KEY_BD_ADDR5";
	case CSR_PSKEY_LINK_KEY_BD_ADDR6:
		return "LINK_KEY_BD_ADDR6";
	case CSR_PSKEY_LINK_KEY_BD_ADDR7:
		return "LINK_KEY_BD_ADDR7";
	case CSR_PSKEY_LINK_KEY_BD_ADDR8:
		return "LINK_KEY_BD_ADDR8";
	case CSR_PSKEY_LINK_KEY_BD_ADDR9:
		return "LINK_KEY_BD_ADDR9";
	case CSR_PSKEY_LINK_KEY_BD_ADDR10:
		return "LINK_KEY_BD_ADDR10";
	case CSR_PSKEY_LINK_KEY_BD_ADDR11:
		return "LINK_KEY_BD_ADDR11";
	case CSR_PSKEY_LINK_KEY_BD_ADDR12:
		return "LINK_KEY_BD_ADDR12";
	case CSR_PSKEY_LINK_KEY_BD_ADDR13:
		return "LINK_KEY_BD_ADDR13";
	case CSR_PSKEY_LINK_KEY_BD_ADDR14:
		return "LINK_KEY_BD_ADDR14";
	case CSR_PSKEY_LINK_KEY_BD_ADDR15:
		return "LINK_KEY_BD_ADDR15";
	case CSR_PSKEY_ENC_KEY_LMIN:
		return "ENC_KEY_LMIN";
	case CSR_PSKEY_ENC_KEY_LMAX:
		return "ENC_KEY_LMAX";
	case CSR_PSKEY_LOCAL_SUPPORTED_FEATURES:
		return "LOCAL_SUPPORTED_FEATURES";
	case CSR_PSKEY_LM_USE_UNIT_KEY:
		return "LM_USE_UNIT_KEY";
	case CSR_PSKEY_HCI_NOP_DISABLE:
		return "HCI_NOP_DISABLE";
	case CSR_PSKEY_LM_MAX_EVENT_FILTERS:
		return "LM_MAX_EVENT_FILTERS";
	case CSR_PSKEY_LM_USE_ENC_MODE_BROADCAST:
		return "LM_USE_ENC_MODE_BROADCAST";
	case CSR_PSKEY_LM_TEST_SEND_ACCEPTED_TWICE:
		return "LM_TEST_SEND_ACCEPTED_TWICE";
	case CSR_PSKEY_LM_MAX_PAGE_HOLD_TIME:
		return "LM_MAX_PAGE_HOLD_TIME";
	case CSR_PSKEY_AFH_ADAPTATION_RESPONSE_TIME:
		return "AFH_ADAPTATION_RESPONSE_TIME";
	case CSR_PSKEY_AFH_OPTIONS:
		return "AFH_OPTIONS";
	case CSR_PSKEY_AFH_RSSI_RUN_PERIOD:
		return "AFH_RSSI_RUN_PERIOD";
	case CSR_PSKEY_AFH_REENABLE_CHANNEL_TIME:
		return "AFH_REENABLE_CHANNEL_TIME";
	case CSR_PSKEY_NO_DROP_ON_ACR_MS_FAIL:
		return "NO_DROP_ON_ACR_MS_FAIL";
	case CSR_PSKEY_MAX_PRIVATE_KEYS:
		return "MAX_PRIVATE_KEYS";
	case CSR_PSKEY_PRIVATE_LINK_KEY_BD_ADDR0:
		return "PRIVATE_LINK_KEY_BD_ADDR0";
	case CSR_PSKEY_PRIVATE_LINK_KEY_BD_ADDR1:
		return "PRIVATE_LINK_KEY_BD_ADDR1";
	case CSR_PSKEY_PRIVATE_LINK_KEY_BD_ADDR2:
		return "PRIVATE_LINK_KEY_BD_ADDR2";
	case CSR_PSKEY_PRIVATE_LINK_KEY_BD_ADDR3:
		return "PRIVATE_LINK_KEY_BD_ADDR3";
	case CSR_PSKEY_PRIVATE_LINK_KEY_BD_ADDR4:
		return "PRIVATE_LINK_KEY_BD_ADDR4";
	case CSR_PSKEY_PRIVATE_LINK_KEY_BD_ADDR5:
		return "PRIVATE_LINK_KEY_BD_ADDR5";
	case CSR_PSKEY_PRIVATE_LINK_KEY_BD_ADDR6:
		return "PRIVATE_LINK_KEY_BD_ADDR6";
	case CSR_PSKEY_PRIVATE_LINK_KEY_BD_ADDR7:
		return "PRIVATE_LINK_KEY_BD_ADDR7";
	case CSR_PSKEY_LOCAL_SUPPORTED_COMMANDS:
		return "LOCAL_SUPPORTED_COMMANDS";
	case CSR_PSKEY_LM_MAX_ABSENCE_INDEX:
		return "LM_MAX_ABSENCE_INDEX";
	case CSR_PSKEY_DEVICE_NAME:
		return "DEVICE_NAME";
	case CSR_PSKEY_AFH_RSSI_THRESHOLD:
		return "AFH_RSSI_THRESHOLD";
	case CSR_PSKEY_LM_CASUAL_SCAN_INTERVAL:
		return "LM_CASUAL_SCAN_INTERVAL";
	case CSR_PSKEY_AFH_MIN_MAP_CHANGE:
		return "AFH_MIN_MAP_CHANGE";
	case CSR_PSKEY_AFH_RSSI_LP_RUN_PERIOD:
		return "AFH_RSSI_LP_RUN_PERIOD";
	case CSR_PSKEY_HCI_LMP_LOCAL_VERSION:
		return "HCI_LMP_LOCAL_VERSION";
	case CSR_PSKEY_LMP_REMOTE_VERSION:
		return "LMP_REMOTE_VERSION";
	case CSR_PSKEY_HOLD_ERROR_MESSAGE_NUMBER:
		return "HOLD_ERROR_MESSAGE_NUMBER";
	case CSR_PSKEY_DFU_ATTRIBUTES:
		return "DFU_ATTRIBUTES";
	case CSR_PSKEY_DFU_DETACH_TO:
		return "DFU_DETACH_TO";
	case CSR_PSKEY_DFU_TRANSFER_SIZE:
		return "DFU_TRANSFER_SIZE";
	case CSR_PSKEY_DFU_ENABLE:
		return "DFU_ENABLE";
	case CSR_PSKEY_DFU_LIN_REG_ENABLE:
		return "DFU_LIN_REG_ENABLE";
	case CSR_PSKEY_DFUENC_VMAPP_PK_MODULUS_MSB:
		return "DFUENC_VMAPP_PK_MODULUS_MSB";
	case CSR_PSKEY_DFUENC_VMAPP_PK_MODULUS_LSB:
		return "DFUENC_VMAPP_PK_MODULUS_LSB";
	case CSR_PSKEY_DFUENC_VMAPP_PK_M_DASH:
		return "DFUENC_VMAPP_PK_M_DASH";
	case CSR_PSKEY_DFUENC_VMAPP_PK_R2N_MSB:
		return "DFUENC_VMAPP_PK_R2N_MSB";
	case CSR_PSKEY_DFUENC_VMAPP_PK_R2N_LSB:
		return "DFUENC_VMAPP_PK_R2N_LSB";
	case CSR_PSKEY_BCSP_LM_PS_BLOCK:
		return "BCSP_LM_PS_BLOCK";
	case CSR_PSKEY_HOSTIO_FC_PS_BLOCK:
		return "HOSTIO_FC_PS_BLOCK";
	case CSR_PSKEY_HOSTIO_PROTOCOL_INFO0:
		return "HOSTIO_PROTOCOL_INFO0";
	case CSR_PSKEY_HOSTIO_PROTOCOL_INFO1:
		return "HOSTIO_PROTOCOL_INFO1";
	case CSR_PSKEY_HOSTIO_PROTOCOL_INFO2:
		return "HOSTIO_PROTOCOL_INFO2";
	case CSR_PSKEY_HOSTIO_PROTOCOL_INFO3:
		return "HOSTIO_PROTOCOL_INFO3";
	case CSR_PSKEY_HOSTIO_PROTOCOL_INFO4:
		return "HOSTIO_PROTOCOL_INFO4";
	case CSR_PSKEY_HOSTIO_PROTOCOL_INFO5:
		return "HOSTIO_PROTOCOL_INFO5";
	case CSR_PSKEY_HOSTIO_PROTOCOL_INFO6:
		return "HOSTIO_PROTOCOL_INFO6";
	case CSR_PSKEY_HOSTIO_PROTOCOL_INFO7:
		return "HOSTIO_PROTOCOL_INFO7";
	case CSR_PSKEY_HOSTIO_PROTOCOL_INFO8:
		return "HOSTIO_PROTOCOL_INFO8";
	case CSR_PSKEY_HOSTIO_PROTOCOL_INFO9:
		return "HOSTIO_PROTOCOL_INFO9";
	case CSR_PSKEY_HOSTIO_PROTOCOL_INFO10:
		return "HOSTIO_PROTOCOL_INFO10";
	case CSR_PSKEY_HOSTIO_PROTOCOL_INFO11:
		return "HOSTIO_PROTOCOL_INFO11";
	case CSR_PSKEY_HOSTIO_PROTOCOL_INFO12:
		return "HOSTIO_PROTOCOL_INFO12";
	case CSR_PSKEY_HOSTIO_PROTOCOL_INFO13:
		return "HOSTIO_PROTOCOL_INFO13";
	case CSR_PSKEY_HOSTIO_PROTOCOL_INFO14:
		return "HOSTIO_PROTOCOL_INFO14";
	case CSR_PSKEY_HOSTIO_PROTOCOL_INFO15:
		return "HOSTIO_PROTOCOL_INFO15";
	case CSR_PSKEY_HOSTIO_UART_RESET_TIMEOUT:
		return "HOSTIO_UART_RESET_TIMEOUT";
	case CSR_PSKEY_HOSTIO_USE_HCI_EXTN:
		return "HOSTIO_USE_HCI_EXTN";
	case CSR_PSKEY_HOSTIO_USE_HCI_EXTN_CCFC:
		return "HOSTIO_USE_HCI_EXTN_CCFC";
	case CSR_PSKEY_HOSTIO_HCI_EXTN_PAYLOAD_SIZE:
		return "HOSTIO_HCI_EXTN_PAYLOAD_SIZE";
	case CSR_PSKEY_BCSP_LM_CNF_CNT_LIMIT:
		return "BCSP_LM_CNF_CNT_LIMIT";
	case CSR_PSKEY_HOSTIO_MAP_SCO_PCM:
		return "HOSTIO_MAP_SCO_PCM";
	case CSR_PSKEY_HOSTIO_AWKWARD_PCM_SYNC:
		return "HOSTIO_AWKWARD_PCM_SYNC";
	case CSR_PSKEY_HOSTIO_BREAK_POLL_PERIOD:
		return "HOSTIO_BREAK_POLL_PERIOD";
	case CSR_PSKEY_HOSTIO_MIN_UART_HCI_SCO_SIZE:
		return "HOSTIO_MIN_UART_HCI_SCO_SIZE";
	case CSR_PSKEY_HOSTIO_MAP_SCO_CODEC:
		return "HOSTIO_MAP_SCO_CODEC";
	case CSR_PSKEY_PCM_CVSD_TX_HI_FREQ_BOOST:
		return "PCM_CVSD_TX_HI_FREQ_BOOST";
	case CSR_PSKEY_PCM_CVSD_RX_HI_FREQ_BOOST:
		return "PCM_CVSD_RX_HI_FREQ_BOOST";
	case CSR_PSKEY_PCM_CONFIG32:
		return "PCM_CONFIG32";
	case CSR_PSKEY_USE_OLD_BCSP_LE:
		return "USE_OLD_BCSP_LE";
	case CSR_PSKEY_PCM_CVSD_USE_NEW_FILTER:
		return "PCM_CVSD_USE_NEW_FILTER";
	case CSR_PSKEY_PCM_FORMAT:
		return "PCM_FORMAT";
	case CSR_PSKEY_CODEC_OUT_GAIN:
		return "CODEC_OUT_GAIN";
	case CSR_PSKEY_CODEC_IN_GAIN:
		return "CODEC_IN_GAIN";
	case CSR_PSKEY_CODEC_PIO:
		return "CODEC_PIO";
	case CSR_PSKEY_PCM_LOW_JITTER_CONFIG:
		return "PCM_LOW_JITTER_CONFIG";
	case CSR_PSKEY_HOSTIO_SCO_PCM_THRESHOLDS:
		return "HOSTIO_SCO_PCM_THRESHOLDS";
	case CSR_PSKEY_HOSTIO_SCO_HCI_THRESHOLDS:
		return "HOSTIO_SCO_HCI_THRESHOLDS";
	case CSR_PSKEY_HOSTIO_MAP_SCO_PCM_SLOT:
		return "HOSTIO_MAP_SCO_PCM_SLOT";
	case CSR_PSKEY_UART_BAUDRATE:
		return "UART_BAUDRATE";
	case CSR_PSKEY_UART_CONFIG_BCSP:
		return "UART_CONFIG_BCSP";
	case CSR_PSKEY_UART_CONFIG_H4:
		return "UART_CONFIG_H4";
	case CSR_PSKEY_UART_CONFIG_H5:
		return "UART_CONFIG_H5";
	case CSR_PSKEY_UART_CONFIG_USR:
		return "UART_CONFIG_USR";
	case CSR_PSKEY_UART_TX_CRCS:
		return "UART_TX_CRCS";
	case CSR_PSKEY_UART_ACK_TIMEOUT:
		return "UART_ACK_TIMEOUT";
	case CSR_PSKEY_UART_TX_MAX_ATTEMPTS:
		return "UART_TX_MAX_ATTEMPTS";
	case CSR_PSKEY_UART_TX_WINDOW_SIZE:
		return "UART_TX_WINDOW_SIZE";
	case CSR_PSKEY_UART_HOST_WAKE:
		return "UART_HOST_WAKE";
	case CSR_PSKEY_HOSTIO_THROTTLE_TIMEOUT:
		return "HOSTIO_THROTTLE_TIMEOUT";
	case CSR_PSKEY_PCM_ALWAYS_ENABLE:
		return "PCM_ALWAYS_ENABLE";
	case CSR_PSKEY_UART_HOST_WAKE_SIGNAL:
		return "UART_HOST_WAKE_SIGNAL";
	case CSR_PSKEY_UART_CONFIG_H4DS:
		return "UART_CONFIG_H4DS";
	case CSR_PSKEY_H4DS_WAKE_DURATION:
		return "H4DS_WAKE_DURATION";
	case CSR_PSKEY_H4DS_MAXWU:
		return "H4DS_MAXWU";
	case CSR_PSKEY_H4DS_LE_TIMER_PERIOD:
		return "H4DS_LE_TIMER_PERIOD";
	case CSR_PSKEY_H4DS_TWU_TIMER_PERIOD:
		return "H4DS_TWU_TIMER_PERIOD";
	case CSR_PSKEY_H4DS_UART_IDLE_TIMER_PERIOD:
		return "H4DS_UART_IDLE_TIMER_PERIOD";
	case CSR_PSKEY_ANA_FTRIM:
		return "ANA_FTRIM";
	case CSR_PSKEY_WD_TIMEOUT:
		return "WD_TIMEOUT";
	case CSR_PSKEY_WD_PERIOD:
		return "WD_PERIOD";
	case CSR_PSKEY_HOST_INTERFACE:
		return "HOST_INTERFACE";
	case CSR_PSKEY_HQ_HOST_TIMEOUT:
		return "HQ_HOST_TIMEOUT";
	case CSR_PSKEY_HQ_ACTIVE:
		return "HQ_ACTIVE";
	case CSR_PSKEY_BCCMD_SECURITY_ACTIVE:
		return "BCCMD_SECURITY_ACTIVE";
	case CSR_PSKEY_ANA_FREQ:
		return "ANA_FREQ";
	case CSR_PSKEY_PIO_PROTECT_MASK:
		return "PIO_PROTECT_MASK";
	case CSR_PSKEY_PMALLOC_SIZES:
		return "PMALLOC_SIZES";
	case CSR_PSKEY_UART_BAUD_RATE:
		return "UART_BAUD_RATE";
	case CSR_PSKEY_UART_CONFIG:
		return "UART_CONFIG";
	case CSR_PSKEY_STUB:
		return "STUB";
	case CSR_PSKEY_TXRX_PIO_CONTROL:
		return "TXRX_PIO_CONTROL";
	case CSR_PSKEY_ANA_RX_LEVEL:
		return "ANA_RX_LEVEL";
	case CSR_PSKEY_ANA_RX_FTRIM:
		return "ANA_RX_FTRIM";
	case CSR_PSKEY_PSBC_DATA_VERSION:
		return "PSBC_DATA_VERSION";
	case CSR_PSKEY_PCM0_ATTENUATION:
		return "PCM0_ATTENUATION";
	case CSR_PSKEY_LO_LVL_MAX:
		return "LO_LVL_MAX";
	case CSR_PSKEY_LO_ADC_AMPL_MIN:
		return "LO_ADC_AMPL_MIN";
	case CSR_PSKEY_LO_ADC_AMPL_MAX:
		return "LO_ADC_AMPL_MAX";
	case CSR_PSKEY_IQ_TRIM_CHANNEL:
		return "IQ_TRIM_CHANNEL";
	case CSR_PSKEY_IQ_TRIM_GAIN:
		return "IQ_TRIM_GAIN";
	case CSR_PSKEY_IQ_TRIM_ENABLE:
		return "IQ_TRIM_ENABLE";
	case CSR_PSKEY_TX_OFFSET_HALF_MHZ:
		return "TX_OFFSET_HALF_MHZ";
	case CSR_PSKEY_GBL_MISC_ENABLES:
		return "GBL_MISC_ENABLES";
	case CSR_PSKEY_UART_SLEEP_TIMEOUT:
		return "UART_SLEEP_TIMEOUT";
	case CSR_PSKEY_DEEP_SLEEP_STATE:
		return "DEEP_SLEEP_STATE";
	case CSR_PSKEY_IQ_ENABLE_PHASE_TRIM:
		return "IQ_ENABLE_PHASE_TRIM";
	case CSR_PSKEY_HCI_HANDLE_FREEZE_PERIOD:
		return "HCI_HANDLE_FREEZE_PERIOD";
	case CSR_PSKEY_MAX_FROZEN_HCI_HANDLES:
		return "MAX_FROZEN_HCI_HANDLES";
	case CSR_PSKEY_PAGETABLE_DESTRUCTION_DELAY:
		return "PAGETABLE_DESTRUCTION_DELAY";
	case CSR_PSKEY_IQ_TRIM_PIO_SETTINGS:
		return "IQ_TRIM_PIO_SETTINGS";
	case CSR_PSKEY_USE_EXTERNAL_CLOCK:
		return "USE_EXTERNAL_CLOCK";
	case CSR_PSKEY_DEEP_SLEEP_WAKE_CTS:
		return "DEEP_SLEEP_WAKE_CTS";
	case CSR_PSKEY_FC_HC2H_FLUSH_DELAY:
		return "FC_HC2H_FLUSH_DELAY";
	case CSR_PSKEY_RX_HIGHSIDE:
		return "RX_HIGHSIDE";
	case CSR_PSKEY_TX_PRE_LVL:
		return "TX_PRE_LVL";
	case CSR_PSKEY_RX_SINGLE_ENDED:
		return "RX_SINGLE_ENDED";
	case CSR_PSKEY_TX_FILTER_CONFIG:
		return "TX_FILTER_CONFIG";
	case CSR_PSKEY_CLOCK_REQUEST_ENABLE:
		return "CLOCK_REQUEST_ENABLE";
	case CSR_PSKEY_RX_MIN_ATTEN:
		return "RX_MIN_ATTEN";
	case CSR_PSKEY_XTAL_TARGET_AMPLITUDE:
		return "XTAL_TARGET_AMPLITUDE";
	case CSR_PSKEY_PCM_MIN_CPU_CLOCK:
		return "PCM_MIN_CPU_CLOCK";
	case CSR_PSKEY_HOST_INTERFACE_PIO_USB:
		return "HOST_INTERFACE_PIO_USB";
	case CSR_PSKEY_CPU_IDLE_MODE:
		return "CPU_IDLE_MODE";
	case CSR_PSKEY_DEEP_SLEEP_CLEAR_RTS:
		return "DEEP_SLEEP_CLEAR_RTS";
	case CSR_PSKEY_RF_RESONANCE_TRIM:
		return "RF_RESONANCE_TRIM";
	case CSR_PSKEY_DEEP_SLEEP_PIO_WAKE:
		return "DEEP_SLEEP_PIO_WAKE";
	case CSR_PSKEY_DRAIN_BORE_TIMERS:
		return "DRAIN_BORE_TIMERS";
	case CSR_PSKEY_DRAIN_TX_POWER_BASE:
		return "DRAIN_TX_POWER_BASE";
	case CSR_PSKEY_MODULE_ID:
		return "MODULE_ID";
	case CSR_PSKEY_MODULE_DESIGN:
		return "MODULE_DESIGN";
	case CSR_PSKEY_MODULE_SECURITY_CODE:
		return "MODULE_SECURITY_CODE";
	case CSR_PSKEY_VM_DISABLE:
		return "VM_DISABLE";
	case CSR_PSKEY_MOD_MANUF0:
		return "MOD_MANUF0";
	case CSR_PSKEY_MOD_MANUF1:
		return "MOD_MANUF1";
	case CSR_PSKEY_MOD_MANUF2:
		return "MOD_MANUF2";
	case CSR_PSKEY_MOD_MANUF3:
		return "MOD_MANUF3";
	case CSR_PSKEY_MOD_MANUF4:
		return "MOD_MANUF4";
	case CSR_PSKEY_MOD_MANUF5:
		return "MOD_MANUF5";
	case CSR_PSKEY_MOD_MANUF6:
		return "MOD_MANUF6";
	case CSR_PSKEY_MOD_MANUF7:
		return "MOD_MANUF7";
	case CSR_PSKEY_MOD_MANUF8:
		return "MOD_MANUF8";
	case CSR_PSKEY_MOD_MANUF9:
		return "MOD_MANUF9";
	case CSR_PSKEY_DUT_VM_DISABLE:
		return "DUT_VM_DISABLE";
	case CSR_PSKEY_USR0:
		return "USR0";
	case CSR_PSKEY_USR1:
		return "USR1";
	case CSR_PSKEY_USR2:
		return "USR2";
	case CSR_PSKEY_USR3:
		return "USR3";
	case CSR_PSKEY_USR4:
		return "USR4";
	case CSR_PSKEY_USR5:
		return "USR5";
	case CSR_PSKEY_USR6:
		return "USR6";
	case CSR_PSKEY_USR7:
		return "USR7";
	case CSR_PSKEY_USR8:
		return "USR8";
	case CSR_PSKEY_USR9:
		return "USR9";
	case CSR_PSKEY_USR10:
		return "USR10";
	case CSR_PSKEY_USR11:
		return "USR11";
	case CSR_PSKEY_USR12:
		return "USR12";
	case CSR_PSKEY_USR13:
		return "USR13";
	case CSR_PSKEY_USR14:
		return "USR14";
	case CSR_PSKEY_USR15:
		return "USR15";
	case CSR_PSKEY_USR16:
		return "USR16";
	case CSR_PSKEY_USR17:
		return "USR17";
	case CSR_PSKEY_USR18:
		return "USR18";
	case CSR_PSKEY_USR19:
		return "USR19";
	case CSR_PSKEY_USR20:
		return "USR20";
	case CSR_PSKEY_USR21:
		return "USR21";
	case CSR_PSKEY_USR22:
		return "USR22";
	case CSR_PSKEY_USR23:
		return "USR23";
	case CSR_PSKEY_USR24:
		return "USR24";
	case CSR_PSKEY_USR25:
		return "USR25";
	case CSR_PSKEY_USR26:
		return "USR26";
	case CSR_PSKEY_USR27:
		return "USR27";
	case CSR_PSKEY_USR28:
		return "USR28";
	case CSR_PSKEY_USR29:
		return "USR29";
	case CSR_PSKEY_USR30:
		return "USR30";
	case CSR_PSKEY_USR31:
		return "USR31";
	case CSR_PSKEY_USR32:
		return "USR32";
	case CSR_PSKEY_USR33:
		return "USR33";
	case CSR_PSKEY_USR34:
		return "USR34";
	case CSR_PSKEY_USR35:
		return "USR35";
	case CSR_PSKEY_USR36:
		return "USR36";
	case CSR_PSKEY_USR37:
		return "USR37";
	case CSR_PSKEY_USR38:
		return "USR38";
	case CSR_PSKEY_USR39:
		return "USR39";
	case CSR_PSKEY_USR40:
		return "USR40";
	case CSR_PSKEY_USR41:
		return "USR41";
	case CSR_PSKEY_USR42:
		return "USR42";
	case CSR_PSKEY_USR43:
		return "USR43";
	case CSR_PSKEY_USR44:
		return "USR44";
	case CSR_PSKEY_USR45:
		return "USR45";
	case CSR_PSKEY_USR46:
		return "USR46";
	case CSR_PSKEY_USR47:
		return "USR47";
	case CSR_PSKEY_USR48:
		return "USR48";
	case CSR_PSKEY_USR49:
		return "USR49";
	case CSR_PSKEY_USB_VERSION:
		return "USB_VERSION";
	case CSR_PSKEY_USB_DEVICE_CLASS_CODES:
		return "USB_DEVICE_CLASS_CODES";
	case CSR_PSKEY_USB_VENDOR_ID:
		return "USB_VENDOR_ID";
	case CSR_PSKEY_USB_PRODUCT_ID:
		return "USB_PRODUCT_ID";
	case CSR_PSKEY_USB_MANUF_STRING:
		return "USB_MANUF_STRING";
	case CSR_PSKEY_USB_PRODUCT_STRING:
		return "USB_PRODUCT_STRING";
	case CSR_PSKEY_USB_SERIAL_NUMBER_STRING:
		return "USB_SERIAL_NUMBER_STRING";
	case CSR_PSKEY_USB_CONFIG_STRING:
		return "USB_CONFIG_STRING";
	case CSR_PSKEY_USB_ATTRIBUTES:
		return "USB_ATTRIBUTES";
	case CSR_PSKEY_USB_MAX_POWER:
		return "USB_MAX_POWER";
	case CSR_PSKEY_USB_BT_IF_CLASS_CODES:
		return "USB_BT_IF_CLASS_CODES";
	case CSR_PSKEY_USB_LANGID:
		return "USB_LANGID";
	case CSR_PSKEY_USB_DFU_CLASS_CODES:
		return "USB_DFU_CLASS_CODES";
	case CSR_PSKEY_USB_DFU_PRODUCT_ID:
		return "USB_DFU_PRODUCT_ID";
	case CSR_PSKEY_USB_PIO_DETACH:
		return "USB_PIO_DETACH";
	case CSR_PSKEY_USB_PIO_WAKEUP:
		return "USB_PIO_WAKEUP";
	case CSR_PSKEY_USB_PIO_PULLUP:
		return "USB_PIO_PULLUP";
	case CSR_PSKEY_USB_PIO_VBUS:
		return "USB_PIO_VBUS";
	case CSR_PSKEY_USB_PIO_WAKE_TIMEOUT:
		return "USB_PIO_WAKE_TIMEOUT";
	case CSR_PSKEY_USB_PIO_RESUME:
		return "USB_PIO_RESUME";
	case CSR_PSKEY_USB_BT_SCO_IF_CLASS_CODES:
		return "USB_BT_SCO_IF_CLASS_CODES";
	case CSR_PSKEY_USB_SUSPEND_PIO_LEVEL:
		return "USB_SUSPEND_PIO_LEVEL";
	case CSR_PSKEY_USB_SUSPEND_PIO_DIR:
		return "USB_SUSPEND_PIO_DIR";
	case CSR_PSKEY_USB_SUSPEND_PIO_MASK:
		return "USB_SUSPEND_PIO_MASK";
	case CSR_PSKEY_USB_ENDPOINT_0_MAX_PACKET_SIZE:
		return "USB_ENDPOINT_0_MAX_PACKET_SIZE";
	case CSR_PSKEY_USB_CONFIG:
		return "USB_CONFIG";
	case CSR_PSKEY_RADIOTEST_ATTEN_INIT:
		return "RADIOTEST_ATTEN_INIT";
	case CSR_PSKEY_RADIOTEST_FIRST_TRIM_TIME:
		return "RADIOTEST_FIRST_TRIM_TIME";
	case CSR_PSKEY_RADIOTEST_SUBSEQUENT_TRIM_TIME:
		return "RADIOTEST_SUBSEQUENT_TRIM_TIME";
	case CSR_PSKEY_RADIOTEST_LO_LVL_TRIM_ENABLE:
		return "RADIOTEST_LO_LVL_TRIM_ENABLE";
	case CSR_PSKEY_RADIOTEST_DISABLE_MODULATION:
		return "RADIOTEST_DISABLE_MODULATION";
	case CSR_PSKEY_RFCOMM_FCON_THRESHOLD:
		return "RFCOMM_FCON_THRESHOLD";
	case CSR_PSKEY_RFCOMM_FCOFF_THRESHOLD:
		return "RFCOMM_FCOFF_THRESHOLD";
	case CSR_PSKEY_IPV6_STATIC_ADDR:
		return "IPV6_STATIC_ADDR";
	case CSR_PSKEY_IPV4_STATIC_ADDR:
		return "IPV4_STATIC_ADDR";
	case CSR_PSKEY_IPV6_STATIC_PREFIX_LEN:
		return "IPV6_STATIC_PREFIX_LEN";
	case CSR_PSKEY_IPV6_STATIC_ROUTER_ADDR:
		return "IPV6_STATIC_ROUTER_ADDR";
	case CSR_PSKEY_IPV4_STATIC_SUBNET_MASK:
		return "IPV4_STATIC_SUBNET_MASK";
	case CSR_PSKEY_IPV4_STATIC_ROUTER_ADDR:
		return "IPV4_STATIC_ROUTER_ADDR";
	case CSR_PSKEY_MDNS_NAME:
		return "MDNS_NAME";
	case CSR_PSKEY_FIXED_PIN:
		return "FIXED_PIN";
	case CSR_PSKEY_MDNS_PORT:
		return "MDNS_PORT";
	case CSR_PSKEY_MDNS_TTL:
		return "MDNS_TTL";
	case CSR_PSKEY_MDNS_IPV4_ADDR:
		return "MDNS_IPV4_ADDR";
	case CSR_PSKEY_ARP_CACHE_TIMEOUT:
		return "ARP_CACHE_TIMEOUT";
	case CSR_PSKEY_HFP_POWER_TABLE:
		return "HFP_POWER_TABLE";
	case CSR_PSKEY_DRAIN_BORE_TIMER_COUNTERS:
		return "DRAIN_BORE_TIMER_COUNTERS";
	case CSR_PSKEY_DRAIN_BORE_COUNTERS:
		return "DRAIN_BORE_COUNTERS";
	case CSR_PSKEY_LOOP_FILTER_TRIM:
		return "LOOP_FILTER_TRIM";
	case CSR_PSKEY_DRAIN_BORE_CURRENT_PEAK:
		return "DRAIN_BORE_CURRENT_PEAK";
	case CSR_PSKEY_VM_E2_CACHE_LIMIT:
		return "VM_E2_CACHE_LIMIT";
	case CSR_PSKEY_FORCE_16MHZ_REF_PIO:
		return "FORCE_16MHZ_REF_PIO";
	case CSR_PSKEY_CDMA_LO_REF_LIMITS:
		return "CDMA_LO_REF_LIMITS";
	case CSR_PSKEY_CDMA_LO_ERROR_LIMITS:
		return "CDMA_LO_ERROR_LIMITS";
	case CSR_PSKEY_CLOCK_STARTUP_DELAY:
		return "CLOCK_STARTUP_DELAY";
	case CSR_PSKEY_DEEP_SLEEP_CORRECTION_FACTOR:
		return "DEEP_SLEEP_CORRECTION_FACTOR";
	case CSR_PSKEY_TEMPERATURE_CALIBRATION:
		return "TEMPERATURE_CALIBRATION";
	case CSR_PSKEY_TEMPERATURE_VS_DELTA_INTERNAL_PA:
		return "TEMPERATURE_VS_DELTA_INTERNAL_PA";
	case CSR_PSKEY_TEMPERATURE_VS_DELTA_TX_PRE_LVL:
		return "TEMPERATURE_VS_DELTA_TX_PRE_LVL";
	case CSR_PSKEY_TEMPERATURE_VS_DELTA_TX_BB:
		return "TEMPERATURE_VS_DELTA_TX_BB";
	case CSR_PSKEY_TEMPERATURE_VS_DELTA_ANA_FTRIM:
		return "TEMPERATURE_VS_DELTA_ANA_FTRIM";
	case CSR_PSKEY_TEST_DELTA_OFFSET:
		return "TEST_DELTA_OFFSET";
	case CSR_PSKEY_RX_DYNAMIC_LVL_OFFSET:
		return "RX_DYNAMIC_LVL_OFFSET";
	case CSR_PSKEY_TEST_FORCE_OFFSET:
		return "TEST_FORCE_OFFSET";
	case CSR_PSKEY_RF_TRAP_BAD_DIVISION_RATIOS:
		return "RF_TRAP_BAD_DIVISION_RATIOS";
	case CSR_PSKEY_RADIOTEST_CDMA_LO_REF_LIMITS:
		return "RADIOTEST_CDMA_LO_REF_LIMITS";
	case CSR_PSKEY_INITIAL_BOOTMODE:
		return "INITIAL_BOOTMODE";
	case CSR_PSKEY_ONCHIP_HCI_CLIENT:
		return "ONCHIP_HCI_CLIENT";
	case CSR_PSKEY_RX_ATTEN_BACKOFF:
		return "RX_ATTEN_BACKOFF";
	case CSR_PSKEY_RX_ATTEN_UPDATE_RATE:
		return "RX_ATTEN_UPDATE_RATE";
	case CSR_PSKEY_SYNTH_TXRX_THRESHOLDS:
		return "SYNTH_TXRX_THRESHOLDS";
	case CSR_PSKEY_MIN_WAIT_STATES:
		return "MIN_WAIT_STATES";
	case CSR_PSKEY_RSSI_CORRECTION:
		return "RSSI_CORRECTION";
	case CSR_PSKEY_SCHED_THROTTLE_TIMEOUT:
		return "SCHED_THROTTLE_TIMEOUT";
	case CSR_PSKEY_DEEP_SLEEP_USE_EXTERNAL_CLOCK:
		return "DEEP_SLEEP_USE_EXTERNAL_CLOCK";
	case CSR_PSKEY_TRIM_RADIO_FILTERS:
		return "TRIM_RADIO_FILTERS";
	case CSR_PSKEY_TRANSMIT_OFFSET:
		return "TRANSMIT_OFFSET";
	case CSR_PSKEY_USB_VM_CONTROL:
		return "USB_VM_CONTROL";
	case CSR_PSKEY_MR_ANA_RX_FTRIM:
		return "MR_ANA_RX_FTRIM";
	case CSR_PSKEY_I2C_CONFIG:
		return "I2C_CONFIG";
	case CSR_PSKEY_IQ_LVL_RX:
		return "IQ_LVL_RX";
	case CSR_PSKEY_MR_TX_FILTER_CONFIG:
		return "MR_TX_FILTER_CONFIG";
	case CSR_PSKEY_MR_TX_CONFIG2:
		return "MR_TX_CONFIG2";
	case CSR_PSKEY_USB_DONT_RESET_BOOTMODE_ON_HOST_RESET:
		return "USB_DONT_RESET_BOOTMODE_ON_HOST_RESET";
	case CSR_PSKEY_LC_USE_THROTTLING:
		return "LC_USE_THROTTLING";
	case CSR_PSKEY_CHARGER_TRIM:
		return "CHARGER_TRIM";
	case CSR_PSKEY_CLOCK_REQUEST_FEATURES:
		return "CLOCK_REQUEST_FEATURES";
	case CSR_PSKEY_TRANSMIT_OFFSET_CLASS1:
		return "TRANSMIT_OFFSET_CLASS1";
	case CSR_PSKEY_TX_AVOID_PA_CLASS1_PIO:
		return "TX_AVOID_PA_CLASS1_PIO";
	case CSR_PSKEY_MR_PIO_CONFIG:
		return "MR_PIO_CONFIG";
	case CSR_PSKEY_UART_CONFIG2:
		return "UART_CONFIG2";
	case CSR_PSKEY_CLASS1_IQ_LVL:
		return "CLASS1_IQ_LVL";
	case CSR_PSKEY_CLASS1_TX_CONFIG2:
		return "CLASS1_TX_CONFIG2";
	case CSR_PSKEY_TEMPERATURE_VS_DELTA_INTERNAL_PA_CLASS1:
		return "TEMPERATURE_VS_DELTA_INTERNAL_PA_CLASS1";
	case CSR_PSKEY_TEMPERATURE_VS_DELTA_EXTERNAL_PA_CLASS1:
		return "TEMPERATURE_VS_DELTA_EXTERNAL_PA_CLASS1";
	case CSR_PSKEY_TEMPERATURE_VS_DELTA_TX_PRE_LVL_MR:
		return "TEMPERATURE_VS_DELTA_TX_PRE_LVL_MR";
	case CSR_PSKEY_TEMPERATURE_VS_DELTA_TX_BB_MR_HEADER:
		return "TEMPERATURE_VS_DELTA_TX_BB_MR_HEADER";
	case CSR_PSKEY_TEMPERATURE_VS_DELTA_TX_BB_MR_PAYLOAD:
		return "TEMPERATURE_VS_DELTA_TX_BB_MR_PAYLOAD";
	case CSR_PSKEY_RX_MR_EQ_TAPS:
		return "RX_MR_EQ_TAPS";
	case CSR_PSKEY_TX_PRE_LVL_CLASS1:
		return "TX_PRE_LVL_CLASS1";
	case CSR_PSKEY_ANALOGUE_ATTENUATOR:
		return "ANALOGUE_ATTENUATOR";
	case CSR_PSKEY_MR_RX_FILTER_TRIM:
		return "MR_RX_FILTER_TRIM";
	case CSR_PSKEY_MR_RX_FILTER_RESPONSE:
		return "MR_RX_FILTER_RESPONSE";
	case CSR_PSKEY_PIO_WAKEUP_STATE:
		return "PIO_WAKEUP_STATE";
	case CSR_PSKEY_MR_TX_IF_ATTEN_OFF_TEMP:
		return "MR_TX_IF_ATTEN_OFF_TEMP";
	case CSR_PSKEY_LO_DIV_LATCH_BYPASS:
		return "LO_DIV_LATCH_BYPASS";
	case CSR_PSKEY_LO_VCO_STANDBY:
		return "LO_VCO_STANDBY";
	case CSR_PSKEY_SLOW_CLOCK_FILTER_SHIFT:
		return "SLOW_CLOCK_FILTER_SHIFT";
	case CSR_PSKEY_SLOW_CLOCK_FILTER_DIVIDER:
		return "SLOW_CLOCK_FILTER_DIVIDER";
	case CSR_PSKEY_USB_ATTRIBUTES_POWER:
		return "USB_ATTRIBUTES_POWER";
	case CSR_PSKEY_USB_ATTRIBUTES_WAKEUP:
		return "USB_ATTRIBUTES_WAKEUP";
	case CSR_PSKEY_DFU_ATTRIBUTES_MANIFESTATION_TOLERANT:
		return "DFU_ATTRIBUTES_MANIFESTATION_TOLERANT";
	case CSR_PSKEY_DFU_ATTRIBUTES_CAN_UPLOAD:
		return "DFU_ATTRIBUTES_CAN_UPLOAD";
	case CSR_PSKEY_DFU_ATTRIBUTES_CAN_DOWNLOAD:
		return "DFU_ATTRIBUTES_CAN_DOWNLOAD";
	case CSR_PSKEY_UART_CONFIG_STOP_BITS:
		return "UART_CONFIG_STOP_BITS";
	case CSR_PSKEY_UART_CONFIG_PARITY_BIT:
		return "UART_CONFIG_PARITY_BIT";
	case CSR_PSKEY_UART_CONFIG_FLOW_CTRL_EN:
		return "UART_CONFIG_FLOW_CTRL_EN";
	case CSR_PSKEY_UART_CONFIG_RTS_AUTO_EN:
		return "UART_CONFIG_RTS_AUTO_EN";
	case CSR_PSKEY_UART_CONFIG_RTS:
		return "UART_CONFIG_RTS";
	case CSR_PSKEY_UART_CONFIG_TX_ZERO_EN:
		return "UART_CONFIG_TX_ZERO_EN";
	case CSR_PSKEY_UART_CONFIG_NON_BCSP_EN:
		return "UART_CONFIG_NON_BCSP_EN";
	case CSR_PSKEY_UART_CONFIG_RX_RATE_DELAY:
		return "UART_CONFIG_RX_RATE_DELAY";
	case CSR_PSKEY_UART_SEQ_TIMEOUT:
		return "UART_SEQ_TIMEOUT";
	case CSR_PSKEY_UART_SEQ_RETRIES:
		return "UART_SEQ_RETRIES";
	case CSR_PSKEY_UART_SEQ_WINSIZE:
		return "UART_SEQ_WINSIZE";
	case CSR_PSKEY_UART_USE_CRC_ON_TX:
		return "UART_USE_CRC_ON_TX";
	case CSR_PSKEY_UART_HOST_INITIAL_STATE:
		return "UART_HOST_INITIAL_STATE";
	case CSR_PSKEY_UART_HOST_ATTENTION_SPAN:
		return "UART_HOST_ATTENTION_SPAN";
	case CSR_PSKEY_UART_HOST_WAKEUP_TIME:
		return "UART_HOST_WAKEUP_TIME";
	case CSR_PSKEY_UART_HOST_WAKEUP_WAIT:
		return "UART_HOST_WAKEUP_WAIT";
	case CSR_PSKEY_BCSP_LM_MODE:
		return "BCSP_LM_MODE";
	case CSR_PSKEY_BCSP_LM_SYNC_RETRIES:
		return "BCSP_LM_SYNC_RETRIES";
	case CSR_PSKEY_BCSP_LM_TSHY:
		return "BCSP_LM_TSHY";
	case CSR_PSKEY_UART_DFU_CONFIG_STOP_BITS:
		return "UART_DFU_CONFIG_STOP_BITS";
	case CSR_PSKEY_UART_DFU_CONFIG_PARITY_BIT:
		return "UART_DFU_CONFIG_PARITY_BIT";
	case CSR_PSKEY_UART_DFU_CONFIG_FLOW_CTRL_EN:
		return "UART_DFU_CONFIG_FLOW_CTRL_EN";
	case CSR_PSKEY_UART_DFU_CONFIG_RTS_AUTO_EN:
		return "UART_DFU_CONFIG_RTS_AUTO_EN";
	case CSR_PSKEY_UART_DFU_CONFIG_RTS:
		return "UART_DFU_CONFIG_RTS";
	case CSR_PSKEY_UART_DFU_CONFIG_TX_ZERO_EN:
		return "UART_DFU_CONFIG_TX_ZERO_EN";
	case CSR_PSKEY_UART_DFU_CONFIG_NON_BCSP_EN:
		return "UART_DFU_CONFIG_NON_BCSP_EN";
	case CSR_PSKEY_UART_DFU_CONFIG_RX_RATE_DELAY:
		return "UART_DFU_CONFIG_RX_RATE_DELAY";
	case CSR_PSKEY_AMUX_AIO0:
		return "AMUX_AIO0";
	case CSR_PSKEY_AMUX_AIO1:
		return "AMUX_AIO1";
	case CSR_PSKEY_AMUX_AIO2:
		return "AMUX_AIO2";
	case CSR_PSKEY_AMUX_AIO3:
		return "AMUX_AIO3";
	case CSR_PSKEY_LOCAL_NAME_SIMPLIFIED:
		return "LOCAL_NAME_SIMPLIFIED";
	case CSR_PSKEY_EXTENDED_STUB:
		return "EXTENDED_STUB";
	default:
		return "UNKNOWN";
	}
}

int csr_write_varid_valueless(int dd, uint16_t seqnum, uint16_t varid)
{
	unsigned char cmd[] = { 0x02, 0x00, 0x09, 0x00,
				seqnum & 0xff, seqnum >> 8, varid & 0xff, varid >> 8, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

	unsigned char cp[254], rp[254];
	struct hci_request rq;

	memset(&cp, 0, sizeof(cp));
	cp[0] = 0xc2;
	memcpy(cp + 1, cmd, sizeof(cmd));

	switch (varid) {
	case CSR_VARID_COLD_RESET:
	case CSR_VARID_WARM_RESET:
	case CSR_VARID_COLD_HALT:
	case CSR_VARID_WARM_HALT:
		return hci_send_cmd(dd, OGF_VENDOR_CMD, 0x00, sizeof(cmd) + 1, cp);
	}

	memset(&rq, 0, sizeof(rq));
	rq.ogf    = OGF_VENDOR_CMD;
	rq.ocf    = 0x00;
	rq.event  = EVT_VENDOR;
	rq.cparam = cp;
	rq.clen   = sizeof(cmd) + 1;
	rq.rparam = rp;
	rq.rlen   = sizeof(rp);

	if (hci_send_req(dd, &rq, 2000) < 0)
		return -1;

	if (rp[0] != 0xc2) {
		errno = EIO;
		return -1;
	}

	if ((rp[9] + (rp[10] << 8)) != 0) {
		errno = ENXIO;
		return -1;
	}

	return 0;
}

int csr_write_varid_complex(int dd, uint16_t seqnum, uint16_t varid, uint8_t *value, uint16_t length)
{
	unsigned char cmd[] = { 0x02, 0x00, ((length / 2) + 5) & 0xff, ((length / 2) + 5) >> 8,
				seqnum & 0xff, seqnum >> 8, varid & 0xff, varid >> 8, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

	unsigned char cp[254], rp[254];
	struct hci_request rq;

	memset(&cp, 0, sizeof(cp));
	cp[0] = 0xc2;
	memcpy(cp + 1, cmd, sizeof(cmd));
	memcpy(cp + 11, value, length);

	memset(&rq, 0, sizeof(rq));
	rq.ogf    = OGF_VENDOR_CMD;
	rq.ocf    = 0x00;
	rq.event  = EVT_VENDOR;
	rq.cparam = cp;
	rq.clen   = sizeof(cmd) + length + 1;
	rq.rparam = rp;
	rq.rlen   = sizeof(rp);

	if (hci_send_req(dd, &rq, 2000) < 0)
		return -1;

	if (rp[0] != 0xc2) {
		errno = EIO;
		return -1;
	}

	if ((rp[9] + (rp[10] << 8)) != 0) {
		errno = ENXIO;
		return -1;
	}

	return 0;
}

int csr_read_varid_complex(int dd, uint16_t seqnum, uint16_t varid, uint8_t *value, uint16_t length)
{
	unsigned char cmd[] = { 0x00, 0x00, ((length / 2) + 5) & 0xff, ((length / 2) + 5) >> 8,
				seqnum & 0xff, seqnum >> 8, varid & 0xff, varid >> 8, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

	unsigned char cp[254], rp[254];
	struct hci_request rq;

	memset(&cp, 0, sizeof(cp));
	cp[0] = 0xc2;
	memcpy(cp + 1, cmd, sizeof(cmd));
	memcpy(cp + 11, value, length);

	memset(&rq, 0, sizeof(rq));
	rq.ogf    = OGF_VENDOR_CMD;
	rq.ocf    = 0x00;
	rq.event  = EVT_VENDOR;
	rq.cparam = cp;
	rq.clen   = sizeof(cmd) + length + 1;
	rq.rparam = rp;
	rq.rlen   = sizeof(rp);

	if (hci_send_req(dd, &rq, 2000) < 0)
		return -1;

	if (rp[0] != 0xc2) {
		errno = EIO;
		return -1;
	}

	if ((rp[9] + (rp[10] << 8)) != 0) {
		errno = ENXIO;
		return -1;
	}

	memcpy(value, rp + 11, length);

	return 0;
}

int csr_read_varid_uint16(int dd, uint16_t seqnum, uint16_t varid, uint16_t *value)
{
	unsigned char cmd[] = { 0x00, 0x00, 0x09, 0x00,
				seqnum & 0xff, seqnum >> 8, varid & 0xff, varid >> 8, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

	unsigned char cp[254], rp[254];
	struct hci_request rq;

	memset(&cp, 0, sizeof(cp));
	cp[0] = 0xc2;
	memcpy(cp + 1, cmd, sizeof(cmd));

	memset(&rq, 0, sizeof(rq));
	rq.ogf    = OGF_VENDOR_CMD;
	rq.ocf    = 0x00;
	rq.event  = EVT_VENDOR;
	rq.cparam = cp;
	rq.clen   = sizeof(cmd) + 1;
	rq.rparam = rp;
	rq.rlen   = sizeof(rp);

	if (hci_send_req(dd, &rq, 2000) < 0)
		return -1;

	if (rp[0] != 0xc2) {
		errno = EIO;
		return -1;
	}

	if ((rp[9] + (rp[10] << 8)) != 0) {
		errno = ENXIO;
		return -1;
	}

	*value = rp[11] + (rp[12] << 8);

	return 0;
}

int csr_read_varid_uint32(int dd, uint16_t seqnum, uint16_t varid, uint32_t *value)
{
	unsigned char cmd[] = { 0x00, 0x00, 0x09, 0x00,
				seqnum & 0xff, seqnum >> 8, varid & 0xff, varid >> 8, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

	unsigned char cp[254], rp[254];
	struct hci_request rq;

	memset(&cp, 0, sizeof(cp));
	cp[0] = 0xc2;
	memcpy(cp + 1, cmd, sizeof(cmd));

	memset(&rq, 0, sizeof(rq));
	rq.ogf    = OGF_VENDOR_CMD;
	rq.ocf    = 0x00;
	rq.event  = EVT_VENDOR;
	rq.cparam = cp;
	rq.clen   = sizeof(cmd) + 1;
	rq.rparam = rp;
	rq.rlen   = sizeof(rp);

	if (hci_send_req(dd, &rq, 2000) < 0)
		return -1;

	if (rp[0] != 0xc2) {
		errno = EIO;
		return -1;
	}

	if ((rp[9] + (rp[10] << 8)) != 0) {
		errno = ENXIO;
		return -1;
	}

	*value = ((rp[11] + (rp[12] << 8)) << 16) + (rp[13] + (rp[14] << 8));

	return 0;
}

int csr_read_pskey_complex(int dd, uint16_t seqnum, uint16_t pskey, uint16_t stores, uint8_t *value, uint16_t length)
{
	unsigned char cmd[] = { 0x00, 0x00, ((length / 2) + 8) & 0xff, ((length / 2) + 8) >> 8,
				seqnum & 0xff, seqnum >> 8, 0x03, 0x70, 0x00, 0x00,
				pskey & 0xff, pskey >> 8,
				(length / 2) & 0xff, (length / 2) >> 8,
				stores & 0xff, stores >> 8, 0x00, 0x00 };

	unsigned char cp[254], rp[254];
	struct hci_request rq;

	memset(&cp, 0, sizeof(cp));
	cp[0] = 0xc2;
	memcpy(cp + 1, cmd, sizeof(cmd));

	memset(&rq, 0, sizeof(rq));
	rq.ogf    = OGF_VENDOR_CMD;
	rq.ocf    = 0x00;
	rq.event  = EVT_VENDOR;
	rq.cparam = cp;
	rq.clen   = sizeof(cmd) + length - 1;
	rq.rparam = rp;
	rq.rlen   = sizeof(rp);

	if (hci_send_req(dd, &rq, 2000) < 0)
		return -1;

	if (rp[0] != 0xc2) {
		errno = EIO;
		return -1;
	}

	if ((rp[9] + (rp[10] << 8)) != 0) {
		errno = ENXIO;
		return -1;
	}

	memcpy(value, rp + 17, length);

	return 0;
}

int csr_write_pskey_complex(int dd, uint16_t seqnum, uint16_t pskey, uint16_t stores, uint8_t *value, uint16_t length)
{
	unsigned char cmd[] = { 0x02, 0x00, ((length / 2) + 8) & 0xff, ((length / 2) + 8) >> 8,
				seqnum & 0xff, seqnum >> 8, 0x03, 0x70, 0x00, 0x00,
				pskey & 0xff, pskey >> 8,
				(length / 2) & 0xff, (length / 2) >> 8,
				stores & 0xff, stores >> 8, 0x00, 0x00 };

	unsigned char cp[254], rp[254];
	struct hci_request rq;

	memset(&cp, 0, sizeof(cp));
	cp[0] = 0xc2;
	memcpy(cp + 1, cmd, sizeof(cmd));

	memcpy(cp + 17, value, length);

	memset(&rq, 0, sizeof(rq));
	rq.ogf    = OGF_VENDOR_CMD;
	rq.ocf    = 0x00;
	rq.event  = EVT_VENDOR;
	rq.cparam = cp;
	rq.clen   = sizeof(cmd) + length - 1;
	rq.rparam = rp;
	rq.rlen   = sizeof(rp);

	if (hci_send_req(dd, &rq, 2000) < 0)
		return -1;

	if (rp[0] != 0xc2) {
		errno = EIO;
		return -1;
	}

	if ((rp[9] + (rp[10] << 8)) != 0) {
		errno = ENXIO;
		return -1;
	}

	return 0;
}

int csr_read_pskey_uint16(int dd, uint16_t seqnum, uint16_t pskey, uint16_t stores, uint16_t *value)
{
	uint8_t array[2] = { 0x00, 0x00 };
	int err;

	err = csr_read_pskey_complex(dd, seqnum, pskey, stores, array, 2);

	*value = array[0] + (array[1] << 8);

	return err;
}

int csr_write_pskey_uint16(int dd, uint16_t seqnum, uint16_t pskey, uint16_t stores, uint16_t value)
{
	uint8_t array[2] = { value & 0xff, value >> 8 };

	return csr_write_pskey_complex(dd, seqnum, pskey, stores, array, 2);
}

int csr_read_pskey_uint32(int dd, uint16_t seqnum, uint16_t pskey, uint16_t stores, uint32_t *value)
{
	uint8_t array[4] = { 0x00, 0x00, 0x00, 0x00 };
	int err;

	err = csr_read_pskey_complex(dd, seqnum, pskey, stores, array, 4);

	*value = ((array[0] + (array[1] << 8)) << 16) +
						(array[2] + (array[3] << 8));

	return err;
}

int csr_write_pskey_uint32(int dd, uint16_t seqnum, uint16_t pskey, uint16_t stores, uint32_t value)
{
	uint8_t array[4] = { (value & 0xff0000) >> 16, value >> 24,
					value & 0xff, (value & 0xff00) >> 8 };

	return csr_write_pskey_complex(dd, seqnum, pskey, stores, array, 4);
}

int psr_put(uint16_t pskey, uint8_t *value, uint16_t size)
{
	struct psr_data *item;

	item = malloc(sizeof(*item));
	if (!item)
		return -ENOMEM;

	item->pskey = pskey;

	if (size > 0) {
		item->value = malloc(size);
		if (!item->value) {
			free(item);
			return -ENOMEM;
		}

		memcpy(item->value, value, size);
		item->size = size;
	} else {
		item->value = NULL;
		item->size = 0;
	}

	item->next = NULL;

	if (!head)
		head = item;
	else
		tail->next = item;

	tail = item;

	return 0;
}

int psr_get(uint16_t *pskey, uint8_t *value, uint16_t *size)
{
	struct psr_data *item = head;

	if (!head)
		return -ENOENT;

	*pskey = item->pskey;

	if (item->value) {
		if (value && item->size > 0)
			memcpy(value, item->value, item->size);
		free(item->value);
		*size = item->size;
	} else
		*size = 0;

	if (head == tail)
		tail = NULL;

	head = head->next;
	free(item);

	return 0;
}

static int parse_line(char *str)
{
	uint8_t array[256];
	uint16_t value, pskey, length = 0;
	char *off, *end;

	pskey = strtol(str + 1, NULL, 16);
	off = strstr(str, "=");
	if (!off)
		return -EIO;

	off++;

	while (1) {
		value = strtol(off, &end, 16);
		if (value == 0 && off == end)
			break;

		array[length++] = value & 0xff;
		array[length++] = value >> 8;

		if (*end == '\0')
			break;

		off = end + 1;
	}

	return psr_put(pskey, array, length);
}

int psr_read(const char *filename)
{
	struct stat st;
	char *str, *map, *off, *end;
	int fd, err = 0;

	fd = open(filename, O_RDONLY);
	if (fd < 0)
		return fd;

	if (fstat(fd, &st) < 0) {
		err = -errno;
		goto close;
	}

	map = mmap(0, st.st_size, PROT_READ, MAP_SHARED, fd, 0);
	if (!map || map == MAP_FAILED) {
		err = -errno;
		goto close;
	}

	off = map;

	while (1) {
		if (*off == '\r' || *off == '\n') {
			off++;
			continue;
		}

		end = strpbrk(off, "\r\n");
		if (!end)
			break;

		str = malloc(end - off + 1);
		if (!str)
			break;

		memset(str, 0, end - off + 1);
		strncpy(str, off, end - off);
		if (*str == '&')
			parse_line(str);

		free(str);
		off = end + 1;
	}

	munmap(map, st.st_size);

close:
	close(fd);

	return err;
}

int psr_print(void)
{
	uint8_t array[256];
	uint16_t pskey, length;
	char *str, val[7];
	int i;

	while (1) {
		if (psr_get(&pskey, array, &length) < 0)
			break;

		str = csr_pskeytoval(pskey);
		if (!strcasecmp(str, "UNKNOWN")) {
			sprintf(val, "0x%04x", pskey);
			str = NULL;
		}

		printf("// %s%s\n&%04x =", str ? "PSKEY_" : "",
						str ? str : val, pskey);
		for (i = 0; i < length / 2; i++)
			printf(" %02x%02x", array[i * 2 + 1], array[i * 2]);
		printf("\n");
	}

	return 0;
}
