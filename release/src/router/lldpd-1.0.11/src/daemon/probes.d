/* -*- mode: c; c-file-style: "openbsd" -*- */
/*
 * Copyright (c) 2013 Vincent Bernat <bernat@luffy.cx>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
provider lldpd {

	/**
	 * Fired when a frame is received, before it is decoded.
	 * @param ifname the name of the interface
	 * @param frame  the received frame
	 * @param len    the len of the received frame
	 */
	probe frame_received(char *ifname, void *frame, size_t len);

	/**
	 * Fired when a frame is decoded.
	 * @param ifname       the name of the interface
	 * @param protocol     the name of the protocol
	 * @param chassis_name the name of chassis (may be NULL)
	 * @param port_descr   the description of the port (may be NULL)
	 */
	probe frame_decoded(char *ifname, char *protocol, char *chassis_name, char *port_descr);

	/**
	 * Fired when a frame is sent.
	 * @param ifname   the name of the interface
	 * @param protocol the name of the protocol
	 */
	probe frame_send(char *ifname, char *protocol);

	/**
	 * Fired when a neighbor is added.
	 * @param ifname       the name of the interface where the neighbor appeared
	 * @param chassis_name the name of chassis (may be NULL)
	 * @param port_descr   the description of the port (may be NULL)
	 * @param count        the total number of neighbors known
	 */
	probe neighbor_new(char *ifname, char *chassis_name, char *port_descr, int count);

	/**
	 * Fired when a neighbor is updated.
	 * @param ifname       the name of the interface where the neighbor updated
	 * @param chassis_name the name of chassis (may be NULL)
	 * @param port_descr   the description of the port (may be NULL)
	 * @param count        the total number of neighbors known
	 */
	probe neighbor_update(char *ifname, char *chassis_name, char *port_descr, int count);

	/**
	 * Fired when a neighbor is deleted.
	 * @param ifname       the name of the interface where the neighbor deleted
	 * @param chassis_name the name of chassis (may be NULL)
	 * @param port_descr   the description of the port (may be NULL)
	 * @param count        the total number of neighbors known
	 */
	probe neighbor_delete(char *ifname, char *chassis_name, char *port_descr);

	/**
	 * Fired before handling a client request.
	 * @param name the name of the request
	 */
	probe client_request(char *name);

	/**
	 * Fired for each iteration of the event loop.
	 */
	probe event_loop();

        /**
	 * Fired when initializing a new interface in privileged mode.
	 * @param name the name of the interface
	 */
	probe priv_interface_init(char *name);

	/**
	 * Fired when setting description of an interface.
	 * @param name the name of the interface
	 * @param desc the description of the interface
	 */
	probe priv_interface_description(char *name, char *description);

	/**
	 * Fired when doing an interface updates.
	 */
	probe interfaces_update();

	/**
	 * Fired when receiving an interface update notification.
	 */
	probe interfaces_notification();

	/**
	 * Fired when an interface is removed.
	 * @param name the name of the interface
	 */
	probe interfaces_delete(char *name);

	/**
	 * Fired when an interface is added.
	 * @param name the name of the interface
	 */
	probe interfaces_new(char *name);
};
