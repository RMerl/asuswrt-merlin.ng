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

SECTION(vendor);

static int read_file(FILE *file, char *buf, size_t size)
{
	int data, count = 0;

	while ((data = fgetc(file)) != EOF) {
		if (count >= size)
			return -EINVAL;
		buf[count] = data;
		count++;
	}

	return count;
}

static int read_hex(int argc, char **argv, char *buf, size_t size)
{
	int i, res;
	unsigned int data;

	if (argc > size)
		return -EINVAL;

	for (i = 0; i < argc; i++) {
		res = sscanf(argv[i], "0x%x", &data);
		if (res != 1 || data > 0xff)
			return -EINVAL;
		buf[i] = data;
	}

	return argc;
}

static int handle_vendor(struct nl80211_state *state, struct nl_cb *cb,
			 struct nl_msg *msg, int argc, char **argv,
			 enum id_input id)
{
	unsigned int oui;
	unsigned int subcmd;
	char buf[2048] = {};
	int res, count = 0;
	FILE *file = NULL;

	if (argc < 3)
		return -EINVAL;

	res = sscanf(argv[0], "0x%x", &oui);
	if (res != 1)
		return -EINVAL;

	res = sscanf(argv[1], "0x%x", &subcmd);
	if (res != 1)
		return -EINVAL;

	if (!strcmp(argv[2], "-"))
		file = stdin;
	else
		file = fopen(argv[2], "r");

	NLA_PUT_U32(msg, NL80211_ATTR_VENDOR_ID, oui);
	NLA_PUT_U32(msg, NL80211_ATTR_VENDOR_SUBCMD, subcmd);

	if (file) {
		count = read_file(file, buf, sizeof(buf));
		fclose(file);
	} else
		count = read_hex(argc - 2, &argv[2], buf, sizeof(buf));

	if (count < 0)
		return -EINVAL;

	if (count > 0)
		NLA_PUT(msg, NL80211_ATTR_VENDOR_DATA, count, buf);

	return 0;

nla_put_failure:
	return -ENOBUFS;
}

COMMAND(vendor, send, "<oui> <subcmd> <filename|-|hex data>", NL80211_CMD_VENDOR, 0, CIB_NETDEV, handle_vendor, "");
