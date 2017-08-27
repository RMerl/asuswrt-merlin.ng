/*
 * Copyright (C) 2016, Broadcom. All Rights Reserved.
 * 
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: $
 */

#ifndef _FLOW_API_H_
#define _FLOW_API_H_

#include <netinet/in.h>

#include <ctf_cfg.h>

/* Suspend a flow */
int flow_suspend(int family,
                 ip_address_t *src_addr, uint16_t src_port,
                 ip_address_t *dst_addr, uint16_t dst_port,
                 uint8_t protocol);

/* Resume a flow */
int flow_resume(int family,
                ip_address_t *src_addr, uint16_t src_port,
                ip_address_t *dst_addr, uint16_t dst_port,
                uint8_t protocol);

/* Delete a flow entry */
int flow_delete(int family,
                ip_address_t *src_addr, uint16_t src_port,
                ip_address_t *dst_addr, uint16_t dst_port,
                uint8_t protocol);

/* Is flow valid? */
int flow_valid(int family,
               ip_address_t *src_addr, uint16_t src_port,
               ip_address_t *dst_addr, uint16_t dst_port,
               uint8_t protocol);

/* Configure flow default forwarding (old - backward compatibility) */
int flow_default_forwarding_get(ctf_fwd_t *fwd);
int flow_default_forwarding_set(ctf_fwd_t fwd);

/* Configure flow default forwarding (use this instead) */
int flow_default_forwarding_get_for_proto(ctf_fwd_t *fwd, uint8_t proto);
int flow_default_forwarding_set_for_proto(ctf_fwd_t fwd, uint8_t proto);

#endif /* _FLOW_API_H_ */
