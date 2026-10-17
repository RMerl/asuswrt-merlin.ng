/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) Marvell International Ltd. and its affiliates
 */

#ifndef __AXP_TRAINING_STATIC_H
#define __AXP_TRAINING_STATIC_H

/*
 * STATIC_TRAINING - Set only if static parameters for training are set and
 * required
 */

MV_DRAM_TRAINING_INIT ddr3_db_rev2_667[MV_MAX_DDR3_STATIC_SIZE] = {
	/* Read Leveling */
	/*PUP   RdSampleDly (+CL)       Phase   RL ADLL value */
	/*0     */
	{0x000016A0, 0xC002011A},
	/*1 */
	{0x000016A0, 0xC0420100},
	/*2 */
	{0x000016A0, 0xC082020A},
	/*3 */
	{0x000016A0, 0xC0C20017},
	/*4 */
	{0x000016A0, 0xC1020113},
	/*5 */
	{0x000016A0, 0xC1420107},
	/*6 */
	{0x000016A0, 0xC182011F},
	/*7 */
	{0x000016A0, 0xC1C2001C},
	/*8 */
	{0x000016A0, 0xC202010D},

	/* Write Leveling */
	/*0 */
	{0x000016A0, 0xC0004A06},
	/*1 */
	{0x000016A0, 0xC040690D},
	/*2 */
	{0x000016A0, 0xC0806A0D},
	/*3 */
	{0x000016A0, 0xC0C0A01B},
	/*4 */
	{0x000016A0, 0xC1003A01},
	/*5 */
	{0x000016A0, 0xC1408113},
	/*6 */
	{0x000016A0, 0xC1805609},
	/*7 */
	{0x000016A0, 0xC1C04504},
	/*8 */
	{0x000016A0, 0xC2009518},

	/*center DQS on read cycle */
	{0x000016A0, 0xC803000F},

	{0x00001538, 0x0000000B},	/*Read Data Sample Delays Register */
	{0x0000153C, 0x0000000F},	/*Read Data Ready Delay Register */

	/*init DRAM */
	{0x00001480, 0x00000001},
	{0x0, 0x0}
};

MV_DRAM_TRAINING_INIT ddr3_db_rev2_800[MV_MAX_DDR3_STATIC_SIZE] = {
	/* Read Leveling */
	/*PUP   RdSampleDly (+CL)       Phase   RL ADLL value */
	/*0     */
	{0x000016A0, 0xC0020301},
	/*1 */
	{0x000016A0, 0xC0420202},
	/*2 */
	{0x000016A0, 0xC0820314},
	/*3 */
	{0x000016A0, 0xC0C20117},
	/*4 */
	{0x000016A0, 0xC1020219},
	/*5 */
	{0x000016A0, 0xC142020B},
	/*6 */
	{0x000016A0, 0xC182030A},
	/*7 */
	{0x000016A0, 0xC1C2011D},
	/*8 */
	{0x000016A0, 0xC2020212},

	/* Write Leveling */
	/*0 */
	{0x000016A0, 0xC0007A12},
	/*1 */
	{0x000016A0, 0xC0408D16},
	/*2 */
	{0x000016A0, 0xC0809E1B},
	/*3 */
	{0x000016A0, 0xC0C0AC1F},
	/*4 */
	{0x000016A0, 0xC1005E0A},
	/*5 */
	{0x000016A0, 0xC140A91D},
	/*6 */
	{0x000016A0, 0xC1808E17},
	/*7 */
	{0x000016A0, 0xC1C05509},
	/*8 */
	{0x000016A0, 0xC2003A01},

	/* PBS Leveling */
	/*0 */
	{0x000016A0, 0xC0007A12},
	/*1 */
	{0x000016A0, 0xC0408D16},
	/*2 */
	{0x000016A0, 0xC0809E1B},
	/*3 */
	{0x000016A0, 0xC0C0AC1F},
	/*4 */
	{0x000016A0, 0xC1005E0A},
	/*5 */
	{0x000016A0, 0xC140A91D},
	/*6 */
	{0x000016A0, 0xC1808E17},
	/*7 */
	{0x000016A0, 0xC1C05509},
	/*8 */
	{0x000016A0, 0xC2003A01},

	/*center DQS on read cycle */
	{0x000016A0, 0xC803000B},

	{0x00001538, 0x0000000D},	/*Read Data Sample Delays Register */
	{0x0000153C, 0x00000011},	/*Read Data Ready Delay Register */

	/*init DRAM */
	{0x00001480, 0x00000001},
	{0x0, 0x0}
};

MV_DRAM_TRAINING_INIT ddr3_db_400[MV_MAX_DDR3_STATIC_SIZE] = {
	/* Read Leveling */
	/*PUP   RdSampleDly (+CL)       Phase   RL ADLL value */
	/*0             2               4               15 */
	{0x000016A0, 0xC002010C},
	/*1             2               4               2 */
	{0x000016A0, 0xC042001C},
	/*2             2               4               27 */
	{0x000016A0, 0xC0820115},
	/*3             2               4               0 */
	{0x000016A0, 0xC0C20019},
	/*4             2               4               13 */
	{0x000016A0, 0xC1020108},
	/*5             2               4               5 */
	{0x000016A0, 0xC1420100},
	/*6             2               4               19 */
	{0x000016A0, 0xC1820111},
	/*7             2               4               0 */
	{0x000016A0, 0xC1C2001B},
	/*8             2               4               10 */
	/*{0x000016A0, 0xC2020117}, */
	{0x000016A0, 0xC202010C},

	/* Write Leveling */
	/*0 */
	{0x000016A0, 0xC0005508},
	/*1 */
	{0x000016A0, 0xC0409819},
	/*2 */
	{0x000016A0, 0xC080650C},
	/*3 */
	{0x000016A0, 0xC0C0700F},
	/*4 */
	{0x000016A0, 0xC1004103},
	/*5 */
	{0x000016A0, 0xC140A81D},
	/*6 */
	{0x000016A0, 0xC180650C},
	/*7 */
	{0x000016A0, 0xC1C08013},
	/*8 */
	{0x000016A0, 0xC2005508},

	/*center DQS on read cycle */
	{0x000016A0, 0xC803000F},

	{0x00001538, 0x00000008},	/*Read Data Sample Delays Register */
	{0x0000153C, 0x0000000A},	/*Read Data Ready Delay Register */

	/*init DRAM */
	{0x00001480, 0x00000001},
	{0x0, 0x0}
};

MV_DRAM_TRAINING_INIT ddr3_db_533[MV_MAX_DDR3_STATIC_SIZE] = {
	/* Read Leveling */
	/*PUP   RdSampleDly (+CL)       Phase   RL ADLL value */
	/*0             2               4               15 */
	{0x000016A0, 0xC002040C},
	/*1             2               4               2 */
	{0x000016A0, 0xC0420117},
	/*2             2               4               27 */
	{0x000016A0, 0xC082041B},
	/*3             2               4               0 */
	{0x000016A0, 0xC0C20117},
	/*4             2               4               13 */
	{0x000016A0, 0xC102040A},
	/*5             2               4               5 */
	{0x000016A0, 0xC1420117},
	/*6             2               4               19 */
	{0x000016A0, 0xC1820419},
	/*7             2               4               0 */
	{0x000016A0, 0xC1C20117},
	/*8             2               4               10 */
	{0x000016A0, 0xC2020117},

	/* Write Leveling */
	/*0 */
	{0x000016A0, 0xC0008113},
	/*1 */
	{0x000016A0, 0xC0404504},
	/*2 */
	{0x000016A0, 0xC0808514},
	/*3 */
	{0x000016A0, 0xC0C09418},
	/*4 */
	{0x000016A0, 0xC1006D0E},
	/*5 */
	{0x000016A0, 0xC1405508},
	/*6 */
	{0x000016A0, 0xC1807D12},
	/*7 */
	{0x000016A0, 0xC1C0b01F},
	/*8 */
	{0x000016A0, 0xC2005D0A},

	/*center DQS on read cycle */
	{0x000016A0, 0xC803000F},

	{0x00001538, 0x00000008},	/*Read Data Sample Delays Register */
	{0x0000153C, 0x0000000A},	/*Read Data Ready Delay Register */

	/*init DRAM */
	{0x00001480, 0x00000001},
	{0x0, 0x0}
};

MV_DRAM_TRAINING_INIT ddr3_db_600[MV_MAX_DDR3_STATIC_SIZE] = {
	/* Read Leveling */
	/*PUP   RdSampleDly (+CL)       Phase   RL ADLL value */
	/*0             2               3               1 */
	{0x000016A0, 0xC0020104},
	/*1             2               2               6 */
	{0x000016A0, 0xC0420010},
	/*2             2               3               16 */
	{0x000016A0, 0xC0820112},
	/*3             2               1               26 */
	{0x000016A0, 0xC0C20009},
	/*4             2               2               29 */
	{0x000016A0, 0xC102001F},
	/*5             2               2               13 */
	{0x000016A0, 0xC1420014},
	/*6             2               3               6 */
	{0x000016A0, 0xC1820109},
	/*7             2               1               31 */
	{0x000016A0, 0xC1C2000C},
	/*8             2               2               22 */
	{0x000016A0, 0xC2020112},

	/* Write Leveling */
	/*0 */
	{0x000016A0, 0xC0009919},
	/*1 */
	{0x000016A0, 0xC0405508},
	/*2 */
	{0x000016A0, 0xC0809919},
	/*3 */
	{0x000016A0, 0xC0C09C1A},
	/*4 */
	{0x000016A0, 0xC1008113},
	/*5 */
	{0x000016A0, 0xC140650C},
	/*6 */
	{0x000016A0, 0xC1809518},
	/*7 */
	{0x000016A0, 0xC1C04103},
	/*8 */
	{0x000016A0, 0xC2006D0E},

	/*center DQS on read cycle */
	{0x000016A0, 0xC803000F},

	{0x00001538, 0x0000000B},	/*Read Data Sample Delays Register */
	{0x0000153C, 0x0000000F},	/*Read Data Ready Delay Register */
	/*init DRAM */
	{0x00001480, 0x00000001},
	{0x0, 0x0}
};

MV_DRAM_TRAINING_INIT ddr3_db_667[MV_MAX_DDR3_STATIC_SIZE] = {

	/* Read Leveling */
	/*PUP   RdSampleDly (+CL)       Phase   RL ADLL value */
	/*0             2               3               1 */
	{0x000016A0, 0xC0020103},
	/*1            2               2               6 */
	{0x000016A0, 0xC0420012},
	/*2            2               3               16 */
	{0x000016A0, 0xC0820113},
	/*3            2               1               26 */
	{0x000016A0, 0xC0C20012},
	/*4            2               2               29 */
	{0x000016A0, 0xC1020100},
	/*5            2               2               13 */
	{0x000016A0, 0xC1420016},
	/*6            2               3               6 */
	{0x000016A0, 0xC1820109},
	/*7            2               1               31 */
	{0x000016A0, 0xC1C20010},
	/*8            2               2               22 */
	{0x000016A0, 0xC2020112},

	/* Write Leveling */
	/*0 */
	{0x000016A0, 0xC000b11F},
	/*1 */
	{0x000016A0, 0xC040690D},
	/*2 */
	{0x000016A0, 0xC0803600},
	/*3 */
	{0x000016A0, 0xC0C0a81D},
	/*4 */
	{0x000016A0, 0xC1009919},
	/*5 */
	{0x000016A0, 0xC1407911},
	/*6 */
	{0x000016A0, 0xC180ad1e},
	/*7 */
	{0x000016A0, 0xC1C04d06},
	/*8 */
	{0x000016A0, 0xC2008514},

	/*center DQS on read cycle */
	{0x000016A0, 0xC803000F},

	{0x00001538, 0x0000000B},	/*Read Data Sample Delays Register */
	{0x0000153C, 0x0000000F},	/*Read Data Ready Delay Register */

	/*init DRAM */
	{0x00001480, 0x00000001},
	{0x0, 0x0}
};

MV_DRAM_TRAINING_INIT ddr3_db_800[MV_MAX_DDR3_STATIC_SIZE] = {

	/* Read Leveling */
	/*PUP   RdSampleDly (+CL)       Phase   RL ADLL value */
	/*0             2               3               1 */
	{0x000016A0, 0xC0020213},
	/*1            2               2               6 */
	{0x000016A0, 0xC0420108},
	/*2            2               3               16 */
	{0x000016A0, 0xC0820210},
	/*3            2               1               26 */
	{0x000016A0, 0xC0C20108},
	/*4            2               2               29 */
	{0x000016A0, 0xC102011A},
	/*5            2               2               13 */
	{0x000016A0, 0xC1420300},
	/*6            2               3               6 */
	{0x000016A0, 0xC1820204},
	/*7            2               1               31 */
	{0x000016A0, 0xC1C20106},
	/*8            2               2               22 */
	{0x000016A0, 0xC2020112},

	/* Write Leveling */
	/*0 */
	{0x000016A0, 0xC000620B},
	/*1 */
	{0x000016A0, 0xC0408D16},
	/*2 */
	{0x000016A0, 0xC0806A0D},
	/*3 */
	{0x000016A0, 0xC0C03D02},
	/*4 */
	{0x000016A0, 0xC1004a05},
	/*5 */
	{0x000016A0, 0xC140A11B},
	/*6 */
	{0x000016A0, 0xC1805E0A},
	/*7 */
	{0x000016A0, 0xC1C06D0E},
	/*8 */
	{0x000016A0, 0xC200AD1E},

	/*center DQS on read cycle */
	{0x000016A0, 0xC803000F},

	{0x00001538, 0x0000000C},	/*Read Data Sample Delays Register */
	{0x0000153C, 0x0000000E},	/*Read Data Ready Delay Register */

	/*init DRAM */
	{0x00001480, 0x00000001},
	{0x0, 0x0}
};

MV_DRAM_TRAINING_INIT ddr3_rd_667_0[MV_MAX_DDR3_STATIC_SIZE] = {
	/* Read Leveling */
	/*PUP   RdSampleDly (+CL)       Phase   RL ADLL value */
	/*0 */
	{0x000016A0, 0xC002010E},
	/*1 */
	{0x000016A0, 0xC042001E},
	/*2 */
	{0x000016A0, 0xC0820118},
	/*3 */
	{0x000016A0, 0xC0C2001E},
	/*4 */
	{0x000016A0, 0xC102010C},
	/*5 */
	{0x000016A0, 0xC1420102},
	/*6 */
	{0x000016A0, 0xC1820111},
	/*7 */
	{0x000016A0, 0xC1C2001C},
	/*8 */
	{0x000016A0, 0xC2020109},

	/* Write Leveling */
	/*0 */
	{0x000016A0, 0xC0003600},
	/*1 */
	{0x000016A0, 0xC040690D},
	/*2 */
	{0x000016A0, 0xC0805207},
	/*3 */
	{0x000016A0, 0xC0C0A81D},
	/*4 */
	{0x000016A0, 0xC1009919},
	/*5 */
	{0x000016A0, 0xC1407911},
	/*6 */
	{0x000016A0, 0xC1803E02},
	/*7 */
	{0x000016A0, 0xC1C05107},
	/*8 */
	{0x000016A0, 0xC2008113},

	/*center DQS on read cycle */
	{0x000016A0, 0xC803000F},

	{0x00001538, 0x0000000B},	/*Read Data Sample Delays Register */
	{0x0000153C, 0x0000000F},	/*Read Data Ready Delay Register */

	/*init DRAM */
	{0x00001480, 0x00000001},
	{0x0, 0x0}
};

MV_DRAM_TRAINING_INIT ddr3_rd_667_1[MV_MAX_DDR3_STATIC_SIZE] = {
	/* Read Leveling */
	/*PUP   RdSampleDly (+CL)       Phase   RL ADLL value */
	/*0 */
	{0x000016A0, 0xC0020106},
	/*1 */
	{0x000016A0, 0xC0420016},
	/*2 */
	{0x000016A0, 0xC0820117},
	/*3 */
	{0x000016A0, 0xC0C2000F},
	/*4 */
	{0x000016A0, 0xC1020105},
	/*5 */
	{0x000016A0, 0xC142001B},
	/*6 */
	{0x000016A0, 0xC182010C},
	/*7 */
	{0x000016A0, 0xC1C20011},
	/*8 */
	{0x000016A0, 0xC2020101},

	/* Write Leveling */
	/*0 */
	{0x000016A0, 0xC0003600},
	/*1 */
	{0x000016A0, 0xC0406D0E},
	/*2 */
	{0x000016A0, 0xC0803600},
	/*3 */
	{0x000016A0, 0xC0C04504},
	/*4 */
	{0x000016A0, 0xC1009919},
	/*5 */
	{0x000016A0, 0xC1407911},
	/*6 */
	{0x000016A0, 0xC1803600},
	/*7 */
	{0x000016A0, 0xC1C0610B},
	/*8 */
	{0x000016A0, 0xC2008113},

	/*center DQS on read cycle */
	{0x000016A0, 0xC803000F},

	{0x00001538, 0x0000000B},	/*Read Data Sample Delays Register */
	{0x0000153C, 0x0000000F},	/*Read Data Ready Delay Register */

	/*init DRAM */
	{0x00001480, 0x00000001},
	{0x0, 0x0}
};

MV_DRAM_TRAINING_INIT ddr3_rd_667_2[MV_MAX_DDR3_STATIC_SIZE] = {
	/* Read Leveling */
	/*PUP   RdSampleDly (+CL)       Phase   RL ADLL value */
	/*0 */
	{0x000016A0, 0xC002010C},
	/*1 */
	{0x000016A0, 0xC042001B},
	/*2 */
	{0x000016A0, 0xC082011D},
	/*3 */
	{0x000016A0, 0xC0C20015},
	/*4 */
	{0x000016A0, 0xC102010B},
	/*5 */
	{0x000016A0, 0xC1420101},
	/*6 */
	{0x000016A0, 0xC1820113},
	/*7 */
	{0x000016A0, 0xC1C20017},
	/*8 */
	{0x000016A0, 0xC2020107},

	/* Write Leveling */
	/*0 */
	{0x000016A0, 0xC0003600},
	/*1 */
	{0x000016A0, 0xC0406D0E},
	/*2 */
	{0x000016A0, 0xC0803600},
	/*3 */
	{0x000016A0, 0xC0C04504},
	/*4 */
	{0x000016A0, 0xC1009919},
	/*5 */
	{0x000016A0, 0xC1407911},
	/*6 */
	{0x000016A0, 0xC180B11F},
	/*7 */
	{0x000016A0, 0xC1C0610B},
	/*8 */
	{0x000016A0, 0xC2008113},

	/*center DQS on read cycle */
	{0x000016A0, 0xC803000F},

	{0x00001538, 0x0000000B},	/*Read Data Sample Delays Register */
	{0x0000153C, 0x0000000F},	/*Read Data Ready Delay Register */

	/*init DRAM */
	{0x00001480, 0x00000001},
	{0x0, 0x0}
};

MV_DRAM_TRAINING_INIT ddr3_db_667_M[MV_MAX_DDR3_STATIC_SIZE] = {
	/* Read Leveling */
	/*PUP   RdSampleDly (+CL)       Phase   RL ADLL value */
	/* CS 0 */
	/*0             2               3               1 */
	{0x000016A0, 0xC0020103},
	/*1            2               2               6 */
	{0x000016A0, 0xC0420012},
	/*2            2               3               16 */
	{0x000016A0, 0xC0820113},
	/*3            2               1               26 */
	{0x000016A0, 0xC0C20012},
	/*4            2               2               29 */
	{0x000016A0, 0xC1020100},
	/*5            2               2               13 */
	{0x000016A0, 0xC1420016},
	/*6            2               3               6 */
	{0x000016A0, 0xC1820109},
	/*7            2               1               31 */
	{0x000016A0, 0xC1C20010},
	/*8            2               2               22 */
	{0x000016A0, 0xC2020112},

	/* Write Leveling */
	/*0 */
	{0x000016A0, 0xC000b11F},
	/*1 */
	{0x000016A0, 0xC040690D},
	/*2 */
	{0x000016A0, 0xC0803600},
	/*3 */
	{0x000016A0, 0xC0C0a81D},
	/*4 */
	{0x000016A0, 0xC1009919},
	/*5 */
	{0x000016A0, 0xC1407911},
	/*6 */
	{0x000016A0, 0xC180ad1e},
	/*7 */
	{0x000016A0, 0xC1C04d06},
	/*8 */
	{0x000016A0, 0xC2008514},

	/*center DQS on read cycle */
	{0x000016A0, 0xC803000F},

	/* CS 1 */

	{0x000016A0, 0xC0060103},
	/*1            2               2               6 */
	{0x000016A0, 0xC0460012},
	/*2            2               3               16 */
	{0x000016A0, 0xC0860113},
	/*3            2               1               26 */
	{0x000016A0, 0xC0C60012},
	/*4            2               2               29 */
	{0x000016A0, 0xC1060100},
	/*5            2               2               13 */
	{0x000016A0, 0xC1460016},
	/*6            2               3               6 */
	{0x000016A0, 0xC1860109},
	/*7            2               1               31 */
	{0x000016A0, 0xC1C60010},
	/*8            2               2               22 */
	{0x000016A0, 0xC2060112},

	/* Write Leveling */
	/*0 */
	{0x000016A0, 0xC004b11F},
	/*1 */
	{0x000016A0, 0xC044690D},
	/*2 */
	{0x000016A0, 0xC0843600},
	/*3 */
	{0x000016A0, 0xC0C4a81D},
	/*4 */
	{0x000016A0, 0xC1049919},
	/*5 */
	{0x000016A0, 0xC1447911},
	/*6 */
	{0x000016A0, 0xC184ad1e},
	/*7 */
	{0x000016A0, 0xC1C44d06},
	/*8 */
	{0x000016A0, 0xC2048514},

	/*center DQS on read cycle */
	{0x000016A0, 0xC807000F},

	/* Both CS */

	{0x00001538, 0x00000B0B},	/*Read Data Sample Delays Register */
	{0x0000153C, 0x00000F0F},	/*Read Data Ready Delay Register */

	/*init DRAM */
	{0x00001480, 0x00000001},
	{0x0, 0x0}
};

MV_DRAM_TRAINING_INIT ddr3_rd_667_3[MV_MAX_DDR3_STATIC_SIZE] = {
	/* Read Leveling */
	/*PUP   RdSampleDly (+CL)       Phase   RL ADLL value */
	/*0 */
	{0x000016A0, 0xC0020118},
	/*1 */
	{0x000016A0, 0xC0420108},
	/*2 */
	{0x000016A0, 0xC0820202},
	/*3 */
	{0x000016A0, 0xC0C20108},
	/*4 */
	{0x000016A0, 0xC1020117},
	/*5 */
	{0x000016A0, 0xC142010C},
	/*6 */
	{0x000016A0, 0xC182011B},
	/*7 */
	{0x000016A0, 0xC1C20107},
	/*8 */
	{0x000016A0, 0xC2020113},

	/* Write Leveling */
	/*0 */
	{0x000016A0, 0xC0003600},
	/*1 */
	{0x000016A0, 0xC0406D0E},
	/*2 */
	{0x000016A0, 0xC0805207},
	/*3 */
	{0x000016A0, 0xC0C0A81D},
	/*4 */
	{0x000016A0, 0xC1009919},
	/*5 */
	{0x000016A0, 0xC1407911},
	/*6 */
	{0x000016A0, 0xC1803E02},
	/*7 */
	{0x000016A0, 0xC1C04D06},
	/*8 */
	{0x000016A0, 0xC2008113},

	/*center DQS on read cycle */
	{0x000016A0, 0xC803000F},

	{0x00001538, 0x0000000B},	/*Read Data Sample Delays Register */
	{0x0000153C, 0x0000000F},	/*Read Data Ready Delay Register */

	/*init DRAM */
	{0x00001480, 0x00000001},
	{0x0, 0x0}
};

MV_DRAM_TRAINING_INIT ddr3_pcac_600[MV_MAX_DDR3_STATIC_SIZE] = {
	/* Read Leveling */
	/*PUP   RdSampleDly (+CL)       Phase   RL ADLL value */
	/*0 */
	{0x000016A0, 0xC0020404},
	/* 1           2               2               6 */
	{0x000016A0, 0xC042031E},
	/* 2           2               3               16 */
	{0x000016A0, 0xC0820411},
	/* 3           2               1               26 */
	{0x000016A0, 0xC0C20400},
	/* 4           2               2               29 */
	{0x000016A0, 0xC1020404},
	/* 5           2               2               13 */
	{0x000016A0, 0xC142031D},
	/* 6           2               3               6 */
	{0x000016A0, 0xC182040C},
	/* 7           2               1               31 */
	{0x000016A0, 0xC1C2031B},
	/* 8           2               2               22 */
	{0x000016A0, 0xC2020112},

	/*  Write Leveling */
	/* 0 */
	{0x000016A0, 0xC0004905},
	/* 1 */
	{0x000016A0, 0xC040A81D},
	/* 2 */
	{0x000016A0, 0xC0804504},
	/* 3 */
	{0x000016A0, 0xC0C08013},
	/* 4 */
	{0x000016A0, 0xC1004504},
	/* 5 */
	{0x000016A0, 0xC140A81D},
	/* 6 */
	{0x000016A0, 0xC1805909},
	/* 7 */
	{0x000016A0, 0xC1C09418},
	/* 8 */
	{0x000016A0, 0xC2006D0E},

	/*center DQS on read cycle */
	{0x000016A0, 0xC803000F},
	{0x00001538, 0x00000009},	/*Read Data Sample Delays Register */
	{0x0000153C, 0x0000000D},	/*Read Data Ready Delay Register */
	/* init DRAM */
	{0x00001480, 0x00000001},
	{0x0, 0x0}
};

#endif /* __AXP_TRAINING_STATIC_H */
