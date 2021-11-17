/***********************************************************************
 *
 *  Copyright (c) 2007  Broadcom Corporation
 *  All Rights Reserved
 *
 <:label-BRCM:2014:DUAL/GPL:standard 
 
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
 *
 ************************************************************************/

#ifndef _SIMCARD_CTL_API_H_
#define _SIMCARD_CTL_API_H_

#include "bcmtypes.h"

typedef void (*sim_insert_cb)(SIMIO_ID_t sim_id, void* app_ctx);  /* Insert callback */
typedef void (*sim_remove_cb)(SIMIO_ID_t sim_id, void* app_ctx);  /* Remove callback */
typedef void (*sim_signal_cb)(SIMIO_ID_t sim_id, SIMIO_SIGNAL_t sim_sig, void* app_ctx); /* Signal callback */
typedef void (*sim_change_cb)(SIMIO_ID_t sim_id, void *app_ctx, int change); /* Configuration changed
                                                                                              callback */

typedef struct bcm_simio_app_cbs_s 
{
    sim_insert_cb insert;
    sim_remove_cb remove;
    sim_signal_cb signal;
    sim_change_cb change;
} bcm_simio_app_cbs_t;

int bcm_simio_lib_init(void);
void bcm_simio_lib_uninit(void);
int bcm_simio_lib_register_app(SIMIO_ID_t sim_id, void* app_ctx, bcm_simio_app_cbs_t *cbs, void **lib_ctx);
void bcm_simio_lib_unregister_app(void *lib_ctx);

int bcm_simio_lib_activate(void *lib_ctx, size_t *size, void *data);
int bcm_simio_lib_is_online(void *lib_ctx, uint32_t *data);
int bcm_simio_lib_set_baudrate(void *lib_ctx, uint32_t F, uint32_t D);
int bcm_simio_lib_set_protocol(void *lib_ctx, PROTOCOL_t prot);
int bcm_simio_lib_set_control(void *lib_ctx, uint32_t control);
int bcm_simio_lib_reset(void *lib_ctx, uint32_t reset, SIMIO_DIVISOR_t freq, SimVoltageLevel_t volt);

int bcm_simio_lib_open(void *lib_ctx, uint16_t file, const char* pin1, const char* pin2);
int bcm_simio_lib_change_file(int fd, uint16_t file, const char* pin1, const char* pin2);
int bcm_simio_lib_close(int fd);
int bcm_simio_lib_get_select_response(int fd, uint8_t *data, size_t *len);
size_t bcm_simio_lib_read(int fd, uint8_t* buffer, size_t offset, size_t size, uint32_t timeout);
size_t bcm_simio_lib_write(int fd, uint8_t* buffer, size_t offset, size_t size, uint32_t timeout);

#endif /* _SIMCARD_CTL_API_H_ */

