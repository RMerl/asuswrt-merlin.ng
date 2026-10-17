/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Configuration for the SEI610
 *
 * Copyright (C) 2019 Baylibre, SAS
 * Author: Jerome Brunet <jbrunet@baylibre.com>
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#define LOGO_UUID "43a3305d-150f-4cc9-bd3b-38fca8693846;"
#define ROOT_UUID "ddb8c3f6-d94d-4394-b633-3134139cc2e0;"

#define PARTS_DEFAULT \
	"uuid_disk=${uuid_gpt_disk};" \
	"name=logo,start=512K,size=2M,uuid=" LOGO_UUID \
	"name=misc,size=512K,uuid=${uuid_gpt_misc};" \
	"name=dtbo,size=8M,uuid=${uuid_gpt_dtbo};" \
	"name=vbmeta,size=512K,uuid=${uuid_gpt_vbmeta};" \
	"name=boot,size=32M,bootable,uuid=${uuid_gpt_boot};" \
	"name=recovery,size=32M,uuid=${uuid_gpt_recovery};" \
	"name=cache,size=256M,uuid=${uuid_gpt_cache};" \
	"name=super,size=2304M,uuid=${uuid_gpt_super};" \
	"name=userdata,size=12274M,uuid=${uuid_gpt_userdata};" \
	"name=rootfs,size=-,uuid=" ROOT_UUID

#include <configs/meson64_android.h>

#endif /* __CONFIG_H */
