/*
 *  HID driver for Logitech Unifying receivers
 *
 *  Copyright (c) 2011 Logitech
 */

/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.

 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 */


#include <linux/device.h>
#include <linux/hid.h>
#include <linux/module.h>
#include <linux/usb.h>
#include <linux/kfifo.h>
#include <asm/unaligned.h>
#include "hid-ids.h"

#define DJ_MAX_PAIRED_DEVICES			6
#define DJ_MAX_NUMBER_NOTIFICATIONS		8
#define DJ_RECEIVER_INDEX			0
#define DJ_DEVICE_INDEX_MIN			1
#define DJ_DEVICE_INDEX_MAX			6

#define DJREPORT_SHORT_LENGTH			15
#define DJREPORT_LONG_LENGTH			32

#define REPORT_ID_DJ_SHORT			0x20
#define REPORT_ID_DJ_LONG			0x21

#define REPORT_ID_HIDPP_SHORT			0x10
#define REPORT_ID_HIDPP_LONG			0x11

#define HIDPP_REPORT_SHORT_LENGTH		7
#define HIDPP_REPORT_LONG_LENGTH		20

#define HIDPP_RECEIVER_INDEX			0xff

#define REPORT_TYPE_RFREPORT_FIRST		0x01
#define REPORT_TYPE_RFREPORT_LAST		0x1F

/* Command Switch to DJ mode */
#define REPORT_TYPE_CMD_SWITCH			0x80
#define CMD_SWITCH_PARAM_DEVBITFIELD		0x00
#define CMD_SWITCH_PARAM_TIMEOUT_SECONDS	0x01
#define TIMEOUT_NO_KEEPALIVE			0x00

/* Command to Get the list of Paired devices */
#define REPORT_TYPE_CMD_GET_PAIRED_DEVICES	0x81

/* Device Paired Notification */
#define REPORT_TYPE_NOTIF_DEVICE_PAIRED		0x41
#define SPFUNCTION_MORE_NOTIF_EXPECTED		0x01
#define SPFUNCTION_DEVICE_LIST_EMPTY		0x02
#define DEVICE_PAIRED_PARAM_SPFUNCTION		0x00
#define DEVICE_PAIRED_PARAM_EQUAD_ID_LSB	0x01
#define DEVICE_PAIRED_PARAM_EQUAD_ID_MSB	0x02
#define DEVICE_PAIRED_RF_REPORT_TYPE		0x03

/* Device Un-Paired Notification */
#define REPORT_TYPE_NOTIF_DEVICE_UNPAIRED	0x40


/* Connection Status Notification */
#define REPORT_TYPE_NOTIF_CONNECTION_STATUS	0x42
#define CONNECTION_STATUS_PARAM_STATUS		0x00
#define STATUS_LINKLOSS				0x01

/* Error Notification */
#define REPORT_TYPE_NOTIF_ERROR			0x7F
#define NOTIF_ERROR_PARAM_ETYPE			0x00
#define ETYPE_KEEPALIVE_TIMEOUT			0x01

/* supported DJ HID && RF report types */
#define REPORT_TYPE_KEYBOARD			0x01
#define REPORT_TYPE_MOUSE			0x02
#define REPORT_TYPE_CONSUMER_CONTROL		0x03
#define REPORT_TYPE_SYSTEM_CONTROL		0x04
#define REPORT_TYPE_MEDIA_CENTER		0x08
#define REPORT_TYPE_LEDS			0x0E

/* RF Report types bitfield */
#define STD_KEYBOARD				0x00000002
#define STD_MOUSE				0x00000004
#define MULTIMEDIA				0x00000008
#define POWER_KEYS				0x00000010
#define MEDIA_CENTER				0x00000100
#define KBD_LEDS				0x00004000

struct dj_report {
	u8 report_id;
	u8 device_index;
	u8 report_type;
	u8 report_params[DJREPORT_SHORT_LENGTH - 3];
};

struct dj_receiver_dev {
	struct hid_device *hdev;
	struct dj_device *paired_dj_devices[DJ_MAX_PAIRED_DEVICES +
					    DJ_DEVICE_INDEX_MIN];
	struct work_struct work;
	struct kfifo notif_fifo;
	spinlock_t lock;
	bool querying_devices;
};

struct dj_device {
	struct hid_device *hdev;
	struct dj_receiver_dev *dj_receiver_dev;
	u32 reports_supported;
	u8 device_index;
};

/* Keyboard descriptor (1) */
static const char kbd_descriptor[] = {
	0x05, 0x01,		/* USAGE_PAGE (generic Desktop)     */
	0x09, 0x06,		/* USAGE (Keyboard)         */
	0xA1, 0x01,		/* COLLECTION (Application)     */
	0x85, 0x01,		/* REPORT_ID (1)            */
	0x95, 0x08,		/*   REPORT_COUNT (8)           */
	0x75, 0x01,		/*   REPORT_SIZE (1)            */
	0x15, 0x00,		/*   LOGICAL_MINIMUM (0)        */
	0x25, 0x01,		/*   LOGICAL_MAXIMUM (1)        */
	0x05, 0x07,		/*   USAGE_PAGE (Keyboard)      */
	0x19, 0xE0,		/*   USAGE_MINIMUM (Left Control)   */
	0x29, 0xE7,		/*   USAGE_MAXIMUM (Right GUI)      */
	0x81, 0x02,		/*   INPUT (Data,Var,Abs)       */
	0x95, 0x06,		/*   REPORT_COUNT (6)           */
	0x75, 0x08,		/*   REPORT_SIZE (8)            */
	0x15, 0x00,		/*   LOGICAL_MINIMUM (0)        */
	0x26, 0xFF, 0x00,	/*   LOGICAL_MAXIMUM (255)      */
	0x05, 0x07,		/*   USAGE_PAGE (Keyboard)      */
	0x19, 0x00,		/*   USAGE_MINIMUM (no event)       */
	0x2A, 0xFF, 0x00,	/*   USAGE_MAXIMUM (reserved)       */
	0x81, 0x00,		/*   INPUT (Data,Ary,Abs)       */
	0x85, 0x0e,		/* REPORT_ID (14)               */
	0x05, 0x08,		/*   USAGE PAGE (LED page)      */
	0x95, 0x05,		/*   REPORT COUNT (5)           */
	0x75, 0x01,		/*   REPORT SIZE (1)            */
	0x15, 0x00,		/*   LOGICAL_MINIMUM (0)        */
	0x25, 0x01,		/*   LOGICAL_MAXIMUM (1)        */
	0x19, 0x01,		/*   USAGE MINIMUM (1)          */
	0x29, 0x05,		/*   USAGE MAXIMUM (5)          */
	0x91, 0x02,		/*   OUTPUT (Data, Variable, Absolute)  */
	0x95, 0x01,		/*   REPORT COUNT (1)           */
	0x75, 0x03,		/*   REPORT SIZE (3)            */
	0x91, 0x01,		/*   OUTPUT (Constant)          */
	0xC0
};

/* Mouse descriptor (2)     */
static const char mse_descriptor[] = {
	0x05, 0x01,		/*  USAGE_PAGE (Generic Desktop)        */
	0x09, 0x02,		/*  USAGE (Mouse)                       */
	0xA1, 0x01,		/*  COLLECTION (Application)            */
	0x85, 0x02,		/*    REPORT_ID = 2                     */
	0x09, 0x01,		/*    USAGE (pointer)                   */
	0xA1, 0x00,		/*    COLLECTION (physical)             */
	0x05, 0x09,		/*      USAGE_PAGE (buttons)            */
	0x19, 0x01,		/*      USAGE_MIN (1)                   */
	0x29, 0x10,		/*      USAGE_MAX (16)                  */
	0x15, 0x00,		/*      LOGICAL_MIN (0)                 */
	0x25, 0x01,		/*      LOGICAL_MAX (1)                 */
	0x95, 0x10,		/*      REPORT_COUNT (16)               */
	0x75, 0x01,		/*      REPORT_SIZE (1)                 */
	0x81, 0x02,		/*      INPUT (data var abs)            */
	0x05, 0x01,		/*      USAGE_PAGE (generic desktop)    */
	0x16, 0x01, 0xF8,	/*      LOGICAL_MIN (-2047)             */
	0x26, 0xFF, 0x07,	/*      LOGICAL_MAX (2047)              */
	0x75, 0x0C,		/*      REPORT_SIZE (12)                */
	0x95, 0x02,		/*      REPORT_COUNT (2)                */
	0x09, 0x30,		/*      USAGE (X)                       */
	0x09, 0x31,		/*      USAGE (Y)                       */
	0x81, 0x06,		/*      INPUT                           */
	0x15, 0x81,		/*      LOGICAL_MIN (-127)              */
	0x25, 0x7F,		/*      LOGICAL_MAX (127)               */
	0x75, 0x08,		/*      REPORT_SIZE (8)                 */
	0x95, 0x01,		/*      REPORT_COUNT (1)                */
	0x09, 0x38,		/*      USAGE (wheel)                   */
	0x81, 0x06,		/*      INPUT                           */
	0x05, 0x0C,		/*      USAGE_PAGE(consumer)            */
	0x0A, 0x38, 0x02,	/*      USAGE(AC Pan)                   */
	0x95, 0x01,		/*      REPORT_COUNT (1)                */
	0x81, 0x06,		/*      INPUT                           */
	0xC0,			/*    END_COLLECTION                    */
	0xC0,			/*  END_COLLECTION                      */
};

/* Consumer Control descriptor (3) */
static const char consumer_descriptor[] = {
	0x05, 0x0C,		/* USAGE_PAGE (Consumer Devices)       */
	0x09, 0x01,		/* USAGE (Consumer Control)            */
	0xA1, 0x01,		/* COLLECTION (Application)            */
	0x85, 0x03,		/* REPORT_ID = 3                       */
	0x75, 0x10,		/* REPORT_SIZE (16)                    */
	0x95, 0x02,		/* REPORT_COUNT (2)                    */
	0x15, 0x01,		/* LOGICAL_MIN (1)                     */
	0x26, 0x8C, 0x02,	/* LOGICAL_MAX (652)                   */
	0x19, 0x01,		/* USAGE_MIN (1)                       */
	0x2A, 0x8C, 0x02,	/* USAGE_MAX (652)                     */
	0x81, 0x00,		/* INPUT (Data Ary Abs)                */
	0xC0,			/* END_COLLECTION                      */
};				/*                                     */

/* System control descriptor (4) */
static const char syscontrol_descriptor[] = {
	0x05, 0x01,		/*   USAGE_PAGE (Generic Desktop)      */
	0x09, 0x80,		/*   USAGE (System Control)            */
	0xA1, 0x01,		/*   COLLECTION (Application)          */
	0x85, 0x04,		/*   REPORT_ID = 4                     */
	0x75, 0x02,		/*   REPORT_SIZE (2)                   */
	0x95, 0x01,		/*   REPORT_COUNT (1)                  */
	0x15, 0x01,		/*   LOGICAL_MIN (1)                   */
	0x25, 0x03,		/*   LOGICAL_MAX (3)                   */
	0x09, 0x82,		/*   USAGE (System Sleep)              */
	0x09, 0x81,		/*   USAGE (System Power Down)         */
	0x09, 0x83,		/*   USAGE (System Wake Up)            */
	0x81, 0x60,		/*   INPUT (Data Ary Abs NPrf Null)    */
	0x75, 0x06,		/*   REPORT_SIZE (6)                   */
	0x81, 0x03,		/*   INPUT (Cnst Var Abs)              */
	0xC0,			/*   END_COLLECTION                    */
};

/* Media descriptor (8) */
static const char media_descriptor[] = {
	0x06, 0xbc, 0xff,	/* Usage Page 0xffbc                   */
	0x09, 0x88,		/* Usage 0x0088                        */
	0xa1, 0x01,		/* BeginCollection                     */
	0x85, 0x08,		/*   Report ID 8                       */
	0x19, 0x01,		/*   Usage Min 0x0001                  */
	0x29, 0xff,		/*   Usage Max 0x00ff                  */
	0x15, 0x01,		/*   Logical Min 1                     */
	0x26, 0xff, 0x00,	/*   Logical Max 255                   */
	0x75, 0x08,		/*   Report Size 8                     */
	0x95, 0x01,		/*   Report Count 1                    */
	0x81, 0x00,		/*   Input                             */
	0xc0,			/* EndCollection                       */
};				/*                                     */

/* HIDPP descriptor */
static const char hidpp_descriptor[] = {
	0x06, 0x00, 0xff,	/* Usage Page (Vendor Defined Page 1)  */
	0x09, 0x01,		/* Usage (Vendor Usage 1)              */
	0xa1, 0x01,		/* Collection (Application)            */
	0x85, 0x10,		/*   Report ID (16)                    */
	0x75, 0x08,		/*   Report Size (8)                   */
	0x95, 0x06,		/*   Report Count (6)                  */
	0x15, 0x00,		/*   Logical Minimum (0)               */
	0x26, 0xff, 0x00,	/*   Logical Maximum (255)             */
	0x09, 0x01,		/*   Usage (Vendor Usage 1)            */
	0x81, 0x00,		/*   Input (Data,Arr,Abs)              */
	0x09, 0x01,		/*   Usage (Vendor Usage 1)            */
	0x91, 0x00,		/*   Output (Data,Arr,Abs)             */
	0xc0,			/* End Collection                      */
	0x06, 0x00, 0xff,	/* Usage Page (Vendor Defined Page 1)  */
	0x09, 0x02,		/* Usage (Vendor Usage 2)              */
	0xa1, 0x01,		/* Collection (Application)            */
	0x85, 0x11,		/*   Report ID (17)                    */
	0x75, 0x08,		/*   Report Size (8)                   */
	0x95, 0x13,		/*   Report Count (19)                 */
	0x15, 0x00,		/*   Logical Minimum (0)               */
	0x26, 0xff, 0x00,	/*   Logical Maximum (255)             */
	0x09, 0x02,		/*   Usage (Vendor Usage 2)            */
	0x81, 0x00,		/*   Input (Data,Arr,Abs)              */
	0x09, 0x02,		/*   Usage (Vendor Usage 2)            */
	0x91, 0x00,		/*   Output (Data,Arr,Abs)             */
	0xc0,			/* End Collection                      */
	0x06, 0x00, 0xff,	/* Usage Page (Vendor Defined Page 1)  */
	0x09, 0x04,		/* Usage (Vendor Usage 0x04)           */
	0xa1, 0x01,		/* Collection (Application)            */
	0x85, 0x20,		/*   Report ID (32)                    */
	0x75, 0x08,		/*   Report Size (8)                   */
	0x95, 0x0e,		/*   Report Count (14)                 */
	0x15, 0x00,		/*   Logical Minimum (0)               */
	0x26, 0xff, 0x00,	/*   Logical Maximum (255)             */
	0x09, 0x41,		/*   Usage (Vendor Usage 0x41)         */
	0x81, 0x00,		/*   Input (Data,Arr,Abs)              */
	0x09, 0x41,		/*   Usage (Vendor Usage 0x41)         */
	0x91, 0x00,		/*   Output (Data,Arr,Abs)             */
	0x85, 0x21,		/*   Report ID (33)                    */
	0x95, 0x1f,		/*   Report Count (31)                 */
	0x15, 0x00,		/*   Logical Minimum (0)               */
	0x26, 0xff, 0x00,	/*   Logical Maximum (255)             */
	0x09, 0x42,		/*   Usage (Vendor Usage 0x42)         */
	0x81, 0x00,		/*   Input (Data,Arr,Abs)              */
	0x09, 0x42,		/*   Usage (Vendor Usage 0x42)         */
	0x91, 0x00,		/*   Output (Data,Arr,Abs)             */
	0xc0,			/* End Collection                      */
};

/* Maximum size of all defined hid reports in bytes (including report id) */
#define MAX_REPORT_SIZE 8

/* Make sure all descriptors are present here */
#define MAX_RDESC_SIZE				\
	(sizeof(kbd_descriptor) +		\
	 sizeof(mse_descriptor) +		\
	 sizeof(consumer_descriptor) +		\
	 sizeof(syscontrol_descriptor) +	\
	 sizeof(media_descriptor) +	\
	 sizeof(hidpp_descriptor))

/* Number of possible hid report types that can be created by this driver.
 *
 * Right now, RF report types have the same report types (or report id's)
 * than the hid report created from those RF reports. In the future
 * this doesnt have to be true.
 *
 * For instance, RF report type 0x01 which has a size of 8 bytes, corresponds
 * to hid report id 0x01, this is standard keyboard. Same thing applies to mice
 * reports and consumer control, etc. If a new RF report is created, it doesn't
 * has to have the same report id as its corresponding hid report, so an
 * translation may have to take place for future report types.
 */
#define NUMBER_OF_HID_REPORTS 32
static const u8 hid_reportid_size_map[NUMBER_OF_HID_REPORTS] = {
	[1] = 8,		/* Standard keyboard */
	[2] = 8,		/* Standard mouse */
	[3] = 5,		/* Consumer control */
	[4] = 2,		/* System control */
	[8] = 2,		/* Media Center */
};


#define LOGITECH_DJ_INTERFACE_NUMBER 0x02

static struct hid_ll_driver logi_dj_ll_driver;

static int logi_dj_recv_query_paired_devices(struct dj_receiver_dev *djrcv_dev);

static void logi_dj_recv_destroy_djhid_device(struct dj_receiver_dev *djrcv_dev,
						struct dj_report *dj_report)
{
	/* Called in delayed work context */
	struct dj_device *dj_dev;
	unsigned long flags;

	spin_lock_irqsave(&djrcv_dev->lock, flags);
	dj_dev = djrcv_dev->paired_dj_devices[dj_report->device_index];
	djrcv_dev->paired_dj_devices[dj_report->device_index] = NULL;
	spin_unlock_irqrestore(&djrcv_dev->lock, flags);

	if (dj_dev != NULL) {
		hid_destroy_device(dj_dev->hdev);
		kfree(dj_dev);
	} else {
		dev_err(&djrcv_dev->hdev->dev, "%s: can't destroy a NULL device\n",
			__func__);
	}
}

static void logi_dj_recv_add_djhid_device(struct dj_receiver_dev *djrcv_dev,
					  struct dj_report *dj_report)
{
	/* Called in delayed work context */
	struct hid_device *djrcv_hdev = djrcv_dev->hdev;
	struct usb_interface *intf = to_usb_interface(djrcv_hdev->dev.parent);
	struct usb_device *usbdev = interface_to_usbdev(intf);
	struct hid_device *dj_hiddev;
	struct dj_device *dj_dev;

	/* Device index goes from 1 to 6, we need 3 bytes to store the
	 * semicolon, the index, and a null terminator
	 */
	unsigned char tmpstr[3];

	if (dj_report->report_params[DEVICE_PAIRED_PARAM_SPFUNCTION] &
	    SPFUNCTION_DEVICE_LIST_EMPTY) {
		dbg_hid("%s: device list is empty\n", __func__);
		djrcv_dev->querying_devices = false;
		return;
	}

	if (djrcv_dev->paired_dj_devices[dj_report->device_index]) {
		/* The device is already known. No need to reallocate it. */
		dbg_hid("%s: device is already known\n", __func__);
		return;
	}

	dj_hiddev = hid_allocate_device();
	if (IS_ERR(dj_hiddev)) {
		dev_err(&djrcv_hdev->dev, "%s: hid_allocate_device failed\n",
			__func__);
		return;
	}

	dj_hiddev->ll_driver = &logi_dj_ll_driver;

	dj_hiddev->dev.parent = &djrcv_hdev->dev;
	dj_hiddev->bus = BUS_USB;
	dj_hiddev->vendor = le16_to_cpu(usbdev->descriptor.idVendor);
	dj_hiddev->product =
		(dj_report->report_params[DEVICE_PAIRED_PARAM_EQUAD_ID_MSB]
									<< 8) |
		dj_report->report_params[DEVICE_PAIRED_PARAM_EQUAD_ID_LSB];
	snprintf(dj_hiddev->name, sizeof(dj_hiddev->name),
		"Logitech Unifying Device. Wireless PID:%04x",
		dj_hiddev->product);

	dj_hiddev->group = HID_GROUP_LOGITECH_DJ_DEVICE;

	usb_make_path(usbdev, dj_hiddev->phys, sizeof(dj_hiddev->phys));
	snprintf(tmpstr, sizeof(tmpstr), ":%d", dj_report->device_index);
	strlcat(dj_hiddev->phys, tmpstr, sizeof(dj_hiddev->phys));

	dj_dev = kzalloc(sizeof(struct dj_device), GFP_KERNEL);

	if (!dj_dev) {
		dev_err(&djrcv_hdev->dev, "%s: failed allocating dj_device\n",
			__func__);
		goto dj_device_allocate_fail;
	}

	dj_dev->reports_supported = get_unaligned_le32(
		dj_report->report_params + DEVICE_PAIRED_RF_REPORT_TYPE);
	dj_dev->hdev = dj_hiddev;
	dj_dev->dj_receiver_dev = djrcv_dev;
	dj_dev->device_index = dj_report->device_index;
	dj_hiddev->driver_data = dj_dev;

	djrcv_dev->paired_dj_devices[dj_report->device_index] = dj_dev;

	if (hid_add_device(dj_hiddev)) {
		dev_err(&djrcv_hdev->dev, "%s: failed adding dj_device\n",
			__func__);
		goto hid_add_device_fail;
	}

	return;

hid_add_device_fail:
	djrcv_dev->paired_dj_devices[dj_report->device_index] = NULL;
	kfree(dj_dev);
dj_device_allocate_fail:
	hid_destroy_device(dj_hiddev);
}

static void delayedwork_callback(struct work_struct *work)
{
	struct dj_receiver_dev *djrcv_dev =
		container_of(work, struct dj_receiver_dev, work);

	struct dj_report dj_report;
	unsigned long flags;
	int count;
	int retval;

	dbg_hid("%s\n", __func__);

	spin_lock_irqsave(&djrcv_dev->lock, flags);

	count = kfifo_out(&djrcv_dev->notif_fifo, &dj_report,
				sizeof(struct dj_report));

	if (count != sizeof(struct dj_report)) {
		dev_err(&djrcv_dev->hdev->dev, "%s: workitem triggered without "
			"notifications available\n", __func__);
		spin_unlock_irqrestore(&djrcv_dev->lock, flags);
		return;
	}

	if (!kfifo_is_empty(&djrcv_dev->notif_fifo)) {
		if (schedule_work(&djrcv_dev->work) == 0) {
			dbg_hid("%s: did not schedule the work item, was "
				"already queued\n", __func__);
		}
	}

	spin_unlock_irqrestore(&djrcv_dev->lock, flags);

	switch (dj_report.report_type) {
	case REPORT_TYPE_NOTIF_DEVICE_PAIRED:
		logi_dj_recv_add_djhid_device(djrcv_dev, &dj_report);
		break;
	case REPORT_TYPE_NOTIF_DEVICE_UNPAIRED:
		logi_dj_recv_destroy_djhid_device(djrcv_dev, &dj_report);
		break;
	default:
	/* A normal report (i. e. not belonging to a pair/unpair notification)
	 * arriving here, means that the report arrived but we did not have a
	 * paired dj_device associated to the report's device_index, this
	 * means that the original "device paired" notification corresponding
	 * to this dj_device never arrived to this driver. The reason is that
	 * hid-core discards all packets coming from a device while probe() is
	 * executing. */
	if (!djrcv_dev->paired_dj_devices[dj_report.device_index]) {
		/* ok, we don't know the device, just re-ask the
		 * receiver for the list of connected devices. */
		retval = logi_dj_recv_query_paired_devices(djrcv_dev);
		if (!retval) {
			/* everything went fine, so just leave */
			break;
		}
		dev_err(&djrcv_dev->hdev->dev,
			"%s:logi_dj_recv_query_paired_devices "
			"error:%d\n", __func__, retval);
		}
		dbg_hid("%s: unexpected report type\n", __func__);
	}
}

static void logi_dj_recv_queue_notification(struct dj_receiver_dev *djrcv_dev,
					   struct dj_report *dj_report)
{
	/* We are called from atomic context (tasklet && djrcv->lock held) */

	kfifo_in(&djrcv_dev->notif_fifo, dj_report, sizeof(struct dj_report));

	if (schedule_work(&djrcv_dev->work) == 0) {
		dbg_hid("%s: did not schedule the work item, was already "
			"queued\n", __func__);
	}
}

static void logi_dj_recv_forward_null_report(struct dj_receiver_dev *djrcv_dev,
					     struct dj_report *dj_report)
{
	/* We are called from atomic context (tasklet && djrcv->lock held) */
	unsigned int i;
	u8 reportbuffer[MAX_REPORT_SIZE];
	struct dj_device *djdev;

	djdev = djrcv_dev->paired_dj_devices[dj_report->device_index];

	memset(reportbuffer, 0, sizeof(reportbuffer));

	for (i = 0; i < NUMBER_OF_HID_REPORTS; i++) {
		if (djdev->reports_supported & (1 << i)) {
			reportbuffer[0] = i;
			if (hid_input_report(djdev->hdev,
					     HID_INPUT_REPORT,
					     reportbuffer,
					     hid_reportid_size_map[i], 1)) {
				dbg_hid("hid_input_report error sending null "
					"report\n");
			}
		}
	}
}

static void logi_dj_recv_forward_report(struct dj_receiver_dev *djrcv_dev,
					struct dj_report *dj_report)
{
	/* We are called from atomic context (tasklet && djrcv->lock held) */
	struct dj_device *dj_device;

	dj_device = djrcv_dev->paired_dj_devices[dj_report->device_index];

	if ((dj_report->report_type > ARRAY_SIZE(hid_reportid_size_map) - 1) ||
	    (hid_reportid_size_map[dj_report->report_type] == 0)) {
		dbg_hid("invalid report type:%x\n", dj_report->report_type);
		return;
	}

	if (hid_input_report(dj_device->hdev,
			HID_INPUT_REPORT, &dj_report->report_type,
			hid_reportid_size_map[dj_report->report_type], 1)) {
		dbg_hid("hid_input_report error\n");
	}
}

static void logi_dj_recv_forward_hidpp(struct dj_device *dj_dev, u8 *data,
				       int size)
{
	/* We are called from atomic context (tasklet && djrcv->lock held) */
	if (hid_input_report(dj_dev->hdev, HID_INPUT_REPORT, data, size, 1))
		dbg_hid("hid_input_report error\n");
}

static int logi_dj_recv_send_report(struct dj_receiver_dev *djrcv_dev,
				    struct dj_report *dj_report)
{
	struct hid_device *hdev = djrcv_dev->hdev;
	struct hid_report *report;
	struct hid_report_enum *output_report_enum;
	u8 *data = (u8 *)(&dj_report->device_index);
	unsigned int i;

	output_report_enum = &hdev->report_enum[HID_OUTPUT_REPORT];
	report = output_report_enum->report_id_hash[REPORT_ID_DJ_SHORT];

	if (!report) {
		dev_err(&hdev->dev, "%s: unable to find dj report\n", __func__);
		return -ENODEV;
	}

	for (i = 0; i < DJREPORT_SHORT_LENGTH - 1; i++)
		report->field[0]->value[i] = data[i];

	hid_hw_request(hdev, report, HID_REQ_SET_REPORT);

	return 0;
}

static int logi_dj_recv_query_paired_devices(struct dj_receiver_dev *djrcv_dev)
{
	struct dj_report *dj_report;
	int retval;

	/* no need to protect djrcv_dev->querying_devices */
	if (djrcv_dev->querying_devices)
		return 0;

	dj_report = kzalloc(sizeof(struct dj_report), GFP_KERNEL);
	if (!dj_report)
		return -ENOMEM;
	dj_report->report_id = REPORT_ID_DJ_SHORT;
	dj_report->device_index = 0xFF;
	dj_report->report_type = REPORT_TYPE_CMD_GET_PAIRED_DEVICES;
	retval = logi_dj_recv_send_report(djrcv_dev, dj_report);
	kfree(dj_report);
	return retval;
}


static int logi_dj_recv_switch_to_dj_mode(struct dj_receiver_dev *djrcv_dev,
					  unsigned timeout)
{
	struct hid_device *hdev = djrcv_dev->hdev;
	struct dj_report *dj_report;
	u8 *buf;
	int retval;

	dj_report = kzalloc(sizeof(struct dj_report), GFP_KERNEL);
	if (!dj_report)
		return -ENOMEM;
	dj_report->report_id = REPORT_ID_DJ_SHORT;
	dj_report->device_index = 0xFF;
	dj_report->report_type = REPORT_TYPE_CMD_SWITCH;
	dj_report->report_params[CMD_SWITCH_PARAM_DEVBITFIELD] = 0x3F;
	dj_report->report_params[CMD_SWITCH_PARAM_TIMEOUT_SECONDS] = (u8)timeout;
	retval = logi_dj_recv_send_report(djrcv_dev, dj_report);

	/*
	 * Ugly sleep to work around a USB 3.0 bug when the receiver is still
	 * processing the "switch-to-dj" command while we send an other command.
	 * 50 msec should gives enough time to the receiver to be ready.
	 */
	msleep(50);

	/*
	 * Magical bits to set up hidpp notifications when the dj devices
	 * are connected/disconnected.
	 *
	 * We can reuse dj_report because HIDPP_REPORT_SHORT_LENGTH is smaller
	 * than DJREPORT_SHORT_LENGTH.
	 */
	buf = (u8 *)dj_report;

	memset(buf, 0, HIDPP_REPORT_SHORT_LENGTH);

	buf[0] = REPORT_ID_HIDPP_SHORT;
	buf[1] = 0xFF;
	buf[2] = 0x80;
	buf[3] = 0x00;
	buf[4] = 0x00;
	buf[5] = 0x09;
	buf[6] = 0x00;

	hid_hw_raw_request(hdev, REPORT_ID_HIDPP_SHORT, buf,
			HIDPP_REPORT_SHORT_LENGTH, HID_OUTPUT_REPORT,
			HID_REQ_SET_REPORT);

	kfree(dj_report);
	return retval;
}


static int logi_dj_ll_open(struct hid_device *hid)
{
	dbg_hid("%s:%s\n", __func__, hid->phys);
	return 0;

}

static void logi_dj_ll_close(struct hid_device *hid)
{
	dbg_hid("%s:%s\n", __func__, hid->phys);
}

static u8 unifying_name_query[]  = {0x10, 0xff, 0x83, 0xb5, 0x40, 0x00, 0x00};
static u8 unifying_name_answer[] = {0x11, 0xff, 0x83, 0xb5};

static int logi_dj_ll_raw_request(struct hid_device *hid,
				  unsigned char reportnum, __u8 *buf,
				  size_t count, unsigned char report_type,
				  int reqtype)
{
	struct dj_device *djdev = hid->driver_data;
	struct dj_receiver_dev *djrcv_dev = djdev->dj_receiver_dev;
	u8 *out_buf;
	int ret;

	if ((buf[0] == REPORT_ID_HIDPP_SHORT) ||
	    (buf[0] == REPORT_ID_HIDPP_LONG)) {
		if (count < 2)
			return -EINVAL;

		/* special case where we should not overwrite
		 * the device_index */
		if (count == 7 && !memcmp(buf, unifying_name_query,
					  sizeof(unifying_name_query)))
			buf[4] |= djdev->device_index - 1;
		else
			buf[1] = djdev->device_index;
		return hid_hw_raw_request(djrcv_dev->hdev, reportnum, buf,
				count, report_type, reqtype);
	}

	if (buf[0] != REPORT_TYPE_LEDS)
		return -EINVAL;

	out_buf = kzalloc(DJREPORT_SHORT_LENGTH, GFP_ATOMIC);
	if (!out_buf)
		return -ENOMEM;

	if (count > DJREPORT_SHORT_LENGTH - 2)
		count = DJREPORT_SHORT_LENGTH - 2;

	out_buf[0] = REPORT_ID_DJ_SHORT;
	out_buf[1] = djdev->device_index;
	memcpy(out_buf + 2, buf, count);

	ret = hid_hw_raw_request(djrcv_dev->hdev, out_buf[0], out_buf,
		DJREPORT_SHORT_LENGTH, report_type, reqtype);

	kfree(out_buf);
	return ret;
}

static void rdcat(char *rdesc, unsigned int *rsize, const char *data, unsigned int size)
{
	memcpy(rdesc + *rsize, data, size);
	*rsize += size;
}

static int logi_dj_ll_parse(struct hid_device *hid)
{
	struct dj_device *djdev = hid->driver_data;
	unsigned int rsize = 0;
	char *rdesc;
	int retval;

	dbg_hid("%s\n", __func__);

	djdev->hdev->version = 0x0111;
	djdev->hdev->country = 0x00;

	rdesc = kmalloc(MAX_RDESC_SIZE, GFP_KERNEL);
	if (!rdesc)
		return -ENOMEM;

	if (djdev->reports_supported & STD_KEYBOARD) {
		dbg_hid("%s: sending a kbd descriptor, reports_supported: %x\n",
			__func__, djdev->reports_supported);
		rdcat(rdesc, &rsize, kbd_descriptor, sizeof(kbd_descriptor));
	}

	if (djdev->reports_supported & STD_MOUSE) {
		dbg_hid("%s: sending a mouse descriptor, reports_supported: "
			"%x\n", __func__, djdev->reports_supported);
		rdcat(rdesc, &rsize, mse_descriptor, sizeof(mse_descriptor));
	}

	if (djdev->reports_supported & MULTIMEDIA) {
		dbg_hid("%s: sending a multimedia report descriptor: %x\n",
			__func__, djdev->reports_supported);
		rdcat(rdesc, &rsize, consumer_descriptor, sizeof(consumer_descriptor));
	}

	if (djdev->reports_supported & POWER_KEYS) {
		dbg_hid("%s: sending a power keys report descriptor: %x\n",
			__func__, djdev->reports_supported);
		rdcat(rdesc, &rsize, syscontrol_descriptor, sizeof(syscontrol_descriptor));
	}

	if (djdev->reports_supported & MEDIA_CENTER) {
		dbg_hid("%s: sending a media center report descriptor: %x\n",
			__func__, djdev->reports_supported);
		rdcat(rdesc, &rsize, media_descriptor, sizeof(media_descriptor));
	}

	if (djdev->reports_supported & KBD_LEDS) {
		dbg_hid("%s: need to send kbd leds report descriptor: %x\n",
			__func__, djdev->reports_supported);
	}

	rdcat(rdesc, &rsize, hidpp_descriptor, sizeof(hidpp_descriptor));

	retval = hid_parse_report(hid, rdesc, rsize);
	kfree(rdesc);

	return retval;
}

static int logi_dj_ll_start(struct hid_device *hid)
{
	dbg_hid("%s\n", __func__);
	return 0;
}

static void logi_dj_ll_stop(struct hid_device *hid)
{
	dbg_hid("%s\n", __func__);
}


static struct hid_ll_driver logi_dj_ll_driver = {
	.parse = logi_dj_ll_parse,
	.start = logi_dj_ll_start,
	.stop = logi_dj_ll_stop,
	.open = logi_dj_ll_open,
	.close = logi_dj_ll_close,
	.raw_request = logi_dj_ll_raw_request,
};

static int logi_dj_dj_event(struct hid_device *hdev,
			     struct hid_report *report, u8 *data,
			     int size)
{
	struct dj_receiver_dev *djrcv_dev = hid_get_drvdata(hdev);
	struct dj_report *dj_report = (struct dj_report *) data;
	unsigned long flags;

	/*
	 * Here we receive all data coming from iface 2, there are 3 cases:
	 *
	 * 1) Data is intended for this driver i. e. data contains arrival,
	 * departure, etc notifications, in which case we queue them for delayed
	 * processing by the work queue. We return 1 to hid-core as no further
	 * processing is required from it.
	 *
	 * 2) Data informs a connection change, if the change means rf link
	 * loss, then we must send a null report to the upper layer to discard
	 * potentially pressed keys that may be repeated forever by the input
	 * layer. Return 1 to hid-core as no further processing is required.
	 *
	 * 3) Data is an actual input event from a paired DJ device in which
	 * case we forward it to the correct hid device (via hid_input_report()
	 * ) and return 1 so hid-core does not anything else with it.
	 */

	if ((dj_report->device_index < DJ_DEVICE_INDEX_MIN) ||
	    (dj_report->device_index > DJ_DEVICE_INDEX_MAX)) {
		/*
		 * Device index is wrong, bail out.
		 * This driver can ignore safely the receiver notifications,
		 * so ignore those reports too.
		 */
		if (dj_report->device_index != DJ_RECEIVER_INDEX)
			dev_err(&hdev->dev, "%s: invalid device index:%d\n",
				__func__, dj_report->device_index);
		return false;
	}

	spin_lock_irqsave(&djrcv_dev->lock, flags);

	if (!djrcv_dev->paired_dj_devices[dj_report->device_index]) {
		/* received an event for an unknown device, bail out */
		logi_dj_recv_queue_notification(djrcv_dev, dj_report);
		goto out;
	}

	switch (dj_report->report_type) {
	case REPORT_TYPE_NOTIF_DEVICE_PAIRED:
		/* pairing notifications are handled above the switch */
		break;
	case REPORT_TYPE_NOTIF_DEVICE_UNPAIRED:
		logi_dj_recv_queue_notification(djrcv_dev, dj_report);
		break;
	case REPORT_TYPE_NOTIF_CONNECTION_STATUS:
		if (dj_report->report_params[CONNECTION_STATUS_PARAM_STATUS] ==
		    STATUS_LINKLOSS) {
			logi_dj_recv_forward_null_report(djrcv_dev, dj_report);
		}
		break;
	default:
		logi_dj_recv_forward_report(djrcv_dev, dj_report);
	}

out:
	spin_unlock_irqrestore(&djrcv_dev->lock, flags);

	return true;
}

static int logi_dj_hidpp_event(struct hid_device *hdev,
			     struct hid_report *report, u8 *data,
			     int size)
{
	struct dj_receiver_dev *djrcv_dev = hid_get_drvdata(hdev);
	struct dj_report *dj_report = (struct dj_report *) data;
	unsigned long flags;
	u8 device_index = dj_report->device_index;

	if (device_index == HIDPP_RECEIVER_INDEX) {
		/* special case were the device wants to know its unifying
		 * name */
		if (size == HIDPP_REPORT_LONG_LENGTH &&
		    !memcmp(data, unifying_name_answer,
			    sizeof(unifying_name_answer)) &&
		    ((data[4] & 0xF0) == 0x40))
			device_index = (data[4] & 0x0F) + 1;
		else
			return false;
	}

	/*
	 * Data is from the HID++ collection, in this case, we forward the
	 * data to the corresponding child dj device and return 0 to hid-core
	 * so he data also goes to the hidraw device of the receiver. This
	 * allows a user space application to implement the full HID++ routing
	 * via the receiver.
	 */

	if ((device_index < DJ_DEVICE_INDEX_MIN) ||
	    (device_index > DJ_DEVICE_INDEX_MAX)) {
		/*
		 * Device index is wrong, bail out.
		 * This driver can ignore safely the receiver notifications,
		 * so ignore those reports too.
		 */
		dev_err(&hdev->dev, "%s: invalid device index:%d\n",
				__func__, dj_report->device_index);
		return false;
	}

	spin_lock_irqsave(&djrcv_dev->lock, flags);

	if (!djrcv_dev->paired_dj_devices[device_index])
		/* received an event for an unknown device, bail out */
		goto out;

	logi_dj_recv_forward_hidpp(djrcv_dev->paired_dj_devices[device_index],
				   data, size);

out:
	spin_unlock_irqrestore(&djrcv_dev->lock, flags);

	return false;
}

static int logi_dj_raw_event(struct hid_device *hdev,
			     struct hid_report *report, u8 *data,
			     int size)
{
	dbg_hid("%s, size:%d\n", __func__, size);

	switch (data[0]) {
	case REPORT_ID_DJ_SHORT:
		if (size != DJREPORT_SHORT_LENGTH) {
			dev_err(&hdev->dev, "DJ report of bad size (%d)", size);
			return false;
		}
		return logi_dj_dj_event(hdev, report, data, size);
	case REPORT_ID_HIDPP_SHORT:
		if (size != HIDPP_REPORT_SHORT_LENGTH) {
			dev_err(&hdev->dev,
				"Short HID++ report of bad size (%d)", size);
			return false;
		}
		return logi_dj_hidpp_event(hdev, report, data, size);
	case REPORT_ID_HIDPP_LONG:
		if (size != HIDPP_REPORT_LONG_LENGTH) {
			dev_err(&hdev->dev,
				"Long HID++ report of bad size (%d)", size);
			return false;
		}
		return logi_dj_hidpp_event(hdev, report, data, size);
	}

	return false;
}

static int logi_dj_probe(struct hid_device *hdev,
			 const struct hid_device_id *id)
{
	struct usb_interface *intf = to_usb_interface(hdev->dev.parent);
	struct dj_receiver_dev *djrcv_dev;
	int retval;

	dbg_hid("%s called for ifnum %d\n", __func__,
		intf->cur_altsetting->desc.bInterfaceNumber);

	/* Ignore interfaces 0 and 1, they will not carry any data, dont create
	 * any hid_device for them */
	if (intf->cur_altsetting->desc.bInterfaceNumber !=
	    LOGITECH_DJ_INTERFACE_NUMBER) {
		dbg_hid("%s: ignoring ifnum %d\n", __func__,
			intf->cur_altsetting->desc.bInterfaceNumber);
		return -ENODEV;
	}

	/* Treat interface 2 */

	djrcv_dev = kzalloc(sizeof(struct dj_receiver_dev), GFP_KERNEL);
	if (!djrcv_dev) {
		dev_err(&hdev->dev,
			"%s:failed allocating dj_receiver_dev\n", __func__);
		return -ENOMEM;
	}
	djrcv_dev->hdev = hdev;
	INIT_WORK(&djrcv_dev->work, delayedwork_callback);
	spin_lock_init(&djrcv_dev->lock);
	if (kfifo_alloc(&djrcv_dev->notif_fifo,
			DJ_MAX_NUMBER_NOTIFICATIONS * sizeof(struct dj_report),
			GFP_KERNEL)) {
		dev_err(&hdev->dev,
			"%s:failed allocating notif_fifo\n", __func__);
		kfree(djrcv_dev);
		return -ENOMEM;
	}
	hid_set_drvdata(hdev, djrcv_dev);

	/* Call  to usbhid to fetch the HID descriptors of interface 2 and
	 * subsequently call to the hid/hid-core to parse the fetched
	 * descriptors, this will in turn create the hidraw and hiddev nodes
	 * for interface 2 of the receiver */
	retval = hid_parse(hdev);
	if (retval) {
		dev_err(&hdev->dev,
			"%s:parse of interface 2 failed\n", __func__);
		goto hid_parse_fail;
	}

	if (!hid_validate_values(hdev, HID_OUTPUT_REPORT, REPORT_ID_DJ_SHORT,
				 0, DJREPORT_SHORT_LENGTH - 1)) {
		retval = -ENODEV;
		goto hid_parse_fail;
	}

	/* Starts the usb device and connects to upper interfaces hiddev and
	 * hidraw */
	retval = hid_hw_start(hdev, HID_CONNECT_DEFAULT);
	if (retval) {
		dev_err(&hdev->dev,
			"%s:hid_hw_start returned error\n", __func__);
		goto hid_hw_start_fail;
	}

	retval = logi_dj_recv_switch_to_dj_mode(djrcv_dev, 0);
	if (retval < 0) {
		dev_err(&hdev->dev,
			"%s:logi_dj_recv_switch_to_dj_mode returned error:%d\n",
			__func__, retval);
		goto switch_to_dj_mode_fail;
	}

	/* This is enabling the polling urb on the IN endpoint */
	retval = hid_hw_open(hdev);
	if (retval < 0) {
		dev_err(&hdev->dev, "%s:hid_hw_open returned error:%d\n",
			__func__, retval);
		goto llopen_failed;
	}

	/* Allow incoming packets to arrive: */
	hid_device_io_start(hdev);

	retval = logi_dj_recv_query_paired_devices(djrcv_dev);
	if (retval < 0) {
		dev_err(&hdev->dev, "%s:logi_dj_recv_query_paired_devices "
			"error:%d\n", __func__, retval);
		goto logi_dj_recv_query_paired_devices_failed;
	}

	return retval;

logi_dj_recv_query_paired_devices_failed:
	hid_hw_close(hdev);

llopen_failed:
switch_to_dj_mode_fail:
	hid_hw_stop(hdev);

hid_hw_start_fail:
hid_parse_fail:
	kfifo_free(&djrcv_dev->notif_fifo);
	kfree(djrcv_dev);
	hid_set_drvdata(hdev, NULL);
	return retval;

}

#ifdef CONFIG_PM
static int logi_dj_reset_resume(struct hid_device *hdev)
{
	int retval;
	struct dj_receiver_dev *djrcv_dev = hid_get_drvdata(hdev);

	retval = logi_dj_recv_switch_to_dj_mode(djrcv_dev, 0);
	if (retval < 0) {
		dev_err(&hdev->dev,
			"%s:logi_dj_recv_switch_to_dj_mode returned error:%d\n",
			__func__, retval);
	}

	return 0;
}
#endif

static void logi_dj_remove(struct hid_device *hdev)
{
	struct dj_receiver_dev *djrcv_dev = hid_get_drvdata(hdev);
	struct dj_device *dj_dev;
	int i;

	dbg_hid("%s\n", __func__);

	cancel_work_sync(&djrcv_dev->work);

	hid_hw_close(hdev);
	hid_hw_stop(hdev);

	/* I suppose that at this point the only context that can access
	 * the djrecv_data is this thread as the work item is guaranteed to
	 * have finished and no more raw_event callbacks should arrive after
	 * the remove callback was triggered so no locks are put around the
	 * code below */
	for (i = 0; i < (DJ_MAX_PAIRED_DEVICES + DJ_DEVICE_INDEX_MIN); i++) {
		dj_dev = djrcv_dev->paired_dj_devices[i];
		if (dj_dev != NULL) {
			hid_destroy_device(dj_dev->hdev);
			kfree(dj_dev);
			djrcv_dev->paired_dj_devices[i] = NULL;
		}
	}

	kfifo_free(&djrcv_dev->notif_fifo);
	kfree(djrcv_dev);
	hid_set_drvdata(hdev, NULL);
}

static const struct hid_device_id logi_dj_receivers[] = {
	{HID_USB_DEVICE(USB_VENDOR_ID_LOGITECH,
		USB_DEVICE_ID_LOGITECH_UNIFYING_RECEIVER)},
	{HID_USB_DEVICE(USB_VENDOR_ID_LOGITECH,
		USB_DEVICE_ID_LOGITECH_UNIFYING_RECEIVER_2)},
	{}
};

MODULE_DEVICE_TABLE(hid, logi_dj_receivers);

static struct hid_driver logi_djreceiver_driver = {
	.name = "logitech-djreceiver",
	.id_table = logi_dj_receivers,
	.probe = logi_dj_probe,
	.remove = logi_dj_remove,
	.raw_event = logi_dj_raw_event,
#ifdef CONFIG_PM
	.reset_resume = logi_dj_reset_resume,
#endif
};

module_hid_driver(logi_djreceiver_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Logitech");
MODULE_AUTHOR("Nestor Lopez Casado");
MODULE_AUTHOR("nlopezcasad@logitech.com");
