/*
 * f_midi.c -- USB MIDI class function driver
 *
 * Copyright (C) 2006 Thumtronics Pty Ltd.
 * Developed for Thumtronics by Grey Innovation
 * Ben Williamson <ben.williamson@greyinnovation.com>
 *
 * Rewritten for the composite framework
 *   Copyright (C) 2011 Daniel Mack <zonque@gmail.com>
 *
 * Based on drivers/usb/gadget/f_audio.c,
 *   Copyright (C) 2008 Bryan Wu <cooloney@kernel.org>
 *   Copyright (C) 2008 Analog Devices, Inc
 *
 * and drivers/usb/gadget/midi.c,
 *   Copyright (C) 2006 Thumtronics Pty Ltd.
 *   Ben Williamson <ben.williamson@greyinnovation.com>
 *
 * Licensed under the GPL-2 or later.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/device.h>

#include <sound/core.h>
#include <sound/initval.h>
#include <sound/rawmidi.h>

#include <linux/usb/ch9.h>
#include <linux/usb/gadget.h>
#include <linux/usb/audio.h>
#include <linux/usb/midi.h>

#include "u_f.h"
#include "u_midi.h"

MODULE_AUTHOR("Ben Williamson");
MODULE_LICENSE("GPL v2");

static const char f_midi_shortname[] = "f_midi";
static const char f_midi_longname[] = "MIDI Gadget";

/*
 * We can only handle 16 cables on one single endpoint, as cable numbers are
 * stored in 4-bit fields. And as the interface currently only holds one
 * single endpoint, this is the maximum number of ports we can allow.
 */
#define MAX_PORTS 16

/*
 * This is a gadget, and the IN/OUT naming is from the host's perspective.
 * USB -> OUT endpoint -> rawmidi
 * USB <- IN endpoint  <- rawmidi
 */
struct gmidi_in_port {
	struct f_midi *midi;
	int active;
	uint8_t cable;
	uint8_t state;
#define STATE_UNKNOWN	0
#define STATE_1PARAM	1
#define STATE_2PARAM_1	2
#define STATE_2PARAM_2	3
#define STATE_SYSEX_0	4
#define STATE_SYSEX_1	5
#define STATE_SYSEX_2	6
	uint8_t data[2];
};

struct f_midi {
	struct usb_function	func;
	struct usb_gadget	*gadget;
	struct usb_ep		*in_ep, *out_ep;
	struct snd_card		*card;
	struct snd_rawmidi	*rmidi;

	struct snd_rawmidi_substream *in_substream[MAX_PORTS];
	struct snd_rawmidi_substream *out_substream[MAX_PORTS];
	struct gmidi_in_port	*in_port[MAX_PORTS];

	unsigned long		out_triggered;
	struct tasklet_struct	tasklet;
	unsigned int in_ports;
	unsigned int out_ports;
	int index;
	char *id;
	unsigned int buflen, qlen;
};

static inline struct f_midi *func_to_midi(struct usb_function *f)
{
	return container_of(f, struct f_midi, func);
}

static void f_midi_transmit(struct f_midi *midi, struct usb_request *req);

DECLARE_UAC_AC_HEADER_DESCRIPTOR(1);
DECLARE_USB_MIDI_OUT_JACK_DESCRIPTOR(1);
DECLARE_USB_MS_ENDPOINT_DESCRIPTOR(16);

/* B.3.1  Standard AC Interface Descriptor */
static struct usb_interface_descriptor ac_interface_desc = {
	.bLength =		USB_DT_INTERFACE_SIZE,
	.bDescriptorType =	USB_DT_INTERFACE,
	/* .bInterfaceNumber =	DYNAMIC */
	/* .bNumEndpoints =	DYNAMIC */
	.bInterfaceClass =	USB_CLASS_AUDIO,
	.bInterfaceSubClass =	USB_SUBCLASS_AUDIOCONTROL,
	/* .iInterface =	DYNAMIC */
};

/* B.3.2  Class-Specific AC Interface Descriptor */
static struct uac1_ac_header_descriptor_1 ac_header_desc = {
	.bLength =		UAC_DT_AC_HEADER_SIZE(1),
	.bDescriptorType =	USB_DT_CS_INTERFACE,
	.bDescriptorSubtype =	USB_MS_HEADER,
	.bcdADC =		cpu_to_le16(0x0100),
	.wTotalLength =		cpu_to_le16(UAC_DT_AC_HEADER_SIZE(1)),
	.bInCollection =	1,
	/* .baInterfaceNr =	DYNAMIC */
};

/* B.4.1  Standard MS Interface Descriptor */
static struct usb_interface_descriptor ms_interface_desc = {
	.bLength =		USB_DT_INTERFACE_SIZE,
	.bDescriptorType =	USB_DT_INTERFACE,
	/* .bInterfaceNumber =	DYNAMIC */
	.bNumEndpoints =	2,
	.bInterfaceClass =	USB_CLASS_AUDIO,
	.bInterfaceSubClass =	USB_SUBCLASS_MIDISTREAMING,
	/* .iInterface =	DYNAMIC */
};

/* B.4.2  Class-Specific MS Interface Descriptor */
static struct usb_ms_header_descriptor ms_header_desc = {
	.bLength =		USB_DT_MS_HEADER_SIZE,
	.bDescriptorType =	USB_DT_CS_INTERFACE,
	.bDescriptorSubtype =	USB_MS_HEADER,
	.bcdMSC =		cpu_to_le16(0x0100),
	/* .wTotalLength =	DYNAMIC */
};

/* B.5.1  Standard Bulk OUT Endpoint Descriptor */
static struct usb_endpoint_descriptor bulk_out_desc = {
	.bLength =		USB_DT_ENDPOINT_AUDIO_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,
	.bEndpointAddress =	USB_DIR_OUT,
	.bmAttributes =		USB_ENDPOINT_XFER_BULK,
};

/* B.5.2  Class-specific MS Bulk OUT Endpoint Descriptor */
static struct usb_ms_endpoint_descriptor_16 ms_out_desc = {
	/* .bLength =		DYNAMIC */
	.bDescriptorType =	USB_DT_CS_ENDPOINT,
	.bDescriptorSubtype =	USB_MS_GENERAL,
	/* .bNumEmbMIDIJack =	DYNAMIC */
	/* .baAssocJackID =	DYNAMIC */
};

/* B.6.1  Standard Bulk IN Endpoint Descriptor */
static struct usb_endpoint_descriptor bulk_in_desc = {
	.bLength =		USB_DT_ENDPOINT_AUDIO_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,
	.bEndpointAddress =	USB_DIR_IN,
	.bmAttributes =		USB_ENDPOINT_XFER_BULK,
};

/* B.6.2  Class-specific MS Bulk IN Endpoint Descriptor */
static struct usb_ms_endpoint_descriptor_16 ms_in_desc = {
	/* .bLength =		DYNAMIC */
	.bDescriptorType =	USB_DT_CS_ENDPOINT,
	.bDescriptorSubtype =	USB_MS_GENERAL,
	/* .bNumEmbMIDIJack =	DYNAMIC */
	/* .baAssocJackID =	DYNAMIC */
};

/* string IDs are assigned dynamically */

#define STRING_FUNC_IDX			0

static struct usb_string midi_string_defs[] = {
	[STRING_FUNC_IDX].s = "MIDI function",
	{  } /* end of list */
};

static struct usb_gadget_strings midi_stringtab = {
	.language	= 0x0409,	/* en-us */
	.strings	= midi_string_defs,
};

static struct usb_gadget_strings *midi_strings[] = {
	&midi_stringtab,
	NULL,
};

static inline struct usb_request *midi_alloc_ep_req(struct usb_ep *ep,
						    unsigned length)
{
	return alloc_ep_req(ep, length, length);
}

static const uint8_t f_midi_cin_length[] = {
	0, 0, 2, 3, 3, 1, 2, 3, 3, 3, 3, 3, 2, 2, 3, 1
};

/*
 * Receives a chunk of MIDI data.
 */
static void f_midi_read_data(struct usb_ep *ep, int cable,
			     uint8_t *data, int length)
{
	struct f_midi *midi = ep->driver_data;
	struct snd_rawmidi_substream *substream = midi->out_substream[cable];

	if (!substream)
		/* Nobody is listening - throw it on the floor. */
		return;

	if (!test_bit(cable, &midi->out_triggered))
		return;

	snd_rawmidi_receive(substream, data, length);
}

static void f_midi_handle_out_data(struct usb_ep *ep, struct usb_request *req)
{
	unsigned int i;
	u8 *buf = req->buf;

	for (i = 0; i + 3 < req->actual; i += 4)
		if (buf[i] != 0) {
			int cable = buf[i] >> 4;
			int length = f_midi_cin_length[buf[i] & 0x0f];
			f_midi_read_data(ep, cable, &buf[i + 1], length);
		}
}

static void
f_midi_complete(struct usb_ep *ep, struct usb_request *req)
{
	struct f_midi *midi = ep->driver_data;
	struct usb_composite_dev *cdev = midi->func.config->cdev;
	int status = req->status;

	switch (status) {
	case 0:			 /* normal completion */
		if (ep == midi->out_ep) {
			/* We received stuff. req is queued again, below */
			f_midi_handle_out_data(ep, req);
		} else if (ep == midi->in_ep) {
			/* Our transmit completed. See if there's more to go.
			 * f_midi_transmit eats req, don't queue it again. */
			f_midi_transmit(midi, req);
			return;
		}
		break;

	/* this endpoint is normally active while we're configured */
	case -ECONNABORTED:	/* hardware forced ep reset */
	case -ECONNRESET:	/* request dequeued */
	case -ESHUTDOWN:	/* disconnect from host */
		VDBG(cdev, "%s gone (%d), %d/%d\n", ep->name, status,
				req->actual, req->length);
		if (ep == midi->out_ep)
			f_midi_handle_out_data(ep, req);

		free_ep_req(ep, req);
		return;

	case -EOVERFLOW:	/* buffer overrun on read means that
				 * we didn't provide a big enough buffer.
				 */
	default:
		DBG(cdev, "%s complete --> %d, %d/%d\n", ep->name,
				status, req->actual, req->length);
		break;
	case -EREMOTEIO:	/* short read */
		break;
	}

	status = usb_ep_queue(ep, req, GFP_ATOMIC);
	if (status) {
		ERROR(cdev, "kill %s:  resubmit %d bytes --> %d\n",
				ep->name, req->length, status);
		usb_ep_set_halt(ep);
		/* FIXME recover later ... somehow */
	}
}

static int f_midi_start_ep(struct f_midi *midi,
			   struct usb_function *f,
			   struct usb_ep *ep)
{
	int err;
	struct usb_composite_dev *cdev = f->config->cdev;

	if (ep->driver_data)
		usb_ep_disable(ep);

	err = config_ep_by_speed(midi->gadget, f, ep);
	if (err) {
		ERROR(cdev, "can't configure %s: %d\n", ep->name, err);
		return err;
	}

	err = usb_ep_enable(ep);
	if (err) {
		ERROR(cdev, "can't start %s: %d\n", ep->name, err);
		return err;
	}

	ep->driver_data = midi;

	return 0;
}

static int f_midi_set_alt(struct usb_function *f, unsigned intf, unsigned alt)
{
	struct f_midi *midi = func_to_midi(f);
	struct usb_composite_dev *cdev = f->config->cdev;
	unsigned i;
	int err;

	err = f_midi_start_ep(midi, f, midi->in_ep);
	if (err)
		return err;

	err = f_midi_start_ep(midi, f, midi->out_ep);
	if (err)
		return err;

	if (midi->out_ep->driver_data)
		usb_ep_disable(midi->out_ep);

	err = config_ep_by_speed(midi->gadget, f, midi->out_ep);
	if (err) {
		ERROR(cdev, "can't configure %s: %d\n",
		      midi->out_ep->name, err);
		return err;
	}

	err = usb_ep_enable(midi->out_ep);
	if (err) {
		ERROR(cdev, "can't start %s: %d\n",
		      midi->out_ep->name, err);
		return err;
	}

	midi->out_ep->driver_data = midi;

	/* allocate a bunch of read buffers and queue them all at once. */
	for (i = 0; i < midi->qlen && err == 0; i++) {
		struct usb_request *req =
			midi_alloc_ep_req(midi->out_ep,
				max_t(unsigned, midi->buflen,
					bulk_out_desc.wMaxPacketSize));
		if (req == NULL)
			return -ENOMEM;

		req->complete = f_midi_complete;
		err = usb_ep_queue(midi->out_ep, req, GFP_ATOMIC);
		if (err) {
			ERROR(midi, "%s queue req: %d\n",
				    midi->out_ep->name, err);
		}
	}

	return 0;
}

static void f_midi_disable(struct usb_function *f)
{
	struct f_midi *midi = func_to_midi(f);
	struct usb_composite_dev *cdev = f->config->cdev;

	DBG(cdev, "disable\n");

	/*
	 * just disable endpoints, forcing completion of pending i/o.
	 * all our completion handlers free their requests in this case.
	 */
	usb_ep_disable(midi->in_ep);
	usb_ep_disable(midi->out_ep);
}

static int f_midi_snd_free(struct snd_device *device)
{
	return 0;
}

static void f_midi_transmit_packet(struct usb_request *req, uint8_t p0,
					uint8_t p1, uint8_t p2, uint8_t p3)
{
	unsigned length = req->length;
	u8 *buf = (u8 *)req->buf + length;

	buf[0] = p0;
	buf[1] = p1;
	buf[2] = p2;
	buf[3] = p3;
	req->length = length + 4;
}

/*
 * Converts MIDI commands to USB MIDI packets.
 */
static void f_midi_transmit_byte(struct usb_request *req,
				 struct gmidi_in_port *port, uint8_t b)
{
	uint8_t p0 = port->cable << 4;

	if (b >= 0xf8) {
		f_midi_transmit_packet(req, p0 | 0x0f, b, 0, 0);
	} else if (b >= 0xf0) {
		switch (b) {
		case 0xf0:
			port->data[0] = b;
			port->state = STATE_SYSEX_1;
			break;
		case 0xf1:
		case 0xf3:
			port->data[0] = b;
			port->state = STATE_1PARAM;
			break;
		case 0xf2:
			port->data[0] = b;
			port->state = STATE_2PARAM_1;
			break;
		case 0xf4:
		case 0xf5:
			port->state = STATE_UNKNOWN;
			break;
		case 0xf6:
			f_midi_transmit_packet(req, p0 | 0x05, 0xf6, 0, 0);
			port->state = STATE_UNKNOWN;
			break;
		case 0xf7:
			switch (port->state) {
			case STATE_SYSEX_0:
				f_midi_transmit_packet(req,
					p0 | 0x05, 0xf7, 0, 0);
				break;
			case STATE_SYSEX_1:
				f_midi_transmit_packet(req,
					p0 | 0x06, port->data[0], 0xf7, 0);
				break;
			case STATE_SYSEX_2:
				f_midi_transmit_packet(req,
					p0 | 0x07, port->data[0],
					port->data[1], 0xf7);
				break;
			}
			port->state = STATE_UNKNOWN;
			break;
		}
	} else if (b >= 0x80) {
		port->data[0] = b;
		if (b >= 0xc0 && b <= 0xdf)
			port->state = STATE_1PARAM;
		else
			port->state = STATE_2PARAM_1;
	} else { /* b < 0x80 */
		switch (port->state) {
		case STATE_1PARAM:
			if (port->data[0] < 0xf0) {
				p0 |= port->data[0] >> 4;
			} else {
				p0 |= 0x02;
				port->state = STATE_UNKNOWN;
			}
			f_midi_transmit_packet(req, p0, port->data[0], b, 0);
			break;
		case STATE_2PARAM_1:
			port->data[1] = b;
			port->state = STATE_2PARAM_2;
			break;
		case STATE_2PARAM_2:
			if (port->data[0] < 0xf0) {
				p0 |= port->data[0] >> 4;
				port->state = STATE_2PARAM_1;
			} else {
				p0 |= 0x03;
				port->state = STATE_UNKNOWN;
			}
			f_midi_transmit_packet(req,
				p0, port->data[0], port->data[1], b);
			break;
		case STATE_SYSEX_0:
			port->data[0] = b;
			port->state = STATE_SYSEX_1;
			break;
		case STATE_SYSEX_1:
			port->data[1] = b;
			port->state = STATE_SYSEX_2;
			break;
		case STATE_SYSEX_2:
			f_midi_transmit_packet(req,
				p0 | 0x04, port->data[0], port->data[1], b);
			port->state = STATE_SYSEX_0;
			break;
		}
	}
}

static void f_midi_transmit(struct f_midi *midi, struct usb_request *req)
{
	struct usb_ep *ep = midi->in_ep;
	int i;

	if (!ep)
		return;

	if (!req)
		req = midi_alloc_ep_req(ep, midi->buflen);

	if (!req) {
		ERROR(midi, "%s: alloc_ep_request failed\n", __func__);
		return;
	}
	req->length = 0;
	req->complete = f_midi_complete;

	for (i = 0; i < MAX_PORTS; i++) {
		struct gmidi_in_port *port = midi->in_port[i];
		struct snd_rawmidi_substream *substream = midi->in_substream[i];

		if (!port || !port->active || !substream)
			continue;

		while (req->length + 3 < midi->buflen) {
			uint8_t b;
			if (snd_rawmidi_transmit(substream, &b, 1) != 1) {
				port->active = 0;
				break;
			}
			f_midi_transmit_byte(req, port, b);
		}
	}

	if (req->length > 0)
		usb_ep_queue(ep, req, GFP_ATOMIC);
	else
		free_ep_req(ep, req);
}

static void f_midi_in_tasklet(unsigned long data)
{
	struct f_midi *midi = (struct f_midi *) data;
	f_midi_transmit(midi, NULL);
}

static int f_midi_in_open(struct snd_rawmidi_substream *substream)
{
	struct f_midi *midi = substream->rmidi->private_data;

	if (!midi->in_port[substream->number])
		return -EINVAL;

	VDBG(midi, "%s()\n", __func__);
	midi->in_substream[substream->number] = substream;
	midi->in_port[substream->number]->state = STATE_UNKNOWN;
	return 0;
}

static int f_midi_in_close(struct snd_rawmidi_substream *substream)
{
	struct f_midi *midi = substream->rmidi->private_data;

	VDBG(midi, "%s()\n", __func__);
	return 0;
}

static void f_midi_in_trigger(struct snd_rawmidi_substream *substream, int up)
{
	struct f_midi *midi = substream->rmidi->private_data;

	if (!midi->in_port[substream->number])
		return;

	VDBG(midi, "%s() %d\n", __func__, up);
	midi->in_port[substream->number]->active = up;
	if (up)
		tasklet_hi_schedule(&midi->tasklet);
}

static int f_midi_out_open(struct snd_rawmidi_substream *substream)
{
	struct f_midi *midi = substream->rmidi->private_data;

	if (substream->number >= MAX_PORTS)
		return -EINVAL;

	VDBG(midi, "%s()\n", __func__);
	midi->out_substream[substream->number] = substream;
	return 0;
}

static int f_midi_out_close(struct snd_rawmidi_substream *substream)
{
	struct f_midi *midi = substream->rmidi->private_data;

	VDBG(midi, "%s()\n", __func__);
	return 0;
}

static void f_midi_out_trigger(struct snd_rawmidi_substream *substream, int up)
{
	struct f_midi *midi = substream->rmidi->private_data;

	VDBG(midi, "%s()\n", __func__);

	if (up)
		set_bit(substream->number, &midi->out_triggered);
	else
		clear_bit(substream->number, &midi->out_triggered);
}

static struct snd_rawmidi_ops gmidi_in_ops = {
	.open = f_midi_in_open,
	.close = f_midi_in_close,
	.trigger = f_midi_in_trigger,
};

static struct snd_rawmidi_ops gmidi_out_ops = {
	.open = f_midi_out_open,
	.close = f_midi_out_close,
	.trigger = f_midi_out_trigger
};

static inline void f_midi_unregister_card(struct f_midi *midi)
{
	if (midi->card) {
		snd_card_free(midi->card);
		midi->card = NULL;
	}
}

/* register as a sound "card" */
static int f_midi_register_card(struct f_midi *midi)
{
	struct snd_card *card;
	struct snd_rawmidi *rmidi;
	int err;
	static struct snd_device_ops ops = {
		.dev_free = f_midi_snd_free,
	};

	err = snd_card_new(&midi->gadget->dev, midi->index, midi->id,
			   THIS_MODULE, 0, &card);
	if (err < 0) {
		ERROR(midi, "snd_card_new() failed\n");
		goto fail;
	}
	midi->card = card;

	err = snd_device_new(card, SNDRV_DEV_LOWLEVEL, midi, &ops);
	if (err < 0) {
		ERROR(midi, "snd_device_new() failed: error %d\n", err);
		goto fail;
	}

	strcpy(card->driver, f_midi_longname);
	strcpy(card->longname, f_midi_longname);
	strcpy(card->shortname, f_midi_shortname);

	/* Set up rawmidi */
	snd_component_add(card, "MIDI");
	err = snd_rawmidi_new(card, card->longname, 0,
			      midi->out_ports, midi->in_ports, &rmidi);
	if (err < 0) {
		ERROR(midi, "snd_rawmidi_new() failed: error %d\n", err);
		goto fail;
	}
	midi->rmidi = rmidi;
	strcpy(rmidi->name, card->shortname);
	rmidi->info_flags = SNDRV_RAWMIDI_INFO_OUTPUT |
			    SNDRV_RAWMIDI_INFO_INPUT |
			    SNDRV_RAWMIDI_INFO_DUPLEX;
	rmidi->private_data = midi;

	/*
	 * Yes, rawmidi OUTPUT = USB IN, and rawmidi INPUT = USB OUT.
	 * It's an upside-down world being a gadget.
	 */
	snd_rawmidi_set_ops(rmidi, SNDRV_RAWMIDI_STREAM_OUTPUT, &gmidi_in_ops);
	snd_rawmidi_set_ops(rmidi, SNDRV_RAWMIDI_STREAM_INPUT, &gmidi_out_ops);

	/* register it - we're ready to go */
	err = snd_card_register(card);
	if (err < 0) {
		ERROR(midi, "snd_card_register() failed\n");
		goto fail;
	}

	VDBG(midi, "%s() finished ok\n", __func__);
	return 0;

fail:
	f_midi_unregister_card(midi);
	return err;
}

/* MIDI function driver setup/binding */

static int f_midi_bind(struct usb_configuration *c, struct usb_function *f)
{
	struct usb_descriptor_header **midi_function;
	struct usb_midi_in_jack_descriptor jack_in_ext_desc[MAX_PORTS];
	struct usb_midi_in_jack_descriptor jack_in_emb_desc[MAX_PORTS];
	struct usb_midi_out_jack_descriptor_1 jack_out_ext_desc[MAX_PORTS];
	struct usb_midi_out_jack_descriptor_1 jack_out_emb_desc[MAX_PORTS];
	struct usb_composite_dev *cdev = c->cdev;
	struct f_midi *midi = func_to_midi(f);
	struct usb_string *us;
	int status, n, jack = 1, i = 0;

	midi->gadget = cdev->gadget;
	tasklet_init(&midi->tasklet, f_midi_in_tasklet, (unsigned long) midi);
	status = f_midi_register_card(midi);
	if (status < 0)
		goto fail_register;

	/* maybe allocate device-global string ID */
	us = usb_gstrings_attach(c->cdev, midi_strings,
				 ARRAY_SIZE(midi_string_defs));
	if (IS_ERR(us)) {
		status = PTR_ERR(us);
		goto fail;
	}
	ac_interface_desc.iInterface = us[STRING_FUNC_IDX].id;

	/* We have two interfaces, AudioControl and MIDIStreaming */
	status = usb_interface_id(c, f);
	if (status < 0)
		goto fail;
	ac_interface_desc.bInterfaceNumber = status;

	status = usb_interface_id(c, f);
	if (status < 0)
		goto fail;
	ms_interface_desc.bInterfaceNumber = status;
	ac_header_desc.baInterfaceNr[0] = status;

	status = -ENODEV;

	/* allocate instance-specific endpoints */
	midi->in_ep = usb_ep_autoconfig(cdev->gadget, &bulk_in_desc);
	if (!midi->in_ep)
		goto fail;
	midi->in_ep->driver_data = cdev;	/* claim */

	midi->out_ep = usb_ep_autoconfig(cdev->gadget, &bulk_out_desc);
	if (!midi->out_ep)
		goto fail;
	midi->out_ep->driver_data = cdev;	/* claim */

	/* allocate temporary function list */
	midi_function = kcalloc((MAX_PORTS * 4) + 9, sizeof(*midi_function),
				GFP_KERNEL);
	if (!midi_function) {
		status = -ENOMEM;
		goto fail;
	}

	/*
	 * construct the function's descriptor set. As the number of
	 * input and output MIDI ports is configurable, we have to do
	 * it that way.
	 */

	/* add the headers - these are always the same */
	midi_function[i++] = (struct usb_descriptor_header *) &ac_interface_desc;
	midi_function[i++] = (struct usb_descriptor_header *) &ac_header_desc;
	midi_function[i++] = (struct usb_descriptor_header *) &ms_interface_desc;

	/* calculate the header's wTotalLength */
	n = USB_DT_MS_HEADER_SIZE
		+ (midi->in_ports + midi->out_ports) *
			(USB_DT_MIDI_IN_SIZE + USB_DT_MIDI_OUT_SIZE(1));
	ms_header_desc.wTotalLength = cpu_to_le16(n);

	midi_function[i++] = (struct usb_descriptor_header *) &ms_header_desc;

	/* configure the external IN jacks, each linked to an embedded OUT jack */
	for (n = 0; n < midi->in_ports; n++) {
		struct usb_midi_in_jack_descriptor *in_ext = &jack_in_ext_desc[n];
		struct usb_midi_out_jack_descriptor_1 *out_emb = &jack_out_emb_desc[n];

		in_ext->bLength			= USB_DT_MIDI_IN_SIZE;
		in_ext->bDescriptorType		= USB_DT_CS_INTERFACE;
		in_ext->bDescriptorSubtype	= USB_MS_MIDI_IN_JACK;
		in_ext->bJackType		= USB_MS_EXTERNAL;
		in_ext->bJackID			= jack++;
		in_ext->iJack			= 0;
		midi_function[i++] = (struct usb_descriptor_header *) in_ext;

		out_emb->bLength		= USB_DT_MIDI_OUT_SIZE(1);
		out_emb->bDescriptorType	= USB_DT_CS_INTERFACE;
		out_emb->bDescriptorSubtype	= USB_MS_MIDI_OUT_JACK;
		out_emb->bJackType		= USB_MS_EMBEDDED;
		out_emb->bJackID		= jack++;
		out_emb->bNrInputPins		= 1;
		out_emb->pins[0].baSourcePin	= 1;
		out_emb->pins[0].baSourceID	= in_ext->bJackID;
		out_emb->iJack			= 0;
		midi_function[i++] = (struct usb_descriptor_header *) out_emb;

		/* link it to the endpoint */
		ms_in_desc.baAssocJackID[n] = out_emb->bJackID;
	}

	/* configure the external OUT jacks, each linked to an embedded IN jack */
	for (n = 0; n < midi->out_ports; n++) {
		struct usb_midi_in_jack_descriptor *in_emb = &jack_in_emb_desc[n];
		struct usb_midi_out_jack_descriptor_1 *out_ext = &jack_out_ext_desc[n];

		in_emb->bLength			= USB_DT_MIDI_IN_SIZE;
		in_emb->bDescriptorType		= USB_DT_CS_INTERFACE;
		in_emb->bDescriptorSubtype	= USB_MS_MIDI_IN_JACK;
		in_emb->bJackType		= USB_MS_EMBEDDED;
		in_emb->bJackID			= jack++;
		in_emb->iJack			= 0;
		midi_function[i++] = (struct usb_descriptor_header *) in_emb;

		out_ext->bLength =		USB_DT_MIDI_OUT_SIZE(1);
		out_ext->bDescriptorType =	USB_DT_CS_INTERFACE;
		out_ext->bDescriptorSubtype =	USB_MS_MIDI_OUT_JACK;
		out_ext->bJackType =		USB_MS_EXTERNAL;
		out_ext->bJackID =		jack++;
		out_ext->bNrInputPins =		1;
		out_ext->iJack =		0;
		out_ext->pins[0].baSourceID =	in_emb->bJackID;
		out_ext->pins[0].baSourcePin =	1;
		midi_function[i++] = (struct usb_descriptor_header *) out_ext;

		/* link it to the endpoint */
		ms_out_desc.baAssocJackID[n] = in_emb->bJackID;
	}

	/* configure the endpoint descriptors ... */
	ms_out_desc.bLength = USB_DT_MS_ENDPOINT_SIZE(midi->in_ports);
	ms_out_desc.bNumEmbMIDIJack = midi->in_ports;

	ms_in_desc.bLength = USB_DT_MS_ENDPOINT_SIZE(midi->out_ports);
	ms_in_desc.bNumEmbMIDIJack = midi->out_ports;

	/* ... and add them to the list */
	midi_function[i++] = (struct usb_descriptor_header *) &bulk_out_desc;
	midi_function[i++] = (struct usb_descriptor_header *) &ms_out_desc;
	midi_function[i++] = (struct usb_descriptor_header *) &bulk_in_desc;
	midi_function[i++] = (struct usb_descriptor_header *) &ms_in_desc;
	midi_function[i++] = NULL;

	/*
	 * support all relevant hardware speeds... we expect that when
	 * hardware is dual speed, all bulk-capable endpoints work at
	 * both speeds
	 */
	/* copy descriptors, and track endpoint copies */
	f->fs_descriptors = usb_copy_descriptors(midi_function);
	if (!f->fs_descriptors)
		goto fail_f_midi;

	if (gadget_is_dualspeed(c->cdev->gadget)) {
		bulk_in_desc.wMaxPacketSize = cpu_to_le16(512);
		bulk_out_desc.wMaxPacketSize = cpu_to_le16(512);
		f->hs_descriptors = usb_copy_descriptors(midi_function);
		if (!f->hs_descriptors)
			goto fail_f_midi;
	}

	kfree(midi_function);

	return 0;

fail_f_midi:
	kfree(midi_function);
	usb_free_descriptors(f->hs_descriptors);
fail:
	f_midi_unregister_card(midi);
fail_register:
	/* we might as well release our claims on endpoints */
	if (midi->out_ep)
		midi->out_ep->driver_data = NULL;
	if (midi->in_ep)
		midi->in_ep->driver_data = NULL;

	ERROR(cdev, "%s: can't bind, err %d\n", f->name, status);

	return status;
}

static inline struct f_midi_opts *to_f_midi_opts(struct config_item *item)
{
	return container_of(to_config_group(item), struct f_midi_opts,
			    func_inst.group);
}

CONFIGFS_ATTR_STRUCT(f_midi_opts);
CONFIGFS_ATTR_OPS(f_midi_opts);

static void midi_attr_release(struct config_item *item)
{
	struct f_midi_opts *opts = to_f_midi_opts(item);

	usb_put_function_instance(&opts->func_inst);
}

static struct configfs_item_operations midi_item_ops = {
	.release	= midi_attr_release,
	.show_attribute	= f_midi_opts_attr_show,
	.store_attribute = f_midi_opts_attr_store,
};

#define F_MIDI_OPT(name, test_limit, limit)				\
static ssize_t f_midi_opts_##name##_show(struct f_midi_opts *opts, char *page) \
{									\
	int result;							\
									\
	mutex_lock(&opts->lock);					\
	result = sprintf(page, "%d\n", opts->name);			\
	mutex_unlock(&opts->lock);					\
									\
	return result;							\
}									\
									\
static ssize_t f_midi_opts_##name##_store(struct f_midi_opts *opts,	\
					 const char *page, size_t len)	\
{									\
	int ret;							\
	u32 num;							\
									\
	mutex_lock(&opts->lock);					\
	if (opts->refcnt) {						\
		ret = -EBUSY;						\
		goto end;						\
	}								\
									\
	ret = kstrtou32(page, 0, &num);					\
	if (ret)							\
		goto end;						\
									\
	if (test_limit && num > limit) {				\
		ret = -EINVAL;						\
		goto end;						\
	}								\
	opts->name = num;						\
	ret = len;							\
									\
end:									\
	mutex_unlock(&opts->lock);					\
	return ret;							\
}									\
									\
static struct f_midi_opts_attribute f_midi_opts_##name =		\
	__CONFIGFS_ATTR(name, S_IRUGO | S_IWUSR, f_midi_opts_##name##_show, \
			f_midi_opts_##name##_store)

F_MIDI_OPT(index, true, SNDRV_CARDS);
F_MIDI_OPT(buflen, false, 0);
F_MIDI_OPT(qlen, false, 0);
F_MIDI_OPT(in_ports, true, MAX_PORTS);
F_MIDI_OPT(out_ports, true, MAX_PORTS);

static ssize_t f_midi_opts_id_show(struct f_midi_opts *opts, char *page)
{
	int result;

	mutex_lock(&opts->lock);
	if (opts->id) {
		result = strlcpy(page, opts->id, PAGE_SIZE);
	} else {
		page[0] = 0;
		result = 0;
	}

	mutex_unlock(&opts->lock);

	return result;
}

static ssize_t f_midi_opts_id_store(struct f_midi_opts *opts,
				    const char *page, size_t len)
{
	int ret;
	char *c;

	mutex_lock(&opts->lock);
	if (opts->refcnt) {
		ret = -EBUSY;
		goto end;
	}

	c = kstrndup(page, len, GFP_KERNEL);
	if (!c) {
		ret = -ENOMEM;
		goto end;
	}
	if (opts->id_allocated)
		kfree(opts->id);
	opts->id = c;
	opts->id_allocated = true;
	ret = len;
end:
	mutex_unlock(&opts->lock);
	return ret;
}

static struct f_midi_opts_attribute f_midi_opts_id =
	__CONFIGFS_ATTR(id, S_IRUGO | S_IWUSR, f_midi_opts_id_show,
			f_midi_opts_id_store);

static struct configfs_attribute *midi_attrs[] = {
	&f_midi_opts_index.attr,
	&f_midi_opts_buflen.attr,
	&f_midi_opts_qlen.attr,
	&f_midi_opts_in_ports.attr,
	&f_midi_opts_out_ports.attr,
	&f_midi_opts_id.attr,
	NULL,
};

static struct config_item_type midi_func_type = {
	.ct_item_ops	= &midi_item_ops,
	.ct_attrs	= midi_attrs,
	.ct_owner	= THIS_MODULE,
};

static void f_midi_free_inst(struct usb_function_instance *f)
{
	struct f_midi_opts *opts;

	opts = container_of(f, struct f_midi_opts, func_inst);

	if (opts->id_allocated)
		kfree(opts->id);

	kfree(opts);
}

static struct usb_function_instance *f_midi_alloc_inst(void)
{
	struct f_midi_opts *opts;

	opts = kzalloc(sizeof(*opts), GFP_KERNEL);
	if (!opts)
		return ERR_PTR(-ENOMEM);

	mutex_init(&opts->lock);
	opts->func_inst.free_func_inst = f_midi_free_inst;
	opts->index = SNDRV_DEFAULT_IDX1;
	opts->id = SNDRV_DEFAULT_STR1;
	opts->buflen = 256;
	opts->qlen = 32;
	opts->in_ports = 1;
	opts->out_ports = 1;

	config_group_init_type_name(&opts->func_inst.group, "",
				    &midi_func_type);

	return &opts->func_inst;
}

static void f_midi_free(struct usb_function *f)
{
	struct f_midi *midi;
	struct f_midi_opts *opts;
	int i;

	midi = func_to_midi(f);
	opts = container_of(f->fi, struct f_midi_opts, func_inst);
	kfree(midi->id);
	mutex_lock(&opts->lock);
	for (i = opts->in_ports - 1; i >= 0; --i)
		kfree(midi->in_port[i]);
	kfree(midi);
	--opts->refcnt;
	mutex_unlock(&opts->lock);
}

static void f_midi_unbind(struct usb_configuration *c, struct usb_function *f)
{
	struct usb_composite_dev *cdev = f->config->cdev;
	struct f_midi *midi = func_to_midi(f);
	struct snd_card *card;

	DBG(cdev, "unbind\n");

	/* just to be sure */
	f_midi_disable(f);

	card = midi->card;
	midi->card = NULL;
	if (card)
		snd_card_free(card);

	usb_free_all_descriptors(f);
}

static struct usb_function *f_midi_alloc(struct usb_function_instance *fi)
{
	struct f_midi *midi;
	struct f_midi_opts *opts;
	int status, i;

	opts = container_of(fi, struct f_midi_opts, func_inst);

	mutex_lock(&opts->lock);
	/* sanity check */
	if (opts->in_ports > MAX_PORTS || opts->out_ports > MAX_PORTS) {
		mutex_unlock(&opts->lock);
		return ERR_PTR(-EINVAL);
	}

	/* allocate and initialize one new instance */
	midi = kzalloc(sizeof(*midi), GFP_KERNEL);
	if (!midi) {
		mutex_unlock(&opts->lock);
		return ERR_PTR(-ENOMEM);
	}

	for (i = 0; i < opts->in_ports; i++) {
		struct gmidi_in_port *port = kzalloc(sizeof(*port), GFP_KERNEL);

		if (!port) {
			status = -ENOMEM;
			mutex_unlock(&opts->lock);
			goto setup_fail;
		}

		port->midi = midi;
		port->active = 0;
		port->cable = i;
		midi->in_port[i] = port;
	}

	/* set up ALSA midi devices */
	midi->id = kstrdup(opts->id, GFP_KERNEL);
	if (opts->id && !midi->id) {
		status = -ENOMEM;
		mutex_unlock(&opts->lock);
		goto kstrdup_fail;
	}
	midi->in_ports = opts->in_ports;
	midi->out_ports = opts->out_ports;
	midi->index = opts->index;
	midi->buflen = opts->buflen;
	midi->qlen = opts->qlen;
	++opts->refcnt;
	mutex_unlock(&opts->lock);

	midi->func.name		= "gmidi function";
	midi->func.bind		= f_midi_bind;
	midi->func.unbind	= f_midi_unbind;
	midi->func.set_alt	= f_midi_set_alt;
	midi->func.disable	= f_midi_disable;
	midi->func.free_func	= f_midi_free;

	return &midi->func;

kstrdup_fail:
	f_midi_unregister_card(midi);
setup_fail:
	for (--i; i >= 0; i--)
		kfree(midi->in_port[i]);
	kfree(midi);
	return ERR_PTR(status);
}

DECLARE_USB_FUNCTION_INIT(midi, f_midi_alloc_inst, f_midi_alloc);
