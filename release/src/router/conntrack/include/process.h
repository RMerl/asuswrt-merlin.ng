#ifndef _PROCESS_H_
#define _PROCESS_H_

enum process_type {
	CTD_PROC_ANY,		/* any type */
	CTD_PROC_FLUSH,		/* flush process */
	CTD_PROC_COMMIT,	/* commit process */
	CTD_PROC_MAX
};

#define CTD_PROC_F_EXCL 	(1 << 0)  /* only one process at a time */

struct child_process {
	struct list_head	head;
	int			pid;
	int			type;
	void			(*cb)(void *data);
	void			*data;
};

int fork_process_new(int type, int flags, void (*cb)(void *data), void *data);
int fork_process_delete(int pid);
void fork_process_dump(int fd);

#endif
