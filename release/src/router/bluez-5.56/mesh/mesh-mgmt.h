/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2019  SILVAIR sp. z o.o. All rights reserved.
 *
 *
 */
#include <stdbool.h>

typedef void (*mesh_mgmt_read_info_func_t)(int index, void *user_data);

bool mesh_mgmt_list(mesh_mgmt_read_info_func_t cb, void *user_data);
