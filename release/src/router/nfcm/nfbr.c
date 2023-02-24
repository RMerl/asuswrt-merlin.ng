#include "nfbr.h"

static const char *state_names[5] = {
	[BR_STATE_DISABLED] = "disabled",
	[BR_STATE_LISTENING] = "listening",
	[BR_STATE_LEARNING] = "learning",
	[BR_STATE_FORWARDING] = "forwarding",
	[BR_STATE_BLOCKING] = "blocking",
};

const char *br_get_state_name(int state)
{
	if (state >= 0 && state <= 4)
		return state_names[state];

	return "<INVALID STATE>";
}

static FILE *fpopen(const char *dir, const char *name)
{
	char path[SYSFS_PATH_MAX];

	snprintf(path, SYSFS_PATH_MAX, "%s/%s", dir, name);
	return fopen(path, "r");
}

static void fetch_id(const char *dev, const char *name, struct bridge_id *id)
{
	FILE *f = fpopen(dev, name);

	if (!f)
		fprintf(stderr, "%s: %s\n", dev, strerror(errno));
	else {
		fscanf(f, "%2hhx%2hhx.%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx",
		       &id->prio[0], &id->prio[1],
		       &id->addr[0], &id->addr[1], &id->addr[2],
		       &id->addr[3], &id->addr[4], &id->addr[5]);
		fclose(f);
	}
}

/* Fetch an integer attribute out of sysfs. */
static int fetch_int(const char *dev, const char *name)
{
	FILE *f = fpopen(dev, name);
	int value = -1;

	if (!f)
		return 0;

	fscanf(f, "%i", &value);
	fclose(f);
	return value;
}

/* Get a time value out of sysfs */
static void fetch_tv(const char *dev, const char *name,
		    struct timeval *tv)
{
	__jiffies_to_tv(tv, fetch_int(dev, name));
}

static inline void __copy_fdb(struct fdb_entry *ent,
			      const struct __fdb_entry *f)
{
	memcpy(ent->mac_addr, f->mac_addr, 6);
	ent->port_no = f->port_no;
	ent->is_local = f->is_local;
	__jiffies_to_tv(&ent->ageing_timer_value, f->ageing_timer_value);
#if defined(CONFIG_BCM_VLAN_AGGREGATION)
	ent->vid = f->vid;
#endif
}

int br_read_fdb(const char *bridge, struct fdb_entry *fdbs,
		unsigned long offset, int num)
{
	FILE *f;
	int i, n;
	struct __fdb_entry fe[num];
	char path[SYSFS_PATH_MAX];

	/* open /sys/class/net/brXXX/brforward */
	snprintf(path, SYSFS_PATH_MAX, SYSFS_CLASS_NET "%s/brforward", bridge);
	f = fopen(path, "r");
	if (f) {
		fseek(f, offset*sizeof(struct __fdb_entry), SEEK_SET);
		n = fread(fe, sizeof(struct __fdb_entry), num, f);
		fclose(f);
	} else {
		/* old kernel, use ioctl */
		unsigned long args[4] = { BRCTL_GET_FDB_ENTRIES, (unsigned long) fe, num, offset };
		struct ifreq ifr;
		int retries = 0;

		strncpy(ifr.ifr_name, bridge, IFNAMSIZ);
		ifr.ifr_data = (char *) args;

	retry:
		n = ioctl(br_socket_fd, SIOCDEVPRIVATE, &ifr);

		/* table can change during ioctl processing */
		if (n < 0 && errno == EAGAIN && ++retries < 10) {
			sleep(0);
			goto retry;
		}
	}

	for (i = 0; i < n; i++)
		__copy_fdb(fdbs+i, fe+i);

	return n;
}

/*
 * Convert device name to an index in the list of ports in bridge.
 *
 * Old API does bridge operations as if ports were an array
 * inside bridge structure.
 */
static int get_portno(const char *brname, const char *ifname)
{
	int i;
	int ifindex = if_nametoindex(ifname);
	int ifindices[MAX_PORTS];
	unsigned long args[4] = { BRCTL_GET_PORT_LIST, (unsigned long)ifindices, MAX_PORTS, 0 };
	struct ifreq ifr;

	if (ifindex <= 0)
		goto error;

	memset(ifindices, 0, sizeof(ifindices));
	strncpy(ifr.ifr_name, brname, IFNAMSIZ);
	ifr.ifr_data = (char *) &args;

	if (ioctl(br_socket_fd, SIOCDEVPRIVATE, &ifr) < 0) {
		dprintf("get_portno: get ports of %s failed: %s\n", brname, strerror(errno));
		goto error;
	}

	for (i = 0; i < MAX_PORTS; i++) {
		if (ifindices[i] == ifindex)
			return i;
	}

	dprintf("%s is not a in bridge %s\n", ifname, brname);
 error:
	return -1;
}

/* get information via ioctl */
static int old_get_bridge_info(const char *bridge, struct bridge_info *info)
{
	struct ifreq ifr;
	struct __bridge_info i;
	unsigned long args[4] = { BRCTL_GET_BRIDGE_INFO, (unsigned long) &i, 0, 0 };

	memset(info, 0, sizeof(*info));
	strncpy(ifr.ifr_name, bridge, IFNAMSIZ);
	ifr.ifr_data = (char *) &args;

	if (ioctl(br_socket_fd, SIOCDEVPRIVATE, &ifr) < 0) {
		dprintf("%s: can't get info %s\n", bridge, strerror(errno));
		return errno;
	}

	memcpy(&info->designated_root, &i.designated_root, 8);
	memcpy(&info->bridge_id, &i.bridge_id, 8);
	info->root_path_cost = i.root_path_cost;
	info->root_port = i.root_port;
	info->topology_change = i.topology_change;
	info->topology_change_detected = i.topology_change_detected;
	info->stp_enabled = i.stp_enabled;
	__jiffies_to_tv(&info->max_age, i.max_age);
	__jiffies_to_tv(&info->hello_time, i.hello_time);
	__jiffies_to_tv(&info->forward_delay, i.forward_delay);
	__jiffies_to_tv(&info->bridge_max_age, i.bridge_max_age);
	__jiffies_to_tv(&info->bridge_hello_time, i.bridge_hello_time);
	__jiffies_to_tv(&info->bridge_forward_delay, i.bridge_forward_delay);
	__jiffies_to_tv(&info->ageing_time, i.ageing_time);
	__jiffies_to_tv(&info->hello_timer_value, i.hello_timer_value);
	__jiffies_to_tv(&info->tcn_timer_value, i.tcn_timer_value);
	__jiffies_to_tv(&info->topology_change_timer_value, i.topology_change_timer_value);
	__jiffies_to_tv(&info->gc_timer_value, i.gc_timer_value);

	return 0;
}

/*
 * Get bridge parameters using either sysfs or old
 * ioctl.
 */
int br_get_bridge_info(const char *bridge, struct bridge_info *info)
{
	DIR *dir;
	char path[SYSFS_PATH_MAX];

	snprintf(path, SYSFS_PATH_MAX, SYSFS_CLASS_NET "%s/bridge", bridge);
	dir = opendir(path);
	if (dir == NULL) {
		dprintf("path '%s' is not a directory\n", path);
		goto fallback;
	}

	memset(info, 0, sizeof(*info));
	fetch_id(path, "root_id", &info->designated_root);
	fetch_id(path, "bridge_id", &info->bridge_id);
	info->root_path_cost = fetch_int(path, "root_path_cost");
	fetch_tv(path, "max_age", &info->max_age);
	fetch_tv(path, "hello_time", &info->hello_time);
	fetch_tv(path, "forward_delay", &info->forward_delay);
	fetch_tv(path, "max_age", &info->bridge_max_age);
	fetch_tv(path, "hello_time", &info->bridge_hello_time);
	fetch_tv(path, "forward_delay", &info->bridge_forward_delay);
	fetch_tv(path, "ageing_time", &info->ageing_time);
	fetch_tv(path, "hello_timer", &info->hello_timer_value);
	fetch_tv(path, "tcn_timer", &info->tcn_timer_value);
	fetch_tv(path, "topology_change_timer", &info->topology_change_timer_value);;
	fetch_tv(path, "gc_timer", &info->gc_timer_value);

	info->root_port = fetch_int(path, "root_port");
	info->stp_enabled = fetch_int(path, "stp_state");
	info->topology_change = fetch_int(path, "topology_change");
	info->topology_change_detected = fetch_int(path, "topology_change_detected");

	closedir(dir);
	return 0;

fallback:
	return old_get_bridge_info(bridge, info);
}

static int old_get_port_info(const char *brname, const char *port, struct port_info *info)
{
	struct __port_info i;
	int index;

	memset(info, 0, sizeof(*info));

	index = get_portno(brname, port);
	if (index < 0)
		return errno;

	else {
		struct ifreq ifr;
		unsigned long args[4] = { BRCTL_GET_PORT_INFO, (unsigned long) &i, index, 0 };

		strncpy(ifr.ifr_name, brname, IFNAMSIZ);
		ifr.ifr_data = (char *) &args;

		if (ioctl(br_socket_fd, SIOCDEVPRIVATE, &ifr) < 0) {
			dprintf("old can't get port %s(%d) info %s\n", brname, index, strerror(errno));
			return errno;
		}
	}

	info->port_no = index;
	memcpy(&info->designated_root, &i.designated_root, 8);
	memcpy(&info->designated_bridge, &i.designated_bridge, 8);
	info->port_id = i.port_id;
	info->designated_port = i.designated_port;
	info->path_cost = i.path_cost;
	info->designated_cost = i.designated_cost;
	info->state = i.state;
	info->top_change_ack = i.top_change_ack;
	info->config_pending = i.config_pending;
	__jiffies_to_tv(&info->message_age_timer_value, i.message_age_timer_value);
	__jiffies_to_tv(&info->forward_delay_timer_value, i.forward_delay_timer_value);
	__jiffies_to_tv(&info->hold_timer_value, i.hold_timer_value);
	info->hairpin_mode = 0;
	return 0;
}

/*
 * Get information about port on bridge.
 */
int br_get_port_info(const char *brname, const char *port, struct port_info *info)
{
	DIR *d;
	char path[SYSFS_PATH_MAX];

	snprintf(path, SYSFS_PATH_MAX, SYSFS_CLASS_NET "%s/brport", port);
	d = opendir(path);
	if (!d)
		goto fallback;

	memset(info, 0, sizeof(*info));

	fetch_id(path, "designated_root", &info->designated_root);
	fetch_id(path, "designated_bridge", &info->designated_bridge);
	info->port_no = fetch_int(path, "port_no");
	info->port_id = fetch_int(path, "port_id");
	info->designated_port = fetch_int(path, "designated_port");
	info->path_cost = fetch_int(path, "path_cost");
	info->designated_cost = fetch_int(path, "designated_cost");
	info->state = fetch_int(path, "state");
	info->top_change_ack = fetch_int(path, "change_ack");
	info->config_pending = fetch_int(path, "config_pending");
	fetch_tv(path, "message_age_timer", &info->message_age_timer_value);
	fetch_tv(path, "forward_delay_timer", &info->forward_delay_timer_value);
	fetch_tv(path, "hold_timer", &info->hold_timer_value);
	info->hairpin_mode = fetch_int(path, "hairpin_mode");

	closedir(d);

	return 0;
fallback:
	return old_get_port_info(brname, port, info);
}

static int compare_fdbs(const void *_f0, const void *_f1)
{
	const struct fdb_entry *f0 = _f0;
	const struct fdb_entry *f1 = _f1;

	return memcmp(f0->mac_addr, f1->mac_addr, 6);
}

int br_cmd_showmacs(int argc, char *const* argv)
{
    const char *brname = argv[1];
#define CHUNK 128
    int i, n;
    struct fdb_entry *fdb = NULL;
    int offset = 0;

    for(;;) {
        fdb = realloc(fdb, (offset + CHUNK) * sizeof(struct fdb_entry));
        if (!fdb) {
            fprintf(stderr, "Out of memory\n");
            return 1;
        }

        n = br_read_fdb(brname, fdb+offset, offset, CHUNK);
        if (n == 0)
            break;

        if (n < 0) {
            fprintf(stderr, "read of forward table failed: %s\n", strerror(errno));
            return 1;
        }

        offset += n;
    }

    qsort(fdb, offset, sizeof(struct fdb_entry), compare_fdbs);

    printf("port no\tmac addr\t\tis local?\tageing timer\n");
    for (i = 0; i < offset; i++) {
        const struct fdb_entry *f = fdb + i;
        printf("%3i\t", f->port_no);
        printf("%.2x:%.2x:%.2x:%.2x:%.2x:%.2x\t",
               f->mac_addr[0], f->mac_addr[1], f->mac_addr[2],
               f->mac_addr[3], f->mac_addr[4], f->mac_addr[5]);
        printf("%s\t\t", f->is_local?"yes":"no");
        br_show_timer(&f->ageing_timer_value);
        printf("\n");
    }
    return 0;
}

int br_cmd_showstp(int argc, char *const* argv)
{
    struct bridge_info info;

    if (br_get_bridge_info(argv[1], &info)) {
        fprintf(stderr, "%s: can't get info %s\n", argv[1], strerror(errno));
        return 1;
    }

    br_dump_info(argv[1], &info);
    return 0;
}
