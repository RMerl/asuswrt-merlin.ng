#include <stdio.h>
#include <getopt.h>
#include <stdbool.h>
#include <ctype.h>

#include <blobmsg_json.h>
#include <avl.h>
#include <avl-cmp.h>
#include "switch.h"

#define DEFAULT_CONFIG "/etc/usb-mode.json"

struct device {
	struct avl_node avl;
	struct blob_attr *data;
};

static int verbose = 0;
static const char *config_file = DEFAULT_CONFIG;
static struct blob_buf conf;

char **messages = NULL;
int *message_len;
int n_messages = 0;

static struct avl_tree devices;

struct libusb_context *usb;
static struct libusb_device **usbdevs;
static int n_usbdevs;

static int hex2num(char c)
{
	if (c >= '0' && c <= '9')
		return c - '0';

	c = toupper(c);
	if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;

	return -1;
}

static int hex2byte(const char *hex)
{
	int a, b;

	a = hex2num(*hex++);
	if (a < 0)
		return -1;

	b = hex2num(*hex++);
	if (b < 0)
		return -1;

	return (a << 4) | b;
}

static int hexstr2bin(const char *hex, char *buffer, int len)
{
	const char *ipos = hex;
	char *opos = buffer;
	int i, a;

	for (i = 0; i < len; i++) {
		a = hex2byte(ipos);
		if (a < 0)
			return -1;

		*opos++ = a;
		ipos += 2;
	}

	return 0;
}

static int convert_message(struct blob_attr *attr)
{
	char *data;
	int len;

	data = blobmsg_data(attr);
	len = strlen(data);
	if (len % 2)
		return -1;

	if (hexstr2bin(data, data, len / 2))
		return -1;

	return len / 2;
}

static int parse_config(void)
{
	enum {
		CONF_MESSAGES,
		CONF_DEVICES,
		__CONF_MAX
	};
	static const struct blobmsg_policy policy[__CONF_MAX] = {
		[CONF_MESSAGES] = { .name = "messages", .type = BLOBMSG_TYPE_ARRAY },
		[CONF_DEVICES] = { .name = "devices", .type = BLOBMSG_TYPE_TABLE },
	};
	struct blob_attr *tb[__CONF_MAX];
	struct blob_attr *cur;
	struct device *dev;
	int rem;

	blobmsg_parse(policy, __CONF_MAX, tb, blob_data(conf.head), blob_len(conf.head));
	if (!tb[CONF_MESSAGES] || !tb[CONF_DEVICES]) {
		fprintf(stderr, "Configuration incomplete\n");
		return -1;
	}

	blobmsg_for_each_attr(cur, tb[CONF_MESSAGES], rem)
		n_messages++;

	messages = calloc(n_messages, sizeof(*messages));
	message_len = calloc(n_messages, sizeof(*message_len));
	n_messages = 0;
	blobmsg_for_each_attr(cur, tb[CONF_MESSAGES], rem) {
		int len = convert_message(cur);

		if (len < 0) {
			fprintf(stderr, "Invalid data in message %d\n", n_messages);
			return -1;
		}

		message_len[n_messages] = len;
		messages[n_messages++] = blobmsg_data(cur);
	}

	blobmsg_for_each_attr(cur, tb[CONF_DEVICES], rem) {
	    dev = calloc(1, sizeof(*dev));
	    dev->avl.key = blobmsg_name(cur);
	    dev->data = cur;
	    avl_insert(&devices, &dev->avl);
	}

	return 0;
}

static int usage(const char *prog)
{
	fprintf(stderr, "Usage: %s <command> <options>\n"
		"Commands:\n"
		"	-l		List matching devices\n"
		"	-s		Modeswitch matching devices\n"
		"\n"
		"Options:\n"
		"	-v		Verbose output\n"
		"	-c <file>	Set configuration file to <file> (default: %s)\n"
		"\n", prog, DEFAULT_CONFIG);
	return 1;
}

typedef void (*cmd_cb_t)(struct usbdev_data *data);

static struct blob_attr *
find_dev_data(struct usbdev_data *data, struct device *dev)
{
	struct blob_attr *cur;
	int rem;

	blobmsg_for_each_attr(cur, dev->data, rem) {
		const char *name = blobmsg_name(cur);
		const char *next;
		char *val;

		if (!strcmp(blobmsg_name(cur), "*"))
			return cur;

		next = strchr(name, '=');
		if (!next)
			continue;

		next++;
		if (!strncmp(name, "uMa", 3)) {
			val = data->mfg;
		} else if (!strncmp(name, "uPr", 3)) {
			val = data->prod;
		} else if (!strncmp(name, "uSe", 3)) {
			val = data->serial;
		} else {
			/* ignore unsupported scsi attributes */
			return cur;
		}

		if (!strcmp(val, next))
			return cur;
	}

	return NULL;
}

static void
parse_interface_config(libusb_device *dev, struct usbdev_data *data)
{
	struct libusb_config_descriptor *config;
	const struct libusb_interface *iface;
	const struct libusb_interface_descriptor *alt;
	int i;

	data->interface = -1;
	if (libusb_get_config_descriptor(dev, 0, &config))
		return;

	data->config = config;
	if (!config->bNumInterfaces)
		return;

	iface = &config->interface[0];
	if (!iface->num_altsetting)
		return;

	alt = &iface->altsetting[0];
	data->interface = alt->bInterfaceNumber;
	data->dev_class = alt->bInterfaceClass;

	for (i = 0; i < alt->bNumEndpoints; i++) {
		const struct libusb_endpoint_descriptor *ep = &alt->endpoint[i];
		bool out = false;

		if (data->msg_endpoint && data->response_endpoint)
			break;

		if ((ep->bmAttributes & LIBUSB_TRANSFER_TYPE_MASK) !=
		    LIBUSB_TRANSFER_TYPE_BULK)
			continue;

		out = (ep->bEndpointAddress & LIBUSB_ENDPOINT_DIR_MASK) ==
		      LIBUSB_ENDPOINT_OUT;

		if (!data->msg_endpoint && out)
			data->msg_endpoint = ep->bEndpointAddress;
		if (!data->response_endpoint && !out)
			data->response_endpoint = ep->bEndpointAddress;
	}
}

static void iterate_devs(cmd_cb_t cb)
{
	struct usbdev_data data;
	struct device *dev;
	int i;

	if (!cb)
		return;

	for (i = 0; i < n_usbdevs; i++) {
		memset(&data, 0, sizeof(data));

		if (libusb_get_device_descriptor(usbdevs[i], &data.desc))
			continue;

		sprintf(data.idstr, "%04x:%04x", data.desc.idVendor, data.desc.idProduct);

		dev = avl_find_element(&devices, data.idstr, dev, avl);
		if (!dev)
			continue;

		if (libusb_open(usbdevs[i], &data.devh))
			continue;

		data.dev = usbdevs[i];

		libusb_get_string_descriptor_ascii(
			data.devh, data.desc.iManufacturer,
			(void *) data.mfg, sizeof(data.mfg));
		libusb_get_string_descriptor_ascii(
			data.devh, data.desc.iProduct,
			(void *) data.prod, sizeof(data.prod));
		libusb_get_string_descriptor_ascii(
			data.devh, data.desc.iSerialNumber,
			(void *) data.serial, sizeof(data.serial));

		parse_interface_config(usbdevs[i], &data);

		data.info = find_dev_data(&data, dev);
		if (data.info)
			cb(&data);

		if (data.config)
			libusb_free_config_descriptor(data.config);

		if (data.devh)
			libusb_close(data.devh);
	}
}

static void handle_list(struct usbdev_data *data)
{
	fprintf(stderr, "Found device: %s (Manufacturer: \"%s\", Product: \"%s\", Serial: \"%s\")\n",
		data->idstr, data->mfg, data->prod, data->serial);
}

int main(int argc, char **argv)
{
	cmd_cb_t cb = NULL;
	int ret;
	int ch;

	avl_init(&devices, avl_strcmp, false, NULL);

	while ((ch = getopt(argc, argv, "lsc:v")) != -1) {
		switch (ch) {
		case 'l':
			cb = handle_list;
			break;
		case 's':
			cb = handle_switch;
			break;
		case 'c':
			config_file = optarg;
			break;
		case 'v':
			verbose++;
			break;
		default:
			return usage(argv[0]);
		}
	}

	blob_buf_init(&conf, 0);
	if (!blobmsg_add_json_from_file(&conf, config_file) ||
	    parse_config()) {
		fprintf(stderr, "Failed to load config file\n");
		return 1;
	}

	ret = libusb_init(&usb);
	if (ret) {
		fprintf(stderr, "Failed to initialize libusb: %s\n", libusb_error_name(ret));
		return 1;
	}

	n_usbdevs = libusb_get_device_list(usb, &usbdevs);
	iterate_devs(cb);
	libusb_free_device_list(usbdevs, 1);
	libusb_exit(usb);

	return 0;
}
