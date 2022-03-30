/* SPDX-License-Identifier: GPL-2.0+
 *
 *  Copyright 2019 Broadcom Ltd.
 */

/*******************************************************************
 * bdmf_types.h
 *
 * Broadcom Device Management Framework - built-in attribute types
 *
 *******************************************************************/

#ifndef BDMF_DATA_TYPES_H_
#define BDMF_DATA_TYPES_H_

typedef struct {
    uint8_t b[6];  /**< Address bytes */
} bdmf_mac_t;

#endif /* BDMF_DATA_TYPES_H_ */
