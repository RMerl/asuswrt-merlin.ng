#ifndef _FDS_H_
#define _FDS_H_

#include "linux_list.h"

struct fds {
	int	maxfd;
	fd_set	readfds;
	struct list_head list;
};

struct fds_item {
	struct list_head        head;
	int                     fd;
	void			(*cb)(void *data);
	void			*data;
};

struct fds *create_fds(void);
void destroy_fds(struct fds *);
int register_fd(int fd, void (*cb)(void *data), void *data, struct fds *fds);
int unregister_fd(int fd, struct fds *fds);

#endif
