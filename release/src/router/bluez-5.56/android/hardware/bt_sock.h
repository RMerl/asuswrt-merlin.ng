/* SPDX-License-Identifier: Apache-2.0 */
/*
 * Copyright (C) 2012 The Android Open Source Project
 *
 */

#ifndef ANDROID_INCLUDE_BT_SOCK_H
#define ANDROID_INCLUDE_BT_SOCK_H

__BEGIN_DECLS

#define BTSOCK_FLAG_ENCRYPT 1
#define BTSOCK_FLAG_AUTH (1 << 1)

typedef enum {
    BTSOCK_RFCOMM = 1,
    BTSOCK_SCO = 2,
    BTSOCK_L2CAP = 3
} btsock_type_t;

/** Represents the standard BT SOCKET interface. */
typedef struct {
    short size;
    bt_bdaddr_t bd_addr;
    int channel;
    int status;
} __attribute__((packed)) sock_connect_signal_t;

typedef struct {

    /** set to size of this struct*/
    size_t          size;
    /**
     * listen to a rfcomm uuid or channel. It returns the socket fd from which
     * btsock_connect_signal can be read out when a remote device connected
     */
    bt_status_t (*listen)(btsock_type_t type, const char* service_name, const uint8_t* service_uuid, int channel, int* sock_fd, int flags);
    /*
     * connect to a rfcomm uuid channel of remote device, It returns the socket fd from which
     * the btsock_connect_signal and a new socket fd to be accepted can be read out when connected
     */
    bt_status_t (*connect)(const bt_bdaddr_t *bd_addr, btsock_type_t type, const uint8_t* uuid, int channel, int* sock_fd, int flags);

} btsock_interface_t;

__END_DECLS

#endif /* ANDROID_INCLUDE_BT_SOCK_H */
