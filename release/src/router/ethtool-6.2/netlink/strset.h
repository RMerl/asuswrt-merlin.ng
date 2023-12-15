/*
 * strset.h - string set handling
 *
 * Interface for local cache of ethtool string sets.
 */

#ifndef ETHTOOL_NETLINK_STRSET_H__
#define ETHTOOL_NETLINK_STRSET_H__

struct nl_socket;
struct stringset;

const struct stringset *global_stringset(unsigned int type,
					 struct nl_socket *nlsk);
const struct stringset *perdev_stringset(const char *dev, unsigned int type,
					 struct nl_socket *nlsk);

unsigned int get_count(const struct stringset *set);
const char *get_string(const struct stringset *set, unsigned int idx);

int preload_global_strings(struct nl_socket *nlsk);
int preload_perdev_strings(struct nl_socket *nlsk, const char *dev);
void cleanup_all_strings(void);

#endif /* ETHTOOL_NETLINK_STRSET_H__ */
