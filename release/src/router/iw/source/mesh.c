#include <net/if.h>
#include <errno.h>
#include <string.h>

#include <netlink/genl/genl.h>
#include <netlink/genl/family.h>
#include <netlink/genl/ctrl.h>
#include <netlink/msg.h>
#include <netlink/attr.h>

#include "nl80211.h"
#include "iw.h"

SECTION(mesh);


typedef struct _any_t {
	union {
		uint32_t as_32;
		int32_t as_s32;
		uint16_t as_16;
		uint8_t as_8;
	} u;
} _any;

/* describes a mesh parameter */
struct mesh_param_descr {
	const char *name;
	enum nl80211_meshconf_params mesh_param_num;
	int (*nla_put_fn)(struct nl_msg*, int, _any*);
	uint32_t (*parse_fn)(const char*, _any*);
	void (*nla_print_fn)(struct nlattr *);
};

/* utility functions for manipulating and printing u8/u16/u32 values and
 * timesouts. */
static int _my_nla_put_u8(struct nl_msg *n, int mesh_param_num, _any *value)
{
	return nla_put(n, mesh_param_num, sizeof(uint8_t), &value->u.as_8);
}

static int _my_nla_put_u16(struct nl_msg *n, int mesh_param_num, _any *value)
{
	return nla_put(n, mesh_param_num, sizeof(uint16_t), &value->u.as_16);
}

static int _my_nla_put_u32(struct nl_msg *n, int mesh_param_num, _any *value)
{
	return nla_put(n, mesh_param_num, sizeof(uint32_t), &value->u.as_32);
}

static uint32_t _parse_u8(const char *str, _any *ret)
{
	char *endptr = NULL;
	unsigned long int v = strtoul(str, &endptr, 10);
	if (*endptr != '\0')
		return 0xff;
	if (v > 0xff)
		return 0xff;
	ret->u.as_8 = (uint8_t)v;
	return 0;
}

static uint32_t _parse_u8_as_bool(const char *str, _any *ret)
{
	char *endptr = NULL;
	unsigned long int v = strtoul(str, &endptr, 10);
	if (*endptr != '\0')
		return 0x1;
	if (v > 0x1)
		return 0x1;
	ret->u.as_8 = (uint8_t)v;
	return 0;
}

static uint32_t _parse_u16(const char *str, _any *ret)
{
	char *endptr = NULL;
	long int v = strtol(str, &endptr, 10);
	if (*endptr != '\0')
		return 0xffff;
	if ((v < 0) || (v > 0xffff))
		return 0xffff;
	ret->u.as_16 = (uint16_t)v;
	return 0;
}

static uint32_t _parse_u32(const char *str, _any *ret)
{
	char *endptr = NULL;
	long long int v = strtoll(str, &endptr, 10);
	if (*endptr != '\0')
		return 0xffffffff;
	if ((v < 0) || (v > 0xffffffff))
		return 0xffffffff;
	ret->u.as_32 = (uint32_t)v;
	return 0;
}

static uint32_t _parse_s32(const char *str, _any *ret)
{
	char *endptr = NULL;
	long int v = strtol(str, &endptr, 10);
	if (*endptr != '\0')
		return 0xffffffff;
	if (v > 0xff)
		return 0xffffffff;
	ret->u.as_s32 = (int32_t)v;
	return 0;
}


static void _print_u8(struct nlattr *a)
{
	printf("%d", nla_get_u8(a));
}

static void _print_u16(struct nlattr *a)
{
	printf("%d", nla_get_u16(a));
}

static void _print_u16_timeout(struct nlattr *a)
{
	printf("%d milliseconds", nla_get_u16(a));
}

static void _print_u16_in_TUs(struct nlattr *a)
{
	printf("%d TUs", nla_get_u16(a));
}

static void _print_u32(struct nlattr *a)
{
	printf("%d", nla_get_u32(a));
}

static void _print_u32_timeout(struct nlattr *a)
{
	printf("%u milliseconds", nla_get_u32(a));
}

static void _print_u32_in_TUs(struct nlattr *a)
{
	printf("%d TUs", nla_get_u32(a));
}

static void _print_s32_in_dBm(struct nlattr *a)
{
	printf("%d dBm", (int32_t) nla_get_u32(a));
}


/* The current mesh parameters */
const static struct mesh_param_descr _mesh_param_descrs[] =
{
	{"mesh_retry_timeout",
	NL80211_MESHCONF_RETRY_TIMEOUT,
	_my_nla_put_u16, _parse_u16, _print_u16_timeout},
	{"mesh_confirm_timeout",
	NL80211_MESHCONF_CONFIRM_TIMEOUT,
	_my_nla_put_u16, _parse_u16, _print_u16_timeout},
	{"mesh_holding_timeout",
	NL80211_MESHCONF_HOLDING_TIMEOUT,
	_my_nla_put_u16, _parse_u16, _print_u16_timeout},
	{"mesh_max_peer_links",
	NL80211_MESHCONF_MAX_PEER_LINKS,
	_my_nla_put_u16, _parse_u16, _print_u16},
	{"mesh_max_retries",
	NL80211_MESHCONF_MAX_RETRIES,
	_my_nla_put_u8, _parse_u8, _print_u8},
	{"mesh_ttl",
	NL80211_MESHCONF_TTL,
	_my_nla_put_u8, _parse_u8, _print_u8},
	{"mesh_element_ttl",
	NL80211_MESHCONF_ELEMENT_TTL,
	_my_nla_put_u8, _parse_u8, _print_u8},
	{"mesh_auto_open_plinks",
	NL80211_MESHCONF_AUTO_OPEN_PLINKS,
	_my_nla_put_u8, _parse_u8_as_bool, _print_u8},
	{"mesh_hwmp_max_preq_retries",
	NL80211_MESHCONF_HWMP_MAX_PREQ_RETRIES,
	_my_nla_put_u8, _parse_u8, _print_u8},
	{"mesh_path_refresh_time",
	NL80211_MESHCONF_PATH_REFRESH_TIME,
	_my_nla_put_u32, _parse_u32, _print_u32_timeout},
	{"mesh_min_discovery_timeout",
	NL80211_MESHCONF_MIN_DISCOVERY_TIMEOUT,
	_my_nla_put_u16, _parse_u16, _print_u16_timeout},
	{"mesh_hwmp_active_path_timeout",
	NL80211_MESHCONF_HWMP_ACTIVE_PATH_TIMEOUT,
	_my_nla_put_u32, _parse_u32, _print_u32_in_TUs},
	{"mesh_hwmp_preq_min_interval",
	NL80211_MESHCONF_HWMP_PREQ_MIN_INTERVAL,
	_my_nla_put_u16, _parse_u16, _print_u16_in_TUs},
	{"mesh_hwmp_net_diameter_traversal_time",
	NL80211_MESHCONF_HWMP_NET_DIAM_TRVS_TIME,
	_my_nla_put_u16, _parse_u16, _print_u16_in_TUs},
	{"mesh_hwmp_rootmode", NL80211_MESHCONF_HWMP_ROOTMODE,
	_my_nla_put_u8, _parse_u8, _print_u8},
	{"mesh_hwmp_rann_interval", NL80211_MESHCONF_HWMP_RANN_INTERVAL,
	_my_nla_put_u16, _parse_u16, _print_u16_in_TUs},
	{"mesh_gate_announcements", NL80211_MESHCONF_GATE_ANNOUNCEMENTS,
	_my_nla_put_u8, _parse_u8, _print_u8},
	{"mesh_fwding", NL80211_MESHCONF_FORWARDING,
	_my_nla_put_u8, _parse_u8_as_bool, _print_u8},
	{"mesh_sync_offset_max_neighor",
	NL80211_MESHCONF_SYNC_OFFSET_MAX_NEIGHBOR,
	_my_nla_put_u32, _parse_u32, _print_u32},
	{"mesh_rssi_threshold", NL80211_MESHCONF_RSSI_THRESHOLD,
	_my_nla_put_u32, _parse_s32, _print_s32_in_dBm},
	{"mesh_hwmp_active_path_to_root_timeout",
	NL80211_MESHCONF_HWMP_PATH_TO_ROOT_TIMEOUT,
	_my_nla_put_u32, _parse_u32, _print_u32_in_TUs},
	{"mesh_hwmp_root_interval", NL80211_MESHCONF_HWMP_ROOT_INTERVAL,
	_my_nla_put_u16, _parse_u16, _print_u16_in_TUs},
	{"mesh_hwmp_confirmation_interval",
	NL80211_MESHCONF_HWMP_CONFIRMATION_INTERVAL,
	_my_nla_put_u16, _parse_u16, _print_u16_in_TUs},
};

static void print_all_mesh_param_descr(void)
{
	int i;

	printf("Possible mesh parameters are:\n");

	for (i = 0; i < ARRAY_SIZE(_mesh_param_descrs); i++)
		printf(" - %s\n", _mesh_param_descrs[i].name);
}

static const struct mesh_param_descr *find_mesh_param(const char *name)
{
	int i;
	const struct mesh_param_descr *mdescr = NULL;

	/* Find out what mesh parameter we want to change. */
	for (i = 0; i < ARRAY_SIZE(_mesh_param_descrs); i++) {
		if (!strcmp(_mesh_param_descrs[i].name, name))
			return _mesh_param_descrs + i;
	}

	if (!mdescr) {
		print_all_mesh_param_descr();
		return NULL;
	}
	return mdescr;
}

/* Setter */
static int set_interface_meshparam(struct nl80211_state *state,
				   struct nl_cb *cb,
				   struct nl_msg *msg,
				   int argc, char **argv,
				   enum id_input id)
{
	const struct mesh_param_descr *mdescr;
	struct nlattr *container;
	uint32_t ret;
	int err;

	container = nla_nest_start(msg, NL80211_ATTR_MESH_PARAMS);
	if (!container)
		return -ENOBUFS;

	if (!argc)
		return 1;

	while (argc) {
		const char *name;
		char *value;
		_any any;

		memset(&any, 0, sizeof(_any));

		name = argv[0];
		value = strchr(name, '=');
		if (value) {
			*value = '\0';
			value++;
			argc--;
			argv++;
		} else {
			/* backward compat -- accept w/o '=' */
			if (argc < 2) {
				printf("Must specify a value for %s.\n", name);
				return 2;
			}
			value = argv[1];
			argc -= 2;
			argv += 2;
		}

		mdescr = find_mesh_param(name);
		if (!mdescr)
			return 2;

		/* Parse the new value */
		ret = mdescr->parse_fn(value, &any);
		if (ret != 0) {
			printf("%s must be set to a number "
			       "between 0 and %u\n", mdescr->name, ret);
			return 2;
		}

		err = mdescr->nla_put_fn(msg, mdescr->mesh_param_num, &any);
		if (err)
			return err;
	}
	nla_nest_end(msg, container);

	return err;
}

COMMAND(set, mesh_param, "<param>=<value> [<param>=<value>]*",
	NL80211_CMD_SET_MESH_PARAMS, 0, CIB_NETDEV, set_interface_meshparam,
	"Set mesh parameter (run command without any to see available ones).");

/* Getter */
static int print_mesh_param_handler(struct nl_msg *msg, void *arg)
{
	const struct mesh_param_descr *mdescr = arg;
	struct nlattr *attrs[NL80211_ATTR_MAX + 1];
	struct nlattr *parent_attr;
	struct nlattr *mesh_params[NL80211_MESHCONF_ATTR_MAX + 1];
	struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));

	/* locate NL80211_ATTR_MESH_PARAMS */
	nla_parse(attrs, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0),
		  genlmsg_attrlen(gnlh, 0), NULL);
	parent_attr = attrs[NL80211_ATTR_MESH_PARAMS];
	if (!parent_attr)
		return -EINVAL;

	/* unpack the mesh parameters */
	if (nla_parse_nested(mesh_params, NL80211_MESHCONF_ATTR_MAX,
			     parent_attr, NULL))
		return -EINVAL;

	if (!mdescr) {
		int i;

		for (i = 0; i < ARRAY_SIZE(_mesh_param_descrs); i++) {
			mdescr = &_mesh_param_descrs[i];
			printf("%s = ", mdescr->name);
			mdescr->nla_print_fn(mesh_params[mdescr->mesh_param_num]);
			printf("\n");
		}
		return NL_SKIP;
	}

	/* print out the mesh parameter */
	mdescr->nla_print_fn(mesh_params[mdescr->mesh_param_num]);
	printf("\n");
	return NL_SKIP;
}

static int get_interface_meshparam(struct nl80211_state *state,
				   struct nl_cb *cb,
				   struct nl_msg *msg,
				   int argc, char **argv,
				   enum id_input id)
{
	const struct mesh_param_descr *mdescr = NULL;

	if (argc > 1)
		return 1;

	if (argc == 1) {
		mdescr = find_mesh_param(argv[0]);
		if (!mdescr)
			return 2;
	}

	nl_cb_set(cb, NL_CB_VALID, NL_CB_CUSTOM,
		  print_mesh_param_handler, (void *)mdescr);
	return 0;
}

COMMAND(get, mesh_param, "[<param>]",
	NL80211_CMD_GET_MESH_PARAMS, 0, CIB_NETDEV, get_interface_meshparam,
	"Retrieve mesh parameter (run command without any to see available ones).");

static int join_mesh(struct nl80211_state *state, struct nl_cb *cb,
		     struct nl_msg *msg, int argc, char **argv,
		     enum id_input id)
{
	struct nlattr *container;
	float rate;
	char *end;

	if (argc < 1)
		return 1;

	NLA_PUT(msg, NL80211_ATTR_MESH_ID, strlen(argv[0]), argv[0]);
	argc--;
	argv++;

	if (argc > 1 && strcmp(argv[0], "mcast-rate") == 0) {
		argv++;
		argc--;

		rate = strtod(argv[0], &end);
		if (*end != '\0')
			return 1;

		NLA_PUT_U32(msg, NL80211_ATTR_MCAST_RATE, (int)(rate * 10));
		argv++;
		argc--;
	}

	container = nla_nest_start(msg, NL80211_ATTR_MESH_SETUP);
	if (!container)
		return -ENOBUFS;

	if (argc > 1 && strcmp(argv[0], "vendor_sync") == 0) {
		argv++;
		argc--;
		if (strcmp(argv[0], "on") == 0)
			NLA_PUT_U8(msg,
				   NL80211_MESH_SETUP_ENABLE_VENDOR_SYNC, 1);
		else
			NLA_PUT_U8(msg,
				   NL80211_MESH_SETUP_ENABLE_VENDOR_SYNC, 0);
		argv++;
		argc--;
	}
	/* parse and put other NL80211_ATTR_MESH_SETUP elements here */

	nla_nest_end(msg, container);

	if (!argc)
		return 0;
	return set_interface_meshparam(state, cb, msg, argc, argv, id);
 nla_put_failure:
	return -ENOBUFS;
}
COMMAND(mesh, join, "<mesh ID> [mcast-rate <rate in Mbps>] [vendor_sync on|off]"
	" [<param>=<value>]*",
	NL80211_CMD_JOIN_MESH, 0, CIB_NETDEV, join_mesh,
	"Join a mesh with the given mesh ID with mcast-rate and mesh parameters.");

static int leave_mesh(struct nl80211_state *state, struct nl_cb *cb,
		      struct nl_msg *msg, int argc, char **argv,
		      enum id_input id)
{
	if (argc)
		return 1;

	return 0;
}
COMMAND(mesh, leave, NULL, NL80211_CMD_LEAVE_MESH, 0, CIB_NETDEV, leave_mesh,
	"Leave a mesh.");
