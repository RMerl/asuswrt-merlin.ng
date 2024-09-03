#include <netlink/attr.h>

int nla_put_flag(struct nl_msg *msg, int flag)
{
	return nla_put(msg, flag, 0, NULL);
}
