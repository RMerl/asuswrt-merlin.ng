# SPDX-License-Identifier: GPL-2.0+

quiet_cmd_srec_cat = SRECCAT $@
      cmd_srec_cat = srec_cat -output $@ -$2 \
			$< -binary \
			-fill 0x00 -within $< -binary -range-pad 16 \
			-offset $3

u-boot.mcs: u-boot.bin
	$(call cmd,srec_cat,intel,0x7c00000)

# if srec_cat is present build u-boot.mcs by default
has_srec_cat = $(call try-run,srec_cat -VERSion,y,n)
INPUTS-$(has_srec_cat) += u-boot.mcs
CLEAN_FILES += u-boot.mcs
