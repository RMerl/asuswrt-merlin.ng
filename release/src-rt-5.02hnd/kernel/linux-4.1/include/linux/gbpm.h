#if defined(CONFIG_BCM_KF_NBUFF)
#ifndef __GBPM_H_INCLUDED__
#define __GBPM_H_INCLUDED__

/*
 *
<:copyright-BRCM:2007:DUAL/GPL:standard

   Copyright (c) 2007 Broadcom 
   All Rights Reserved

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License, version 2, as published by
the Free Software Foundation (the "GPL").

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.


A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.

:>
*/

/*
 *******************************************************************************
 * File Name : gbpm.h
 *
 *******************************************************************************
 */
#define GBPM_VERSION             "v0.1"
#define GBPM_VER_STR             GBPM_VERSION
#define GBPM_MODNAME             "Broadcom GBPM "

#define GBPM_ERROR               (-1)
#define GBPM_SUCCESS             0

#define GBPM_RXCHNL_MAX              4
#define GBPM_RXCHNL_DISABLED         0
#define GBPM_RXCHNL_ENABLED          1


typedef enum {
    GBPM_PORT_ETH,
    GBPM_PORT_XTM,
    GBPM_PORT_FWD,
    GBPM_PORT_WLAN,
    GBPM_PORT_USB,
    GBPM_PORT_MAX
} gbpm_port_t;

typedef void (* gbpm_evt_hook_t) (void);

typedef void (* gbpm_thresh_hook_t)(void);
typedef void (* gbpm_upd_buf_lvl_hook_t)(int);
typedef void (* gbpm_status_hook_t)(void);


typedef int (* gbpm_dyn_buf_lvl_hook_t) (void);
typedef int (* gbpm_alloc_mult_hook_t)( uint32_t, void **);
typedef void (* gbpm_free_mult_hook_t)( uint32_t, void **);
typedef void * (* gbpm_alloc_hook_t)(void);
typedef void (* gbpm_free_hook_t)( void * );
typedef int (* gbpm_resv_rx_hook_t)(gbpm_port_t, uint32_t, uint32_t, uint32_t );
typedef int (* gbpm_unresv_rx_hook_t)( gbpm_port_t, uint32_t );
typedef uint32_t (* gbpm_get_total_bufs_hook_t)(void);
typedef uint32_t (* gbpm_get_avail_bufs_hook_t)(void);
typedef uint32_t (* gbpm_get_max_dyn_bufs_hook_t)(void);


int gbpm_get_dyn_buf_level(void);
int gbpm_resv_rx_buf( gbpm_port_t port, uint32_t chnl,
        uint32_t num_rx_buf, uint32_t bulk_alloc_count );
int gbpm_unresv_rx_buf( gbpm_port_t port, uint32_t chnl );

int gbpm_alloc_mult_buf( uint32_t num, void **buf_p );
void gbpm_free_mult_buf( uint32_t num, void **buf_p );

void * gbpm_alloc_buf( void );
void gbpm_free_buf( void * buf_p );

uint32_t gbpm_get_total_bufs( void );
#define CONFIG_GBPM_API_HAS_GET_TOTAL_BUFS 1

uint32_t gbpm_get_avail_bufs( void );
#define CONFIG_GBPM_API_HAS_GET_AVAIL_BUFS 1

uint32_t gbpm_get_max_dyn_bufs( void );


void gbpm_queue_work(void);
void gbpm_bind( gbpm_dyn_buf_lvl_hook_t gbpm_dyn_buf_lvl, 
                gbpm_alloc_mult_hook_t gbpm_alloc_mult,
                gbpm_free_mult_hook_t gbpm_free_mult,
                gbpm_alloc_hook_t gbpm_alloc,
                gbpm_free_hook_t gbpm_free,
                gbpm_resv_rx_hook_t gbpm_resv_rx, 
                gbpm_unresv_rx_hook_t gbpm_unresv_rx ,
                gbpm_get_total_bufs_hook_t gbpm_get_total_bufs ,
                gbpm_get_avail_bufs_hook_t gbpm_get_avail_bufs,
                gbpm_get_max_dyn_bufs_hook_t gbpm_get_max_dyn_bufs );

void gbpm_unbind( void );

#endif  /* defined(__GBPM_H_INCLUDED__) */

#endif
