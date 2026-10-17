// SPDX-License-Identifier: GPL-2.0+
/*
 * logicore_dp_tx.c
 *
 * Driver for XILINX LogiCore DisplayPort v6.1 TX (Source)
 * based on Xilinx dp_v3_1 driver sources, updated to dp_v4_0
 *
 * (C) Copyright 2016
 * Dirk Eibach,  Guntermann & Drunck GmbH, dirk.eibach@gdsys.cc
 */

#include <common.h>
#include <clk.h>
#include <display.h>
#include <dm.h>
#include <errno.h>
#include <linux/delay.h>

#include "axi.h"
#include "logicore_dp_dpcd.h"
#include "logicore_dp_tx.h"
#include "logicore_dp_tx_regif.h"

/* Default AXI clock frequency value */
#define S_AXI_CLK_DEFAULT 100000000

/* Default DP phy clock value */
#define PHY_CLOCK_SELECT_DEFAULT PHY_CLOCK_SELECT_540GBPS

/* The maximum voltage swing level is 3 */
#define MAXIMUM_VS_LEVEL 3
/* The maximum pre-emphasis level is 3 */
#define MAXIMUM_PE_LEVEL 3

/* Error out if an AUX request yields a defer reply more than 50 times */
#define AUX_MAX_DEFER_COUNT 50
/* Error out if an AUX request times out more than 50 times awaiting a reply */
#define AUX_MAX_TIMEOUT_COUNT 50
/* Error out if checking for a connected device times out more than 50 times */
#define IS_CONNECTED_MAX_TIMEOUT_COUNT 50

/**
 * enum link_training_states - States for link training state machine
 * @TS_CLOCK_RECOVERY:       State for clock recovery
 * @TS_CHANNEL_EQUALIZATION: State for channel equalization
 * @TS_ADJUST_LINK_RATE:     State where link rate is reduced in reaction to
 *			     failed link training
 * @TS_ADJUST_LANE_COUNT:    State where lane count is reduced in reaction to
 *			     failed link training
 * @TS_FAILURE:              State of link training failure
 * @TS_SUCCESS::             State for successfully completed link training
 */
enum link_training_states {
	TS_CLOCK_RECOVERY,
	TS_CHANNEL_EQUALIZATION,
	TS_ADJUST_LINK_RATE,
	TS_ADJUST_LANE_COUNT,
	TS_FAILURE,
	TS_SUCCESS
};

/**
 * struct aux_transaction - Description of an AUX channel transaction
 * @cmd_code:  Command code of the transaction
 * @num_bytes: The number of bytes in the transaction's payload data
 * @address:   The DPCD address of the transaction
 * @data:      Payload data of the AUX channel transaction
 */
struct aux_transaction {
	u16 cmd_code;
	u8 num_bytes;
	u32 address;
	u8 *data;
};

/**
 * struct main_stream_attributes - Main stream attributes
 * @pixel_clock_hz: Pixel clock of the stream (in Hz)
 * @misc_0:                    Miscellaneous stream attributes 0 as specified
 *			       by the DisplayPort 1.2 specification
 * @misc_1:                    Miscellaneous stream attributes 1 as specified
 *			       by the DisplayPort 1.2 specification
 * @n_vid:                     N value for the video stream
 * @m_vid:                     M value used to recover the video clock from the
 *			       link clock
 * @user_pixel_width:          Width of the user data input port
 * @data_per_lane:             Used to translate the number of pixels per line
 *			       to the native internal 16-bit datapath
 * @avg_bytes_per_tu:          Average number of bytes per transfer unit,
 *			       scaled up by a factor of 1000
 * @transfer_unit_size:        Size of the transfer unit in the framing logic
 *			       In MST mode, this is also the number of time
 *			       slots that are alloted in the payload ID table
 * @init_wait:                 Number of initial wait cycles at the start of a
 *			       new line by the framing logic
 * @bits_per_color:            Bits per color component
 * @component_format:          The component format currently in use by the
 *			       video stream
 * @dynamic_range:             The dynamic range currently in use by the video
 *			       stream
 * @y_cb_cr_colorimetry:       The YCbCr colorimetry currently in use by the
 *			       video stream
 * @synchronous_clock_mode:    Synchronous clock mode is currently in use by
 *			       the video stream
 * @override_user_pixel_width: If set to 1, the value stored for
 *			       user_pixel_width will be used as the pixel width
 * @h_start:                   Horizontal blank start (pixels)
 * @h_active:                  Horizontal active resolution (pixels)
 * @h_sync_width:              Horizontal sync width (pixels)
 * @h_total:                   Horizontal total (pixels)
 * @h_sync_polarity:           Horizontal sync polarity (0=neg|1=pos)
 * @v_start:                   Vertical blank start (in lines)
 * @v_active:                  Vertical active resolution (lines)
 * @v_sync_width:              Vertical sync width (lines)
 * @v_total:                   Vertical total (lines)
 * @v_sync_polarity:           Vertical sync polarity (0=neg|1=pos)
 *
 * All porch parameters have been removed, because our videodata is
 * hstart/vstart based, and there is no benefit in keeping the porches
 */
struct main_stream_attributes {
	u32 pixel_clock_hz;
	u32 misc_0;
	u32 misc_1;
	u32 n_vid;
	//u32 m_vid;
	u32 user_pixel_width;
	u32 data_per_lane;
	u32 avg_bytes_per_tu;
	u32 transfer_unit_size;
	u32 init_wait;
	u32 bits_per_color;
	u8 component_format;
	u8 dynamic_range;
	u8 y_cb_cr_colorimetry;
	u8 synchronous_clock_mode;
	u8 override_user_pixel_width;
	u32 h_start;
	u16 h_active;
	u16 h_sync_width;
	u16 h_total;
	bool h_sync_polarity;
	u32 v_start;
	u16 v_active;
	u16 v_sync_width;
	u16 v_total;
	bool v_sync_polarity;
};

/**
 * struct link_config - Description of link configuration
 * @lane_count:                    Currently selected lane count for this link
 * @link_rate:                     Currently selected link rate for this link
 * @scrambler_en:                  Flag to determine whether the scrambler is
 *				   enabled for this link
 * @enhanced_framing_mode:         Flag to determine whether enhanced framing
 *				   mode is active for this link
 * @max_lane_count:                Maximum lane count for this link
 * @max_link_rate:                 Maximum link rate for this link
 * @support_enhanced_framing_mode: Flag to indicate whether the link supports
 *				   enhanced framing mode
 * @vs_level:                      Voltage swing for each lane
 * @pe_level:                      Pre-emphasis/cursor level for each lane
 */
struct link_config {
	u8 lane_count;
	u8 link_rate;
	bool scrambler_en;
	bool enhanced_framing_mode;
	u8 max_lane_count;
	u8 max_link_rate;
	bool support_enhanced_framing_mode;
	u8 vs_level;
	u8 pe_level;
};

/**
 * struct dp_tx - Private data structure of LogiCore DP TX devices
 *
 * @base:                   Address of register base of device
 * @s_axi_clk:              The AXI clock frequency in Hz
 * @train_adaptive:         Use adaptive link trainig (i.e. successively reduce
 *			    link rate and/or lane count) for this device
 * @max_link_rate:          Maximum link rate for this device
 * @max_lane_count:         Maximum lane count for this device
 * @dpcd_rx_caps:           RX device's status registers, see below
 * @lane_status_ajd_reqs:   Lane status and adjustment requests information for
 *			    this device
 * @link_config:            The link configuration for this device
 * @main_stream_attributes: MSA set for this device
 *
 * dpcd_rx_caps is a raw read of the RX device's status registers. The first 4
 * bytes correspond to the lane status associated with clock recovery, channel
 * equalization, symbol lock, and interlane alignment. The remaining 2 bytes
 * represent the pre-emphasis and voltage swing level adjustments requested by
 * the RX device.
 */
struct dp_tx {
	u32 base;
	u32 s_axi_clk;
	bool train_adaptive;
	u8 max_link_rate;
	u8 max_lane_count;
	u8 dpcd_rx_caps[16];
	u8 lane_status_ajd_reqs[6];
	struct link_config link_config;
	struct main_stream_attributes main_stream_attributes;
};

/*
 * Internal API
 */

/**
 * get_reg() - Read a register of a LogiCore DP TX device
 * @dev: The LogiCore DP TX device in question
 * @reg: The offset of the register to read
 *
 * Return: The read register value
 */
static u32 get_reg(struct udevice *dev, u32 reg)
{
	struct dp_tx *dp_tx = dev_get_priv(dev);
	u32 value = 0;
	int res;

	/* TODO(mario.six@gdsys.cc): error handling */
	res = axi_read(dev->parent, dp_tx->base + reg, &value, AXI_SIZE_32);
	if (res < 0)
		printf("%s() failed; res = %d\n", __func__, res);

	return value;
}

/**
 * set_reg() - Write a register of a LogiCore DP TX device
 * @dev:   The LogiCore DP TX device in question
 * @reg:   The offset of the register to write
 * @value: The value to write to the register
 */
static void set_reg(struct udevice *dev, u32 reg, u32 value)
{
	struct dp_tx *dp_tx = dev_get_priv(dev);

	axi_write(dev->parent, dp_tx->base + reg, &value, AXI_SIZE_32);
}

/**
 * is_connected() - Check if there is a connected RX device
 * @dev: The LogiCore DP TX device in question
 *
 * The Xilinx original calls msleep_interruptible at least once, ignoring
 * status.
 *
 * Return: true if a connected RX device was detected, false otherwise
 */
static bool is_connected(struct udevice *dev)
{
	u8 retries = 0;

	do {
		int status = get_reg(dev, REG_INTERRUPT_SIG_STATE) &
			     INTERRUPT_SIG_STATE_HPD_STATE_MASK;
		if (status)
			return true;

		udelay(1000);
	} while (retries++ < IS_CONNECTED_MAX_TIMEOUT_COUNT);

	return false;
}

/**
 * wait_phy_ready() - Wait for the DisplayPort PHY to come out of reset
 * @dev:  The LogiCore DP TX device in question
 * @mask: Bit mask specifying which bit in the status register should be waited
 *	  for
 *
 * Return: 0 if wait succeeded, -ve if error occurred
 */
static int wait_phy_ready(struct udevice *dev, u32 mask)
{
	u16 timeout = 20000;
	u32 phy_status;

	/* Wait until the PHY is ready. */
	do {
		phy_status = get_reg(dev, REG_PHY_STATUS) & mask;

		/* Protect against an infinite loop. */
		if (!timeout--)
			return -ETIMEDOUT;

		udelay(20);
	} while (phy_status != mask);

	return 0;
}

/* AUX channel access */

/**
 * aux_wait_ready() -  Wait until another request is no longer in progress
 * @dev: The LogiCore DP TX device in question
 *
 * Return: 0 if wait succeeded, -ve if error occurred
 */
static int aux_wait_ready(struct udevice *dev)
{
	int status;
	u32 timeout = 100;

	/* Wait until the DisplayPort TX core is ready. */
	do {
		status = get_reg(dev, REG_INTERRUPT_SIG_STATE);

		/* Protect against an infinite loop. */
		if (!timeout--)
			return -ETIMEDOUT;
		udelay(20);
	} while (status & REPLY_STATUS_REPLY_IN_PROGRESS_MASK);

	return 0;
}

/**
 * aux_wait_reply() - Wait for reply on AUX channel
 * @dev: The LogiCore DP TX device in question
 *
 * Wait for a reply indicating that the most recent AUX request
 * has been received by the RX device.
 *
 * Return: 0 if wait succeeded, -ve if error occurred
 */
static int aux_wait_reply(struct udevice *dev)
{
	u32 timeout = 100;

	while (timeout > 0) {
		int status = get_reg(dev, REG_REPLY_STATUS);

		/* Check for error. */
		if (status & REPLY_STATUS_REPLY_ERROR_MASK)
			return -ETIMEDOUT;

		/* Check for a reply. */
		if ((status & REPLY_STATUS_REPLY_RECEIVED_MASK) &&
		    !(status &
		      REPLY_STATUS_REQUEST_IN_PROGRESS_MASK) &&
		    !(status &
		      REPLY_STATUS_REPLY_IN_PROGRESS_MASK)) {
			return 0;
		}

		timeout--;
		udelay(20);
	}

	return -ETIMEDOUT;
}

/**
 * aux_request_send() - Send request on the AUX channel
 * @dev:     The LogiCore DP TX device in question
 * @request: The request to send
 *
 * Submit the supplied AUX request to the RX device over the AUX
 * channel by writing the command, the destination address, (the write buffer
 * for write commands), and the data size to the DisplayPort TX core.
 *
 * This is the lower-level sending routine, which is called by aux_request().
 *
 * Return: 0 if request was sent successfully, -ve on error
 */
static int aux_request_send(struct udevice *dev,
			    struct aux_transaction *request)
{
	u32 timeout_count;
	int status;
	u8 index;

	/* Ensure that any pending AUX transactions have completed. */
	timeout_count = 0;
	do {
		status = get_reg(dev, REG_REPLY_STATUS);

		udelay(20);
		timeout_count++;
		if (timeout_count >= AUX_MAX_TIMEOUT_COUNT)
			return -ETIMEDOUT;
	} while ((status & REPLY_STATUS_REQUEST_IN_PROGRESS_MASK) ||
		 (status & REPLY_STATUS_REPLY_IN_PROGRESS_MASK));

	set_reg(dev, REG_AUX_ADDRESS, request->address);

	if (request->cmd_code == AUX_CMD_WRITE ||
	    request->cmd_code == AUX_CMD_I2C_WRITE ||
	    request->cmd_code == AUX_CMD_I2C_WRITE_MOT) {
		/* Feed write data into the DisplayPort TX core's write FIFO. */
		for (index = 0; index < request->num_bytes; index++) {
			set_reg(dev,
				REG_AUX_WRITE_FIFO, request->data[index]);
		}
	}

	/* Submit the command and the data size. */
	set_reg(dev, REG_AUX_CMD,
		((request->cmd_code << AUX_CMD_SHIFT) |
		 ((request->num_bytes - 1) &
		  AUX_CMD_NBYTES_TRANSFER_MASK)));

	/* Check for a reply from the RX device to the submitted request. */
	status = aux_wait_reply(dev);
	if (status)
		/* Waiting for a reply timed out. */
		return -ETIMEDOUT;

	/* Analyze the reply. */
	status = get_reg(dev, REG_AUX_REPLY_CODE);
	if (status == AUX_REPLY_CODE_DEFER ||
	    status == AUX_REPLY_CODE_I2C_DEFER) {
		/* The request was deferred. */
		return -EAGAIN;
	} else if ((status == AUX_REPLY_CODE_NACK) ||
		   (status == AUX_REPLY_CODE_I2C_NACK)) {
		/* The request was not acknowledged. */
		return -EIO;
	}

	/* The request was acknowledged. */

	if (request->cmd_code == AUX_CMD_READ ||
	    request->cmd_code == AUX_CMD_I2C_READ ||
	    request->cmd_code == AUX_CMD_I2C_READ_MOT) {
		/* Wait until all data has been received. */
		timeout_count = 0;
		do {
			status = get_reg(dev, REG_REPLY_DATA_COUNT);

			udelay(100);
			timeout_count++;
			if (timeout_count >= AUX_MAX_TIMEOUT_COUNT)
				return -ETIMEDOUT;
		} while (status != request->num_bytes);

		/* Obtain the read data from the reply FIFO. */
		for (index = 0; index < request->num_bytes; index++)
			request->data[index] = get_reg(dev, REG_AUX_REPLY_DATA);
	}

	return 0;
}

/**
 * aux_request() - Submit request on the AUX channel
 * @dev:     The LogiCore DP TX device in question
 * @request: The request to submit
 *
 * Submit the supplied AUX request to the RX device over the AUX
 * channel. If waiting for a reply times out, or if the DisplayPort TX core
 * indicates that the request was deferred, the request is sent again (up to a
 * maximum specified by AUX_MAX_DEFER_COUNT|AUX_MAX_TIMEOUT_COUNT).
 *
 * Return: 0 if request was submitted successfully, -ve on error
 */
static int aux_request(struct udevice *dev, struct aux_transaction *request)
{
	u32 defer_count = 0;
	u32 timeout_count = 0;

	while ((defer_count < AUX_MAX_DEFER_COUNT) &&
	       (timeout_count < AUX_MAX_TIMEOUT_COUNT)) {
		int status = aux_wait_ready(dev);

		if (status) {
			/* The RX device isn't ready yet. */
			timeout_count++;
			continue;
		}

		status = aux_request_send(dev, request);
		if (status == -EAGAIN) {
			/* The request was deferred. */
			defer_count++;
		} else if (status == -ETIMEDOUT) {
			/* Waiting for a reply timed out. */
			timeout_count++;
		} else {
			/*
			 * -EIO indicates that the request was NACK'ed,
			 * 0 indicates that the request was ACK'ed.
			 */
			return status;
		}

		udelay(100);
	}

	/* The request was not successfully received by the RX device. */
	return -ETIMEDOUT;
}

/**
 * aux_common() - Common (read/write) AUX communication transmission
 * @dev:       The LogiCore DP TX device in question
 * @cmd_type:  Command code of the transaction
 * @address:   The DPCD address of the transaction
 * @num_bytes: Number of bytes in the payload data
 * @data:      The payload data of the AUX command
 *
 * Common sequence of submitting an AUX command for AUX read, AUX write,
 * I2C-over-AUX read, and I2C-over-AUX write transactions. If required, the
 * reads and writes are split into multiple requests, each acting on a maximum
 * of 16 bytes.
 *
 * Return: 0 if OK, -ve on error
 */
static int aux_common(struct udevice *dev, u32 cmd_type, u32 address,
		      u32 num_bytes, u8 *data)
{
	struct aux_transaction request;
	u32 bytes_left;

	/*
	 * Set the start address for AUX transactions. For I2C transactions,
	 * this is the address of the I2C bus.
	 */
	request.address = address;

	bytes_left = num_bytes;
	while (bytes_left) {
		int status;

		request.cmd_code = cmd_type;

		if (cmd_type == AUX_CMD_READ ||
		    cmd_type == AUX_CMD_WRITE) {
			/* Increment address for normal AUX transactions. */
			request.address = address + (num_bytes - bytes_left);
		}

		/* Increment the pointer to the supplied data buffer. */
		request.data = &data[num_bytes - bytes_left];

		request.num_bytes = (bytes_left > 16) ? 16 : bytes_left;
		bytes_left -= request.num_bytes;

		if (cmd_type == AUX_CMD_I2C_READ && bytes_left) {
			/*
			 * Middle of a transaction I2C read request. Override
			 * the command code that was set to cmd_type.
			 */
			request.cmd_code = AUX_CMD_I2C_READ_MOT;
		} else if ((cmd_type == AUX_CMD_I2C_WRITE) && bytes_left) {
			/*
			 * Middle of a transaction I2C write request. Override
			 * the command code that was set to cmd_type.
			 */
			request.cmd_code = AUX_CMD_I2C_WRITE_MOT;
		}

		status = aux_request(dev, &request);
		if (status)
			return status;
	}

	return 0;
}

/**
 * aux_read() - Issue AUX read request
 * @dev:           The LogiCore DP TX device in question
 * @dpcd_address:  The DPCD address to read from
 * @bytes_to_read: Number of bytes to read
 * @read_data:     Buffer to receive the read data
 *
 * Issue a read request over the AUX channel that will read from the RX
 * device's DisplayPort Configuration data (DPCD) address space. The read
 * message will be divided into multiple transactions which read a maximum of
 * 16 bytes each.
 *
 * Return: 0 if read operation was successful, -ve on error
 */
static int aux_read(struct udevice *dev, u32 dpcd_address, u32 bytes_to_read,
		    void *read_data)
{
	int status;

	if (!is_connected(dev))
		return -ENODEV;

	/* Send AUX read transaction. */
	status = aux_common(dev, AUX_CMD_READ, dpcd_address,
			    bytes_to_read, (u8 *)read_data);

	return status;
}

/**
 * aux_write() - Issue AUX write request
 * @dev:            The LogiCore DP TX device in question
 * @dpcd_address:   The DPCD address to write to
 * @bytes_to_write: Number of bytes to write
 * @write_data:     Buffer containig data to be written
 *
 * Issue a write request over the AUX channel that will write to
 * the RX device's DisplayPort Configuration data (DPCD) address space. The
 * write message will be divided into multiple transactions which write a
 * maximum of 16 bytes each.
 *
 * Return: 0 if write operation was successful, -ve on error
 */
static int aux_write(struct udevice *dev, u32 dpcd_address, u32 bytes_to_write,
		     void *write_data)
{
	int status;

	if (!is_connected(dev))
		return -ENODEV;

	/* Send AUX write transaction. */
	status = aux_common(dev, AUX_CMD_WRITE, dpcd_address,
			    bytes_to_write, (u8 *)write_data);

	return status;
}

/* Core initialization */

/**
 * initialize() - Initialize a LogiCore DP TX device
 * @dev: The LogiCore DP TX device in question
 *
 * Return: Always 0
 */
static int initialize(struct udevice *dev)
{
	struct dp_tx *dp_tx = dev_get_priv(dev);
	u32 val;
	u32 phy_config;
	unsigned int k;

	/* place the PHY (and GTTXRESET) into reset. */
	phy_config = get_reg(dev, REG_PHY_CONFIG);
	set_reg(dev, REG_PHY_CONFIG, phy_config | PHY_CONFIG_GT_ALL_RESET_MASK);

	/* reset the video streams and AUX logic. */
	set_reg(dev, REG_SOFT_RESET,
		SOFT_RESET_VIDEO_STREAM_ALL_MASK |
		SOFT_RESET_AUX_MASK);

	/* disable the DisplayPort TX core. */
	set_reg(dev, REG_ENABLE, 0);

	/* set the clock divider. */
	val = get_reg(dev, REG_AUX_CLK_DIVIDER);
	val &= ~AUX_CLK_DIVIDER_VAL_MASK;
	val |= dp_tx->s_axi_clk / 1000000;
	set_reg(dev, REG_AUX_CLK_DIVIDER, val);

	/* set the DisplayPort TX core's clock speed. */
	set_reg(dev, REG_PHY_CLOCK_SELECT, PHY_CLOCK_SELECT_DEFAULT);

	/* bring the PHY (and GTTXRESET) out of reset. */
	set_reg(dev, REG_PHY_CONFIG,
		phy_config & ~PHY_CONFIG_GT_ALL_RESET_MASK);

	/* enable the DisplayPort TX core. */
	set_reg(dev, REG_ENABLE, 1);

	/* Unmask Hot-Plug-Detect (HPD) interrupts. */
	set_reg(dev, REG_INTERRUPT_MASK,
		~INTERRUPT_MASK_HPD_PULSE_DETECTED_MASK &
		~INTERRUPT_MASK_HPD_EVENT_MASK &
		~INTERRUPT_MASK_HPD_IRQ_MASK);

	for (k = 0; k < 4; k++) {
		/* Disable pre-cursor levels. */
		set_reg(dev, REG_PHY_PRECURSOR_LANE_0 + 4 * k, 0);

		/* Write default voltage swing levels to the TX registers. */
		set_reg(dev, REG_PHY_VOLTAGE_DIFF_LANE_0 + 4 * k, 0);

		/* Write default pre-emphasis levels to the TX registers. */
		set_reg(dev, REG_PHY_POSTCURSOR_LANE_0 + 4 * k, 0);
	}

	return 0;
}

/**
 * is_link_rate_valid() - Check if given link rate is valif for device
 * @dev:       The LogiCore DP TX device in question
 * @link_rate: The link rate to be checked for validity
 *
 * Return: true if he supplied link rate is valid, false otherwise
 */
static bool is_link_rate_valid(struct udevice *dev, u8 link_rate)
{
	struct dp_tx *dp_tx = dev_get_priv(dev);
	bool valid = true;

	if (link_rate != LINK_BW_SET_162GBPS &&
	    link_rate != LINK_BW_SET_270GBPS &&
	    link_rate != LINK_BW_SET_540GBPS)
		valid = false;
	else if (link_rate > dp_tx->link_config.max_link_rate)
		valid = false;

	return valid;
}

/**
 * is_lane_count_valid() - Check if given lane count is valif for device
 * @dev:        The LogiCore DP TX device in question
 * @lane_count: The lane count to be checked for validity
 *
 * Return: true if he supplied lane count is valid, false otherwise
 */
static bool is_lane_count_valid(struct udevice *dev, u8 lane_count)
{
	struct dp_tx *dp_tx = dev_get_priv(dev);
	bool valid = true;

	if (lane_count != LANE_COUNT_SET_1 &&
	    lane_count != LANE_COUNT_SET_2 &&
	    lane_count != LANE_COUNT_SET_4)
		valid = false;
	else if (lane_count > dp_tx->link_config.max_lane_count)
		valid = false;

	return valid;
}

/**
 * get_rx_capabilities() - Check if capabilities of RX device are valid for TX
 *			   device
 * @dev: The LogiCore DP TX device in question
 *
 * Return: 0 if the capabilities of the RX device are valid for the TX device,
 *	   -ve if not, of an error occurred during capability determination
 */
static int get_rx_capabilities(struct udevice *dev)
{
	struct dp_tx *dp_tx = dev_get_priv(dev);
	int status;
	u8 rx_max_link_rate;
	u8 rx_max_lane_count;

	if (!is_connected(dev))
		return -ENODEV;

	status = aux_read(dev, DPCD_RECEIVER_CAP_FIELD_START, 16,
			  dp_tx->dpcd_rx_caps);
	if (status)
		return -EIO;

	rx_max_link_rate = dp_tx->dpcd_rx_caps[DPCD_MAX_LINK_RATE];
	rx_max_lane_count = dp_tx->dpcd_rx_caps[DPCD_MAX_LANE_COUNT] &
			    DPCD_MAX_LANE_COUNT_MASK;

	dp_tx->link_config.max_link_rate =
		(rx_max_link_rate > dp_tx->max_link_rate) ?
		dp_tx->max_link_rate : rx_max_link_rate;
	if (!is_link_rate_valid(dev, rx_max_link_rate))
		return -EINVAL;

	dp_tx->link_config.max_lane_count =
		(rx_max_lane_count > dp_tx->max_lane_count) ?
		dp_tx->max_lane_count : rx_max_lane_count;
	if (!is_lane_count_valid(dev, rx_max_lane_count))
		return -EINVAL;

	dp_tx->link_config.support_enhanced_framing_mode =
		dp_tx->dpcd_rx_caps[DPCD_MAX_LANE_COUNT] &
		DPCD_ENHANCED_FRAME_SUPPORT_MASK;

	return 0;
}

/**
 * enable_main_link() - Switch on main link for a device
 * @dev: The LogiCore DP TX device in question
 */
static void enable_main_link(struct udevice *dev)
{
	/* reset the scrambler. */
	set_reg(dev, REG_FORCE_SCRAMBLER_RESET, 0x1);

	/* enable the main stream. */
	set_reg(dev, REG_ENABLE_MAIN_STREAM, 0x1);
}

/**
 * disable_main_link() - Switch off main link for a device
 * @dev: The LogiCore DP TX device in question
 */
static void disable_main_link(struct udevice *dev)
{
	/* reset the scrambler. */
	set_reg(dev, REG_FORCE_SCRAMBLER_RESET, 0x1);

	/* Disable the main stream. */
	set_reg(dev, REG_ENABLE_MAIN_STREAM, 0x0);
}

/**
 * reset_dp_phy() - Reset a device
 * @dev:   The LogiCore DP TX device in question
 * @reset: Bit mask determining which bits in the device's config register
 *	   should be set for the reset
 */
static void reset_dp_phy(struct udevice *dev, u32 reset)
{
	struct dp_tx *dp_tx = dev_get_priv(dev);
	u32 val;

	set_reg(dev, REG_ENABLE, 0x0);

	val = get_reg(dev, REG_PHY_CONFIG);

	/* Apply reset. */
	set_reg(dev, REG_PHY_CONFIG, val | reset);

	/* Remove reset. */
	set_reg(dev, REG_PHY_CONFIG, val);

	/* Wait for the PHY to be ready. */
	wait_phy_ready(dev, phy_status_lanes_ready_mask(dp_tx->max_lane_count));

	set_reg(dev, REG_ENABLE, 0x1);
}

/**
 * set_enhanced_frame_mode() - Enable/Disable enhanced frame mode
 * @dev:    The LogiCore DP TX device in question
 * @enable: Flag to determine whether to enable (1) or disable (0) the enhanced
 *	    frame mode
 *
 * Enable or disable the enhanced framing symbol sequence for
 * both the DisplayPort TX core and the RX device.
 *
 * Return: 0 if enabling/disabling the enhanced frame mode was successful, -ve
 *	   on error
 */
static int set_enhanced_frame_mode(struct udevice *dev, u8 enable)
{
	struct dp_tx *dp_tx = dev_get_priv(dev);
	int status;
	u8 val;

	if (!is_connected(dev))
		return -ENODEV;

	if (dp_tx->link_config.support_enhanced_framing_mode)
		dp_tx->link_config.enhanced_framing_mode = enable;
	else
		dp_tx->link_config.enhanced_framing_mode = false;

	/* Write enhanced frame mode enable to the DisplayPort TX core. */
	set_reg(dev, REG_ENHANCED_FRAME_EN,
		dp_tx->link_config.enhanced_framing_mode);

	/* Write enhanced frame mode enable to the RX device. */
	status = aux_read(dev, DPCD_LANE_COUNT_SET, 0x1, &val);
	if (status)
		return -EIO;

	if (dp_tx->link_config.enhanced_framing_mode)
		val |= DPCD_ENHANCED_FRAME_EN_MASK;
	else
		val &= ~DPCD_ENHANCED_FRAME_EN_MASK;

	status = aux_write(dev, DPCD_LANE_COUNT_SET, 0x1, &val);
	if (status)
		return -EIO;

	return 0;
}

/**
 * set_lane_count() - Set the lane count
 * @dev:        The LogiCore DP TX device in question
 * @lane_count: Lane count to set
 *
 * Set the number of lanes to be used by the main link for both
 * the DisplayPort TX core and the RX device.
 *
 * Return: 0 if setting the lane count was successful, -ve on error
 */
static int set_lane_count(struct udevice *dev, u8 lane_count)
{
	struct dp_tx *dp_tx = dev_get_priv(dev);
	int status;
	u8 val;

	if (!is_connected(dev))
		return -ENODEV;

	printf("       set lane count to %u\n", lane_count);

	dp_tx->link_config.lane_count = lane_count;

	/* Write the new lane count to the DisplayPort TX core. */
	set_reg(dev, REG_LANE_COUNT_SET, dp_tx->link_config.lane_count);

	/* Write the new lane count to the RX device. */
	status = aux_read(dev, DPCD_LANE_COUNT_SET, 0x1, &val);
	if (status)
		return -EIO;
	val &= ~DPCD_LANE_COUNT_SET_MASK;
	val |= dp_tx->link_config.lane_count;

	status = aux_write(dev, DPCD_LANE_COUNT_SET, 0x1, &val);
	if (status)
		return -EIO;

	return 0;
}

/**
 * set_clk_speed() - Set DP phy clock speed
 * @dev:   The LogiCore DP TX device in question
 * @speed: The clock frquency to set (one of PHY_CLOCK_SELECT_*)
 *
 * Set the clock frequency for the DisplayPort PHY corresponding to a desired
 * data rate.
 *
 * Return: 0 if setting the DP phy clock speed was successful, -ve on error
 */
static int set_clk_speed(struct udevice *dev, u32 speed)
{
	struct dp_tx *dp_tx = dev_get_priv(dev);
	int status;
	u32 val;
	u32 mask;

	/* Disable the DisplayPort TX core first. */
	val = get_reg(dev, REG_ENABLE);
	set_reg(dev, REG_ENABLE, 0x0);

	/* Change speed of the feedback clock. */
	set_reg(dev, REG_PHY_CLOCK_SELECT, speed);

	/* Re-enable the DisplayPort TX core if it was previously enabled. */
	if (val)
		set_reg(dev, REG_ENABLE, 0x1);

	/* Wait until the PHY is ready. */
	mask = phy_status_lanes_ready_mask(dp_tx->max_lane_count);
	status = wait_phy_ready(dev, mask);
	if (status)
		return -EIO;

	return 0;
}

/**
 * set_link_rate() - Set the link rate
 * @dev:       The LogiCore DP TX device in question
 * @link_rate: The link rate to set (one of LINK_BW_SET_*)
 *
 * Set the data rate to be used by the main link for both the DisplayPort TX
 * core and the RX device.
 *
 * Return: 0 if setting the link rate was successful, -ve on error
 */
static int set_link_rate(struct udevice *dev, u8 link_rate)
{
	struct dp_tx *dp_tx = dev_get_priv(dev);
	int status;

	/* Write a corresponding clock frequency to the DisplayPort TX core. */
	switch (link_rate) {
	case LINK_BW_SET_162GBPS:
		printf("       set link rate to 1.62 Gb/s\n");
		status = set_clk_speed(dev, PHY_CLOCK_SELECT_162GBPS);
		break;
	case LINK_BW_SET_270GBPS:
		printf("       set link rate to 2.70 Gb/s\n");
		status = set_clk_speed(dev, PHY_CLOCK_SELECT_270GBPS);
		break;
	case LINK_BW_SET_540GBPS:
		printf("       set link rate to 5.40 Gb/s\n");
		status = set_clk_speed(dev, PHY_CLOCK_SELECT_540GBPS);
		break;
	default:
		return -EINVAL;
	}
	if (status)
		return -EIO;

	dp_tx->link_config.link_rate = link_rate;

	/* Write new link rate to the DisplayPort TX core. */
	set_reg(dev, REG_LINK_BW_SET, dp_tx->link_config.link_rate);

	/* Write new link rate to the RX device. */
	status = aux_write(dev, DPCD_LINK_BW_SET, 1,
			   &dp_tx->link_config.link_rate);
	if (status)
		return -EIO;

	return 0;
}

/* Link training */

/**
 * get_training_delay() - Get training delay
 * @dev:            The LogiCore DP TX device in question
 * @training_state: The training state for which the required training delay
 *		    should be queried
 *
 * Determine what the RX device's required training delay is for
 * link training.
 *
 * Return: The training delay in us
 */
static int get_training_delay(struct udevice *dev, int training_state)
{
	struct dp_tx *dp_tx = dev_get_priv(dev);
	u16 delay;

	switch (dp_tx->dpcd_rx_caps[DPCD_TRAIN_AUX_RD_INTERVAL]) {
	case DPCD_TRAIN_AUX_RD_INT_100_400US:
		if (training_state == TS_CLOCK_RECOVERY)
			/* delay for the clock recovery phase. */
			delay = 100;
		else
			/* delay for the channel equalization phase. */
			delay = 400;
		break;
	case DPCD_TRAIN_AUX_RD_INT_4MS:
		delay = 4000;
		break;
	case DPCD_TRAIN_AUX_RD_INT_8MS:
		delay = 8000;
		break;
	case DPCD_TRAIN_AUX_RD_INT_12MS:
		delay = 12000;
		break;
	case DPCD_TRAIN_AUX_RD_INT_16MS:
		delay = 16000;
		break;
	default:
		/* Default to 20 ms. */
		delay = 20000;
		break;
	}

	return delay;
}

/**
 * set_vswing_preemp() - Build AUX data to set voltage swing and pre-emphasis
 * @dev:      The LogiCore DP TX device in question
 * @aux_data: Buffer to receive the built AUX data
 *
 * Build AUX data to set current voltage swing and pre-emphasis level settings;
 * the necessary data is taken from the link_config structure.
 */
static void set_vswing_preemp(struct udevice *dev, u8 *aux_data)
{
	struct dp_tx *dp_tx = dev_get_priv(dev);
	u8 data;
	u8 vs_level_rx = dp_tx->link_config.vs_level;
	u8 pe_level_rx = dp_tx->link_config.pe_level;

	/* Set up the data buffer for writing to the RX device. */
	data = (pe_level_rx << DPCD_TRAINING_LANEX_SET_PE_SHIFT) | vs_level_rx;
	/* The maximum voltage swing has been reached. */
	if (vs_level_rx == MAXIMUM_VS_LEVEL)
		data |= DPCD_TRAINING_LANEX_SET_MAX_VS_MASK;

	/* The maximum pre-emphasis level has been reached. */
	if (pe_level_rx == MAXIMUM_PE_LEVEL)
		data |= DPCD_TRAINING_LANEX_SET_MAX_PE_MASK;
	memset(aux_data, data, 4);
}

/**
 * adj_vswing_preemp() - Adjust voltage swing and pre-emphasis
 * @dev: The LogiCore DP TX device in question
 *
 * Set new voltage swing and pre-emphasis levels using the
 * adjustment requests obtained from the RX device.
 *
 * Return: 0 if voltage swing and pre-emphasis could be adjusted successfully,
 *	   -ve on error
 */
static int adj_vswing_preemp(struct udevice *dev)
{
	struct dp_tx *dp_tx = dev_get_priv(dev);
	int status;
	u8 index;
	u8 vs_level_adj_req[4];
	u8 pe_level_adj_req[4];
	u8 aux_data[4];
	u8 *ajd_reqs = &dp_tx->lane_status_ajd_reqs[4];

	/*
	 * Analyze the adjustment requests for changes in voltage swing and
	 * pre-emphasis levels.
	 */
	vs_level_adj_req[0] = ajd_reqs[0] & DPCD_ADJ_REQ_LANE_0_2_VS_MASK;
	vs_level_adj_req[1] = (ajd_reqs[0] & DPCD_ADJ_REQ_LANE_1_3_VS_MASK) >>
			      DPCD_ADJ_REQ_LANE_1_3_VS_SHIFT;
	vs_level_adj_req[2] = ajd_reqs[1] & DPCD_ADJ_REQ_LANE_0_2_VS_MASK;
	vs_level_adj_req[3] = (ajd_reqs[1] & DPCD_ADJ_REQ_LANE_1_3_VS_MASK) >>
			      DPCD_ADJ_REQ_LANE_1_3_VS_SHIFT;
	pe_level_adj_req[0] = (ajd_reqs[0] & DPCD_ADJ_REQ_LANE_0_2_PE_MASK) >>
			      DPCD_ADJ_REQ_LANE_0_2_PE_SHIFT;
	pe_level_adj_req[1] = (ajd_reqs[0] & DPCD_ADJ_REQ_LANE_1_3_PE_MASK) >>
			      DPCD_ADJ_REQ_LANE_1_3_PE_SHIFT;
	pe_level_adj_req[2] = (ajd_reqs[1] & DPCD_ADJ_REQ_LANE_0_2_PE_MASK) >>
			      DPCD_ADJ_REQ_LANE_0_2_PE_SHIFT;
	pe_level_adj_req[3] = (ajd_reqs[1] & DPCD_ADJ_REQ_LANE_1_3_PE_MASK) >>
			      DPCD_ADJ_REQ_LANE_1_3_PE_SHIFT;

	/*
	 * Change the drive settings to match the adjustment requests. Use the
	 * greatest level requested.
	 */
	dp_tx->link_config.vs_level = 0;
	dp_tx->link_config.pe_level = 0;
	for (index = 0; index < dp_tx->link_config.lane_count; index++) {
		if (vs_level_adj_req[index] > dp_tx->link_config.vs_level)
			dp_tx->link_config.vs_level = vs_level_adj_req[index];
		if (pe_level_adj_req[index] > dp_tx->link_config.pe_level)
			dp_tx->link_config.pe_level = pe_level_adj_req[index];
	}

	/*
	 * Verify that the voltage swing and pre-emphasis combination is
	 * allowed. Some combinations will result in a differential peak-to-peak
	 * voltage that is outside the permissible range. See the VESA
	 * DisplayPort v1.2 Specification, section 3.1.5.2.
	 * The valid combinations are:
	 *      PE=0    PE=1    PE=2    PE=3
	 * VS=0 valid   valid   valid   valid
	 * VS=1 valid   valid   valid
	 * VS=2 valid   valid
	 * VS=3 valid
	 *
	 * NOTE:
	 * Xilinix dp_v3_1 driver seems to have an off by one error when
	 * limiting pe_level which is fixed here.
	 */
	if (dp_tx->link_config.pe_level > (3 - dp_tx->link_config.vs_level))
		dp_tx->link_config.pe_level = 3 - dp_tx->link_config.vs_level;

	/*
	 * Make the adjustments to both the DisplayPort TX core and the RX
	 * device.
	 */
	set_vswing_preemp(dev, aux_data);
	/*
	 * Write the voltage swing and pre-emphasis levels for each lane to the
	 * RX device.
	 */
	status = aux_write(dev, DPCD_TRAINING_LANE0_SET, 4, aux_data);
	if (status)
		return -EIO;

	return 0;
}

/**
 * get_lane_status_adj_reqs() - Read lane status and adjustment requests
 *				information from the device
 * @dev: The LogiCore DP TX device in question
 *
 * Do a burst AUX read from the RX device over the AUX channel. The contents of
 * the status registers will be stored for later use by check_clock_recovery,
 * check_channel_equalization, and adj_vswing_preemp.
 *
 * Return: 0 if the status information were read successfully, -ve on error
 */
static int get_lane_status_adj_reqs(struct udevice *dev)
{
	struct dp_tx *dp_tx = dev_get_priv(dev);
	int status;

	/*
	 * Read and store 4 bytes of lane status and 2 bytes of adjustment
	 * requests.
	 */
	status = aux_read(dev, DPCD_STATUS_LANE_0_1, 6,
			  dp_tx->lane_status_ajd_reqs);
	if (status)
		return -EIO;

	return 0;
}

/**
 * check_clock_recovery() - Check clock recovery success
 * @dev:        The LogiCore DP TX device in question
 * @lane_count: The number of lanes for which to check clock recovery success
 *
 * Check if the RX device's DisplayPort Configuration data (DPCD) indicates
 * that the clock recovery sequence during link training was successful - the
 * RX device's link clock and data recovery unit has realized and maintained
 * the frequency lock for all lanes currently in use.
 *
 * Return: 0 if clock recovery was successful on all lanes in question, -ve if
 *	   not
 */
static int check_clock_recovery(struct udevice *dev, u8 lane_count)
{
	struct dp_tx *dp_tx = dev_get_priv(dev);
	u8 *lane_status = dp_tx->lane_status_ajd_reqs;

	/* Check that all LANEx_CR_DONE bits are set. */
	switch (lane_count) {
	case LANE_COUNT_SET_4:
		if (!(lane_status[1] & DPCD_STATUS_LANE_3_CR_DONE_MASK))
			goto out_fail;
		if (!(lane_status[1] & DPCD_STATUS_LANE_2_CR_DONE_MASK))
			goto out_fail;
	/* Drop through and check lane 1. */
	case LANE_COUNT_SET_2:
		if (!(lane_status[0] & DPCD_STATUS_LANE_1_CR_DONE_MASK))
			goto out_fail;
	/* Drop through and check lane 0. */
	case LANE_COUNT_SET_1:
		if (!(lane_status[0] & DPCD_STATUS_LANE_0_CR_DONE_MASK))
			goto out_fail;
	default:
		/* All (lane_count) lanes have achieved clock recovery. */
		break;
	}

	return 0;

out_fail:
	return -EIO;
}

/**
 * check_channel_equalization() - Check channel equalization success
 * @dev:        The LogiCore DP TX device in question
 * @lane_count: The number of lanes for which to check channel equalization
 *		success
 *
 * Check if the RX device's DisplayPort Configuration data (DPCD) indicates
 * that the channel equalization sequence during link training was successful -
 * the RX device has achieved channel equalization, symbol lock, and interlane
 * alignment for all lanes currently in use.
 *
 * Return: 0 if channel equalization was successful on all lanes in question,
 *	   -ve if not
 */
static int check_channel_equalization(struct udevice *dev, u8 lane_count)
{
	struct dp_tx *dp_tx = dev_get_priv(dev);
	u8 *lane_status = dp_tx->lane_status_ajd_reqs;

	/* Check that all LANEx_CHANNEL_EQ_DONE bits are set. */
	switch (lane_count) {
	case LANE_COUNT_SET_4:
		if (!(lane_status[1] & DPCD_STATUS_LANE_3_CE_DONE_MASK))
			goto out_fail;
		if (!(lane_status[1] & DPCD_STATUS_LANE_2_CE_DONE_MASK))
			goto out_fail;
	/* Drop through and check lane 1. */
	case LANE_COUNT_SET_2:
		if (!(lane_status[0] & DPCD_STATUS_LANE_1_CE_DONE_MASK))
			goto out_fail;
	/* Drop through and check lane 0. */
	case LANE_COUNT_SET_1:
		if (!(lane_status[0] & DPCD_STATUS_LANE_0_CE_DONE_MASK))
			goto out_fail;
	default:
		/* All (lane_count) lanes have achieved channel equalization. */
		break;
	}

	/* Check that all LANEx_SYMBOL_LOCKED bits are set. */
	switch (lane_count) {
	case LANE_COUNT_SET_4:
		if (!(lane_status[1] & DPCD_STATUS_LANE_3_SL_DONE_MASK))
			goto out_fail;
		if (!(lane_status[1] & DPCD_STATUS_LANE_2_SL_DONE_MASK))
			goto out_fail;
	/* Drop through and check lane 1. */
	case LANE_COUNT_SET_2:
		if (!(lane_status[0] & DPCD_STATUS_LANE_1_SL_DONE_MASK))
			goto out_fail;
	/* Drop through and check lane 0. */
	case LANE_COUNT_SET_1:
		if (!(lane_status[0] & DPCD_STATUS_LANE_0_SL_DONE_MASK))
			goto out_fail;
	default:
		/* All (lane_count) lanes have achieved symbol lock. */
		break;
	}

	/* Check that interlane alignment is done. */
	if (!(lane_status[2] & DPCD_LANE_ALIGN_STATUS_UPDATED_IA_DONE_MASK))
		goto out_fail;

	return 0;

out_fail:
	return -EIO;
}

/**
 * set_training_pattern() - Set training pattern for link training
 * @dev:     The LogiCore DP TX device in question
 * @pattern: The training pattern to set
 *
 * Set the training pattern to be used during link training for both the
 * DisplayPort TX core and the RX device.
 *
 * Return: 0 if the training pattern could be set successfully, -ve if not
 */
static int set_training_pattern(struct udevice *dev, u32 pattern)
{
	struct dp_tx *dp_tx = dev_get_priv(dev);
	int status;
	u8 aux_data[5];

	/* Write to the DisplayPort TX core. */
	set_reg(dev, REG_TRAINING_PATTERN_SET, pattern);

	aux_data[0] = pattern;

	/* Write scrambler disable to the DisplayPort TX core. */
	switch (pattern) {
	case TRAINING_PATTERN_SET_OFF:
		set_reg(dev, REG_SCRAMBLING_DISABLE, 0);
		dp_tx->link_config.scrambler_en = 1;
		break;
	case TRAINING_PATTERN_SET_TP1:
	case TRAINING_PATTERN_SET_TP2:
	case TRAINING_PATTERN_SET_TP3:
		aux_data[0] |= DPCD_TP_SET_SCRAMB_DIS_MASK;
		set_reg(dev, REG_SCRAMBLING_DISABLE, 1);
		dp_tx->link_config.scrambler_en = 0;
		break;
	default:
		break;
	}

	/*
	 * Make the adjustments to both the DisplayPort TX core and the RX
	 * device.
	 */
	set_vswing_preemp(dev, &aux_data[1]);
	/*
	 * Write the voltage swing and pre-emphasis levels for each lane to the
	 * RX device.
	 */
	if  (pattern == TRAINING_PATTERN_SET_OFF)
		status = aux_write(dev, DPCD_TP_SET, 1, aux_data);
	else
		status = aux_write(dev, DPCD_TP_SET, 5, aux_data);
	if (status)
		return -EIO;

	return 0;
}

/**
 * training_state_clock_recovery() - Run clock recovery part of link training
 * @dev: The LogiCore DP TX device in question
 *
 * Run the clock recovery sequence as part of link training. The
 * sequence is as follows:
 *
 *	0) Start signaling at the minimum voltage swing, pre-emphasis, and
 *	   post- cursor levels.
 *	1) Transmit training pattern 1 over the main link with symbol
 *	   scrambling disabled.
 *	2) The clock recovery loop. If clock recovery is unsuccessful after
 *	   MaxIterations loop iterations, return.
 *	2a) Wait for at least the period of time specified in the RX device's
 *	    DisplayPort Configuration data (DPCD) register,
 *	    TRAINING_AUX_RD_INTERVAL.
 *	2b) Check if all lanes have achieved clock recovery lock. If so,
 *	    return.
 *	2c) Check if the same voltage swing level has been used 5 consecutive
 *	    times or if the maximum level has been reached. If so, return.
 *	2d) Adjust the voltage swing, pre-emphasis, and post-cursor levels as
 *	    requested by the RX device.
 *	2e) Loop back to 2a.
 *
 * For a more detailed description of the clock recovery sequence, see section
 * 3.5.1.2.1 of the DisplayPort 1.2a specification document.
 *
 * Return: The next state machine state to advance to
 */
static unsigned int training_state_clock_recovery(struct udevice *dev)
{
	struct dp_tx *dp_tx = dev_get_priv(dev);
	int status;
	u32 delay_us;
	u8 prev_vs_level = 0;
	u8 same_vs_level_count = 0;

	/*
	 * Obtain the required delay for clock recovery as specified by the
	 * RX device.
	 */
	delay_us = get_training_delay(dev, TS_CLOCK_RECOVERY);

	/* Start CRLock. */

	/* Transmit training pattern 1. */
	/* Disable the scrambler. */
	/* Start from minimal voltage swing and pre-emphasis levels. */
	dp_tx->link_config.vs_level = 0;
	dp_tx->link_config.pe_level = 0;
	status = set_training_pattern(dev, TRAINING_PATTERN_SET_TP1);
	if (status)
		return TS_FAILURE;

	while (1) {
		/* Wait delay specified in TRAINING_AUX_RD_INTERVAL. */
		udelay(delay_us);

		/* Get lane and adjustment requests. */
		status = get_lane_status_adj_reqs(dev);
		if (status)
			return TS_FAILURE;

		/*
		 * Check if all lanes have realized and maintained the frequency
		 * lock and get adjustment requests.
		 */
		status = check_clock_recovery(dev,
					      dp_tx->link_config.lane_count);
		if (!status)
			return TS_CHANNEL_EQUALIZATION;

		/*
		 * Check if the same voltage swing for each lane has been used 5
		 * consecutive times.
		 */
		if (prev_vs_level == dp_tx->link_config.vs_level) {
			same_vs_level_count++;
		} else {
			same_vs_level_count = 0;
			prev_vs_level = dp_tx->link_config.vs_level;
		}
		if (same_vs_level_count >= 5)
			break;

		/* Only try maximum voltage swing once. */
		if (dp_tx->link_config.vs_level == MAXIMUM_VS_LEVEL)
			break;

		/* Adjust the drive settings as requested by the RX device. */
		status = adj_vswing_preemp(dev);
		if (status)
			/* The AUX write failed. */
			return TS_FAILURE;
	}

	return TS_ADJUST_LINK_RATE;
}

/**
 * training_state_channel_equalization() - Run channel equalization part of
 *					   link training
 * @dev: The LogiCore DP TX device in question
 *
 * Run the channel equalization sequence as part of link
 * training. The sequence is as follows:
 *
 *	0) Start signaling with the same drive settings used at the end of the
 *	   clock recovery sequence.
 *	1) Transmit training pattern 2 (or 3) over the main link with symbol
 *	   scrambling disabled.
 *	2) The channel equalization loop. If channel equalization is
 *	   unsuccessful after 5 loop iterations, return.
 *	2a) Wait for at least the period of time specified in the RX device's
 *	    DisplayPort Configuration data (DPCD) register,
 *	    TRAINING_AUX_RD_INTERVAL.
 *	2b) Check if all lanes have achieved channel equalization, symbol lock,
 *	    and interlane alignment. If so, return.
 *	2c) Check if the same voltage swing level has been used 5 consecutive
 *	    times or if the maximum level has been reached. If so, return.
 *	2d) Adjust the voltage swing, pre-emphasis, and post-cursor levels as
 *	    requested by the RX device.
 *	2e) Loop back to 2a.
 *
 * For a more detailed description of the channel equalization sequence, see
 * section 3.5.1.2.2 of the DisplayPort 1.2a specification document.
 *
 * Return: The next state machine state to advance to
 */
static int training_state_channel_equalization(struct udevice *dev)
{
	struct dp_tx *dp_tx = dev_get_priv(dev);
	int status;
	u32 delay_us;
	u32 iteration_count = 0;

	/*
	 * Obtain the required delay for channel equalization as specified by
	 * the RX device.
	 */
	delay_us = get_training_delay(dev, TS_CHANNEL_EQUALIZATION);

	/* Start channel equalization. */

	/* Write the current drive settings. */
	/* Transmit training pattern 2/3. */
	if (dp_tx->dpcd_rx_caps[DPCD_MAX_LANE_COUNT] & DPCD_TPS3_SUPPORT_MASK)
		status = set_training_pattern(dev, TRAINING_PATTERN_SET_TP3);
	else
		status = set_training_pattern(dev, TRAINING_PATTERN_SET_TP2);

	if (status)
		return TS_FAILURE;

	while (iteration_count < 5) {
		/* Wait delay specified in TRAINING_AUX_RD_INTERVAL. */
		udelay(delay_us);

		/* Get lane and adjustment requests. */
		status = get_lane_status_adj_reqs(dev);
		if (status)
			/* The AUX read failed. */
			return TS_FAILURE;

		/* Check that all lanes still have their clocks locked. */
		status = check_clock_recovery(dev,
					      dp_tx->link_config.lane_count);
		if (status)
			break;

		/*
		 * Check if all lanes have accomplished channel equalization,
		 * symbol lock, and interlane alignment.
		 */
		status =
		    check_channel_equalization(dev,
					       dp_tx->link_config.lane_count);
		if (!status)
			return TS_SUCCESS;

		/* Adjust the drive settings as requested by the RX device. */
		status = adj_vswing_preemp(dev);
		if (status)
			/* The AUX write failed. */
			return TS_FAILURE;

		iteration_count++;
	}

	/*
	 * Tried 5 times with no success. Try a reduced bitrate first, then
	 * reduce the number of lanes.
	 */
	return TS_ADJUST_LINK_RATE;
}

/**
 * training_state_adjust_link_rate() - Downshift data rate and/or lane count
 * @dev: The LogiCore DP TX device in question
 *
 * This function is reached if either the clock recovery or the channel
 * equalization process failed during training. As a result, the data rate will
 * be downshifted, and training will be re-attempted (starting with clock
 * recovery) at the reduced data rate. If the data rate is already at 1.62
 * Gbps, a downshift in lane count will be attempted.
 *
 * Return: The next state machine state to advance to
 */
static int training_state_adjust_link_rate(struct udevice *dev)
{
	struct dp_tx *dp_tx = dev_get_priv(dev);
	int status;

	switch (dp_tx->link_config.link_rate) {
	case LINK_BW_SET_540GBPS:
		status = set_link_rate(dev, LINK_BW_SET_270GBPS);
		if (status) {
			status = TS_FAILURE;
			break;
		}
		status = TS_CLOCK_RECOVERY;
		break;
	case LINK_BW_SET_270GBPS:
		status = set_link_rate(dev, LINK_BW_SET_162GBPS);
		if (status) {
			status = TS_FAILURE;
			break;
		}
		status = TS_CLOCK_RECOVERY;
		break;
	default:
		/*
		 * Already at the lowest link rate. Try reducing the lane
		 * count next.
		 */
		status = TS_ADJUST_LANE_COUNT;
		break;
	}

	return status;
}

/**
 * trainig_state_adjust_lane_count - Downshift lane count
 * @dev: The LogiCore DP TX device in question
 *
 * This function is reached if either the clock recovery or the channel
 * equalization process failed during training, and a minimal data rate of 1.62
 * Gbps was being used. As a result, the number of lanes in use will be
 * reduced, and training will be re-attempted (starting with clock recovery) at
 * this lower lane count.
 *
 * Return: The next state machine state to advance to
 */
static int trainig_state_adjust_lane_count(struct udevice *dev)
{
	struct dp_tx *dp_tx = dev_get_priv(dev);
	int status;

	switch (dp_tx->link_config.lane_count) {
	case LANE_COUNT_SET_4:
		status = set_lane_count(dev, LANE_COUNT_SET_2);
		if (status) {
			status = TS_FAILURE;
			break;
		}

		status = set_link_rate(dev, dp_tx->link_config.max_link_rate);
		if (status) {
			status = TS_FAILURE;
			break;
		}
		status = TS_CLOCK_RECOVERY;
		break;
	case LANE_COUNT_SET_2:
		status = set_lane_count(dev, LANE_COUNT_SET_1);
		if (status) {
			status = TS_FAILURE;
			break;
		}

		status = set_link_rate(dev, dp_tx->link_config.max_link_rate);
		if (status) {
			status = TS_FAILURE;
			break;
		}
		status = TS_CLOCK_RECOVERY;
		break;
	default:
		/*
		 * Already at the lowest lane count. Training has failed at the
		 * lowest lane count and link rate.
		 */
		status = TS_FAILURE;
		break;
	}

	return status;
}

/**
 * check_link_status() - Check status of link
 * @dev:        The LogiCore DP TX device in question
 * @lane_count: The lane count to use for the check
 *
 * Check if the receiver's DisplayPort Configuration data (DPCD) indicates the
 * receiver has achieved and maintained clock recovery, channel equalization,
 * symbol lock, and interlane alignment for all lanes currently in use.
 *
 * Return: 0 if the link status is OK, -ve if a error occurred during checking
 */
static int check_link_status(struct udevice *dev, u8 lane_count)
{
	u8 retry_count = 0;

	if (!is_connected(dev))
		return -ENODEV;

	/* Retrieve AUX info. */
	do {
		int status;

		/* Get lane and adjustment requests. */
		status = get_lane_status_adj_reqs(dev);
		if (status)
			return -EIO;

		/* Check if the link needs training. */
		if ((check_clock_recovery(dev, lane_count) == 0) &&
		    (check_channel_equalization(dev, lane_count) == 0))
			return 0;

		retry_count++;
	} while (retry_count < 5); /* Retry up to 5 times. */

	return -EIO;
}

/**
 * run_training() - Run link training
 * @dev: The LogiCore DP TX device in question
 *
 * Run the link training process. It is implemented as a state machine, with
 * each state returning the next state. First, the clock recovery sequence will
 * be run; if successful, the channel equalization sequence will run. If either
 * the clock recovery or channel equalization sequence failed, the link rate or
 * the number of lanes used will be reduced and training will be re-attempted.
 * If training fails at the minimal data rate, 1.62 Gbps with a single lane,
 * training will no longer re-attempt and fail.
 *
 * ### Here be dragons ###
 * There are undocumented timeout constraints in the link training process. In
 * DP v1.2a spec, Chapter 3.5.1.2.2 a 10ms limit for the complete training
 * process is mentioned. Which individual timeouts are derived and implemented
 * by sink manufacturers is unknown. So each step should be as short as
 * possible and link training should start as soon as possible after HPD.
 *
 * Return: 0 if the training sequence ran successfully, -ve if a error occurred
 *	   or the training failed
 */
static int run_training(struct udevice *dev)
{
	struct dp_tx *dp_tx = dev_get_priv(dev);
	int status;
	int training_state = TS_CLOCK_RECOVERY;

	while (1) {
		switch (training_state) {
		case TS_CLOCK_RECOVERY:
			training_state =
				training_state_clock_recovery(dev);
			break;
		case TS_CHANNEL_EQUALIZATION:
			training_state =
				training_state_channel_equalization(dev);
			break;
		case TS_ADJUST_LINK_RATE:
			training_state =
				training_state_adjust_link_rate(dev);
			break;
		case TS_ADJUST_LANE_COUNT:
			training_state =
				trainig_state_adjust_lane_count(dev);
			break;
		default:
			break;
		}

		if (training_state == TS_SUCCESS)
			break;
		else if (training_state == TS_FAILURE)
			return -EIO;

		if (training_state == TS_ADJUST_LINK_RATE ||
		    training_state == TS_ADJUST_LANE_COUNT) {
			if (!dp_tx->train_adaptive)
				return -EIO;

			status = set_training_pattern(dev,
						      TRAINING_PATTERN_SET_OFF);
			if (status)
				return -EIO;
		}
	}

	/* Final status check. */
	status = check_link_status(dev, dp_tx->link_config.lane_count);
	if (status)
		return -EIO;

	return 0;
}

/* Link policy maker */

/**
 * cfg_main_link_max() - Determine best common capabilities
 * @dev: The LogiCore DP TX device in question
 *
 * Determine the common capabilities between the DisplayPort TX core and the RX
 * device.
 *
 * Return: 0 if the determination succeeded, -ve on error
 */
static int cfg_main_link_max(struct udevice *dev)
{
	struct dp_tx *dp_tx = dev_get_priv(dev);
	int status;

	if (!is_connected(dev))
		return -ENODEV;

	/*
	 * Configure the main link to the maximum common link rate between the
	 * DisplayPort TX core and the RX device.
	 */
	status = set_link_rate(dev, dp_tx->link_config.max_link_rate);
	if (status)
		return status;

	/*
	 * Configure the main link to the maximum common lane count between the
	 * DisplayPort TX core and the RX device.
	 */
	status = set_lane_count(dev, dp_tx->link_config.max_lane_count);
	if (status)
		return status;

	return 0;
}

/**
 * establish_link() - Establish a link
 * @dev: The LogiCore DP TX device in question
 *
 * Check if the link needs training and run the training sequence if training
 * is required.
 *
 * Return: 0 if the link was established successfully, -ve on error
 */
static int establish_link(struct udevice *dev)
{
	struct dp_tx *dp_tx = dev_get_priv(dev);
	int status;
	int status2;
	u32 mask;

	reset_dp_phy(dev, PHY_CONFIG_PHY_RESET_MASK);

	/* Disable main link during training. */
	disable_main_link(dev);

	/* Wait for the PHY to be ready. */
	mask = phy_status_lanes_ready_mask(dp_tx->max_lane_count);
	status = wait_phy_ready(dev, mask);
	if (status)
		return -EIO;

	/* Train main link. */
	status = run_training(dev);

	/* Turn off the training pattern and enable scrambler. */
	status2 = set_training_pattern(dev, TRAINING_PATTERN_SET_OFF);
	if (status || status2)
		return -EIO;

	return 0;
}

/*
 * Stream policy maker
 */

/**
 * cfg_msa_recalculate() - Calculate MSA parameters
 * @dev: The LogiCore DP TX device in question
 *
 * Calculate the following Main Stream Attributes (MSA):
 * - Transfer unit size
 * - User pixel width
 * - Horizontal total clock
 * - Vertical total clock
 * - misc_0
 * - misc_1
 * - Data per lane
 * - Average number of bytes per transfer unit
 * - Number of initial wait cycles
 *
 * These values are derived from:
 * - Bits per color
 * - Horizontal resolution
 * - Vertical resolution
 * - Horizontal blank start
 * - Vertical blank start
 * - Pixel clock (in KHz)
 * - Horizontal sync polarity
 * - Vertical sync polarity
 * - Horizontal sync pulse width
 * - Vertical sync pulse width
 */
static void cfg_msa_recalculate(struct udevice *dev)
{
	struct dp_tx *dp_tx = dev_get_priv(dev);
	u32 video_bw;
	u32 link_bw;
	u32 words_per_line;
	u8 bits_per_pixel;
	struct main_stream_attributes *msa_config;
	struct link_config *link_config;

	msa_config = &dp_tx->main_stream_attributes;
	link_config = &dp_tx->link_config;

	/*
	 * Set the user pixel width to handle clocks that exceed the
	 * capabilities of the DisplayPort TX core.
	 */
	if (msa_config->override_user_pixel_width == 0) {
		if (msa_config->pixel_clock_hz > 300000000 &&
		    link_config->lane_count == LANE_COUNT_SET_4) {
			msa_config->user_pixel_width = 4;
		} /*
		   * Xilinx driver used 75 MHz as a limit here, 150 MHZ should
		   * be more sane
		   */
		else if ((msa_config->pixel_clock_hz > 150000000) &&
			 (link_config->lane_count != LANE_COUNT_SET_1)) {
			msa_config->user_pixel_width = 2;
		} else {
			msa_config->user_pixel_width = 1;
		}
	}

	/* Compute the rest of the MSA values. */
	msa_config->n_vid = 27 * 1000 * link_config->link_rate;

	/* Miscellaneous attributes. */
	if (msa_config->bits_per_color == 6)
		msa_config->misc_0 = MAIN_STREAMX_MISC0_BDC_6BPC;
	else if (msa_config->bits_per_color == 8)
		msa_config->misc_0 = MAIN_STREAMX_MISC0_BDC_8BPC;
	else if (msa_config->bits_per_color == 10)
		msa_config->misc_0 = MAIN_STREAMX_MISC0_BDC_10BPC;
	else if (msa_config->bits_per_color == 12)
		msa_config->misc_0 = MAIN_STREAMX_MISC0_BDC_12BPC;
	else if (msa_config->bits_per_color == 16)
		msa_config->misc_0 = MAIN_STREAMX_MISC0_BDC_16BPC;

	msa_config->misc_0 = (msa_config->misc_0 <<
			      MAIN_STREAMX_MISC0_BDC_SHIFT) |
			     (msa_config->y_cb_cr_colorimetry <<
			      MAIN_STREAMX_MISC0_YCBCR_COLORIMETRY_SHIFT) |
			     (msa_config->dynamic_range <<
			      MAIN_STREAMX_MISC0_DYNAMIC_RANGE_SHIFT) |
			     (msa_config->component_format <<
			      MAIN_STREAMX_MISC0_COMPONENT_FORMAT_SHIFT) |
			     (msa_config->synchronous_clock_mode);

	msa_config->misc_1 = 0;

	/*
	 * Determine the number of bits per pixel for the specified color
	 * component format.
	 */
	if (msa_config->component_format ==
	    MAIN_STREAMX_MISC0_COMPONENT_FORMAT_YCBCR422)
		/* YCbCr422 color component format. */
		bits_per_pixel = msa_config->bits_per_color * 2;
	else
		/* RGB or YCbCr 4:4:4 color component format. */
		bits_per_pixel = msa_config->bits_per_color * 3;

	/* Calculate the data per lane. */
	words_per_line = (msa_config->h_active * bits_per_pixel);
	if (words_per_line % 16)
		words_per_line += 16;
	words_per_line /= 16;

	msa_config->data_per_lane = words_per_line - link_config->lane_count;
	if (words_per_line % link_config->lane_count)
		msa_config->data_per_lane += (words_per_line %
					      link_config->lane_count);

	/*
	 * Allocate a fixed size for single-stream transport (SST)
	 * operation.
	 */
	msa_config->transfer_unit_size = 64;

	/*
	 * Calculate the average number of bytes per transfer unit.
	 * Note: Both the integer and the fractional part is stored in
	 * avg_bytes_per_tu.
	 */
	video_bw = ((msa_config->pixel_clock_hz / 1000) * bits_per_pixel) / 8;
	link_bw = (link_config->lane_count * link_config->link_rate * 27);
	msa_config->avg_bytes_per_tu = (video_bw *
					msa_config->transfer_unit_size) /
					link_bw;

	/*
	 * The number of initial wait cycles at the start of a new line
	 * by the framing logic. This allows enough data to be buffered
	 * in the input FIFO before video is sent.
	 */
	if ((msa_config->avg_bytes_per_tu / 1000) <= 4)
		msa_config->init_wait = 64;
	else
		msa_config->init_wait = msa_config->transfer_unit_size -
					(msa_config->avg_bytes_per_tu / 1000);
}

/**
 * set_line_reset() - Enable/Disable end-of-line-reset
 * @dev: The LogiCore DP TX device in question
 *
 * Disable/enable the end-of-line-reset to the internal video pipe in case of
 * reduced blanking as required.
 */
static void set_line_reset(struct udevice *dev)
{
	struct dp_tx *dp_tx = dev_get_priv(dev);
	u32 reg_val;
	u16 h_blank;
	u16 h_reduced_blank;
	struct main_stream_attributes *msa_config =
		&dp_tx->main_stream_attributes;

	h_blank = msa_config->h_total - msa_config->h_active;
	/* Reduced blanking starts at ceil(0.2 * HTotal). */
	h_reduced_blank = 2 * msa_config->h_total;
	if (h_reduced_blank % 10)
		h_reduced_blank += 10;
	h_reduced_blank /= 10;

	/* CVT spec. states h_blank is either 80 or 160 for reduced blanking. */
	reg_val = get_reg(dev, REG_LINE_RESET_DISABLE);
	if (h_blank < h_reduced_blank &&
	    (h_blank == 80 || h_blank == 160)) {
		reg_val |= LINE_RESET_DISABLE_MASK;
	} else {
		reg_val &= ~LINE_RESET_DISABLE_MASK;
	}
	set_reg(dev, REG_LINE_RESET_DISABLE, reg_val);
}

/**
 * clear_msa_values() - Clear MSA values
 * @dev: The LogiCore DP TX device in question
 *
 * Clear the main stream attributes registers of the DisplayPort TX core.
 */
static void clear_msa_values(struct udevice *dev)
{
	set_reg(dev, REG_MAIN_STREAM_HTOTAL, 0);
	set_reg(dev, REG_MAIN_STREAM_VTOTAL, 0);
	set_reg(dev, REG_MAIN_STREAM_POLARITY, 0);
	set_reg(dev, REG_MAIN_STREAM_HSWIDTH, 0);
	set_reg(dev, REG_MAIN_STREAM_VSWIDTH, 0);
	set_reg(dev, REG_MAIN_STREAM_HRES, 0);
	set_reg(dev, REG_MAIN_STREAM_VRES, 0);
	set_reg(dev, REG_MAIN_STREAM_HSTART, 0);
	set_reg(dev, REG_MAIN_STREAM_VSTART, 0);
	set_reg(dev, REG_MAIN_STREAM_MISC0, 0);
	set_reg(dev, REG_MAIN_STREAM_MISC1, 0);
	set_reg(dev, REG_USER_PIXEL_WIDTH, 0);
	set_reg(dev, REG_USER_DATA_COUNT_PER_LANE, 0);
	set_reg(dev, REG_M_VID, 0);
	set_reg(dev, REG_N_VID, 0);

	set_reg(dev, REG_STREAM1, 0);
	set_reg(dev, REG_TU_SIZE, 0);
	set_reg(dev, REG_MIN_BYTES_PER_TU, 0);
	set_reg(dev, REG_FRAC_BYTES_PER_TU, 0);
	set_reg(dev, REG_INIT_WAIT, 0);
}

/**
 * set_msa_values() - Set MSA values
 * @dev: The LogiCore DP TX device in question
 *
 * Set the main stream attributes registers of the DisplayPort TX
 * core with the values specified in the main stream attributes configuration
 * structure.
 */
static void set_msa_values(struct udevice *dev)
{
	struct dp_tx *dp_tx = dev_get_priv(dev);
	struct main_stream_attributes *msa_config =
		&dp_tx->main_stream_attributes;

	printf("       set MSA %u x %u\n", msa_config->h_active,
	       msa_config->v_active);

	set_reg(dev, REG_MAIN_STREAM_HTOTAL, msa_config->h_total);
	set_reg(dev, REG_MAIN_STREAM_VTOTAL, msa_config->v_total);
	set_reg(dev, REG_MAIN_STREAM_POLARITY,
		msa_config->h_sync_polarity |
		(msa_config->v_sync_polarity <<
		 MAIN_STREAMX_POLARITY_VSYNC_POL_SHIFT));
	set_reg(dev, REG_MAIN_STREAM_HSWIDTH, msa_config->h_sync_width);
	set_reg(dev, REG_MAIN_STREAM_VSWIDTH, msa_config->v_sync_width);
	set_reg(dev, REG_MAIN_STREAM_HRES, msa_config->h_active);
	set_reg(dev, REG_MAIN_STREAM_VRES, msa_config->v_active);
	set_reg(dev, REG_MAIN_STREAM_HSTART, msa_config->h_start);
	set_reg(dev, REG_MAIN_STREAM_VSTART, msa_config->v_start);
	set_reg(dev, REG_MAIN_STREAM_MISC0, msa_config->misc_0);
	set_reg(dev, REG_MAIN_STREAM_MISC1, msa_config->misc_1);
	set_reg(dev, REG_USER_PIXEL_WIDTH, msa_config->user_pixel_width);

	set_reg(dev, REG_M_VID, msa_config->pixel_clock_hz / 1000);
	set_reg(dev, REG_N_VID, msa_config->n_vid);
	set_reg(dev, REG_USER_DATA_COUNT_PER_LANE, msa_config->data_per_lane);

	set_line_reset(dev);

	set_reg(dev, REG_TU_SIZE, msa_config->transfer_unit_size);
	set_reg(dev, REG_MIN_BYTES_PER_TU, msa_config->avg_bytes_per_tu / 1000);
	set_reg(dev, REG_FRAC_BYTES_PER_TU,
		(msa_config->avg_bytes_per_tu % 1000) * 1024 / 1000);
	set_reg(dev, REG_INIT_WAIT, msa_config->init_wait);
}

/*
 * external API
 */

/**
 * logicore_dp_tx_set_msa() - Set given MSA values on device
 * @dev: The LogiCore DP TX device in question
 * @msa: The MSA values to set for the device
 */
static void logicore_dp_tx_set_msa(struct udevice *dev,
				   struct logicore_dp_tx_msa *msa)
{
	struct dp_tx *dp_tx = dev_get_priv(dev);

	memset(&dp_tx->main_stream_attributes, 0,
	       sizeof(struct main_stream_attributes));

	dp_tx->main_stream_attributes.pixel_clock_hz = msa->pixel_clock_hz;
	dp_tx->main_stream_attributes.bits_per_color = msa->bits_per_color;
	dp_tx->main_stream_attributes.h_active = msa->h_active;
	dp_tx->main_stream_attributes.h_start = msa->h_start;
	dp_tx->main_stream_attributes.h_sync_polarity = msa->h_sync_polarity;
	dp_tx->main_stream_attributes.h_sync_width = msa->h_sync_width;
	dp_tx->main_stream_attributes.h_total = msa->h_total;
	dp_tx->main_stream_attributes.v_active = msa->v_active;
	dp_tx->main_stream_attributes.v_start = msa->v_start;
	dp_tx->main_stream_attributes.v_sync_polarity = msa->v_sync_polarity;
	dp_tx->main_stream_attributes.v_sync_width = msa->v_sync_width;
	dp_tx->main_stream_attributes.v_total = msa->v_total;
	dp_tx->main_stream_attributes.override_user_pixel_width =
		msa->override_user_pixel_width;
	dp_tx->main_stream_attributes.user_pixel_width = msa->user_pixel_width;
	dp_tx->main_stream_attributes.synchronous_clock_mode = 0;
}

/**
 * logicore_dp_tx_video_enable() - Enable video output
 * @dev: The LogiCore DP TX device in question
 * @msa: The MSA values to set for the device
 *
 * Return: 0 if the video was enabled successfully, -ve on error
 */
static int logicore_dp_tx_video_enable(struct udevice *dev,
				       struct logicore_dp_tx_msa *msa)
{
	struct dp_tx *dp_tx = dev_get_priv(dev);
	int res;
	u8 power = 0x01;

	if (!is_connected(dev)) {
		printf("       no DP sink connected\n");
		return -EIO;
	}

	initialize(dev);

	disable_main_link(dev);

	logicore_dp_tx_set_msa(dev, msa);

	get_rx_capabilities(dev);

	printf("       DP sink connected\n");
	aux_write(dev, DPCD_SET_POWER_DP_PWR_VOLTAGE, 1, &power);
	set_enhanced_frame_mode(dev, true);
	cfg_main_link_max(dev);
	res = establish_link(dev);
	printf("       establish_link: %s, vs: %d, pe: %d\n",
	       res ? "failed" : "ok", dp_tx->link_config.vs_level,
	       dp_tx->link_config.pe_level);

	cfg_msa_recalculate(dev);

	clear_msa_values(dev);
	set_msa_values(dev);

	enable_main_link(dev);

	return 0;
}

/*
 * Driver functions
 */

static int logicore_dp_tx_enable(struct udevice *dev, int panel_bpp,
				 const struct display_timing *timing)
{
	struct clk pixclock;
	struct logicore_dp_tx_msa *msa;
	struct logicore_dp_tx_msa mode_640_480_60 = {
		.pixel_clock_hz = 25175000,
		.bits_per_color = 8,
		.h_active = 640,
		.h_start = 144,
		.h_sync_polarity = false,
		.h_sync_width = 96,
		.h_total = 800,
		.v_active = 480,
		.v_start = 35,
		.v_sync_polarity = false,
		.v_sync_width = 2,
		.v_total = 525,
		.override_user_pixel_width = false,
		.user_pixel_width = 0,
	};

	struct logicore_dp_tx_msa mode_720_400_70 = {
		.pixel_clock_hz = 28300000,
		.bits_per_color = 8,
		.h_active = 720,
		.h_start = 162,
		.h_sync_polarity = false,
		.h_sync_width = 108,
		.h_total = 900,
		.v_active = 400,
		.v_start = 37,
		.v_sync_polarity = true,
		.v_sync_width = 2,
		.v_total = 449,
		.override_user_pixel_width = false,
		.user_pixel_width = 0,
	};

	struct logicore_dp_tx_msa mode_1024_768_60 = {
		.pixel_clock_hz = 65000000,
		.bits_per_color = 8,
		.h_active = 1024,
		.h_start = 296,
		.h_sync_polarity = false,
		.h_sync_width = 136,
		.h_total = 1344,
		.v_active = 768,
		.v_start = 35,
		.v_sync_polarity = false,
		.v_sync_width = 2,
		.v_total = 806,
		.override_user_pixel_width = false,
		.user_pixel_width = 0,
	};

	if (timing->hactive.typ == 1024 && timing->vactive.typ == 768)
		msa = &mode_1024_768_60;
	else if (timing->hactive.typ == 720 && timing->vactive.typ == 400)
		msa = &mode_720_400_70;
	else
		msa = &mode_640_480_60;

	if (clk_get_by_index(dev, 0, &pixclock)) {
		printf("%s: Could not get pixelclock\n", dev->name);
		return -1;
	}
	clk_set_rate(&pixclock, msa->pixel_clock_hz);

	return logicore_dp_tx_video_enable(dev, msa);
}

static int logicore_dp_tx_probe(struct udevice *dev)
{
	struct dp_tx *dp_tx = dev_get_priv(dev);

	dp_tx->s_axi_clk = S_AXI_CLK_DEFAULT;
	dp_tx->train_adaptive = false;
	dp_tx->max_link_rate = DPCD_MAX_LINK_RATE_540GBPS;
	dp_tx->max_lane_count = DPCD_MAX_LANE_COUNT_4;

	dp_tx->base = dev_read_u32_default(dev, "reg", -1);

	return 0;
}

static const struct dm_display_ops logicore_dp_tx_ops = {
	.enable = logicore_dp_tx_enable,
};

static const struct udevice_id logicore_dp_tx_ids[] = {
	{ .compatible = "gdsys,logicore_dp_tx" },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(logicore_dp_tx) = {
	.name			= "logicore_dp_tx",
	.id			= UCLASS_DISPLAY,
	.of_match		= logicore_dp_tx_ids,
	.probe			= logicore_dp_tx_probe,
	.priv_auto	= sizeof(struct dp_tx),
	.ops			= &logicore_dp_tx_ops,
};
