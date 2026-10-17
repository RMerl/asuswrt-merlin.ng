/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2020 Marvell International Ltd.
 *
 * Helper functions for common, but complicated tasks.
 */

#ifndef __CVMX_HELPER_H__
#define __CVMX_HELPER_H__

#include "cvmx-wqe.h"

/* Max number of GMXX */
#define CVMX_HELPER_MAX_GMX                                                                        \
	(OCTEON_IS_MODEL(OCTEON_CN78XX) ?                                                          \
		       6 :                                                                               \
		       (OCTEON_IS_MODEL(OCTEON_CN68XX) ?                                                 \
				5 :                                                                      \
				(OCTEON_IS_MODEL(OCTEON_CN73XX) ?                                        \
					 3 :                                                             \
					 (OCTEON_IS_MODEL(OCTEON_CNF75XX) ? 1 : 2))))

#define CVMX_HELPER_CSR_INIT0                                                                      \
	0 /* Do not change as
						   CVMX_HELPER_WRITE_CSR()
						   assumes it */
#define CVMX_HELPER_CSR_INIT_READ -1

/*
 * CVMX_HELPER_WRITE_CSR--set a field in a CSR with a value.
 *
 * @param chcsr_init    initial value of the csr (CVMX_HELPER_CSR_INIT_READ
 *                      means to use the existing csr value as the
 *                      initial value.)
 * @param chcsr_csr     the name of the csr
 * @param chcsr_type    the type of the csr (see the -defs.h)
 * @param chcsr_chip    the chip for the csr/field
 * @param chcsr_fld     the field in the csr
 * @param chcsr_val     the value for field
 */
#define CVMX_HELPER_WRITE_CSR(chcsr_init, chcsr_csr, chcsr_type, chcsr_chip, chcsr_fld, chcsr_val) \
	do {                                                                                       \
		chcsr_type csr;                                                                    \
		if ((chcsr_init) == CVMX_HELPER_CSR_INIT_READ)                                     \
			csr.u64 = cvmx_read_csr(chcsr_csr);                                        \
		else                                                                               \
			csr.u64 = (chcsr_init);                                                    \
		csr.chcsr_chip.chcsr_fld = (chcsr_val);                                            \
		cvmx_write_csr((chcsr_csr), csr.u64);                                              \
	} while (0)

/*
 * CVMX_HELPER_WRITE_CSR0--set a field in a CSR with the initial value of 0
 */
#define CVMX_HELPER_WRITE_CSR0(chcsr_csr, chcsr_type, chcsr_chip, chcsr_fld, chcsr_val)            \
	CVMX_HELPER_WRITE_CSR(CVMX_HELPER_CSR_INIT0, chcsr_csr, chcsr_type, chcsr_chip, chcsr_fld, \
			      chcsr_val)

/*
 * CVMX_HELPER_WRITE_CSR1--set a field in a CSR with the initial value of
 *                      the CSR's current value.
 */
#define CVMX_HELPER_WRITE_CSR1(chcsr_csr, chcsr_type, chcsr_chip, chcsr_fld, chcsr_val)            \
	CVMX_HELPER_WRITE_CSR(CVMX_HELPER_CSR_INIT_READ, chcsr_csr, chcsr_type, chcsr_chip,        \
			      chcsr_fld, chcsr_val)

/* These flags are passed to __cvmx_helper_packet_hardware_enable */

typedef enum {
	CVMX_HELPER_INTERFACE_MODE_DISABLED,
	CVMX_HELPER_INTERFACE_MODE_RGMII,
	CVMX_HELPER_INTERFACE_MODE_GMII,
	CVMX_HELPER_INTERFACE_MODE_SPI,
	CVMX_HELPER_INTERFACE_MODE_PCIE,
	CVMX_HELPER_INTERFACE_MODE_XAUI,
	CVMX_HELPER_INTERFACE_MODE_SGMII,
	CVMX_HELPER_INTERFACE_MODE_PICMG,
	CVMX_HELPER_INTERFACE_MODE_NPI,
	CVMX_HELPER_INTERFACE_MODE_LOOP,
	CVMX_HELPER_INTERFACE_MODE_SRIO,
	CVMX_HELPER_INTERFACE_MODE_ILK,
	CVMX_HELPER_INTERFACE_MODE_RXAUI,
	CVMX_HELPER_INTERFACE_MODE_QSGMII,
	CVMX_HELPER_INTERFACE_MODE_AGL,
	CVMX_HELPER_INTERFACE_MODE_XLAUI,
	CVMX_HELPER_INTERFACE_MODE_XFI,
	CVMX_HELPER_INTERFACE_MODE_10G_KR,
	CVMX_HELPER_INTERFACE_MODE_40G_KR4,
	CVMX_HELPER_INTERFACE_MODE_MIXED,
} cvmx_helper_interface_mode_t;

typedef union cvmx_helper_link_info {
	u64 u64;
	struct {
		u64 reserved_20_63 : 43;
		u64 init_success : 1;
		u64 link_up : 1;
		u64 full_duplex : 1;
		u64 speed : 18;
	} s;
} cvmx_helper_link_info_t;

/**
 * Sets the back pressure configuration in internal data structure.
 * @param backpressure_dis disable/enable backpressure
 */
void cvmx_rgmii_set_back_pressure(u64 backpressure_dis);

#include "cvmx-helper-fpa.h"

#include "cvmx-helper-agl.h"
#include "cvmx-helper-errata.h"
#include "cvmx-helper-ilk.h"
#include "cvmx-helper-loop.h"
#include "cvmx-helper-npi.h"
#include "cvmx-helper-rgmii.h"
#include "cvmx-helper-sgmii.h"
#include "cvmx-helper-spi.h"
#include "cvmx-helper-srio.h"
#include "cvmx-helper-util.h"
#include "cvmx-helper-xaui.h"

#include "cvmx-fpa3.h"

enum cvmx_pko_padding {
	CVMX_PKO_PADDING_NONE = 0,
	CVMX_PKO_PADDING_60 = 1,
};

/**
 * cvmx_override_iface_phy_mode(int interface, int index) is a function pointer.
 * It is meant to allow customization of interfaces which do not have a PHY.
 *
 * @returns 0 if MAC decides TX_CONFIG_REG or 1 if PHY decides  TX_CONFIG_REG.
 *
 * If this function pointer is NULL then it defaults to the MAC.
 */
extern int (*cvmx_override_iface_phy_mode) (int interface, int index);

/**
 * cvmx_override_ipd_port_setup(int ipd_port) is a function
 * pointer. It is meant to allow customization of the IPD port/port kind
 * setup before packet input/output comes online. It is called
 * after cvmx-helper does the default IPD configuration, but
 * before IPD is enabled. Users should set this pointer to a
 * function before calling any cvmx-helper operations.
 */
extern void (*cvmx_override_ipd_port_setup) (int ipd_port);

/**
 * This function enables the IPD and also enables the packet interfaces.
 * The packet interfaces (RGMII and SPI) must be enabled after the
 * IPD.  This should be called by the user program after any additional
 * IPD configuration changes are made if CVMX_HELPER_ENABLE_IPD
 * is not set in the executive-config.h file.
 *
 * Return: 0 on success
 *         -1 on failure
 */
int cvmx_helper_ipd_and_packet_input_enable_node(int node);
int cvmx_helper_ipd_and_packet_input_enable(void);

/**
 * Initialize and allocate memory for the SSO.
 *
 * @param wqe_entries The maximum number of work queue entries to be
 * supported.
 *
 * Return: Zero on success, non-zero on failure.
 */
int cvmx_helper_initialize_sso(int wqe_entries);

/**
 * Initialize and allocate memory for the SSO on a specific node.
 *
 * @param node Node SSO to initialize
 * @param wqe_entries The maximum number of work queue entries to be
 * supported.
 *
 * Return: Zero on success, non-zero on failure.
 */
int cvmx_helper_initialize_sso_node(unsigned int node, int wqe_entries);

/**
 * Undo the effect of cvmx_helper_initialize_sso().
 *
 * Return: Zero on success, non-zero on failure.
 */
int cvmx_helper_uninitialize_sso(void);

/**
 * Undo the effect of cvmx_helper_initialize_sso_node().
 *
 * @param node Node SSO to initialize
 *
 * Return: Zero on success, non-zero on failure.
 */
int cvmx_helper_uninitialize_sso_node(unsigned int node);

/**
 * Initialize the PIP, IPD, and PKO hardware to support
 * simple priority based queues for the ethernet ports. Each
 * port is configured with a number of priority queues based
 * on CVMX_PKO_QUEUES_PER_PORT_* where each queue is lower
 * priority than the previous.
 *
 * Return: Zero on success, non-zero on failure
 */
int cvmx_helper_initialize_packet_io_global(void);
/**
 * Initialize the PIP, IPD, and PKO hardware to support
 * simple priority based queues for the ethernet ports. Each
 * port is configured with a number of priority queues based
 * on CVMX_PKO_QUEUES_PER_PORT_* where each queue is lower
 * priority than the previous.
 *
 * @param node Node on which to initialize packet io hardware
 *
 * Return: Zero on success, non-zero on failure
 */
int cvmx_helper_initialize_packet_io_node(unsigned int node);

/**
 * Does core local initialization for packet io
 *
 * Return: Zero on success, non-zero on failure
 */
int cvmx_helper_initialize_packet_io_local(void);

/**
 * Undo the initialization performed in
 * cvmx_helper_initialize_packet_io_global(). After calling this routine and the
 * local version on each core, packet IO for Octeon will be disabled and placed
 * in the initial reset state. It will then be safe to call the initialize
 * later on. Note that this routine does not empty the FPA pools. It frees all
 * buffers used by the packet IO hardware to the FPA so a function emptying the
 * FPA after shutdown should find all packet buffers in the FPA.
 *
 * Return: Zero on success, negative on failure.
 */
int cvmx_helper_shutdown_packet_io_global(void);

/**
 * Helper function for 78xx global packet IO shutdown
 */
int cvmx_helper_shutdown_packet_io_global_cn78xx(int node);

/**
 * Does core local shutdown of packet io
 *
 * Return: Zero on success, non-zero on failure
 */
int cvmx_helper_shutdown_packet_io_local(void);

/**
 * Returns the number of ports on the given interface.
 * The interface must be initialized before the port count
 * can be returned.
 *
 * @param interface Which interface to return port count for.
 *
 * Return: Port count for interface
 *         -1 for uninitialized interface
 */
int cvmx_helper_ports_on_interface(int interface);

/**
 * Return the number of interfaces the chip has. Each interface
 * may have multiple ports. Most chips support two interfaces,
 * but the CNX0XX and CNX1XX are exceptions. These only support
 * one interface.
 *
 * Return: Number of interfaces on chip
 */
int cvmx_helper_get_number_of_interfaces(void);

/**
 * Get the operating mode of an interface. Depending on the Octeon
 * chip and configuration, this function returns an enumeration
 * of the type of packet I/O supported by an interface.
 *
 * @param xiface Interface to probe
 *
 * Return: Mode of the interface. Unknown or unsupported interfaces return
 *         DISABLED.
 */
cvmx_helper_interface_mode_t cvmx_helper_interface_get_mode(int xiface);

/**
 * Auto configure an IPD/PKO port link state and speed. This
 * function basically does the equivalent of:
 * cvmx_helper_link_set(ipd_port, cvmx_helper_link_get(ipd_port));
 *
 * @param ipd_port IPD/PKO port to auto configure
 *
 * Return: Link state after configure
 */
cvmx_helper_link_info_t cvmx_helper_link_autoconf(int ipd_port);

/**
 * Return the link state of an IPD/PKO port as returned by
 * auto negotiation. The result of this function may not match
 * Octeon's link config if auto negotiation has changed since
 * the last call to cvmx_helper_link_set().
 *
 * @param ipd_port IPD/PKO port to query
 *
 * Return: Link state
 */
cvmx_helper_link_info_t cvmx_helper_link_get(int ipd_port);

/**
 * Configure an IPD/PKO port for the specified link state. This
 * function does not influence auto negotiation at the PHY level.
 * The passed link state must always match the link state returned
 * by cvmx_helper_link_get(). It is normally best to use
 * cvmx_helper_link_autoconf() instead.
 *
 * @param ipd_port  IPD/PKO port to configure
 * @param link_info The new link state
 *
 * Return: Zero on success, negative on failure
 */
int cvmx_helper_link_set(int ipd_port, cvmx_helper_link_info_t link_info);

/**
 * This function probes an interface to determine the actual number of
 * hardware ports connected to it. It does some setup the ports but
 * doesn't enable them. The main goal here is to set the global
 * interface_port_count[interface] correctly. Final hardware setup of
 * the ports will be performed later.
 *
 * @param xiface Interface to probe
 *
 * Return: Zero on success, negative on failure
 */
int cvmx_helper_interface_probe(int xiface);

/**
 * Determine the actual number of hardware ports connected to an
 * interface. It doesn't setup the ports or enable them.
 *
 * @param xiface Interface to enumerate
 *
 * Return: Zero on success, negative on failure
 */
int cvmx_helper_interface_enumerate(int xiface);

/**
 * Configure a port for internal and/or external loopback. Internal loopback
 * causes packets sent by the port to be received by Octeon. External loopback
 * causes packets received from the wire to sent out again.
 *
 * @param ipd_port IPD/PKO port to loopback.
 * @param enable_internal
 *                 Non zero if you want internal loopback
 * @param enable_external
 *                 Non zero if you want external loopback
 *
 * Return: Zero on success, negative on failure.
 */
int cvmx_helper_configure_loopback(int ipd_port, int enable_internal, int enable_external);

/**
 * Returns the number of ports on the given interface.
 *
 * @param interface Which interface to return port count for.
 *
 * Return: Port count for interface
 *         -1 for uninitialized interface
 */
int __cvmx_helper_early_ports_on_interface(int interface);

void cvmx_helper_setup_simulator_io_buffer_counts(int node, int num_packet_buffers,
						  int pko_buffers);

void cvmx_helper_set_wqe_no_ptr_mode(bool mode);
void cvmx_helper_set_pkt_wqe_le_mode(bool mode);
int cvmx_helper_shutdown_fpa_pools(int node);

/**
 * Convert Ethernet QoS/PCP value to system-level priority
 *
 * In OCTEON, highest priority is 0, in Ethernet 802.1p PCP field
 * the highest priority is 7, lowest is 1. Here is the full conversion
 * table between QoS (PCP) and OCTEON priority values, per IEEE 802.1Q-2005:
 *
 * PCP	Priority	Acronym	Traffic Types
 * 1	7 (lowest)	BK	Background
 * 0	6	BE	Best Effort
 * 2	5	EE	Excellent Effort
 * 3	4	CA	Critical Applications
 * 4	3	VI	Video, < 100 ms latency and jitter
 * 5	2	VO	Voice, < 10 ms latency and jitter
 * 6	1	IC	Internetwork Control
 * 7	0 (highest)	NC	Network Control
 */
static inline u8 cvmx_helper_qos2prio(u8 qos)
{
	static const unsigned int pcp_map = 6 << (4 * 0) | 7 << (4 * 1) | 5 << (4 * 2) |
					    4 << (4 * 3) | 3 << (4 * 4) | 2 << (4 * 5) |
					    1 << (4 * 6) | 0 << (4 * 7);

	return (pcp_map >> ((qos & 0x7) << 2)) & 0x7;
}

/**
 * Convert system-level priority to Ethernet QoS/PCP value
 *
 * Calculate the reverse of cvmx_helper_qos2prio() per IEEE 802.1Q-2005.
 */
static inline u8 cvmx_helper_prio2qos(u8 prio)
{
	static const unsigned int prio_map = 7 << (4 * 0) | 6 << (4 * 1) | 5 << (4 * 2) |
					     4 << (4 * 3) | 3 << (4 * 4) | 2 << (4 * 5) |
					     0 << (4 * 6) | 1 << (4 * 7);

	return (prio_map >> ((prio & 0x7) << 2)) & 0x7;
}

/**
 * @INTERNAL
 * Get the number of ipd_ports on an interface.
 *
 * @param xiface
 *
 * Return: the number of ipd_ports on the interface and -1 for error.
 */
int __cvmx_helper_get_num_ipd_ports(int xiface);

enum cvmx_pko_padding __cvmx_helper_get_pko_padding(int xiface);

/**
 * @INTERNAL
 *
 * @param xiface
 * @param num_ipd_ports is the number of ipd_ports on the interface
 * @param has_fcs indicates if PKO does FCS for the ports on this
 * @param pad The padding that PKO should apply.
 * interface.
 *
 * Return: 0 for success and -1 for failure
 */
int __cvmx_helper_init_interface(int xiface, int num_ipd_ports, int has_fcs,
				 enum cvmx_pko_padding pad);

void __cvmx_helper_shutdown_interfaces(void);

/*
 * @INTERNAL
 * Enable packet input/output from the hardware. This function is
 * called after all internal setup is complete and IPD is enabled.
 * After this function completes, packets will be accepted from the
 * hardware ports. PKO should still be disabled to make sure packets
 * aren't sent out partially setup hardware.
 *
 * Return: Zero on success, negative on failure
 */
int __cvmx_helper_packet_hardware_enable(int xiface);

/*
 * @INTERNAL
 *
 * Return: 0 for success and -1 for failure
 */
int __cvmx_helper_set_link_info(int xiface, int index, cvmx_helper_link_info_t link_info);

/**
 * @INTERNAL
 *
 * @param xiface
 * @param port
 *
 * Return: valid link_info on success or -1 on failure
 */
cvmx_helper_link_info_t __cvmx_helper_get_link_info(int xiface, int port);

/**
 * @INTERNAL
 *
 * @param xiface
 *
 * Return: 0 if PKO does not do FCS and 1 otherwise.
 */
int __cvmx_helper_get_has_fcs(int xiface);

void *cvmx_helper_mem_alloc(int node, u64 alloc_size, u64 align);
void cvmx_helper_mem_free(void *buffer, u64 size);

#define CVMX_QOS_NUM 8 /* Number of QoS priority classes */

typedef enum {
	CVMX_QOS_PROTO_NONE,  /* Disable QOS */
	CVMX_QOS_PROTO_PAUSE, /* IEEE 802.3 PAUSE */
	CVMX_QOS_PROTO_PFC    /* IEEE 802.1Qbb-2011 PFC/CBFC */
} cvmx_qos_proto_t;

typedef enum {
	CVMX_QOS_PKT_MODE_HWONLY, /* PAUSE packets processed in Hardware only. */
	CVMX_QOS_PKT_MODE_SWONLY, /* PAUSE packets processed in Software only. */
	CVMX_QOS_PKT_MODE_HWSW,	  /* PAUSE packets processed in both HW and SW. */
	CVMX_QOS_PKT_MODE_DROP	  /* Ignore PAUSE packets. */
} cvmx_qos_pkt_mode_t;

typedef enum {
	CVMX_QOS_POOL_PER_PORT, /* Pool per Physical Port */
	CVMX_QOS_POOL_PER_CLASS /* Pool per Priority Class */
} cvmx_qos_pool_mode_t;

typedef struct cvmx_qos_config {
	cvmx_qos_proto_t qos_proto;	/* QoS protocol.*/
	cvmx_qos_pkt_mode_t pkt_mode;	/* PAUSE processing mode.*/
	cvmx_qos_pool_mode_t pool_mode; /* FPA Pool mode.*/
	int pktbuf_size;		/* Packet buffer size */
	int aura_size;			/* Number of buffers */
	int drop_thresh[CVMX_QOS_NUM];	/* DROP threashold in % */
	int red_thresh[CVMX_QOS_NUM];	/* RED threashold in % */
	int bp_thresh[CVMX_QOS_NUM];	/* BP threashold in % */
	int groups[CVMX_QOS_NUM];	/* Base SSO group for QOS group set. */
	int group_prio[CVMX_QOS_NUM];	/* SSO group priorities.*/
	int pko_pfc_en;			/* Enable PKO PFC layout. */
	int vlan_num;			/* VLAN number: 0 = 1st or 1 = 2nd. */
	int p_time;			/* PAUSE packets send time (in number of 512 bit-times).*/
	int p_interval; /* PAUSE packet send interval (in number of 512 bit-times).*/
	/* Internal parameters (should not be used by application developer): */
	cvmx_fpa3_pool_t gpools[CVMX_QOS_NUM];	/* Pool to use.*/
	cvmx_fpa3_gaura_t gauras[CVMX_QOS_NUM]; /* Global auras -- one per priority class. */
	int bpids[CVMX_QOS_NUM];		/* PKI BPID.*/
	int qpg_base;				/* QPG Table base index.*/
} cvmx_qos_config_t;

/**
 * Initialize QoS configuraiton with the SDK defaults.
 *
 * @param qos_cfg   User QOS configuration parameters.
 * Return: Zero on success, negative number otherwise.
 */
int cvmx_helper_qos_config_init(cvmx_qos_proto_t qos_proto, cvmx_qos_config_t *qos_cfg);

/**
 * Update the user static processor configuration.
 * It should be done before any initialization of the DP units is performed.
 *
 * @param xipdport  Global IPD port
 * @param qos_cfg   User QOS configuration parameters.
 * Return: Zero on success, negative number otherwise.
 */
int cvmx_helper_qos_port_config_update(int xipdport, cvmx_qos_config_t *qos_cfg);

/**
 * Configure the Data Path components for QOS function.
 * This function is called after the global processor initialization is
 * performed.
 *
 * @param xipdport  Global IPD port
 * @param qos_cfg   User QOS configuration parameters.
 * Return: Zero on success, negative number otherwise.
 */
int cvmx_helper_qos_port_setup(int xipdport, cvmx_qos_config_t *qos_cfg);

/**
 * Configure the SSO for QOS function.
 * This function is called after the global processor initialization is
 * performed.
 *
 * @param node      OCTEON3 node number.
 * @param qos_cfg   User QOS configuration parameters.
 * Return: Zero on success, negative number otherwise.
 */
int cvmx_helper_qos_sso_setup(int node, cvmx_qos_config_t *qos_cfg);

/**
 * Return PKI_CHAN_E channel name based on the provided index.
 * @param chan     Channel index.
 * @param namebuf  Name buffer (output).
 * @param buflen   Name maximum length.
 * Return: Length of name (in bytes) on success, negative number otherwise.
 */
int cvmx_helper_get_chan_e_name(int chan, char *namebuf, int buflen);

#ifdef CVMX_DUMP_DIAGNOSTICS
void cvmx_helper_dump_for_diagnostics(int node);
#endif

#endif /* __CVMX_HELPER_H__ */
