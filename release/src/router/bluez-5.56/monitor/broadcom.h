/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2011-2014  Intel Corporation
 *  Copyright (C) 2002-2010  Marcel Holtmann <marcel@holtmann.org>
 *
 *
 */

#include <stdint.h>

struct vendor_ocf;
struct vendor_evt;

const struct vendor_ocf *broadcom_vendor_ocf(uint16_t ocf);
const struct vendor_evt *broadcom_vendor_evt(uint8_t evt);
void broadcom_lm_diag(const void *data, uint8_t size);
