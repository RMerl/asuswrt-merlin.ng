/* -*- mode: c; c-file-style: "openbsd" -*- */
/*
 * Copyright (c) 2008 Vincent Bernat <bernat@luffy.cx>
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

#include "lldpd.h"

#include <assert.h>

#include "agent.h"

#if HAVE_NET_SNMP_AGENT_UTIL_FUNCS_H
#include <net-snmp/agent/util_funcs.h>
#else
/* The above header may be buggy. We just need this function. */
int header_generic(struct variable *, oid *, size_t *, int,
		   size_t *, WriteMethod **);
#endif

/* For net-snmp */
extern int register_sysORTable(oid *, size_t, const char *);
extern int unregister_sysORTable(oid *, size_t);

/* Global variable because no way to pass it as argument. Should not be used
 * elsewhere. */
#define scfg agent_scfg
struct lldpd *agent_scfg;

static uint8_t
swap_bits(uint8_t n)
{
  n = ((n&0xF0) >>4 ) | ( (n&0x0F) <<4);
  n = ((n&0xCC) >>2 ) | ( (n&0x33) <<2);
  n = ((n&0xAA) >>1 ) | ( (n&0x55) <<1);

  return  n;
};

extern struct timeval starttime;
static long int
lastchange(struct lldpd_port *port)
{
	if (port->p_lastchange > starttime.tv_sec)
		return (port->p_lastchange - starttime.tv_sec)*100;
	return 0;
}

/* -------------
  Helper functions to build header_*indexed_table() functions.
  Those functions keep an internal state. They are not reentrant!
*/
struct header_index {
	struct variable *vp;
	oid             *name;	 /* Requested/returned OID */
	size_t          *length; /* Length of above OID */
	int              exact;
	oid              best[MAX_OID_LEN]; /* Best OID */
	size_t           best_len;	    /* Best OID length */
	void            *entity;	    /* Best entity */
};
static struct header_index header_idx;

static int
header_index_init(struct variable *vp, oid *name, size_t *length,
    int exact, size_t *var_len, WriteMethod **write_method)
{
	/* If the requested OID name is less than OID prefix we
	   handle, adjust it to our prefix. */
        if ((snmp_oid_compare(name, *length, vp->name, vp->namelen)) < 0) {
                memcpy(name, vp->name, sizeof(oid) * vp->namelen);
                *length = vp->namelen;
        }
	/* Now, we can only handle OID matching our prefix. Those two
	   tests are not really necessary since NetSNMP won't give us
	   OID "above" our prefix. But this makes unit tests
	   easier.  */
	if (*length < vp->namelen) return 0;
	if (memcmp(name, vp->name, vp->namelen * sizeof(oid))) return 0;

	if(write_method != NULL) *write_method = 0;
	*var_len = sizeof(long);

	/* Initialize our header index structure */
	header_idx.vp = vp;
	header_idx.name = name;
	header_idx.length = length;
	header_idx.exact = exact;
	header_idx.best_len = 0;
	header_idx.entity = NULL;
	return 1;
}

static int
header_index_add(oid *index, size_t len, void *entity)
{
	int      result;
	oid     *target;
	size_t   target_len;

        target = header_idx.name + header_idx.vp->namelen;
        target_len = *header_idx.length - header_idx.vp->namelen;
	if ((result = snmp_oid_compare(index, len, target, target_len)) < 0)
		return 0;	/* Too small. */
	if (result == 0)
		return header_idx.exact;
	if (header_idx.best_len == 0 ||
	    (snmp_oid_compare(index, len,
			      header_idx.best,
			      header_idx.best_len) < 0)) {
		memcpy(header_idx.best, index, sizeof(oid) * len);
		header_idx.best_len = len;
		header_idx.entity = entity;
	}
	return 0;		/* No best match yet. */	
}

void*
header_index_best()
{
	if (header_idx.entity == NULL)
		return NULL;
	if (header_idx.exact)
		return NULL;
	memcpy(header_idx.name + header_idx.vp->namelen,
	       header_idx.best, sizeof(oid) * header_idx.best_len);
	*header_idx.length = header_idx.vp->namelen + header_idx.best_len;
	return header_idx.entity;
}
/* ----------------------------- */

static struct lldpd_hardware*
header_portindexed_table(struct variable *vp, oid *name, size_t *length,
    int exact, size_t *var_len, WriteMethod **write_method)
{
	struct lldpd_hardware *hardware;

	if (!header_index_init(vp, name, length, exact, var_len, write_method)) return NULL;
	TAILQ_FOREACH(hardware, &scfg->g_hardware, h_entries) {
		oid index[1] = { hardware->h_ifindex };
		if (header_index_add(index, 1,
				     hardware))
			return hardware;
	}
	return header_index_best();
}

#ifdef ENABLE_LLDPMED
static struct lldpd_med_policy*
header_pmedindexed_policy_table(struct variable *vp, oid *name, size_t *length,
    int exact, size_t *var_len, WriteMethod **write_method)
{
	struct lldpd_hardware *hardware;
	int i;
	oid index[2];

	if (!header_index_init(vp, name, length, exact, var_len, write_method)) return NULL;
	TAILQ_FOREACH(hardware, &scfg->g_hardware, h_entries) {
		for (i = 0; i < LLDP_MED_APPTYPE_LAST; i++) {
			if (hardware->h_lport.p_med_policy[i].type != i+1)
				continue;
			index[0] = hardware->h_ifindex;
			index[1] = i + 1;
			if (header_index_add(index, 2,
					     &hardware->h_lport.p_med_policy[i]))
				return &hardware->h_lport.p_med_policy[i];
		}
	}
	return header_index_best();
}

static struct lldpd_med_loc*
header_pmedindexed_location_table(struct variable *vp, oid *name, size_t *length,
    int exact, size_t *var_len, WriteMethod **write_method)
{
	struct lldpd_hardware *hardware;
	int i;
	oid index[2];

	if (!header_index_init(vp, name, length, exact, var_len, write_method)) return NULL;
	TAILQ_FOREACH(hardware, &scfg->g_hardware, h_entries) {
		for (i = 0; i < LLDP_MED_LOCFORMAT_LAST; i++) {
			if (hardware->h_lport.p_med_location[i].format != i+1)
				continue;
			index[0] = hardware->h_ifindex;
			index[1] = i + 2;
			if (header_index_add(index, 2,
					     &hardware->h_lport.p_med_location[i]))
				return &hardware->h_lport.p_med_location[i];
		}
	}
	return header_index_best();
}
#endif

static struct lldpd_port*
header_tprindexed_table(struct variable *vp, oid *name, size_t *length,
			int exact, size_t *var_len, WriteMethod **write_method,
			int withmed)
{
	struct lldpd_hardware *hardware;
	struct lldpd_port *port;
	oid index[3];

	if (!header_index_init(vp, name, length, exact, var_len, write_method)) return NULL;
	TAILQ_FOREACH(hardware, &scfg->g_hardware, h_entries) {
		TAILQ_FOREACH(port, &hardware->h_rports, p_entries) {
			if (SMART_HIDDEN(port)) continue;
#ifdef ENABLE_LLDPMED
			if (withmed && !port->p_chassis->c_med_cap_available) continue;
#endif
			index[0] = lastchange(port);
			index[1] = hardware->h_ifindex;
			index[2] = port->p_chassis->c_index;
			if (header_index_add(index, 3,
					     port))
				return port;
		}
	}
	return header_index_best();
}

static struct lldpd_mgmt*
header_ipindexed_table(struct variable *vp, oid *name, size_t *length,
    int exact, size_t *var_len, WriteMethod **write_method)
{
	struct lldpd_chassis *chassis = LOCAL_CHASSIS(scfg);
	struct lldpd_mgmt *mgmt;
	oid index[2 + 16];

	if (!header_index_init(vp, name, length, exact, var_len, write_method)) return NULL;
	TAILQ_FOREACH(mgmt, &chassis->c_mgmt, m_entries) {
		int i;
		switch (mgmt->m_family) {
		case LLDPD_AF_IPV4: index[0] = 1; break;
		case LLDPD_AF_IPV6: index[0] = 2; break;
		default: assert(0);
		}
		index[1] = mgmt->m_addrsize;
		if (index[1] > sizeof(index) - 2)
			continue; /* Odd... */
		for (i = 0; i < index[1]; i++)
			index[i + 2] = mgmt->m_addr.octets[i];
		if (header_index_add(index, 2 + index[1], mgmt))
			return mgmt;
	}

	return header_index_best();
}

static struct lldpd_mgmt*
header_tpripindexed_table(struct variable *vp, oid *name, size_t *length,
    int exact, size_t *var_len, WriteMethod **write_method)
{
	struct lldpd_hardware *hardware;
	struct lldpd_port *port;
	struct lldpd_mgmt *mgmt;
	oid index[5 + 16];

	if (!header_index_init(vp, name, length, exact, var_len, write_method)) return NULL;
	TAILQ_FOREACH(hardware, &scfg->g_hardware, h_entries) {
		TAILQ_FOREACH(port, &hardware->h_rports, p_entries) {
			if (SMART_HIDDEN(port)) continue;
			TAILQ_FOREACH(mgmt, &port->p_chassis->c_mgmt, m_entries) {
				int i;
				index[0] = lastchange(port);
				index[1] = hardware->h_ifindex;
				index[2] = port->p_chassis->c_index;
				switch (mgmt->m_family) {
				case LLDPD_AF_IPV4: index[3] = 1; break;
				case LLDPD_AF_IPV6: index[3] = 2; break;
				default: assert(0);
				}
				index[4] = mgmt->m_addrsize;
				if (index[4] > sizeof(index) - 5)
					continue; /* Odd... */
				for (i = 0; i < index[4]; i++)
					index[i + 5] = mgmt->m_addr.octets[i];
				if (header_index_add(index, 5 + index[4], mgmt))
					return mgmt;
			}
		}
	}
	return header_index_best();
}

#ifdef ENABLE_CUSTOM
static struct lldpd_custom*
header_tprcustomindexed_table(struct variable *vp, oid *name, size_t *length,
    int exact, size_t *var_len, WriteMethod **write_method)
{
	struct lldpd_hardware *hardware;
	struct lldpd_port *port;
	struct lldpd_custom *custom;
	oid index[8];
	oid idx;

	if (!header_index_init(vp, name, length, exact, var_len, write_method)) return NULL;
	TAILQ_FOREACH(hardware, &scfg->g_hardware, h_entries) {
		TAILQ_FOREACH(port, &hardware->h_rports, p_entries) {
			if (SMART_HIDDEN(port)) continue;
			idx = 1;
			TAILQ_FOREACH(custom, &port->p_custom_list, next) {
				index[0] = lastchange(port);
				index[1] = hardware->h_ifindex;
				index[2] = port->p_chassis->c_index;
				index[3] = custom->oui[0];
				index[4] = custom->oui[1];
				index[5] = custom->oui[2];
				index[6] = custom->subtype;
				index[7] = idx++;
				if (header_index_add(index, 8, custom))
					return custom;
			}
		}
	}
	return header_index_best();
}
#endif

#ifdef ENABLE_LLDPMED
#define TPR_VARIANT_MED_POLICY 2
#define TPR_VARIANT_MED_LOCATION 3
static void*
header_tprmedindexed_table(struct variable *vp, oid *name, size_t *length,
    int exact, size_t *var_len, WriteMethod **write_method, int variant)
{
	struct lldpd_hardware *hardware;
	struct lldpd_port *port;
	int j;
	oid index[4];

	if (!header_index_init(vp, name, length, exact, var_len, write_method)) return NULL;
	TAILQ_FOREACH(hardware, &scfg->g_hardware, h_entries) {
		TAILQ_FOREACH(port, &hardware->h_rports, p_entries) {
			if (SMART_HIDDEN(port)) continue;
			if (!port->p_chassis->c_med_cap_available) continue;
			switch (variant) {
			case TPR_VARIANT_MED_POLICY:
				for (j = 0;
				     j < LLDP_MED_APPTYPE_LAST;
				     j++) {
					if (port->p_med_policy[j].type != j+1)
						continue;
					index[0] = lastchange(port);
					index[1] = hardware->h_ifindex;
					index[2] = port->p_chassis->c_index;
					index[3] = j+1;
					if (header_index_add(index, 4,
							     &port->p_med_policy[j]))
						return &port->p_med_policy[j];
				}
				break;
			case TPR_VARIANT_MED_LOCATION:
				for (j = 0;
				     j < LLDP_MED_LOCFORMAT_LAST;
				     j++) {
					if (port->p_med_location[j].format != j+1)
						continue;
					index[0] = lastchange(port);
					index[1] = hardware->h_ifindex;
					index[2] = port->p_chassis->c_index;
					index[3] = j+2;
					if (header_index_add(index, 4,
							     &port->p_med_location[j]))
						return &port->p_med_location[j];
				}
				break;
			}
		}
	}
	return header_index_best();
}
#endif

#ifdef ENABLE_DOT1
static struct lldpd_vlan*
header_pvindexed_table(struct variable *vp, oid *name, size_t *length,
    int exact, size_t *var_len, WriteMethod **write_method)
{
	struct lldpd_hardware *hardware;
        struct lldpd_vlan *vlan;

	if (!header_index_init(vp, name, length, exact, var_len, write_method)) return NULL;
	TAILQ_FOREACH(hardware, &scfg->g_hardware, h_entries) {
		TAILQ_FOREACH(vlan, &hardware->h_lport.p_vlans, v_entries) {
			oid index[2] = { hardware->h_ifindex,
					 vlan->v_vid };
			if (header_index_add(index, 2, vlan))
				return vlan;
		}
	}
	return header_index_best();
}

static struct lldpd_vlan*
header_tprvindexed_table(struct variable *vp, oid *name, size_t *length,
    int exact, size_t *var_len, WriteMethod **write_method)
{
	struct lldpd_hardware *hardware;
	struct lldpd_port *port;
        struct lldpd_vlan *vlan;

	if (!header_index_init(vp, name, length, exact, var_len, write_method)) return NULL;
	TAILQ_FOREACH(hardware, &scfg->g_hardware, h_entries) {
		TAILQ_FOREACH(port, &hardware->h_rports, p_entries) {
			if (SMART_HIDDEN(port)) continue;
                        TAILQ_FOREACH(vlan, &port->p_vlans, v_entries) {
				oid index[4] = { lastchange(port),
						 hardware->h_ifindex,
						 port->p_chassis->c_index,
						 vlan->v_vid };
				if (header_index_add(index, 4,
						     vlan))
					return vlan;
			}
		}
	}
	return header_index_best();
}

static struct lldpd_ppvid*
header_pppvidindexed_table(struct variable *vp, oid *name, size_t *length,
    int exact, size_t *var_len, WriteMethod **write_method)
{
	struct lldpd_hardware *hardware;
        struct lldpd_ppvid *ppvid;

	if (!header_index_init(vp, name, length, exact, var_len, write_method)) return NULL;
	TAILQ_FOREACH(hardware, &scfg->g_hardware, h_entries) {
		TAILQ_FOREACH(ppvid, &hardware->h_lport.p_ppvids, p_entries) {
			oid index[2] = { hardware->h_ifindex,
					 ppvid->p_ppvid };
			if (header_index_add(index, 2,
					     ppvid))
				return ppvid;
		}
	}
	return header_index_best();
}

static struct lldpd_ppvid*
header_tprppvidindexed_table(struct variable *vp, oid *name, size_t *length,
    int exact, size_t *var_len, WriteMethod **write_method)
{
	struct lldpd_hardware *hardware;
	struct lldpd_port *port;
        struct lldpd_ppvid *ppvid;

	if (!header_index_init(vp, name, length, exact, var_len, write_method)) return NULL;
	TAILQ_FOREACH(hardware, &scfg->g_hardware, h_entries) {
		TAILQ_FOREACH(port, &hardware->h_rports, p_entries) {
			if (SMART_HIDDEN(port)) continue;
                        TAILQ_FOREACH(ppvid, &port->p_ppvids, p_entries) {
				oid index[4] = { lastchange(port),
						 hardware->h_ifindex,
						 port->p_chassis->c_index,
						 ppvid->p_ppvid };
				if (header_index_add(index, 4,
						     ppvid))
					return ppvid;
                        }
		}
	}
	return header_index_best();
}

static struct lldpd_pi*
header_ppiindexed_table(struct variable *vp, oid *name, size_t *length,
			int exact, size_t *var_len, WriteMethod **write_method)
{
	struct lldpd_hardware *hardware;
        struct lldpd_pi *pi;

	if (!header_index_init(vp, name, length, exact, var_len, write_method)) return NULL;
	TAILQ_FOREACH(hardware, &scfg->g_hardware, h_entries) {
		TAILQ_FOREACH(pi, &hardware->h_lport.p_pids, p_entries) {
			oid index[2] = { hardware->h_ifindex,
					 frame_checksum((const u_char*)pi->p_pi,
					     pi->p_pi_len, 0) };
			if (header_index_add(index, 2,
					     pi))
				return pi;
		}
	}
	return header_index_best();
}

static struct lldpd_pi*
header_tprpiindexed_table(struct variable *vp, oid *name, size_t *length,
			  int exact, size_t *var_len, WriteMethod **write_method)
{
	struct lldpd_hardware *hardware;
	struct lldpd_port *port;
        struct lldpd_pi *pi;

	if (!header_index_init(vp, name, length, exact, var_len, write_method)) return NULL;
	TAILQ_FOREACH(hardware, &scfg->g_hardware, h_entries) {
		TAILQ_FOREACH(port, &hardware->h_rports, p_entries) {
			if (SMART_HIDDEN(port)) continue;
                        TAILQ_FOREACH(pi, &port->p_pids, p_entries) {
				oid index[4] = { lastchange(port),
						 hardware->h_ifindex,
						 port->p_chassis->c_index,
						 frame_checksum((const u_char *)pi->p_pi,
						     pi->p_pi_len, 0) };
				if (header_index_add(index, 4,
						     pi))
					return pi;
                        }
		}
	}
	return header_index_best();
}
#endif

/* Scalars */
#define LLDP_SNMP_TXINTERVAL 1
#define LLDP_SNMP_TXMULTIPLIER 2
#define LLDP_SNMP_REINITDELAY 3
#define LLDP_SNMP_TXDELAY 4
#define LLDP_SNMP_NOTIFICATION 5
#define LLDP_SNMP_LASTUPDATE 6
#define LLDP_SNMP_STATS_INSERTS 7
#define LLDP_SNMP_STATS_DELETES 8
#define LLDP_SNMP_STATS_DROPS 9
#define LLDP_SNMP_STATS_AGEOUTS 10
/* Chassis */
#define LLDP_SNMP_CIDSUBTYPE 1
#define LLDP_SNMP_CID 2
#define LLDP_SNMP_SYSNAME 3
#define LLDP_SNMP_SYSDESCR 4
#define LLDP_SNMP_SYSCAP_SUP 5
#define LLDP_SNMP_SYSCAP_ENA 6
/* Stats */
#define LLDP_SNMP_STATS_TX 2
#define LLDP_SNMP_STATS_RX_DISCARDED 4
#define LLDP_SNMP_STATS_RX_ERRORS 5
#define LLDP_SNMP_STATS_RX 6
#define LLDP_SNMP_STATS_RX_TLVDISCARDED 7
#define LLDP_SNMP_STATS_RX_TLVUNRECOGNIZED 8
#define LLDP_SNMP_STATS_RX_AGEOUTS 9
/* Ports */
#define LLDP_SNMP_PIDSUBTYPE 2
#define LLDP_SNMP_PID 3
#define LLDP_SNMP_PORTDESC 4
#define LLDP_SNMP_DOT3_AUTONEG_SUPPORT 5
#define LLDP_SNMP_DOT3_AUTONEG_ENABLED 6
#define LLDP_SNMP_DOT3_AUTONEG_ADVERTISED 7
#define LLDP_SNMP_DOT3_AUTONEG_MAU 8
#define LLDP_SNMP_DOT3_AGG_STATUS 9
#define LLDP_SNMP_DOT3_AGG_ID 10
#define LLDP_SNMP_DOT3_MFS 11
#define LLDP_SNMP_DOT3_POWER_DEVICETYPE 12
#define LLDP_SNMP_DOT3_POWER_SUPPORT 13
#define LLDP_SNMP_DOT3_POWER_ENABLED 14
#define LLDP_SNMP_DOT3_POWER_PAIRCONTROL 15
#define LLDP_SNMP_DOT3_POWER_PAIRS 16
#define LLDP_SNMP_DOT3_POWER_CLASS 17
#define LLDP_SNMP_DOT3_POWER_TYPE 18
#define LLDP_SNMP_DOT3_POWER_SOURCE 19
#define LLDP_SNMP_DOT3_POWER_PRIORITY 20
#define LLDP_SNMP_DOT3_POWER_REQUESTED 21
#define LLDP_SNMP_DOT3_POWER_ALLOCATED 22
#define LLDP_SNMP_DOT1_PVID 23
/* Vlans */
#define LLDP_SNMP_DOT1_VLANNAME 1
/* Protocol VLAN IDs */
#define LLDP_SNMP_DOT1_PPVLAN_SUPPORTED	2
#define LLDP_SNMP_DOT1_PPVLAN_ENABLED	3
/* Protocol Identity */
#define LLDP_SNMP_DOT1_PI			1
/* Management address */
#define LLDP_SNMP_ADDR_LEN 1
#define LLDP_SNMP_ADDR_IFSUBTYPE 2
#define LLDP_SNMP_ADDR_IFID 3
#define LLDP_SNMP_ADDR_OID 4
/* Custom TLVs */
#define LLDP_SNMP_ORG_DEF_INFO 1
/* LLDP-MED */
#define LLDP_SNMP_MED_CAP_AVAILABLE 1
#define LLDP_SNMP_MED_CAP_ENABLED 2
#define LLDP_SNMP_MED_CLASS 3
#define LLDP_SNMP_MED_HW 4
#define LLDP_SNMP_MED_FW 5
#define LLDP_SNMP_MED_SW 6
#define LLDP_SNMP_MED_SN 7
#define LLDP_SNMP_MED_MANUF 8
#define LLDP_SNMP_MED_MODEL 9
#define LLDP_SNMP_MED_ASSET 10
#define LLDP_SNMP_MED_POLICY_VID 11
#define LLDP_SNMP_MED_POLICY_PRIO 12
#define LLDP_SNMP_MED_POLICY_DSCP 13
#define LLDP_SNMP_MED_POLICY_UNKNOWN 14
#define LLDP_SNMP_MED_POLICY_TAGGED 15
#define LLDP_SNMP_MED_LOCATION 16
#define LLDP_SNMP_MED_POE_DEVICETYPE 17
#define LLDP_SNMP_MED_POE_PSE_POWERVAL 19
#define LLDP_SNMP_MED_POE_PSE_POWERSOURCE 20
#define LLDP_SNMP_MED_POE_PSE_POWERPRIORITY 21
#define LLDP_SNMP_MED_POE_PD_POWERVAL 22
#define LLDP_SNMP_MED_POE_PD_POWERSOURCE 23
#define LLDP_SNMP_MED_POE_PD_POWERPRIORITY 24

/* The following macro should be used anytime where the selected OID
   is finally not returned (for example, when the associated data is
   not available). In this case, we retry the function with the next
   OID. */
#define TRYNEXT(X)							\
	do {								\
		if (!exact && (name[*length-1] < MAX_SUBID))		\
			return X(vp, name, length,			\
				 exact, var_len, write_method);		\
		return NULL;						\
	} while (0)


static u_char*
agent_h_scalars(struct variable *vp, oid *name, size_t *length,
    int exact, size_t *var_len, WriteMethod **write_method)
{
	static unsigned long long_ret;
	struct lldpd_hardware *hardware;
	struct lldpd_port *port;

	if (header_generic(vp, name, length, exact, var_len, write_method))
		return NULL;

	switch (vp->magic) {
	case LLDP_SNMP_TXINTERVAL:
                long_ret = (scfg->g_config.c_tx_interval+999) / 1000;
		return (u_char *)&long_ret;
	case LLDP_SNMP_TXMULTIPLIER:
		long_ret = scfg->g_config.c_tx_hold;
		return (u_char *)&long_ret;
	case LLDP_SNMP_REINITDELAY:
		long_ret = 1;
		return (u_char *)&long_ret;
	case LLDP_SNMP_TXDELAY:
		long_ret = LLDPD_TX_MSGDELAY;
		return (u_char *)&long_ret;
	case LLDP_SNMP_NOTIFICATION:
		long_ret = 5;
		return (u_char *)&long_ret;
	case LLDP_SNMP_LASTUPDATE:
		long_ret = 0;
		TAILQ_FOREACH(hardware, &scfg->g_hardware, h_entries) {
			/* Check if the last removal of a remote port on this local port was the last change. */
			if (hardware->h_lport.p_lastremove > long_ret)
				long_ret = hardware->h_lport.p_lastremove;
			/* Check if any change on the existing remote ports was the last change. */
			TAILQ_FOREACH(port, &hardware->h_rports, p_entries) {
				if (SMART_HIDDEN(port)) continue;
				if (port->p_lastchange > long_ret)
					long_ret = port->p_lastchange;
			}
		}
		if (long_ret)
			long_ret = (long_ret - starttime.tv_sec) * 100;
		return (u_char *)&long_ret;
	case LLDP_SNMP_STATS_INSERTS:
		/* We assume this is equal to valid frames received on all ports */
		long_ret = 0;
		TAILQ_FOREACH(hardware, &scfg->g_hardware, h_entries)
		    long_ret += hardware->h_insert_cnt;
		return (u_char *)&long_ret;
	case LLDP_SNMP_STATS_AGEOUTS:
		long_ret = 0;
		TAILQ_FOREACH(hardware, &scfg->g_hardware, h_entries)
		    long_ret += hardware->h_ageout_cnt;
		return (u_char *)&long_ret;
	case LLDP_SNMP_STATS_DELETES:
		long_ret = 0;
		TAILQ_FOREACH(hardware, &scfg->g_hardware, h_entries)
		    long_ret += hardware->h_delete_cnt;
		return (u_char *)&long_ret;
	case LLDP_SNMP_STATS_DROPS:
		long_ret = 0;
		TAILQ_FOREACH(hardware, &scfg->g_hardware, h_entries)
		    long_ret += hardware->h_drop_cnt;
		return (u_char *)&long_ret;
	default:
		break;
	}
	return NULL;
}

#ifdef ENABLE_LLDPMED
static u_char*
agent_v_med_power(struct variable *vp, size_t *var_len, struct lldpd_med_power *power)
{
	static unsigned long long_ret;

	switch (vp->magic) {
	case LLDP_SNMP_MED_POE_DEVICETYPE:
		switch (power->devicetype) {
		case LLDP_MED_POW_TYPE_PSE:
			long_ret = 2; break;
		case LLDP_MED_POW_TYPE_PD:
			long_ret = 3; break;
		case 0:
			long_ret = 4; break;
		default:
			long_ret = 1;
		}
		return (u_char *)&long_ret;
	case LLDP_SNMP_MED_POE_PSE_POWERVAL:
	case LLDP_SNMP_MED_POE_PD_POWERVAL:
		if (((vp->magic == LLDP_SNMP_MED_POE_PSE_POWERVAL) &&
			(power->devicetype ==
			LLDP_MED_POW_TYPE_PSE)) ||
		    ((vp->magic == LLDP_SNMP_MED_POE_PD_POWERVAL) &&
			(power->devicetype ==
			    LLDP_MED_POW_TYPE_PD))) {
			long_ret = power->val;
			return (u_char *)&long_ret;
		}
		break;
	case LLDP_SNMP_MED_POE_PSE_POWERSOURCE:
		if (power->devicetype ==
		    LLDP_MED_POW_TYPE_PSE) {
			switch (power->source) {
			case LLDP_MED_POW_SOURCE_PRIMARY:
				long_ret = 2; break;
			case LLDP_MED_POW_SOURCE_BACKUP:
				long_ret = 3; break;
			default:
				long_ret = 1;
			}
			return (u_char *)&long_ret;
		}
		break;
	case LLDP_SNMP_MED_POE_PD_POWERSOURCE:
		if (power->devicetype ==
		    LLDP_MED_POW_TYPE_PD) {
			switch (power->source) {
			case LLDP_MED_POW_SOURCE_PSE:
				long_ret = 2; break;
			case LLDP_MED_POW_SOURCE_LOCAL:
				long_ret = 3; break;
			case LLDP_MED_POW_SOURCE_BOTH:
				long_ret = 4; break;
			default:
				long_ret = 1;
			}
			return (u_char *)&long_ret;
		}
		break;
	case LLDP_SNMP_MED_POE_PSE_POWERPRIORITY:
	case LLDP_SNMP_MED_POE_PD_POWERPRIORITY:
		if (((vp->magic == LLDP_SNMP_MED_POE_PSE_POWERPRIORITY) &&
			(power->devicetype ==
			LLDP_MED_POW_TYPE_PSE)) ||
		    ((vp->magic == LLDP_SNMP_MED_POE_PD_POWERPRIORITY) &&
			(power->devicetype ==
			    LLDP_MED_POW_TYPE_PD))) {
			switch (power->priority) {
			case LLDP_MED_POW_PRIO_CRITICAL:
				long_ret = 2; break;
			case LLDP_MED_POW_PRIO_HIGH:
				long_ret = 3; break;
			case LLDP_MED_POW_PRIO_LOW:
				long_ret = 4; break;
			default:
				long_ret = 1;
			}
			return (u_char *)&long_ret;
		}
		break;
	}

	return NULL;
}
static u_char*
agent_h_local_med_power(struct variable *vp, oid *name, size_t *length,
    int exact, size_t *var_len, WriteMethod **write_method)
{
	struct lldpd_med_power *power = NULL;
	struct lldpd_hardware  *hardware;
	int                     pse   = 0;

	if (!LOCAL_CHASSIS(scfg)->c_med_cap_available)
		return NULL;
	if (header_generic(vp, name, length, exact, var_len, write_method))
		return NULL;

	/* LLDP-MED requires only one device type for all
	   ports. Moreover, a PSE can only have one power source. At
	   least, all PD values are global and not per-port. We try to
	   do our best. For device type, we decide on the number of
	   PD/PSE ports. */
	TAILQ_FOREACH(hardware, &scfg->g_hardware, h_entries) {
		if (hardware->h_lport.p_med_power.devicetype ==
		    LLDP_MED_POW_TYPE_PSE) {
			pse++;
			if (pse == 1) /* Take this port as a reference */
				power = &hardware->h_lport.p_med_power;
		} else if (hardware->h_lport.p_med_power.devicetype ==
			   LLDP_MED_POW_TYPE_PD) {
			pse--;
			if (pse == -1) /* Take this one instead */
				power = &hardware->h_lport.p_med_power;
		}
	}
	if (power) {
		u_char *a;
		if ((a = agent_v_med_power(vp, var_len, power)) != NULL)
			return a;
	}
	TRYNEXT(agent_h_local_med_power);
}
static u_char*
agent_h_remote_med_power(struct variable *vp, oid *name, size_t *length,
    int exact, size_t *var_len, WriteMethod **write_method)
{
	struct lldpd_port *port;
	u_char *a;

	if ((port = header_tprindexed_table(vp, name, length,
					    exact, var_len, write_method, 1)) == NULL)
		return NULL;

	if ((a = agent_v_med_power(vp, var_len, &port->p_med_power)) != NULL)
		return a;
	TRYNEXT(agent_h_remote_med_power);
}

static u_char*
agent_v_med(struct variable *vp, size_t *var_len,
	    struct lldpd_chassis *chassis,
	    struct lldpd_port *port)
{
        static unsigned long long_ret;
	static uint8_t bit;

	switch (vp->magic) {
        case LLDP_SNMP_MED_CLASS:
                long_ret = chassis->c_med_type;
		return (u_char *)&long_ret;
	case LLDP_SNMP_MED_CAP_AVAILABLE:
		*var_len = 1;
		bit = swap_bits(chassis->c_med_cap_available);
		return (u_char *)&bit;
	case LLDP_SNMP_MED_CAP_ENABLED:
		if (!port) break;
		*var_len = 1;
		bit = swap_bits(port->p_med_cap_enabled);
		return (u_char *)&bit;

#define LLDP_H_MED(magic, variable)				\
		case magic:						\
		    if (chassis->variable) {		\
			    *var_len = strlen(				\
				    chassis->variable);	\
			    return (u_char *)				\
				chassis->variable;		\
		    }							\
		break

	LLDP_H_MED(LLDP_SNMP_MED_HW,
	    c_med_hw);
	LLDP_H_MED(LLDP_SNMP_MED_SW,
	    c_med_sw);
	LLDP_H_MED(LLDP_SNMP_MED_FW,
	    c_med_fw);
	LLDP_H_MED(LLDP_SNMP_MED_SN,
	    c_med_sn);
	LLDP_H_MED(LLDP_SNMP_MED_MANUF,
	    c_med_manuf);
	LLDP_H_MED(LLDP_SNMP_MED_MODEL,
	    c_med_model);
	LLDP_H_MED(LLDP_SNMP_MED_ASSET,
	    c_med_asset);

        }
	return NULL;
}
static u_char*
agent_h_local_med(struct variable *vp, oid *name, size_t *length,
    int exact, size_t *var_len, WriteMethod **write_method)
{
	u_char *a;

	if (!LOCAL_CHASSIS(scfg)->c_med_cap_available)
		return NULL;
	if (header_generic(vp, name, length, exact, var_len, write_method))
		return NULL;

	if ((a = agent_v_med(vp, var_len,
			     LOCAL_CHASSIS(scfg), NULL)) != NULL)
		return a;
	TRYNEXT(agent_h_local_med);
}

static u_char*
agent_h_remote_med(struct variable *vp, oid *name, size_t *length,
    int exact, size_t *var_len, WriteMethod **write_method)
{
	struct lldpd_port *port;
	u_char *a;

	if ((port = header_tprindexed_table(vp, name, length,
					    exact, var_len, write_method, 1)) == NULL)
		return NULL;

	if ((a = agent_v_med(vp, var_len,
			     port->p_chassis, port)) != NULL)
		return a;
	TRYNEXT(agent_h_remote_med);
}

static u_char*
agent_v_med_policy(struct variable *vp, size_t *var_len,
		   struct lldpd_med_policy *policy)
{
        static unsigned long long_ret;

	switch (vp->magic) {
        case LLDP_SNMP_MED_POLICY_VID:
                long_ret = policy->vid;
		return (u_char *)&long_ret;
	case LLDP_SNMP_MED_POLICY_PRIO:
		long_ret = policy->priority;
		return (u_char *)&long_ret;
	case LLDP_SNMP_MED_POLICY_DSCP:
		long_ret = policy->dscp;
		return (u_char *)&long_ret;
	case LLDP_SNMP_MED_POLICY_UNKNOWN:
		long_ret = policy->unknown?1:2;
		return (u_char *)&long_ret;
	case LLDP_SNMP_MED_POLICY_TAGGED:
		long_ret = policy->tagged?1:2;
		return (u_char *)&long_ret;
	default:
		return NULL;
        }
}
static u_char*
agent_h_remote_med_policy(struct variable *vp, oid *name, size_t *length,
    int exact, size_t *var_len, WriteMethod **write_method)
{
	struct lldpd_med_policy *policy;

	if ((policy = (struct lldpd_med_policy *)header_tprmedindexed_table(vp, name, length,
		    exact, var_len, write_method, TPR_VARIANT_MED_POLICY)) == NULL)
		return NULL;

	return agent_v_med_policy(vp, var_len, policy);
}
static u_char*
agent_h_local_med_policy(struct variable *vp, oid *name, size_t *length,
    int exact, size_t *var_len, WriteMethod **write_method)
{
	struct lldpd_med_policy *policy;

	if ((policy = (struct lldpd_med_policy *)header_pmedindexed_policy_table(vp, name, length,
		    exact, var_len, write_method)) == NULL)
		return NULL;

	return agent_v_med_policy(vp, var_len, policy);
}

static u_char*
agent_v_med_location(struct variable *vp, size_t *var_len,
		     struct lldpd_med_loc *location)
{
	switch (vp->magic) {
        case LLDP_SNMP_MED_LOCATION:
		*var_len = location->data_len;
		return (u_char *)location->data;
	default:
		return NULL;
        }
}
static u_char*
agent_h_remote_med_location(struct variable *vp, oid *name, size_t *length,
    int exact, size_t *var_len, WriteMethod **write_method)
{
	struct lldpd_med_loc *location;

	if ((location = (struct lldpd_med_loc *)header_tprmedindexed_table(vp, name, length,
		    exact, var_len, write_method, TPR_VARIANT_MED_LOCATION)) == NULL)
		return NULL;

	return agent_v_med_location(vp, var_len, location);
}
static u_char*
agent_h_local_med_location(struct variable *vp, oid *name, size_t *length,
    int exact, size_t *var_len, WriteMethod **write_method)
{
	struct lldpd_med_loc *location;

	if ((location = (struct lldpd_med_loc *)header_pmedindexed_location_table(vp, name, length,
		    exact, var_len, write_method)) == NULL)
		return NULL;

	return agent_v_med_location(vp, var_len, location);
}
#endif

static u_char*
agent_v_chassis(struct variable *vp, size_t *var_len,
		struct lldpd_chassis *chassis)
{
	static uint8_t bit;
        static unsigned long long_ret;

	switch (vp->magic) {
	case LLDP_SNMP_CIDSUBTYPE:
                long_ret = chassis->c_id_subtype;
		return (u_char *)&long_ret;
	case LLDP_SNMP_CID:
		*var_len = chassis->c_id_len;
		return (u_char *)chassis->c_id;
	case LLDP_SNMP_SYSNAME:
		if (!chassis->c_name || *chassis->c_name == '\0') break;
		*var_len = strlen(chassis->c_name);
		return (u_char *)chassis->c_name;
	case LLDP_SNMP_SYSDESCR:
		if (!chassis->c_descr || *chassis->c_descr == '\0') break;
		*var_len = strlen(chassis->c_descr);
		return (u_char *)chassis->c_descr;
	case LLDP_SNMP_SYSCAP_SUP:
		*var_len = 1;
		bit = swap_bits(chassis->c_cap_available);
		return (u_char *)&bit;
	case LLDP_SNMP_SYSCAP_ENA:
		*var_len = 1;
		bit = swap_bits(chassis->c_cap_enabled);
		return (u_char *)&bit;
	default:
		break;
        }
	return NULL;
}
static u_char*
agent_h_local_chassis(struct variable *vp, oid *name, size_t *length,
    int exact, size_t *var_len, WriteMethod **write_method)
{
	u_char *a;

	if (header_generic(vp, name, length, exact, var_len, write_method))
		return NULL;

	if ((a = agent_v_chassis(vp, var_len, LOCAL_CHASSIS(scfg))) != NULL)
		return a;
	TRYNEXT(agent_h_local_chassis);
}
static u_char*
agent_h_remote_chassis(struct variable *vp, oid*name, size_t *length,
    int exact, size_t *var_len, WriteMethod **write_method)
{
	struct lldpd_port *port;
	u_char *a;

	if ((port = header_tprindexed_table(vp, name, length,
					    exact, var_len, write_method, 0)) == NULL)
		return NULL;

	if ((a = agent_v_chassis(vp, var_len, port->p_chassis)) != NULL)
		return a;
	TRYNEXT(agent_h_remote_chassis);
}

static u_char*
agent_h_stats(struct variable *vp, oid *name, size_t *length,
    int exact, size_t *var_len, WriteMethod **write_method)
{
	static unsigned long long_ret;
	struct lldpd_hardware *hardware;

	if ((hardware = header_portindexed_table(vp, name, length,
		    exact, var_len, write_method)) == NULL)
		return NULL;

	switch (vp->magic) {
	case LLDP_SNMP_STATS_TX:
                long_ret = hardware->h_tx_cnt;
                return (u_char *)&long_ret;
	case LLDP_SNMP_STATS_RX:
                long_ret = hardware->h_rx_cnt;
		return (u_char *)&long_ret;
	case LLDP_SNMP_STATS_RX_DISCARDED:
	case LLDP_SNMP_STATS_RX_ERRORS:
		/* We discard only frame with errors. Therefore, the two values
		 * are equal */
                long_ret = hardware->h_rx_discarded_cnt;
		return (u_char *)&long_ret;
	case LLDP_SNMP_STATS_RX_TLVDISCARDED:
	case LLDP_SNMP_STATS_RX_TLVUNRECOGNIZED:
		/* We discard only unrecognized TLV. Malformed TLV
		   implies dropping the whole frame */
		long_ret = hardware->h_rx_unrecognized_cnt;
		return (u_char *)&long_ret;
	case LLDP_SNMP_STATS_RX_AGEOUTS:
                long_ret = hardware->h_ageout_cnt;
		return (u_char *)&long_ret;
	default:
		return NULL;
        }
}

#ifdef ENABLE_DOT1
static u_char*
agent_v_vlan(struct variable *vp, size_t *var_len, struct lldpd_vlan *vlan)
{
	switch (vp->magic) {
	case LLDP_SNMP_DOT1_VLANNAME:
		*var_len = strlen(vlan->v_name);
		return (u_char *)vlan->v_name;
	default:
		return NULL;
        }
}
static u_char*
agent_h_local_vlan(struct variable *vp, oid *name, size_t *length,
    int exact, size_t *var_len, WriteMethod **write_method)
{
	struct lldpd_vlan *vlan;

	if ((vlan = header_pvindexed_table(vp, name, length,
		    exact, var_len, write_method)) == NULL)
		return NULL;

	return agent_v_vlan(vp, var_len, vlan);
}
static u_char*
agent_h_remote_vlan(struct variable *vp, oid *name, size_t *length,
    int exact, size_t *var_len, WriteMethod **write_method)
{
	struct lldpd_vlan *vlan;

	if ((vlan = header_tprvindexed_table(vp, name, length,
		    exact, var_len, write_method)) == NULL)
		return NULL;

	return agent_v_vlan(vp, var_len, vlan);
}

static u_char*
agent_v_ppvid(struct variable *vp, size_t *var_len, struct lldpd_ppvid *ppvid)
{
	static unsigned long long_ret;

	switch (vp->magic) {
	case LLDP_SNMP_DOT1_PPVLAN_SUPPORTED:
		long_ret = (ppvid->p_cap_status & LLDP_PPVID_CAP_SUPPORTED)?1:2;
		return (u_char *)&long_ret;
	case LLDP_SNMP_DOT1_PPVLAN_ENABLED:
		long_ret = (ppvid->p_cap_status & LLDP_PPVID_CAP_ENABLED)?1:2;
		return (u_char *)&long_ret;
	default:
		return NULL;
        }
}
static u_char*
agent_h_local_ppvid(struct variable *vp, oid *name, size_t *length,
    int exact, size_t *var_len, WriteMethod **write_method)
{
	struct lldpd_ppvid *ppvid;

	if ((ppvid = header_pppvidindexed_table(vp, name, length,
		    exact, var_len, write_method)) == NULL)
		return NULL;

	return agent_v_ppvid(vp, var_len, ppvid);
}

static u_char*
agent_h_remote_ppvid(struct variable *vp, oid *name, size_t *length,
    int exact, size_t *var_len, WriteMethod **write_method)
{
	struct lldpd_ppvid *ppvid;

	if ((ppvid = header_tprppvidindexed_table(vp, name, length,
		    exact, var_len, write_method)) == NULL)
		return NULL;

	return agent_v_ppvid(vp, var_len, ppvid);
}

static u_char*
agent_v_pi(struct variable *vp, size_t *var_len, struct lldpd_pi *pi)
{
	switch (vp->magic) {
	case LLDP_SNMP_DOT1_PI:
		*var_len = pi->p_pi_len;
		return (u_char *)pi->p_pi;
	default:
		return NULL;
        }
}
static u_char*
agent_h_local_pi(struct variable *vp, oid *name, size_t *length,
		 int exact, size_t *var_len, WriteMethod **write_method)
{
	struct lldpd_pi *pi;

	if ((pi = header_ppiindexed_table(vp, name, length,
		    exact, var_len, write_method)) == NULL)
		return NULL;

	return agent_v_pi(vp, var_len, pi);
}
static u_char*
agent_h_remote_pi(struct variable *vp, oid *name, size_t *length,
		  int exact, size_t *var_len, WriteMethod **write_method)
{
	struct lldpd_pi *pi;

	if ((pi = header_tprpiindexed_table(vp, name, length,
		    exact, var_len, write_method)) == NULL)
		return NULL;

	return agent_v_pi(vp, var_len, pi);
}
#endif

static u_char*
agent_v_port(struct variable *vp, size_t *var_len, struct lldpd_port *port)
{
#ifdef ENABLE_DOT3
	static uint16_t short_ret;
	static uint8_t bit;
#endif
        static unsigned long long_ret;

	switch (vp->magic) {
        case LLDP_SNMP_PIDSUBTYPE:
                long_ret = port->p_id_subtype;
		return (u_char *)&long_ret;
        case LLDP_SNMP_PID:
		*var_len = port->p_id_len;
		return (u_char *)port->p_id;
        case LLDP_SNMP_PORTDESC:
		if (!port->p_descr || *port->p_descr == '\0') break;
		*var_len = strlen(port->p_descr);
		return (u_char *)port->p_descr;
#ifdef ENABLE_DOT3
        case LLDP_SNMP_DOT3_AUTONEG_SUPPORT:
                long_ret = 2 - port->p_macphy.autoneg_support;
                return (u_char *)&long_ret;
        case LLDP_SNMP_DOT3_AUTONEG_ENABLED:
                long_ret = 2 - port->p_macphy.autoneg_enabled;
                return (u_char *)&long_ret;
        case LLDP_SNMP_DOT3_AUTONEG_ADVERTISED:
                *var_len = 2;
		short_ret = htons(port->p_macphy.autoneg_advertised);
                return (u_char *)&short_ret;
        case LLDP_SNMP_DOT3_AUTONEG_MAU:
                long_ret = port->p_macphy.mau_type;
                return (u_char *)&long_ret;
        case LLDP_SNMP_DOT3_AGG_STATUS:
                bit = swap_bits((port->p_aggregid > 0) ? 3 : 0);
                *var_len = 1;
                return (u_char *)&bit;
        case LLDP_SNMP_DOT3_AGG_ID:
                long_ret = port->p_aggregid;
                return (u_char *)&long_ret;
        case LLDP_SNMP_DOT3_MFS:
		if (port->p_mfs) {
			long_ret = port->p_mfs;
			return (u_char *)&long_ret;
		}
		break;
	case LLDP_SNMP_DOT3_POWER_DEVICETYPE:
		if (port->p_power.devicetype) {
			long_ret = (port->p_power.devicetype == LLDP_DOT3_POWER_PSE)?1:2;
			return (u_char *)&long_ret;
		}
		break;
	case LLDP_SNMP_DOT3_POWER_SUPPORT:
		if (port->p_power.devicetype) {
			long_ret = (port->p_power.supported)?1:2;
			return (u_char *)&long_ret;
		}
		break;
	case LLDP_SNMP_DOT3_POWER_ENABLED:
		if (port->p_power.devicetype) {
			long_ret = (port->p_power.enabled)?1:2;
			return (u_char *)&long_ret;
		}
		break;
	case LLDP_SNMP_DOT3_POWER_PAIRCONTROL:
		if (port->p_power.devicetype) {
			long_ret = (port->p_power.paircontrol)?1:2;
			return (u_char *)&long_ret;
		}
		break;
	case LLDP_SNMP_DOT3_POWER_PAIRS:
		if (port->p_power.devicetype) {
			long_ret = port->p_power.pairs;
			return (u_char *)&long_ret;
		}
		break;
	case LLDP_SNMP_DOT3_POWER_CLASS:
		if (port->p_power.devicetype && port->p_power.class) {
			long_ret = port->p_power.class;
			return (u_char *)&long_ret;
		}
		break;
	case LLDP_SNMP_DOT3_POWER_TYPE:
		if (port->p_power.devicetype &&
		    port->p_power.powertype != LLDP_DOT3_POWER_8023AT_OFF) {
			*var_len = 1;
			bit = (((port->p_power.powertype ==
				    LLDP_DOT3_POWER_8023AT_TYPE1)?0:1) << 7) |
			    (((port->p_power.devicetype ==
				    LLDP_DOT3_POWER_PSE)?0:1) << 6);
			return (u_char *)&bit;
		}
		break;
	case LLDP_SNMP_DOT3_POWER_SOURCE:
		if (port->p_power.devicetype &&
		    port->p_power.powertype != LLDP_DOT3_POWER_8023AT_OFF) {
			*var_len = 1;
			bit = swap_bits(port->p_power.source%(1<<2));
			return (u_char *)&bit;
		}
		break;
	case LLDP_SNMP_DOT3_POWER_PRIORITY:
		if (port->p_power.devicetype &&
		    port->p_power.powertype != LLDP_DOT3_POWER_8023AT_OFF) {
			/* See 30.12.2.1.16. This seems defined in reverse order... */
			long_ret = 4 - port->p_power.priority;
			return (u_char *)&long_ret;
		}
		break;
	case LLDP_SNMP_DOT3_POWER_REQUESTED:
		if (port->p_power.devicetype &&
		    port->p_power.powertype != LLDP_DOT3_POWER_8023AT_OFF) {
			long_ret = port->p_power.requested;
			return (u_char *)&long_ret;
		}
		break;
	case LLDP_SNMP_DOT3_POWER_ALLOCATED:
		if (port->p_power.devicetype &&
		    port->p_power.powertype != LLDP_DOT3_POWER_8023AT_OFF) {
			long_ret = port->p_power.allocated;
			return (u_char *)&long_ret;
		}
		break;
#endif
#ifdef ENABLE_DOT1
        case LLDP_SNMP_DOT1_PVID:
                long_ret = port->p_pvid;
                return (u_char *)&long_ret;
#endif
	default:
		break;
        }
	return NULL;
}
static u_char*
agent_h_remote_port(struct variable *vp, oid *name, size_t *length,
    int exact, size_t *var_len, WriteMethod **write_method)
{
	struct lldpd_port *port;
	u_char *a;

	if ((port = header_tprindexed_table(vp, name, length,
					    exact, var_len, write_method, 0)) == NULL)
		return NULL;

	if ((a = agent_v_port(vp, var_len, port)) != NULL)
		return a;
	TRYNEXT(agent_h_remote_port);
}
static u_char*
agent_h_local_port(struct variable *vp, oid *name, size_t *length,
    int exact, size_t *var_len, WriteMethod **write_method)
{
	struct lldpd_hardware *hardware;
	u_char *a;

	if ((hardware = header_portindexed_table(vp, name, length,
		    exact, var_len, write_method)) == NULL)
		return NULL;

	if ((a = agent_v_port(vp, var_len, &hardware->h_lport)) != NULL)
		return a;
	TRYNEXT(agent_h_local_port);
}

static u_char*
agent_v_management(struct variable *vp, size_t *var_len, struct lldpd_mgmt *mgmt)
{
        static unsigned long int long_ret;
        static oid zeroDotZero[2] = {0, 0};

	switch (vp->magic) {
        case LLDP_SNMP_ADDR_LEN:
                long_ret = mgmt->m_addrsize + 1;
                return (u_char*)&long_ret;
        case LLDP_SNMP_ADDR_IFSUBTYPE:
                if (mgmt->m_iface != 0)
                        long_ret = LLDP_MGMT_IFACE_IFINDEX;
                else
                        long_ret = 1;
                return (u_char*)&long_ret;
        case LLDP_SNMP_ADDR_IFID:
                long_ret = mgmt->m_iface;
                return (u_char*)&long_ret;
        case LLDP_SNMP_ADDR_OID:
                *var_len = sizeof(zeroDotZero);
                return (u_char*)zeroDotZero;
	default:
		return NULL;
        }
}
static u_char*
agent_h_local_management(struct variable *vp, oid *name, size_t *length,
    int exact, size_t *var_len, WriteMethod **write_method)
{

	struct lldpd_mgmt *mgmt;

	if ((mgmt = header_ipindexed_table(vp, name, length,
		    exact, var_len, write_method)) == NULL)
		return NULL;

	return agent_v_management(vp, var_len, mgmt);
}
static u_char*
agent_h_remote_management(struct variable *vp, oid *name, size_t *length,
    int exact, size_t *var_len, WriteMethod **write_method)
{
	struct lldpd_mgmt *mgmt;

	if ((mgmt = header_tpripindexed_table(vp, name, length,
		    exact, var_len, write_method)) == NULL)
		return NULL;

        return agent_v_management(vp, var_len, mgmt);
}

#ifdef ENABLE_CUSTOM
static u_char*
agent_v_custom(struct variable *vp, size_t *var_len, struct lldpd_custom *custom)
{
	switch (vp->magic) {
        case LLDP_SNMP_ORG_DEF_INFO:
		*var_len = custom->oui_info_len;
		return (u_char *)custom->oui_info;
	default:
		return NULL;
        }
}
static u_char*
agent_h_remote_custom(struct variable *vp, oid *name, size_t *length,
    int exact, size_t *var_len, WriteMethod **write_method)
{
	struct lldpd_custom *custom;

	if ((custom = header_tprcustomindexed_table(vp, name, length,
		    exact, var_len, write_method)) == NULL)
		return NULL;

        return agent_v_custom(vp, var_len, custom);
}
#endif

/*
  Here is how it works: a agent_h_*() function will handle incoming
  requests. It will use an appropriate header_*indexed_table()
  function to grab the appropriate structure that was queried (a port,
  a chassis, ...). It will then delegate to a agent_v_*() function the
  responsability to extract the appropriate answer.

  agent_h_*() functions and header_*indexed_table() are not shared
  between remote and not remote version while agent_v_*() functions
  are the same for both version.
*/

/* For testing purposes, keep this structure ordered by increasing OID! */
struct variable8 agent_lldp_vars[] = {
	/* Scalars */
	{LLDP_SNMP_TXINTERVAL, ASN_INTEGER, RONLY, agent_h_scalars, 3, {1, 1, 1}},
	{LLDP_SNMP_TXMULTIPLIER, ASN_INTEGER, RONLY, agent_h_scalars, 3, {1, 1, 2}},
	{LLDP_SNMP_REINITDELAY, ASN_INTEGER, RONLY, agent_h_scalars, 3, {1, 1, 3}},
	{LLDP_SNMP_TXDELAY, ASN_INTEGER, RONLY, agent_h_scalars, 3, {1, 1, 4}},
	{LLDP_SNMP_NOTIFICATION, ASN_INTEGER, RONLY, agent_h_scalars, 3, {1, 1, 5}},
	{LLDP_SNMP_LASTUPDATE, ASN_TIMETICKS, RONLY, agent_h_scalars, 3, {1, 2, 1}},
	{LLDP_SNMP_STATS_INSERTS, ASN_GAUGE, RONLY, agent_h_scalars, 3, {1, 2, 2}},
	{LLDP_SNMP_STATS_DELETES, ASN_GAUGE, RONLY, agent_h_scalars, 3, {1, 2, 3}},
	{LLDP_SNMP_STATS_DROPS, ASN_GAUGE, RONLY, agent_h_scalars, 3, {1, 2, 4}},
	{LLDP_SNMP_STATS_AGEOUTS, ASN_GAUGE, RONLY, agent_h_scalars, 3, {1, 2, 5}},
	/* Stats */
	{LLDP_SNMP_STATS_TX, ASN_COUNTER, RONLY, agent_h_stats, 5, {1, 2, 6, 1, 2}},
	{LLDP_SNMP_STATS_RX_DISCARDED, ASN_COUNTER, RONLY, agent_h_stats, 5, {1, 2, 7, 1, 2}},
	{LLDP_SNMP_STATS_RX_ERRORS, ASN_COUNTER, RONLY, agent_h_stats, 5, {1, 2, 7, 1, 3}},
	{LLDP_SNMP_STATS_RX, ASN_COUNTER, RONLY, agent_h_stats, 5, {1, 2, 7, 1, 4}},
	{LLDP_SNMP_STATS_RX_TLVDISCARDED, ASN_COUNTER, RONLY, agent_h_stats, 5, {1, 2, 7, 1, 5}},
	{LLDP_SNMP_STATS_RX_TLVUNRECOGNIZED, ASN_COUNTER, RONLY, agent_h_stats, 5, {1, 2, 7, 1, 6}},
	{LLDP_SNMP_STATS_RX_AGEOUTS, ASN_GAUGE, RONLY, agent_h_stats, 5, {1, 2, 7, 1, 7}},
	/* Local chassis */
	{LLDP_SNMP_CIDSUBTYPE, ASN_INTEGER, RONLY, agent_h_local_chassis, 3, {1, 3, 1}},
	{LLDP_SNMP_CID, ASN_OCTET_STR, RONLY, agent_h_local_chassis, 3, {1, 3, 2}},
	{LLDP_SNMP_SYSNAME, ASN_OCTET_STR, RONLY, agent_h_local_chassis, 3, {1, 3, 3}},
	{LLDP_SNMP_SYSDESCR, ASN_OCTET_STR, RONLY, agent_h_local_chassis, 3, {1, 3, 4}},
	{LLDP_SNMP_SYSCAP_SUP, ASN_OCTET_STR, RONLY, agent_h_local_chassis, 3, {1, 3, 5}},
	{LLDP_SNMP_SYSCAP_ENA, ASN_OCTET_STR, RONLY, agent_h_local_chassis, 3, {1, 3, 6}},
	/* Local ports */
	{LLDP_SNMP_PIDSUBTYPE, ASN_INTEGER, RONLY, agent_h_local_port, 5, {1, 3, 7, 1, 2}},
	{LLDP_SNMP_PID, ASN_OCTET_STR, RONLY, agent_h_local_port, 5, {1, 3, 7, 1, 3}},
	{LLDP_SNMP_PORTDESC, ASN_OCTET_STR, RONLY, agent_h_local_port, 5, {1, 3, 7, 1, 4}},
        /* Local management address */
        {LLDP_SNMP_ADDR_LEN, ASN_INTEGER, RONLY, agent_h_local_management, 5,
         {1, 3, 8, 1, 3}},
        {LLDP_SNMP_ADDR_IFSUBTYPE, ASN_INTEGER, RONLY, agent_h_local_management, 5,
         {1, 3, 8, 1, 4}},
        {LLDP_SNMP_ADDR_IFID, ASN_INTEGER, RONLY, agent_h_local_management, 5,
         {1, 3, 8, 1, 5}},
        {LLDP_SNMP_ADDR_OID, ASN_OBJECT_ID, RONLY, agent_h_local_management, 5,
         {1, 3, 8, 1, 6}},
        /* Remote ports */
        {LLDP_SNMP_CIDSUBTYPE, ASN_INTEGER, RONLY, agent_h_remote_chassis, 5, {1, 4, 1, 1, 4}},
        {LLDP_SNMP_CID, ASN_OCTET_STR, RONLY, agent_h_remote_chassis, 5, {1, 4, 1, 1, 5}},
        {LLDP_SNMP_PIDSUBTYPE, ASN_INTEGER, RONLY, agent_h_remote_port, 5, {1, 4, 1, 1, 6}},
        {LLDP_SNMP_PID, ASN_OCTET_STR, RONLY, agent_h_remote_port, 5, {1, 4, 1, 1, 7}},
        {LLDP_SNMP_PORTDESC, ASN_OCTET_STR, RONLY, agent_h_remote_port, 5, {1, 4, 1, 1, 8}},
        {LLDP_SNMP_SYSNAME, ASN_OCTET_STR, RONLY, agent_h_remote_chassis, 5, {1, 4, 1, 1, 9}},
        {LLDP_SNMP_SYSDESCR, ASN_OCTET_STR, RONLY, agent_h_remote_chassis, 5, {1, 4, 1, 1, 10}},
        {LLDP_SNMP_SYSCAP_SUP, ASN_OCTET_STR, RONLY, agent_h_remote_chassis, 5, {1, 4, 1, 1, 11}},
        {LLDP_SNMP_SYSCAP_ENA, ASN_OCTET_STR, RONLY, agent_h_remote_chassis, 5, {1, 4, 1, 1, 12}},
	/* Remote management address */
        {LLDP_SNMP_ADDR_IFSUBTYPE, ASN_INTEGER, RONLY, agent_h_remote_management, 5,
         {1, 4, 2, 1, 3}},
        {LLDP_SNMP_ADDR_IFID, ASN_INTEGER, RONLY, agent_h_remote_management, 5,
         {1, 4, 2, 1, 4}},
        {LLDP_SNMP_ADDR_OID, ASN_OBJECT_ID, RONLY, agent_h_remote_management, 5,
         {1, 4, 2, 1, 5}},
#ifdef ENABLE_CUSTOM
	/* Custom TLVs */
	{LLDP_SNMP_ORG_DEF_INFO, ASN_OCTET_STR, RONLY, agent_h_remote_custom, 5,
	 {1, 4, 4, 1, 4}},
#endif
#ifdef ENABLE_DOT3
	/* Dot3, local ports */
        {LLDP_SNMP_DOT3_AUTONEG_SUPPORT, ASN_INTEGER, RONLY, agent_h_local_port, 8,
         {1, 5, 4623, 1, 2, 1, 1, 1}},
        {LLDP_SNMP_DOT3_AUTONEG_ENABLED, ASN_INTEGER, RONLY, agent_h_local_port, 8,
         {1, 5, 4623, 1, 2, 1, 1, 2}},
        {LLDP_SNMP_DOT3_AUTONEG_ADVERTISED, ASN_OCTET_STR, RONLY, agent_h_local_port, 8,
         {1, 5, 4623, 1, 2, 1, 1, 3}},
        {LLDP_SNMP_DOT3_AUTONEG_MAU, ASN_INTEGER, RONLY, agent_h_local_port, 8,
         {1, 5, 4623, 1, 2, 1, 1, 4}},
	{LLDP_SNMP_DOT3_POWER_DEVICETYPE, ASN_INTEGER, RONLY, agent_h_local_port, 8,
	 {1, 5, 4623, 1, 2, 2, 1, 1}},
	{LLDP_SNMP_DOT3_POWER_SUPPORT, ASN_INTEGER, RONLY, agent_h_local_port, 8,
	 {1, 5, 4623, 1, 2, 2, 1, 2}},
	{LLDP_SNMP_DOT3_POWER_ENABLED, ASN_INTEGER, RONLY, agent_h_local_port, 8,
	 {1, 5, 4623, 1, 2, 2, 1, 3}},
	{LLDP_SNMP_DOT3_POWER_PAIRCONTROL, ASN_INTEGER, RONLY, agent_h_local_port, 8,
	 {1, 5, 4623, 1, 2, 2, 1, 4}},
	{LLDP_SNMP_DOT3_POWER_PAIRS, ASN_INTEGER, RONLY, agent_h_local_port, 8,
	 {1, 5, 4623, 1, 2, 2, 1, 5}},
	{LLDP_SNMP_DOT3_POWER_CLASS, ASN_INTEGER, RONLY, agent_h_local_port, 8,
	 {1, 5, 4623, 1, 2, 2, 1, 6}},
	{LLDP_SNMP_DOT3_POWER_TYPE, ASN_OCTET_STR, RONLY, agent_h_local_port, 8,
	 {1, 5, 4623, 1, 2, 2, 1, 7}},
	{LLDP_SNMP_DOT3_POWER_SOURCE, ASN_OCTET_STR, RONLY, agent_h_local_port, 8,
	 {1, 5, 4623, 1, 2, 2, 1, 8}},
	{LLDP_SNMP_DOT3_POWER_PRIORITY, ASN_INTEGER, RONLY, agent_h_local_port, 8,
	 {1, 5, 4623, 1, 2, 2, 1, 9}},
	{LLDP_SNMP_DOT3_POWER_REQUESTED, ASN_INTEGER, RONLY, agent_h_local_port, 8,
	 {1, 5, 4623, 1, 2, 2, 1, 10}},
	{LLDP_SNMP_DOT3_POWER_ALLOCATED, ASN_INTEGER, RONLY, agent_h_local_port, 8,
	 {1, 5, 4623, 1, 2, 2, 1, 11}},
        {LLDP_SNMP_DOT3_AGG_STATUS, ASN_OCTET_STR, RONLY, agent_h_local_port, 8,
         {1, 5, 4623, 1, 2, 3, 1, 1}},
        {LLDP_SNMP_DOT3_AGG_ID, ASN_INTEGER, RONLY, agent_h_local_port, 8,
         {1, 5, 4623, 1, 2, 3, 1, 2}},
        {LLDP_SNMP_DOT3_MFS, ASN_INTEGER, RONLY, agent_h_local_port, 8,
         {1, 5, 4623, 1, 2, 4, 1, 1}},
#endif
	/* Dot3, remote ports */
#ifdef ENABLE_DOT3
        {LLDP_SNMP_DOT3_AUTONEG_SUPPORT, ASN_INTEGER, RONLY, agent_h_remote_port, 8,
         {1, 5, 4623, 1, 3, 1, 1, 1}},
        {LLDP_SNMP_DOT3_AUTONEG_ENABLED, ASN_INTEGER, RONLY, agent_h_remote_port, 8,
         {1, 5, 4623, 1, 3, 1, 1, 2}},
        {LLDP_SNMP_DOT3_AUTONEG_ADVERTISED, ASN_OCTET_STR, RONLY, agent_h_remote_port, 8,
         {1, 5, 4623, 1, 3, 1, 1, 3}},
        {LLDP_SNMP_DOT3_AUTONEG_MAU, ASN_INTEGER, RONLY, agent_h_remote_port, 8,
         {1, 5, 4623, 1, 3, 1, 1, 4}},
	{LLDP_SNMP_DOT3_POWER_DEVICETYPE, ASN_INTEGER, RONLY, agent_h_remote_port, 8,
	 {1, 5, 4623, 1, 3, 2, 1, 1}},
	{LLDP_SNMP_DOT3_POWER_SUPPORT, ASN_INTEGER, RONLY, agent_h_remote_port, 8,
	 {1, 5, 4623, 1, 3, 2, 1, 2}},
	{LLDP_SNMP_DOT3_POWER_ENABLED, ASN_INTEGER, RONLY, agent_h_remote_port, 8,
	 {1, 5, 4623, 1, 3, 2, 1, 3}},
	{LLDP_SNMP_DOT3_POWER_PAIRCONTROL, ASN_INTEGER, RONLY, agent_h_remote_port, 8,
	 {1, 5, 4623, 1, 3, 2, 1, 4}},
	{LLDP_SNMP_DOT3_POWER_PAIRS, ASN_INTEGER, RONLY, agent_h_remote_port, 8,
	 {1, 5, 4623, 1, 3, 2, 1, 5}},
	{LLDP_SNMP_DOT3_POWER_CLASS, ASN_INTEGER, RONLY, agent_h_remote_port, 8,
	 {1, 5, 4623, 1, 3, 2, 1, 6}},
	{LLDP_SNMP_DOT3_POWER_TYPE, ASN_OCTET_STR, RONLY, agent_h_remote_port, 8,
	 {1, 5, 4623, 1, 3, 2, 1, 7}},
	{LLDP_SNMP_DOT3_POWER_SOURCE, ASN_OCTET_STR, RONLY, agent_h_remote_port, 8,
	 {1, 5, 4623, 1, 3, 2, 1, 8}},
	{LLDP_SNMP_DOT3_POWER_PRIORITY, ASN_INTEGER, RONLY, agent_h_remote_port, 8,
	 {1, 5, 4623, 1, 3, 2, 1, 9}},
	{LLDP_SNMP_DOT3_POWER_REQUESTED, ASN_INTEGER, RONLY, agent_h_remote_port, 8,
	 {1, 5, 4623, 1, 3, 2, 1, 10}},
	{LLDP_SNMP_DOT3_POWER_ALLOCATED, ASN_INTEGER, RONLY, agent_h_remote_port, 8,
	 {1, 5, 4623, 1, 3, 2, 1, 11}},
        {LLDP_SNMP_DOT3_AGG_STATUS, ASN_OCTET_STR, RONLY, agent_h_remote_port, 8,
         {1, 5, 4623, 1, 3, 3, 1, 1}},
        {LLDP_SNMP_DOT3_AGG_ID, ASN_INTEGER, RONLY, agent_h_remote_port, 8,
         {1, 5, 4623, 1, 3, 3, 1, 2}},
        {LLDP_SNMP_DOT3_MFS, ASN_INTEGER, RONLY, agent_h_remote_port, 8,
         {1, 5, 4623, 1, 3, 4, 1, 1}},
#endif
#ifdef ENABLE_LLDPMED
	/* LLDP-MED local */
	{LLDP_SNMP_MED_CLASS, ASN_INTEGER, RONLY, agent_h_local_med, 6,
	 {1, 5, 4795, 1, 1, 1}},
	{LLDP_SNMP_MED_POLICY_VID, ASN_INTEGER, RONLY, agent_h_local_med_policy, 8,
	 {1, 5, 4795, 1, 2, 1, 1, 2}},
	{LLDP_SNMP_MED_POLICY_PRIO, ASN_INTEGER, RONLY, agent_h_local_med_policy, 8,
	 {1, 5, 4795, 1, 2, 1, 1, 3}},
	{LLDP_SNMP_MED_POLICY_DSCP, ASN_INTEGER, RONLY, agent_h_local_med_policy, 8,
	 {1, 5, 4795, 1, 2, 1, 1, 4}},
	{LLDP_SNMP_MED_POLICY_UNKNOWN, ASN_INTEGER, RONLY, agent_h_local_med_policy, 8,
	 {1, 5, 4795, 1, 2, 1, 1, 5}},
	{LLDP_SNMP_MED_POLICY_TAGGED, ASN_INTEGER, RONLY, agent_h_local_med_policy, 8,
	 {1, 5, 4795, 1, 2, 1, 1, 6}},
	{LLDP_SNMP_MED_HW, ASN_OCTET_STR, RONLY, agent_h_local_med, 6,
	 {1, 5, 4795, 1, 2, 2}},
	{LLDP_SNMP_MED_FW, ASN_OCTET_STR, RONLY, agent_h_local_med, 6,
	 {1, 5, 4795, 1, 2, 3}},
	{LLDP_SNMP_MED_SW, ASN_OCTET_STR, RONLY, agent_h_local_med, 6,
	 {1, 5, 4795, 1, 2, 4}},
	{LLDP_SNMP_MED_SN, ASN_OCTET_STR, RONLY, agent_h_local_med, 6,
	 {1, 5, 4795, 1, 2, 5}},
	{LLDP_SNMP_MED_MANUF, ASN_OCTET_STR, RONLY, agent_h_local_med, 6,
	 {1, 5, 4795, 1, 2, 6}},
	{LLDP_SNMP_MED_MODEL, ASN_OCTET_STR, RONLY, agent_h_local_med, 6,
	 {1, 5, 4795, 1, 2, 7}},
	{LLDP_SNMP_MED_ASSET, ASN_OCTET_STR, RONLY, agent_h_local_med, 6,
	 {1, 5, 4795, 1, 2, 8}},
	{LLDP_SNMP_MED_LOCATION, ASN_OCTET_STR, RONLY, agent_h_local_med_location, 8,
	 {1, 5, 4795, 1, 2, 9, 1, 2}},
	{LLDP_SNMP_MED_POE_DEVICETYPE, ASN_INTEGER, RONLY, agent_h_local_med_power, 6,
	 {1, 5, 4795, 1, 2, 10}},
	{LLDP_SNMP_MED_POE_PSE_POWERVAL, ASN_GAUGE, RONLY, agent_h_local_med_power, 8,
	 {1, 5, 4795, 1, 2, 11, 1, 1}},
	{LLDP_SNMP_MED_POE_PSE_POWERPRIORITY, ASN_INTEGER, RONLY, agent_h_local_med_power, 8,
	 {1, 5, 4795, 1, 2, 11, 1, 2}},
	{LLDP_SNMP_MED_POE_PSE_POWERSOURCE, ASN_INTEGER, RONLY, agent_h_local_med_power, 6,
	 {1, 5, 4795, 1, 2, 12}},
	{LLDP_SNMP_MED_POE_PD_POWERVAL, ASN_GAUGE, RONLY, agent_h_local_med_power, 6,
	 {1, 5, 4795, 1, 2, 13}},
	{LLDP_SNMP_MED_POE_PD_POWERSOURCE, ASN_INTEGER, RONLY, agent_h_local_med_power, 6,
	 {1, 5, 4795, 1, 2, 14}},
	{LLDP_SNMP_MED_POE_PD_POWERPRIORITY, ASN_INTEGER, RONLY, agent_h_local_med_power, 6,
	 {1, 5, 4795, 1, 2, 15}},
	/* LLDP-MED remote */
	{LLDP_SNMP_MED_CAP_AVAILABLE, ASN_OCTET_STR, RONLY, agent_h_remote_med, 8,
	 {1, 5, 4795, 1, 3, 1, 1, 1}},
	{LLDP_SNMP_MED_CAP_ENABLED, ASN_OCTET_STR, RONLY, agent_h_remote_med, 8,
	 {1, 5, 4795, 1, 3, 1, 1, 2}},
	{LLDP_SNMP_MED_CLASS, ASN_INTEGER, RONLY, agent_h_remote_med, 8,
	 {1, 5, 4795, 1, 3, 1, 1, 3}},
	{LLDP_SNMP_MED_POLICY_VID, ASN_INTEGER, RONLY, agent_h_remote_med_policy, 8,
	 {1, 5, 4795, 1, 3, 2, 1, 2}},
	{LLDP_SNMP_MED_POLICY_PRIO, ASN_INTEGER, RONLY, agent_h_remote_med_policy, 8,
	 {1, 5, 4795, 1, 3, 2, 1, 3}},
	{LLDP_SNMP_MED_POLICY_DSCP, ASN_INTEGER, RONLY, agent_h_remote_med_policy, 8,
	 {1, 5, 4795, 1, 3, 2, 1, 4}},
	{LLDP_SNMP_MED_POLICY_UNKNOWN, ASN_INTEGER, RONLY, agent_h_remote_med_policy, 8,
	 {1, 5, 4795, 1, 3, 2, 1, 5}},
	{LLDP_SNMP_MED_POLICY_TAGGED, ASN_INTEGER, RONLY, agent_h_remote_med_policy, 8,
	 {1, 5, 4795, 1, 3, 2, 1, 6}},
	{LLDP_SNMP_MED_HW, ASN_OCTET_STR, RONLY, agent_h_remote_med, 8,
	 {1, 5, 4795, 1, 3, 3, 1, 1}},
	{LLDP_SNMP_MED_FW, ASN_OCTET_STR, RONLY, agent_h_remote_med, 8,
	 {1, 5, 4795, 1, 3, 3, 1, 2}},
	{LLDP_SNMP_MED_SW, ASN_OCTET_STR, RONLY, agent_h_remote_med, 8,
	 {1, 5, 4795, 1, 3, 3, 1, 3}},
	{LLDP_SNMP_MED_SN, ASN_OCTET_STR, RONLY, agent_h_remote_med, 8,
	 {1, 5, 4795, 1, 3, 3, 1, 4}},
	{LLDP_SNMP_MED_MANUF, ASN_OCTET_STR, RONLY, agent_h_remote_med, 8,
	 {1, 5, 4795, 1, 3, 3, 1, 5}},
	{LLDP_SNMP_MED_MODEL, ASN_OCTET_STR, RONLY, agent_h_remote_med, 8,
	 {1, 5, 4795, 1, 3, 3, 1, 6}},
	{LLDP_SNMP_MED_ASSET, ASN_OCTET_STR, RONLY, agent_h_remote_med, 8,
	 {1, 5, 4795, 1, 3, 3, 1, 7}},
	{LLDP_SNMP_MED_LOCATION, ASN_OCTET_STR, RONLY, agent_h_remote_med_location, 8,
	 {1, 5, 4795, 1, 3, 4, 1, 2}},
	{LLDP_SNMP_MED_POE_DEVICETYPE, ASN_INTEGER, RONLY, agent_h_remote_med_power, 8,
	 {1, 5, 4795, 1, 3, 5, 1, 1}},
	{LLDP_SNMP_MED_POE_PSE_POWERVAL, ASN_GAUGE, RONLY, agent_h_remote_med_power, 8,
	 {1, 5, 4795, 1, 3, 6, 1, 1}},
	{LLDP_SNMP_MED_POE_PSE_POWERSOURCE, ASN_INTEGER, RONLY, agent_h_remote_med_power, 8,
	 {1, 5, 4795, 1, 3, 6, 1, 2}},
	{LLDP_SNMP_MED_POE_PSE_POWERPRIORITY, ASN_INTEGER, RONLY, agent_h_remote_med_power, 8,
	 {1, 5, 4795, 1, 3, 6, 1, 3}},
	{LLDP_SNMP_MED_POE_PD_POWERVAL, ASN_GAUGE, RONLY, agent_h_remote_med_power, 8,
	 {1, 5, 4795, 1, 3, 7, 1, 1}},
	{LLDP_SNMP_MED_POE_PD_POWERSOURCE, ASN_INTEGER, RONLY, agent_h_remote_med_power, 8,
	 {1, 5, 4795, 1, 3, 7, 1, 2}},
	{LLDP_SNMP_MED_POE_PD_POWERPRIORITY, ASN_INTEGER, RONLY, agent_h_remote_med_power, 8,
	 {1, 5, 4795, 1, 3, 7, 1, 3}},
#endif
	/* Dot1, local and remote ports */
#ifdef ENABLE_DOT1
        {LLDP_SNMP_DOT1_PVID, ASN_INTEGER, RONLY, agent_h_local_port, 8,
         {1, 5, 32962, 1, 2, 1, 1, 1}},
        {LLDP_SNMP_DOT1_PPVLAN_SUPPORTED, ASN_INTEGER, RONLY, agent_h_local_ppvid, 8,
         {1, 5, 32962, 1, 2, 2, 1, 2}},
        {LLDP_SNMP_DOT1_PPVLAN_ENABLED, ASN_INTEGER, RONLY, agent_h_local_ppvid, 8,
         {1, 5, 32962, 1, 2, 2, 1, 3}},
        {LLDP_SNMP_DOT1_VLANNAME, ASN_OCTET_STR, RONLY, agent_h_local_vlan, 8,
         {1, 5, 32962, 1, 2, 3, 1, 2}},
	{LLDP_SNMP_DOT1_PI, ASN_OCTET_STR, RONLY, agent_h_local_pi, 8,
	 {1, 5, 32962, 1, 2, 4, 1, 2}},
#endif
#ifdef ENABLE_DOT1
        {LLDP_SNMP_DOT1_PVID, ASN_INTEGER, RONLY, agent_h_remote_port, 8,
         {1, 5, 32962, 1, 3, 1, 1, 1}},
        {LLDP_SNMP_DOT1_PPVLAN_SUPPORTED, ASN_INTEGER, RONLY, agent_h_remote_ppvid, 8,
         {1, 5, 32962, 1, 3, 2, 1, 2}},
        {LLDP_SNMP_DOT1_PPVLAN_ENABLED, ASN_INTEGER, RONLY, agent_h_remote_ppvid, 8,
         {1, 5, 32962, 1, 3, 2, 1, 3}},
        /* Remote vlans */
        {LLDP_SNMP_DOT1_VLANNAME, ASN_OCTET_STR, RONLY, agent_h_remote_vlan, 8,
         {1, 5, 32962, 1, 3, 3, 1, 2}},
	/* Protocol identity */
	{LLDP_SNMP_DOT1_PI, ASN_OCTET_STR, RONLY, agent_h_remote_pi, 8,
	 {1, 5, 32962, 1, 3, 4, 1, 2}},
#endif
};
size_t agent_lldp_vars_size(void) {
	return sizeof(agent_lldp_vars)/sizeof(struct variable8);
}

/**
 * Send a notification about a change in one remote neighbor.
 *
 * @param hardware Interface on which the change has happened.
 * @param type     Type of change (add, delete, update)
 * @param rport    Changed remote port
 */
void
agent_notify(struct lldpd_hardware *hardware, int type,
    struct lldpd_port *rport)
{
	struct lldpd_hardware *h;

	/* OID of the notification */
	oid notification_oid[] = { LLDP_OID, 0, 0, 1 };
	size_t notification_oid_len = OID_LENGTH(notification_oid);
	/* OID for snmpTrapOID.0 */
	oid objid_snmptrap[] = { SNMPTRAP_OID };
	size_t objid_snmptrap_len = OID_LENGTH(objid_snmptrap);

	/* Other OID */
        oid inserts_oid[] = { LLDP_OID, 1, 2, 2 };
	size_t inserts_oid_len = OID_LENGTH(inserts_oid);
	unsigned long inserts = 0;

        oid deletes_oid[] = { LLDP_OID, 1, 2, 3 };
	size_t deletes_oid_len = OID_LENGTH(deletes_oid);
	unsigned long deletes = 0;

        oid drops_oid[] = { LLDP_OID, 1, 2, 4 };
	size_t drops_oid_len = OID_LENGTH(drops_oid);
	unsigned long drops = 0;

        oid ageouts_oid[] = { LLDP_OID, 1, 2, 5 };
	size_t ageouts_oid_len = OID_LENGTH(ageouts_oid);
	unsigned long ageouts = 0;

	/* We also add some extra. Easy ones. */
	oid locport_oid[] = { LLDP_OID, 1, 3, 7, 1, 4,
			      hardware->h_ifindex };
	size_t locport_oid_len = OID_LENGTH(locport_oid);
	oid sysname_oid[] = { LLDP_OID, 1, 4, 1, 1, 9,
			      lastchange(rport), hardware->h_ifindex,
			      rport->p_chassis->c_index };
	size_t sysname_oid_len = OID_LENGTH(sysname_oid);
	oid portdescr_oid[] = { LLDP_OID, 1, 4, 1, 1, 8,
			      lastchange(rport), hardware->h_ifindex,
			      rport->p_chassis->c_index };
	size_t portdescr_oid_len = OID_LENGTH(portdescr_oid);

	netsnmp_variable_list *notification_vars = NULL;

	if (!hardware->h_cfg->g_snmp) return;

	switch (type) {
	case NEIGHBOR_CHANGE_DELETED:
		log_debug("snmp", "send notification for neighbor deleted on %s",
		    hardware->h_ifname);
		break;
	case NEIGHBOR_CHANGE_UPDATED:
		log_debug("snmp", "send notification for neighbor updated on %s",
		    hardware->h_ifname);
		break;
	case NEIGHBOR_CHANGE_ADDED:
		log_debug("snmp", "send notification for neighbor added on %s",
		    hardware->h_ifname);
		break;
	}

	TAILQ_FOREACH(h, &hardware->h_cfg->g_hardware, h_entries) {
		inserts += h->h_insert_cnt;
		deletes += h->h_delete_cnt;
		ageouts += h->h_ageout_cnt;
		drops   += h->h_drop_cnt;
	}

	/* snmpTrapOID */
	snmp_varlist_add_variable(&notification_vars,
	    objid_snmptrap, objid_snmptrap_len,
	    ASN_OBJECT_ID,
	    (u_char *) notification_oid,
	    notification_oid_len * sizeof(oid));

	snmp_varlist_add_variable(&notification_vars,
	    inserts_oid, inserts_oid_len,
	    ASN_GAUGE,
	    (u_char *)&inserts,
	    sizeof(inserts));
	snmp_varlist_add_variable(&notification_vars,
	    deletes_oid, deletes_oid_len,
	    ASN_GAUGE,
	    (u_char *)&deletes,
	    sizeof(inserts));
	snmp_varlist_add_variable(&notification_vars,
	    drops_oid, drops_oid_len,
	    ASN_GAUGE,
	    (u_char *)&drops,
	    sizeof(drops));
	snmp_varlist_add_variable(&notification_vars,
	    ageouts_oid, ageouts_oid_len,
	    ASN_GAUGE,
	    (u_char *)&ageouts,
	    sizeof(ageouts));

	if (type != NEIGHBOR_CHANGE_DELETED) {
		snmp_varlist_add_variable(&notification_vars,
		    locport_oid, locport_oid_len,
		    ASN_OCTET_STR,
		    (u_char *)hardware->h_ifname,
		    strnlen(hardware->h_ifname, IFNAMSIZ));
		if (rport->p_chassis->c_name && *rport->p_chassis->c_name != '\0') {
			snmp_varlist_add_variable(&notification_vars,
			    sysname_oid, sysname_oid_len,
			    ASN_OCTET_STR,
			    (u_char *)rport->p_chassis->c_name,
			    strlen(rport->p_chassis->c_name));
		}
		if (rport->p_descr) {
			snmp_varlist_add_variable(&notification_vars,
			    portdescr_oid, portdescr_oid_len,
			    ASN_OCTET_STR,
			    (u_char *)rport->p_descr,
			    strlen(rport->p_descr));
		}
	}

	log_debug("snmp", "sending SNMP trap (%ld, %ld, %ld)",
	    inserts, deletes, ageouts);
	send_v2trap(notification_vars);
	snmp_free_varbind(notification_vars);
}


/* Logging NetSNMP messages */
static int
agent_log_callback(int major, int minor,
			void *serverarg, void *clientarg) {
  struct snmp_log_message *slm = (struct snmp_log_message *)serverarg;
  char *msg = strdup(slm->msg);
  (void)major; (void)minor; (void)clientarg;

  if (msg && msg[strlen(msg)-1] == '\n') msg[strlen(msg)-1] = '\0';
  switch (slm->priority) {
  case LOG_EMERG:   log_warnx("libsnmp", "%s", msg?msg:slm->msg); break;
  case LOG_ALERT:   log_warnx("libsnmp", "%s", msg?msg:slm->msg); break;
  case LOG_CRIT:    log_warnx("libsnmp", "%s", msg?msg:slm->msg); break;
  case LOG_ERR:     log_warnx("libsnmp", "%s", msg?msg:slm->msg); break;
  case LOG_WARNING: log_warnx("libsnmp", "%s", msg?msg:slm->msg); break;
  case LOG_NOTICE:  log_info ("libsnmp", "%s", msg?msg:slm->msg); break;
  case LOG_INFO:    log_info ("libsnmp", "%s", msg?msg:slm->msg); break;
  case LOG_DEBUG:   log_debug("libsnmp", "%s", msg?msg:slm->msg); break;
  }
  free(msg);
  return SNMP_ERR_NOERROR;
}

void
agent_init(struct lldpd *cfg, const char *agentx)
{
	int rc;

	log_info("snmp", "enable SNMP subagent");
	netsnmp_enable_subagent();

	log_debug("snmp", "enable logging");
	snmp_disable_log();
	snmp_enable_calllog();
	snmp_register_callback(SNMP_CALLBACK_LIBRARY,
			       SNMP_CALLBACK_LOGGING,
			       agent_log_callback,
			       NULL);

	scfg = cfg;

	/* We are chrooted, we don't want to handle persistent states */
	netsnmp_ds_set_boolean(NETSNMP_DS_LIBRARY_ID,
	    NETSNMP_DS_LIB_DONT_PERSIST_STATE, TRUE);
	/* Do not load any MIB */
	setenv("MIBS", "", 1);
	setenv("MIBDIRS", "/dev/null", 1);

#ifdef ENABLE_PRIVSEP
	/* We provide our UNIX domain transport */
	log_debug("snmp", "register UNIX domain transport");
	agent_priv_register_domain();
#endif

	if (agentx)
		netsnmp_ds_set_string(NETSNMP_DS_APPLICATION_ID,
				      NETSNMP_DS_AGENT_X_SOCKET, agentx);
	init_agent("lldpAgent");
	REGISTER_MIB("lldp", agent_lldp_vars, variable8, lldp_oid);
	init_snmp("lldpAgent");

	log_debug("snmp", "register to sysORTable");
	if ((rc = register_sysORTable(lldp_oid, OID_LENGTH(lldp_oid),
		    "lldpMIB implementation by lldpd")) != 0)
		log_warnx("snmp", "unable to register to sysORTable (%d)", rc);
}

void
agent_shutdown()
{
	log_debug("snmp", "agent shutdown");
	unregister_sysORTable(lldp_oid, OID_LENGTH(lldp_oid));
	snmp_shutdown("lldpAgent");
}
