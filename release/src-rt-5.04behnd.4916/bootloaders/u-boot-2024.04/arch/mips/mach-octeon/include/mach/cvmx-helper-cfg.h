/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2020 Marvell International Ltd.
 *
 * Helper Functions for the Configuration Framework
 *
 * OCTEON_CN68XX introduces a flexible hw interface configuration
 * scheme. To cope with this change and the requirements of
 * configurability for other system resources, e.g., IPD/PIP pknd and
 * PKO ports and queues, a configuration framework for the SDK is
 * designed. It has two goals: first to recognize and establish the
 * default configuration and, second, to allow the user to define key
 * parameters in a high-level language.
 *
 * The helper functions query the QLM setup to help achieving the
 * first goal.
 *
 * The second goal is accomplished by generating
 * cvmx_helper_cfg_init() from a high-level lanaguage.
 */

#ifndef __CVMX_HELPER_CFG_H__
#define __CVMX_HELPER_CFG_H__

#include "cvmx-helper-util.h"

#define CVMX_HELPER_CFG_MAX_PKO_PORT	   128
#define CVMX_HELPER_CFG_MAX_PIP_BPID	   64
#define CVMX_HELPER_CFG_MAX_PIP_PKND	   64
#define CVMX_HELPER_CFG_MAX_PKO_QUEUES	   256
#define CVMX_HELPER_CFG_MAX_PORT_PER_IFACE 256

#define CVMX_HELPER_CFG_INVALID_VALUE -1

#define cvmx_helper_cfg_assert(cond)                                                               \
	do {                                                                                       \
		if (!(cond)) {                                                                     \
			debug("cvmx_helper_cfg_assert (%s) at %s:%d\n", #cond, __FILE__,           \
			      __LINE__);                                                           \
		}                                                                                  \
	} while (0)

extern int cvmx_npi_max_pknds;

/*
 * Config Options
 *
 * These options have to be set via cvmx_helper_cfg_opt_set() before calling the
 * routines that set up the hw. These routines process the options and set them
 * correctly to take effect at runtime.
 */
enum cvmx_helper_cfg_option {
	CVMX_HELPER_CFG_OPT_USE_DWB, /*
					 * Global option to control if
					 * the SDK configures units (DMA,
					 * SSO, and PKO) to send don't
					 * write back (DWB) requests for
					 * freed buffers. Set to 1/0 to
					 * enable/disable DWB.
					 *
					 * For programs that fit inside
					 * L2, sending DWB just causes
					 * more L2 operations without
					 * benefit.
					 */

	CVMX_HELPER_CFG_OPT_MAX
};

typedef enum cvmx_helper_cfg_option cvmx_helper_cfg_option_t;

struct cvmx_phy_info;
struct cvmx_fdt_sfp_info;
struct cvmx_vsc7224_chan;
struct phy_device;

struct cvmx_srio_port_param {
	/** True to override SRIO CTLE zero setting */
	bool srio_rx_ctle_zero_override : 1;
	/** Equalization peaking control dft: 6 */
	u8 srio_rx_ctle_zero : 4;
	/** Set true to override CTLE taps */
	bool srio_rx_ctle_agc_override : 1;
	u8 srio_rx_agc_pre_ctle : 4;	    /** AGC pre-CTLE gain */
	u8 srio_rx_agc_post_ctle : 4;	    /** AGC post-CTLE gain */
	bool srio_tx_swing_override : 1;    /** True to override TX Swing */
	u8 srio_tx_swing : 5;		    /** TX Swing */
	bool srio_tx_gain_override : 1;	    /** True to override TX gain */
	u8 srio_tx_gain : 3;		    /** TX gain */
	bool srio_tx_premptap_override : 1; /** True to override premptap values */
	u8 srio_tx_premptap_pre : 4;	    /** Pre premptap value */
	u8 srio_tx_premptap_post : 5;	    /** Post premptap value */
	bool srio_tx_vboost_override : 1;   /** True to override TX vboost setting */
	bool srio_tx_vboost : 1;	    /** vboost setting (default 1) */
};

/*
 * Per physical port
 * Note: This struct is passed between linux and SE apps.
 */
struct cvmx_cfg_port_param {
	int port_fdt_node;		/** Node offset in FDT of node */
	int phy_fdt_node;		/** Node offset in FDT of PHY */
	struct cvmx_phy_info *phy_info; /** Data structure with PHY information */
	int8_t ccpp_pknd;
	int8_t ccpp_bpid;
	int8_t ccpp_pko_port_base;
	int8_t ccpp_pko_num_ports;
	u8 agl_rx_clk_skew;		  /** AGL rx clock skew setting (default 0) */
	u8 rgmii_tx_clk_delay;		  /** RGMII TX clock delay value if not bypassed */
	bool valid : 1;			  /** 1 = port valid, 0 = invalid */
	bool sgmii_phy_mode : 1;	  /** 1 = port in PHY mode, 0 = MAC mode */
	bool sgmii_1000x_mode : 1;	  /** 1 = 1000Base-X mode, 0 = SGMII mode */
	bool agl_rx_clk_delay_bypass : 1; /** 1 = use rx clock delay bypass for AGL mode */
	bool force_link_up : 1;		  /** Ignore PHY and always report link up */
	bool disable_an : 1;		  /** true to disable autonegotiation */
	bool link_down_pwr_dn : 1;	  /** Power PCS off when link is down */
	bool phy_present : 1;		  /** true if PHY is present */
	bool tx_clk_delay_bypass : 1;	  /** True to bypass the TX clock delay */
	bool enable_fec : 1;		  /** True to enable FEC for 10/40G links */
	/** Settings for short-run SRIO host */
	struct cvmx_srio_port_param srio_short;
	/** Settings for long-run SRIO host */
	struct cvmx_srio_port_param srio_long;
	u8 agl_refclk_sel; /** RGMII refclk select to use */
	/** Set if local (non-PHY) LEDs are used */
	struct cvmx_phy_gpio_leds *gpio_leds;
	struct cvmx_fdt_sfp_info *sfp_info; /** SFP+/QSFP info for port */
	/** Offset of SFP/SFP+/QSFP slot in device tree */
	int sfp_of_offset;
	/** Microsemi VSC7224 channel info data structure */
	struct cvmx_vsc7224_chan *vsc7224_chan;
	/** Avago AVSP-5410 Phy */
	struct cvmx_avsp5410 *avsp5410;
	struct phy_device *phydev;
};

/*
 * Per pko_port
 */
struct cvmx_cfg_pko_port_param {
	s16 ccppp_queue_base;
	s16 ccppp_num_queues;
};

/*
 * A map from pko_port to
 *     interface,
 *     index, and
 *     pko engine id
 */
struct cvmx_cfg_pko_port_map {
	s16 ccppl_interface;
	s16 ccppl_index;
	s16 ccppl_eid;
};

/*
 * This is for looking up pko_base_port and pko_nport for ipd_port
 */
struct cvmx_cfg_pko_port_pair {
	int8_t ccppp_base_port;
	int8_t ccppp_nports;
};

typedef union cvmx_user_static_pko_queue_config {
	struct {
		struct pko_queues_cfg {
			unsigned queues_per_port : 11, qos_enable : 1, pfc_enable : 1;
		} pko_cfg_iface[6];
		struct pko_queues_cfg pko_cfg_loop;
		struct pko_queues_cfg pko_cfg_npi;
	} pknd;
	struct {
		u8 pko_ports_per_interface[5];
		u8 pko_queues_per_port_interface[5];
		u8 pko_queues_per_port_loop;
		u8 pko_queues_per_port_pci;
		u8 pko_queues_per_port_srio[4];
	} non_pknd;
} cvmx_user_static_pko_queue_config_t;

extern cvmx_user_static_pko_queue_config_t __cvmx_pko_queue_static_config[CVMX_MAX_NODES];
extern struct cvmx_cfg_pko_port_map cvmx_cfg_pko_port_map[CVMX_HELPER_CFG_MAX_PKO_PORT];
extern struct cvmx_cfg_port_param cvmx_cfg_port[CVMX_MAX_NODES][CVMX_HELPER_MAX_IFACE]
					       [CVMX_HELPER_CFG_MAX_PORT_PER_IFACE];
extern struct cvmx_cfg_pko_port_param cvmx_pko_queue_table[];
extern int cvmx_enable_helper_flag;

/*
 * @INTERNAL
 * Return configured pknd for the port
 *
 * @param interface the interface number
 * @param index the port's index number
 * Return: the pknd
 */
int __cvmx_helper_cfg_pknd(int interface, int index);

/*
 * @INTERNAL
 * Return the configured bpid for the port
 *
 * @param interface the interface number
 * @param index the port's index number
 * Return: the bpid
 */
int __cvmx_helper_cfg_bpid(int interface, int index);

/**
 * @INTERNAL
 * Return the configured pko_port base for the port
 *
 * @param interface the interface number
 * @param index the port's index number
 * Return: the pko_port base
 */
int __cvmx_helper_cfg_pko_port_base(int interface, int index);

/*
 * @INTERNAL
 * Return the configured number of pko_ports for the port
 *
 * @param interface the interface number
 * @param index the port's index number
 * Return: the number of pko_ports
 */
int __cvmx_helper_cfg_pko_port_num(int interface, int index);

/*
 * @INTERNAL
 * Return the configured pko_queue base for the pko_port
 *
 * @param pko_port
 * Return: the pko_queue base
 */
int __cvmx_helper_cfg_pko_queue_base(int pko_port);

/*
 * @INTERNAL
 * Return the configured number of pko_queues for the pko_port
 *
 * @param pko_port
 * Return: the number of pko_queues
 */
int __cvmx_helper_cfg_pko_queue_num(int pko_port);

/*
 * @INTERNAL
 * Return the interface the pko_port is configured for
 *
 * @param pko_port
 * Return: the interface for the pko_port
 */
int __cvmx_helper_cfg_pko_port_interface(int pko_port);

/*
 * @INTERNAL
 * Return the index of the port the pko_port is configured for
 *
 * @param pko_port
 * Return: the index of the port
 */
int __cvmx_helper_cfg_pko_port_index(int pko_port);

/*
 * @INTERNAL
 * Return the pko_eid of the pko_port
 *
 * @param pko_port
 * Return: the pko_eid
 */
int __cvmx_helper_cfg_pko_port_eid(int pko_port);

/*
 * @INTERNAL
 * Return the max# of pko queues allocated.
 *
 * Return: the max# of pko queues
 *
 * Note: there might be holes in the queue space depending on user
 * configuration. The function returns the highest queue's index in
 * use.
 */
int __cvmx_helper_cfg_pko_max_queue(void);

/*
 * @INTERNAL
 * Return the max# of PKO DMA engines allocated.
 *
 * Return: the max# of DMA engines
 *
 * NOTE: the DMA engines are allocated contiguously and starting from
 * 0.
 */
int __cvmx_helper_cfg_pko_max_engine(void);

/*
 * Get the value set for the config option ``opt''.
 *
 * @param opt is the config option.
 * Return: the value set for the option
 *
 * LR: only used for DWB in NPI, POW, PKO1
 */
u64 cvmx_helper_cfg_opt_get(cvmx_helper_cfg_option_t opt);

/*
 * Set the value for a config option.
 *
 * @param opt is the config option.
 * @param val is the value to set for the opt.
 * Return: 0 for success and -1 on error
 *
 * Note an option here is a config-time parameter and this means that
 * it has to be set before calling the corresponding setup functions
 * that actually sets the option in hw.
 *
 * LR: Not used.
 */
int cvmx_helper_cfg_opt_set(cvmx_helper_cfg_option_t opt, u64 val);

/*
 * Retrieve the pko_port base given ipd_port.
 *
 * @param ipd_port is the IPD eport
 * Return: the corresponding PKO port base for the physical port
 * represented by the IPD eport or CVMX_HELPER_CFG_INVALID_VALUE.
 */
int cvmx_helper_cfg_ipd2pko_port_base(int ipd_port);

/*
 * Retrieve the number of pko_ports given ipd_port.
 *
 * @param ipd_port is the IPD eport
 * Return: the corresponding number of PKO ports for the physical port
 *  represented by IPD eport or CVMX_HELPER_CFG_INVALID_VALUE.
 */
int cvmx_helper_cfg_ipd2pko_port_num(int ipd_port);

/*
 * @INTERNAL
 * The init function
 *
 * @param node
 * Return: 0 for success.
 *
 * Note: this function is meant to be called to set the ``configured
 * parameters,'' e.g., pknd, bpid, etc. and therefore should be before
 * any of the corresponding cvmx_helper_cfg_xxxx() functions are
 * called.
 */
int __cvmx_helper_init_port_config_data(int node);

/*
 * @INTERNAL
 * The local init function
 *
 * @param none
 * Return: 0 for success.
 *
 * Note: this function is meant to be called to set the ``configured
 * parameters locally,'' e.g., pknd, bpid, etc. and therefore should be before
 * any of the corresponding cvmx_helper_cfg_xxxx() functions are
 * called.
 */
int __cvmx_helper_init_port_config_data_local(void);

/*
 * Set the frame max size and jabber size to 65535.
 *
 */
void cvmx_helper_cfg_set_jabber_and_frame_max(void);

/*
 * Enable storing short packets only in the WQE.
 */
void cvmx_helper_cfg_store_short_packets_in_wqe(void);

/*
 * Allocated a block of internal ports and queues for the specified
 * interface/port
 *
 * @param  interface  the interface for which the internal ports and queues
 *                    are requested
 * @param  port       the index of the port within in the interface for which
		      the internal ports and queues are requested.
 * @param  pot_count  the number of internal ports requested
 * @param  queue_cnt  the number of queues requested for each of the internal
 *                    port. This call will allocate a total of
 *		      (port_cnt * queue_cnt) queues
 *
 * Return:  0 on success
 *         -1 on failure
 *
 * LR: Called ONLY from comfig-parse!
 */
int cvmx_pko_alloc_iport_and_queues(int interface, int port, int port_cnt, int queue_cnt);

/*
 * Free the queues that are associated with the specified port
 *
 * @param  port   the internal port for which the queues are freed.
 *
 * Return:  0 on success
 *         -1 on failure
 */
int cvmx_pko_queue_free(u64 port);

/*
 * Initializes the pko queue range data structure.
 * Return:  0 on success
 *         -1 on failure
 */
int init_cvmx_pko_que_range(void);

/*
 * Frees up all the allocated ques.
 */
void cvmx_pko_queue_free_all(void);

/**
 * Returns if port is valid for a given interface
 *
 * @param xiface     interface to check
 * @param index      port index in the interface
 *
 * Return: status of the port present or not.
 */
int cvmx_helper_is_port_valid(int xiface, int index);

/**
 * Set whether or not a port is valid
 *
 * @param interface interface to set
 * @param index     port index to set
 * @param valid     set 0 to make port invalid, 1 for valid
 */
void cvmx_helper_set_port_valid(int interface, int index, bool valid);

/**
 * @INTERNAL
 * Return if port is in PHY mode
 *
 * @param interface the interface number
 * @param index the port's index number
 *
 * Return: 1 if port is in PHY mode, 0 if port is in MAC mode
 */
bool cvmx_helper_get_mac_phy_mode(int interface, int index);
void cvmx_helper_set_mac_phy_mode(int interface, int index, bool valid);

/**
 * @INTERNAL
 * Return if port is in 1000Base X mode
 *
 * @param interface the interface number
 * @param index the port's index number
 *
 * Return: 1 if port is in 1000Base X mode, 0 if port is in SGMII mode
 */
bool cvmx_helper_get_1000x_mode(int interface, int index);
void cvmx_helper_set_1000x_mode(int interface, int index, bool valid);

/**
 * @INTERNAL
 * Return if an AGL port should bypass the RX clock delay
 *
 * @param interface the interface number
 * @param index the port's index number
 */
bool cvmx_helper_get_agl_rx_clock_delay_bypass(int interface, int index);
void cvmx_helper_set_agl_rx_clock_delay_bypass(int interface, int index, bool valid);

/**
 * @INTERNAL
 * Forces a link to always return that it is up ignoring the PHY (if present)
 *
 * @param interface the interface number
 * @param index the port's index
 */
bool cvmx_helper_get_port_force_link_up(int interface, int index);
void cvmx_helper_set_port_force_link_up(int interface, int index, bool value);

/**
 * @INTERNAL
 * Return true if PHY is present to the passed xiface
 *
 * @param xiface the interface number
 * @param index the port's index
 */
bool cvmx_helper_get_port_phy_present(int xiface, int index);
void cvmx_helper_set_port_phy_present(int xiface, int index, bool value);

/**
 * @INTERNAL
 * Return the AGL port rx clock skew, only used
 * if agl_rx_clock_delay_bypass is set.
 *
 * @param interface the interface number
 * @param index the port's index number
 */
u8 cvmx_helper_get_agl_rx_clock_skew(int interface, int index);
void cvmx_helper_set_agl_rx_clock_skew(int interface, int index, u8 value);
u8 cvmx_helper_get_agl_refclk_sel(int interface, int index);
void cvmx_helper_set_agl_refclk_sel(int interface, int index, u8 value);

/**
 * @INTERNAL
 * Store the FDT node offset in the device tree of a port
 *
 * @param xiface	node and interface
 * @param index		port index
 * @param node_offset	node offset to store
 */
void cvmx_helper_set_port_fdt_node_offset(int xiface, int index, int node_offset);

/**
 * @INTERNAL
 * Return the FDT node offset in the device tree of a port
 *
 * @param xiface	node and interface
 * @param index		port index
 * Return:		node offset of port or -1 if invalid
 */
int cvmx_helper_get_port_fdt_node_offset(int xiface, int index);

/**
 * @INTERNAL
 * Store the FDT node offset in the device tree of a phy
 *
 * @param xiface	node and interface
 * @param index		port index
 * @param node_offset	node offset to store
 */
void cvmx_helper_set_phy_fdt_node_offset(int xiface, int index, int node_offset);

/**
 * @INTERNAL
 * Return the FDT node offset in the device tree of a phy
 *
 * @param xiface	node and interface
 * @param index		port index
 * Return:		node offset of phy or -1 if invalid
 */
int cvmx_helper_get_phy_fdt_node_offset(int xiface, int index);

/**
 * @INTERNAL
 * Override default autonegotiation for a port
 *
 * @param xiface	node and interface
 * @param index		port index
 * @param enable	true to enable autonegotiation, false to force full
 *			duplex, full speed.
 */
void cvmx_helper_set_port_autonegotiation(int xiface, int index, bool enable);

/**
 * @INTERNAL
 * Returns if autonegotiation is enabled or not.
 *
 * @param xiface	node and interface
 * @param index		port index
 *
 * Return: 0 if autonegotiation is disabled, 1 if enabled.
 */
bool cvmx_helper_get_port_autonegotiation(int xiface, int index);

/**
 * @INTERNAL
 * Returns if forward error correction is enabled or not.
 *
 * @param xiface	node and interface
 * @param index		port index
 *
 * Return: 0 if fec is disabled, 1 if enabled.
 */
bool cvmx_helper_get_port_fec(int xiface, int index);

/**
 * @INTERNAL
 * Override default forward error correction for a port
 *
 * @param xiface	node and interface
 * @param index		port index
 * @param enable	true to enable fec, false to disable.
 */
void cvmx_helper_set_port_fec(int xiface, int index, bool enable);

/**
 * @INTERNAL
 * Configure the SRIO RX interface AGC settings in host mode
 *
 * @param xiface	node and interface
 * @param index		lane
 * @param long_run	true for long run, false for short run
 * @param agc_override	true to put AGC in manual mode
 * @param ctle_zero	RX equalizer peaking control (default 0x6)
 * @param agc_pre_ctle	AGC pre-CTLE gain (default 0x5)
 * @param agc_post_ctle	AGC post-CTLE gain (default 0x4)
 *
 * NOTE: This must be called before SRIO is initialized to take effect
 */
void cvmx_helper_set_srio_rx(int xiface, int index, bool long_run, bool ctle_zero_override,
			     u8 ctle_zero, bool agc_override, u8 agc_pre_ctle, u8 agc_post_ctle);

/**
 * @INTERNAL
 * Get the SRIO RX interface AGC settings for host mode
 *
 * @param xiface	node and interface
 * @param index		lane
 * @param long_run	true for long run, false for short run
 * @param[out] ctle_zero_override true if overridden
 * @param[out] ctle_zero	RX equalizer peaking control (default 0x6)
 * @param[out] agc_override	true to put AGC in manual mode
 * @param[out] agc_pre_ctle	AGC pre-CTLE gain (default 0x5)
 * @param[out] agc_post_ctle	AGC post-CTLE gain (default 0x4)
 */
void cvmx_helper_get_srio_rx(int xiface, int index, bool long_run, bool *ctle_zero_override,
			     u8 *ctle_zero, bool *agc_override, u8 *agc_pre_ctle,
			     u8 *agc_post_ctle);

/**
 * @INTERNAL
 * Configure the SRIO TX interface for host mode
 *
 * @param xiface		node and interface
 * @param index			lane
 * @param long_run	true for long run, false for short run
 * @param tx_swing		tx swing value to use (default 0x7), -1 to not
 *				override.
 * @param tx_gain		PCS SDS TX gain (default 0x3), -1 to not
 *				override
 * @param tx_premptap_override	true to override preemphasis control
 * @param tx_premptap_pre	preemphasis pre tap value (default 0x0)
 * @param tx_premptap_post	preemphasis post tap value (default 0xF)
 * @param tx_vboost		vboost enable (1 = enable, -1 = don't override)
 *				hardware default is 1.
 *
 * NOTE: This must be called before SRIO is initialized to take effect
 */
void cvmx_helper_set_srio_tx(int xiface, int index, bool long_run, int tx_swing, int tx_gain,
			     bool tx_premptap_override, u8 tx_premptap_pre, u8 tx_premptap_post,
			     int tx_vboost);

/**
 * @INTERNAL
 * Get the SRIO TX interface settings for host mode
 *
 * @param xiface			node and interface
 * @param index				lane
 * @param long_run			true for long run, false for short run
 * @param[out] tx_swing_override	true to override pcs_sds_txX_swing
 * @param[out] tx_swing			tx swing value to use (default 0x7)
 * @param[out] tx_gain_override		true to override default gain
 * @param[out] tx_gain			PCS SDS TX gain (default 0x3)
 * @param[out] tx_premptap_override	true to override preemphasis control
 * @param[out] tx_premptap_pre		preemphasis pre tap value (default 0x0)
 * @param[out] tx_premptap_post		preemphasis post tap value (default 0xF)
 * @param[out] tx_vboost_override	override vboost setting
 * @param[out] tx_vboost		vboost enable (default true)
 */
void cvmx_helper_get_srio_tx(int xiface, int index, bool long_run, bool *tx_swing_override,
			     u8 *tx_swing, bool *tx_gain_override, u8 *tx_gain,
			     bool *tx_premptap_override, u8 *tx_premptap_pre, u8 *tx_premptap_post,
			     bool *tx_vboost_override, bool *tx_vboost);

/**
 * @INTERNAL
 * Sets the PHY info data structure
 *
 * @param xiface	node and interface
 * @param index		port index
 * @param[in] phy_info	phy information data structure pointer
 */
void cvmx_helper_set_port_phy_info(int xiface, int index, struct cvmx_phy_info *phy_info);
/**
 * @INTERNAL
 * Returns the PHY information data structure for a port
 *
 * @param xiface	node and interface
 * @param index		port index
 *
 * Return: pointer to PHY information data structure or NULL if not set
 */
struct cvmx_phy_info *cvmx_helper_get_port_phy_info(int xiface, int index);

/**
 * @INTERNAL
 * Returns a pointer to the PHY LED configuration (if local GPIOs drive them)
 *
 * @param xiface	node and interface
 * @param index		portindex
 *
 * Return: pointer to the PHY LED information data structure or NULL if not
 *	   present
 */
struct cvmx_phy_gpio_leds *cvmx_helper_get_port_phy_leds(int xiface, int index);

/**
 * @INTERNAL
 * Sets a pointer to the PHY LED configuration (if local GPIOs drive them)
 *
 * @param xiface	node and interface
 * @param index		portindex
 * @param leds		pointer to led data structure
 */
void cvmx_helper_set_port_phy_leds(int xiface, int index, struct cvmx_phy_gpio_leds *leds);

/**
 * @INTERNAL
 * Disables RGMII TX clock bypass and sets delay value
 *
 * @param xiface	node and interface
 * @param index		portindex
 * @param bypass	Set true to enable the clock bypass and false
 *			to sync clock and data synchronously.
 *			Default is false.
 * @param clk_delay	Delay value to skew TXC from TXD
 */
void cvmx_helper_cfg_set_rgmii_tx_clk_delay(int xiface, int index, bool bypass, int clk_delay);

/**
 * @INTERNAL
 * Gets RGMII TX clock bypass and delay value
 *
 * @param xiface	node and interface
 * @param index		portindex
 * @param bypass	Set true to enable the clock bypass and false
 *			to sync clock and data synchronously.
 *			Default is false.
 * @param clk_delay	Delay value to skew TXC from TXD, default is 0.
 */
void cvmx_helper_cfg_get_rgmii_tx_clk_delay(int xiface, int index, bool *bypass, int *clk_delay);

/**
 * @INTERNAL
 * Retrieve node-specific PKO Queue configuration.
 *
 * @param node		OCTEON3 node.
 * @param pkocfg	PKO Queue static configuration.
 */
int cvmx_helper_pko_queue_config_get(int node, cvmx_user_static_pko_queue_config_t *cfg);

/**
 * @INTERNAL
 * Update node-specific PKO Queue configuration.
 *
 * @param node		OCTEON3 node.
 * @param pkocfg	PKO Queue static configuration.
 */
int cvmx_helper_pko_queue_config_set(int node, cvmx_user_static_pko_queue_config_t *cfg);

/**
 * @INTERNAL
 * Retrieve the SFP node offset in the device tree
 *
 * @param xiface	node and interface
 * @param index		port index
 *
 * Return: offset in device tree or -1 if error or not defined.
 */
int cvmx_helper_cfg_get_sfp_fdt_offset(int xiface, int index);

/**
 * Search for a port based on its FDT node offset
 *
 * @param	of_offset	Node offset of port to search for
 *
 * Return:	ipd_port or -1 if not found
 */
int cvmx_helper_cfg_get_ipd_port_by_fdt_node_offset(int of_offset);

/**
 * @INTERNAL
 * Sets the SFP node offset
 *
 * @param xiface	node and interface
 * @param index		port index
 * @param sfp_of_offset	Offset of SFP node in device tree
 */
void cvmx_helper_cfg_set_sfp_fdt_offset(int xiface, int index, int sfp_of_offset);

/**
 * Search for a port based on its FDT node offset
 *
 * @param	of_offset	Node offset of port to search for
 * @param[out]	xiface		xinterface of match
 * @param[out]	index		port index of match
 *
 * Return:	0 if found, -1 if not found
 */
int cvmx_helper_cfg_get_xiface_index_by_fdt_node_offset(int of_offset, int *xiface, int *index);

/**
 * Get data structure defining the Microsemi VSC7224 channel info
 * or NULL if not present
 *
 * @param xiface	node and interface
 * @param index		port index
 *
 * Return: pointer to vsc7224 data structure or NULL if not present
 */
struct cvmx_vsc7224_chan *cvmx_helper_cfg_get_vsc7224_chan_info(int xiface, int index);

/**
 * Sets the Microsemi VSC7224 channel data structure
 *
 * @param	xiface	node and interface
 * @param	index	port index
 * @param[in]	vsc7224_info	Microsemi VSC7224 data structure
 */
void cvmx_helper_cfg_set_vsc7224_chan_info(int xiface, int index,
					   struct cvmx_vsc7224_chan *vsc7224_chan_info);

/**
 * Get data structure defining the Avago AVSP5410 phy info
 * or NULL if not present
 *
 * @param xiface        node and interface
 * @param index         port index
 *
 * Return: pointer to avsp5410 data structure or NULL if not present
 */
struct cvmx_avsp5410 *cvmx_helper_cfg_get_avsp5410_info(int xiface, int index);

/**
 * Sets the Avago AVSP5410 phy info data structure
 *
 * @param       xiface  node and interface
 * @param       index   port index
 * @param[in]   avsp5410_info   Avago AVSP5410 data structure
 */
void cvmx_helper_cfg_set_avsp5410_info(int xiface, int index, struct cvmx_avsp5410 *avsp5410_info);

/**
 * Gets the SFP data associated with a port
 *
 * @param	xiface	node and interface
 * @param	index	port index
 *
 * Return:	pointer to SFP data structure or NULL if none
 */
struct cvmx_fdt_sfp_info *cvmx_helper_cfg_get_sfp_info(int xiface, int index);

/**
 * Sets the SFP data associated with a port
 *
 * @param	xiface		node and interface
 * @param	index		port index
 * @param[in]	sfp_info	port SFP data or NULL for none
 */
void cvmx_helper_cfg_set_sfp_info(int xiface, int index, struct cvmx_fdt_sfp_info *sfp_info);

/*
 * Initializes cvmx with user specified config info.
 */
int cvmx_user_static_config(void);
void cvmx_pko_queue_show(void);
int cvmx_fpa_pool_init_from_cvmx_config(void);
int __cvmx_helper_init_port_valid(void);

/**
 * Returns a pointer to the phy device associated with a port
 *
 * @param	xiface		node and interface
 * @param	index		port index
 *
 * return	pointer to phy device or NULL if none
 */
struct phy_device *cvmx_helper_cfg_get_phy_device(int xiface, int index);

/**
 * Sets the phy device associated with a port
 *
 * @param	xiface		node and interface
 * @param	index		port index
 * @param[in]	phydev		phy device to assiciate
 */
void cvmx_helper_cfg_set_phy_device(int xiface, int index, struct phy_device *phydev);

#endif /* __CVMX_HELPER_CFG_H__ */
