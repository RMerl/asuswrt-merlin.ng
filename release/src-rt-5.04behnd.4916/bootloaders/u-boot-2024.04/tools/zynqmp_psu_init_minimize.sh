#!/bin/bash
# SPDX-License-Identifier: GPL-2.0+
# Copyright (C) 2018 Michal Simek <michal.simek@amd.com>
# Copyright (C) 2019 Luca Ceresoli <luca@lucaceresoli.net>
# Copyright (C) 2022 Weidmüller Interface GmbH & Co. KG
# Stefan Herbrechtsmeier <stefan.herbrechtsmeier@weidmueller.com>

usage()
{
    cat <<EOF

Transform a pair of psu_init_gpl.c and .h files produced by the Xilinx
Vivado tool for ZynqMP into a smaller psu_init_gpl.c file that is almost
checkpatch compliant. Minor coding style might still be needed. Must be
run from the top-level U-Boot source directory.

Usage:   zynqmp_psu_init_minimize.sh INPUT_DIR OUTPUT_DIR
Example: zynqmp_psu_init_minimize.sh \\
                 /path/to/original/psu_init_gpl_c_and_h/ \\
                 board/xilinx/zynqmp/<my_board>/

Notes:   INPUT_DIR must contain both .c and .h files.
         If INPUT_DIR and OUTPUT_DIR are the same directory,
         psu_init_gpl.c will be overwritten.

EOF
}

set -o errexit -o errtrace
set -o nounset

if [ $# -ne 2 ]
then
    usage >&2
    exit 1
fi

IN="${1}/psu_init_gpl.c"
OUT="${2}/psu_init_gpl.c"
TMP=$(mktemp /tmp/psu_init_gpl.XXXXXX)
trap "rm ${TMP}" ERR

# Step through a temp file to allow both $IN!=$OUT and $IN==$OUT
sed -e '/sleep.h/d' \
    -e '/xil_io.h/d' \
    ${IN} >${TMP}
cp ${TMP} ${OUT}

# preprocess to expand defines, then remove cpp lines starting with '#'
gcc -I${1} -E ${OUT} -o ${TMP}
sed '/^#/d' ${TMP} >${OUT}

# Remove trivial code before psu_pll_init_data()
sed -ni '/psu_pll_init_data/,$p' ${OUT}

# Functions are lowercase in U-Boot, rename them
sed -i 's/PSU_Mask_Write/psu_mask_write/g' ${OUT}
sed -i 's/mask_pollOnValue/mask_pollonvalue/g' ${OUT}
sed -i 's/RegValue/regvalue/g' ${OUT}
sed -i 's/MaskStatus/maskstatus/g' ${OUT}

sed -i '/&= psu_peripherals_powerdwn_data()/d' ${OUT}

FUNCS_TO_REMOVE="psu_protection
psu_..._protection
psu_init_xppu_aper_ram
mask_delay(u32
mask_read(u32
mask_poll(u32
mask_pollonvalue(u32
psu_ps_pl_reset_config_data
psu_ps_pl_isolation_removal_data
psu_apply_master_tz
psu_post_config_data
psu_post_config_data
psu_peripherals_powerdwn_data
psu_init_ddr_self_refresh
xmpu
xppu
"
for i in $FUNCS_TO_REMOVE; do
sed -i "/$i/,/^}$/d" ${OUT}
done

scripts/Lindent ${OUT}

# Prepend 'static' to internal functions
sed -i 's/^.*data(void)$/static &/g' ${OUT}
sed -i 's/^.*psu_afi_config(void)$/static &/g' ${OUT}
sed -i 's/^void init_peripheral/static &/g' ${OUT}
sed -i 's/^int serdes/static &/g' ${OUT}
sed -i 's/^int init_serdes/static &/g' ${OUT}
sed -i 's/^unsigned long /static &/g' ${OUT}

sed -i 's/()$/(void)/g' ${OUT}
sed -i 's/0X/0x/g' ${OUT}

# return (0) -> return 0
sed -ri 's/return \(([0-9]+)\)/return \1/g' ${OUT}

# Add header
cat << EOF >${TMP}
// SPDX-License-Identifier: GPL-2.0+
/*
 * (c) Copyright 2015 Xilinx, Inc. All rights reserved.
 */

#include <asm/arch/psu_init_gpl.h>
#include <xil_io.h>

EOF

cat ${OUT} >>${TMP}
cp ${TMP} ${OUT}

# Temporarily convert newlines to do some mangling across lines
tr "\n" "\r" <${OUT} >${TMP}

# Cleanup empty loops. E.g.:
# |while (e) {|
# |           | ==> |while (e)|
# |    }      |     |    ;    |
# |           |
sed -i -r 's| \{\r+(\t*)\}\r\r|\r\1\t;\r|g' ${TMP}

# Remove empty line between variable declaration
sed -i -r 's|\r(\r\t(unsigned )?int )|\1|g' ${TMP}

# Remove empty lines at function beginning/end
sed -i -e 's|\r{\r\r|\r{\r|g' ${TMP}
sed -i -e 's|\r\r}\r|\r}\r|g' ${TMP}

# Remove empty lines after '{' line
sed -i -e 's| {\r\r| {\r|g' ${TMP}

# Remove braces {} around single statement blocks. E.g.:
# | while (e) { |    | while (e) |
# |     stg();  | => |     stg();|
# | }           |
sed -i -r 's| \{(\r[^\r]*;)\r\t*\}|\1|g' ${TMP}

# Remove Unnecessary parentheses around 'n_code <= 0x3C' and similar. E.g.:
# if ((p_code >= 0x26) && ...) -> if (p_code >= 0x26 && ...)
sed -i -r 's|\((._code .= [x[:xdigit:]]+)\)|\1|g' ${TMP}

# Move helper functions below header includes
TARGET="#include <xil_io.h>"
START="static int serdes_rst_seq"
END="static int serdes_enb_coarse_saturation"

sed -i -e "s|\(${TARGET}\r\r\)\(.*\)\(${START}(.*\)\(${END}(\)|\1\3\2\4|g" \
    ${TMP}

# Convert back newlines
tr "\r" "\n" <${TMP} >${OUT}

# Remove unnecessary settings
# - Low level UART
SETTINGS_TO_REMOVE="0xFF000000
0xFF000004
0xFF000018
0xFF000034
0xFF010000
0xFF010004
0xFF010018
0xFF010034
"
for i in $SETTINGS_TO_REMOVE; do
sed -i "/^\tpsu_mask_write($i,.*$/d" ${OUT}
done

rm ${TMP}
