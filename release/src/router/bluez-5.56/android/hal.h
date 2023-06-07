/* SPDX-License-Identifier: Apache-2.0 */
/*
 * Copyright (C) 2013 Intel Corporation
 *
 */

#include <hardware/bluetooth.h>
#include <hardware/bt_sock.h>
#include <hardware/bt_hh.h>
#include <hardware/bt_pan.h>
#include <hardware/bt_av.h>
#include <hardware/bt_rc.h>
#include <hardware/bt_hf.h>
#include <hardware/bt_gatt.h>
#include <hardware/bt_gatt_client.h>
#include <hardware/bt_gatt_server.h>
#include <hardware/bt_hl.h>

#define PLATFORM_VER(a, b, c) ((a << 16) | ( b << 8) | (c))

#if ANDROID_VERSION >= PLATFORM_VER(5, 0, 0)
#include <hardware/bt_hf_client.h>
#include <hardware/bt_mce.h>
#endif

btsock_interface_t *bt_get_socket_interface(void);
bthh_interface_t *bt_get_hidhost_interface(void);
btpan_interface_t *bt_get_pan_interface(void);
btav_interface_t *bt_get_a2dp_interface(void);
btrc_interface_t *bt_get_avrcp_interface(void);
bthf_interface_t *bt_get_handsfree_interface(void);
btgatt_interface_t *bt_get_gatt_interface(void);
bthl_interface_t *bt_get_health_interface(void);

#if ANDROID_VERSION >= PLATFORM_VER(5, 0, 0)
btrc_ctrl_interface_t *bt_get_avrcp_ctrl_interface(void);
bthf_client_interface_t *bt_get_hf_client_interface(void);
btmce_interface_t *bt_get_map_client_interface(void);
btav_interface_t *bt_get_a2dp_sink_interface(void);
#endif

void bt_thread_associate(void);
void bt_thread_disassociate(void);
