#ifndef _EVENT_H_
#define _EVENT_H_

struct evfd *create_evfd(void);

void destroy_evfd(struct evfd *e);

int get_read_evfd(struct evfd *evfd);

int write_evfd(struct evfd *evfd);

int read_evfd(struct evfd *evfd);

#endif
