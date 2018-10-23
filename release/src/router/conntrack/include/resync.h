#ifndef _RESYNC_H_
#define _RESYNC_H_

void resync_req(void);
void resync_send(int (*do_cache_to_tx)(void *data1, void *data2));
void resync_at_startup(void);

#endif /*_RESYNC_H_ */
