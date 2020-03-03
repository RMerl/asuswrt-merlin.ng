/*
 * oxfw.c - a part of driver for OXFW970/971 based devices
 *
 * Copyright (c) Clemens Ladisch <clemens@ladisch.de>
 * Licensed under the terms of the GNU General Public License, version 2.
 */

#include "oxfw.h"

#define OXFORD_FIRMWARE_ID_ADDRESS	(CSR_REGISTER_BASE + 0x50000)
/* 0x970?vvvv or 0x971?vvvv, where vvvv = firmware version */

#define OXFORD_HARDWARE_ID_ADDRESS	(CSR_REGISTER_BASE + 0x90020)
#define OXFORD_HARDWARE_ID_OXFW970	0x39443841
#define OXFORD_HARDWARE_ID_OXFW971	0x39373100

#define VENDOR_LOUD		0x000ff2
#define VENDOR_GRIFFIN		0x001292
#define VENDOR_BEHRINGER	0x001564
#define VENDOR_LACIE		0x00d04b

#define SPECIFIER_1394TA	0x00a02d
#define VERSION_AVC		0x010001

MODULE_DESCRIPTION("Oxford Semiconductor FW970/971 driver");
MODULE_AUTHOR("Clemens Ladisch <clemens@ladisch.de>");
MODULE_LICENSE("GPL v2");
MODULE_ALIAS("snd-firewire-speakers");

static bool detect_loud_models(struct fw_unit *unit)
{
	const char *const models[] = {
		"Onyxi",
		"Onyx-i",
		"d.Pro",
		"Mackie Onyx Satellite",
		"Tapco LINK.firewire 4x6",
		"U.420"};
	char model[32];
	unsigned int i;
	int err;

	err = fw_csr_string(unit->directory, CSR_MODEL,
			    model, sizeof(model));
	if (err < 0)
		return false;

	for (i = 0; i < ARRAY_SIZE(models); i++) {
		if (strcmp(models[i], model) == 0)
			break;
	}

	return (i < ARRAY_SIZE(models));
}

static int name_card(struct snd_oxfw *oxfw)
{
	struct fw_device *fw_dev = fw_parent_device(oxfw->unit);
	char vendor[24];
	char model[32];
	const char *d, *v, *m;
	u32 firmware;
	int err;

	/* get vendor name from root directory */
	err = fw_csr_string(fw_dev->config_rom + 5, CSR_VENDOR,
			    vendor, sizeof(vendor));
	if (err < 0)
		goto end;

	/* get model name from unit directory */
	err = fw_csr_string(oxfw->unit->directory, CSR_MODEL,
			    model, sizeof(model));
	if (err < 0)
		goto end;

	err = snd_fw_transaction(oxfw->unit, TCODE_READ_QUADLET_REQUEST,
				 OXFORD_FIRMWARE_ID_ADDRESS, &firmware, 4, 0);
	if (err < 0)
		goto end;
	be32_to_cpus(&firmware);

	/* to apply card definitions */
	if (oxfw->device_info) {
		d = oxfw->device_info->driver_name;
		v = oxfw->device_info->vendor_name;
		m = oxfw->device_info->model_name;
	} else {
		d = "OXFW";
		v = vendor;
		m = model;
	}

	strcpy(oxfw->card->driver, d);
	strcpy(oxfw->card->mixername, m);
	strcpy(oxfw->card->shortname, m);

	snprintf(oxfw->card->longname, sizeof(oxfw->card->longname),
		 "%s %s (OXFW%x %04x), GUID %08x%08x at %s, S%d",
		 v, m, firmware >> 20, firmware & 0xffff,
		 fw_dev->config_rom[3], fw_dev->config_rom[4],
		 dev_name(&oxfw->unit->device), 100 << fw_dev->max_speed);
end:
	return err;
}

/*
 * This module releases the FireWire unit data after all ALSA character devices
 * are released by applications. This is for releasing stream data or finishing
 * transactions safely. Thus at returning from .remove(), this module still keep
 * references for the unit.
 */
static void oxfw_card_free(struct snd_card *card)
{
	struct snd_oxfw *oxfw = card->private_data;
	unsigned int i;

	snd_oxfw_stream_destroy_simplex(oxfw, &oxfw->rx_stream);
	if (oxfw->has_output)
		snd_oxfw_stream_destroy_simplex(oxfw, &oxfw->tx_stream);

	fw_unit_put(oxfw->unit);

	for (i = 0; i < SND_OXFW_STREAM_FORMAT_ENTRIES; i++) {
		kfree(oxfw->tx_stream_formats[i]);
		kfree(oxfw->rx_stream_formats[i]);
	}

	mutex_destroy(&oxfw->mutex);
}

static int oxfw_probe(struct fw_unit *unit,
		       const struct ieee1394_device_id *id)
{
	struct snd_card *card;
	struct snd_oxfw *oxfw;
	int err;

	if ((id->vendor_id == VENDOR_LOUD) && !detect_loud_models(unit))
		return -ENODEV;

	err = snd_card_new(&unit->device, -1, NULL, THIS_MODULE,
			   sizeof(*oxfw), &card);
	if (err < 0)
		return err;

	card->private_free = oxfw_card_free;
	oxfw = card->private_data;
	oxfw->card = card;
	mutex_init(&oxfw->mutex);
	oxfw->unit = fw_unit_get(unit);
	oxfw->device_info = (const struct device_info *)id->driver_data;
	spin_lock_init(&oxfw->lock);
	init_waitqueue_head(&oxfw->hwdep_wait);

	err = snd_oxfw_stream_discover(oxfw);
	if (err < 0)
		goto error;

	err = name_card(oxfw);
	if (err < 0)
		goto error;

	err = snd_oxfw_create_pcm(oxfw);
	if (err < 0)
		goto error;

	if (oxfw->device_info) {
		err = snd_oxfw_create_mixer(oxfw);
		if (err < 0)
			goto error;
	}

	snd_oxfw_proc_init(oxfw);

	err = snd_oxfw_create_midi(oxfw);
	if (err < 0)
		goto error;

	err = snd_oxfw_create_hwdep(oxfw);
	if (err < 0)
		goto error;

	err = snd_oxfw_stream_init_simplex(oxfw, &oxfw->rx_stream);
	if (err < 0)
		goto error;
	if (oxfw->has_output) {
		err = snd_oxfw_stream_init_simplex(oxfw, &oxfw->tx_stream);
		if (err < 0)
			goto error;
	}

	err = snd_card_register(card);
	if (err < 0) {
		snd_oxfw_stream_destroy_simplex(oxfw, &oxfw->rx_stream);
		if (oxfw->has_output)
			snd_oxfw_stream_destroy_simplex(oxfw, &oxfw->tx_stream);
		goto error;
	}
	dev_set_drvdata(&unit->device, oxfw);

	return 0;
error:
	snd_card_free(card);
	return err;
}

static void oxfw_bus_reset(struct fw_unit *unit)
{
	struct snd_oxfw *oxfw = dev_get_drvdata(&unit->device);

	fcp_bus_reset(oxfw->unit);

	mutex_lock(&oxfw->mutex);

	snd_oxfw_stream_update_simplex(oxfw, &oxfw->rx_stream);
	if (oxfw->has_output)
		snd_oxfw_stream_update_simplex(oxfw, &oxfw->tx_stream);

	mutex_unlock(&oxfw->mutex);
}

static void oxfw_remove(struct fw_unit *unit)
{
	struct snd_oxfw *oxfw = dev_get_drvdata(&unit->device);

	/* No need to wait for releasing card object in this context. */
	snd_card_free_when_closed(oxfw->card);
}

static const struct device_info griffin_firewave = {
	.driver_name = "FireWave",
	.vendor_name = "Griffin",
	.model_name = "FireWave",
	.mixer_channels = 6,
	.mute_fb_id   = 0x01,
	.volume_fb_id = 0x02,
};

static const struct device_info lacie_speakers = {
	.driver_name = "FWSpeakers",
	.vendor_name = "LaCie",
	.model_name = "FireWire Speakers",
	.mixer_channels = 1,
	.mute_fb_id   = 0x01,
	.volume_fb_id = 0x01,
};

static const struct ieee1394_device_id oxfw_id_table[] = {
	{
		.match_flags  = IEEE1394_MATCH_VENDOR_ID |
				IEEE1394_MATCH_MODEL_ID |
				IEEE1394_MATCH_SPECIFIER_ID |
				IEEE1394_MATCH_VERSION,
		.vendor_id    = VENDOR_GRIFFIN,
		.model_id     = 0x00f970,
		.specifier_id = SPECIFIER_1394TA,
		.version      = VERSION_AVC,
		.driver_data  = (kernel_ulong_t)&griffin_firewave,
	},
	{
		.match_flags  = IEEE1394_MATCH_VENDOR_ID |
				IEEE1394_MATCH_MODEL_ID |
				IEEE1394_MATCH_SPECIFIER_ID |
				IEEE1394_MATCH_VERSION,
		.vendor_id    = VENDOR_LACIE,
		.model_id     = 0x00f970,
		.specifier_id = SPECIFIER_1394TA,
		.version      = VERSION_AVC,
		.driver_data  = (kernel_ulong_t)&lacie_speakers,
	},
	/* Behringer,F-Control Audio 202 */
	{
		.match_flags	= IEEE1394_MATCH_VENDOR_ID |
				  IEEE1394_MATCH_MODEL_ID,
		.vendor_id	= VENDOR_BEHRINGER,
		.model_id	= 0x00fc22,
	},
	/*
	 * Any Mackie(Loud) models (name string/model id):
	 *  Onyx-i series (former models):	0x081216
	 *  Mackie Onyx Satellite:		0x00200f
	 *  Tapco LINK.firewire 4x6:		0x000460
	 *  d.2 pro:				Unknown
	 *  d.4 pro:				Unknown
	 *  U.420:				Unknown
	 *  U.420d:				Unknown
	 */
	{
		.match_flags	= IEEE1394_MATCH_VENDOR_ID |
				  IEEE1394_MATCH_SPECIFIER_ID |
				  IEEE1394_MATCH_VERSION,
		.vendor_id	= VENDOR_LOUD,
		.specifier_id	= SPECIFIER_1394TA,
		.version	= VERSION_AVC,
	},
	{ }
};
MODULE_DEVICE_TABLE(ieee1394, oxfw_id_table);

static struct fw_driver oxfw_driver = {
	.driver   = {
		.owner	= THIS_MODULE,
		.name	= KBUILD_MODNAME,
		.bus	= &fw_bus_type,
	},
	.probe    = oxfw_probe,
	.update   = oxfw_bus_reset,
	.remove   = oxfw_remove,
	.id_table = oxfw_id_table,
};

static int __init snd_oxfw_init(void)
{
	return driver_register(&oxfw_driver.driver);
}

static void __exit snd_oxfw_exit(void)
{
	driver_unregister(&oxfw_driver.driver);
}

module_init(snd_oxfw_init);
module_exit(snd_oxfw_exit);
