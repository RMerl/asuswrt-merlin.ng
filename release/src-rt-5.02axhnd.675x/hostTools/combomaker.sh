#!/bin/bash
#sample script to create combo image
../hostTools/bcmComboMaker --output-file=image.out --header-flags=0x4321 \
--blocksize=2 \
--image=0x6838,96838REF,bcm96838GWO_JUMBO_nand_fs_image_128_pureubi.CL239387.w \
--image=0x6846,96846REF,bcm96846GWO_JUMBO_nand_fs_image_128_pureubi.CL239387.w \
--image=0x6878,968781REF,bcm96878GO_JUMBO_nand_fs_image_128_pureubi.CL239387.w \
--image=0x6858,96858REF,bcm96858GWO_JUMBO_nand_fs_image_128_pureubi.CL239387.w

cat image.out > comboimage.w
cat bcm96838GWO_JUMBO_nand_fs_image_128_pureubi.CL239387.w >> comboimage.w
cat bcm96846GWO_JUMBO_nand_fs_image_128_pureubi.CL239387.w >> comboimage.w
cat bcm96878GO_JUMBO_nand_fs_image_128_pureubi.CL239387.w >> comboimage.w
cat bcm96858GWO_JUMBO_nand_fs_image_128_pureubi.CL239387.w >> comboimage.w

