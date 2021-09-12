/* -*- mode: c; c-file-style: "openbsd" -*- */
/*
 * Copyright (c) 2012 Vincent Bernat <bernat@luffy.cx>
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

#include <sys/queue.h>
#include "../lldpd-structs.h"
#include "../compat/compat.h"
#include "../marshal.h"
#include "../ctl.h"

/* connection.c */
struct lldpctl_conn_t {
	/* the Unix-domain socket to connect to lldpd */
	char *ctlname;

	/* Callback handling */
	lldpctl_recv_callback recv; /* Receive callback */
	lldpctl_send_callback send; /* Send callback */
	void *user_data;	    /* Callback user data */

	/* IO state handling. */
	uint8_t *input_buffer;	/* Current input/output buffer */
	uint8_t *output_buffer; /* Current input/output buffer */
	size_t input_buffer_len;
	size_t output_buffer_len;

#define CONN_STATE_IDLE			0
#define CONN_STATE_GET_INTERFACES_SEND	1
#define CONN_STATE_GET_INTERFACES_RECV	2
#define CONN_STATE_GET_PORT_SEND	3
#define CONN_STATE_GET_PORT_RECV	4
#define CONN_STATE_SET_PORT_SEND	5
#define CONN_STATE_SET_PORT_RECV	6
#define CONN_STATE_SET_WATCH_SEND	7
#define CONN_STATE_SET_WATCH_RECV	8
#define CONN_STATE_GET_CONFIG_SEND	9
#define CONN_STATE_GET_CONFIG_RECV	10
#define CONN_STATE_SET_CONFIG_SEND	11
#define CONN_STATE_SET_CONFIG_RECV	12
#define CONN_STATE_GET_CHASSIS_SEND	13
#define CONN_STATE_GET_CHASSIS_RECV	14
#define CONN_STATE_GET_DEFAULT_PORT_SEND 15
#define CONN_STATE_GET_DEFAULT_PORT_RECV 16
#define CONN_STATE_WATCHING		17
	int state;		/* Current state */
	/* Data attached to the state. It is used to check that we are using the
	 * same data as a previous call until the state machine goes to
	 * CONN_STATE_IDLE. */
	char state_data[IFNAMSIZ + 64];
	/* Error handling */
	lldpctl_error_t error;	/* Last error */

	/* Handling notifications */
	lldpctl_change_callback watch_cb;
	lldpctl_change_callback2 watch_cb2;
	void *watch_data;
	int watch_triggered;
};

/* User data for synchronous callbacks. */
struct lldpctl_conn_sync_t {
	int fd;			/* File descriptor to the socket. */
};

ssize_t _lldpctl_needs(lldpctl_conn_t *lldpctl, size_t length);
int _lldpctl_do_something(lldpctl_conn_t *conn,
    int state_send, int state_recv, const char *state_data,
    enum hmsg_type type,
    void *to_send, struct marshal_info *mi_send,
    void **to_recv, struct marshal_info *mi_recv);

/* errors.c */
#define SET_ERROR(conn, x)    ((conn)->error = x)
#define RESET_ERROR(conn)     SET_ERROR((conn), LLDPCTL_NO_ERROR)


/* atom.c and atom-private.c */
typedef enum {
	atom_config,
	atom_interfaces_list,
	atom_interface,
	atom_ports_list,
	atom_port,
	atom_mgmts_list,
	atom_mgmt,
#ifdef ENABLE_DOT3
	atom_dot3_power,
#endif
#ifdef ENABLE_DOT1
	atom_vlans_list,
	atom_vlan,
	atom_ppvids_list,
	atom_ppvid,
	atom_pis_list,
	atom_pi,
#endif
#ifdef ENABLE_LLDPMED
	atom_med_policies_list,
	atom_med_policy,
	atom_med_locations_list,
	atom_med_location,
	atom_med_caelements_list,
	atom_med_caelement,
	atom_med_power,
#endif
#ifdef ENABLE_CUSTOM
	atom_custom_list,
	atom_custom,
#endif
	atom_chassis,
} atom_t;

void *_lldpctl_alloc_in_atom(lldpctl_atom_t *, size_t);
const char *_lldpctl_dump_in_atom(lldpctl_atom_t *, const uint8_t *, size_t, char, size_t);

struct atom_buffer {
	TAILQ_ENTRY(atom_buffer) next;
	u_int8_t data[0];
};

struct lldpctl_atom_t {
	int count;
	atom_t type;
	lldpctl_conn_t *conn;
	TAILQ_HEAD(, atom_buffer) buffers; /* List of buffers */

	/* Destructor */
	void                 (*free)(lldpctl_atom_t *);

	/* Iterators */
	lldpctl_atom_iter_t *(*iter)(lldpctl_atom_t *);
	lldpctl_atom_iter_t *(*next)(lldpctl_atom_t *, lldpctl_atom_iter_t *);
	lldpctl_atom_t      *(*value)(lldpctl_atom_t *, lldpctl_atom_iter_t *);

	/* Getters */
	lldpctl_atom_t *(*get)(lldpctl_atom_t *, lldpctl_key_t);
	const char     *(*get_str)(lldpctl_atom_t *, lldpctl_key_t);
	const u_int8_t *(*get_buffer)(lldpctl_atom_t *, lldpctl_key_t, size_t *);
	long int        (*get_int)(lldpctl_atom_t *, lldpctl_key_t);

	/* Setters */
	lldpctl_atom_t *(*set)(lldpctl_atom_t *, lldpctl_key_t, lldpctl_atom_t *);
	lldpctl_atom_t *(*set_str)(lldpctl_atom_t *, lldpctl_key_t, const char *);
	lldpctl_atom_t *(*set_buffer)(lldpctl_atom_t *, lldpctl_key_t, const u_int8_t *, size_t);
	lldpctl_atom_t *(*set_int)(lldpctl_atom_t *, lldpctl_key_t, long int);
	lldpctl_atom_t *(*create)(lldpctl_atom_t *);
};

struct _lldpctl_atom_config_t {
	lldpctl_atom_t base;
	struct lldpd_config *config;
};

struct _lldpctl_atom_interfaces_list_t {
	lldpctl_atom_t base;
	struct lldpd_interface_list *ifs;
};

struct _lldpctl_atom_interface_t {
	lldpctl_atom_t base;
	char *name;
};

struct _lldpctl_atom_chassis_t {
	lldpctl_atom_t base;
	struct lldpd_chassis *chassis;
	struct _lldpctl_atom_port_t *parent; /* Optional: parent of this atom (owning our reference) */
	int embedded;			     /* This atom is "embedded" (not refcounted) */
};

struct _lldpctl_atom_port_t {
	lldpctl_atom_t base;
	int local;			 /* Local or remote port? */
	struct lldpd_hardware *hardware; /* Local port only (but optional) */
	struct lldpd_port     *port;	 /* Local and remote */
	struct _lldpctl_atom_port_t *parent; /* Local port if we are a remote port */
	lldpctl_atom_t *chassis; /* Internal atom for chassis */
};

/* Can represent any simple list holding just a reference to a port. */
struct _lldpctl_atom_any_list_t {
	lldpctl_atom_t base;
	struct _lldpctl_atom_port_t *parent;
};

struct _lldpctl_atom_mgmts_list_t {
	lldpctl_atom_t base;
	lldpctl_atom_t *parent;
	struct lldpd_chassis *chassis; /* Chassis containing the list of IP addresses */
};

struct _lldpctl_atom_mgmt_t {
	lldpctl_atom_t base;
	lldpctl_atom_t *parent;
	struct lldpd_mgmt *mgmt;
};

#ifdef ENABLE_DOT3
struct _lldpctl_atom_dot3_power_t {
	lldpctl_atom_t base;
	struct _lldpctl_atom_port_t *parent;
};
#endif

#ifdef ENABLE_DOT1
struct _lldpctl_atom_vlan_t {
	lldpctl_atom_t base;
	struct _lldpctl_atom_port_t *parent;
	struct lldpd_vlan *vlan;
};

struct _lldpctl_atom_ppvid_t {
	lldpctl_atom_t base;
	struct _lldpctl_atom_port_t *parent;
	struct lldpd_ppvid *ppvid;
};

struct _lldpctl_atom_pi_t {
	lldpctl_atom_t base;
	struct _lldpctl_atom_port_t *parent;
	struct lldpd_pi *pi;
};
#endif

#ifdef ENABLE_LLDPMED
struct _lldpctl_atom_med_policy_t {
	lldpctl_atom_t base;
	struct _lldpctl_atom_port_t *parent;
	struct lldpd_med_policy *policy;
};

struct _lldpctl_atom_med_location_t {
	lldpctl_atom_t base;
	struct _lldpctl_atom_port_t *parent;
	struct lldpd_med_loc *location;
};

/* This list should have the same structure than _llpdctl_atom_any_list_t */
struct _lldpctl_atom_med_caelements_list_t {
	lldpctl_atom_t base;
	struct _lldpctl_atom_med_location_t *parent;
};

struct _lldpctl_atom_med_caelement_t {
	lldpctl_atom_t base;
	struct _lldpctl_atom_med_location_t *parent;
	int type;
	uint8_t *value;
	size_t   len;
};

struct _lldpctl_atom_med_power_t {
	lldpctl_atom_t base;
	struct _lldpctl_atom_port_t *parent;
};
#endif

#ifdef ENABLE_CUSTOM
struct _lldpctl_atom_custom_list_t {
	lldpctl_atom_t base;
	struct _lldpctl_atom_port_t *parent;
	struct lldpd_custom_list *list;
};

struct _lldpctl_atom_custom_t {
	lldpctl_atom_t base;
	struct _lldpctl_atom_port_t *parent;
	int op;
	struct lldpd_custom *tlv;
};
#endif

struct lldpctl_atom_t *_lldpctl_new_atom(lldpctl_conn_t *conn, atom_t type, ...);

struct atom_map {
	int key;
	struct atom_map *next;
	lldpctl_map_t   map[];
};

void atom_map_register(struct atom_map *map, int);
void init_atom_map(void);

#define ATOM_MAP_REGISTER(NAME, PRIO) void init_atom_map_ ## NAME() { atom_map_register(& NAME, PRIO); }

struct atom_builder {
	atom_t type;	/* Atom type */
	size_t size;	/* Size of structure to allocate */
	int  (*init)(lldpctl_atom_t *, va_list); /* Optional additional init steps */
	void (*free)(lldpctl_atom_t *); /* Optional deallocation steps */

	lldpctl_atom_iter_t* (*iter)(lldpctl_atom_t *); /* Optional, return an iterator for this object */
	lldpctl_atom_iter_t* (*next)(lldpctl_atom_t *,  lldpctl_atom_iter_t *); /* Return the next object for the provided iterator */
	lldpctl_atom_t*      (*value)(lldpctl_atom_t *, lldpctl_atom_iter_t *); /* Return the current object for the provided iterator */

	lldpctl_atom_t*      (*get)(lldpctl_atom_t *,        lldpctl_key_t);
	const char*          (*get_str)(lldpctl_atom_t *,    lldpctl_key_t);
	const u_int8_t*      (*get_buffer)(lldpctl_atom_t *, lldpctl_key_t, size_t *);
	long int             (*get_int)(lldpctl_atom_t *,    lldpctl_key_t);

	lldpctl_atom_t*      (*set)(lldpctl_atom_t *, lldpctl_key_t, lldpctl_atom_t *);
	lldpctl_atom_t*      (*set_str)(lldpctl_atom_t *, lldpctl_key_t, const char *);
	lldpctl_atom_t*      (*set_buffer)(lldpctl_atom_t *, lldpctl_key_t, const u_int8_t *, size_t);
	lldpctl_atom_t*      (*set_int)(lldpctl_atom_t *, lldpctl_key_t, long int);
	lldpctl_atom_t*      (*create)(lldpctl_atom_t *);
	struct atom_builder  *nextb;
};

void atom_builder_register(struct atom_builder *builder, int);
void init_atom_builder(void);

#define ATOM_BUILDER_REGISTER(NAME, PRIO) void init_atom_builder_ ## NAME() { atom_builder_register(& NAME, PRIO); }
