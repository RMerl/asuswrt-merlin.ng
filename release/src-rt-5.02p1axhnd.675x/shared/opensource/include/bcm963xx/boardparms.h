/*
    Copyright 2000-2012 Broadcom Corporation

    <:label-BRCM:2011:DUAL/GPL:standard
    
    Unless you and Broadcom execute a separate written software license
    agreement governing use of this software, this software is licensed
    to you under the terms of the GNU General Public License version 2
    (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
    with the following added to such license:
    
       As a special exception, the copyright holders of this software give
       you permission to link this software with independent modules, and
       to copy and distribute the resulting executable under terms of your
       choice, provided that you also meet, for each linked independent
       module, the terms and conditions of the license of that module.
       An independent module is a module which is not derived from this
       software.  The special exception does not apply to any modifications
       of the software.
    
    Not withstanding the above, under no circumstances may you combine
    this software in any way with any other Broadcom software provided
    under a license other than the GPL, without Broadcom's express prior
    written consent.
    
    :>
*/                       

/**************************************************************************
 * File Name  : boardparms.h
 *
 * Description: This file contains definitions and function prototypes for
 *              the BCM63xx board parameter access functions.
 *
 * Updates    : 07/14/2003  Created.
 ***************************************************************************/

#if !defined(_BOARDPARMS_H)
#define _BOARDPARMS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "bp_defs.h"

/* Return codes. */
#define BP_SUCCESS                              0
#define BP_BOARD_ID_NOT_FOUND                   1
#define BP_VALUE_NOT_DEFINED                    2
#define BP_BOARD_ID_NOT_SET                     3
#define BP_MAX_CHANNELS_EXCEEDED                4
#define BP_NO_ZSI_ON_BOARD_ERR                  5
#define BP_MAX_ITEM_EXCEEDED                    6
#define BP_SUCCESS_LAST                         7
#define BP_INTERRUPT_ALREADY_DEFINED            8
#define BP_GPIO_ALREADY_USED_AS_INTERRUPT       9
#define BP_INVALID_INTERRUPT                    10
#define BP_INVALID_GPIO                         11
#define BP_INVALID_INTF                         12

/* Values for EthernetMacInfo PhyType. */
#define BP_ENET_NO_PHY                          0
#define BP_ENET_INTERNAL_PHY                    1
#define BP_ENET_EXTERNAL_SWITCH                 2
#define BP_ENET_SWITCH_VIA_INTERNAL_PHY         3      /* it is for cpu internal phy connects to port 4 of 5325e */

/* Values for EthernetMacInfo Configuration type. */
#define BP_ENET_CONFIG_MDIO                     0       /* Internal PHY, External PHY, Switch+(no GPIO, no SPI, no MDIO Pseudo phy */
#define BP_ENET_CONFIG_MDIO_PSEUDO_PHY          1
#define BP_ENET_CONFIG_SPI_SSB_0                2
#define BP_ENET_CONFIG_SPI_SSB_1                3
#define BP_ENET_CONFIG_SPI_SSB_2                4
#define BP_ENET_CONFIG_SPI_SSB_3                5
#define BP_ENET_CONFIG_MMAP                     6
#define BP_ENET_CONFIG_GPIO_MDIO                7       /* use GPIO to simulate MDC/MDIO */
#define BP_ENET_CONFIG_HS_SPI_SSB_0             8
#define BP_ENET_CONFIG_HS_SPI_SSB_1             9
#define BP_ENET_CONFIG_HS_SPI_SSB_2             10
#define BP_ENET_CONFIG_HS_SPI_SSB_3             11
#define BP_ENET_CONFIG_HS_SPI_SSB_4             12
#define BP_ENET_CONFIG_HS_SPI_SSB_5             13
#define BP_ENET_CONFIG_HS_SPI_SSB_6             14
#define BP_ENET_CONFIG_HS_SPI_SSB_7             15


/* Values for VoIPDSPInfo DSPType. */
#define BP_VOIP_NO_DSP                          0
#define BP_VOIP_DSP                             1

/* Values for GPIO pin assignments (AH = Active High, AL = Active Low). */
#define BP_GPIO_NUM_MASK                        0x00FF
#define BP_GPIO_NUM_MAX                         256
#define BP_ACTIVE_MASK                          0x8000
#define BP_ACTIVE_HIGH                          0x0000
#define BP_ACTIVE_LOW                           0x8000
#define BP_GPIO_SERIAL                          0x4000
#define BP_NONGPIO_PIN                          0x2000
#define BP_LED_PIN                              0x1000
#define BP_LED_USE_GPIO                         0x0800

#define BP_GPIO_NONE                            (BP_GPIO_NUM_MASK)
#define BP_GPIO_0_AH                            (0)
#define BP_GPIO_0_AL                            (0  | BP_ACTIVE_LOW)
#define BP_PIN_DSL_CTRL_4                       (0  | BP_NONGPIO_PIN)
#define BP_PIN_AON_POWER                        (0  | BP_NONGPIO_PIN)
#define BP_GPIO_1_AH                            (1)
#define BP_GPIO_1_AL                            (1  | BP_ACTIVE_LOW)
#define BP_PIN_DSL_CTRL_5                       (1  | BP_NONGPIO_PIN)
#define BP_GPIO_2_AH                            (2)
#define BP_GPIO_2_AL                            (2  | BP_ACTIVE_LOW)
#define BP_GPIO_3_AH                            (3)
#define BP_GPIO_3_AL                            (3  | BP_ACTIVE_LOW)
#define BP_GPIO_4_AH                            (4)
#define BP_GPIO_4_AL                            (4  | BP_ACTIVE_LOW)
#define BP_PIN_TSYNC_8KHZ_4                     (4  | BP_NONGPIO_PIN)
#define BP_GPIO_5_AH                            (5)
#define BP_GPIO_5_AL                            (5  | BP_ACTIVE_LOW)
#define BP_GPIO_6_AH                            (6)
#define BP_GPIO_6_AL                            (6  | BP_ACTIVE_LOW)
#define BP_PIN_TSYNC_1PPS_6                     (6  | BP_NONGPIO_PIN)
#define BP_GPIO_7_AH                            (7)
#define BP_GPIO_7_AL                            (7  | BP_ACTIVE_LOW)
#define BP_GPIO_8_AH                            (8)
#define BP_GPIO_8_AL                            (8  | BP_ACTIVE_LOW)
#define BP_GPIO_9_AH                            (9)
#define BP_GPIO_9_AL                            (9  | BP_ACTIVE_LOW)
#define BP_GPIO_10_AH                           (10)
#define BP_GPIO_10_AL                           (10 | BP_ACTIVE_LOW)
#define BP_GPIO_11_AH                           (11)
#define BP_GPIO_11_AL                           (11 | BP_ACTIVE_LOW)
#define BP_PIN_TSYNC_1025MHZ_11                 (11 | BP_NONGPIO_PIN)
#define BP_GPIO_12_AH                           (12)
#define BP_GPIO_12_AL                           (12 | BP_ACTIVE_LOW)
#define BP_GPIO_13_AH                           (13)
#define BP_GPIO_13_AL                           (13 | BP_ACTIVE_LOW)
#define BP_GPIO_14_AH                           (14)
#define BP_GPIO_14_AL                           (14 | BP_ACTIVE_LOW)
#define BP_GPIO_15_AH                           (15)
#define BP_GPIO_15_AL                           (15 | BP_ACTIVE_LOW)
#define BP_GPIO_16_AH                           (16)
#define BP_GPIO_16_AL                           (16 | BP_ACTIVE_LOW)
#define BP_GPIO_17_AH                           (17)
#define BP_GPIO_17_AL                           (17 | BP_ACTIVE_LOW)
#define BP_GPIO_18_AH                           (18)
#define BP_GPIO_18_AL                           (18 | BP_ACTIVE_LOW)
#define BP_GPIO_19_AH                           (19)
#define BP_GPIO_19_AL                           (19 | BP_ACTIVE_LOW)
#define BP_GPIO_20_AH                           (20)
#define BP_GPIO_20_AL                           (20 | BP_ACTIVE_LOW)
#define BP_GPIO_21_AH                           (21)
#define BP_GPIO_21_AL                           (21 | BP_ACTIVE_LOW)
#define BP_GPIO_22_AH                           (22)
#define BP_GPIO_22_AL                           (22 | BP_ACTIVE_LOW)
#define BP_GPIO_23_AH                           (23)
#define BP_GPIO_23_AL                           (23 | BP_ACTIVE_LOW)
#define BP_GPIO_24_AH                           (24)
#define BP_GPIO_24_AL                           (24 | BP_ACTIVE_LOW)
#define BP_GPIO_25_AH                           (25)
#define BP_GPIO_25_AL                           (25 | BP_ACTIVE_LOW)
#define BP_GPIO_26_AH                           (26)
#define BP_GPIO_26_AL                           (26 | BP_ACTIVE_LOW)
#define BP_GPIO_27_AH                           (27)
#define BP_GPIO_27_AL                           (27 | BP_ACTIVE_LOW)
#define BP_GPIO_28_AH                           (28)
#define BP_GPIO_28_AL                           (28 | BP_ACTIVE_LOW)
#define BP_GPIO_29_AH                           (29)
#define BP_GPIO_29_AL                           (29 | BP_ACTIVE_LOW)
#define BP_GPIO_30_AH                           (30)
#define BP_GPIO_30_AL                           (30 | BP_ACTIVE_LOW)
#define BP_GPIO_31_AH                           (31)
#define BP_GPIO_31_AL                           (31 | BP_ACTIVE_LOW)
#define BP_GPIO_32_AH                           (32)
#define BP_GPIO_32_AL                           (32 | BP_ACTIVE_LOW)
#define BP_GPIO_33_AH                           (33)
#define BP_GPIO_33_AL                           (33 | BP_ACTIVE_LOW)
#define BP_GPIO_34_AH                           (34)
#define BP_GPIO_34_AL                           (34 | BP_ACTIVE_LOW)
#define BP_GPIO_35_AH                           (35)
#define BP_GPIO_35_AL                           (35 | BP_ACTIVE_LOW)
#define BP_GPIO_36_AH                           (36)
#define BP_GPIO_36_AL                           (36 | BP_ACTIVE_LOW)
#define BP_GPIO_37_AH                           (37)
#define BP_GPIO_37_AL                           (37 | BP_ACTIVE_LOW)
#define BP_GPIO_38_AH                           (38)
#define BP_GPIO_38_AL                           (38 | BP_ACTIVE_LOW)
#define BP_GPIO_39_AH                           (39)
#define BP_GPIO_39_AL                           (39 | BP_ACTIVE_LOW)
#define BP_GPIO_40_AH                           (40)
#define BP_GPIO_40_AL                           (40 | BP_ACTIVE_LOW)
#define BP_GPIO_41_AH                           (41)
#define BP_GPIO_41_AL                           (41 | BP_ACTIVE_LOW)
#define BP_GPIO_42_AH                           (42)
#define BP_GPIO_42_AL                           (42 | BP_ACTIVE_LOW)
#define BP_GPIO_43_AH                           (43)
#define BP_GPIO_43_AL                           (43 | BP_ACTIVE_LOW)
#define BP_GPIO_44_AH                           (44)
#define BP_GPIO_44_AL                           (44 | BP_ACTIVE_LOW)
#define BP_GPIO_45_AH                           (45)
#define BP_GPIO_45_AL                           (45 | BP_ACTIVE_LOW)
#define BP_GPIO_46_AH                           (46)
#define BP_GPIO_46_AL                           (46 | BP_ACTIVE_LOW)
#define BP_GPIO_47_AH                           (47)
#define BP_GPIO_47_AL                           (47 | BP_ACTIVE_LOW)
#define BP_GPIO_48_AH                           (48)
#define BP_GPIO_48_AL                           (48 | BP_ACTIVE_LOW)
#define BP_GPIO_49_AH                           (49)
#define BP_GPIO_49_AL                           (49 | BP_ACTIVE_LOW)
#define BP_GPIO_50_AH                           (50)
#define BP_GPIO_50_AL                           (50 | BP_ACTIVE_LOW)
#define BP_GPIO_51_AH                           (51)
#define BP_GPIO_51_AL                           (51 | BP_ACTIVE_LOW)
#define BP_GPIO_52_AH                           (52)
#define BP_GPIO_52_AL                           (52 | BP_ACTIVE_LOW)
#define BP_PIN_TSYNC_1PPS_52                    (52 | BP_NONGPIO_PIN)
#define BP_GPIO_53_AH                           (53)
#define BP_GPIO_53_AL                           (53 | BP_ACTIVE_LOW)
#define BP_GPIO_54_AH                           (54)
#define BP_GPIO_54_AL                           (54 | BP_ACTIVE_LOW)
#define BP_GPIO_55_AH                           (55)
#define BP_GPIO_55_AL                           (55 | BP_ACTIVE_LOW)
#define BP_GPIO_56_AH                           (56)
#define BP_GPIO_56_AL                           (56 | BP_ACTIVE_LOW)
#define BP_GPIO_57_AH                           (57)
#define BP_GPIO_57_AL                           (57 | BP_ACTIVE_LOW)
#define BP_GPIO_58_AH                           (58)
#define BP_GPIO_58_AL                           (58 | BP_ACTIVE_LOW)
#define BP_GPIO_59_AH                           (59)
#define BP_GPIO_59_AL                           (59 | BP_ACTIVE_LOW)
#define BP_GPIO_60_AH                           (60)
#define BP_GPIO_60_AL                           (60 | BP_ACTIVE_LOW)
#define BP_GPIO_61_AH                           (61)
#define BP_GPIO_61_AL                           (61 | BP_ACTIVE_LOW)
#define BP_GPIO_62_AH                           (62)
#define BP_GPIO_62_AL                           (62 | BP_ACTIVE_LOW)
#define BP_GPIO_63_AH                           (63)
#define BP_GPIO_63_AL                           (63 | BP_ACTIVE_LOW)
#define BP_GPIO_64_AH                           (64)
#define BP_GPIO_64_AL                           (64 | BP_ACTIVE_LOW)
#define BP_GPIO_65_AH                           (65)
#define BP_GPIO_65_AL                           (65 | BP_ACTIVE_LOW)
#define BP_GPIO_66_AH                           (66)
#define BP_GPIO_66_AL                           (66 | BP_ACTIVE_LOW)
#define BP_GPIO_67_AH                           (67)
#define BP_GPIO_67_AL                           (67 | BP_ACTIVE_LOW)
#define BP_GPIO_68_AH                           (68)
#define BP_GPIO_68_AL                           (68 | BP_ACTIVE_LOW)
#define BP_GPIO_69_AH                           (69)
#define BP_GPIO_69_AL                           (69 | BP_ACTIVE_LOW)
#define BP_GPIO_70_AH                           (70)
#define BP_GPIO_70_AL                           (70 | BP_ACTIVE_LOW)
#define BP_GPIO_71_AH                           (71)
#define BP_GPIO_71_AL                           (71 | BP_ACTIVE_LOW)
#define BP_GPIO_72_AH                           (72)
#define BP_GPIO_72_AL                           (72 | BP_ACTIVE_LOW)
#define BP_GPIO_73_AH                           (73)
#define BP_GPIO_73_AL                           (73 | BP_ACTIVE_LOW)
#define BP_GPIO_74_AH                           (74)
#define BP_GPIO_74_AL                           (74 | BP_ACTIVE_LOW)
#define BP_GPIO_75_AH                           (75)
#define BP_GPIO_75_AL                           (75 | BP_ACTIVE_LOW)
#define BP_GPIO_76_AH                           (76)
#define BP_GPIO_76_AL                           (76 | BP_ACTIVE_LOW)
#define BP_GPIO_77_AH                           (77)
#define BP_GPIO_77_AL                           (77 | BP_ACTIVE_LOW)
#define BP_GPIO_78_AH                           (78)
#define BP_GPIO_78_AL                           (78 | BP_ACTIVE_LOW)
#define BP_GPIO_79_AH                           (79)
#define BP_GPIO_79_AL                           (79 | BP_ACTIVE_LOW)
#define BP_GPIO_80_AH                           (80)
#define BP_GPIO_80_AL                           (80 | BP_ACTIVE_LOW)
#define BP_GPIO_81_AH                           (81)
#define BP_GPIO_81_AL                           (81 | BP_ACTIVE_LOW)
#define BP_GPIO_82_AH                           (82)
#define BP_GPIO_82_AL                           (82 | BP_ACTIVE_LOW)
#define BP_GPIO_83_AH                           (83)
#define BP_GPIO_83_AL                           (83 | BP_ACTIVE_LOW)
#define BP_GPIO_84_AH                           (84)
#define BP_GPIO_84_AL                           (84 | BP_ACTIVE_LOW)
#define BP_GPIO_85_AH                           (85)
#define BP_GPIO_85_AL                           (85 | BP_ACTIVE_LOW)
#define BP_GPIO_86_AH                           (86)
#define BP_GPIO_86_AL                           (86 | BP_ACTIVE_LOW)
#define BP_GPIO_87_AH                           (87)
#define BP_GPIO_87_AL                           (87 | BP_ACTIVE_LOW)
#define BP_GPIO_88_AH                           (88)
#define BP_GPIO_88_AL                           (88 | BP_ACTIVE_LOW)
#define BP_GPIO_89_AH                           (89)
#define BP_GPIO_89_AL                           (89 | BP_ACTIVE_LOW)
#define BP_GPIO_90_AH                           (90)
#define BP_GPIO_90_AL                           (90 | BP_ACTIVE_LOW)
#define BP_GPIO_91_AH                           (91)
#define BP_GPIO_91_AL                           (91 | BP_ACTIVE_LOW)
#define BP_GPIO_92_AH                           (92)
#define BP_GPIO_92_AL                           (92 | BP_ACTIVE_LOW)
#define BP_GPIO_93_AH                           (93)
#define BP_GPIO_93_AL                           (93 | BP_ACTIVE_LOW)
#define BP_GPIO_94_AH                           (94)
#define BP_GPIO_94_AL                           (94 | BP_ACTIVE_LOW)
#define BP_GPIO_95_AH                           (95)
#define BP_GPIO_95_AL                           (95 | BP_ACTIVE_LOW)
#define BP_GPIO_96_AH                           (96)
#define BP_GPIO_96_AL                           (96 | BP_ACTIVE_LOW)
#define BP_GPIO_97_AH                           (97)
#define BP_GPIO_97_AL                           (97 | BP_ACTIVE_LOW)
#define BP_GPIO_98_AH                           (98)
#define BP_GPIO_98_AL                           (98 | BP_ACTIVE_LOW)
#define BP_GPIO_99_AH                           (99)
#define BP_GPIO_99_AL                           (99 | BP_ACTIVE_LOW)
#define BP_GPIO_100_AH                          (100)
#define BP_GPIO_100_AL                          (100 | BP_ACTIVE_LOW)
#define BP_GPIO_101_AH                          (101)
#define BP_GPIO_101_AL                          (101 | BP_ACTIVE_LOW)
#define BP_GPIO_102_AH                          (102)
#define BP_GPIO_102_AL                          (102 | BP_ACTIVE_LOW)
#define BP_GPIO_103_AH                          (103)
#define BP_GPIO_103_AL                          (103 | BP_ACTIVE_LOW)
#define BP_GPIO_104_AH                          (104)
#define BP_GPIO_104_AL                          (104 | BP_ACTIVE_LOW)
#define BP_GPIO_105_AH                          (105)
#define BP_GPIO_105_AL                          (105 | BP_ACTIVE_LOW)
#define BP_GPIO_106_AH                          (106)
#define BP_GPIO_106_AL                          (106 | BP_ACTIVE_LOW)
#define BP_GPIO_107_AH                          (107)
#define BP_GPIO_107_AL                          (107 | BP_ACTIVE_LOW)
#define BP_GPIO_108_AH                          (108)
#define BP_GPIO_108_AL                          (108 | BP_ACTIVE_LOW)
#define BP_GPIO_109_AH                          (109)
#define BP_GPIO_109_AL                          (109 | BP_ACTIVE_LOW)
#define BP_GPIO_110_AH                          (110)
#define BP_GPIO_110_AL                          (110 | BP_ACTIVE_LOW)
#define BP_GPIO_111_AH                          (111)
#define BP_GPIO_111_AL                          (111 | BP_ACTIVE_LOW)
#define BP_GPIO_112_AH                          (112)
#define BP_GPIO_112_AL                          (112 | BP_ACTIVE_LOW)
#define BP_GPIO_113_AH                          (113)
#define BP_GPIO_113_AL                          (113 | BP_ACTIVE_LOW)
#define BP_GPIO_114_AH                          (114)
#define BP_GPIO_114_AL                          (114 | BP_ACTIVE_LOW)
#define BP_GPIO_115_AH                          (115)
#define BP_GPIO_115_AL                          (115 | BP_ACTIVE_LOW)
#define BP_GPIO_116_AH                          (116)
#define BP_GPIO_116_AL                          (116 | BP_ACTIVE_LOW)
#define BP_GPIO_117_AH                          (117)
#define BP_GPIO_117_AL                          (117 | BP_ACTIVE_LOW)
#define BP_GPIO_118_AH                          (118)
#define BP_GPIO_118_AL                          (118 | BP_ACTIVE_LOW)
#define BP_GPIO_119_AH                          (119)
#define BP_GPIO_119_AL                          (119 | BP_ACTIVE_LOW)
#define BP_GPIO_120_AL                          (120 | BP_ACTIVE_LOW)
#define BP_GPIO_121_AH                          (121)
#define BP_GPIO_121_AL                          (121 | BP_ACTIVE_LOW)
#define BP_GPIO_122_AH                          (122)
#define BP_GPIO_122_AL                          (122 | BP_ACTIVE_LOW)
#define BP_GPIO_123_AH                          (123)
#define BP_GPIO_123_AL                          (123 | BP_ACTIVE_LOW)
#define BP_GPIO_124_AH                          (124)
#define BP_GPIO_124_AL                          (124 | BP_ACTIVE_LOW)
#define BP_GPIO_125_AH                          (125)
#define BP_GPIO_125_AL                          (125 | BP_ACTIVE_LOW)
#define BP_GPIO_126_AH                          (126)
#define BP_GPIO_126_AL                          (126 | BP_ACTIVE_LOW)
#define BP_GPIO_127_AH                          (127)
#define BP_GPIO_127_AL                          (127 | BP_ACTIVE_LOW)
#define BP_GPIO_128_AH                          (128)
#define BP_GPIO_128_AL                          (128 | BP_ACTIVE_LOW)
#define BP_GPIO_129_AH                          (129)
#define BP_GPIO_129_AL                          (129 | BP_ACTIVE_LOW)
#define BP_GPIO_130_AL                          (130 | BP_ACTIVE_LOW)
#define BP_GPIO_131_AH                          (131)
#define BP_GPIO_131_AL                          (131 | BP_ACTIVE_LOW)
#define BP_GPIO_132_AH                          (132)
#define BP_GPIO_132_AL                          (132 | BP_ACTIVE_LOW)
#define BP_GPIO_133_AH                          (133)
#define BP_GPIO_133_AL                          (133 | BP_ACTIVE_LOW)
#define BP_GPIO_134_AH                          (134)
#define BP_GPIO_134_AL                          (134 | BP_ACTIVE_LOW)
#define BP_GPIO_135_AH                          (135)
#define BP_GPIO_135_AL                          (135 | BP_ACTIVE_LOW)
#define BP_GPIO_136_AH                          (136)
#define BP_GPIO_136_AL                          (136 | BP_ACTIVE_LOW)
#define BP_GPIO_137_AH                          (137)
#define BP_GPIO_137_AL                          (137 | BP_ACTIVE_LOW)
#define BP_GPIO_138_AH                          (138)
#define BP_GPIO_138_AL                          (138 | BP_ACTIVE_LOW)
#define BP_GPIO_139_AH                          (139)
#define BP_GPIO_139_AL                          (139 | BP_ACTIVE_LOW)
#define BP_GPIO_140_AL                          (140 | BP_ACTIVE_LOW)
#define BP_GPIO_141_AH                          (141)
#define BP_GPIO_141_AL                          (141 | BP_ACTIVE_LOW)


#define BP_SERIAL_GPIO_0_AH                     (0  | BP_GPIO_SERIAL)
#define BP_SERIAL_GPIO_0_AL                     (0  | BP_GPIO_SERIAL | BP_ACTIVE_LOW)
#define BP_SERIAL_GPIO_1_AH                     (1  | BP_GPIO_SERIAL)
#define BP_SERIAL_GPIO_1_AL                     (1  | BP_GPIO_SERIAL | BP_ACTIVE_LOW)
#define BP_SERIAL_GPIO_2_AH                     (2  | BP_GPIO_SERIAL)
#define BP_SERIAL_GPIO_2_AL                     (2  | BP_GPIO_SERIAL | BP_ACTIVE_LOW)
#define BP_SERIAL_GPIO_3_AH                     (3  | BP_GPIO_SERIAL)
#define BP_SERIAL_GPIO_3_AL                     (3  | BP_GPIO_SERIAL | BP_ACTIVE_LOW)
#define BP_SERIAL_GPIO_4_AH                     (4  | BP_GPIO_SERIAL)
#define BP_SERIAL_GPIO_4_AL                     (4  | BP_GPIO_SERIAL | BP_ACTIVE_LOW)
#define BP_SERIAL_GPIO_5_AH                     (5  | BP_GPIO_SERIAL)
#define BP_SERIAL_GPIO_5_AL                     (5  | BP_GPIO_SERIAL | BP_ACTIVE_LOW)
#define BP_SERIAL_GPIO_6_AH                     (6  | BP_GPIO_SERIAL)
#define BP_SERIAL_GPIO_6_AL                     (6  | BP_GPIO_SERIAL | BP_ACTIVE_LOW)
#define BP_SERIAL_GPIO_7_AH                     (7  | BP_GPIO_SERIAL)
#define BP_SERIAL_GPIO_7_AL                     (7  | BP_GPIO_SERIAL | BP_ACTIVE_LOW)
#define BP_SERIAL_GPIO_8_AH                     (8  | BP_GPIO_SERIAL)
#define BP_SERIAL_GPIO_8_AL                     (8  | BP_GPIO_SERIAL | BP_ACTIVE_LOW)
#define BP_SERIAL_GPIO_9_AH                     (9  | BP_GPIO_SERIAL)
#define BP_SERIAL_GPIO_9_AL                     (9  | BP_GPIO_SERIAL | BP_ACTIVE_LOW)
#define BP_SERIAL_GPIO_10_AH                    (10 | BP_GPIO_SERIAL)
#define BP_SERIAL_GPIO_10_AL                    (10 | BP_GPIO_SERIAL | BP_ACTIVE_LOW)
#define BP_SERIAL_GPIO_11_AH                    (11 | BP_GPIO_SERIAL)
#define BP_SERIAL_GPIO_11_AL                    (11 | BP_GPIO_SERIAL | BP_ACTIVE_LOW)
#define BP_SERIAL_GPIO_12_AH                    (12 | BP_GPIO_SERIAL)
#define BP_SERIAL_GPIO_12_AL                    (12 | BP_GPIO_SERIAL | BP_ACTIVE_LOW)
#define BP_SERIAL_GPIO_13_AH                    (13 | BP_GPIO_SERIAL)
#define BP_SERIAL_GPIO_13_AL                    (13 | BP_GPIO_SERIAL | BP_ACTIVE_LOW)
#define BP_SERIAL_GPIO_14_AH                    (14 | BP_GPIO_SERIAL)
#define BP_SERIAL_GPIO_14_AL                    (14 | BP_GPIO_SERIAL | BP_ACTIVE_LOW)
#define BP_SERIAL_GPIO_15_AH                    (15 | BP_GPIO_SERIAL)
#define BP_SERIAL_GPIO_15_AL                    (15 | BP_GPIO_SERIAL | BP_ACTIVE_LOW)
#define BP_SERIAL_GPIO_16_AH                    (16 | BP_GPIO_SERIAL)
#define BP_SERIAL_GPIO_16_AL                    (16 | BP_GPIO_SERIAL | BP_ACTIVE_LOW)
#define BP_SERIAL_GPIO_17_AH                    (17 | BP_GPIO_SERIAL)
#define BP_SERIAL_GPIO_17_AL                    (17 | BP_GPIO_SERIAL | BP_ACTIVE_LOW)
#define BP_SERIAL_GPIO_18_AH                    (18 | BP_GPIO_SERIAL)
#define BP_SERIAL_GPIO_18_AL                    (18 | BP_GPIO_SERIAL | BP_ACTIVE_LOW)
#define BP_SERIAL_GPIO_19_AH                    (19 | BP_GPIO_SERIAL)
#define BP_SERIAL_GPIO_19_AL                    (19 | BP_GPIO_SERIAL | BP_ACTIVE_LOW)
#define BP_SERIAL_GPIO_20_AH                    (20 | BP_GPIO_SERIAL)
#define BP_SERIAL_GPIO_20_AL                    (20 | BP_GPIO_SERIAL | BP_ACTIVE_LOW)
#define BP_SERIAL_GPIO_21_AH                    (21 | BP_GPIO_SERIAL)
#define BP_SERIAL_GPIO_21_AL                    (21 | BP_GPIO_SERIAL | BP_ACTIVE_LOW)
#define BP_SERIAL_GPIO_22_AH                    (22 | BP_GPIO_SERIAL)
#define BP_SERIAL_GPIO_22_AL                    (22 | BP_GPIO_SERIAL | BP_ACTIVE_LOW)
#define BP_SERIAL_GPIO_23_AH                    (23 | BP_GPIO_SERIAL)
#define BP_SERIAL_GPIO_23_AL                    (23 | BP_GPIO_SERIAL | BP_ACTIVE_LOW)
#define BP_SERIAL_GPIO_24_AH                    (24 | BP_GPIO_SERIAL)
#define BP_SERIAL_GPIO_24_AL                    (24 | BP_GPIO_SERIAL | BP_ACTIVE_LOW)
#define BP_SERIAL_GPIO_25_AH                    (25 | BP_GPIO_SERIAL)
#define BP_SERIAL_GPIO_25_AL                    (25 | BP_GPIO_SERIAL | BP_ACTIVE_LOW)
#define BP_SERIAL_GPIO_26_AH                    (26 | BP_GPIO_SERIAL)
#define BP_SERIAL_GPIO_26_AL                    (26 | BP_GPIO_SERIAL | BP_ACTIVE_LOW)
#define BP_SERIAL_GPIO_27_AH                    (27 | BP_GPIO_SERIAL)
#define BP_SERIAL_GPIO_27_AL                    (27 | BP_GPIO_SERIAL | BP_ACTIVE_LOW)
#define BP_SERIAL_GPIO_28_AH                    (28 | BP_GPIO_SERIAL)
#define BP_SERIAL_GPIO_28_AL                    (28 | BP_GPIO_SERIAL | BP_ACTIVE_LOW)
#define BP_SERIAL_GPIO_29_AH                    (29 | BP_GPIO_SERIAL)
#define BP_SERIAL_GPIO_29_AL                    (29 | BP_GPIO_SERIAL | BP_ACTIVE_LOW)
#define BP_SERIAL_GPIO_30_AH                    (30 | BP_GPIO_SERIAL)
#define BP_SERIAL_GPIO_30_AL                    (30 | BP_GPIO_SERIAL | BP_ACTIVE_LOW)
#define BP_SERIAL_GPIO_31_AH                    (31 | BP_GPIO_SERIAL)
#define BP_SERIAL_GPIO_31_AL                    (31 | BP_GPIO_SERIAL | BP_ACTIVE_LOW)

#define BP_LED_0_AH                             (0 | BP_LED_PIN)
#define BP_LED_0_AL                             (0 | BP_LED_PIN | BP_ACTIVE_LOW)
#define BP_LED_1_AH                             (1 | BP_LED_PIN)
#define BP_LED_1_AL                             (1 | BP_LED_PIN | BP_ACTIVE_LOW)
#define BP_LED_2_AH                             (2 | BP_LED_PIN)
#define BP_LED_2_AL                             (2 | BP_LED_PIN | BP_ACTIVE_LOW)
#define BP_LED_3_AH                             (3 | BP_LED_PIN)
#define BP_LED_3_AL                             (3 | BP_LED_PIN | BP_ACTIVE_LOW)
#define BP_LED_4_AH                             (4 | BP_LED_PIN)
#define BP_LED_4_AL                             (4 | BP_LED_PIN | BP_ACTIVE_LOW)
#define BP_LED_5_AH                             (5 | BP_LED_PIN)
#define BP_LED_5_AL                             (5 | BP_LED_PIN | BP_ACTIVE_LOW)
#define BP_LED_6_AH                             (6 | BP_LED_PIN)
#define BP_LED_6_AL                             (6 | BP_LED_PIN | BP_ACTIVE_LOW)
#define BP_LED_7_AH                             (7 | BP_LED_PIN)
#define BP_LED_7_AL                             (7 | BP_LED_PIN | BP_ACTIVE_LOW)

/* LED controller can shift out 16 bit out of 24 bit data for serial GPIO output. Which
 * 16 bits selected by default is chip dependent. The 24 bits data are grouped to 3 groups
 * and can be configured to pick two groups as the output.
 *
 * On 6318, group 1 and 2(bit 8 to 23) are shifted out by default. But user can configure to
 * select group 0 and 2(bit 0 to 7 and bit 16 to 23).
 */
#define BP_SERIAL_MUX_SEL_GROUP0                1 /* bit  0 to  7 */
#define BP_SERIAL_MUX_SEL_GROUP1                2 /* bit  8 to 15 */
#define BP_SERIAL_MUX_SEL_GROUP2                4 /* bit 16 to 23 */

/* for OREN boards - shift register can be connected in three ways */
#define BP_SERIAL_LED_MUX_GROUPA                0x10    /*CLK is pin 65, DATA is pin 67 */
#define BP_SERIAL_LED_MUX_GROUPB                0x20    /*CLK is pin 33, DATA is pin 34 */
#define BP_SERIAL_LED_MUX_GROUPC                0x40    /*CLK is pin 10, DATA is pin 11 */

#define BP_SERIAL_LED_SHIFT_LSB_FIRST           0x0
#define BP_SERIAL_LED_SHIFT_MSB_FIRST           0x1

/* Values for external interrupt assignments. */
#define BP_EXT_INTR_0                           0
#define BP_EXT_INTR_1                           1
#define BP_EXT_INTR_2                           2
#define BP_EXT_INTR_3                           3
#define BP_EXT_INTR_4                           4
#define BP_EXT_INTR_5                           5
#define BP_EXT_INTR_6                           6
#define BP_EXT_INTR_7                           7
#define BP_EXT_INTR_NUM_MAX                     (BP_EXT_INTR_7+1)
#define BP_EXT_INTR_NONE                        0x00ff
#define BP_EXT_INTR_NUM_MASK                    0x00ff

/* Interrupt trigger types */
#define BP_EXT_INTR_TYPE_IRQ_LEVEL_MASK         (0x1 << 12)
#define BP_EXT_INTR_TYPE_IRQ_LOW_LEVEL          (0x0 << 12)
#define BP_EXT_INTR_TYPE_IRQ_HIGH_LEVEL         (0x1 << 12)
#define BP_EXT_INTR_TYPE_IRQ_SENSE_MASK         (0x1 << 13)
#define BP_EXT_INTR_TYPE_IRQ_SENSE_LEVEL        (0x0 << 13)
#define BP_EXT_INTR_TYPE_IRQ_SENSE_EDGE         (0x1 << 13)
#define BP_EXT_INTR_TYPE_IRQ_BOTH_EDGE          (0x1 << 14)
#define BP_EXT_INTR_TYPE_IRQ_BOTH_EDGE_MASK     (0x1 << 14)
#define BP_EXT_INTR_TYPE_IRQ_MASK               (0x7 << 12)

#define BP_EXT_INTR_NOT_SHARED                  (0x0 << 15)
#define BP_EXT_INTR_SHARED                      (0x1 << 15)
#define BP_EXT_INTR_SHARE_MASK                  (0x1 << 15)

#define BP_EXT_INTR_CONFLICT_MASK               (0x1 << 31)

#define BP_EXT_INTR_FLAGS_MASK                  (BP_EXT_INTR_TYPE_IRQ_MASK | BP_EXT_INTR_SHARE_MASK | BP_EXT_INTR_CONFLICT_MASK)

#define IsExtIntrTypeActLow(irq)                ((irq&BP_EXT_INTR_TYPE_IRQ_LEVEL_MASK) == BP_EXT_INTR_TYPE_IRQ_LOW_LEVEL)
#define IsExtIntrTypeActHigh(irq)               ((irq&BP_EXT_INTR_TYPE_IRQ_LEVEL_MASK) == BP_EXT_INTR_TYPE_IRQ_HIGH_LEVEL)
#define IsExtIntrTypeSenseLevel(irq)            ((irq&BP_EXT_INTR_TYPE_IRQ_SENSE_MASK) == BP_EXT_INTR_TYPE_IRQ_SENSE_LEVEL)
#define IsExtIntrTypeSenseEdge(irq)             ((irq&BP_EXT_INTR_TYPE_IRQ_SENSE_MASK) == BP_EXT_INTR_TYPE_IRQ_SENSE_EDGE)
#define IsExtIntrTypeBothEdge(irq)              ((irq&BP_EXT_INTR_TYPE_IRQ_BOTH_EDGE_MASK) == BP_EXT_INTR_TYPE_IRQ_BOTH_EDGE)
#define IsExtIntrShared(irq)                    ((irq&BP_EXT_INTR_SHARE_MASK) == BP_EXT_INTR_SHARED)
#define IsExtIntrConflict(irq)                  (irq&BP_EXT_INTR_CONFLICT_MASK)

/* Values for chip select assignments. */
#define BP_CS_0                                 0
#define BP_CS_1                                 1
#define BP_CS_2                                 2
#define BP_CS_3                                 3

#define BP_OVERLAY_GPON_TX_EN_L                 (1<<0)
#define BP_OVERLAY_PCI                          (1<<0)
#define BP_OVERLAY_PCIE_CLKREQ                  (1<<0)
#define BP_OVERLAY_CB                           (1<<1) // Unused
#define BP_OVERLAY_SPI_EXT_CS                   (1<<2)
#define BP_OVERLAY_UART1                        (1<<3) // Unused
#define BP_OVERLAY_PHY                          (1<<4)
#define BP_OVERLAY_SERIAL_LEDS                  (1<<5)
#define BP_OVERLAY_EPHY_LED_0                   (1<<6)
#define BP_OVERLAY_EPHY_LED_1                   (1<<7)
#define BP_OVERLAY_EPHY_LED_2                   (1<<8)
#define BP_OVERLAY_EPHY_LED_3                   (1<<9)
#define BP_OVERLAY_GPHY_LED_0                   (1<<10)
#define BP_OVERLAY_GPHY_LED_1                   (1<<11)
#define BP_OVERLAY_INET_LED                     (1<<12)
#define BP_OVERLAY_MOCA_LED                     (1<<13)
#define BP_OVERLAY_USB_LED                      (1<<14)
#define BP_OVERLAY_USB_DEVICE                   (1<<15)
#define BP_OVERLAY_SPI_SSB2_EXT_CS              (1<<16)
#define BP_OVERLAY_SPI_SSB3_EXT_CS              (1<<17)
/* Redefine to be consistent. The legacy and HS SPI controllers share the slave select pin */
#define BP_OVERLAY_HS_SPI_SSB2_EXT_CS           BP_OVERLAY_SPI_SSB2_EXT_CS
#define BP_OVERLAY_HS_SPI_SSB3_EXT_CS           BP_OVERLAY_SPI_SSB3_EXT_CS
#define BP_OVERLAY_HS_SPI_SSB4_EXT_CS           (1<<18)
#define BP_OVERLAY_HS_SPI_SSB5_EXT_CS           (1<<19)
#define BP_OVERLAY_HS_SPI_SSB6_EXT_CS           (1<<20)
#define BP_OVERLAY_HS_SPI_SSB7_EXT_CS           (1<<21)
#define BP_OVERLAY_VREG_CLK                     (1<<22) // VREG_SYNC

#define BP_SIMCARD_GROUPA           1
#define BP_SIMCARD_GROUPA_OD        2
#define BP_SIMCARD_GROUPB           3

#define BP_SLIC_GROUPC           3
#define BP_SLIC_GROUPD           4

/* Value for GPIO and external interrupt fields that are not used. */
#define BP_NOT_DEFINED                          0xffff

/* Maximum size of the board id string. */
#define BP_BOARD_ID_LEN                         16

/* Maximum size of the board id string. */
#define BP_OPTICAL_PARAMS_LEN                   48

/* Maximum number of pinmux entries. */
#define BP_PINMUX_MAX                           256

/* Maximum number of Ethernet MACs. */
#define BP_MAX_ENET_MACS                        2
#if defined(CONFIG_BCM96858) || defined(_BCM96858_) || \
    defined(CONFIG_BCM96856) || defined(_BCM96856_) || \
    defined(CONFIG_BCM96846) || defined(_BCM96846_) || \
    defined(CONFIG_BCM96878) || defined(_BCM96878_)
#define BP_MAX_SWITCH_PORTS                     9
#else
#define BP_MAX_SWITCH_PORTS                     8
#endif
#define BP_CROSSBAR_PORT_BASE                   (BP_MAX_SWITCH_PORTS + 1)
/* 63138B0 onwards 5x3 crossbar - so the MAX_xxx_PORTS are changed to the max needed by 5x3 */
#define BP_MAX_CROSSBAR_EXT_PORTS               5
#define BP_MAX_CROSSBAR_INT_PORTS               3
#define BP_MAX_PHY_PORTS                        (BP_CROSSBAR_PORT_BASE + BP_MAX_CROSSBAR_EXT_PORTS)
#define BP_PHY_PORT_TO_CROSSBAR_PORT(phy_port)  (phy_port - BP_CROSSBAR_PORT_BASE)
#define BP_CROSSBAR_PORT_TO_PHY_PORT(cross_port) (cross_port + BP_CROSSBAR_PORT_BASE)
#define BP_IS_CROSSBAR_PORT(phy_port)            (phy_port >= BP_CROSSBAR_PORT_BASE)
#define BP_IS_CROSSBAR_PORT_DEFINED(sw_info,cross_port) (sw_info.crossbar[cross_port].switch_port != BP_CROSSBAR_NOT_DEFINED)
#define BP_MAX_ENET_INTERNAL                    2
/* Maximum number of VoIP DSPs. */
#define BP_MAX_VOIP_DSP                         2

/* Wireless Antenna Settings. */
#define BP_WLAN_ANT_MAIN                        0
#define BP_WLAN_ANT_AUX                         1
#define BP_WLAN_ANT_BOTH                        3

/* Wireless FLAGS */
#define BP_WLAN_MAC_ADDR_OVERRIDE               0x0001   /* use kerSysGetMacAddress for mac address */
#define BP_WLAN_EXCLUDE_ONBOARD                 0x0002   /* exclude onboard wireless  */
#define BP_WLAN_EXCLUDE_ONBOARD_FORCE           0x0004   /* force exclude onboard wireless even without addon card*/
#define BP_WLAN_USE_OTP                         0x0008   /* don't use sw srom map, may fall to OTP or uninitialzed */

#define BP_WLAN_NVRAM_NAME_LEN      16
#define BP_WLAN_MAX_PATCH_ENTRY     32

/* AFE IDs */
#define BP_AFE_DEFAULT                  0

#define BP_AFE_CHIP_INT                 (1 << 28)
#define BP_AFE_CHIP_6505                (2 << 28)
#define BP_AFE_CHIP_6306                (3 << 28)
#define BP_AFE_CHIP_CH0                 (4 << 28)
#define BP_AFE_CHIP_CH1                 (5 << 28)
#define BP_AFE_CHIP_GFAST               (6 << 28)
#define BP_AFE_CHIP_GFAST0              (6 << 28)
#define BP_AFE_CHIP_GFAST_CH0           (7 << 28)
#define BP_AFE_CHIP_GFAST1              (8 << 28)
#define BP_AFE_CHIP_GFAST_CH1           (9 << 28)

#define BP_AFE_LD_ISIL1556              (1 << 21)
#define BP_AFE_LD_6301                  (2 << 21)
#define BP_AFE_LD_6302                  (3 << 21)
#define BP_AFE_LD_6303                  (4 << 21)
#define BP_AFE_LD_6304                  (5 << 21)
#define BP_AFE_LD_6305                  (6 << 21)

#define BP_AFE_LD_REV_6303_VR5P3        (1 << 18)

#define BP_AFE_FE_ANNEXA                (1 << 15)
#define BP_AFE_FE_ANNEXB                (2 << 15)
#define BP_AFE_FE_ANNEXJ                (3 << 15)
#define BP_AFE_FE_ANNEXBJ               (4 << 15)
#define BP_AFE_FE_ANNEXM                (5 << 15)
#define BP_AFE_FE_ANNEXC                (6 << 15)

#define BP_AFE_FE_AVMODE_COMBO          (0 << 13)
#define BP_AFE_FE_AVMODE_ADSL           (1 << 13)
#define BP_AFE_FE_AVMODE_VDSL           (2 << 13)

/* VDSL only */
#define BP_AFE_FE_REV_ISIL_REV1         (1 << 8)
#define BP_AFE_FE_REV_12_20             BP_AFE_FE_REV_ISIL_REV1
#define BP_AFE_FE_REV_12_21             (2 << 8)

/* Combo */
#define BP_AFE_FE_REV_6302_REV1         (1 << 8)
#define BP_AFE_FE_REV_6302_REV_7_12     (1 << 8)
#define BP_AFE_FE_REV_6302_REV_7_2_21   (2 << 8)

#define BP_AFE_FE_REV_6302_REV_7_2_1    (3 << 8)
#define BP_AFE_FE_REV_6302_REV_7_2      (4 << 8)
#define BP_AFE_FE_REV_6302_REV_7_2_UR2  (5 << 8)
#define BP_AFE_FE_REV_6302_REV_7_2_2    (6 << 8)
#define BP_AFE_FE_REV_6302_REV_7_2_30    (7 << 8)
#define BP_AFE_6302_6306_REV_A_12_40    (8 << 8)
#define BP_AFE_FE_REV_6303_REV_12_3_30    (9 << 8)
#define BP_AFE_FE_REV_6303_REV_12_3_20    (1 << 8)
#define BP_AFE_FE_REV_6303_REV_12_3_40    (1 << 8)
#define BP_AFE_FE_REV_6303_REV_12_3_60    (1 << 8)
#define BP_AFE_FE_REV_6303_REV_12_3_50    (2 << 8)
#define BP_AFE_FE_REV_6303_REV_12_3_35    (3 << 8)
#define BP_AFE_FE_REV_6303_REV_12_3_70    (3 << 8)
#define BP_AFE_FE_REV_6303_REV_12_50      (1 << 8)
#define BP_AFE_FE_REV_6303_REV_12_51      (2 << 8)
#define BP_AFE_FE_REV_6304_REV_12_4_40      (1 << 8)
#define BP_AFE_FE_REV_6304_REV_12_4_45      (2 << 8)
#define BP_AFE_FE_REV_6304_REV_12_4_60      (1 << 8)
#define BP_AFE_FE_REV_6305_REV_12_5_60_1    (1 << 8)
#define BP_AFE_FE_REV_6305_REV_12_5_60_2    (2 << 8)


/* ADSL only*/
#define BP_AFE_FE_REV_6302_REV_5_2_1    (1 << 8)
#define BP_AFE_FE_REV_6302_REV_5_2_2    (2 << 8)
#define BP_AFE_FE_REV_6302_REV_5_2_3    (3 << 8)
#define BP_AFE_FE_REV_6301_REV_5_1_1    (1 << 8)
#define BP_AFE_FE_REV_6301_REV_5_1_2    (2 << 8)
#define BP_AFE_FE_REV_6301_REV_5_1_3    (3 << 8)
#define BP_AFE_FE_REV_6301_REV_5_1_4    (4 << 8)

#define BP_AFE_FE_COAX                  (1 << 7)

#define BP_AFE_FE_RNC                   (1 << 6)
#define BP_AFE_FE_8dBm                  (1 << 5)


#define BP_GET_EXT_AFE_DEFINED
#define BP_GET_INT_AFE_DEFINED


/* DEVICE CONFIGURATION OPTIONS */

#define BP_DEVICE_OPTION_ENABLE_GMAC           (1 << 0)
#define BP_DEVICE_OPTION_DISABLE_LED_INVERSION (1 << 1)

#if !defined(__ASSEMBLER__)

#define BP_MOCA_TYPE_WAN                0
#define BP_MOCA_TYPE_LAN                1
#define BP_MOCA_MAX_NUM                 2

#define BP_MOCA_RF_BAND_D_LOW           0
#define BP_MOCA_RF_BAND_D_HIGH          1
#define BP_MOCA_RF_BAND_EXT_D           2
#define BP_MOCA_RF_BAND_E               3
#define BP_MOCA_RF_BAND_F               4

#define BP_MOCA_HOST_INTR_IDX           0
#define BP_MOCA_SB_INTR_BASE            1
#define BP_MOCA_MAX_SB_INTR_NUM         6
#define BP_MOCA_MAX_INTR_NUM            7  /* host intr, sb0 to 4, last one is sb_all */

enum cmdToExec
{
    CMD_READ,
    CMD_WRITE,
    CMD_WAIT,
    CMD_END
};

typedef struct
{
    enum cmdToExec command;
    unsigned int addr;
    unsigned int value;
} BpCmdElem;


typedef struct _BP_SPISLAVE_INFO {
  unsigned short resetGpio;
  unsigned short bootModeGpio;
  unsigned short busNum;
  unsigned short select;
  unsigned short mode;
  unsigned int ctrlState;
  unsigned int maxFreq;
} BP_SPISLAVE_INFO, *PBP_SPISLAVE_INFO;


/* Information about MoCA chip */
typedef struct _BP_MOCA_INFO {
  unsigned int type;
  unsigned int rfBand;
  unsigned short intr[BP_MOCA_MAX_INTR_NUM];
  unsigned short intrGpio[BP_MOCA_MAX_INTR_NUM];
  BpCmdElem* initCmd;
  BP_SPISLAVE_INFO spiDevInfo;
} BP_MOCA_INFO, *PBP_MOCA_INFO;

/* Add a new id bp_usPhyConnType to specify the phy connection type and save it in phyconn field for app
 * because we are running out flag bits such as CONNECTED_TO_EPON_MAC in the phy id itself. This is used
 * initially for PLC MAC connection type, but will use it for MOCA, EPON, GPON in the futures and remove
 * the phy id connection type flags */

#define PHY_CONN_TYPE_INT_PHY           0  /* default, switch MAC port with integrated PHY */
#define PHY_CONN_TYPE_EXT_PHY           1  /* switch MAC port connected to external PHY */
#define PHY_CONN_TYPE_EXT_SW            2  /* switch MAC port connected to external switch */
#define PHY_CONN_TYPE_EPON              3  /* switch MAC port connected to EPON */
#define PHY_CONN_TYPE_GPON              4  /* switch MAC port connected to GPON */
#define PHY_CONN_TYPE_MOCA              5  /* switch MAC port connected to MOCA */
#define PHY_CONN_TYPE_PLC               6  /* switch MAC port connected to PLC */
#define PHY_CONN_TYPE_FEMTO             7  /* switch MAC port connected to FEMTO */
#define PHY_CONN_TYPE_MOCA_ETH          8  /* switch MAC port connected to MOCA ethernet */

#define PHY_CONN_TYPE_NOT_DEFINED       0xffff  /* not specified */
#define PHY_DEVNAME_NOT_DEFINED         0       /* not specified */

#define BP_CROSSBAR_NOT_DEFINED         -1

typedef struct {
  unsigned short duplexLed;
  unsigned short speedLed100;
  unsigned short speedLed1000;
  unsigned short LedLan;
} LED_INFO;

/* boardparms-based PHY initialization mechanism */
/* A new Phy-specific parameter, bp_pPhyInit, that can follow a PhyId and, if specified, contain a pointer to a list of tuples.
 * Each tuple would be an opcode followed by a 16-bit register Address followed by a Data WORD to be written to the Phy during
 * initialization. The list would be terminated with a 0 opcode.
 */
#define BP_MDIO_INIT_OP_NULL                0x0
#define BP_MDIO_INIT_OP_WRITE               0x1
#define BP_MDIO_INIT_OP_UPDATE              0x2 /* read-modify-write op */

struct bp_mdio_init_write {
    unsigned short op;
    unsigned short reg;
    unsigned int  data;
};

struct bp_mdio_init_update {
    unsigned short op;
    unsigned short reg;
    unsigned short mask;
    unsigned short data;
};

struct bp_mdio_init_template {
    unsigned short op;
    unsigned short dummy1;
    unsigned int  dummy2;
};

typedef struct bp_mdio_init {
  union {
    struct bp_mdio_init_template op;
    struct bp_mdio_init_write write;
    struct bp_mdio_init_update update;
  } u;
} bp_mdio_init_t;

typedef struct {
  unsigned int switch_port;
  unsigned int phy_id;
  unsigned int phy_id_ext;
  unsigned int phyconn;
  char *phy_devName;
  LED_INFO ledInfo;
  bp_mdio_init_t* phyinit;
  unsigned int port_flags;
  unsigned short phyReset;
  short oamIndex;
  unsigned int portMaxRate;
} ETHERNET_CROSSBAR_INFO;

/* Information about Ethernet switch */
typedef struct {
  unsigned int port_map;
  unsigned int phy_id[BP_MAX_SWITCH_PORTS];
  unsigned int phy_id_ext[BP_MAX_SWITCH_PORTS];
  unsigned int phyconn[BP_MAX_SWITCH_PORTS];
  char *phy_devName[BP_MAX_SWITCH_PORTS];
  LED_INFO ledInfo[BP_MAX_SWITCH_PORTS];
  bp_mdio_init_t* phyinit[BP_MAX_SWITCH_PORTS];
  unsigned int port_flags[BP_MAX_SWITCH_PORTS];
  unsigned short phyReset[BP_MAX_SWITCH_PORTS];
  short oamIndex[BP_MAX_SWITCH_PORTS];
  unsigned int portMaxRate[BP_MAX_SWITCH_PORTS];
  ETHERNET_CROSSBAR_INFO crossbar[BP_MAX_CROSSBAR_EXT_PORTS];
} ETHERNET_SW_INFO;

#define MAX_LEDS_PER_PORT 4
typedef struct {
    unsigned int is_activity_led_present;
    struct {
        unsigned int   SpeedLed[MAX_LEDS_PER_PORT];
        unsigned int   ActivityLed[MAX_LEDS_PER_PORT];
        short          skip_in_aggregate;
    } ledInfo[BP_MAX_SWITCH_PORTS];
} LEDS_ADVANCED_INFO;

#ifdef CONFIG_BP_PHYS_INTF

#define BP_MAX_PHYS_INTF_PORTS                32

#define BP_INTF_TYPE_INVALID                  0
#define BP_INTF_TYPE_xDSL                     1
#define BP_INTF_TYPE_xPON                     2      /* GPON, EPON and AE */
#define BP_INTF_TYPE_xMII                     3
#define BP_INTF_TYPE_GPHY                     4  
#define BP_INTF_TYPE_SGMII                    5
#define BP_INTF_TYPE_I2C                      6
#define BP_INTF_TYPE_UART                     7
#define BP_INTF_TYPE_MAX                      8

#define BP_INTF_MGMT_TYPE_I2C                 0
#define BP_INTF_MGMT_TYPE_MDIO                1
#define BP_INTF_MGMT_TYPE_SPI                 2
#define BP_INTF_MGMT_TYPE_MAX                 3

/* SF2 does not have fixed number for its WAN port. Use a large number
   for identification purpose only */
#define SF2_WAN_PORT_NUM                      64
      
typedef struct {
    unsigned short intfId;
    unsigned short intfType;
    unsigned short portNum;
    bp_elem_t*     pIntfStart;
} PHYS_INTF_INFO;

typedef struct {
    unsigned int     SpeedLed[MAX_LEDS_PER_PORT];
    unsigned int     ActivityLed[MAX_LEDS_PER_PORT];
    unsigned int     LedSettings[MAX_LEDS_PER_PORT];
    PHYS_INTF_INFO*  pIntf;
} PHYS_INTF_ADV_LEDS_INFO;

#endif

#define BP_PHY_ID_0                            (0)
#define BP_PHY_ID_1                            (1)
#define BP_PHY_ID_2                            (2)
#define BP_PHY_ID_3                            (3)
#define BP_PHY_ID_4                            (4)
#define BP_PHY_ID_5                            (5)
#define BP_PHY_ID_6                            (6)
#define BP_PHY_ID_7                            (7)
#define BP_PHY_ID_8                            (8)
#define BP_PHY_ID_9                            (9)
#define BP_PHY_ID_10                           (10)
#define BP_PHY_ID_11                           (11)
#define BP_PHY_ID_12                           (12)
#define BP_PHY_ID_13                           (13)
#define BP_PHY_ID_14                           (14)
#define BP_PHY_ID_15                           (15)
#define BP_PHY_ID_16                           (16)
#define BP_PHY_ID_17                           (17)
#define BP_PHY_ID_18                           (18)
#define BP_PHY_ID_19                           (19)
#define BP_PHY_ID_20                           (20)
#define BP_PHY_ID_21                           (21)
#define BP_PHY_ID_22                           (22)
#define BP_PHY_ID_23                           (23)
#define BP_PHY_ID_24                           (24)
#define BP_PHY_ID_25                           (25)
#define BP_PHY_ID_26                           (26)
#define BP_PHY_ID_27                           (27)
#define BP_PHY_ID_28                           (28)
#define BP_PHY_ID_29                           (29)
#define BP_PHY_ID_30                           (30)
#define BP_PHY_ID_31                           (31)
#define BP_PHY_ID_NOT_SPECIFIED                (0xFF)
#define BP_PHY_NOT_PRESENT                     (0)


/* Phy config info embedded into phy_id of ETHERNET_SW_INFO */
#define PHY_LNK_CFG_M           0x7
#define PHY_LNK_CFG_S           8
#define ATONEG_FOR_LINK         (0 << PHY_LNK_CFG_S)
#define FORCE_LINK_DOWN         (1 << PHY_LNK_CFG_S)
#define FORCE_LINK_10HD         (2 << PHY_LNK_CFG_S)
#define FORCE_LINK_10FD         (3 << PHY_LNK_CFG_S)
#define FORCE_LINK_100HD        (4 << PHY_LNK_CFG_S)
#define FORCE_LINK_100FD        (5 << PHY_LNK_CFG_S)
#define FORCE_LINK_1000FD       (6 << PHY_LNK_CFG_S)
#define FORCE_LINK_200FD        (7 << PHY_LNK_CFG_S)

#define PHY_LNK_CFG_VALID_M     1
#define PHY_LNK_CFG_VALID_S     11
#define PHY_LNK_CFG_VALID       (PHY_LNK_CFG_VALID_M << PHY_LNK_CFG_VALID_S)

#define PHY_ADV_CAP_CFG_M       0x3F
#define PHY_ADV_CAP_CFG_S       12
#define ADVERTISE_10HD          (1 << PHY_ADV_CAP_CFG_S)
#define ADVERTISE_10FD          (2 << PHY_ADV_CAP_CFG_S)
#define ADVERTISE_100HD         (4 << PHY_ADV_CAP_CFG_S)
#define ADVERTISE_100FD         (8 << PHY_ADV_CAP_CFG_S)
#define ADVERTISE_1000HD        (16 << PHY_ADV_CAP_CFG_S)
#define ADVERTISE_1000FD        (32 << PHY_ADV_CAP_CFG_S)
#define ADVERTISE_ALL_GMII      (ADVERTISE_10HD | ADVERTISE_10FD | ADVERTISE_100HD | ADVERTISE_100FD | ADVERTISE_1000HD | ADVERTISE_1000FD)
#define ADVERTISE_ALL_MII       (ADVERTISE_10HD | ADVERTISE_10FD | ADVERTISE_100HD | ADVERTISE_100FD)

#define PHY_ADV_CFG_VALID_M     1
#define PHY_ADV_CFG_VALID_S     18
#define PHY_ADV_CFG_VALID       (PHY_ADV_CFG_VALID_M << PHY_ADV_CFG_VALID_S)

#define PHY_INTEGRATED_M        0x1
#define PHY_INTEGRATED_S        19
#define PHY_INTERNAL            (0 << PHY_INTEGRATED_S)
#define PHY_EXTERNAL            (1 << PHY_INTEGRATED_S)

#define PHY_INTEGRATED_VALID_M  1
#define PHY_INTEGRATED_VALID_S  20
#define PHY_INTEGRATED_VALID    (PHY_INTEGRATED_VALID_M << PHY_INTEGRATED_VALID_S)

#define MAC_CONN_M              0x1
#define MAC_CONN_S              21
#define MAC_CONNECTION          (MAC_CONN_M << MAC_CONN_S)
#define MAC_PHY_IF              (0 << MAC_CONN_S)
#define MAC_MAC_IF              (1 << MAC_CONN_S)

#define MAC_CONN_VALID_M        1
#define MAC_CONN_VALID_S        22
#define MAC_CONN_VALID          (MAC_CONN_VALID_M << MAC_CONN_VALID_S)

/* For RGMII interface, Default is 2.5V RGMII(MAC_IF_RGMII). 3.3V RGMII(MAC_IF_RGMII_3P3V) is supported in
 * 6818, 6318 and future devices
 */
#define MAC_IFACE_M             0xF
#define MAC_IFACE_S             23
#define MAC_IFACE               (MAC_IFACE_M << MAC_IFACE_S)
#define MAC_IF_INVALID          (0 << MAC_IFACE_S)
#define MAC_IF_MII              (1 << MAC_IFACE_S)
#define MAC_IF_RvMII            (2 << MAC_IFACE_S)
#define MAC_IF_GMII             (3 << MAC_IFACE_S)
#define MAC_IF_RGMII_1P8V       (4 << MAC_IFACE_S)
#define MAC_IF_RGMII_2P5V       (5 << MAC_IFACE_S)
#define MAC_IF_RGMII_3P3V       (6 << MAC_IFACE_S)
#define MAC_IF_RGMII            MAC_IF_RGMII_2P5V
#define MAC_IF_QSGMII           (7 << MAC_IFACE_S)
#define MAC_IF_SGMII            (8 << MAC_IFACE_S)
#define MAC_IF_HSGMII           (9 << MAC_IFACE_S)
#define MAC_IF_SS_SMII          (10 << MAC_IFACE_S)
#define MAC_IF_TMII             (11 << MAC_IFACE_S)
#define MAC_IF_SERDES           (12 << MAC_IFACE_S)
#define PHY_TYPE_CL45GPHY       (MAC_IF_QSGMII)
#define MAC_IF_XFI              (13 << MAC_IFACE_S)
#define MAC_IF_XGAE_SERDES      (14 << MAC_IFACE_S)

/* PORT FLAGS for specific ports, bp_ulPortFlags */
#define PORT_FLAG_MGMT              (1 << 0)
#define PORT_FLAG_SOFT_SWITCHING    (1 << 1)
#define PORT_FLAG_TX_INTERNAL_DELAY (1 << 2)
#define PORT_FLAG_RX_INTERNAL_DELAY (1 << 3)
#define PORT_FLAG_ATTACHED          (1 << 4)
#define PORT_FLAG_SWAP_PAIR         (1 << 5)
#define PORT_FLAG_DETECT            (1 << 6)
#define PORT_FLAG_AGGREGATE_SKIP    (1 << 7)

#define ATTACHED_FLAG_SHIFT    8
#define ATTACHED_FLAG_CONTROL  (1 << ATTACHED_FLAG_SHIFT)
#define ATTACHED_FLAG_ES       (2 << ATTACHED_FLAG_SHIFT)

#define PORT_FLAG_LANWAN_SHIFT      7
#define PORT_FLAG_LANWAN_M          (3 << PORT_FLAG_LANWAN_SHIFT)
#define PORT_FLAG_LANWAN_BOTH       (0 << PORT_FLAG_LANWAN_SHIFT)
#define PORT_FLAG_WAN_ONLY          (1 << PORT_FLAG_LANWAN_SHIFT)
#define PORT_FLAG_WAN_PREFERRED     (2 << PORT_FLAG_LANWAN_SHIFT)
#define PORT_FLAG_LAN_ONLY          (3 << PORT_FLAG_LANWAN_SHIFT)

#define EXTSW_CONNECTED_M       1
#define EXTSW_CONNECTED_S       27
#define EXTSW_CONNECTED         (EXTSW_CONNECTED_M << EXTSW_CONNECTED_S)

#define CONNECTED_TO_EPON_MAC_M 1
#define CONNECTED_TO_EPON_MAC_S 28
#define CONNECTED_TO_EPON_MAC  (CONNECTED_TO_EPON_MAC_M << CONNECTED_TO_EPON_MAC_S)

#define CONNECTED_TO_EXTERN_SW_M 1
#define CONNECTED_TO_EXTERN_SW_S 29
#define CONNECTED_TO_EXTERN_SW  (CONNECTED_TO_EXTERN_SW_M << CONNECTED_TO_EXTERN_SW_S)

/* MII over GPIO config info embedded into phy_id of ETHERNET_SW_INFO */
#define MII_OVER_GPIO_M          1
#define MII_OVER_GPIO_S          30
#define MII_OVER_GPIO_VALID      (MII_OVER_GPIO_M << MII_OVER_GPIO_S)

/* Currently used for qualifying WAN_PORT and MII_OVER_GPIO. Can be split into 2 if needed. */
#define PHYCFG_VALID_M           1
#define PHYCFG_VALID_S           31
#define PHYCFG_VALID             (PHYCFG_VALID_M << PHYCFG_VALID_S)

#define PHYID_LSBYTE_M           0xFF
#define BCM_PHY_ID_M             0x1F

/* MII - RvMII connection. Force Link to 100FD */
#define MII_DIRECT               (PHY_LNK_CFG_VALID | FORCE_LINK_100FD | MAC_CONN_VALID | MAC_MAC_IF | MAC_IF_MII)
#define RGMII_DIRECT             (PHY_LNK_CFG_VALID | FORCE_LINK_1000FD | MAC_CONN_VALID | MAC_MAC_IF | MAC_IF_RGMII)
#define RGMII_DIRECT_3P3V        (PHY_LNK_CFG_VALID | FORCE_LINK_1000FD | MAC_CONN_VALID | MAC_MAC_IF | MAC_IF_RGMII_3P3V)
#define GMII_DIRECT              (PHY_LNK_CFG_VALID | FORCE_LINK_1000FD | MAC_CONN_VALID | MAC_MAC_IF | MAC_IF_GMII)
#define TMII_DIRECT              (PHY_LNK_CFG_VALID | FORCE_LINK_200FD | MAC_CONN_VALID | MAC_MAC_IF | MAC_IF_TMII)

/* Optical Tranceiver values */
#define BP_OPTICAL_WAN_GPON      (1 << 0)
#define BP_OPTICAL_WAN_EPON      (1 << 1)
#define BP_OPTICAL_WAN_GPON_EPON (BP_OPTICAL_WAN_GPON | BP_OPTICAL_WAN_EPON)
#define BP_OPTICAL_WAN_AE        (1 << 2)

/* WAN port flag in the phy_id of ETHERNET_SW_INFO */
#define BCM_WAN_PORT        0x40
#define IsWanPort(id)       (((id) & PHYCFG_VALID)?((id) & BCM_WAN_PORT):(((id) & BCM_WAN_PORT) && (((id) & PHYID_LSBYTE_M) != 0xFF)))
#define IsPhyConnected(id)  (((id) & MAC_CONN_VALID)?(((id) & MAC_CONNECTION) != MAC_MAC_IF):(((id) & PHYID_LSBYTE_M) != 0xFF))
#define IsExtPhyId(id)      (((id) & PHY_INTEGRATED_VALID)?((id) & PHY_EXTERNAL):(((id) & BCM_PHY_ID_M) >= 0x10))
#define IsRgmiiDirect(id)   (((id) & RGMII_DIRECT) == RGMII_DIRECT)
#define IsRgmiiDirect_3P3V(id)   (((id) & RGMII_DIRECT_3P3V) == RGMII_DIRECT_3P3V)
#define IsRGMII_1P8V(id)    (((id) & MAC_IFACE) == MAC_IF_RGMII_1P8V)
#define IsRGMII_2P5V(id)    (((id) & MAC_IFACE) == MAC_IF_RGMII_2P5V)
#define IsRGMII_3P3V(id)    (((id) & MAC_IFACE) == MAC_IF_RGMII_3P3V)
#define IsRGMII(id)         (IsRGMII_2P5V((id)) || IsRGMII_3P3V((id)) || IsRGMII_1P8V(id))
#define IsRvMII(id)         (((id) & MAC_IFACE) == MAC_IF_RvMII)
#define IsGMII(id)          (((id) & MAC_IFACE) == MAC_IF_GMII)
#define IsMII(id)           (((id) & MAC_IFACE) == MAC_IF_MII)
#define IsQSGMII(id)        (((id) & MAC_IFACE) == MAC_IF_QSGMII)
#define IsSGMII(id)         (((id) & MAC_IFACE) == MAC_IF_SGMII)
#define IsSS_SMII(id)       (((id) & MAC_IFACE) == MAC_IF_SS_SMII)
#define IsTMII(id)          (((id) & MAC_IFACE) == MAC_IF_TMII)
#define IsSerdes(id)        (((id) & MAC_IFACE) == MAC_IF_SERDES || ((id) & MAC_IFACE) == MAC_IF_XGAE_SERDES)
#define IsCL45GPhy(id)        (((id) & MAC_IFACE) == PHY_TYPE_CL45GPHY)
#define IsPortConnectedToExternalSwitch(id)  (((id) & EXTSW_CONNECTED)?1:0)
#define IsPhyAdvCapConfigValid(id) (((id) & PHY_ADV_CFG_VALID)?1:0)
#define IsMacToMac(id)      (((id) & (MAC_CONN_VALID|MAC_CONNECTION)) == (MAC_CONN_VALID|MAC_MAC_IF))

#define IsPortMgmt(flags)            (((flags) & PORT_FLAG_MGMT) != 0)
#define IsPortSoftSwitching(flags)   (((flags) & PORT_FLAG_SOFT_SWITCHING) != 0)
#define IsPortTxInternalDelay(flags) (((flags) & PORT_FLAG_TX_INTERNAL_DELAY) != 0)
#define IsPortRxInternalDelay(flags) (((flags) & PORT_FLAG_RX_INTERNAL_DELAY) != 0)
#define IsPortSwapPair(flags)        (((flags) & PORT_FLAG_SWAP_PAIR) != 0)

#define c0(n) (((n) & 0x55555555) + (((n) >> 1) & 0x55555555))
#define c1(n) (((n) & 0x33333333) + (((n) >> 2) & 0x33333333))
#define c2(n) (((n) & 0x0f0f0f0f) + (((n) >> 4) & 0x0f0f0f0f))
#define bitcount(r, n) {r = n; r = c0(r); r = c1(r); r = c2(r); r %= 255;}

enum pmd_polarity
{
    pmd_use_def_polarity,
    pmd_polarity_invert
};

/* PMD functionalities (first 4 bits are reserved for pon type)*/
#define BP_PMD_APD_REG_DISABLED         (0x0 << 4)
#define BP_PMD_APD_REG_ENABLED          (0x1 << 4)
#define BP_PMD_APD_TYPE_FLYBACK         (0x0 << 5)
#define BP_PMD_APD_TYPE_BOOST           (0x1 << 5)


/* VREG Settings */
#define BP_VREG_EXTERNAL           1

/* Flags for use in pinmux muxinfo fields */
#define BP_PINMUX_PIN_MASK   0xff
#define BP_PINMUX_VAL_SHIFT  8
#define BP_PINMUX_VAL_MASK   (0xf << BP_PINMUX_VAL_SHIFT)
#define BP_PINMUX_VAL_0      (0 << BP_PINMUX_VAL_SHIFT)
#define BP_PINMUX_VAL_1      (1 << BP_PINMUX_VAL_SHIFT)
#define BP_PINMUX_VAL_2      (2 << BP_PINMUX_VAL_SHIFT)
#define BP_PINMUX_VAL_3      (3 << BP_PINMUX_VAL_SHIFT)
#define BP_PINMUX_VAL_4      (4 << BP_PINMUX_VAL_SHIFT)
#define BP_PINMUX_VAL_5      (5 << BP_PINMUX_VAL_SHIFT)
#define BP_PINMUX_VAL_6      (6 << BP_PINMUX_VAL_SHIFT)
#define BP_PINMUX_VAL_7      (7 << BP_PINMUX_VAL_SHIFT)
#define BP_PINMUX_VAL_DUMMY  BP_PINMUX_VAL_MASK
#define BP_PINMUX_ARG_SHIFT  12
#define BP_PINMUX_ARG_MASK   (0xff << BP_PINMUX_ARG_SHIFT)
#define BP_PINMUX_OP_SHIFT   20
#define BP_PINMUX_OP_MASK    (0x0f << BP_PINMUX_OP_SHIFT)
#define BP_PINMUX_VALID      (1 << 31)
#define BP_PINMUX_HWLED         ( 1 << BP_PINMUX_OP_SHIFT )
#define BP_PINMUX_SWLED         ( 2 << BP_PINMUX_OP_SHIFT )
#define BP_PINMUX_PWMLED        ( 3 << BP_PINMUX_OP_SHIFT )
#define BP_PINMUX_VDSLCTL       ( 4 << BP_PINMUX_OP_SHIFT )
#define BP_PINMUX_SWGPIO        ( 5 << BP_PINMUX_OP_SHIFT )
#define BP_PINMUX_DIRECT_HWLED  ( 6 << BP_PINMUX_OP_SHIFT )
#define BP_PINMUX_PADCTL        ( 7 << BP_PINMUX_OP_SHIFT )
#define BP_VDSLCTL_0  (BP_PINMUX_VDSLCTL | (0 << BP_PINMUX_ARG_SHIFT))
#define BP_VDSLCTL_1  (BP_PINMUX_VDSLCTL | (1 << BP_PINMUX_ARG_SHIFT))
#define BP_VDSLCTL_2  (BP_PINMUX_VDSLCTL | (2 << BP_PINMUX_ARG_SHIFT))
#define BP_VDSLCTL_3  (BP_PINMUX_VDSLCTL | (3 << BP_PINMUX_ARG_SHIFT))
#define BP_VDSLCTL_4  (BP_PINMUX_VDSLCTL | (4 << BP_PINMUX_ARG_SHIFT))
#define BP_VDSLCTL_5  (BP_PINMUX_VDSLCTL | (5 << BP_PINMUX_ARG_SHIFT))
#define BP_PINMUX_OPTLED_SHIFT   24
#define BP_PINMUX_OPTLED_MASK   (0x1f << BP_PINMUX_OPTLED_SHIFT)
#define BP_PINMUX_OPTLED_VALID  (0x20 << BP_PINMUX_OPTLED_SHIFT)
#define BP_PINMUX_OPTLED_NUM(n) (((n) << BP_PINMUX_OPTLED_SHIFT) | BP_PINMUX_OPTLED_VALID)

#define BP_PINMUX_FNTYPE_SHIFT       24
#define BP_PINMUX_FNTYPE_MASK        (0xff << BP_PINMUX_FNTYPE_SHIFT)
#define BP_PINMUX_FNPORT_MASK        0xff
#define BP_PINMUX_FNTYPE_xMII        (1 << BP_PINMUX_FNTYPE_SHIFT)
#define BP_PINMUX_FNTYPE_PCM         (2 << BP_PINMUX_FNTYPE_SHIFT)
#define BP_PINMUX_FNTYPE_ZAR         (3 << BP_PINMUX_FNTYPE_SHIFT)
#define BP_PINMUX_FNTYPE_SATA        (4 << BP_PINMUX_FNTYPE_SHIFT)
#define BP_PINMUX_FNTYPE_DECT        (5 << BP_PINMUX_FNTYPE_SHIFT)
#define BP_PINMUX_FNTYPE_NAND        (6 << BP_PINMUX_FNTYPE_SHIFT)
#define BP_PINMUX_FNTYPE_DEFAULT     (7 << BP_PINMUX_FNTYPE_SHIFT)
#define BP_PINMUX_FNTYPE_IRQ         (8 << BP_PINMUX_FNTYPE_SHIFT)
#define BP_PINMUX_FNTYPE_HS_SPI      (9 << BP_PINMUX_FNTYPE_SHIFT)
#define BP_PINMUX_FNTYPE_APM         (10 << BP_PINMUX_FNTYPE_SHIFT)
#define BP_PINMUX_FNTYPE_I2S         (11 << BP_PINMUX_FNTYPE_SHIFT)
#define BP_PINMUX_FNTYPE_EMMC        (12 << BP_PINMUX_FNTYPE_SHIFT)
#define BP_PINMUX_FNTYPE_LPORT       (13 << BP_PINMUX_FNTYPE_SHIFT)

/* Information about DDR memory configuration */
#if defined(CONFIG_BCM96846) || defined(_BCM96846_) || defined(CONFIG_BCM96878) || defined(_BCM96878_)
#define BP_DDR_SUPPORT_2L_PCB               1
#endif

#if defined(CONFIG_BCM963178) || defined(_BCM963178_)
#define BP_DDR_SUPPORT_VTT                  1
/* 63178 board does not support this optoin */
#define BP_DDR_SUPPORT_VTT_DIS_PASVTERM     0
#endif

#define BP_DDR_SPEED_MASK               0x1f
#define BP_DDR_SPEED_SHIFT              0
#define BP_DDR_SPEED_SAFE               0
#define BP_DDR_SPEED_400_6_6_6          1
#define BP_DDR_SPEED_533_7_7_7          2
#define BP_DDR_SPEED_533_8_8_8          3
#define BP_DDR_SPEED_667_9_9_9          4
#define BP_DDR_SPEED_667_10_10_10       5
#define BP_DDR_SPEED_800_10_10_10       6
#define BP_DDR_SPEED_800_11_11_11       7
#define BP_DDR_SPEED_1067_11_11_11      8
#define BP_DDR_SPEED_1067_12_12_12      9
#define BP_DDR_SPEED_1067_13_13_13      10
#define BP_DDR_SPEED_1067_14_14_14      11
#define BP_DDR_SPEED_933_10_10_10       12
#define BP_DDR_SPEED_933_11_11_11       13
#define BP_DDR_SPEED_933_12_12_12       14
#define BP_DDR_SPEED_933_13_13_13       15
#define BP_DDR_SPEED_1067_15_15_15      16   /* For DDR4 */
#define BP_DDR_SPEED_1067_16_16_16      17   /* For DDR4 */
#define BP_DDR_SPEED_CUSTOM_1           27
#define BP_DDR_SPEED_CUSTOM_2           28
#define BP_DDR_SPEED_CUSTOM_3           29
#define BP_DDR_SPEED_CUSTOM_4           30

#define BP_DDR_SPEED_IS_1067(spd)       ((spd) == BP_DDR_SPEED_1067_11_11_11 ||\
                                         (spd) == BP_DDR_SPEED_1067_12_12_12 ||\
                                         (spd) == BP_DDR_SPEED_1067_13_13_13 ||\
                                         (spd) == BP_DDR_SPEED_1067_14_14_14 ||\
                                         (spd) == BP_DDR_SPEED_1067_15_15_15 ||\
                                         (spd) == BP_DDR_SPEED_1067_16_16_16)

#define BP_DDR_SPEED_IS_800(spd)        ((spd) == BP_DDR_SPEED_800_10_10_10 ||\
                                         (spd) == BP_DDR_SPEED_800_11_11_11)


#define BP_DDR_DEVICE_WIDTH_MASK        0xe0
#define BP_DDR_DEVICE_WIDTH_SHIFT       5
#define BP_DDR_DEVICE_WIDTH_8           (0 << BP_DDR_DEVICE_WIDTH_SHIFT)
#define BP_DDR_DEVICE_WIDTH_16          (1 << BP_DDR_DEVICE_WIDTH_SHIFT)
#define BP_DDR_DEVICE_WIDTH_32          (2 << BP_DDR_DEVICE_WIDTH_SHIFT)

#define BP_DDR_TOTAL_SIZE_MASK          0xf00
#define BP_DDR_TOTAL_SIZE_SHIFT         8
#define BP_DDR_TOTAL_SIZE_64MB          (1 << BP_DDR_TOTAL_SIZE_SHIFT)
#define BP_DDR_TOTAL_SIZE_128MB         (2 << BP_DDR_TOTAL_SIZE_SHIFT)
#define BP_DDR_TOTAL_SIZE_256MB         (3 << BP_DDR_TOTAL_SIZE_SHIFT)
#define BP_DDR_TOTAL_SIZE_512MB         (4 << BP_DDR_TOTAL_SIZE_SHIFT)
#define BP_DDR_TOTAL_SIZE_1024MB        (5 << BP_DDR_TOTAL_SIZE_SHIFT)
#define BP_DDR_TOTAL_SIZE_2048MB        (6 << BP_DDR_TOTAL_SIZE_SHIFT)
#define BP_DDR_TOTAL_SIZE_4096MB        (7 << BP_DDR_TOTAL_SIZE_SHIFT)

#define BP_DDR_SSC_CONFIG_MASK          0xf000
#define BP_DDR_SSC_CONFIG_SHIFT         12
#define BP_DDR_SSC_CONFIG_NONE          (0 << BP_DDR_SSC_CONFIG_SHIFT)
#define BP_DDR_SSC_CONFIG_1             (1 << BP_DDR_SSC_CONFIG_SHIFT)
#define BP_DDR_SSC_CONFIG_2             (2 << BP_DDR_SSC_CONFIG_SHIFT)
#define BP_DDR_SSC_CONFIG_CUSTOM        (3 << BP_DDR_SSC_CONFIG_SHIFT)

#define BP_DDR_TEMP_MASK                0x30000
#define BP_DDR_TEMP_SHIFT               16
#define BP_DDR_TEMP_NORMAL              (0 << BP_DDR_TEMP_SHIFT)   /* Self-Refresh for Normal Temperature */
#define BP_DDR_TEMP_EXTENDED_SRT        (1 << BP_DDR_TEMP_SHIFT)   /* Self-Refresh for Extended Temperature */
#define BP_DDR_TEMP_EXTENDED_ASR        (2 << BP_DDR_TEMP_SHIFT)   /* Auto Self-Refresh Enabled for Normal and Extended Temperature */

#define BP_DDR_TOTAL_WIDTH_MASK         0xc0000
#define BP_DDR_TOTAL_WIDTH_SHIFT        18
#define BP_DDR_TOTAL_WIDTH_16BIT        (0 << BP_DDR_TOTAL_WIDTH_SHIFT)
#define BP_DDR_TOTAL_WIDTH_32BIT        (1 << BP_DDR_TOTAL_WIDTH_SHIFT)
#define BP_DDR_TOTAL_WIDTH_8BIT         (2 << BP_DDR_TOTAL_WIDTH_SHIFT)

#if defined(_BCM947189_) || defined(CONFIG_BCM947189)
/*only meaningful for 47189*/
/*ddr enable half TREF */
#define BP_DDR_HALF_TREF_MASK           0x100000
#define BP_DDR_HALF_TREF_SHIFT          20
#define BP_DDR_HALF_TREF                (1 << BP_DDR_HALF_TREF_SHIFT)

/*ddr rRFC */
#define BP_DDR_TRFC_MASK                0xe00000
#define BP_DDR_TRFC_SHIFT               21
#define BP_DDR_TRFC_90NS                (1 << BP_DDR_TRFC_SHIFT) 
#define BP_DDR_TRFC_110NS               (2 << BP_DDR_TRFC_SHIFT) 
#define BP_DDR_TRFC_160NS               (3 << BP_DDR_TRFC_SHIFT) 
#define BP_DDR_TRFC_260NS               (4 << BP_DDR_TRFC_SHIFT) 
#define BP_DDR_TRFC_350NS               (5 << BP_DDR_TRFC_SHIFT) 
#else
#define BP_DDR_TYPE_MASK                0x300000
#define BP_DDR_TYPE_SHIFT               20
#define BP_DDR_TYPE_DDR3                (0 << BP_DDR_TYPE_SHIFT)
#define BP_DDR_TYPE_DDR4                (1 << BP_DDR_TYPE_SHIFT)
#endif

/* Vtt termination settings. Vtt termination is required in the board design for 
   Address and Control line to improve the signal integrity when it need to support
   DDR at high clock. For slow DDR, Vtt termination is not used. It can be no termination 
   at all with direct connection through serial resistor from DDR chip to the PHY or 
   with passive terminatin of a pull-up and pull-down resistors.

   In low-end 2 layer board design, Vtt usually is not used as DDR runs slow so PCB_2LAYER
   option is essetnially Vtt dsiabled. But Vtt option is not determined by the layer of PCB. 
   Keep this PCB definition only for compatiblity reason and new design should use VTT setting.
 */
#if defined(BP_DDR_SUPPORT_2L_PCB)
#define BP_DDR_PCB_MASK                 0x20000000
#define BP_DDR_PCB_SHIFT                29
#define BP_DDR_PCB_MULTI_LAYER          (0 << BP_DDR_PCB_SHIFT)
#define BP_DDR_PCB_2LAYER               (1 << BP_DDR_PCB_SHIFT)   
#endif

#if defined(BP_DDR_SUPPORT_VTT)
#define BP_DDR_VTT_MASK                 0x30000000
#define BP_DDR_VTT_SHIFT                28
#define BP_DDR_VTT_EN                   (0 << BP_DDR_VTT_SHIFT)  /* Vtt enabled */
#define BP_DDR_VTT_DIS_NOTERM           (2 << BP_DDR_VTT_SHIFT)  /* Vtt disabled with no AC termination */
#if defined(BP_DDR_SUPPORT_VTT_DIS_PASVTERM)
#define BP_DDR_VTT_DIS_PASVTERM         (1 << BP_DDR_VTT_SHIFT)  /* Vtt disabled with passive AC termination */
#endif
#endif

#define BP_DDR_CONFIG_MASK              (~(BP_DDR_CONFIG_DEBUG|BP_DDR_CONFIG_OVERRIDE))
#define BP_DDR_CONFIG_DEBUG             (1 << 30)
#define BP_DDR_CONFIG_OVERRIDE          (1 << 31)

#define BP_MAX_ATTACHED_PORTS          16

#define BP_BTN_ID_NONE              0xFF

#define BP_BTN_TRIG_TYPE_MASK       0xC000
#define BP_BTN_TRIG_PRESS           0x0000
#define BP_BTN_TRIG_HOLD            0x4000
#define BP_BTN_TRIG_RELEASE         0x8000

#define BP_BTN_ACTION_MASK                  0x3F00
#define BP_BTN_ACTION_SHIFT                 8

#define BP_BTN_ACTION_NONE                  (0 << BP_BTN_ACTION_SHIFT)
#define BP_BTN_ACTION_SES                   (1 << BP_BTN_ACTION_SHIFT)
#define BP_BTN_ACTION_PLC_UKE               (2 << BP_BTN_ACTION_SHIFT)
#define BP_BTN_ACTION_RANDOMIZE_PLC         (3 << BP_BTN_ACTION_SHIFT)
#define BP_BTN_ACTION_RESTORE_DEFAULTS      (4 << BP_BTN_ACTION_SHIFT)
#define BP_BTN_ACTION_RESET                 (5 << BP_BTN_ACTION_SHIFT)
#define BP_BTN_ACTION_PRINT                 (6 << BP_BTN_ACTION_SHIFT)
#define BP_BTN_ACTION_WLAN_DOWN             (7 << BP_BTN_ACTION_SHIFT)

#define SES_BTN_PARAM_AP                    0
#define SES_BTN_PARAM_STA                   1


#define BP_BTN_TRIG_TIME_MASK                   0x00FF
#define BP_BTN_TRIG_TIME_UNIT_IN_MS             100

#define BP_BTN_TRIG_0S                          (0 * (1000/BP_BTN_TRIG_TIME_UNIT_IN_MS))
#define BP_BTN_TRIG_1S                          (1 * (1000/BP_BTN_TRIG_TIME_UNIT_IN_MS))
#define BP_BTN_TRIG_2S                          (2 * (1000/BP_BTN_TRIG_TIME_UNIT_IN_MS))
#define BP_BTN_TRIG_3S                          (3 * (1000/BP_BTN_TRIG_TIME_UNIT_IN_MS))
#define BP_BTN_TRIG_4S                          (4 * (1000/BP_BTN_TRIG_TIME_UNIT_IN_MS))
#define BP_BTN_TRIG_5S                          (5 * (1000/BP_BTN_TRIG_TIME_UNIT_IN_MS))
#define BP_BTN_TRIG_6S                          (6 * (1000/BP_BTN_TRIG_TIME_UNIT_IN_MS))
#define BP_BTN_TRIG_7S                          (7 * (1000/BP_BTN_TRIG_TIME_UNIT_IN_MS))
#define BP_BTN_TRIG_8S                          (8 * (1000/BP_BTN_TRIG_TIME_UNIT_IN_MS))
#define BP_BTN_TRIG_9S                          (9 * (1000/BP_BTN_TRIG_TIME_UNIT_IN_MS))
#define BP_BTN_TRIG_10S                         (10 * (1000/BP_BTN_TRIG_TIME_UNIT_IN_MS))
#define BP_BTN_TRIG_20S                         (20 * (1000/BP_BTN_TRIG_TIME_UNIT_IN_MS))

#define BP_PCI0_DUAL_LANE                       0x1

/* Information about an Ethernet MAC.  If ucPhyType is BP_ENET_NO_PHY,
 * then the other fields are not valid.
 */
typedef struct EthernetMacInfo
{
    unsigned char ucPhyType;                    /* BP_ENET_xxx             */
    unsigned char ucPhyAddress;                 /* 0 to 31                 */
    unsigned short usConfigType;                /* Configuration type */
    ETHERNET_SW_INFO sw;                        /* switch information */
    unsigned short usGpioMDC;                   /* GPIO pin to simulate MDC */
    unsigned short usGpioMDIO;                  /* GPIO pin to simulate MDIO */
} ETHERNET_MAC_INFO, *PETHERNET_MAC_INFO;

typedef struct
{
  unsigned int port_map;
  unsigned int ports[BP_MAX_ATTACHED_PORTS];
  char *devnames[BP_MAX_ATTACHED_PORTS];
} BP_ATTACHED_INFO;

typedef struct WlanSromEntry {
    unsigned short wordOffset;
    unsigned short value;
} WLAN_SROM_ENTRY;

typedef struct WlanSromPatchInfo {
    char szboardId[BP_BOARD_ID_LEN];
    unsigned short usWirelessChipId;
    unsigned short usNeededSize;
    WLAN_SROM_ENTRY *uniqueEntries;
    WLAN_SROM_ENTRY *commonEntries;
} WLAN_SROM_PATCH_INFO, *PWLAN_SROM_PATCH_INFO;

typedef struct WlanPciEntry {
    char name[BP_WLAN_NVRAM_NAME_LEN];
    unsigned int dwordOffset;
    unsigned int value;
} WLAN_PCI_ENTRY;

typedef struct WlanPciPatchInfo {
    char szboardId[BP_BOARD_ID_LEN];
    unsigned int usWirelessPciId;
    int usNeededSize;
    WLAN_PCI_ENTRY entries[BP_WLAN_MAX_PATCH_ENTRY];
} WLAN_PCI_PATCH_INFO, *PWLAN_PCI_PATCH_INFO;

/* Information about VoIP DSPs.  If ucDspType is BP_VOIP_NO_DSP,
 * then the other fields are not valid.
 */
typedef struct VoIPDspInfo
{
    unsigned char  ucDspType;
    unsigned char  ucDspAddress;
    unsigned short usGpioLedVoip;
    unsigned short usGpioVoip1Led;
    unsigned short usGpioVoip1LedFail;
    unsigned short usGpioVoip2Led;
    unsigned short usGpioVoip2LedFail;
    unsigned short usGpioPotsLed;
    unsigned short usGpioDectLed;

} VOIP_DSP_INFO;

/* GPIOs used to reset PHY, board
   also, GPIOs used for USB overcurrent
*/

typedef struct {
  unsigned char gpio_for_oc_detect;
  unsigned char gpio_for_oc_output;
} GPIO_USB_INFO;

/* xdsl distpoint hardware related information
 */
#define BP_XDSL_DISTPOINT_MAX_SPI_SLAVE 8
#define BP_XDSL_DISTPOINT_MAX_GPIO 16
#define BP_XDSL_DISTPOINT_MAX_RESET 8

typedef struct {
    unsigned short busNum;
    unsigned short mode;
    unsigned int ctrlState;
    unsigned int maxFreq;
    unsigned short protoRev;
    unsigned short nbSlaves;
    unsigned short selectNum[BP_XDSL_DISTPOINT_MAX_SPI_SLAVE];
    unsigned short reset[BP_XDSL_DISTPOINT_MAX_SPI_SLAVE];
} XDSL_DISTPOINT_SPI_SLAVE_INFO;

typedef struct {
    unsigned short gpio;
    char *name;
    unsigned short releaseOnInit;
} XDSL_DISTPOINT_RESET_INFO;

typedef struct {
    unsigned short gpio;
    unsigned short initValue;
    char *info;
    char *infoValue0;
    char *infoValue1;
} XDSL_DISTPOINT_GPIO_INFO;

typedef struct {
    unsigned short nbReset;
    unsigned short nbGpio;
    XDSL_DISTPOINT_SPI_SLAVE_INFO spi;
    XDSL_DISTPOINT_RESET_INFO reset[BP_XDSL_DISTPOINT_MAX_RESET];
    XDSL_DISTPOINT_GPIO_INFO gpio[BP_XDSL_DISTPOINT_MAX_GPIO];
} XDSL_DISTPOINT_INFO, *PXDSL_DISTPOINT_INFO;

/* Defines for LED LPORT configuration */
#define BP_NET_LED_SPEED_SHIFT 0
#define BP_NET_LED_SPEED_MASK (0xffff << (BP_NET_LED_SPEED_SHIFT))
#define BP_NET_LED_SPEED_10   (1 << (BP_NET_LED_SPEED_SHIFT))
#define BP_NET_LED_SPEED_100  (1 << (BP_NET_LED_SPEED_SHIFT + 1))
#define BP_NET_LED_SPEED_1G   (1 << (BP_NET_LED_SPEED_SHIFT + 2))
#define BP_NET_LED_SPEED_2500 (1 << (BP_NET_LED_SPEED_SHIFT + 3))
#define BP_NET_LED_SPEED_10G  (1 << (BP_NET_LED_SPEED_SHIFT + 4))

#define BP_NET_LED_SPEED_FAE  (BP_NET_LED_SPEED_10 | BP_NET_LED_SPEED_100)
#define BP_NET_LED_SPEED_GBE  (BP_NET_LED_SPEED_FAE | BP_NET_LED_SPEED_1G)

/* To avoid conflicting with BP_NOT_DEFINED, use the actual bit definition */
#define BP_NET_LED_SPEED_ALL  (BP_NET_LED_SPEED_GBE|BP_NET_LED_SPEED_2500|BP_NET_LED_SPEED_10G)

#define BP_NET_LED_ACTIVITY_ALL  (BP_NET_LED_SPEED_ALL) 

#define BP_NET_LED_SETTINGS_SHIFT 0
#define BP_NET_LED_SETTINGS_MASK (0x3 << (BP_NET_LED_SETTINGS_SHIFT))
#define BP_NET_LED_SETTINGS_ACT_RX   (1 << (BP_NET_LED_SETTINGS_SHIFT))
#define BP_NET_LED_SETTINGS_ACT_TX   (1 << (BP_NET_LED_SETTINGS_SHIFT + 1))
#define BP_NET_LED_SETTINGS_ACT_ALL  (BP_NET_LED_SETTINGS_ACT_RX|BP_NET_LED_SETTINGS_ACT_TX)

extern WLAN_SROM_PATCH_INFO wlanPaInfo[];
extern WLAN_PCI_PATCH_INFO wlanPciInfo[];

/***********************************************************************
 * SMP locking notes for BoardParm functions
 *
 * No locking is needed for any of these boardparm functions as long
 * as the following conditions/assumptions are not violated.
 *
 * 1. Initialization functions such as BpSetBoardId() are only called
 *    during startup when no other BoardParam functions are in progress.
 *    BpSetBoardId() modifies the internal global pointer g_CurrentBp,
 *    which other functions deference multiple times inside their
 *    functions.  So if g_CurrentBp changes in the middle of a function,
 *    inconsistent data could be returned.
 *    Actually, BpSetBoardId is also called when cfe or whole image is
 *    being written to flash, but this is when system is about to shut
 *    down, so should also be OK.
 *
 * 2. Callers to functions which return a pointer to the boardparm data
 *    (currently there is only 1: BpGetVoipDspConfig) should not modify
 *    the boardparm data.  All other functions are well written
 *    in this regard, they only return a copy of the requested data and
 *    not a pointer to the data itself.
 *
 *
 ************************************************************************/

int BpSetBoardId(const char *pszBoardId );
int BpGetBoardId( char *pszBoardId);
char * BpGetBoardIdNameByIndex( int i );
int BpGetBoardIds( char *pszBoardIds, int nBoardIdsSize );
int BPGetNumBoardIds(void);


int BpGetComment( char **pcpValue );

int BpGetGPIOverlays( unsigned int *pusValue );

int BpGetRj11InnerOuterPairGpios( unsigned short *pusInner, unsigned short *pusOuter );
int BpGetRtsCtsUartGpios( unsigned short *pusRts, unsigned short *pusCts );

int BpGetAdslLedGpio( unsigned short *pusValue );
int BpGetAdslFailLedGpio( unsigned short *pusValue );
int BpGetSecAdslLedGpio( unsigned short *pusValue );
int BpGetSecAdslFailLedGpio( unsigned short *pusValue );
int BpGetPwrSyncGpio( unsigned short *pusValue );
int BpGetVregSyncGpio( unsigned short *pusValue );
int BpGetWirelessSesLedGpio( unsigned short *pusValue );
int BpGetWanDataLedGpio( unsigned short *pusValue );
int BpGetSecWanDataLedGpio( unsigned short *pusValue );
int BpGetWanErrorLedGpio( unsigned short *pusValue );
int BpGetBootloaderPowerOnLedGpio( unsigned short *pusValue );
int BpGetBootloaderPowerOnLedBlinkTimeOn( unsigned int *pulValue );
int BpGetBootloaderPowerOnLedBlinkTimeOff( unsigned int *pulValue );
int BpGetBootloaderStopLedGpio( unsigned short *pusValue );
int BpGetPassDyingGaspGpio( unsigned short *pusValue );
int BpGetDyingGaspIntrPin( unsigned int *pusValue );
int BpGetFpgaResetGpio( unsigned short *pusValue );
int BpGetGponLedGpio( unsigned short *pusValue );
int BpGetGponFailLedGpio( unsigned short *pusValue );
int BpGetOpticalLinkFailLedGpio( unsigned short *pusValue );
int BpGetUSBLedGpio( unsigned short *pusValue );
int BpGetMoCALedGpio( unsigned short *pusValue );
int BpGetMoCAFailLedGpio( unsigned short *pusValue );
int BpGetEponLedGpio( unsigned short *pusValue );
int BpGetEponFailLedGpio( unsigned short *pusValue );
int BpGetAggregateLnkLedGpio( unsigned short *pusValue );
int BpGetAggregateActLedGpio( unsigned short *pusValue );
int BpGetPLCResetGpio( unsigned short *pusValue );
int BpGetPLCPwrEnGpio( unsigned short *pusValue );

int BpGetPLCStandByExtIntr( unsigned short *pusValue );
int BpGetResetToDefaultExtIntr( unsigned short *pusValue );
int BpGetResetToDefault2ExtIntr( unsigned short *pusValue );
int BpGetButtonInfo( void          **token,
                     unsigned short *pusIdx, 
                     unsigned short *pusGpio,  
                     unsigned short *pusExtIrq,
                     unsigned short *pusNumHooks,
                     unsigned short *pausHooks,
                     void  **pausHookParms
                     );
int BpGetButtonInfoByIdx(   unsigned short btnIdx,
                            unsigned short *pusIdx, 
                            unsigned short *pusGpio,  
                            unsigned short *pusExtIrq,
                            unsigned short *pusNumHooks,
                            unsigned short *pausHooks,
                            void  **pausHookParms
                            );


int BpGetWirelessSesExtIntr( unsigned short *pusValue );
int BpGetNfcExtIntr( unsigned short *pusValue );
int BpGetNfcPowerGpio( unsigned short *pusValue );
int BpGetNfcWakeGpio( unsigned short *pusValue );
int BpGetBitbangSclGpio( unsigned short *pusValue );
int BpGetBitbangSdaGpio( unsigned short *pusValue );
int BpGetBtResetGpio( unsigned short *pusValue );
int BpGetBtWakeGpio( unsigned short *pusValue );
int BpGetResetToDefaultExtIntrGpio( unsigned short *pusValue );
int BpGetResetToDefault2ExtIntrGpio( unsigned short *pusValue );
int BpGetOpticalModuleTxPwrDownGpio( unsigned short *pusValue );
int BpGetWirelessSesExtIntrGpio( unsigned short *pusValue );
#if !defined(_CFE_)
int BpGetMocaInfo( PBP_MOCA_INFO pMocaInfos, int* pNumEntry );
#endif
int BpGetWirelessAntInUse( unsigned short *pusValue );
int BpGetWirelessFlags( unsigned short *pusValue );
int BpGetWirelessPowerDownGpio( unsigned short *pusValue );
int BpUpdateWirelessSromMap(unsigned short chipID, unsigned short* pBase, int sizeInWords);
int BpUpdateWirelessPciConfig (unsigned int pciID, unsigned int* pBase, int sizeInDWords);

int BpGetEthernetMacInfo( PETHERNET_MAC_INFO pEnetInfos, int nNumEnetInfos );
const ETHERNET_MAC_INFO* BpGetEthernetMacInfoArrayPtr(void);
int BpGetPortConnectedToExtSwitch(void);
int BpGet6829PortInfo( unsigned char *portInfo6829 );
int BpGetDslPhyAfeIds( unsigned int *pulValues );
int BpGetDslPhyAfeIdByIntfIdx( int intfId, unsigned int *pulValues );
#ifdef CONFIG_BP_PHYS_INTF
int BpGetAFELDPwrBoostGpio( int intfIdx, unsigned short *pusValue );
int BpGetAFELDRelayGpio( int intfIdx, unsigned short *pusValue );
int BpGetAFEVR5P3PwrEnGpio( int intfIdx, unsigned short *pusValue );
int BpGetOpticalModulePresenceExtIntrGpio( unsigned short type, int intfIdx, unsigned short *pusValue );
int BpGetOpticalModulePresenceExtIntr( unsigned short type, int intfIdx, unsigned short *pusValue );
#else
int BpGetAFELDPwrBoostGpio( unsigned short *pusValue );
int BpGetAFELDRelayGpio( unsigned short *pusValue );
int BpGetAFEVR5P3PwrEnGpio( unsigned short *pusValue );
int BpGetOpticalModulePresenceExtIntrGpio( unsigned short *pusValue );
int BpGetOpticalModulePresenceExtIntr( unsigned short *pusValue );
#endif
int BpGetI2cDefXponBus( unsigned short *pusValue );
int BpGetExtAFEResetGpio( unsigned short *pulValues );
int BpGetExtAFELDPwrGpio( unsigned short *pulValues );
int BpGetExtAFELDModeGpio( unsigned short *pulValues );
int BpGetIntAFELDPwrGpio( unsigned short *pusValue );
int BpGetIntAFELDModeGpio( unsigned short *pulValues );
int BpGetExtAFELDDataGpio( unsigned short *pusValue );
int BpGetExtAFELDClkGpio( unsigned short *pusValue );
int BpGetExtAFELDPwrDslCtl( unsigned short *pulValues );
int BpGetExtAFELDModeDslCtl( unsigned short *pulValues );
int BpGetIntAFELDPwrDslCtl( unsigned short *pusValue );
int BpGetIntAFELDModeDslCtl( unsigned short *pulValues );
int BpGetIntAFELDDataDslCtl( unsigned short *pusValue );
int BpGetIntAFELDClkDslCtl( unsigned short *pusValue );
int BpGetExtAFELDDataDslCtl( unsigned short *pusValue );
int BpGetExtAFELDClkDslCtl( unsigned short *pusValue );
int BpGetUart2SdoutGpio( unsigned short *pusValue );
int BpGetUart2SdinGpio( unsigned short *pusValue );
int BpGetSerialLedData( unsigned short *pusValue );
int BpGetSerialLedShiftOrder( unsigned short *pusValue );


int BpGetEthSpdLedGpio( unsigned short port, unsigned short enetIdx,
                         unsigned short ledIdx, unsigned short *pusValue );

VOIP_DSP_INFO *BpGetVoipDspConfig( unsigned char dspNum );
int BpGetVoipLedGpio( unsigned short *pusValue );
int BpGetVoip1LedGpio( unsigned short *pusValue );
int BpGetVoip1FailLedGpio( unsigned short *pusValue );
int BpGetVoip2LedGpio( unsigned short *pusValue );
int BpGetVoip2FailLedGpio( unsigned short *pusValue );
int BpGetPotsLedGpio( unsigned short *pusValue );
int BpGetDectLedGpio( unsigned short *pusValue );

int BpGetLaserDisGpio( unsigned short *pusValue );
int BpGetLaserTxPwrEnGpio( unsigned short *pusValue );
int BpGetLaserResetGpio( unsigned short *pusValue );
int BpGetEponOpticalSDGpio( unsigned short *pusValue );
int BpGetVregSel1P2( unsigned short *pusValue );
int BpGetVreg1P8( unsigned char *pucValue );
int BpGetVregAvsMin( unsigned short *pusValue );
int BpGetMiiOverGpioFlag( unsigned int* pMiiOverGpioFlag );
int BpGetFemtoResetGpio( unsigned short *pusValue );
int BpGetEphyBaseAddress( unsigned short *pusValue );
int BpGetGphyBaseAddress( unsigned short *pusValue );

int bpstrcmp(const char *dest,const char *src);
int BpGetI2cGpios( unsigned short *pusScl, unsigned short *pusSda );
int BpGetSgmiiGpios( unsigned short *sgmiiGpio);
int BpGetSfpDetectGpio( unsigned short *sfpDetectGpio);
int BpGetPortMacType(unsigned short port, unsigned int *pulValue );
int BpGetNumFePorts( unsigned int *pulValue );
int BpGetNumGePorts( unsigned int *pulValue );
int BpGetNumVoipPorts( unsigned int *pulValue );
int BpGetSwitchPortMap (unsigned int *pulValue);

int BpGetSpiSlaveResetGpio( unsigned short *pusValue );
int BpGetSpiSlaveBootModeGpio( unsigned short *pusValue );
int BpGetSpiSlaveBusNum( unsigned short *pusValue );
int BpGetSpiSlaveSelectNum( unsigned short *pusValue );
int BpGetSpiSlaveMode( unsigned short *pusValue );
int BpGetSpiSlaveCtrlState( unsigned int *pulValue );
int BpGetSpiSlaveMaxFreq( unsigned int *pulValue );
int BpGetSpiSlaveProtoRev( unsigned short *pusValue );
int BpGetSimInterfaces( unsigned short *pusValue );
int BpGetSlicInterfaces( unsigned short *pusValue );
int BpGetAePolarity( unsigned short *pusValue );
int BpGetPonTxEnGpio( unsigned short *pusValue );
int BpGetPonRxEnGpio( unsigned short *pusValue );
int BpGetPonResetGpio( unsigned short *pusValue );
int BpGetRogueOnuEn( unsigned short *pusValue );
int BpGetGpioLedSim( unsigned short *pusValue );
int BpGetGpioLedSimITMS( unsigned short *pusValue );

int BpGetSerialLEDMuxSel( unsigned short *pusValue );
int BpGetDeviceOptions( unsigned int *pulValue );

int BpGetGpioGpio(int idx, void** token, unsigned short *pusValue);
int BpGetLedGpio(int idx, void** token,  unsigned short *pusValue);
int BpGetExtIntrNumGpio(int idx, void** token, unsigned short *pusExtInt, unsigned short *pusGpio);
int BpCheckExtIntr(int ext_int_num, int gpio, int first);
int BpIsGpioInUse(unsigned short gpio);
int BpGetEponGpio(int idx, unsigned short *pusValue);

int BpGetPhyResetGpio(int unit, int port, unsigned short *pusValue);
int BpGetBoardResetGpio(unsigned short *pusValue);
int BpGetUSBGpio(int usb, GPIO_USB_INFO *gpios);
int BpGetPhyAddr(int unit, int port);
int BpGetOpticalWan( unsigned int *pulValue );
int BpGetLedPinMuxGpio(int idx, unsigned short *pusValue);

int BpGetTsync1025mhzPin( unsigned short *pusValue );
int BpGetTsync8khzPin( unsigned short *pusValue );
int BpGetTsync1ppsPin( unsigned short *pusValue );
int BpGetTsyncPonUnstableGpio( unsigned short *pusValue );

int BpGetAllPinmux(int maxnum, int *outcnt, int *errcnt, unsigned short *pusFunction, unsigned int *pulMuxInfo);
int BpGetIfacePinmux(unsigned int interface, int maxnum, int *outcnt, int *errcnt, unsigned short *pusFunction, unsigned int *pulMuxInfo);
int BpGetMemoryConfig( unsigned int *pulValue );
int BpGetBatteryEnable( unsigned short *pusValue );

int BpGetPmdMACEwakeEn( unsigned short *pusValue );
int BpGetPmdAlarmExtIntr( unsigned short *pusValue );
int BpGetPmdInvSerdesRxPol( unsigned short *pusValue );
int BpGetPmdInvSerdesTxPol( unsigned short *pusValue );
int BpGetWanSignalDetectedExtIntr( unsigned short *pusValue );
int BpGetPmdAlarmExtIntrGpio( unsigned short *pusValue );
int BpGetWanSignalDetectedExtIntrGpio( unsigned short *pusValue );
int BpGetGpioPmdReset( unsigned short *pusValue );
int BpGetPmdFunc( unsigned short *pusValue );

int BpGetAttachedInfo(int attached_port_idx, BP_ATTACHED_INFO *bp_attached_info);
int BpGetTrplxrTxFailExtIntr( unsigned short *pusValue );
int BpGetTrplxrTxFailExtIntrGpio( unsigned short *pusValue );
int BpGetTrplxrSdExtIntr( unsigned short *pusValue );
int BpGetTrplxrSdExtIntrGpio( unsigned short *pusValue );
int BpGetTxLaserOnOutN( unsigned short *pusValue );
int BpGet1ppsStableGpio( unsigned short *pusValue );
int BpGetLteResetGpio( unsigned short *pusValue );
int BpGetStrapTxEnGpio( unsigned short *pusValue );
int BpGetWanNco10M( unsigned short *pusValue );
int BpGetTrxSignalDetect( unsigned short *pusValue );
int BpGetWifiOnOffExtIntr( unsigned short *pusValue );
int BpGetWifiOnOffExtIntrGpio( unsigned short *pusValue );
int BpGetLteExtIntr( unsigned short *pusValue );
int BpGetLteExtIntrGpio( unsigned short *pusValue );

int BpGetSDCardDetectExtIntrGpio( unsigned short *pusValue );
int BpGetSDCardDetectExtIntr( unsigned short *pusValue );

int BpGetGpioSpromClk( unsigned short *pusValue );
int BpGetGpioSpromData( unsigned short *pusValue );
int BpGetGpioSpromRst( unsigned short *pusValue );
int BpGetGpioAttachedDevReset( unsigned short *pusValue );
int BpGetXdslDistpointInfo(PXDSL_DISTPOINT_INFO pXdslDistpointInfo);
int BpGetMiiInterfaceEn( unsigned short *pusValue );

int BpGetWanSignalDetectedGpio( unsigned short *pusValue );

int BpGetUsbPwrOn0( unsigned short *pusValue );
int BpGetUsbPwrOn1( unsigned short *pusValue );
int BpGetUsbPwrFlt0( unsigned short *pusValue );
int BpGetUsbPwrFlt1( unsigned short *pusValue );
int BpGetMaxNumCpu( unsigned int *pulValue );
int BpGetDHDMemReserve( int index, unsigned char *pucValue );

int BpEnumCompatChipId( void** token, unsigned int *pulValue );
int BpEnumGpioInitState( void** token, unsigned short *pusValue );

#if !defined(_CFE_)
#if defined(CONFIG_NEW_LEDS)
int BpGetLedName(int idx, void** token,  unsigned short *pusValue, char **ledName);
#endif
#endif
int BpGetTxDisGpio( int lane, unsigned short *pusValue );
int BpGetLedsAdvancedInfo(LEDS_ADVANCED_INFO *pLedsInfo);
int BpGetPciPortDualLane( int port, int *enabled ); 

int BpGetUsbDis(unsigned short *pusValue);
int BpGetPciDis(unsigned short *pusValue);
int BpGetSataDis(unsigned short *pusValue);

int BpGetWirelessDisGpio(int index, unsigned short *pusValue);

#ifdef CONFIG_BP_PHYS_INTF
int BpInitPhyIntfInfo(void);
PHYS_INTF_INFO* BpGetAllPhyIntfInfo(int *totalIntfNum);
PHYS_INTF_INFO* BpGetPhyIntfInfo(int intfIdx);
PHYS_INTF_INFO* BpGetPhyIntfInfoByType(unsigned short type, int intfIdx);
int BpHasEthWanIntf(void);
unsigned short BpGetPhyIntfNumByType(unsigned short type);
int BpGetAFELDClkGpio( int portIdx, unsigned short *pusValue );
int BpGetAFELDModeGpio( int intfIdx, unsigned short *pusValue );
int BpGetAFELDDataGpio( int intfIdx, unsigned short *pusValue );
int BpGetAFELDPwrGpio( int intfIdx, unsigned short *pusValue );
int BpGetAFELDClkDslCtl( int portIdx, unsigned short *pusValue );
int BpGetAFELDModeDslCtl( int intfIdx, unsigned short *pusValue );
int BpGetAFELDPwrDslCtl( int intfIdx, unsigned short *pusValue );
int BpGetAFELDDataDslCtl( int intfIdx, unsigned short *pusValue );
int BpGetAFEResetGpio( int intfIdx, unsigned short *pusValue );
int BpGetWanActLedGpio(int type, int intfIdx, unsigned short *pusValue);
int BpGetWanErrLedGpio(int type, int intfIdx, unsigned short *pusValue);
int BpGetWanLinkLedGpio(int type, int intfIdx, unsigned short *pusValue);
int BpGetWanLinkFailLedGpio(int type, int intfIdx, unsigned short *pusValue);
int BpGetSfpModDetectGpio(int type, int intfIdx, unsigned short *pusValue);
int BpGetSfpSigDetect(int type, int intfIdx, unsigned short *pusValue);
int BpGetIntfPortNum(int type, int intfIdx, unsigned short *pusValue);
int BpGetIntfMgmtType(int type, int intfIdx, unsigned short *pusValue);
int BpGetIntfMgmtBusNum(int type, int intfIdx, unsigned short *pusValue);
int BpGetAllAdvLedInfo(PHYS_INTF_ADV_LEDS_INFO* ledInfo, int* numEntry);
int BpGetAdvLedInfo(unsigned short type, int intfIdx, PHYS_INTF_ADV_LEDS_INFO* ledInfo);
#endif

#if defined(_BCM947189_) || defined(CONFIG_BCM947189)
int BpGetMoCAResetGpio( unsigned short *pusValue );
int BpGetSpiClkGpio( unsigned short *pusValue );
int BpGetSpiCsGpio( unsigned short *pusValue );
int BpGetSpiMisoGpio( unsigned short *pusValue );
int BpGetSpiMosiGpio( unsigned short *pusValue );
#endif 

int BpGetWL0ActLedGpio( unsigned short *pusValue );
int BpGetWL1ActLedGpio( unsigned short *pusValue );

#endif /* __ASSEMBLER__ */

#ifdef __cplusplus
}
#endif

#endif /* _BOARDPARMS_H */

