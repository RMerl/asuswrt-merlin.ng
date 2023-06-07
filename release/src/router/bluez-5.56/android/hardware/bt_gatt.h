/* SPDX-License-Identifier: Apache-2.0 */
/*
 * Copyright (C) 2013 The Android Open Source Project
 *
 */


#ifndef ANDROID_INCLUDE_BT_GATT_H
#define ANDROID_INCLUDE_BT_GATT_H

#include <stdint.h>
#include "bt_gatt_client.h"
#include "bt_gatt_server.h"

__BEGIN_DECLS

/** BT-GATT callbacks */
typedef struct {
    /** Set to sizeof(btgatt_callbacks_t) */
    size_t size;

    /** GATT Client callbacks */
    const btgatt_client_callbacks_t* client;

    /** GATT Server callbacks */
    const btgatt_server_callbacks_t* server;
} btgatt_callbacks_t;

/** Represents the standard Bluetooth GATT interface. */
typedef struct {
    /** Set to sizeof(btgatt_interface_t) */
    size_t          size;

    /**
     * Initializes the interface and provides callback routines
     */
    bt_status_t (*init)( const btgatt_callbacks_t* callbacks );

    /** Closes the interface */
    void (*cleanup)( void );

    /** Pointer to the GATT client interface methods.*/
    const btgatt_client_interface_t* client;

    /** Pointer to the GATT server interface methods.*/
    const btgatt_server_interface_t* server;
} btgatt_interface_t;

__END_DECLS

#endif /* ANDROID_INCLUDE_BT_GATT_H */
