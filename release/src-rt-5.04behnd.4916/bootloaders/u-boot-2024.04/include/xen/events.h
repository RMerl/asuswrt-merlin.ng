/* SPDX-License-Identifier: GPL-2.0
 *
 * (C) 2003 - Rolf Neugebauer - Intel Research Cambridge
 * (C) 2005 - Grzegorz Milos - Intel Reseach Cambridge
 * (C) 2020 - EPAM Systems Inc.
 *
 * File: events.h
 * Author: Rolf Neugebauer (neugebar@dcs.gla.ac.uk)
 * Changes: Grzegorz Milos (gm281@cam.ac.uk)
 *
 * Date: Jul 2003, changes Jun 2005
 *
 * Description: Deals with events on the event channels
 */
#ifndef _EVENTS_H_
#define _EVENTS_H_

#include <asm/xen/hypercall.h>
#include <xen/interface/event_channel.h>

void init_events(void);
void fini_events(void);

int do_event(evtchn_port_t port, struct pt_regs *regs);
void unbind_evtchn(evtchn_port_t port);
void unbind_all_ports(void);
int evtchn_alloc_unbound(domid_t pal,
			 void (*handler)(evtchn_port_t, struct pt_regs *, void *),
			 void *data, evtchn_port_t *port);

/* Send notification via event channel */
static inline int notify_remote_via_evtchn(evtchn_port_t port)
{
	struct evtchn_send op;

	op.port = port;
	return HYPERVISOR_event_channel_op(EVTCHNOP_send, &op);
}

void eventchn_poll(void);

#endif /* _EVENTS_H_ */
