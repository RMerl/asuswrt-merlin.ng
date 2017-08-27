#ifndef __HLIST_H__
#define __HLIST_H__

#include <sys/types.h>

typedef void (*hlistDestroyFunc)(void *data);
typedef struct __hlist__t_ {
	size_t size;
	struct __h_listnode__t_   *save;
	struct __h_listnode__t_   *list;
	hlistDestroyFunc destroy;
} *HLIST_T;

HLIST_T hlInit(hlistDestroyFunc destroy);

void hlInsertTop(HLIST_T ref, void *data);
void hlInsertEnd(HLIST_T ref, void *data);
void hlCopy(HLIST_T src, HLIST_T dest);
void hlReset(HLIST_T ref);
void hlDestroy(HLIST_T ref);
void hlistPop(HLIST_T ref);
void *hlGetNext(HLIST_T ref);
void *hlGetTop(HLIST_T ref);

#endif

