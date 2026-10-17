/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright 2016 Intel Corporation
 * Copyright 2020 Google LLC
 *
 * Taken from coreboot apl gpe.h
 */

#ifndef _ASM_ARCH_GPE_H_
#define _ASM_ARCH_GPE_H_

/* bit position in GPE0a_STS register */
#define GPE0A_PCIE_SCI_STS		0
#define GPE0A_SWGPE_STS			2
#define GPE0A_PCIE_WAKE0_STS		3
#define GPE0A_PUNIT_SCI_STS		4
#define GPE0A_PCIE_WAKE1_STS		6
#define GPE0A_PCIE_WAKE2_STS		7
#define GPE0A_PCIE_WAKE3_STS		8
#define GPE0A_PCIE_GPE_STS		9
#define GPE0A_BATLOW_STS		10
#define GPE0A_CSE_PME_STS		11
#define GPE0A_XDCI_PME_STS		12
#define GPE0A_XHCI_PME_STS		13
#define GPE0A_AVS_PME_STS		14
#define GPE0A_GPIO_TIER1_SCI_STS	15
#define GPE0A_SMB_WAK_STS		16
#define GPE0A_SATA_PME_STS		17
#define GPE0A_CNVI_PME_STS	        18

/* Group DW0 is reserved in Apollolake */

/* GPE_63_32 */
#define GPE0_DW1_00		32
#define GPE0_DW1_01		33
#define GPE0_DW1_02		34
#define GPE0_DW1_03		36
#define GPE0_DW1_04		36
#define GPE0_DW1_05		37
#define GPE0_DW1_06		38
#define GPE0_DW1_07		39
#define GPE0_DW1_08		40
#define GPE0_DW1_09		41
#define GPE0_DW1_10		42
#define GPE0_DW1_11		43
#define GPE0_DW1_12		44
#define GPE0_DW1_13		45
#define GPE0_DW1_14		46
#define GPE0_DW1_15		47
#define GPE0_DW1_16		48
#define GPE0_DW1_17		49
#define GPE0_DW1_18		50
#define GPE0_DW1_19		51
#define GPE0_DW1_20		52
#define GPE0_DW1_21		53
#define GPE0_DW1_22		54
#define GPE0_DW1_23		55
#define GPE0_DW1_24		56
#define GPE0_DW1_25		57
#define GPE0_DW1_26		58
#define GPE0_DW1_27		59
#define GPE0_DW1_28		60
#define GPE0_DW1_29		61
#define GPE0_DW1_30		62
#define GPE0_DW1_31		63
/* GPE_95_64 */
#define GPE0_DW2_00		64
#define GPE0_DW2_01		65
#define GPE0_DW2_02		66
#define GPE0_DW2_03		67
#define GPE0_DW2_04		68
#define GPE0_DW2_05		69
#define GPE0_DW2_06		70
#define GPE0_DW2_07		71
#define GPE0_DW2_08		72
#define GPE0_DW2_09		73
#define GPE0_DW2_10		74
#define GPE0_DW2_11		75
#define GPE0_DW2_12		76
#define GPE0_DW2_13		77
#define GPE0_DW2_14		78
#define GPE0_DW2_15		79
#define GPE0_DW2_16		80
#define GPE0_DW2_17		81
#define GPE0_DW2_18		82
#define GPE0_DW2_19		83
#define GPE0_DW2_20		84
#define GPE0_DW2_21		85
#define GPE0_DW2_22		86
#define GPE0_DW2_23		87
#define GPE0_DW2_24		88
#define GPE0_DW2_25		89
#define GPE0_DW2_26		90
#define GPE0_DW2_27		91
#define GPE0_DW2_28		92
#define GPE0_DW2_29		93
#define GPE0_DW2_30		94
#define GPE0_DW2_31		95
/* GPE_127_96 */
#define GPE0_DW3_00		96
#define GPE0_DW3_01		97
#define GPE0_DW3_02		98
#define GPE0_DW3_03		99
#define GPE0_DW3_04		100
#define GPE0_DW3_05		101
#define GPE0_DW3_06		102
#define GPE0_DW3_07		103
#define GPE0_DW3_08		104
#define GPE0_DW3_09		105
#define GPE0_DW3_10		106
#define GPE0_DW3_11		107
#define GPE0_DW3_12		108
#define GPE0_DW3_13		109
#define GPE0_DW3_14		110
#define GPE0_DW3_15		111
#define GPE0_DW3_16		112
#define GPE0_DW3_17		113
#define GPE0_DW3_18		114
#define GPE0_DW3_19		115
#define GPE0_DW3_20		116
#define GPE0_DW3_21		117
#define GPE0_DW3_22		118
#define GPE0_DW3_23		119
#define GPE0_DW3_24		120
#define GPE0_DW3_25		121
#define GPE0_DW3_26		122
#define GPE0_DW3_27		123
#define GPE0_DW3_28		124
#define GPE0_DW3_29		125
#define GPE0_DW3_30		126
#define GPE0_DW3_31		127

#define GPE_MAX			GPE0_DW3_31

#endif
