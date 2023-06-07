/* SPDX-License-Identifier: Apache-2.0 */
/*
 * Copyright (C) 2014 The Android Open Source Project
 *
 */

#ifndef ANDROID_INCLUDE_BT_MCE_H
#define ANDROID_INCLUDE_BT_MCE_H

__BEGIN_DECLS

/** MAS instance description */
typedef struct
{
    int  id;
    int  scn;
    int  msg_types;
    char *p_name;
} btmce_mas_instance_t;

/** callback for get_remote_mas_instances */
typedef void (*btmce_remote_mas_instances_callback)(bt_status_t status, bt_bdaddr_t *bd_addr,
                                                    int num_instances, btmce_mas_instance_t *instances);

typedef struct {
    /** set to sizeof(btmce_callbacks_t) */
    size_t      size;
    btmce_remote_mas_instances_callback  remote_mas_instances_cb;
} btmce_callbacks_t;

typedef struct {
    /** set to size of this struct */
    size_t size;

    /** register BT MCE callbacks */
    bt_status_t (*init)(btmce_callbacks_t *callbacks);

    /** search for MAS instances on remote device */
    bt_status_t (*get_remote_mas_instances)(bt_bdaddr_t *bd_addr);
} btmce_interface_t;

__END_DECLS

#endif /* ANDROID_INCLUDE_BT_MCE_H */
