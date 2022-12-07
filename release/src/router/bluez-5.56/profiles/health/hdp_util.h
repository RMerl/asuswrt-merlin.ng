/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2010 GSyC/LibreSoft, Universidad Rey Juan Carlos.
 *
 */

#ifndef __HDP_UTIL_H__
#define __HDP_UTIL_H__

typedef void (*hdp_continue_mdep_f)(uint8_t mdep, gpointer user_data,
								GError *err);
typedef void (*hdp_continue_dcpsm_f)(uint16_t dcpsm, gpointer user_data,
								GError *err);
typedef void (*hdp_continue_proc_f)(gpointer user_data, GError *err);

struct hdp_application *hdp_get_app_config(DBusMessageIter *iter, GError **err);
gboolean hdp_update_sdp_record(struct hdp_adapter *adapter, GSList *app_list);
gboolean hdp_get_mdep(struct hdp_device *device, struct hdp_application *app,
				hdp_continue_mdep_f func,
				gpointer data, GDestroyNotify destroy,
				GError **err);

gboolean hdp_establish_mcl(struct hdp_device *device,
						hdp_continue_proc_f func,
						gpointer data,
						GDestroyNotify destroy,
						GError **err);

gboolean hdp_get_dcpsm(struct hdp_device *device, hdp_continue_dcpsm_f func,
							gpointer data,
							GDestroyNotify destroy,
							GError **err);


struct hdp_application *hdp_application_ref(struct hdp_application *app);
void hdp_application_unref(struct hdp_application *app);

struct hdp_device *health_device_ref(struct hdp_device *hdp_dev);
void health_device_unref(struct hdp_device *hdp_dev);


#endif /* __HDP_UTIL_H__ */
