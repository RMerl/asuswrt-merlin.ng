/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2015 Samsung Electronics
 * Przemyslaw Marczak <p.marczak@samsung.com>
 */

#ifndef _ADC_H_
#define _ADC_H_

/* ADC_CHANNEL() - ADC channel bit mask, to select only required channels */
#define ADC_CHANNEL(x)		(1 << x)

/* The last possible selected channel with 32-bit mask */
#define ADC_MAX_CHANNEL		31

/**
 * adc_data_format: define the ADC output data format, can be useful when
 * the device's input Voltage range is bipolar.
 * - ADC_DATA_FORMAT_BIN - binary offset
 * - ADC_DATA_FORMAT_2S  - two's complement
 *
 * Note: Device's driver should fill the 'data_format' field of its uclass's
 * platform data using one of the above data format types.
 */
enum adc_data_format {
	ADC_DATA_FORMAT_BIN,
	ADC_DATA_FORMAT_2S,
};

/**
 * struct adc_channel - structure to hold channel conversion data.
 * Useful to keep the result of a multi-channel conversion output.
 *
 * @id   - channel id
 * @data - channel conversion data
 */
struct adc_channel {
	int id;
	unsigned int data;
};

/**
 * struct adc_uclass_plat - basic ADC info
 *
 * Note: The positive/negative reference Voltage is only a name and it doesn't
 * provide an information about the value polarity. It is possible, for both
 * values to be a negative or positive. For this purpose the uclass's platform
 * data provides a bool fields: 'vdd/vss_supply_is_negative'. This is useful,
 * since the regulator API returns only a positive Voltage values.
 *
 * To get the reference Voltage values with polarity, use functions:
 * - adc_vdd_value()
 * - adc_vss_value()
 * Those are useful for some cases of ADC's references, e.g.:
 * * Vdd: +3.3V; Vss: -3.3V -> 6.6 Vdiff
 * * Vdd: +3.3V; Vss: +0.3V -> 3.0 Vdiff
 * * Vdd: +3.3V; Vss:  0.0V -> 3.3 Vdiff
 * The last one is usually standard and doesn't require the fdt polarity info.
 *
 * For more informations read binding info:
 * - doc/device-tree-bindings/adc/adc.txt
 *
 * @data_mask              - conversion output data mask
 * @data_timeout_us        - single channel conversion timeout
 * @multidata_timeout_us   - multi channel conversion timeout
 * @channel_mask           - bit mask of available channels [0:31]
 * @vdd_supply             - positive reference Voltage supply (regulator)
 * @vss_supply             - negative reference Voltage supply (regulator)
 * @vdd_polarity_negative  - positive reference Voltage has negative polarity
 * @vss_polarity_negative  - negative reference Voltage has negative polarity
 * @vdd_microvolts         - positive reference Voltage value
 * @vss_microvolts         - negative reference Voltage value
 */
struct adc_uclass_plat {
	int data_format;
	unsigned int data_mask;
	unsigned int data_timeout_us;
	unsigned int multidata_timeout_us;
	unsigned int channel_mask;
	struct udevice *vdd_supply;
	struct udevice *vss_supply;
	bool vdd_polarity_negative;
	bool vss_polarity_negative;
	int vdd_microvolts;
	int vss_microvolts;
};

/**
 * struct adc_ops - ADC device operations for single/multi-channel operation.
 */
struct adc_ops {
	/**
	 * start_channel() - start conversion with its default parameters
	 *                   for the given channel number.
	 *
	 * @dev:          ADC device to init
	 * @channel:      analog channel number
	 * @return:       0 if OK, -ve on error
	 */
	int (*start_channel)(struct udevice *dev, int channel);

	/**
	 * start_channels() - start conversion with its default parameters
	 *                    for the channel numbers selected by the bit mask.
	 *
	 * This is optional, useful when the hardware supports multichannel
	 * conversion by the single software trigger.
	 *
	 * @dev:          ADC device to init
	 * @channel_mask: bit mask of selected analog channels
	 * @return:       0 if OK, -ve on error
	 */
	int (*start_channels)(struct udevice *dev, unsigned int channel_mask);

	/**
	 * channel_data() - get conversion output data for the given channel.
	 *
	 * Note: The implementation of this function should only check, that
	 * the conversion data is available at the call time. If the hardware
	 * requires some delay to get the data, then this function should
	 * return with -EBUSY value. The ADC API will call it in a loop,
	 * until the data is available or the timeout expires. The maximum
	 * timeout for this operation is defined by the field 'data_timeout_us'
	 * in ADC uclasses platform data structure.
	 *
	 * @dev:          ADC device to trigger
	 * @channel:      selected analog channel number
	 * @data:         returned pointer to selected channel's output data
	 * @return:       0 if OK, -EBUSY if busy, and other negative on error
	 */
	int (*channel_data)(struct udevice *dev, int channel,
			    unsigned int *data);

	/**
	 * channels_data() - get conversion data for the selected channels.
	 *
	 * This is optional, useful when multichannel conversion is supported
	 * by the hardware, by the single software trigger.
	 *
	 * For the proper implementation, please look at the 'Note' for the
	 * above method. The only difference is in used timeout value, which
	 * is defined by field 'multidata_timeout_us'.
	 *
	 * @dev:          ADC device to trigger
	 * @channel_mask: bit mask of selected analog channels
	 * @channels:     returned pointer to array of output data for channels
	 *                selected by the given mask
	 * @return:       0 if OK, -ve on error
	 */
	int (*channels_data)(struct udevice *dev, unsigned int channel_mask,
			     struct adc_channel *channels);

	/**
	 * stop() - stop conversion of the given ADC device
	 *
	 * @dev:          ADC device to stop
	 * @return:       0 if OK, -ve on error
	 */
	int (*stop)(struct udevice *dev);
};

/**
 * adc_start_channel() - start conversion for given device/channel and exit.
 *
 * @dev:     ADC device
 * @channel: analog channel number
 * @return:  0 if OK, -ve on error
 */
int adc_start_channel(struct udevice *dev, int channel);

/**
 * adc_start_channels() - start conversion for given device/channels and exit.
 *
 * Note:
 * To use this function, device must implement method: start_channels().
 *
 * @dev:          ADC device to start
 * @channel_mask: channel selection - a bit mask
 * @channel_mask: bit mask of analog channels
 * @return:       0 if OK, -ve on error
 */
int adc_start_channels(struct udevice *dev, unsigned int channel_mask);

/**
 * adc_channel_data() - get conversion data for the given device channel number.
 *
 * @dev:     ADC device to read
 * @channel: analog channel number
 * @data:    pointer to returned channel's data
 * @return:  0 if OK, -ve on error
 */
int adc_channel_data(struct udevice *dev, int channel, unsigned int *data);

/**
 * adc_channels_data() - get conversion data for the channels selected by mask
 *
 * Note:
 * To use this function, device must implement methods:
 * - start_channels()
 * - channels_data()
 *
 * @dev:          ADC device to read
 * @channel_mask: channel selection - a bit mask
 * @channels:     pointer to structure array of returned data for each channel
 * @return:       0 if OK, -ve on error
 */
int adc_channels_data(struct udevice *dev, unsigned int channel_mask,
		      struct adc_channel *channels);

/**
 * adc_data_mask() - get data mask (ADC resolution bitmask) for given ADC device
 *
 * This can be used if adc uclass platform data is filled.
 *
 * @dev:       ADC device to check
 * @data_mask: pointer to the returned data bitmask
 * @return: 0 if OK, -ve on error
 */
int adc_data_mask(struct udevice *dev, unsigned int *data_mask);

/**
 * adc_channel_mask() - get channel mask for given ADC device
 *
 * This can be used if adc uclass platform data is filled.
 *
 * @dev:       ADC device to check
 * @channel_mask: pointer to the returned channel bitmask
 * @return: 0 if OK, -ve on error
 */
int adc_channel_mask(struct udevice *dev, unsigned int *channel_mask);

/**
 * adc_channel_single_shot() - get output data of conversion for the ADC
 * device's channel. This function searches for the device with the given name,
 * starts the given channel conversion and returns the output data.
 *
 * Note: To use this function, device must implement metods:
 * - start_channel()
 * - channel_data()
 *
 * @name:    device's name to search
 * @channel: device's input channel to init
 * @data:    pointer to conversion output data
 * @return:  0 if OK, -ve on error
 */
int adc_channel_single_shot(const char *name, int channel, unsigned int *data);

/**
 * adc_channels_single_shot() - get ADC conversion output data for the selected
 * device's channels. This function searches for the device by the given name,
 * starts the selected channels conversion and returns the output data as array
 * of type 'struct adc_channel'.
 *
 * Note: This function can be used if device implements one of ADC's single
 * or multi-channel operation API. If multi-channel operation is not supported,
 * then each selected channel is triggered by the sequence start/data in a loop.
 *
 * @name:         device's name to search
 * @channel_mask: channel selection - a bit mask
 * @channels:     pointer to conversion output data for the selected channels
 * @return:       0 if OK, -ve on error
 */
int adc_channels_single_shot(const char *name, unsigned int channel_mask,
			     struct adc_channel *channels);

/**
 * adc_vdd_value() - get the ADC device's positive reference Voltage value
 *
 * Note: Depending on bool value 'vdd_supply_is_negative' of platform data,
 * the returned uV value can be negative, and it's not an error.
 *
 * @dev:     ADC device to check
 * @uV:      Voltage value with polarization sign (uV)
 * @return:  0 on success or -ve on error
*/
int adc_vdd_value(struct udevice *dev, int *uV);

/**
 * adc_vss_value() - get the ADC device's negative reference Voltage value
 *
 * Note: Depending on bool value 'vdd_supply_is_negative' of platform data,
 * the returned uV value can be negative, and it's not an error.
 *
 * @dev:     ADC device to check
 * @uV:      Voltage value with polarization sign (uV)
 * @return:  0 on success or -ve on error
*/
int adc_vss_value(struct udevice *dev, int *uV);

/**
 * adc_stop() - stop operation for given ADC device.
 *
 * @dev:     ADC device to stop
 * @return:  0 if OK, -ve on error
 */
int adc_stop(struct udevice *dev);

/**
 * adc_raw_to_uV() - converts raw value to microvolts for given ADC device.
 *
 * @dev:     ADC device used from conversion
 * @raw:     raw value to convert
 * @uV:	     converted value in microvolts
 * @return:  0 on success or -ve on error
 */
int adc_raw_to_uV(struct udevice *dev, unsigned int raw, int *uV);

#endif
