#define _ATFILE_SOURCE
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/inotify.h>
#include <sys/mount.h>
#include <sys/syscall.h>
#include <stdio.h>
#include <string.h>
#include <sched.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <unistd.h>
#include <ctype.h>
#include <linux/limits.h>

#include <linux/net_namespace.h>

#include "utils.h"
#include "hlist.h"
#include "ip_common.h"
#include "namespace.h"

static int usage(void)
{
	fprintf(stderr, "Usage: ip netns list\n");
	fprintf(stderr, "       ip netns add NAME\n");
	fprintf(stderr, "       ip netns set NAME NETNSID\n");
	fprintf(stderr, "       ip [-all] netns delete [NAME]\n");
	fprintf(stderr, "       ip netns identify [PID]\n");
	fprintf(stderr, "       ip netns pids NAME\n");
	fprintf(stderr, "       ip [-all] netns exec [NAME] cmd ...\n");
	fprintf(stderr, "       ip netns monitor\n");
	fprintf(stderr, "       ip netns list-id\n");
	exit(-1);
}

/* This socket is used to get nsid */
static struct rtnl_handle rtnsh = { .fd = -1 };

static int have_rtnl_getnsid = -1;

static int ipnetns_accept_msg(const struct sockaddr_nl *who,
			      struct rtnl_ctrl_data *ctrl,
			      struct nlmsghdr *n, void *arg)
{
	struct nlmsgerr *err = (struct nlmsgerr *)NLMSG_DATA(n);

	if (n->nlmsg_type == NLMSG_ERROR &&
	    (err->error == -EOPNOTSUPP || err->error == -EINVAL))
		have_rtnl_getnsid = 0;
	else
		have_rtnl_getnsid = 1;
	return -1;
}

static int ipnetns_have_nsid(void)
{
	struct {
		struct nlmsghdr n;
		struct rtgenmsg g;
		char            buf[1024];
	} req;
	int fd;

	if (have_rtnl_getnsid < 0) {
		memset(&req, 0, sizeof(req));
		req.n.nlmsg_len = NLMSG_LENGTH(sizeof(struct rtgenmsg));
		req.n.nlmsg_flags = NLM_F_REQUEST;
		req.n.nlmsg_type = RTM_GETNSID;
		req.g.rtgen_family = AF_UNSPEC;

		fd = open("/proc/self/ns/net", O_RDONLY);
		if (fd < 0) {
			perror("open(\"/proc/self/ns/net\")");
			exit(1);
		}

		addattr32(&req.n, 1024, NETNSA_FD, fd);

		if (rtnl_send(&rth, &req.n, req.n.nlmsg_len) < 0) {
			perror("request send failed");
			exit(1);
		}
		rtnl_listen(&rth, ipnetns_accept_msg, NULL);
		close(fd);
	}

	return have_rtnl_getnsid;
}

static int get_netnsid_from_name(const char *name)
{
	struct {
		struct nlmsghdr n;
		struct rtgenmsg g;
		char            buf[1024];
	} req, answer;
	struct rtattr *tb[NETNSA_MAX + 1];
	struct rtgenmsg *rthdr;
	int len, fd;

	memset(&req, 0, sizeof(req));
	req.n.nlmsg_len = NLMSG_LENGTH(sizeof(struct rtgenmsg));
	req.n.nlmsg_flags = NLM_F_REQUEST;
	req.n.nlmsg_type = RTM_GETNSID;
	req.g.rtgen_family = AF_UNSPEC;

	fd = netns_get_fd(name);
	if (fd < 0)
		return fd;

	addattr32(&req.n, 1024, NETNSA_FD, fd);
	if (rtnl_talk(&rtnsh, &req.n, &answer.n, sizeof(answer)) < 0) {
		close(fd);
		return -2;
	}
	close(fd);

	/* Validate message and parse attributes */
	if (answer.n.nlmsg_type == NLMSG_ERROR)
		return -1;

	rthdr = NLMSG_DATA(&answer.n);
	len = answer.n.nlmsg_len - NLMSG_SPACE(sizeof(*rthdr));
	if (len < 0)
		return -1;

	parse_rtattr(tb, NETNSA_MAX, NETNS_RTA(rthdr), len);

	if (tb[NETNSA_NSID])
		return rta_getattr_u32(tb[NETNSA_NSID]);

	return -1;
}

struct nsid_cache {
	struct hlist_node	nsid_hash;
	struct hlist_node	name_hash;
	int			nsid;
	char			name[0];
};

#define NSIDMAP_SIZE		128
#define NSID_HASH_NSID(nsid)	(nsid & (NSIDMAP_SIZE - 1))
#define NSID_HASH_NAME(name)	(namehash(name) & (NSIDMAP_SIZE - 1))

static struct hlist_head	nsid_head[NSIDMAP_SIZE];
static struct hlist_head	name_head[NSIDMAP_SIZE];

static struct nsid_cache *netns_map_get_by_nsid(int nsid)
{
	uint32_t h = NSID_HASH_NSID(nsid);
	struct hlist_node *n;

	hlist_for_each(n, &nsid_head[h]) {
		struct nsid_cache *c = container_of(n, struct nsid_cache,
						    nsid_hash);
		if (c->nsid == nsid)
			return c;
	}

	return NULL;
}

static int netns_map_add(int nsid, const char *name)
{
	struct nsid_cache *c;
	uint32_t h;

	if (netns_map_get_by_nsid(nsid) != NULL)
		return -EEXIST;

	c = malloc(sizeof(*c) + strlen(name));
	if (c == NULL) {
		perror("malloc");
		return -ENOMEM;
	}
	c->nsid = nsid;
	strcpy(c->name, name);

	h = NSID_HASH_NSID(nsid);
	hlist_add_head(&c->nsid_hash, &nsid_head[h]);

	h = NSID_HASH_NAME(name);
	hlist_add_head(&c->name_hash, &name_head[h]);

	return 0;
}

static void netns_map_del(struct nsid_cache *c)
{
	hlist_del(&c->name_hash);
	hlist_del(&c->nsid_hash);
	free(c);
}

void netns_map_init(void)
{
	static int initialized;
	struct dirent *entry;
	DIR *dir;
	int nsid;

	if (initialized || !ipnetns_have_nsid())
		return;

	if (rtnl_open(&rtnsh, 0) < 0) {
		fprintf(stderr, "Cannot open rtnetlink\n");
		exit(1);
	}

	dir = opendir(NETNS_RUN_DIR);
	if (!dir)
		return;

	while ((entry = readdir(dir)) != NULL) {
		if (strcmp(entry->d_name, ".") == 0)
			continue;
		if (strcmp(entry->d_name, "..") == 0)
			continue;
		nsid = get_netnsid_from_name(entry->d_name);

		if (nsid >= 0)
			netns_map_add(nsid, entry->d_name);
	}
	closedir(dir);
	initialized = 1;
}

static int netns_get_name(int nsid, char *name)
{
	struct dirent *entry;
	DIR *dir;
	int id;

	dir = opendir(NETNS_RUN_DIR);
	if (!dir)
		return -ENOENT;

	while ((entry = readdir(dir)) != NULL) {
		if (strcmp(entry->d_name, ".") == 0)
			continue;
		if (strcmp(entry->d_name, "..") == 0)
			continue;
		id = get_netnsid_from_name(entry->d_name);

		if (nsid == id) {
			strcpy(name, entry->d_name);
			closedir(dir);
			return 0;
		}
	}
	closedir(dir);
	return -ENOENT;
}

int print_nsid(const struct sockaddr_nl *who, struct nlmsghdr *n, void *arg)
{
	struct rtgenmsg *rthdr = NLMSG_DATA(n);
	struct rtattr *tb[NETNSA_MAX+1];
	int len = n->nlmsg_len;
	FILE *fp = (FILE *)arg;
	struct nsid_cache *c;
	char name[NAME_MAX];
	int nsid;

	if (n->nlmsg_type != RTM_NEWNSID && n->nlmsg_type != RTM_DELNSID)
		return 0;

	len -= NLMSG_SPACE(sizeof(*rthdr));
	if (len < 0) {
		fprintf(stderr, "BUG: wrong nlmsg len %d in %s\n", len,
			__func__);
		return -1;
	}

	parse_rtattr(tb, NETNSA_MAX, NETNS_RTA(rthdr), len);
	if (tb[NETNSA_NSID] == NULL) {
		fprintf(stderr, "BUG: NETNSA_NSID is missing %s\n", __func__);
		return -1;
	}

	if (n->nlmsg_type == RTM_DELNSID)
		fprintf(fp, "Deleted ");

	nsid = rta_getattr_u32(tb[NETNSA_NSID]);
	fprintf(fp, "nsid %u ", nsid);

	c = netns_map_get_by_nsid(nsid);
	if (c != NULL) {
		fprintf(fp, "(iproute2 netns name: %s)", c->name);
		netns_map_del(c);
	}

	/* During 'ip monitor nsid', no chance to have new nsid in cache. */
	if (c == NULL && n->nlmsg_type == RTM_NEWNSID)
		if (netns_get_name(nsid, name) == 0) {
			fprintf(fp, "(iproute2 netns name: %s)", name);
			netns_map_add(nsid, name);
		}

	fprintf(fp, "\n");
	fflush(fp);
	return 0;
}

static int netns_list_id(int argc, char **argv)
{
	if (!ipnetns_have_nsid()) {
		fprintf(stderr,
			"RTM_GETNSID is not supported by the kernel.\n");
		return -ENOTSUP;
	}

	if (rtnl_wilddump_request(&rth, AF_UNSPEC, RTM_GETNSID) < 0) {
		perror("Cannot send dump request");
		exit(1);
	}
	if (rtnl_dump_filter(&rth, print_nsid, stdout) < 0) {
		fprintf(stderr, "Dump terminated\n");
		exit(1);
	}
	return 0;
}

static int netns_list(int argc, char **argv)
{
	struct dirent *entry;
	DIR *dir;
	int id;

	dir = opendir(NETNS_RUN_DIR);
	if (!dir)
		return 0;

	while ((entry = readdir(dir)) != NULL) {
		if (strcmp(entry->d_name, ".") == 0)
			continue;
		if (strcmp(entry->d_name, "..") == 0)
			continue;
		printf("%s", entry->d_name);
		if (ipnetns_have_nsid()) {
			id = get_netnsid_from_name(entry->d_name);
			if (id >= 0)
				printf(" (id: %d)", id);
		}
		printf("\n");
	}
	closedir(dir);
	return 0;
}

static int cmd_exec(const char *cmd, char **argv, bool do_fork)
{
	fflush(stdout);
	if (do_fork) {
		int status;
		pid_t pid;

		pid = fork();
		if (pid < 0) {
			perror("fork");
			exit(1);
		}

		if (pid != 0) {
			/* Parent  */
			if (waitpid(pid, &status, 0) < 0) {
				perror("waitpid");
				exit(1);
			}

			if (WIFEXITED(status)) {
				return WEXITSTATUS(status);
			}

			exit(1);
		}
	}

	if (execvp(cmd, argv)  < 0)
		fprintf(stderr, "exec of \"%s\" failed: %s\n",
				cmd, strerror(errno));
	_exit(1);
}

static int on_netns_exec(char *nsname, void *arg)
{
	char **argv = arg;
	cmd_exec(argv[1], argv + 1, true);
	return 0;
}

static int netns_exec(int argc, char **argv)
{
	/* Setup the proper environment for apps that are not netns
	 * aware, and execute a program in that environment.
	 */
	const char *cmd;

	if (argc < 1 && !do_all) {
		fprintf(stderr, "No netns name specified\n");
		return -1;
	}
	if ((argc < 2 && !do_all) || (argc < 1 && do_all)) {
		fprintf(stderr, "No command specified\n");
		return -1;
	}

	if (do_all)
		return do_each_netns(on_netns_exec, --argv, 1);

	if (netns_switch(argv[0]))
		return -1;

	/* ip must return the status of the child,
	 * but do_cmd() will add a minus to this,
	 * so let's add another one here to cancel it.
	 */
	cmd = argv[1];
	return -cmd_exec(cmd, argv + 1, !!batch_mode);
}

static int is_pid(const char *str)
{
	int ch;
	for (; (ch = *str); str++) {
		if (!isdigit(ch))
			return 0;
	}
	return 1;
}

static int netns_pids(int argc, char **argv)
{
	const char *name;
	char net_path[PATH_MAX];
	int netns;
	struct stat netst;
	DIR *dir;
	struct dirent *entry;

	if (argc < 1) {
		fprintf(stderr, "No netns name specified\n");
		return -1;
	}
	if (argc > 1) {
		fprintf(stderr, "extra arguments specified\n");
		return -1;
	}

	name = argv[0];
	snprintf(net_path, sizeof(net_path), "%s/%s", NETNS_RUN_DIR, name);
	netns = open(net_path, O_RDONLY);
	if (netns < 0) {
		fprintf(stderr, "Cannot open network namespace: %s\n",
			strerror(errno));
		return -1;
	}
	if (fstat(netns, &netst) < 0) {
		fprintf(stderr, "Stat of netns failed: %s\n",
			strerror(errno));
		return -1;
	}
	dir = opendir("/proc/");
	if (!dir) {
		fprintf(stderr, "Open of /proc failed: %s\n",
			strerror(errno));
		return -1;
	}
	while((entry = readdir(dir))) {
		char pid_net_path[PATH_MAX];
		struct stat st;
		if (!is_pid(entry->d_name))
			continue;
		snprintf(pid_net_path, sizeof(pid_net_path), "/proc/%s/ns/net",
			entry->d_name);
		if (stat(pid_net_path, &st) != 0)
			continue;
		if ((st.st_dev == netst.st_dev) &&
		    (st.st_ino == netst.st_ino)) {
			printf("%s\n", entry->d_name);
		}
	}
	closedir(dir);
	return 0;

}

static int netns_identify(int argc, char **argv)
{
	const char *pidstr;
	char net_path[PATH_MAX];
	int netns;
	struct stat netst;
	DIR *dir;
	struct dirent *entry;

	if (argc < 1) {
		pidstr = "self";
	} else if (argc > 1) {
		fprintf(stderr, "extra arguments specified\n");
		return -1;
	} else {
		pidstr = argv[0];
		if (!is_pid(pidstr)) {
			fprintf(stderr, "Specified string '%s' is not a pid\n",
					pidstr);
			return -1;
		}
	}

	snprintf(net_path, sizeof(net_path), "/proc/%s/ns/net", pidstr);
	netns = open(net_path, O_RDONLY);
	if (netns < 0) {
		fprintf(stderr, "Cannot open network namespace: %s\n",
			strerror(errno));
		return -1;
	}
	if (fstat(netns, &netst) < 0) {
		fprintf(stderr, "Stat of netns failed: %s\n",
			strerror(errno));
		return -1;
	}
	dir = opendir(NETNS_RUN_DIR);
	if (!dir) {
		/* Succeed treat a missing directory as an empty directory */
		if (errno == ENOENT)
			return 0;

		fprintf(stderr, "Failed to open directory %s:%s\n",
			NETNS_RUN_DIR, strerror(errno));
		return -1;
	}

	while((entry = readdir(dir))) {
		char name_path[PATH_MAX];
		struct stat st;

		if (strcmp(entry->d_name, ".") == 0)
			continue;
		if (strcmp(entry->d_name, "..") == 0)
			continue;

		snprintf(name_path, sizeof(name_path), "%s/%s",	NETNS_RUN_DIR,
			entry->d_name);

		if (stat(name_path, &st) != 0)
			continue;

		if ((st.st_dev == netst.st_dev) &&
		    (st.st_ino == netst.st_ino)) {
			printf("%s\n", entry->d_name);
		}
	}
	closedir(dir);
	return 0;

}

static int on_netns_del(char *nsname, void *arg)
{
	char netns_path[PATH_MAX];

	snprintf(netns_path, sizeof(netns_path), "%s/%s", NETNS_RUN_DIR, nsname);
	umount2(netns_path, MNT_DETACH);
	if (unlink(netns_path) < 0) {
		fprintf(stderr, "Cannot remove namespace file \"%s\": %s\n",
			netns_path, strerror(errno));
		return -1;
	}
	return 0;
}

static int netns_delete(int argc, char **argv)
{
	if (argc < 1 && !do_all) {
		fprintf(stderr, "No netns name specified\n");
		return -1;
	}

	if (do_all)
		return netns_foreach(on_netns_del, NULL);

	return on_netns_del(argv[0], NULL);
}

static int create_netns_dir(void)
{
	/* Create the base netns directory if it doesn't exist */
	if (mkdir(NETNS_RUN_DIR, S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH)) {
		if (errno != EEXIST) {
			fprintf(stderr, "mkdir %s failed: %s\n",
				NETNS_RUN_DIR, strerror(errno));
			return -1;
		}
	}

	return 0;
}

static int netns_add(int argc, char **argv)
{
	/* This function creates a new network namespace and
	 * a new mount namespace and bind them into a well known
	 * location in the filesystem based on the name provided.
	 *
	 * The mount namespace is created so that any necessary
	 * userspace tweaks like remounting /sys, or bind mounting
	 * a new /etc/resolv.conf can be shared between uers.
	 */
	char netns_path[PATH_MAX];
	const char *name;
	int fd;
	int made_netns_run_dir_mount = 0;

	if (argc < 1) {
		fprintf(stderr, "No netns name specified\n");
		return -1;
	}
	name = argv[0];

	snprintf(netns_path, sizeof(netns_path), "%s/%s", NETNS_RUN_DIR, name);

	if (create_netns_dir())
		return -1;

	/* Make it possible for network namespace mounts to propagate between
	 * mount namespaces.  This makes it likely that a unmounting a network
	 * namespace file in one namespace will unmount the network namespace
	 * file in all namespaces allowing the network namespace to be freed
	 * sooner.
	 */
	while (mount("", NETNS_RUN_DIR, "none", MS_SHARED | MS_REC, NULL)) {
		/* Fail unless we need to make the mount point */
		if (errno != EINVAL || made_netns_run_dir_mount) {
			fprintf(stderr, "mount --make-shared %s failed: %s\n",
				NETNS_RUN_DIR, strerror(errno));
			return -1;
		}

		/* Upgrade NETNS_RUN_DIR to a mount point */
		if (mount(NETNS_RUN_DIR, NETNS_RUN_DIR, "none", MS_BIND, NULL)) {
			fprintf(stderr, "mount --bind %s %s failed: %s\n",
				NETNS_RUN_DIR, NETNS_RUN_DIR, strerror(errno));
			return -1;
		}
		made_netns_run_dir_mount = 1;
	}

	/* Create the filesystem state */
	fd = open(netns_path, O_RDONLY|O_CREAT|O_EXCL, 0);
	if (fd < 0) {
		fprintf(stderr, "Cannot create namespace file \"%s\": %s\n",
			netns_path, strerror(errno));
		return -1;
	}
	close(fd);
	if (unshare(CLONE_NEWNET) < 0) {
		fprintf(stderr, "Failed to create a new network namespace \"%s\": %s\n",
			name, strerror(errno));
		goto out_delete;
	}

	/* Bind the netns last so I can watch for it */
	if (mount("/proc/self/ns/net", netns_path, "none", MS_BIND, NULL) < 0) {
		fprintf(stderr, "Bind /proc/self/ns/net -> %s failed: %s\n",
			netns_path, strerror(errno));
		goto out_delete;
	}
	return 0;
out_delete:
	netns_delete(argc, argv);
	return -1;
}

static int set_netnsid_from_name(const char *name, int nsid)
{
	struct {
		struct nlmsghdr n;
		struct rtgenmsg g;
		char            buf[1024];
	} req;
	int fd, err = 0;

	memset(&req, 0, sizeof(req));
	req.n.nlmsg_len = NLMSG_LENGTH(sizeof(struct rtgenmsg));
	req.n.nlmsg_flags = NLM_F_REQUEST;
	req.n.nlmsg_type = RTM_NEWNSID;
	req.g.rtgen_family = AF_UNSPEC;

	fd = netns_get_fd(name);
	if (fd < 0)
		return fd;

	addattr32(&req.n, 1024, NETNSA_FD, fd);
	addattr32(&req.n, 1024, NETNSA_NSID, nsid);
	if (rtnl_talk(&rth, &req.n, NULL, 0) < 0)
		err = -2;

	close(fd);
	return err;
}

static int netns_set(int argc, char **argv)
{
	char netns_path[PATH_MAX];
	const char *name;
	int netns, nsid;

	if (argc < 1) {
		fprintf(stderr, "No netns name specified\n");
		return -1;
	}
	if (argc < 2) {
		fprintf(stderr, "No nsid specified\n");
		return -1;
	}
	name = argv[0];
	nsid = atoi(argv[1]);

	snprintf(netns_path, sizeof(netns_path), "%s/%s", NETNS_RUN_DIR, name);
	netns = open(netns_path, O_RDONLY | O_CLOEXEC);
	if (netns < 0) {
		fprintf(stderr, "Cannot open network namespace \"%s\": %s\n",
			name, strerror(errno));
		return -1;
	}

	return set_netnsid_from_name(name, nsid);
}

static int netns_monitor(int argc, char **argv)
{
	char buf[4096];
	struct inotify_event *event;
	int fd;
	fd = inotify_init();
	if (fd < 0) {
		fprintf(stderr, "inotify_init failed: %s\n",
			strerror(errno));
		return -1;
	}

	if (create_netns_dir())
		return -1;

	if (inotify_add_watch(fd, NETNS_RUN_DIR, IN_CREATE | IN_DELETE) < 0) {
		fprintf(stderr, "inotify_add_watch failed: %s\n",
			strerror(errno));
		return -1;
	}
	for(;;) {
		ssize_t len = read(fd, buf, sizeof(buf));
		if (len < 0) {
			fprintf(stderr, "read failed: %s\n",
				strerror(errno));
			return -1;
		}
		for (event = (struct inotify_event *)buf;
		     (char *)event < &buf[len];
		     event = (struct inotify_event *)((char *)event + sizeof(*event) + event->len)) {
			if (event->mask & IN_CREATE)
				printf("add %s\n", event->name);
			if (event->mask & IN_DELETE)
				printf("delete %s\n", event->name);
		}
	}
	return 0;
}

int do_netns(int argc, char **argv)
{
	netns_map_init();

	if (argc < 1)
		return netns_list(0, NULL);

	if ((matches(*argv, "list") == 0) || (matches(*argv, "show") == 0) ||
	    (matches(*argv, "lst") == 0))
		return netns_list(argc-1, argv+1);

	if ((matches(*argv, "list-id") == 0))
		return netns_list_id(argc-1, argv+1);

	if (matches(*argv, "help") == 0)
		return usage();

	if (matches(*argv, "add") == 0)
		return netns_add(argc-1, argv+1);

	if (matches(*argv, "set") == 0)
		return netns_set(argc-1, argv+1);

	if (matches(*argv, "delete") == 0)
		return netns_delete(argc-1, argv+1);

	if (matches(*argv, "identify") == 0)
		return netns_identify(argc-1, argv+1);

	if (matches(*argv, "pids") == 0)
		return netns_pids(argc-1, argv+1);

	if (matches(*argv, "exec") == 0)
		return netns_exec(argc-1, argv+1);

	if (matches(*argv, "monitor") == 0)
		return netns_monitor(argc-1, argv+1);

	fprintf(stderr, "Command \"%s\" is unknown, try \"ip netns help\".\n", *argv);
	exit(-1);
}
