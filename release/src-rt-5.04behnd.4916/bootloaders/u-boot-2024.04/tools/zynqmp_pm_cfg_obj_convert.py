#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0+
# Copyright (C) 2019 Luca Ceresoli <luca@lucaceresoli.net>

import sys
import re
import struct
import logging
import argparse

parser = argparse.ArgumentParser(
    description='Convert a PMU configuration object from C source to a binary blob.')
parser.add_argument('-D', '--debug', action="store_true")
parser.add_argument(
    "in_file", metavar='INPUT_FILE',
    help='PMU configuration object (C source as produced by Xilinx XSDK)')
parser.add_argument(
    "out_file", metavar='OUTPUT_FILE',
    help='PMU configuration object binary blob')
args = parser.parse_args()

logging.basicConfig(format='%(levelname)s:%(message)s',
                    level=(logging.DEBUG if args.debug else logging.WARNING))

pm_define = {
    'PM_CAP_ACCESS'   : 0x1,
    'PM_CAP_CONTEXT'  : 0x2,
    'PM_CAP_WAKEUP'   : 0x4,

    'NODE_UNKNOWN'    :  0,
    'NODE_APU'        :  1,
    'NODE_APU_0'      :  2,
    'NODE_APU_1'      :  3,
    'NODE_APU_2'      :  4,
    'NODE_APU_3'      :  5,
    'NODE_RPU'        :  6,
    'NODE_RPU_0'      :  7,
    'NODE_RPU_1'      :  8,
    'NODE_PLD'        :  9,
    'NODE_FPD'        : 10,
    'NODE_OCM_BANK_0' : 11,
    'NODE_OCM_BANK_1' : 12,
    'NODE_OCM_BANK_2' : 13,
    'NODE_OCM_BANK_3' : 14,
    'NODE_TCM_0_A'    : 15,
    'NODE_TCM_0_B'    : 16,
    'NODE_TCM_1_A'    : 17,
    'NODE_TCM_1_B'    : 18,
    'NODE_L2'         : 19,
    'NODE_GPU_PP_0'   : 20,
    'NODE_GPU_PP_1'   : 21,
    'NODE_USB_0'      : 22,
    'NODE_USB_1'      : 23,
    'NODE_TTC_0'      : 24,
    'NODE_TTC_1'      : 25,
    'NODE_TTC_2'      : 26,
    'NODE_TTC_3'      : 27,
    'NODE_SATA'       : 28,
    'NODE_ETH_0'      : 29,
    'NODE_ETH_1'      : 30,
    'NODE_ETH_2'      : 31,
    'NODE_ETH_3'      : 32,
    'NODE_UART_0'     : 33,
    'NODE_UART_1'     : 34,
    'NODE_SPI_0'      : 35,
    'NODE_SPI_1'      : 36,
    'NODE_I2C_0'      : 37,
    'NODE_I2C_1'      : 38,
    'NODE_SD_0'       : 39,
    'NODE_SD_1'       : 40,
    'NODE_DP'         : 41,
    'NODE_GDMA'       : 42,
    'NODE_ADMA'       : 43,
    'NODE_NAND'       : 44,
    'NODE_QSPI'       : 45,
    'NODE_GPIO'       : 46,
    'NODE_CAN_0'      : 47,
    'NODE_CAN_1'      : 48,
    'NODE_EXTERN'     : 49,
    'NODE_APLL'       : 50,
    'NODE_VPLL'       : 51,
    'NODE_DPLL'       : 52,
    'NODE_RPLL'       : 53,
    'NODE_IOPLL'      : 54,
    'NODE_DDR'        : 55,
    'NODE_IPI_APU'    : 56,
    'NODE_IPI_RPU_0'  : 57,
    'NODE_GPU'        : 58,
    'NODE_PCIE'       : 59,
    'NODE_PCAP'       : 60,
    'NODE_RTC'        : 61,
    'NODE_LPD'        : 62,
    'NODE_VCU'        : 63,
    'NODE_IPI_RPU_1'  : 64,
    'NODE_IPI_PL_0'   : 65,
    'NODE_IPI_PL_1'   : 66,
    'NODE_IPI_PL_2'   : 67,
    'NODE_IPI_PL_3'   : 68,
    'NODE_PL'         : 69,
    'NODE_ID_MA'      : 70,

    'XILPM_RESET_PCIE_CFG'         : 1000,
    'XILPM_RESET_PCIE_BRIDGE'      : 1001,
    'XILPM_RESET_PCIE_CTRL'        : 1002,
    'XILPM_RESET_DP'               : 1003,
    'XILPM_RESET_SWDT_CRF'         : 1004,
    'XILPM_RESET_AFI_FM5'          : 1005,
    'XILPM_RESET_AFI_FM4'          : 1006,
    'XILPM_RESET_AFI_FM3'          : 1007,
    'XILPM_RESET_AFI_FM2'          : 1008,
    'XILPM_RESET_AFI_FM1'          : 1009,
    'XILPM_RESET_AFI_FM0'          : 1010,
    'XILPM_RESET_GDMA'             : 1011,
    'XILPM_RESET_GPU_PP1'          : 1012,
    'XILPM_RESET_GPU_PP0'          : 1013,
    'XILPM_RESET_GPU'              : 1014,
    'XILPM_RESET_GT'               : 1015,
    'XILPM_RESET_SATA'             : 1016,
    'XILPM_RESET_ACPU3_PWRON'      : 1017,
    'XILPM_RESET_ACPU2_PWRON'      : 1018,
    'XILPM_RESET_ACPU1_PWRON'      : 1019,
    'XILPM_RESET_ACPU0_PWRON'      : 1020,
    'XILPM_RESET_APU_L2'           : 1021,
    'XILPM_RESET_ACPU3'            : 1022,
    'XILPM_RESET_ACPU2'            : 1023,
    'XILPM_RESET_ACPU1'            : 1024,
    'XILPM_RESET_ACPU0'            : 1025,
    'XILPM_RESET_DDR'              : 1026,
    'XILPM_RESET_APM_FPD'          : 1027,
    'XILPM_RESET_SOFT'             : 1028,
    'XILPM_RESET_GEM0'             : 1029,
    'XILPM_RESET_GEM1'             : 1030,
    'XILPM_RESET_GEM2'             : 1031,
    'XILPM_RESET_GEM3'             : 1032,
    'XILPM_RESET_QSPI'             : 1033,
    'XILPM_RESET_UART0'            : 1034,
    'XILPM_RESET_UART1'            : 1035,
    'XILPM_RESET_SPI0'             : 1036,
    'XILPM_RESET_SPI1'             : 1037,
    'XILPM_RESET_SDIO0'            : 1038,
    'XILPM_RESET_SDIO1'            : 1039,
    'XILPM_RESET_CAN0'             : 1040,
    'XILPM_RESET_CAN1'             : 1041,
    'XILPM_RESET_I2C0'             : 1042,
    'XILPM_RESET_I2C1'             : 1043,
    'XILPM_RESET_TTC0'             : 1044,
    'XILPM_RESET_TTC1'             : 1045,
    'XILPM_RESET_TTC2'             : 1046,
    'XILPM_RESET_TTC3'             : 1047,
    'XILPM_RESET_SWDT_CRL'         : 1048,
    'XILPM_RESET_NAND'             : 1049,
    'XILPM_RESET_ADMA'             : 1050,
    'XILPM_RESET_GPIO'             : 1051,
    'XILPM_RESET_IOU_CC'           : 1052,
    'XILPM_RESET_TIMESTAMP'        : 1053,
    'XILPM_RESET_RPU_R50'          : 1054,
    'XILPM_RESET_RPU_R51'          : 1055,
    'XILPM_RESET_RPU_AMBA'         : 1056,
    'XILPM_RESET_OCM'              : 1057,
    'XILPM_RESET_RPU_PGE'          : 1058,
    'XILPM_RESET_USB0_CORERESET'   : 1059,
    'XILPM_RESET_USB1_CORERESET'   : 1060,
    'XILPM_RESET_USB0_HIBERRESET'  : 1061,
    'XILPM_RESET_USB1_HIBERRESET'  : 1062,
    'XILPM_RESET_USB0_APB'         : 1063,
    'XILPM_RESET_USB1_APB'         : 1064,
    'XILPM_RESET_IPI'              : 1065,
    'XILPM_RESET_APM_LPD'          : 1066,
    'XILPM_RESET_RTC'              : 1067,
    'XILPM_RESET_SYSMON'           : 1068,
    'XILPM_RESET_AFI_FM6'          : 1069,
    'XILPM_RESET_LPD_SWDT'         : 1070,
    'XILPM_RESET_FPD'              : 1071,
    'XILPM_RESET_RPU_DBG1'         : 1072,
    'XILPM_RESET_RPU_DBG0'         : 1073,
    'XILPM_RESET_DBG_LPD'          : 1074,
    'XILPM_RESET_DBG_FPD'          : 1075,
    'XILPM_RESET_APLL'             : 1076,
    'XILPM_RESET_DPLL'             : 1077,
    'XILPM_RESET_VPLL'             : 1078,
    'XILPM_RESET_IOPLL'            : 1079,
    'XILPM_RESET_RPLL'             : 1080,
    'XILPM_RESET_GPO3_PL_0'        : 1081,
    'XILPM_RESET_GPO3_PL_1'        : 1082,
    'XILPM_RESET_GPO3_PL_2'        : 1083,
    'XILPM_RESET_GPO3_PL_3'        : 1084,
    'XILPM_RESET_GPO3_PL_4'        : 1085,
    'XILPM_RESET_GPO3_PL_5'        : 1086,
    'XILPM_RESET_GPO3_PL_6'        : 1087,
    'XILPM_RESET_GPO3_PL_7'        : 1088,
    'XILPM_RESET_GPO3_PL_8'        : 1089,
    'XILPM_RESET_GPO3_PL_9'        : 1090,
    'XILPM_RESET_GPO3_PL_10'       : 1091,
    'XILPM_RESET_GPO3_PL_11'       : 1092,
    'XILPM_RESET_GPO3_PL_12'       : 1093,
    'XILPM_RESET_GPO3_PL_13'       : 1094,
    'XILPM_RESET_GPO3_PL_14'       : 1095,
    'XILPM_RESET_GPO3_PL_15'       : 1096,
    'XILPM_RESET_GPO3_PL_16'       : 1097,
    'XILPM_RESET_GPO3_PL_17'       : 1098,
    'XILPM_RESET_GPO3_PL_18'       : 1099,
    'XILPM_RESET_GPO3_PL_19'       : 1100,
    'XILPM_RESET_GPO3_PL_20'       : 1101,
    'XILPM_RESET_GPO3_PL_21'       : 1102,
    'XILPM_RESET_GPO3_PL_22'       : 1103,
    'XILPM_RESET_GPO3_PL_23'       : 1104,
    'XILPM_RESET_GPO3_PL_24'       : 1105,
    'XILPM_RESET_GPO3_PL_25'       : 1106,
    'XILPM_RESET_GPO3_PL_26'       : 1107,
    'XILPM_RESET_GPO3_PL_27'       : 1108,
    'XILPM_RESET_GPO3_PL_28'       : 1109,
    'XILPM_RESET_GPO3_PL_29'       : 1110,
    'XILPM_RESET_GPO3_PL_30'       : 1111,
    'XILPM_RESET_GPO3_PL_31'       : 1112,
    'XILPM_RESET_RPU_LS'           : 1113,
    'XILPM_RESET_PS_ONLY'          : 1114,
    'XILPM_RESET_PL'               : 1115,
    'XILPM_RESET_GPIO5_EMIO_92'    : 1116,
    'XILPM_RESET_GPIO5_EMIO_93'    : 1117,
    'XILPM_RESET_GPIO5_EMIO_94'    : 1118,
    'XILPM_RESET_GPIO5_EMIO_95'    : 1119,

    'PM_CONFIG_MASTER_SECTION_ID'        : 0x101,
    'PM_CONFIG_SLAVE_SECTION_ID'         : 0x102,
    'PM_CONFIG_PREALLOC_SECTION_ID'      : 0x103,
    'PM_CONFIG_POWER_SECTION_ID'         : 0x104,
    'PM_CONFIG_RESET_SECTION_ID'         : 0x105,
    'PM_CONFIG_SHUTDOWN_SECTION_ID'      : 0x106,
    'PM_CONFIG_SET_CONFIG_SECTION_ID'    : 0x107,
    'PM_CONFIG_GPO_SECTION_ID'           : 0x108,

    'PM_SLAVE_FLAG_IS_SHAREABLE'         : 0x1,
    'PM_MASTER_USING_SLAVE_MASK'         : 0x2,

    'PM_CONFIG_GPO1_MIO_PIN_34_MAP'      : (1 << 10),
    'PM_CONFIG_GPO1_MIO_PIN_35_MAP'      : (1 << 11),
    'PM_CONFIG_GPO1_MIO_PIN_36_MAP'      : (1 << 12),
    'PM_CONFIG_GPO1_MIO_PIN_37_MAP'      : (1 << 13),

    'PM_CONFIG_GPO1_BIT_2_MASK'          : (1 << 2),
    'PM_CONFIG_GPO1_BIT_3_MASK'          : (1 << 3),
    'PM_CONFIG_GPO1_BIT_4_MASK'          : (1 << 4),
    'PM_CONFIG_GPO1_BIT_5_MASK'          : (1 << 5),

    'SUSPEND_TIMEOUT'                    : 0xFFFFFFFF,

    'PM_CONFIG_OBJECT_TYPE_BASE'         : 0x1,

    'PM_CONFIG_IPI_PSU_CORTEXA53_0_MASK' : 0x00000001,
    'PM_CONFIG_IPI_PSU_CORTEXR5_0_MASK'  : 0x00000100,
    'PM_CONFIG_IPI_PSU_CORTEXR5_1_MASK'  : 0x00000200,
}

in_file  = open(args.in_file,  mode='r')
out_file = open(args.out_file, mode='wb')

num_re   = re.compile(r"^([0-9]+)U?$")
const_re = re.compile(r"^([A-Z_][A-Z0-9_]*)$")

def process_item(item):
    logging.debug("* ITEM   " + item)

    value = 0
    for item in item.split('|'):
        item = item.strip()

        num_match   = num_re  .match(item)
        const_match = const_re.match(item)

        if num_match:
            num = int(num_match.group(1))
            logging.debug("  - num  " + str(num))
            value |= num
        elif const_match:
            name = const_match.group(1)
            if not name in pm_define:
                sys.stderr.write("Unknown define " + name + "!\n")
                exit(1)
            num = pm_define[name]
            logging.debug("  - def  " + hex(num))
            value |= num

    logging.debug("  = res  " + hex(value))
    out_file.write(struct.pack('<L', value))


# Read all code
code = in_file.read()

# remove comments
code = re.sub('//.*?\n|/\*.*?\*/', '', code, flags=re.DOTALL)

# remove everything outside the XPm_ConfigObject array definition
code = re.search('const u32 XPm_ConfigObject.*=.*{\n(.*)};',
                 code, flags=re.DOTALL).group(1)

# Process each comma-separated array item
for item in code.split(','):
    item = item.strip()
    if item:
        process_item(item)

print("Wrote %d bytes" % out_file.tell())
