#ifndef _LINUX_RESOURCE_H
#define _LINUX_RESOURCE_H

#include <uapi/linux/resource.h>


struct task_struct;

int getrusage(struct task_struct *p, int who, struct rusage __user *ru);
int do_prlimit(struct task_struct *tsk, unsigned int resource,
		struct rlimit *new_rlim, struct rlimit *old_rlim);

#endif
