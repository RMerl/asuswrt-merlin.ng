/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2010 GSyC/LibreSoft, Universidad Rey Juan Carlos.
 *
 */

int hdp_adapter_register(struct btd_adapter *btd_adapter);
void hdp_adapter_unregister(struct btd_adapter *btd_adapter);

int hdp_device_register(struct btd_device *device);
void hdp_device_unregister(struct btd_device *device);

int hdp_manager_start(void);
void hdp_manager_stop(void);

gboolean hdp_set_mcl_cb(struct hdp_device *device, GError **err);
