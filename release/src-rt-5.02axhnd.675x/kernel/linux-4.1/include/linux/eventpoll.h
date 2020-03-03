/*
 *  include/linux/eventpoll.h ( Efficient event polling implementation )
 *  Copyright (C) 2001,...,2006	 Davide Libenzi
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  Davide Libenzi <davidel@xmailserver.org>
 *
 */
#ifndef _LINUX_EVENTPOLL_H
#define _LINUX_EVENTPOLL_H

#include <uapi/linux/eventpoll.h>


/* Forward declarations to avoid compiler errors */
struct file;


#ifdef CONFIG_EPOLL

/* Used to initialize the epoll bits inside the "struct file" */
static inline void eventpoll_init_file(struct file *file)
{
	INIT_LIST_HEAD(&file->f_ep_links);
	INIT_LIST_HEAD(&file->f_tfile_llink);
}


/* Used to release the epoll bits inside the "struct file" */
void eventpoll_release_file(struct file *file);

/*
 * This is called from inside fs/file_table.c:__fput() to unlink files
 * from the eventpoll interface. We need to have this facility to cleanup
 * correctly files that are closed without being removed from the eventpoll
 * interface.
 */
static inline void eventpoll_release(struct file *file)
{

	/*
	 * Fast check to avoid the get/release of the semaphore. Since
	 * we're doing this outside the semaphore lock, it might return
	 * false negatives, but we don't care. It'll help in 99.99% of cases
	 * to avoid the semaphore lock. False positives simply cannot happen
	 * because the file in on the way to be removed and nobody ( but
	 * eventpoll ) has still a reference to this file.
	 */
	if (likely(list_empty(&file->f_ep_links)))
		return;

	/*
	 * The file is being closed while it is still linked to an epoll
	 * descriptor. We need to handle this by correctly unlinking it
	 * from its containers.
	 */
	eventpoll_release_file(file);
}

#else

static inline void eventpoll_init_file(struct file *file) {}
static inline void eventpoll_release(struct file *file) {}

#endif

#endif /* #ifndef _LINUX_EVENTPOLL_H */
