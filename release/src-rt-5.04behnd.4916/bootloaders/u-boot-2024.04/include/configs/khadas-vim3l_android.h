/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Configuration for the khadas VIM3L Android
 *
 * Copyright (C) 2021 Baylibre, SAS
 * Author: Guillaume LA ROQUE <glaroque@baylibre.com>
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#define LOGO_UUID "43a3305d-150f-4cc9-bd3b-38fca8693846;"
#define ROOT_UUID "ddb8c3f6-d94d-4394-b633-3134139cc2e0;"

#if defined(CONFIG_CMD_AB_SELECT)
#define PARTS_DEFAULT \
	"uuid_disk=${uuid_gpt_disk};" \
	"name=logo,start=512K,size=2M,uuid=" LOGO_UUID \
	"name=misc,size=512K,uuid=${uuid_gpt_misc};" \
	"name=dtbo_a,size=8M,uuid=${uuid_gpt_dtbo_a};" \
	"name=dtbo_b,size=8M,uuid=${uuid_gpt_dtbo_b};" \
	"name=vbmeta_a,size=512K,uuid=${uuid_gpt_vbmeta_a};" \
	"name=vbmeta_b,size=512K,uuid=${uuid_gpt_vbmeta_b};" \
	"name=boot_a,size=32M,bootable,uuid=${uuid_gpt_boot_a};" \
	"name=boot_b,size=32M,bootable,uuid=${uuid_gpt_boot_b};" \
	"name=super,size=3072M,uuid=${uuid_gpt_super};" \
	"name=userdata,size=11282M,uuid=${uuid_gpt_userdata};" \
	"name=rootfs,size=-,uuid=" ROOT_UUID
#else
#define PARTS_DEFAULT \
	"uuid_disk=${uuid_gpt_disk};" \
	"name=logo,start=512K,size=2M,uuid=" LOGO_UUID \
	"name=misc,size=512K,uuid=${uuid_gpt_misc};" \
	"name=dtbo,size=8M,uuid=${uuid_gpt_dtbo};" \
	"name=vbmeta,size=512K,uuid=${uuid_gpt_vbmeta};" \
	"name=boot,size=32M,bootable,uuid=${uuid_gpt_boot};" \
	"name=recovery,size=32M,uuid=${uuid_gpt_recovery};" \
	"name=cache,size=256M,uuid=${uuid_gpt_cache};" \
	"name=super,size=1792M,uuid=${uuid_gpt_super};" \
	"name=userdata,size=12786M,uuid=${uuid_gpt_userdata};" \
	"name=rootfs,size=-,uuid=" ROOT_UUID
#endif

#define EXTRA_ANDROID_ENV_SETTINGS \
	"board=vim3l\0" \
	"board_name=vim3l\0" \

#include <configs/meson64_android.h>

#endif /* __CONFIG_H */
