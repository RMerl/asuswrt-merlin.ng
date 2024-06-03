/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Texas Instruments System Control Interface Protocol
 * Based on include/linux/soc/ti/ti_sci_protocol.h from Linux.
 *
 * Copyright (C) 2018 Texas Instruments Incorporated - http://www.ti.com/
 *	Nishanth Menon
 *	Lokesh Vutla <lokeshvutla@ti.com>
 */

#ifndef __TISCI_PROTOCOL_H
#define __TISCI_PROTOCOL_H

/**
 * struct ti_sci_version_info - version information structure
 * @abi_major:	Major ABI version. Change here implies risk of backward
 *		compatibility break.
 * @abi_minor:	Minor ABI version. Change here implies new feature addition,
 *		or compatible change in ABI.
 * @firmware_revision:	Firmware revision (not usually used).
 * @firmware_description: Firmware description (not usually used).
 */
struct ti_sci_version_info {
	u8 abi_major;
	u8 abi_minor;
	u16 firmware_revision;
	char firmware_description[32];
};

struct ti_sci_handle;

/**
 * struct ti_sci_board_ops - Board config operations
 * @board_config: Command to set the board configuration
 *		  Returns 0 for successful exclusive request, else returns
 *		  corresponding error message.
 * @board_config_rm: Command to set the board resource management
 *		  configuration
 *		  Returns 0 for successful exclusive request, else returns
 *		  corresponding error message.
 * @board_config_security: Command to set the board security configuration
 *		  Returns 0 for successful exclusive request, else returns
 *		  corresponding error message.
 * @board_config_pm: Command to trigger and set the board power and clock
 *		  management related configuration
 *		  Returns 0 for successful exclusive request, else returns
 *		  corresponding error message.
 */
struct ti_sci_board_ops {
	int (*board_config)(const struct ti_sci_handle *handle,
			    u64 addr, u32 size);
	int (*board_config_rm)(const struct ti_sci_handle *handle,
			       u64 addr, u32 size);
	int (*board_config_security)(const struct ti_sci_handle *handle,
				     u64 addr, u32 size);
	int (*board_config_pm)(const struct ti_sci_handle *handle,
			       u64 addr, u32 size);
};

/**
 * struct ti_sci_dev_ops - Device control operations
 * @get_device: Command to request for device managed by TISCI
 *		Returns 0 for successful exclusive request, else returns
 *		corresponding error message.
 * @idle_device: Command to idle a device managed by TISCI
 *		Returns 0 for successful exclusive request, else returns
 *		corresponding error message.
 * @put_device:	Command to release a device managed by TISCI
 *		Returns 0 for successful release, else returns corresponding
 *		error message.
 * @is_valid:	Check if the device ID is a valid ID.
 *		Returns 0 if the ID is valid, else returns corresponding error.
 * @get_context_loss_count: Command to retrieve context loss counter - this
 *		increments every time the device looses context. Overflow
 *		is possible.
 *		- count: pointer to u32 which will retrieve counter
 *		Returns 0 for successful information request and count has
 *		proper data, else returns corresponding error message.
 * @is_idle:	Reports back about device idle state
 *		- req_state: Returns requested idle state
 *		Returns 0 for successful information request and req_state and
 *		current_state has proper data, else returns corresponding error
 *		message.
 * @is_stop:	Reports back about device stop state
 *		- req_state: Returns requested stop state
 *		- current_state: Returns current stop state
 *		Returns 0 for successful information request and req_state and
 *		current_state has proper data, else returns corresponding error
 *		message.
 * @is_on:	Reports back about device ON(or active) state
 *		- req_state: Returns requested ON state
 *		- current_state: Returns current ON state
 *		Returns 0 for successful information request and req_state and
 *		current_state has proper data, else returns corresponding error
 *		message.
 * @is_transitioning: Reports back if the device is in the middle of transition
 *		of state.
 *		-current_state: Returns 'true' if currently transitioning.
 * @set_device_resets: Command to configure resets for device managed by TISCI.
 *		-reset_state: Device specific reset bit field
 *		Returns 0 for successful request, else returns
 *		corresponding error message.
 * @get_device_resets: Command to read state of resets for device managed
 *		by TISCI.
 *		-reset_state: pointer to u32 which will retrieve resets
 *		Returns 0 for successful request, else returns
 *		corresponding error message.
 *
 * NOTE: for all these functions, the following parameters are generic in
 * nature:
 * -handle:	Pointer to TISCI handle as retrieved by *ti_sci_get_handle
 * -id:		Device Identifier
 *
 * Request for the device - NOTE: the client MUST maintain integrity of
 * usage count by balancing get_device with put_device. No refcounting is
 * managed by driver for that purpose.
 */
struct ti_sci_dev_ops {
	int (*get_device)(const struct ti_sci_handle *handle, u32 id);
	int (*idle_device)(const struct ti_sci_handle *handle, u32 id);
	int (*put_device)(const struct ti_sci_handle *handle, u32 id);
	int (*is_valid)(const struct ti_sci_handle *handle, u32 id);
	int (*get_context_loss_count)(const struct ti_sci_handle *handle,
				      u32 id, u32 *count);
	int (*is_idle)(const struct ti_sci_handle *handle, u32 id,
		       bool *requested_state);
	int (*is_stop)(const struct ti_sci_handle *handle, u32 id,
		       bool *req_state, bool *current_state);
	int (*is_on)(const struct ti_sci_handle *handle, u32 id,
		     bool *req_state, bool *current_state);
	int (*is_transitioning)(const struct ti_sci_handle *handle, u32 id,
				bool *current_state);
	int (*set_device_resets)(const struct ti_sci_handle *handle, u32 id,
				 u32 reset_state);
	int (*get_device_resets)(const struct ti_sci_handle *handle, u32 id,
				 u32 *reset_state);
};

/**
 * struct ti_sci_clk_ops - Clock control operations
 * @get_clock:	Request for activation of clock and manage by processor
 *		- needs_ssc: 'true' if Spread Spectrum clock is desired.
 *		- can_change_freq: 'true' if frequency change is desired.
 *		- enable_input_term: 'true' if input termination is desired.
 * @idle_clock:	Request for Idling a clock managed by processor
 * @put_clock:	Release the clock to be auto managed by TISCI
 * @is_auto:	Is the clock being auto managed
 *		- req_state: state indicating if the clock is auto managed
 * @is_on:	Is the clock ON
 *		- req_state: if the clock is requested to be forced ON
 *		- current_state: if the clock is currently ON
 * @is_off:	Is the clock OFF
 *		- req_state: if the clock is requested to be forced OFF
 *		- current_state: if the clock is currently Gated
 * @set_parent:	Set the clock source of a specific device clock
 *		- parent_id: Parent clock identifier to set.
 * @get_parent:	Get the current clock source of a specific device clock
 *		- parent_id: Parent clock identifier which is the parent.
 * @get_num_parents: Get the number of parents of the current clock source
 *		- num_parents: returns the number of parent clocks.
 * @get_best_match_freq: Find a best matching frequency for a frequency
 *		range.
 *		- match_freq: Best matching frequency in Hz.
 * @set_freq:	Set the Clock frequency
 * @get_freq:	Get the Clock frequency
 *		- current_freq: Frequency in Hz that the clock is at.
 *
 * NOTE: for all these functions, the following parameters are generic in
 * nature:
 * -handle:	Pointer to TISCI handle as retrieved by *ti_sci_get_handle
 * -did:	Device identifier this request is for
 * -cid:	Clock identifier for the device for this request.
 *		Each device has it's own set of clock inputs. This indexes
 *		which clock input to modify.
 * -min_freq:	The minimum allowable frequency in Hz. This is the minimum
 *		allowable programmed frequency and does not account for clock
 *		tolerances and jitter.
 * -target_freq: The target clock frequency in Hz. A frequency will be
 *		processed as close to this target frequency as possible.
 * -max_freq:	The maximum allowable frequency in Hz. This is the maximum
 *		allowable programmed frequency and does not account for clock
 *		tolerances and jitter.
 *
 * Request for the clock - NOTE: the client MUST maintain integrity of
 * usage count by balancing get_clock with put_clock. No refcounting is
 * managed by driver for that purpose.
 */
struct ti_sci_clk_ops {
	int (*get_clock)(const struct ti_sci_handle *handle, u32 did, u8 cid,
			 bool needs_ssc, bool can_change_freq,
			 bool enable_input_term);
	int (*idle_clock)(const struct ti_sci_handle *handle, u32 did, u8 cid);
	int (*put_clock)(const struct ti_sci_handle *handle, u32 did, u8 cid);
	int (*is_auto)(const struct ti_sci_handle *handle, u32 did, u8 cid,
		       bool *req_state);
	int (*is_on)(const struct ti_sci_handle *handle, u32 did, u8 cid,
		     bool *req_state, bool *current_state);
	int (*is_off)(const struct ti_sci_handle *handle, u32 did, u8 cid,
		      bool *req_state, bool *current_state);
	int (*set_parent)(const struct ti_sci_handle *handle, u32 did, u8 cid,
			  u8 parent_id);
	int (*get_parent)(const struct ti_sci_handle *handle, u32 did, u8 cid,
			  u8 *parent_id);
	int (*get_num_parents)(const struct ti_sci_handle *handle, u32 did,
			       u8 cid, u8 *num_parents);
	int (*get_best_match_freq)(const struct ti_sci_handle *handle, u32 did,
				   u8 cid, u64 min_freq, u64 target_freq,
				   u64 max_freq, u64 *match_freq);
	int (*set_freq)(const struct ti_sci_handle *handle, u32 did, u8 cid,
			u64 min_freq, u64 target_freq, u64 max_freq);
	int (*get_freq)(const struct ti_sci_handle *handle, u32 did, u8 cid,
			u64 *current_freq);
};

/**
 * struct ti_sci_rm_core_ops - Resource management core operations
 * @get_range:		Get a range of resources belonging to ti sci host.
 * @get_rage_from_shost:	Get a range of resources belonging to
 *				specified host id.
 *			- s_host: Host processing entity to which the
 *				  resources are allocated
 *
 * NOTE: for these functions, all the parameters are consolidated and defined
 * as below:
 * - handle:	Pointer to TISCI handle as retrieved by *ti_sci_get_handle
 * - dev_id:	TISCI device ID.
 * - subtype:	Resource assignment subtype that is being requested
 *		from the given device.
 * - range_start:	Start index of the resource range
 * - range_end:		Number of resources in the range
 */
struct ti_sci_rm_core_ops {
	int (*get_range)(const struct ti_sci_handle *handle, u32 dev_id,
			 u8 subtype, u16 *range_start, u16 *range_num);
	int (*get_range_from_shost)(const struct ti_sci_handle *handle,
				    u32 dev_id, u8 subtype, u8 s_host,
				    u16 *range_start, u16 *range_num);
};

/**
 * struct ti_sci_core_ops - SoC Core Operations
 * @reboot_device: Reboot the SoC
 *		Returns 0 for successful request(ideally should never return),
 *		else returns corresponding error value.
 * @query_msmc: Query the size of available msmc
 *		Return 0 for successful query else appropriate error value.
 */
struct ti_sci_core_ops {
	int (*reboot_device)(const struct ti_sci_handle *handle);
	int (*query_msmc)(const struct ti_sci_handle *handle,
			  u64 *msmc_start, u64 *msmc_end);
};

/**
 * struct ti_sci_proc_ops - Processor specific operations.
 *
 * @proc_request: Request for controlling a physical processor.
 *		The requesting host should be in the processor access list.
 * @proc_release: Relinquish a physical processor control
 * @proc_handover: Handover a physical processor control to another host
 *		   in the permitted list.
 * @set_proc_boot_cfg: Base configuration of the processor
 * @set_proc_boot_ctrl: Setup limited control flags in specific cases.
 * @proc_auth_boot_image:
 * @get_proc_boot_status: Get the state of physical processor
 *
 * NOTE: for all these functions, the following parameters are generic in
 * nature:
 * -handle:	Pointer to TISCI handle as retrieved by *ti_sci_get_handle
 * -pid:	Processor ID
 *
 */
struct ti_sci_proc_ops {
	int (*proc_request)(const struct ti_sci_handle *handle, u8 pid);
	int (*proc_release)(const struct ti_sci_handle *handle, u8 pid);
	int (*proc_handover)(const struct ti_sci_handle *handle, u8 pid,
			     u8 hid);
	int (*set_proc_boot_cfg)(const struct ti_sci_handle *handle, u8 pid,
				 u64 bv, u32 cfg_set, u32 cfg_clr);
	int (*set_proc_boot_ctrl)(const struct ti_sci_handle *handle, u8 pid,
				  u32 ctrl_set, u32 ctrl_clr);
	int (*proc_auth_boot_image)(const struct ti_sci_handle *handle,
				    u64 *image_addr, u32 *image_size);
	int (*get_proc_boot_status)(const struct ti_sci_handle *handle, u8 pid,
				    u64 *bv, u32 *cfg_flags, u32 *ctrl_flags,
				    u32 *sts_flags);
};

#define TI_SCI_RING_MODE_RING			(0)
#define TI_SCI_RING_MODE_MESSAGE		(1)
#define TI_SCI_RING_MODE_CREDENTIALS		(2)
#define TI_SCI_RING_MODE_QM			(3)

#define TI_SCI_MSG_UNUSED_SECONDARY_HOST TI_SCI_RM_NULL_U8

/* RA config.addr_lo parameter is valid for RM ring configure TI_SCI message */
#define TI_SCI_MSG_VALUE_RM_RING_ADDR_LO_VALID	BIT(0)
/* RA config.addr_hi parameter is valid for RM ring configure TI_SCI message */
#define TI_SCI_MSG_VALUE_RM_RING_ADDR_HI_VALID	BIT(1)
 /* RA config.count parameter is valid for RM ring configure TI_SCI message */
#define TI_SCI_MSG_VALUE_RM_RING_COUNT_VALID	BIT(2)
/* RA config.mode parameter is valid for RM ring configure TI_SCI message */
#define TI_SCI_MSG_VALUE_RM_RING_MODE_VALID	BIT(3)
/* RA config.size parameter is valid for RM ring configure TI_SCI message */
#define TI_SCI_MSG_VALUE_RM_RING_SIZE_VALID	BIT(4)
/* RA config.order_id parameter is valid for RM ring configure TISCI message */
#define TI_SCI_MSG_VALUE_RM_RING_ORDER_ID_VALID	BIT(5)

#define TI_SCI_MSG_VALUE_RM_ALL_NO_ORDER \
	(TI_SCI_MSG_VALUE_RM_RING_ADDR_LO_VALID | \
	TI_SCI_MSG_VALUE_RM_RING_ADDR_HI_VALID | \
	TI_SCI_MSG_VALUE_RM_RING_COUNT_VALID | \
	TI_SCI_MSG_VALUE_RM_RING_MODE_VALID | \
	TI_SCI_MSG_VALUE_RM_RING_SIZE_VALID)

/**
 * struct ti_sci_rm_ringacc_ops - Ring Accelerator Management operations
 * @config: configure the SoC Navigator Subsystem Ring Accelerator ring
 * @get_config: get the SoC Navigator Subsystem Ring Accelerator ring
 *		configuration
 */
struct ti_sci_rm_ringacc_ops {
	int (*config)(const struct ti_sci_handle *handle,
		      u32 valid_params, u16 nav_id, u16 index,
		      u32 addr_lo, u32 addr_hi, u32 count, u8 mode,
		      u8 size, u8 order_id
	);
	int (*get_config)(const struct ti_sci_handle *handle,
			  u32 nav_id, u32 index, u8 *mode,
			  u32 *addr_lo, u32 *addr_hi, u32 *count,
			  u8 *size, u8 *order_id);
};

/**
 * struct ti_sci_rm_psil_ops - PSI-L thread operations
 * @pair: pair PSI-L source thread to a destination thread.
 *	If the src_thread is mapped to UDMA tchan, the corresponding channel's
 *	TCHAN_THRD_ID register is updated.
 *	If the dst_thread is mapped to UDMA rchan, the corresponding channel's
 *	RCHAN_THRD_ID register is updated.
 * @unpair: unpair PSI-L source thread from a destination thread.
 *	If the src_thread is mapped to UDMA tchan, the corresponding channel's
 *	TCHAN_THRD_ID register is cleared.
 *	If the dst_thread is mapped to UDMA rchan, the corresponding channel's
 *	RCHAN_THRD_ID register is cleared.
 */
struct ti_sci_rm_psil_ops {
	int (*pair)(const struct ti_sci_handle *handle, u32 nav_id,
		    u32 src_thread, u32 dst_thread);
	int (*unpair)(const struct ti_sci_handle *handle, u32 nav_id,
		      u32 src_thread, u32 dst_thread);
};

/* UDMAP channel types */
#define TI_SCI_RM_UDMAP_CHAN_TYPE_PKT_PBRR		2
#define TI_SCI_RM_UDMAP_CHAN_TYPE_PKT_PBRR_SB		3	/* RX only */
#define TI_SCI_RM_UDMAP_CHAN_TYPE_3RDP_PBRR		10
#define TI_SCI_RM_UDMAP_CHAN_TYPE_3RDP_PBVR		11
#define TI_SCI_RM_UDMAP_CHAN_TYPE_3RDP_BCOPY_PBRR	12
#define TI_SCI_RM_UDMAP_CHAN_TYPE_3RDP_BCOPY_PBVR	13

/* UDMAP channel atypes */
#define TI_SCI_RM_UDMAP_ATYPE_PHYS			0
#define TI_SCI_RM_UDMAP_ATYPE_INTERMEDIATE		1
#define TI_SCI_RM_UDMAP_ATYPE_VIRTUAL			2

/* UDMAP channel scheduling priorities */
#define TI_SCI_RM_UDMAP_SCHED_PRIOR_HIGH		0
#define TI_SCI_RM_UDMAP_SCHED_PRIOR_MEDHIGH		1
#define TI_SCI_RM_UDMAP_SCHED_PRIOR_MEDLOW		2
#define TI_SCI_RM_UDMAP_SCHED_PRIOR_LOW			3

#define TI_SCI_RM_UDMAP_RX_FLOW_DESC_HOST		0
#define TI_SCI_RM_UDMAP_RX_FLOW_DESC_MONO		2

/* UDMAP TX/RX channel valid_params common declarations */
#define TI_SCI_MSG_VALUE_RM_UDMAP_CH_PAUSE_ON_ERR_VALID		BIT(0)
#define TI_SCI_MSG_VALUE_RM_UDMAP_CH_ATYPE_VALID                BIT(1)
#define TI_SCI_MSG_VALUE_RM_UDMAP_CH_CHAN_TYPE_VALID            BIT(2)
#define TI_SCI_MSG_VALUE_RM_UDMAP_CH_FETCH_SIZE_VALID           BIT(3)
#define TI_SCI_MSG_VALUE_RM_UDMAP_CH_CQ_QNUM_VALID              BIT(4)
#define TI_SCI_MSG_VALUE_RM_UDMAP_CH_PRIORITY_VALID             BIT(5)
#define TI_SCI_MSG_VALUE_RM_UDMAP_CH_QOS_VALID                  BIT(6)
#define TI_SCI_MSG_VALUE_RM_UDMAP_CH_ORDER_ID_VALID             BIT(7)
#define TI_SCI_MSG_VALUE_RM_UDMAP_CH_SCHED_PRIORITY_VALID       BIT(8)

/**
 * Configures a Navigator Subsystem UDMAP transmit channel
 *
 * Configures a Navigator Subsystem UDMAP transmit channel registers.
 * See @ti_sci_msg_rm_udmap_tx_ch_cfg_req
 */
struct ti_sci_msg_rm_udmap_tx_ch_cfg {
	u32 valid_params;
#define TI_SCI_MSG_VALUE_RM_UDMAP_CH_TX_FILT_EINFO_VALID        BIT(9)
#define TI_SCI_MSG_VALUE_RM_UDMAP_CH_TX_FILT_PSWORDS_VALID      BIT(10)
#define TI_SCI_MSG_VALUE_RM_UDMAP_CH_TX_SUPR_TDPKT_VALID        BIT(11)
#define TI_SCI_MSG_VALUE_RM_UDMAP_CH_TX_CREDIT_COUNT_VALID      BIT(12)
#define TI_SCI_MSG_VALUE_RM_UDMAP_CH_TX_FDEPTH_VALID            BIT(13)
	u16 nav_id;
	u16 index;
	u8 tx_pause_on_err;
	u8 tx_filt_einfo;
	u8 tx_filt_pswords;
	u8 tx_atype;
	u8 tx_chan_type;
	u8 tx_supr_tdpkt;
	u16 tx_fetch_size;
	u8 tx_credit_count;
	u16 txcq_qnum;
	u8 tx_priority;
	u8 tx_qos;
	u8 tx_orderid;
	u16 fdepth;
	u8 tx_sched_priority;
};

/**
 * Configures a Navigator Subsystem UDMAP receive channel
 *
 * Configures a Navigator Subsystem UDMAP receive channel registers.
 * See @ti_sci_msg_rm_udmap_rx_ch_cfg_req
 */
struct ti_sci_msg_rm_udmap_rx_ch_cfg {
	u32 valid_params;
#define TI_SCI_MSG_VALUE_RM_UDMAP_CH_RX_FLOWID_START_VALID      BIT(9)
#define TI_SCI_MSG_VALUE_RM_UDMAP_CH_RX_FLOWID_CNT_VALID        BIT(10)
#define TI_SCI_MSG_VALUE_RM_UDMAP_CH_RX_IGNORE_SHORT_VALID      BIT(11)
#define TI_SCI_MSG_VALUE_RM_UDMAP_CH_RX_IGNORE_LONG_VALID       BIT(12)
	u16 nav_id;
	u16 index;
	u16 rx_fetch_size;
	u16 rxcq_qnum;
	u8 rx_priority;
	u8 rx_qos;
	u8 rx_orderid;
	u8 rx_sched_priority;
	u16 flowid_start;
	u16 flowid_cnt;
	u8 rx_pause_on_err;
	u8 rx_atype;
	u8 rx_chan_type;
	u8 rx_ignore_short;
	u8 rx_ignore_long;
};

/**
 * Configures a Navigator Subsystem UDMAP receive flow
 *
 * Configures a Navigator Subsystem UDMAP receive flow's registers.
 * See @tis_ci_msg_rm_udmap_flow_cfg_req
 */
struct ti_sci_msg_rm_udmap_flow_cfg {
	u32 valid_params;
#define TI_SCI_MSG_VALUE_RM_UDMAP_FLOW_EINFO_PRESENT_VALID	BIT(0)
#define TI_SCI_MSG_VALUE_RM_UDMAP_FLOW_PSINFO_PRESENT_VALID     BIT(1)
#define TI_SCI_MSG_VALUE_RM_UDMAP_FLOW_ERROR_HANDLING_VALID     BIT(2)
#define TI_SCI_MSG_VALUE_RM_UDMAP_FLOW_DESC_TYPE_VALID          BIT(3)
#define TI_SCI_MSG_VALUE_RM_UDMAP_FLOW_SOP_OFFSET_VALID         BIT(4)
#define TI_SCI_MSG_VALUE_RM_UDMAP_FLOW_DEST_QNUM_VALID          BIT(5)
#define TI_SCI_MSG_VALUE_RM_UDMAP_FLOW_SRC_TAG_HI_VALID         BIT(6)
#define TI_SCI_MSG_VALUE_RM_UDMAP_FLOW_SRC_TAG_LO_VALID         BIT(7)
#define TI_SCI_MSG_VALUE_RM_UDMAP_FLOW_DEST_TAG_HI_VALID        BIT(8)
#define TI_SCI_MSG_VALUE_RM_UDMAP_FLOW_DEST_TAG_LO_VALID        BIT(9)
#define TI_SCI_MSG_VALUE_RM_UDMAP_FLOW_SRC_TAG_HI_SEL_VALID     BIT(10)
#define TI_SCI_MSG_VALUE_RM_UDMAP_FLOW_SRC_TAG_LO_SEL_VALID     BIT(11)
#define TI_SCI_MSG_VALUE_RM_UDMAP_FLOW_DEST_TAG_HI_SEL_VALID    BIT(12)
#define TI_SCI_MSG_VALUE_RM_UDMAP_FLOW_DEST_TAG_LO_SEL_VALID    BIT(13)
#define TI_SCI_MSG_VALUE_RM_UDMAP_FLOW_FDQ0_SZ0_QNUM_VALID      BIT(14)
#define TI_SCI_MSG_VALUE_RM_UDMAP_FLOW_FDQ1_QNUM_VALID          BIT(15)
#define TI_SCI_MSG_VALUE_RM_UDMAP_FLOW_FDQ2_QNUM_VALID          BIT(16)
#define TI_SCI_MSG_VALUE_RM_UDMAP_FLOW_FDQ3_QNUM_VALID          BIT(17)
#define TI_SCI_MSG_VALUE_RM_UDMAP_FLOW_PS_LOCATION_VALID        BIT(18)
	u16 nav_id;
	u16 flow_index;
	u8 rx_einfo_present;
	u8 rx_psinfo_present;
	u8 rx_error_handling;
	u8 rx_desc_type;
	u16 rx_sop_offset;
	u16 rx_dest_qnum;
	u8 rx_src_tag_hi;
	u8 rx_src_tag_lo;
	u8 rx_dest_tag_hi;
	u8 rx_dest_tag_lo;
	u8 rx_src_tag_hi_sel;
	u8 rx_src_tag_lo_sel;
	u8 rx_dest_tag_hi_sel;
	u8 rx_dest_tag_lo_sel;
	u16 rx_fdq0_sz0_qnum;
	u16 rx_fdq1_qnum;
	u16 rx_fdq2_qnum;
	u16 rx_fdq3_qnum;
	u8 rx_ps_location;
};

/**
 * struct ti_sci_rm_udmap_ops - UDMA Management operations
 * @tx_ch_cfg: configure SoC Navigator Subsystem UDMA transmit channel.
 * @rx_ch_cfg: configure SoC Navigator Subsystem UDMA receive channel.
 * @rx_flow_cfg: configure SoC Navigator Subsystem UDMA receive flow.
 */
struct ti_sci_rm_udmap_ops {
	int (*tx_ch_cfg)(const struct ti_sci_handle *handle,
			 const struct ti_sci_msg_rm_udmap_tx_ch_cfg *params);
	int (*rx_ch_cfg)(const struct ti_sci_handle *handle,
			 const struct ti_sci_msg_rm_udmap_rx_ch_cfg *params);
	int (*rx_flow_cfg)(
		const struct ti_sci_handle *handle,
		const struct ti_sci_msg_rm_udmap_flow_cfg *params);
};

/**
 * struct ti_sci_msg_fwl_region_cfg - Request and Response for firewalls settings
 *
 * @fwl_id:		Firewall ID in question
 * @region:		Region or channel number to set config info
 *			This field is unused in case of a simple firewall  and must be initialized
 *			to zero.  In case of a region based firewall, this field indicates the
 *			region in question. (index starting from 0) In case of a channel based
 *			firewall, this field indicates the channel in question (index starting
 *			from 0)
 * @n_permission_regs:	Number of permission registers to set
 * @control:		Contents of the firewall CONTROL register to set
 * @permissions:	Contents of the firewall PERMISSION register to set
 * @start_address:	Contents of the firewall START_ADDRESS register to set
 * @end_address:	Contents of the firewall END_ADDRESS register to set
 */
struct ti_sci_msg_fwl_region {
	u16 fwl_id;
	u16 region;
	u32 n_permission_regs;
	u32 control;
	u32 permissions[3];
	u64 start_address;
	u64 end_address;
} __packed;

/**
 * \brief Request and Response for firewall owner change
 *
 * @fwl_id:		Firewall ID in question
 * @region:		Region or channel number to set config info
 *			This field is unused in case of a simple firewall  and must be initialized
 *			to zero.  In case of a region based firewall, this field indicates the
 *			region in question. (index starting from 0) In case of a channel based
 *			firewall, this field indicates the channel in question (index starting
 *			from 0)
 * @n_permission_regs:	Number of permission registers <= 3
 * @control:		Control register value for this region
 * @owner_index:	New owner index to change to. Owner indexes are setup in DMSC firmware boot configuration data
 * @owner_privid:	New owner priv-id, used to lookup owner_index is not known, must be set to zero otherwise
 * @owner_permission_bits: New owner permission bits
 */
struct ti_sci_msg_fwl_owner {
	u16 fwl_id;
	u16 region;
	u8 owner_index;
	u8 owner_privid;
	u16 owner_permission_bits;
} __packed;

/**
 * struct ti_sci_fwl_ops - Firewall specific operations
 * @set_fwl_region: Request for configuring the firewall permissions.
 * @get_fwl_region: Request for retrieving the firewall permissions.
 * @change_fwl_owner: Request for a change of firewall owner.
 */
struct ti_sci_fwl_ops {
	int (*set_fwl_region)(const struct ti_sci_handle *handle, const struct ti_sci_msg_fwl_region *region);
	int (*get_fwl_region)(const struct ti_sci_handle *handle, struct ti_sci_msg_fwl_region *region);
	int (*change_fwl_owner)(const struct ti_sci_handle *handle, struct ti_sci_msg_fwl_owner *owner);
};

/**
 * struct ti_sci_ops - Function support for TI SCI
 * @board_ops:	Miscellaneous operations
 * @dev_ops:	Device specific operations
 * @clk_ops:	Clock specific operations
 * @core_ops:	Core specific operations
 * @proc_ops:	Processor specific operations
 * @ring_ops: Ring Accelerator Management operations
 * @fw_ops:	Firewall specific operations
 */
struct ti_sci_ops {
	struct ti_sci_board_ops board_ops;
	struct ti_sci_dev_ops dev_ops;
	struct ti_sci_clk_ops clk_ops;
	struct ti_sci_core_ops core_ops;
	struct ti_sci_proc_ops proc_ops;
	struct ti_sci_rm_core_ops rm_core_ops;
	struct ti_sci_rm_ringacc_ops rm_ring_ops;
	struct ti_sci_rm_psil_ops rm_psil_ops;
	struct ti_sci_rm_udmap_ops rm_udmap_ops;
	struct ti_sci_fwl_ops fwl_ops;
};

/**
 * struct ti_sci_handle - Handle returned to TI SCI clients for usage.
 * @ops:	operations that are made available to TI SCI clients
 * @version:	structure containing version information
 */
struct ti_sci_handle {
	struct ti_sci_ops ops;
	struct ti_sci_version_info version;
};

#define TI_SCI_RESOURCE_NULL	0xffff

/**
 * struct ti_sci_resource_desc - Description of TI SCI resource instance range.
 * @start:	Start index of the resource.
 * @num:	Number of resources.
 * @res_map:	Bitmap to manage the allocation of these resources.
 */
struct ti_sci_resource_desc {
	u16 start;
	u16 num;
	unsigned long *res_map;
};

/**
 * struct ti_sci_resource - Structure representing a resource assigned
 *			    to a device.
 * @sets:	Number of sets available from this resource type
 * @desc:	Array of resource descriptors.
 */
struct ti_sci_resource {
	u16 sets;
	struct ti_sci_resource_desc *desc;
};

#if IS_ENABLED(CONFIG_TI_SCI_PROTOCOL)

const struct ti_sci_handle *ti_sci_get_handle_from_sysfw(struct udevice *dev);
const struct ti_sci_handle *ti_sci_get_handle(struct udevice *dev);
const struct ti_sci_handle *ti_sci_get_by_phandle(struct udevice *dev,
						  const char *property);
u16 ti_sci_get_free_resource(struct ti_sci_resource *res);
void ti_sci_release_resource(struct ti_sci_resource *res, u16 id);
struct ti_sci_resource *
devm_ti_sci_get_of_resource(const struct ti_sci_handle *handle,
			    struct udevice *dev, u32 dev_id, char *of_prop);

#else	/* CONFIG_TI_SCI_PROTOCOL */

static inline
const struct ti_sci_handle *ti_sci_get_handle_from_sysfw(struct udevice *dev)
{
	return ERR_PTR(-EINVAL);
}

static inline const struct ti_sci_handle *ti_sci_get_handle(struct udevice *dev)
{
	return ERR_PTR(-EINVAL);
}

static inline
const struct ti_sci_handle *ti_sci_get_by_phandle(struct udevice *dev,
						  const char *property)
{
	return ERR_PTR(-EINVAL);
}

static inline u16 ti_sci_get_free_resource(struct ti_sci_resource *res)
{
	return TI_SCI_RESOURCE_NULL;
}

static inline void ti_sci_release_resource(struct ti_sci_resource *res, u16 id)
{
}

static inline struct ti_sci_resource *
devm_ti_sci_get_of_resource(const struct ti_sci_handle *handle,
			    struct udevice *dev, u32 dev_id, char *of_prop)
{
	return ERR_PTR(-EINVAL);
}
#endif	/* CONFIG_TI_SCI_PROTOCOL */

#endif	/* __TISCI_PROTOCOL_H */
