#!/bin/sh
# SPDX-License-Identifier: GPL-2.0+
#
# script to check whether the file exists in imximage.cfg for i.MX8M
#

file=$1

post_process=$2

blobs=`awk '/^SIGNED_HDMI/ {print $2} /^LOADER/ {print $2} /^SECOND_LOADER/ {print $2} /^DDR_FW/ {print $2}' $file`
for f in $blobs; do
	if [ $f = "spl/u-boot-spl-ddr.bin" ] || [ $f = "u-boot.itb" ]; then
		continue
	fi

	if [ ! -f $f ]; then
		echo "WARNING '$f' not found, resulting binary is not-functional" >&2
		exit 1
	fi
done

if [ $post_process = 1 ]; then
	if [ -f lpddr4_pmu_train_1d_imem.bin ]; then
		objcopy -I binary -O binary --pad-to 0x8000 --gap-fill=0x0 lpddr4_pmu_train_1d_imem.bin lpddr4_pmu_train_1d_imem_pad.bin
		objcopy -I binary -O binary --pad-to 0x4000 --gap-fill=0x0 lpddr4_pmu_train_1d_dmem.bin lpddr4_pmu_train_1d_dmem_pad.bin
		objcopy -I binary -O binary --pad-to 0x8000 --gap-fill=0x0 lpddr4_pmu_train_2d_imem.bin lpddr4_pmu_train_2d_imem_pad.bin
		cat lpddr4_pmu_train_1d_imem_pad.bin lpddr4_pmu_train_1d_dmem_pad.bin > lpddr4_pmu_train_1d_fw.bin
		cat lpddr4_pmu_train_2d_imem_pad.bin lpddr4_pmu_train_2d_dmem.bin > lpddr4_pmu_train_2d_fw.bin
		dd if=spl/u-boot-spl.bin of=spl/u-boot-spl-pad.bin bs=4 conv=sync
		cat spl/u-boot-spl-pad.bin lpddr4_pmu_train_1d_fw.bin lpddr4_pmu_train_2d_fw.bin > spl/u-boot-spl-ddr.bin
		rm -f lpddr4_pmu_train_1d_fw.bin lpddr4_pmu_train_2d_fw.bin lpddr4_pmu_train_1d_imem_pad.bin lpddr4_pmu_train_1d_dmem_pad.bin lpddr4_pmu_train_2d_imem_pad.bin spl/u-boot-spl-pad.bin
	fi
	if [ -f ddr4_imem_1d.bin ]; then
		objcopy -I binary -O binary --pad-to 0x8000 --gap-fill=0x0 ddr4_imem_1d.bin ddr4_imem_1d_pad.bin
		objcopy -I binary -O binary --pad-to 0x4000 --gap-fill=0x0 ddr4_dmem_1d.bin ddr4_dmem_1d_pad.bin
		objcopy -I binary -O binary --pad-to 0x8000 --gap-fill=0x0 ddr4_imem_2d.bin ddr4_imem_2d_pad.bin
		cat ddr4_imem_1d_pad.bin ddr4_dmem_1d_pad.bin > ddr4_1d_fw.bin
		cat ddr4_imem_2d_pad.bin ddr4_dmem_2d.bin > ddr4_2d_fw.bin
		dd if=spl/u-boot-spl.bin of=spl/u-boot-spl-pad.bin bs=4 conv=sync
		cat spl/u-boot-spl-pad.bin ddr4_1d_fw.bin ddr4_2d_fw.bin > spl/u-boot-spl-ddr.bin
		rm -f ddr4_1d_fw.bin ddr4_2d_fw.bin ddr4_imem_1d_pad.bin ddr4_dmem_1d_pad.bin ddr4_imem_2d_pad.bin spl/u-boot-spl-pad.bin
	fi
	if [ -f ddr3_imem_1d.bin ]; then
		objcopy -I binary -O binary --pad-to 0x8000 --gap-fill=0x0 ddr3_imem_1d.bin ddr3_imem_1d_pad.bin
		cat ddr3_imem_1d_pad.bin ddr3_dmem_1d.bin > ddr3_1d_fw.bin
		dd if=spl/u-boot-spl.bin of=spl/u-boot-spl-pad.bin bs=4 conv=sync
		cat spl/u-boot-spl-pad.bin ddr3_1d_fw.bin > spl/u-boot-spl-ddr.bin
		rm -f ddr3_1d_fw.bin ddr3_imem_1d_pad.bin spl/u-boot-spl-pad.bin
	fi
fi

exit 0
