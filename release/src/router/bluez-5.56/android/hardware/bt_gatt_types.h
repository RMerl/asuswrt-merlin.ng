/* SPDX-License-Identifier: Apache-2.0 */
/*
 * Copyright (C) 2013 The Android Open Source Project
 *
 */


#ifndef ANDROID_INCLUDE_BT_GATT_TYPES_H
#define ANDROID_INCLUDE_BT_GATT_TYPES_H

#include <stdint.h>
#include <stdbool.h>

__BEGIN_DECLS

/**
 * GATT Service types
 */
#define BTGATT_SERVICE_TYPE_PRIMARY 0
#define BTGATT_SERVICE_TYPE_SECONDARY 1

/** GATT ID adding instance id tracking to the UUID */
typedef struct
{
    bt_uuid_t           uuid;
    uint8_t             inst_id;
} btgatt_gatt_id_t;

/** GATT Service ID also identifies the service type (primary/secondary) */
typedef struct
{
    btgatt_gatt_id_t    id;
    uint8_t             is_primary;
} btgatt_srvc_id_t;

/** Preferred physical Transport for GATT connection */
typedef enum
{
    GATT_TRANSPORT_AUTO,
    GATT_TRANSPORT_BREDR,
    GATT_TRANSPORT_LE
} btgatt_transport_t;

__END_DECLS

#endif /* ANDROID_INCLUDE_BT_GATT_TYPES_H */
