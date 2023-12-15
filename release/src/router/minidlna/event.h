#include "config.h"

#include <sys/types.h>
#ifdef HAVE_KQUEUE
#include <sys/event.h>
#endif

struct event;

typedef enum {
#ifdef HAVE_KQUEUE
	EVENT_READ =	EVFILT_READ,
	EVENT_WRITE =	EVFILT_WRITE,
	EVENT_VNODE =	EVFILT_VNODE,
#else
	EVENT_READ,
	EVENT_WRITE,
#endif
} event_t;

#define	EV_FLAG_CLOSING	0x00000001

typedef	void	event_process_t(struct event *);
#ifdef HAVE_KQUEUE
typedef	void	event_vnode_process_t(struct event *, u_int);
#endif

struct event {
	int		 fd;
	int		 index;
	event_t		 rdwr;
	union {
		event_process_t		*process;
#ifdef HAVE_KQUEUE
		event_vnode_process_t	*process_vnode;
#endif
	} up;
	void		*data;
};

typedef	int	event_module_add_t(struct event *);
typedef	int	event_module_del_t(struct event *, int flags);
typedef int	event_module_init_t(void);
typedef void	event_module_fini_t(void);
typedef int	event_module_process_t(struct timeval *);
struct event_module {
	event_module_add_t	*add;
	event_module_del_t	*del;
	event_module_process_t	*process;
	event_module_init_t	*init;
	event_module_fini_t	*fini;
};

extern struct event_module event_module;
