// SPDX-License-Identifier: GPL-2.0
/*
 * (C) 2003 - Rolf Neugebauer - Intel Research Cambridge
 * (C) 2005 - Grzegorz Milos - Intel Research Cambridge
 * (C) 2020 - EPAM Systems Inc.
 *
 * File: events.c [1]
 * Author: Rolf Neugebauer (neugebar@dcs.gla.ac.uk)
 * Changes: Grzegorz Milos (gm281@cam.ac.uk)
 *
 * Date: Jul 2003, changes Jun 2005
 *
 * Description: Deals with events received on event channels
 *
 * [1] - http://xenbits.xen.org/gitweb/?p=mini-os.git;a=summary
 */
#include <common.h>
#include <log.h>

#include <asm/io.h>
#include <asm/xen/system.h>

#include <xen/events.h>
#include <xen/hvm.h>

#if IS_ENABLED(CONFIG_XEN_SERIAL)
extern u32 console_evtchn;
#endif /* IS_ENABLED(CONFIG_XEN_SERIAL) */

#define NR_EVS 1024

/**
 * struct _ev_action - represents a event handler.
 *
 * Chaining or sharing is not allowed
 */
struct _ev_action {
	void (*handler)(evtchn_port_t port, struct pt_regs *regs, void *data);
	void *data;
	u32 count;
};

static struct _ev_action ev_actions[NR_EVS];
void default_handler(evtchn_port_t port, struct pt_regs *regs, void *data);

static unsigned long bound_ports[NR_EVS / (8 * sizeof(unsigned long))];

void unbind_all_ports(void)
{
	int i;
	int cpu = 0;
	struct shared_info *s = HYPERVISOR_shared_info;
	struct vcpu_info *vcpu_info = &s->vcpu_info[cpu];

	for (i = 0; i < NR_EVS; i++) {
#if IS_ENABLED(CONFIG_XEN_SERIAL)
		if (i == console_evtchn)
			continue;
#endif /* IS_ENABLED(CONFIG_XEN_SERIAL) */

		if (test_and_clear_bit(i, bound_ports)) {
			printf("port %d still bound!\n", i);
			unbind_evtchn(i);
		}
	}
	vcpu_info->evtchn_upcall_pending = 0;
	vcpu_info->evtchn_pending_sel = 0;
}

int do_event(evtchn_port_t port, struct pt_regs *regs)
{
	struct _ev_action *action;

	clear_evtchn(port);

	if (port >= NR_EVS) {
		printk("WARN: do_event(): Port number too large: %d\n", port);
		return 1;
	}

	action = &ev_actions[port];
	action->count++;

	/* call the handler */
	action->handler(port, regs, action->data);

	return 1;
}

evtchn_port_t bind_evtchn(evtchn_port_t port,
			  void (*handler)(evtchn_port_t, struct pt_regs *, void *),
			  void *data)
{
	if (ev_actions[port].handler != default_handler)
		printf("WARN: Handler for port %d already registered, replacing\n",
		       port);

	ev_actions[port].data = data;
	wmb();
	ev_actions[port].handler = handler;
	synch_set_bit(port, bound_ports);

	return port;
}

/**
 * unbind_evtchn() - Unbind event channel for selected port
 */
void unbind_evtchn(evtchn_port_t port)
{
	struct evtchn_close close;
	int rc;

	if (ev_actions[port].handler == default_handler)
		debug("Default handler for port %d when unbinding\n", port);
	mask_evtchn(port);
	clear_evtchn(port);

	ev_actions[port].handler = default_handler;
	wmb();
	ev_actions[port].data = NULL;
	synch_clear_bit(port, bound_ports);

	close.port = port;
	rc = HYPERVISOR_event_channel_op(EVTCHNOP_close, &close);
	if (rc)
		printf("WARN: close_port %d failed rc=%d. ignored\n", port, rc);
}

void default_handler(evtchn_port_t port, struct pt_regs *regs, void *ignore)
{
	debug("[Port %d] - event received\n", port);
}

/**
 * evtchn_alloc_unbound() - Create a port available to the pal for
 * exchanging notifications.
 *
 * Unfortunate confusion of terminology: the port is unbound as far
 * as Xen is concerned, but we automatically bind a handler to it.
 *
 * Return: The result of the hypervisor call.
 */
int evtchn_alloc_unbound(domid_t pal,
			 void (*handler)(evtchn_port_t, struct pt_regs *, void *),
			 void *data, evtchn_port_t *port)
{
	int rc;

	struct evtchn_alloc_unbound op;

	op.dom = DOMID_SELF;
	op.remote_dom = pal;
	rc = HYPERVISOR_event_channel_op(EVTCHNOP_alloc_unbound, &op);
	if (rc) {
		printf("ERROR: alloc_unbound failed with rc=%d", rc);
		return rc;
	}
	if (!handler)
		handler = default_handler;
	*port = bind_evtchn(op.port, handler, data);
	return rc;
}

/**
 * eventchn_poll() - Event channel polling function
 *
 * Check and process any pending events
 */
void eventchn_poll(void)
{
	do_hypervisor_callback(NULL);
}

/**
 * init_events() - Initialize event handler
 *
 * Initially all events are without a handler and disabled.
 */
void init_events(void)
{
	int i;

	debug("%s\n", __func__);

	for (i = 0; i < NR_EVS; i++) {
		ev_actions[i].handler = default_handler;
		mask_evtchn(i);
	}
}

/**
 * fini_events() - Close all ports
 *
 * Mask and clear event channels. Close port using EVTCHNOP_close
 * hypercall.
 */
void fini_events(void)
{
	debug("%s\n", __func__);
	/* Dealloc all events */
	unbind_all_ports();
}
