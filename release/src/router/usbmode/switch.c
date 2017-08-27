#include <unistd.h>
#include "switch.h"

enum {
	DATA_MODE,
	DATA_MSG,
	DATA_INTERFACE,
	DATA_MSG_EP,
	DATA_RES_EP,
	DATA_RESPONSE,
	DATA_RELEASE_DELAY,
	DATA_CONFIG,
	DATA_ALT,
	DATA_DEV_CLASS,
	__DATA_MAX
};

static void detach_driver(struct usbdev_data *data)
{
	libusb_detach_kernel_driver(data->devh, data->interface);
}

struct msg_entry {
	char *data;
	int len;
};

static int send_msg(struct usbdev_data *data, struct msg_entry *msg)
{
	int transferred;

	return libusb_bulk_transfer(data->devh, data->msg_endpoint,
				    (void *) msg->data, msg->len,
				    &transferred, 3000);
}

static int read_response(struct usbdev_data *data, int len)
{
	unsigned char *buf;
	int ret, transferred;

	if (len < 13)
		len = 13;
	buf = alloca(len);
	ret = libusb_bulk_transfer(data->devh, data->response_endpoint,
				   buf, len, &transferred, 3000);
	libusb_bulk_transfer(data->devh, data->response_endpoint,
			     buf, 13, &transferred, 100);
	return ret;
}

static void send_messages(struct usbdev_data *data, struct msg_entry *msg, int n_msg)
{
	int i, len;

	libusb_claim_interface(data->devh, data->interface);
	libusb_clear_halt(data->devh, data->msg_endpoint);

	for (i = 0; i < n_msg; i++) {
		if (send_msg(data, &msg[i])) {
			fprintf(stderr, "Failed to send switch message\n");
			continue;
		}

		if (!data->need_response)
			continue;

		if (!memcmp(msg[i].data, "\x55\x53\x42\x43", 4))
			len = 13;
		else
			len = msg[i].len;

		if (read_response(data, len))
			return;
	}

	libusb_clear_halt(data->devh, data->msg_endpoint);
	libusb_clear_halt(data->devh, data->response_endpoint);

	usleep(200000);

	if (data->release_delay)
		usleep(data->release_delay * 1000);

	libusb_release_interface(data->devh, data->interface);
	return;
}

static void send_config_messages(struct usbdev_data *data, struct blob_attr *attr)
{
	struct blob_attr *cur;
	int rem, n_msg = 0;
	struct msg_entry *msg;

	blobmsg_for_each_attr(cur, attr, rem)
		n_msg++;

	msg = alloca(n_msg * sizeof(*msg));
	n_msg = 0;
	blobmsg_for_each_attr(cur, attr, rem) {
		int msg_nr;

		if (blobmsg_type(cur) != BLOBMSG_TYPE_INT32) {
			fprintf(stderr, "Invalid data in message list\n");
			return;
		}

		msg_nr = blobmsg_get_u32(cur);
		if (msg_nr >= n_messages) {
			fprintf(stderr, "Message index out of range!\n");
			return;
		}

		msg[n_msg].data = messages[msg_nr];
		msg[n_msg++].len = message_len[msg_nr];
	}

	send_messages(data, msg, n_msg);
}

static void handle_generic(struct usbdev_data *data, struct blob_attr **tb)
{
	detach_driver(data);
	send_config_messages(data, tb[DATA_MSG]);
}

static void send_control_packet(struct usbdev_data *data, uint8_t type, uint8_t req,
				uint16_t val, uint16_t idx, int len)
{
	unsigned char *buffer = alloca(len ? len : 1);

	libusb_control_transfer(data->devh, type, req, val, idx, buffer, len, 1000);
}

static void handle_huawei(struct usbdev_data *data, struct blob_attr **tb)
{
	int type = LIBUSB_REQUEST_TYPE_STANDARD | LIBUSB_RECIPIENT_DEVICE;
	send_control_packet(data, type, LIBUSB_REQUEST_SET_FEATURE, 1, 0, 0);
}

static void handle_huaweinew(struct usbdev_data *data, struct blob_attr **tb)
{
	static struct msg_entry msgs[] = {
		{
			"\x55\x53\x42\x43\x12\x34\x56\x78\x00\x00\x00\x00\x00\x00\x00\x11"
			"\x06\x20\x00\x00\x01\x01\x00\x01\x00\x00\x00\x00\x00\x00\x00", 31
		}
	};

	detach_driver(data);
	data->need_response = false;
	send_messages(data, msgs, ARRAY_SIZE(msgs));
}

static void handle_standardeject(struct usbdev_data *data, struct blob_attr **tb)
{
	static struct msg_entry msgs[] = {
		{
			"\x55\x53\x42\x43\x12\x34\x56\x78\x00\x00\x00\x00\x00\x00\x06\x1e"
			"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 31
		}, {
			"\x55\x53\x42\x43\x12\x34\x56\x79\x00\x00\x00\x00\x00\x00\x06\x1b"
			"\x00\x00\x00\x02\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 31
		}, {
			"\x55\x53\x42\x43\x12\x34\x56\x78\x00\x00\x00\x00\x00\x01\x06\x1e"
			"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 31
		}, {
			"\x55\x53\x42\x43\x12\x34\x56\x79\x00\x00\x00\x00\x00\x01\x06\x1b"
			"\x00\x00\x00\x02\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 31
		}
	};

	detach_driver(data);
	data->need_response = true;
	send_messages(data, msgs, ARRAY_SIZE(msgs));
}

static void handle_sierra(struct usbdev_data *data, struct blob_attr **tb)
{
	int type = LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE;
	send_control_packet(data, type, LIBUSB_REQUEST_SET_INTERFACE, 1, 0, 0);
}

static void handle_sony(struct usbdev_data *data, struct blob_attr **tb)
{
	int type = LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_ENDPOINT_IN;
	int i;

	detach_driver(data);
	send_control_packet(data, type, 0x11, 2, 0, 3);

	libusb_close(data->devh);
	sleep(5);

	for (i = 0; i < 25; i++) {
		data->devh = libusb_open_device_with_vid_pid(usb,
			data->desc.idVendor, data->desc.idProduct);
		if (data->devh)
			break;
	}

	send_control_packet(data, type, 0x11, 2, 0, 3);
}

static void handle_qisda(struct usbdev_data *data, struct blob_attr **tb)
{
	static unsigned char buffer[] = "\x05\x8c\x04\x08\xa0\xee\x20\x00\x5c\x01\x04\x08\x98\xcd\xea\xbf";
	int type = LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE;

	libusb_control_transfer(data->devh, type, 0x04, 0, 0, buffer, 16, 1000);
}

static void handle_gct(struct usbdev_data *data, struct blob_attr **tb)
{
	int type = LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_RECIPIENT_INTERFACE | LIBUSB_ENDPOINT_IN;

	detach_driver(data);

	if (libusb_claim_interface(data->devh, data->interface))
	    return;

	send_control_packet(data, type, 0xa0, 0, data->interface, 1);
	send_control_packet(data, type, 0xfe, 0, data->interface, 1);

	libusb_release_interface(data->devh, data->interface);
}

static void handle_kobil(struct usbdev_data *data, struct blob_attr **tb)
{
	int type = LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE | LIBUSB_ENDPOINT_IN;

	detach_driver(data);
	send_control_packet(data, type, 0x88, 0, 0, 8);
}

static void handle_sequans(struct usbdev_data *data, struct blob_attr **tb)
{
	int type = LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE;
	send_control_packet(data, type, LIBUSB_REQUEST_SET_INTERFACE, 2, 0, 0);
}

static void mobile_action_interrupt_msg(struct usbdev_data *data, void *msg, int n_in)
{
	unsigned char *buf = alloca(8);
	int ep_out = 0x02, ep_in = 0x81;
	int transferred;
	int i;

	if (msg)
		libusb_interrupt_transfer(data->devh, ep_out, msg, 8, &transferred, 1000);
	for (i = 0; i < n_in; i++)
		libusb_interrupt_transfer(data->devh, ep_in, buf, 8, &transferred, 1000);
}

static void handle_mobile_action(struct usbdev_data *data, struct blob_attr **tb)
{
	int type = LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_RECIPIENT_INTERFACE;
	char *msg[] = {
		"\xb0\x04\x00\x00\x02\x90\x26\x86",
		"\x37\x01\xfe\xdb\xc1\x33\x1f\x83",
		"\x37\x0e\xb5\x9d\x3b\x8a\x91\x51",
		"\x34\x87\xba\x0d\xfc\x8a\x91\x51",
		"\x37\x01\xfe\xdb\xc1\x33\x1f\x83",
		"\x37\x0e\xb5\x9d\x3b\x8a\x91\x51",
		"\x34\x87\xba\x0d\xfc\x8a\x91\x51",
		"\x33\x04\xfe\x00\xf4\x6c\x1f\xf0",
		"\x32\x07\xfe\xf0\x29\xb9\x3a\xf0"
	};
	int i;

	for (i = 0; i < 2; i++)
		libusb_control_transfer(data->devh, type, 0x09, 0x0300, 0, (void *) msg[0], 8, 1000);
	mobile_action_interrupt_msg(data, NULL, 2);
	mobile_action_interrupt_msg(data, msg[1], 1);
	mobile_action_interrupt_msg(data, msg[2], 1);
	mobile_action_interrupt_msg(data, msg[3], 63);
	mobile_action_interrupt_msg(data, msg[4], 1);
	mobile_action_interrupt_msg(data, msg[5], 1);
	mobile_action_interrupt_msg(data, msg[6], 73);
	mobile_action_interrupt_msg(data, msg[7], 1);
	mobile_action_interrupt_msg(data, msg[8], 1);
}

static void handle_cisco(struct usbdev_data *data, struct blob_attr **tb)
{
	static struct msg_entry msgs[] = {
		{
			"\x55\x53\x42\x43\xf8\x3b\xcd\x81\x00\x02\x00\x00\x80\x00\x0a\xfd"
			"\x00\x00\x00\x03\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00", 31
		}, {
			"\x55\x53\x42\x43\x98\x43\x00\x82\x00\x02\x00\x00\x80\x00\x0a\xfd"
			"\x00\x00\x00\x07\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00", 31
		}, {
			"\x55\x53\x42\x43\x98\x43\x00\x82\x00\x00\x00\x00\x00\x00\x0a\xfd"
			"\x00\x01\x00\x07\x10\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 31
		}, {
			"\x55\x53\x42\x43\x98\x43\x00\x82\x00\x02\x00\x00\x80\x00\x0a\xfd"
			"\x00\x02\x00\x23\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00", 31
		}, {
			"\x55\x53\x42\x43\x98\x43\x00\x82\x00\x00\x00\x00\x00\x00\x0a\xfd"
			"\x00\x03\x00\x23\x82\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 31
		}, {
			"\x55\x53\x42\x43\x98\x43\x00\x82\x00\x02\x00\x00\x80\x00\x0a\xfd"
			"\x00\x02\x00\x26\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00", 31
		}, {
			"\x55\x53\x42\x43\x98\x43\x00\x82\x00\x00\x00\x00\x00\x00\x0a\xfd"
			"\x00\x03\x00\x26\xc8\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 31
		}, {
			"\x55\x53\x42\x43\xd8\x4c\x04\x82\x00\x02\x00\x00\x80\x00\x0a\xfd"
			"\x00\x00\x10\x73\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00", 31
		}, {
			"\x55\x53\x42\x43\xd8\x4c\x04\x82\x00\x02\x00\x00\x80\x00\x0a\xfd"
			"\x00\x02\x00\x24\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00", 31
		}, {
			"\x55\x53\x42\x43\xd8\x4c\x04\x82\x00\x00\x00\x00\x00\x00\x0a\xfd"
			"\x00\x03\x00\x24\x13\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 31
		}, {
			"\x55\x53\x42\x43\xd8\x4c\x04\x82\x00\x00\x00\x00\x00\x00\x0a\xfd"
			"\x00\x01\x10\x73\x24\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 31
		}

	};

	detach_driver(data);
	data->need_response = true;
	send_messages(data, msgs, ARRAY_SIZE(msgs));
}

static void handle_mbim(struct usbdev_data *data, struct blob_attr **tb)
{
	int j;

	if (data->desc.bNumConfigurations < 2)
		return;

	for (j = 0; j < data->desc.bNumConfigurations; j++) {
		struct libusb_config_descriptor *config;
		int i;

		libusb_get_config_descriptor(data->dev, j, &config);

		for (i = 0; i < config->bNumInterfaces; i++) {
			if (config->interface[i].altsetting[0].bInterfaceClass == 2) {
				if (config->interface[i].altsetting[0].bInterfaceSubClass == 0x0e) {
					struct libusb_config_descriptor *active;
					int count = 5;

				        libusb_get_active_config_descriptor(data->dev, &active);
					if (active->bConfigurationValue == config->bConfigurationValue)
						return;
					while ((libusb_set_configuration(data->devh, config->bConfigurationValue) < 0) && --count)
						libusb_detach_kernel_driver(data->devh, active->interface[0].altsetting[0].bInterfaceNumber);

					libusb_free_config_descriptor(config);
					return;
				}
			}
		}

		libusb_free_config_descriptor(config);
	}
}

static void set_alt_setting(struct usbdev_data *data, int setting)
{
	if (libusb_claim_interface(data->devh, data->interface))
		return;

	libusb_set_interface_alt_setting(data->devh, data->interface, setting);
	libusb_release_interface(data->devh, data->interface);
}

enum {
	MODE_GENERIC,
	MODE_HUAWEI,
	MODE_HUAWEINEW,
	MODE_SIERRA,
	MODE_STDEJECT,
	MODE_SONY,
	MODE_QISDA,
	MODE_GCT,
	MODE_KOBIL,
	MODE_SEQUANS,
	MODE_MOBILE_ACTION,
	MODE_CISCO,
	MODE_MBIM,
	__MODE_MAX
};

static const struct {
	const char *name;
	void (*cb)(struct usbdev_data *data, struct blob_attr **tb);
} modeswitch_cb[__MODE_MAX] = {
	[MODE_GENERIC] = { "Generic", handle_generic },
	[MODE_STDEJECT] = { "StandardEject", handle_standardeject },
	[MODE_HUAWEI] = { "Huawei", handle_huawei },
	[MODE_HUAWEINEW] = { "HuaweiNew", handle_huaweinew },
	[MODE_SIERRA] = { "Sierra", handle_sierra },
	[MODE_SONY] = { "Sony", handle_sony },
	[MODE_QISDA] = { "Qisda", handle_qisda },
	[MODE_GCT] = { "GCT", handle_gct },
	[MODE_KOBIL] = { "Kobil", handle_kobil },
	[MODE_SEQUANS] = { "Sequans", handle_sequans },
	[MODE_MOBILE_ACTION] = { "MobileAction", handle_mobile_action },
	[MODE_CISCO] = { "Cisco", handle_cisco },
	[MODE_MBIM] = { "MBIM", handle_mbim },
};

void handle_switch(struct usbdev_data *data)
{
	static const struct blobmsg_policy data_policy[__DATA_MAX] = {
		[DATA_MODE] = { .name = "mode", .type = BLOBMSG_TYPE_STRING },
		[DATA_MSG] = { .name = "msg", .type = BLOBMSG_TYPE_ARRAY },
		[DATA_INTERFACE] = { .name = "interface", .type = BLOBMSG_TYPE_INT32 },
		[DATA_MSG_EP] = { .name = "msg_endpoint", .type = BLOBMSG_TYPE_INT32 },
		[DATA_RES_EP] = { .name = "response_endpoint", .type = BLOBMSG_TYPE_INT32 },
		[DATA_RESPONSE] = { .name = "response", .type = BLOBMSG_TYPE_BOOL },
		[DATA_CONFIG] = { .name = "config", .type = BLOBMSG_TYPE_INT32 },
		[DATA_ALT] = { .name = "alt", .type = BLOBMSG_TYPE_INT32 },
		[DATA_DEV_CLASS] = { .name = "t_class", .type = BLOBMSG_TYPE_INT32 },
	};
	struct blob_attr *tb[__DATA_MAX];
	int mode = MODE_GENERIC;
	int t_class = 0;

	blobmsg_parse(data_policy, __DATA_MAX, tb, blobmsg_data(data->info), blobmsg_data_len(data->info));

	if (tb[DATA_DEV_CLASS])
		t_class = blobmsg_get_u32(tb[DATA_DEV_CLASS]);

	if (tb[DATA_INTERFACE])
		data->interface = blobmsg_get_u32(tb[DATA_INTERFACE]);

	if (tb[DATA_MSG_EP])
		data->msg_endpoint = blobmsg_get_u32(tb[DATA_MSG_EP]);

	if (tb[DATA_RES_EP])
		data->response_endpoint = blobmsg_get_u32(tb[DATA_RES_EP]);

	if (tb[DATA_RELEASE_DELAY])
		data->release_delay = blobmsg_get_u32(tb[DATA_RELEASE_DELAY]);

	if (tb[DATA_RESPONSE])
		data->need_response = blobmsg_get_bool(tb[DATA_RESPONSE]);

	if (t_class > 0 && data->dev_class != t_class)
		return;

	if (tb[DATA_MODE]) {
		const char *modestr;
		int i;

		modestr = blobmsg_data(tb[DATA_MODE]);
		for (i = 0; i < __MODE_MAX; i++) {
			if (strcmp(modeswitch_cb[i].name, modestr) != 0)
				continue;

			mode = i;
			break;
		}
	}

	modeswitch_cb[mode].cb(data, tb);

	if (tb[DATA_CONFIG]) {
		int config, config_new;

		config_new = blobmsg_get_u32(tb[DATA_CONFIG]);
		if (libusb_get_configuration(data->devh, &config) ||
		    config != config_new)
			libusb_set_configuration(data->devh, config_new);
	}

	if (tb[DATA_ALT]) {
		int new = blobmsg_get_u32(tb[DATA_ALT]);
		set_alt_setting(data, new);
	}
}
