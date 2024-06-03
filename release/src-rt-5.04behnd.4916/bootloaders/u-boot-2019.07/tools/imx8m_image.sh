#!/bin/sh
# SPDX-License-Identifier: GPL-2.0+
#
# script to check whether the file exists in imximage.cfg for i.MX8M
#

file=$1

post_process=$2

blobs=`awk '/^SIGNED_HDMI/ {print $2} /^LOADER/ {print $2} /^SECOND_LOADER/ {print $2} /^DDR_FW/ {print $2}' $file`
for f in $blobs; do
	tmp=$srctree/$f

	if [ $f = "spl/u-boot-spl-ddr.bin" ] || [ $f = "u-boot.itb" ]; then
		continue
	fi

	if [ -f $f ]; then
		continue
	fi

	if [ ! -f $tmp ]; then
		echo "WARNING '$tmp' not found, resulting binary is not-functional" >&2
		exit 1
	fi

	sed -in "s;$f;$tmp;" $file
done

if [ $post_process = 1 ]; then
	if [ -f $srctree/lpddr4_pmu_train_1d_imem.bin ]; then
		objcopy -I binary -O binary --pad-to 0x8000 --gap-fill=0x0 $srctree/lpddr4_pmu_train_1d_imem.bin lpddr4_pmu_train_1d_imem_pad.bin
		objcopy -I binary -O binary --pad-to 0x4000 --gap-fill=0x0 $srctree/lpddr4_pmu_train_1d_dmem.bin lpddr4_pmu_train_1d_dmem_pad.bin
		objcopy -I binary -O binary --pad-to 0x8000 --gap-fill=0x0 $srctree/lpddr4_pmu_train_2d_imem.bin lpddr4_pmu_train_2d_imem_pad.bin
		cat lpddr4_pmu_train_1d_imem_pad.bin lpddr4_pmu_train_1d_dmem_pad.bin > lpddr4_pmu_train_1d_fw.bin
		cat lpddr4_pmu_train_2d_imem_pad.bin $srctree/lpddr4_pmu_train_2d_dmem.bin > lpddr4_pmu_train_2d_fw.bin
		cat spl/u-boot-spl.bin lpddr4_pmu_train_1d_fw.bin lpddr4_pmu_train_2d_fw.bin > spl/u-boot-spl-ddr.bin
		rm -f lpddr4_pmu_train_1d_fw.bin lpddr4_pmu_train_2d_fw.bin lpddr4_pmu_train_1d_imem_pad.bin lpddr4_pmu_train_1d_dmem_pad.bin lpddr4_pmu_train_2d_imem_pad.bin
	fi
fi

exit 0
