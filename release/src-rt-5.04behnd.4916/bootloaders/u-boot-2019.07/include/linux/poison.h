#ifndef _LINUX_POISON_H
#define _LINUX_POISON_H

/********** include/linux/list.h **********/
/*
 * used to verify that nobody uses non-initialized list entries.
 */
#define LIST_POISON1  ((void *) 0x0)
#define LIST_POISON2  ((void *) 0x0)

#endif
