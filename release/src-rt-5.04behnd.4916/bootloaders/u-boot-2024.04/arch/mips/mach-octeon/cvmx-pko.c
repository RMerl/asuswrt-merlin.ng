// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2018-2022 Marvell International Ltd.
 *
 * Support library for the hardware Packet Output unit.
 */

#include <errno.h>
#include <log.h>
#include <time.h>
#include <linux/delay.h>

#include <mach/cvmx-regs.h>
#include <mach/cvmx-csr.h>
#include <mach/cvmx-bootmem.h>
#include <mach/octeon-model.h>
#include <mach/cvmx-fuse.h>
#include <mach/octeon-feature.h>
#include <mach/cvmx-qlm.h>
#include <mach/octeon_qlm.h>
#include <mach/cvmx-pcie.h>
#include <mach/cvmx-coremask.h>

#include <mach/cvmx-agl-defs.h>
#include <mach/cvmx-bgxx-defs.h>
#include <mach/cvmx-ciu-defs.h>
#include <mach/cvmx-gmxx-defs.h>
#include <mach/cvmx-gserx-defs.h>
#include <mach/cvmx-ilk-defs.h>
#include <mach/cvmx-iob-defs.h>
#include <mach/cvmx-ipd-defs.h>
#include <mach/cvmx-pcsx-defs.h>
#include <mach/cvmx-pcsxx-defs.h>
#include <mach/cvmx-pki-defs.h>
#include <mach/cvmx-pko-defs.h>
#include <mach/cvmx-xcv-defs.h>

#include <mach/cvmx-hwpko.h>
#include <mach/cvmx-ilk.h>
#include <mach/cvmx-pki.h>
#include <mach/cvmx-pko3.h>
#include <mach/cvmx-pko3-queue.h>
#include <mach/cvmx-pko3-resources.h>

#include <mach/cvmx-helper.h>
#include <mach/cvmx-helper-board.h>
#include <mach/cvmx-helper-cfg.h>

#include <mach/cvmx-helper-bgx.h>
#include <mach/cvmx-helper-cfg.h>
#include <mach/cvmx-helper-util.h>
#include <mach/cvmx-helper-pki.h>
#include <mach/cvmx-helper-pko.h>

DECLARE_GLOBAL_DATA_PTR;

#define CVMX_PKO_NQ_PER_PORT_MAX 32

static cvmx_pko_return_value_t cvmx_pko2_config_port(short ipd_port,
						     int base_queue,
						     int num_queues,
						     const u8 priority[]);

static const int debug;

/**
 * Internal state of packet output
 */

/*
 * PKO port iterator
 * XXX this macro only works for 68XX
 */

#define pko_for_each_port(__p)                                                 \
	for (__p = 0; __p < CVMX_HELPER_CFG_MAX_PKO_PORT; __p++)               \
		if (__cvmx_helper_cfg_pko_queue_base(__p) !=                   \
		    CVMX_HELPER_CFG_INVALID_VALUE)

/*
 * @INTERNAL
 *
 * Get INT for a port
 *
 * @param interface
 * @param index
 * @return the INT value on success and -1 on error
 *
 * This function is only for CN68XX.
 */
static int __cvmx_pko_int(int interface, int index)
{
	cvmx_helper_cfg_assert(interface < CVMX_HELPER_MAX_IFACE);
	cvmx_helper_cfg_assert(index >= 0);

	switch (interface) {
	case 0:
		cvmx_helper_cfg_assert(index < 4);
		return index;
	case 1:
		cvmx_helper_cfg_assert(index == 0);
		return 4;
	case 2:
		cvmx_helper_cfg_assert(index < 4);
		return index + 8;
	case 3:
		cvmx_helper_cfg_assert(index < 4);
		return index + 0xC;
	case 4:
		cvmx_helper_cfg_assert(index < 4);
		return index + 0x10;
	case 5:
		cvmx_helper_cfg_assert(index < 256);
		return 0x1C;
	case 6:
		cvmx_helper_cfg_assert(index < 256);
		return 0x1D;
	case 7:
		cvmx_helper_cfg_assert(index < 32);
		return 0x1E;
	case 8:
		cvmx_helper_cfg_assert(index < 8);
		return 0x1F;
	}

	return -1;
}

int cvmx_pko_get_base_pko_port(int interface, int index)
{
	if (octeon_has_feature(OCTEON_FEATURE_CN78XX_WQE))
		return cvmx_helper_get_ipd_port(interface, index);
	else if (octeon_has_feature(OCTEON_FEATURE_PKND))
		return __cvmx_helper_cfg_pko_port_base(interface, index);
	else
		return cvmx_helper_get_ipd_port(interface, index);
}

int cvmx_pko_get_base_queue(int port)
{
	if (octeon_has_feature(OCTEON_FEATURE_CN78XX_WQE)) {
		return cvmx_pko3_get_queue_base(port);
	} else if (octeon_has_feature(OCTEON_FEATURE_PKND)) {
		return __cvmx_helper_cfg_pko_queue_base(
			cvmx_helper_cfg_ipd2pko_port_base(port));
	} else {
		if (port < 48)
			return cvmx_pko_queue_table[port].ccppp_queue_base;
		else
			return CVMX_PKO_ILLEGAL_QUEUE;
	}
}

int cvmx_pko_get_num_queues(int port)
{
	if (octeon_has_feature(OCTEON_FEATURE_CN78XX_WQE)) {
		return cvmx_pko3_get_queue_num(port);
	} else if (octeon_has_feature(OCTEON_FEATURE_PKND)) {
		return __cvmx_helper_cfg_pko_queue_num(
			cvmx_helper_cfg_ipd2pko_port_base(port));
	} else {
		if (port < 48)
			return cvmx_pko_queue_table[port].ccppp_num_queues;
	}
	return 0;
}

/*
 * Allocate memory for PKO engines.
 *
 * @param engine is the PKO engine ID.
 * @return # of 2KB-chunks allocated to this PKO engine.
 */
static int __cvmx_pko_memory_per_engine_o68(int engine)
{
	/* CN68XX has 40KB to devide between the engines in 2KB chunks */
	int max_engine;
	int size_per_engine;
	int size;

	max_engine = __cvmx_helper_cfg_pko_max_engine();
	size_per_engine = 40 / 2 / max_engine;

	if (engine >= max_engine)
		/* Unused engines get no space */
		size = 0;
	else if (engine == max_engine - 1)
		/*
		 * The last engine gets all the space lost by rounding. This means
		 * the ILK gets the most space
		 */
		size = 40 / 2 - engine * size_per_engine;
	else
		/* All other engines get the same space */
		size = size_per_engine;

	return size;
}

/*
 * Setup one-to-one mapping between PKO2 iport and eport.
 * @INTERNAL
 */
static void __cvmx_pko2_chip_init(void)
{
	int i;
	int interface, index, port;
	cvmx_helper_interface_mode_t mode;
	union cvmx_pko_mem_iport_ptrs config;

	/*
	 * Initialize every iport with the invalid eid.
	 */
#define CVMX_O68_PKO2_INVALID_EID 31
	config.u64 = 0;
	config.s.eid = CVMX_O68_PKO2_INVALID_EID;
	for (i = 0; i < CVMX_HELPER_CFG_MAX_PKO_PORT; i++) {
		config.s.ipid = i;
		csr_wr(CVMX_PKO_MEM_IPORT_PTRS, config.u64);
	}

	/*
	 * Set up PKO_MEM_IPORT_PTRS
	 */
	pko_for_each_port(port) {
		interface = __cvmx_helper_cfg_pko_port_interface(port);
		index = __cvmx_helper_cfg_pko_port_index(port);
		mode = cvmx_helper_interface_get_mode(interface);

		if (mode == CVMX_HELPER_INTERFACE_MODE_DISABLED)
			continue;

		config.s.ipid = port;
		config.s.qos_mask = 0xff;
		config.s.crc = __cvmx_helper_get_has_fcs(interface);
		config.s.min_pkt = __cvmx_helper_get_pko_padding(interface);
		config.s.intr = __cvmx_pko_int(interface, index);
		config.s.eid = __cvmx_helper_cfg_pko_port_eid(port);
		config.s.pipe = (mode == CVMX_HELPER_INTERFACE_MODE_LOOP) ?
					index :
					      port;
		csr_wr(CVMX_PKO_MEM_IPORT_PTRS, config.u64);
	}
}

int __cvmx_pko_get_pipe(int interface, int index)
{
	/* The loopback ports do not have pipes */
	if (cvmx_helper_interface_get_mode(interface) ==
	    CVMX_HELPER_INTERFACE_MODE_LOOP)
		return -1;
	/* We use pko_port as the pipe. See __cvmx_pko_port_map_o68(). */
	return cvmx_helper_get_pko_port(interface, index);
}

static void __cvmx_pko1_chip_init(void)
{
	int queue;
	union cvmx_pko_mem_queue_ptrs config;
	union cvmx_pko_reg_queue_ptrs1 config1;
	const int port = CVMX_PKO_MEM_QUEUE_PTRS_ILLEGAL_PID;

	/* Initialize all queues to connect to port 63 (ILLEGAL_PID) */
	for (queue = 0; queue < CVMX_PKO_MAX_OUTPUT_QUEUES; queue++) {
		config1.u64 = 0;
		config1.s.idx3 = 0;
		config1.s.qid7 = queue >> 7;

		config.u64 = 0;
		config.s.tail = 1;
		config.s.index = 0;
		config.s.port = port;
		config.s.queue = queue;
		config.s.buf_ptr = 0;

		csr_wr(CVMX_PKO_REG_QUEUE_PTRS1, config1.u64);
		csr_wr(CVMX_PKO_MEM_QUEUE_PTRS, config.u64);
	}
}

/**
 * Call before any other calls to initialize the packet
 * output system.  This does chip global config, and should only be
 * done by one core.
 */
void cvmx_pko_hw_init(u8 pool, unsigned int bufsize)
{
	union cvmx_pko_reg_cmd_buf config;
	union cvmx_iob_fau_timeout fau_to;
	int i;

	if (debug)
		debug("%s: pool=%u bufsz=%u\n", __func__, pool, bufsize);

	/* chip-specific setup. */
	if (OCTEON_IS_MODEL(OCTEON_CN68XX))
		__cvmx_pko2_chip_init();
	else
		__cvmx_pko1_chip_init();

	/*
	 * Set the size of the PKO command buffers to an odd number of
	 * 64bit words. This allows the normal two word send to stay
	 * aligned and never span a command word buffer.
	 */
	config.u64 = 0;
	config.s.pool = pool;
	config.s.size = bufsize / 8 - 1;
	csr_wr(CVMX_PKO_REG_CMD_BUF, config.u64);

	/*
	 * Disable tagwait FAU timeout. This needs to be done before
	 * anyone might start packet output using tags.
	 */
	fau_to.u64 = 0;
	fau_to.s.tout_val = 0xfff;
	fau_to.s.tout_enb = 0;
	csr_wr(CVMX_IOB_FAU_TIMEOUT, fau_to.u64);

	if (OCTEON_IS_MODEL(OCTEON_CN68XX)) {
		union cvmx_pko_reg_min_pkt min_pkt;

		min_pkt.u64 = 0;
		min_pkt.s.size1 = 59;
		min_pkt.s.size2 = 59;
		min_pkt.s.size3 = 59;
		min_pkt.s.size4 = 59;
		min_pkt.s.size5 = 59;
		min_pkt.s.size6 = 59;
		min_pkt.s.size7 = 59;
		csr_wr(CVMX_PKO_REG_MIN_PKT, min_pkt.u64);
	}

	/*
	 * If we aren't using all of the queues optimize PKO's
	 * internal memory.
	 */
	if (OCTEON_IS_OCTEON2() || OCTEON_IS_MODEL(OCTEON_CN70XX)) {
		int max_queues = __cvmx_helper_cfg_pko_max_queue();

		if (OCTEON_IS_MODEL(OCTEON_CN68XX) && max_queues <= 32)
			csr_wr(CVMX_PKO_REG_QUEUE_MODE, 3);
		else if (max_queues <= 64)
			csr_wr(CVMX_PKO_REG_QUEUE_MODE, 2);
		else if (max_queues <= 128)
			csr_wr(CVMX_PKO_REG_QUEUE_MODE, 1);
		else
			csr_wr(CVMX_PKO_REG_QUEUE_MODE, 0);
		if (OCTEON_IS_MODEL(OCTEON_CN68XX)) {
			for (i = 0; i < 2; i++) {
				union cvmx_pko_reg_engine_storagex
					engine_storage;

#define PKO_ASSIGN_ENGINE_STORAGE(index)                                       \
	engine_storage.s.engine##index =                                       \
		__cvmx_pko_memory_per_engine_o68(16 * i + (index))

				engine_storage.u64 = 0;
				PKO_ASSIGN_ENGINE_STORAGE(0);
				PKO_ASSIGN_ENGINE_STORAGE(1);
				PKO_ASSIGN_ENGINE_STORAGE(2);
				PKO_ASSIGN_ENGINE_STORAGE(3);
				PKO_ASSIGN_ENGINE_STORAGE(4);
				PKO_ASSIGN_ENGINE_STORAGE(5);
				PKO_ASSIGN_ENGINE_STORAGE(6);
				PKO_ASSIGN_ENGINE_STORAGE(7);
				PKO_ASSIGN_ENGINE_STORAGE(8);
				PKO_ASSIGN_ENGINE_STORAGE(9);
				PKO_ASSIGN_ENGINE_STORAGE(10);
				PKO_ASSIGN_ENGINE_STORAGE(11);
				PKO_ASSIGN_ENGINE_STORAGE(12);
				PKO_ASSIGN_ENGINE_STORAGE(13);
				PKO_ASSIGN_ENGINE_STORAGE(14);
				PKO_ASSIGN_ENGINE_STORAGE(15);
				csr_wr(CVMX_PKO_REG_ENGINE_STORAGEX(i),
				       engine_storage.u64);
			}
		}
	}
}

/**
 * Enables the packet output hardware. It must already be
 * configured.
 */
void cvmx_pko_enable(void)
{
	union cvmx_pko_reg_flags flags;

	flags.u64 = csr_rd(CVMX_PKO_REG_FLAGS);
	if (flags.s.ena_pko)
		debug("Warning: Enabling PKO when PKO already enabled.\n");

	flags.s.ena_dwb = cvmx_helper_cfg_opt_get(CVMX_HELPER_CFG_OPT_USE_DWB);
	flags.s.ena_pko = 1;
	/*
	 * always enable big endian for 3-word command.  Does nothing
	 * for 2-word.
	 */
	flags.s.store_be = 1;
	csr_wr(CVMX_PKO_REG_FLAGS, flags.u64);
}

/**
 * Configure a output port and the associated queues for use.
 *
 * @param port       Port to configure.
 * @param base_queue First queue number to associate with this port.
 * @param num_queues Number of queues to associate with this port
 * @param priority   Array of priority levels for each queue. Values are
 *                   allowed to be 0-8. A value of 8 get 8 times the traffic
 *                   of a value of 1.  A value of 0 indicates that no rounds
 *                   will be participated in. These priorities can be changed
 *                   on the fly while the pko is enabled. A priority of 9
 *                   indicates that static priority should be used.  If static
 *                   priority is used all queues with static priority must be
 *                   contiguous starting at the base_queue, and lower numbered
 *                   queues have higher priority than higher numbered queues.
 *                   There must be num_queues elements in the array.
 */
cvmx_pko_return_value_t cvmx_pko_config_port(int port, int base_queue,
					     int num_queues,
					     const u8 priority[])
{
	cvmx_pko_return_value_t result_code;
	int queue;
	union cvmx_pko_mem_queue_ptrs config;
	union cvmx_pko_reg_queue_ptrs1 config1;
	int static_priority_base = -1;
	int static_priority_end = -1;
	int outputbuffer_pool = (int)cvmx_fpa_get_pko_pool();
	u64 outputbuffer_pool_size = cvmx_fpa_get_pko_pool_block_size();

	/* This function is not used for CN68XX */
	if (OCTEON_IS_MODEL(OCTEON_CN68XX))
		return cvmx_pko2_config_port(port, base_queue, num_queues,
					     priority);

	if (debug)
		debug("%s: port=%d queue=%d-%d pri %#x %#x %#x %#x\n", __func__,
		      port, base_queue, (base_queue + num_queues - 1),
		      priority[0], priority[1], priority[2], priority[3]);

	/* The need to handle ILLEGAL_PID port argument
	 * is obsolete now, the code here can be simplified.
	 */

	if (port >= CVMX_PKO_NUM_OUTPUT_PORTS &&
	    port != CVMX_PKO_MEM_QUEUE_PTRS_ILLEGAL_PID) {
		debug("ERROR: %s: Invalid port %llu\n", __func__,
		      (unsigned long long)port);
		return CVMX_PKO_INVALID_PORT;
	}

	if (base_queue + num_queues > CVMX_PKO_MAX_OUTPUT_QUEUES) {
		debug("ERROR: %s: Invalid queue range port = %lld base=%llu numques=%lld\n",
		      __func__, (unsigned long long)port,
		      (unsigned long long)base_queue,
		      (unsigned long long)num_queues);
		return CVMX_PKO_INVALID_QUEUE;
	}

	if (port != CVMX_PKO_MEM_QUEUE_PTRS_ILLEGAL_PID) {
		/*
		 * Validate the static queue priority setup and set
		 * static_priority_base and static_priority_end
		 * accordingly.
		 */
		for (queue = 0; queue < num_queues; queue++) {
			/* Find first queue of static priority */
			int p_queue = queue % 16;

			if (static_priority_base == -1 &&
			    priority[p_queue] == CVMX_PKO_QUEUE_STATIC_PRIORITY)
				static_priority_base = queue;
			/* Find last queue of static priority */
			if (static_priority_base != -1 &&
			    static_priority_end == -1 &&
			    priority[p_queue] !=
				    CVMX_PKO_QUEUE_STATIC_PRIORITY &&
			    queue)
				static_priority_end = queue - 1;
			else if (static_priority_base != -1 &&
				 static_priority_end == -1 &&
				 queue == num_queues - 1)
				/* all queues're static priority */
				static_priority_end = queue;

			/*
			 * Check to make sure all static priority
			 * queues are contiguous.  Also catches some
			 * cases of static priorites not starting at
			 * queue 0.
			 */
			if (static_priority_end != -1 &&
			    (int)queue > static_priority_end &&
			    priority[p_queue] ==
				    CVMX_PKO_QUEUE_STATIC_PRIORITY) {
				debug("ERROR: %s: Static priority queues aren't contiguous or don't start at base queue. q: %d, eq: %d\n",
				      __func__, (int)queue, static_priority_end);
				return CVMX_PKO_INVALID_PRIORITY;
			}
		}
		if (static_priority_base > 0) {
			debug("ERROR: %s: Static priority queues don't start at base queue. sq: %d\n",
			      __func__, static_priority_base);
			return CVMX_PKO_INVALID_PRIORITY;
		}
	}

	/*
	 * At this point, static_priority_base and static_priority_end
	 * are either both -1, or are valid start/end queue numbers
	 */

	result_code = CVMX_PKO_SUCCESS;

	for (queue = 0; queue < num_queues; queue++) {
		u64 *buf_ptr = NULL;
		int p_queue = queue % 16;

		config1.u64 = 0;
		config1.s.idx3 = queue >> 3;
		config1.s.qid7 = (base_queue + queue) >> 7;

		config.u64 = 0;
		config.s.tail = queue == (num_queues - 1);
		config.s.index = queue;
		config.s.port = port;
		config.s.queue = base_queue + queue;

		config.s.static_p = static_priority_base >= 0;
		config.s.static_q = (int)queue <= static_priority_end;
		config.s.s_tail = (int)queue == static_priority_end;
		/*
		 * Convert the priority into an enable bit field. Try
		 * to space the bits out evenly so the packet don't
		 * get grouped up.
		 */
		switch ((int)priority[p_queue]) {
		case 0:
			config.s.qos_mask = 0x00;
			break;
		case 1:
			config.s.qos_mask = 0x01;
			break;
		case 2:
			config.s.qos_mask = 0x11;
			break;
		case 3:
			config.s.qos_mask = 0x49;
			break;
		case 4:
			config.s.qos_mask = 0x55;
			break;
		case 5:
			config.s.qos_mask = 0x57;
			break;
		case 6:
			config.s.qos_mask = 0x77;
			break;
		case 7:
			config.s.qos_mask = 0x7f;
			break;
		case 8:
			config.s.qos_mask = 0xff;
			break;
		case CVMX_PKO_QUEUE_STATIC_PRIORITY:
			config.s.qos_mask = 0xff;
			break;
		default:
			debug("ERROR: %s: Invalid priority %llu\n", __func__,
			      (unsigned long long)priority[p_queue]);
			config.s.qos_mask = 0xff;
			result_code = CVMX_PKO_INVALID_PRIORITY;
			break;
		}

		if (port != CVMX_PKO_MEM_QUEUE_PTRS_ILLEGAL_PID) {
			cvmx_cmd_queue_result_t cmd_res;

			cmd_res = cvmx_cmd_queue_initialize(
				CVMX_CMD_QUEUE_PKO(base_queue + queue),
				CVMX_PKO_MAX_QUEUE_DEPTH, outputbuffer_pool,
				outputbuffer_pool_size -
					CVMX_PKO_COMMAND_BUFFER_SIZE_ADJUST *
						8);
			if (cmd_res != CVMX_CMD_QUEUE_SUCCESS) {
				switch (cmd_res) {
				case CVMX_CMD_QUEUE_NO_MEMORY:
					debug("ERROR: %s: Unable to allocate output buffer\n",
					      __func__);
					return CVMX_PKO_NO_MEMORY;
				case CVMX_CMD_QUEUE_ALREADY_SETUP:
					debug("ERROR: %s: Port already setup. port=%d\n",
					      __func__, (int)port);
					return CVMX_PKO_PORT_ALREADY_SETUP;
				case CVMX_CMD_QUEUE_INVALID_PARAM:
				default:
					debug("ERROR: %s: Command queue initialization failed.\n",
					      __func__);
					return CVMX_PKO_CMD_QUEUE_INIT_ERROR;
				}
			}

			buf_ptr = (u64 *)cvmx_cmd_queue_buffer(
				CVMX_CMD_QUEUE_PKO(base_queue + queue));
			config.s.buf_ptr = cvmx_ptr_to_phys(buf_ptr);
		} else {
			config.s.buf_ptr = 0;
		}

		CVMX_SYNCWS;

		csr_wr(CVMX_PKO_REG_QUEUE_PTRS1, config1.u64);
		csr_wr(CVMX_PKO_MEM_QUEUE_PTRS, config.u64);
	}

	return result_code;
}

/*
 * Configure queues for an internal port.
 * @INTERNAL
 * @param pko_port PKO internal port number
 * @note this is the PKO2 equivalent to cvmx_pko_config_port()
 */
static cvmx_pko_return_value_t cvmx_pko2_config_port(short ipd_port,
						     int base_queue,
						     int num_queues,
						     const u8 priority[])
{
	int queue, pko_port;
	int static_priority_base;
	int static_priority_end;
	union cvmx_pko_mem_iqueue_ptrs config;
	u64 *buf_ptr = NULL;
	int outputbuffer_pool = (int)cvmx_fpa_get_pko_pool();
	u64 outputbuffer_pool_size = cvmx_fpa_get_pko_pool_block_size();

	pko_port = cvmx_helper_cfg_ipd2pko_port_base(ipd_port);

	if (debug)
		debug("%s: ipd_port %d pko_iport %d qbase %d qnum %d\n",
		      __func__, ipd_port, pko_port, base_queue, num_queues);

	static_priority_base = -1;
	static_priority_end = -1;

	/*
	 * static queue priority validation
	 */
	for (queue = 0; queue < num_queues; queue++) {
		int p_queue = queue % 16;

		if (static_priority_base == -1 &&
		    priority[p_queue] == CVMX_PKO_QUEUE_STATIC_PRIORITY)
			static_priority_base = queue;

		if (static_priority_base != -1 && static_priority_end == -1 &&
		    priority[p_queue] != CVMX_PKO_QUEUE_STATIC_PRIORITY &&
		    queue)
			static_priority_end = queue - 1;
		else if (static_priority_base != -1 &&
			 static_priority_end == -1 && queue == num_queues - 1)
			static_priority_end =
				queue; /* all queues are static priority */

		/*
		 * Check to make sure all static priority queues are contiguous.
		 * Also catches some cases of static priorites not starting from
		 * queue 0.
		 */
		if (static_priority_end != -1 &&
		    (int)queue > static_priority_end &&
		    priority[p_queue] == CVMX_PKO_QUEUE_STATIC_PRIORITY) {
			debug("ERROR: %s: Static priority queues aren't contiguous or don't start at base queue. q: %d, eq: %d\n",
			      __func__, (int)queue, static_priority_end);
		}
		if (static_priority_base > 0) {
			debug("ERROR: %s: Static priority queues don't start at base queue. sq: %d\n",
			      __func__, static_priority_base);
		}
	}

	/*
	 * main loop to set the fields of CVMX_PKO_MEM_IQUEUE_PTRS for
	 * each queue
	 */
	for (queue = 0; queue < num_queues; queue++) {
		int p_queue = queue % 8;

		config.u64 = 0;
		config.s.index = queue;
		config.s.qid = base_queue + queue;
		config.s.ipid = pko_port;
		config.s.tail = (queue == (num_queues - 1));
		config.s.s_tail = (queue == static_priority_end);
		config.s.static_p = (static_priority_base >= 0);
		config.s.static_q = (queue <= static_priority_end);

		/*
		 * Convert the priority into an enable bit field.
		 * Try to space the bits out evenly so the packet
		 * don't get grouped up.
		 */
		switch ((int)priority[p_queue]) {
		case 0:
			config.s.qos_mask = 0x00;
			break;
		case 1:
			config.s.qos_mask = 0x01;
			break;
		case 2:
			config.s.qos_mask = 0x11;
			break;
		case 3:
			config.s.qos_mask = 0x49;
			break;
		case 4:
			config.s.qos_mask = 0x55;
			break;
		case 5:
			config.s.qos_mask = 0x57;
			break;
		case 6:
			config.s.qos_mask = 0x77;
			break;
		case 7:
			config.s.qos_mask = 0x7f;
			break;
		case 8:
			config.s.qos_mask = 0xff;
			break;
		case CVMX_PKO_QUEUE_STATIC_PRIORITY:
			config.s.qos_mask = 0xff;
			break;
		default:
			debug("ERROR: %s: Invalid priority %llu\n", __func__,
			      (unsigned long long)priority[p_queue]);
			config.s.qos_mask = 0xff;
			break;
		}

		/*
		 * The command queues
		 */
		{
			cvmx_cmd_queue_result_t cmd_res;

			cmd_res = cvmx_cmd_queue_initialize(
				CVMX_CMD_QUEUE_PKO(base_queue + queue),
				CVMX_PKO_MAX_QUEUE_DEPTH, outputbuffer_pool,
				(outputbuffer_pool_size -
				 CVMX_PKO_COMMAND_BUFFER_SIZE_ADJUST * 8));

			if (cmd_res != CVMX_CMD_QUEUE_SUCCESS) {
				switch (cmd_res) {
				case CVMX_CMD_QUEUE_NO_MEMORY:
					debug("ERROR: %s: Unable to allocate output buffer\n",
					      __func__);
					break;
				case CVMX_CMD_QUEUE_ALREADY_SETUP:
					debug("ERROR: %s: Port already setup\n",
					      __func__);
					break;
				case CVMX_CMD_QUEUE_INVALID_PARAM:
				default:
					debug("ERROR: %s: Command queue initialization failed.",
					      __func__);
					break;
				}
				debug(" pko_port%d base_queue%d num_queues%d queue%d.\n",
				      pko_port, base_queue, num_queues, queue);
			}

			buf_ptr = (u64 *)cvmx_cmd_queue_buffer(
				CVMX_CMD_QUEUE_PKO(base_queue + queue));
			config.s.buf_ptr = cvmx_ptr_to_phys(buf_ptr) >> 7;
		}

		CVMX_SYNCWS;
		csr_wr(CVMX_PKO_MEM_IQUEUE_PTRS, config.u64);
	}

	/* Error detection is resirable here */
	return 0;
}
