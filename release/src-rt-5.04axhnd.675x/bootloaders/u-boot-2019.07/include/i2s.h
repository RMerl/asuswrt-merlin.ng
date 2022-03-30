/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2012 Samsung Electronics
 * R. Chandrasekar <rcsekar@samsung.com>
 */

#ifndef __I2S_H__
#define __I2S_H__

/*
 * DAI hardware audio formats.
 *
 * Describes the physical PCM data formating and clocking. Add new formats
 * to the end.
 */
#define SND_SOC_DAIFMT_I2S		1 /* I2S mode */
#define SND_SOC_DAIFMT_RIGHT_J		2 /* Right Justified mode */
#define SND_SOC_DAIFMT_LEFT_J		3 /* Left Justified mode */
#define SND_SOC_DAIFMT_DSP_A		4 /* L data MSB after FRM LRC */
#define SND_SOC_DAIFMT_DSP_B		5 /* L data MSB during FRM LRC */
#define SND_SOC_DAIFMT_AC97		6 /* AC97 */
#define SND_SOC_DAIFMT_PDM		7 /* Pulse density modulation */

/* left and right justified also known as MSB and LSB respectively */
#define SND_SOC_DAIFMT_MSB		SND_SOC_DAIFMT_LEFT_J
#define SND_SOC_DAIFMT_LSB		SND_SOC_DAIFMT_RIGHT_J

/*
 * DAI hardware signal inversions.
 *
 * Specifies whether the DAI can also support inverted clocks for the specified
 * format.
 */
#define SND_SOC_DAIFMT_NB_NF	(1 << 8) /* normal bit clock + frame */
#define SND_SOC_DAIFMT_NB_IF	(2 << 8) /* normal BCLK + inv FRM */
#define SND_SOC_DAIFMT_IB_NF	(3 << 8) /* invert BCLK + nor FRM */
#define SND_SOC_DAIFMT_IB_IF	(4 << 8) /* invert BCLK + FRM */

/*
 * DAI hardware clock masters.
 *
 * This is wrt the codec, the inverse is true for the interface
 * i.e. if the codec is clk and FRM master then the interface is
 * clk and frame slave.
 */
#define SND_SOC_DAIFMT_CBM_CFM	(1 << 12) /* codec clk & FRM master */
#define SND_SOC_DAIFMT_CBS_CFM	(2 << 12) /* codec clk slave & FRM master */
#define SND_SOC_DAIFMT_CBM_CFS	(3 << 12) /* codec clk master & frame slave */
#define SND_SOC_DAIFMT_CBS_CFS	(4 << 12) /* codec clk & FRM slave */

#define SND_SOC_DAIFMT_FORMAT_MASK	0x000f
#define SND_SOC_DAIFMT_CLOCK_MASK	0x00f0
#define SND_SOC_DAIFMT_INV_MASK		0x0f00
#define SND_SOC_DAIFMT_MASTER_MASK	0xf000

/*
 * Master Clock Directions
 */
#define SND_SOC_CLOCK_IN		0
#define SND_SOC_CLOCK_OUT		1

/* I2S Tx Control */
#define I2S_TX_ON	1
#define I2S_TX_OFF	0

#define FIFO_LENGTH	64

/* I2s Registers */
struct i2s_reg {
	unsigned int con;	/* base + 0 , Control register */
	unsigned int mod;	/* Mode register */
	unsigned int fic;	/* FIFO control register */
	unsigned int psr;	/* Reserved */
	unsigned int txd;	/* Transmit data register */
	unsigned int rxd;	/* Receive Data Register */
};

/* This structure stores the i2s related information */
struct i2s_uc_priv {
	unsigned int rfs;		/* LR clock frame size */
	unsigned int bfs;		/* Bit clock frame size */
	unsigned int audio_pll_clk;	/* Audio pll frequency in Hz */
	unsigned int samplingrate;	/* sampling rate */
	unsigned int bitspersample;	/* bits per sample */
	unsigned int channels;		/* audio channels */
	unsigned int base_address;	/* I2S Register Base */
	unsigned int id;		/* I2S controller id */
};

/* Operations for i2s devices */
struct i2s_ops {
	/**
	 * tx_data() - Transmit audio data
	 *
	 * @dev: I2C device
	 * @data: Data buffer to play
	 * @data_size: Size of data buffer in bytes
	 * @return 0 if OK, -ve on error
	 */
	int (*tx_data)(struct udevice *dev, void *data, uint data_size);
};

#define i2s_get_ops(dev)	((struct i2s_ops *)(dev)->driver->ops)

/**
 * i2s_tx_data() - Transmit audio data
 *
 * @dev: I2C device
 * @data: Data buffer to play
 * @data_size: Size of data buffer in bytes
 * @return 0 if OK, -ve on error
 */
int i2s_tx_data(struct udevice *dev, void *data, uint data_size);

/*
 * Sends the given data through i2s tx
 *
 * @param pi2s_tx	pointer of i2s transmitter parameter structure.
 * @param data		address of the data buffer
 * @param data_size	size of the data (in bytes)
 * @return		int value 0 for success, -1 in case of error
 */
int i2s_transfer_tx_data(struct i2s_uc_priv *pi2s_tx, void *data,
			 uint data_size);

#endif /* __I2S_H__ */
