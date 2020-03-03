/*
 * AD7887 SPI ADC driver
 *
 * Copyright 2010 Analog Devices Inc.
 *
 * Licensed under the GPL-2 or later.
 */
#ifndef IIO_ADC_AD7887_H_
#define IIO_ADC_AD7887_H_

/**
 * struct ad7887_platform_data - AD7887 ADC driver platform data
 * @en_dual: Whether to use dual channel mode. If set to true AIN1 becomes the
 *	second input channel, and Vref is internally connected to Vdd. If set to
 *	false the device is used in single channel mode and AIN1/Vref is used as
 *	VREF input.
 * @use_onchip_ref: Whether to use the onchip reference. If set to true the
 *	internal 2.5V reference is used. If set to false a external reference is
 *	used.
 */
struct ad7887_platform_data {
	bool en_dual;
	bool use_onchip_ref;
};

#endif /* IIO_ADC_AD7887_H_ */
