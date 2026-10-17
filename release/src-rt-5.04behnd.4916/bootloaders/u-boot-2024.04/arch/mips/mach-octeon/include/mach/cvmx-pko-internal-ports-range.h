/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2020 Marvell International Ltd.
 */

#ifndef __CVMX_INTERNAL_PORTS_RANGE__
#define __CVMX_INTERNAL_PORTS_RANGE__

/*
 * Allocated a block of internal ports for the specified interface/port
 *
 * @param  interface  the interface for which the internal ports are requested
 * @param  port       the index of the port within in the interface for which the internal ports
 *                    are requested.
 * @param  count      the number of internal ports requested
 *
 * Return:  0 on success
 *         -1 on failure
 */
int cvmx_pko_internal_ports_alloc(int interface, int port, u64 count);

/*
 * Free the internal ports associated with the specified interface/port
 *
 * @param  interface  the interface for which the internal ports are requested
 * @param  port       the index of the port within in the interface for which the internal ports
 *                    are requested.
 *
 * Return:  0 on success
 *         -1 on failure
 */
int cvmx_pko_internal_ports_free(int interface, int port);

/*
 * Frees up all the allocated internal ports.
 */
void cvmx_pko_internal_ports_range_free_all(void);

void cvmx_pko_internal_ports_range_show(void);

int __cvmx_pko_internal_ports_range_init(void);

#endif
