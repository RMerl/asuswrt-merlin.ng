/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2020 Marvell International Ltd.
 *
 * Interface to the hardware Input Packet Data unit.
 */

#ifndef __CVMX_IPD_H__
#define __CVMX_IPD_H__

#include "cvmx-pki.h"

/* CSR typedefs have been moved to cvmx-ipd-defs.h */

typedef cvmx_ipd_1st_mbuff_skip_t cvmx_ipd_mbuff_not_first_skip_t;
typedef cvmx_ipd_1st_next_ptr_back_t cvmx_ipd_second_next_ptr_back_t;

typedef struct cvmx_ipd_tag_fields {
	u64 ipv6_src_ip : 1;
	u64 ipv6_dst_ip : 1;
	u64 ipv6_src_port : 1;
	u64 ipv6_dst_port : 1;
	u64 ipv6_next_header : 1;
	u64 ipv4_src_ip : 1;
	u64 ipv4_dst_ip : 1;
	u64 ipv4_src_port : 1;
	u64 ipv4_dst_port : 1;
	u64 ipv4_protocol : 1;
	u64 input_port : 1;
} cvmx_ipd_tag_fields_t;

typedef struct cvmx_pip_port_config {
	u64 parse_mode;
	u64 tag_type;
	u64 tag_mode;
	cvmx_ipd_tag_fields_t tag_fields;
} cvmx_pip_port_config_t;

typedef struct cvmx_ipd_config_struct {
	u64 first_mbuf_skip;
	u64 not_first_mbuf_skip;
	u64 ipd_enable;
	u64 enable_len_M8_fix;
	u64 cache_mode;
	cvmx_fpa_pool_config_t packet_pool;
	cvmx_fpa_pool_config_t wqe_pool;
	cvmx_pip_port_config_t port_config;
} cvmx_ipd_config_t;

extern cvmx_ipd_config_t cvmx_ipd_cfg;

/**
 * Gets the fpa pool number of packet pool
 */
static inline s64 cvmx_fpa_get_packet_pool(void)
{
	return (cvmx_ipd_cfg.packet_pool.pool_num);
}

/**
 * Gets the buffer size of packet pool buffer
 */
static inline u64 cvmx_fpa_get_packet_pool_block_size(void)
{
	return (cvmx_ipd_cfg.packet_pool.buffer_size);
}

/**
 * Gets the buffer count of packet pool
 */
static inline u64 cvmx_fpa_get_packet_pool_buffer_count(void)
{
	return (cvmx_ipd_cfg.packet_pool.buffer_count);
}

/**
 * Gets the fpa pool number of wqe pool
 */
static inline s64 cvmx_fpa_get_wqe_pool(void)
{
	return (cvmx_ipd_cfg.wqe_pool.pool_num);
}

/**
 * Gets the buffer size of wqe pool buffer
 */
static inline u64 cvmx_fpa_get_wqe_pool_block_size(void)
{
	return (cvmx_ipd_cfg.wqe_pool.buffer_size);
}

/**
 * Gets the buffer count of wqe pool
 */
static inline u64 cvmx_fpa_get_wqe_pool_buffer_count(void)
{
	return (cvmx_ipd_cfg.wqe_pool.buffer_count);
}

/**
 * Sets the ipd related configuration in internal structure which is then used
 * for seting IPD hardware block
 */
int cvmx_ipd_set_config(cvmx_ipd_config_t ipd_config);

/**
 * Gets the ipd related configuration from internal structure.
 */
void cvmx_ipd_get_config(cvmx_ipd_config_t *ipd_config);

/**
 * Sets the internal FPA pool data structure for packet buffer pool.
 * @param pool	fpa pool number yo use
 * @param buffer_size	buffer size of pool
 * @param buffer_count	number of buufers to allocate to pool
 */
void cvmx_ipd_set_packet_pool_config(s64 pool, u64 buffer_size, u64 buffer_count);

/**
 * Sets the internal FPA pool data structure for wqe pool.
 * @param pool	fpa pool number yo use
 * @param buffer_size	buffer size of pool
 * @param buffer_count	number of buufers to allocate to pool
 */
void cvmx_ipd_set_wqe_pool_config(s64 pool, u64 buffer_size, u64 buffer_count);

/**
 * Gets the FPA packet buffer pool parameters.
 */
static inline void cvmx_fpa_get_packet_pool_config(s64 *pool, u64 *buffer_size, u64 *buffer_count)
{
	if (pool)
		*pool = cvmx_ipd_cfg.packet_pool.pool_num;
	if (buffer_size)
		*buffer_size = cvmx_ipd_cfg.packet_pool.buffer_size;
	if (buffer_count)
		*buffer_count = cvmx_ipd_cfg.packet_pool.buffer_count;
}

/**
 * Sets the FPA packet buffer pool parameters.
 */
static inline void cvmx_fpa_set_packet_pool_config(s64 pool, u64 buffer_size, u64 buffer_count)
{
	cvmx_ipd_set_packet_pool_config(pool, buffer_size, buffer_count);
}

/**
 * Gets the FPA WQE pool parameters.
 */
static inline void cvmx_fpa_get_wqe_pool_config(s64 *pool, u64 *buffer_size, u64 *buffer_count)
{
	if (pool)
		*pool = cvmx_ipd_cfg.wqe_pool.pool_num;
	if (buffer_size)
		*buffer_size = cvmx_ipd_cfg.wqe_pool.buffer_size;
	if (buffer_count)
		*buffer_count = cvmx_ipd_cfg.wqe_pool.buffer_count;
}

/**
 * Sets the FPA WQE pool parameters.
 */
static inline void cvmx_fpa_set_wqe_pool_config(s64 pool, u64 buffer_size, u64 buffer_count)
{
	cvmx_ipd_set_wqe_pool_config(pool, buffer_size, buffer_count);
}

/**
 * Configure IPD
 *
 * @param mbuff_size Packets buffer size in 8 byte words
 * @param first_mbuff_skip
 *                   Number of 8 byte words to skip in the first buffer
 * @param not_first_mbuff_skip
 *                   Number of 8 byte words to skip in each following buffer
 * @param first_back Must be same as first_mbuff_skip / 128
 * @param second_back
 *                   Must be same as not_first_mbuff_skip / 128
 * @param wqe_fpa_pool
 *                   FPA pool to get work entries from
 * @param cache_mode
 * @param back_pres_enable_flag
 *                   Enable or disable port back pressure at a global level.
 *                   This should always be 1 as more accurate control can be
 *                   found in IPD_PORTX_BP_PAGE_CNT[BP_ENB].
 */
void cvmx_ipd_config(u64 mbuff_size, u64 first_mbuff_skip, u64 not_first_mbuff_skip, u64 first_back,
		     u64 second_back, u64 wqe_fpa_pool, cvmx_ipd_mode_t cache_mode,
		     u64 back_pres_enable_flag);
/**
 * Enable IPD
 */
void cvmx_ipd_enable(void);

/**
 * Disable IPD
 */
void cvmx_ipd_disable(void);

void __cvmx_ipd_free_ptr(void);

void cvmx_ipd_set_packet_pool_buffer_count(u64 buffer_count);
void cvmx_ipd_set_wqe_pool_buffer_count(u64 buffer_count);

/**
 * Setup Random Early Drop on a specific input queue
 *
 * @param queue  Input queue to setup RED on (0-7)
 * @param pass_thresh
 *               Packets will begin slowly dropping when there are less than
 *               this many packet buffers free in FPA 0.
 * @param drop_thresh
 *               All incoming packets will be dropped when there are less
 *               than this many free packet buffers in FPA 0.
 * Return: Zero on success. Negative on failure
 */
int cvmx_ipd_setup_red_queue(int queue, int pass_thresh, int drop_thresh);

/**
 * Setup Random Early Drop to automatically begin dropping packets.
 *
 * @param pass_thresh
 *               Packets will begin slowly dropping when there are less than
 *               this many packet buffers free in FPA 0.
 * @param drop_thresh
 *               All incoming packets will be dropped when there are less
 *               than this many free packet buffers in FPA 0.
 * Return: Zero on success. Negative on failure
 */
int cvmx_ipd_setup_red(int pass_thresh, int drop_thresh);

#endif /*  __CVMX_IPD_H__ */
