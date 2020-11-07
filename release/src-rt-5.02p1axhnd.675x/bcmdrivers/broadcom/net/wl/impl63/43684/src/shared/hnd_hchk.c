/*
 * Health Check Module
 *
 * Copyright 2020 Broadcom
 *
 * This program is the proprietary software of Broadcom and/or
 * its licensors, and may only be used, duplicated, modified or distributed
 * pursuant to the terms and conditions of a separate, written license
 * agreement executed between you and Broadcom (an "Authorized License").
 * Except as set forth in an Authorized License, Broadcom grants no license
 * (express or implied), right to use, or waiver of any kind with respect to
 * the Software, and Broadcom expressly reserves all rights in and to the
 * Software and all intellectual property rights therein.  IF YOU HAVE NO
 * AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY
 * WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF
 * THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use
 * all reasonable efforts to protect the confidentiality thereof, and to
 * use this information only in connection with your use of Broadcom
 * integrated circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES,
 * REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR
 * OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
 * DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY,
 * NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES,
 * ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
 * CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING
 * OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL
 * BROADCOM OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL,
 * SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR
 * IN ANY WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
 * IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii)
 * ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF
 * OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY
 * NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 * $Id$
 */
#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <hnd_trap.h>
#include <bcmutils.h>
#include <bcmdevs.h>
#include <awd.h>
#include <hnd_debug.h>
#include <bcmendian.h>
#include <ethernet.h>
#include <802.11.h>
#include <hnd_event.h>
#include <hnd_hchk.h>

/* The header contains type and length */
#define HEALTH_CHECK_XTLV_HEADER_SIZE			4
#define HEALTH_CHECK_TOP_LEVEL_ENTITY_XTLV_HEADER_SIZE	4
#define	HC_ALIGN(ad, al)	(((ad) + ((al) - 1)) & ~((al) - 1))

#ifdef EVENT_LOG_COMPILE
#define HC_ERROR(args)	EVENT_LOG_COMPACT_CAST_PAREN_ARGS(EVENT_LOG_TAG_HEALTH_CHECK_ERROR, args)
#else
#define HC_ERROR(args)	printf args
#endif // endif

struct health_check_client_info {
	health_check_fn fn;
	void *context;
	uint16 module_id;				/* module within an entity. */
	uint32 last_health_check_status;
	struct health_check_client_info *next;
};

struct health_check_info {
	health_check_client_info_t *client_health_check;
	uint32 last_health_check_time;			/* Last health check time */
	uint8 *buffer;					/* buffer for information */
	uint16 max_length;				/* Length of the buffer. */
	hchk_sw_entity_t top_level_sw_entity_type;	/* which entity is this? */
	osl_t *osh;
};

/* Each SW entity calls this function to register itself with health check. */
health_check_info_t*
BCMATTACHFN(health_check_init)(osl_t *osh,
	hchk_sw_entity_t top_level_sw_entity_type, uint16 buffer_size)
{
	health_check_info_t *hc;
	uint16 local_buf_size = 0;

	if (top_level_sw_entity_type == HCHK_SW_ENTITY_UNDEFINED) {
		return NULL;
	}

	hc = (health_check_info_t*)MALLOCZ(osh, sizeof(health_check_info_t));
	if (hc == NULL) {
		HC_ERROR(("Health check init: "
			"Out of memory during descriptor allocation for entity: %d\n",
			top_level_sw_entity_type));
		return NULL;
	}

	/* Add the header and make buf size word align */
	local_buf_size = HC_ALIGN(buffer_size + HEALTH_CHECK_XTLV_HEADER_SIZE +
		HEALTH_CHECK_TOP_LEVEL_ENTITY_XTLV_HEADER_SIZE, sizeof(uint32));
	hc->buffer = (uint8*)MALLOCZ(osh, local_buf_size);
	if (hc->buffer == NULL) {
		HC_ERROR(("Health check init: Out of memory on buffer "
			"Out of memory on buffer allocation for top level SW entity: %d \n",
			top_level_sw_entity_type));
		MFREE(osh, hc, sizeof(health_check_info_t));
		return NULL;
	}

	hc->max_length = local_buf_size;
	hc->top_level_sw_entity_type = top_level_sw_entity_type;
	hc->osh = osh;

	return hc;
}

health_check_client_info_t*
BCMATTACHFN(health_check_module_register)(health_check_info_t* desc,
	health_check_fn fn, void* context, int module_id)
{
	health_check_client_info_t *client_info;

	ASSERT(fn != NULL);
	ASSERT(desc != NULL);

	client_info = (health_check_client_info_t*)MALLOCZ(desc->osh,
		sizeof(health_check_client_info_t));

	if (client_info == NULL) {
		HC_ERROR(("Health check init: Out of memory on buffer "
			"Out of memory on creating client info for entity: %d module_id: %d\n",
			desc->top_level_sw_entity_type, module_id));
		return NULL;
	}

	client_info->fn = fn;
	client_info->context = context;
	client_info->module_id = module_id;
	client_info->last_health_check_status = HEALTH_CHECK_STATUS_OK;

	client_info->next = desc->client_health_check;
	desc->client_health_check = client_info;

	return client_info;
}

/* Free an already registered client */
int
health_check_module_unregister(health_check_info_t *desc,
	health_check_client_info_t *client)
{
	health_check_client_info_t *head;
	int rc;

	if (!client) {
		return BCME_ERROR;
	}

	head = desc->client_health_check;

	if (head == client) {
		desc->client_health_check = client->next;
		MFREE(desc->osh, client, sizeof(*client));
		return BCME_OK;
	}
	else {
		while (head->next) {
			if (head->next == client) {
				head->next = client->next;
				MFREE(desc->osh, client,
					sizeof(*client));
				return BCME_OK;
			}
			head = head->next;
		}
		rc = BCME_NOTFOUND;
	}
	return rc;
}

/* This can be called from timer too! Pass in a list of client to run health check on.
 * No need to pass num_modules ideally. The list of modules could be NULL terminated.
 */
int
health_check_execute(health_check_info_t *desc,
	health_check_client_info_t **modules, uint16 num_modules)
{
	int rc = BCME_OK;
	uint16 type = SOCRAM_IND_TAG_HEALTH_CHECK, len = 0, sw_entity;
	int health_status = HEALTH_CHECK_STATUS_OK;
	int16 remaining_len = 0, bytes_written = 0;
	uint8 *buf_ptr, *buf_ptr_align;
	health_check_client_info_t *client_hc_info;
	uint16 module_id_index = 0;

	client_hc_info = (modules) ? modules[module_id_index++] :
		desc->client_health_check;

	/* Make an offset of 8 bytes into the buffer.
	 * This is where all registered clients will put their XTLV payload
	 * First populate the payload on Health check and then fill XTLV header
	 * later
	 */
	ASSERT(desc->buffer);
	buf_ptr = desc->buffer +
		(HEALTH_CHECK_XTLV_HEADER_SIZE +
			HEALTH_CHECK_TOP_LEVEL_ENTITY_XTLV_HEADER_SIZE);

	/* CAll all registered clients and store their health check status.
	 * If remaining_len changes, this mean at least one client put its
	 * stuff in the buffer.
	 * So send it.
	 */
	while (client_hc_info) {
		ASSERT(client_hc_info->fn);

		/* If a problem is detected by a module, it must populate its TLV
		 * If there is no space left in the buffer, it can skip populating
		 * its data. It must populate appropriate status bits.
		 */
		bytes_written = 0;

		/* Next set of TLV must start at word boundary */
		buf_ptr_align = (uint8*)HC_ALIGN((uint32)buf_ptr, sizeof(uint32));

		/* Derive remaining length in the buffer */
		remaining_len = desc->buffer + desc->max_length - buf_ptr_align;

		if (remaining_len < 0) {
			remaining_len = 0;
		}

		/* if remaining len == 0, the client MUST not fill the buffer.
		 * STill calling the client to get its health check status.
		 */
		client_hc_info->last_health_check_status =
			client_hc_info->fn(buf_ptr_align, remaining_len,
				client_hc_info->context, &bytes_written);
		/* We should have different health check configurations such as:
		 * Generate soc ram dump event indication
		 * Generate a trap
		 */
		health_status |= client_hc_info->last_health_check_status;

		if (bytes_written > 0) {
			/* remaining_len - bytes_written < 0 => The submodule overran health check's
			* buffer and therefore memory corruption.
			* In such a case simply trap.
			* This is not guaranteed. If the HC code space is corrupted, then a trap
			* may not be generated.
			*/
			if ((remaining_len - bytes_written) < 0) {
				HC_ERROR(("Health check buffer overrun. Top level module: %d "
					"Sub module: %d "
					"Remaining len: %d "
					"Number of bytes written: %d ",
					desc->top_level_sw_entity_type,
					client_hc_info->module_id,
					remaining_len,
					bytes_written));

				/* Generate a trap as memory is corrupted. */
				health_status |= HEALTH_CHECK_STATUS_TRAP;
				/* Store the last time at which health check was performed. */
				desc->last_health_check_time = OSL_SYSUPTIME();
				OSL_SYS_HALT();
				return BCME_ERROR;
			}
			/* modify the buf_ptr (the unaligned location where to write next)
			* only when bytes were written. This is to make sure that buf
			* does not overrun allocated buffer space
			*/
			buf_ptr = buf_ptr_align + bytes_written;
		}

		/* If a client's status reports trap, do not break out.
		 * Go thru all registered clients and get their statuses.
		 */
		if (modules) {
			client_hc_info = (module_id_index <= num_modules - 1) ?
				modules[module_id_index++] : NULL;
		}
		else {
			client_hc_info = client_hc_info->next;
		}
	}

	/* Store the last time at which health check was performed. */
	desc->last_health_check_time = OSL_SYSUPTIME();

	/* Fill header on trap or error */
	if (health_status & (HEALTH_CHECK_STATUS_ERROR | HEALTH_CHECK_STATUS_TRAP)) {
		/* Doing this way because in future there could be a user supplied buffer that is
		 * not aligned to a word boundary.
		 */

		/* Fill XTLV header for health check and top level module. */
		len = buf_ptr - desc->buffer - HEALTH_CHECK_XTLV_HEADER_SIZE;

		/* Put health check type.
		 * type size = uint16
		 */
		buf_ptr = desc->buffer;
		memcpy(buf_ptr, &type, sizeof(type));

		/* Followed by length
		 * len size = uint16
		 */
		buf_ptr += sizeof(type);
		memcpy(buf_ptr, &len, sizeof(len));

		/* The value portion of health check TLV contains sub TLV.
		 * Set the type as top_level_sw_entity_type.
		 * sw_entity size = uint16
		 */
		buf_ptr += sizeof(len);
		sw_entity = (uint16)desc->top_level_sw_entity_type;
		memcpy(buf_ptr, &sw_entity, sizeof(sw_entity));

		/* Set the length of data in this top level module's TLV. */
		buf_ptr += sizeof(sw_entity);
		len -= HEALTH_CHECK_TOP_LEVEL_ENTITY_XTLV_HEADER_SIZE;
		memcpy(buf_ptr, &len, sizeof(len));

		if (health_status & HEALTH_CHECK_STATUS_TRAP) {
			OSL_SYS_HALT();
			return BCME_ERROR;
		}

		/* Send the SOCRAM dump indication event */
		rc = hnd_send_dngl_event(DNGL_E_SOCRAM_IND, desc->buffer,
			(uint32)(len + HEALTH_CHECK_XTLV_HEADER_SIZE +
			HEALTH_CHECK_TOP_LEVEL_ENTITY_XTLV_HEADER_SIZE));
		if (rc != BCME_OK) {
			HC_ERROR(("SocRAM dump indication error. Top level module: %d "
				"Last Health check time in ms: %d\n",
				desc->top_level_sw_entity_type,
				desc->last_health_check_time));
		}
	}

	return rc;
}
