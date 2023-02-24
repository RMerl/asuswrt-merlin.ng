#ifndef _IFTABLE_H
#define _IFTABLE_H

int iftable_delete(uint32_t dst, uint32_t mask, uint32_t gw, uint32_t oif);
int iftable_insert(uint32_t dst, uint32_t mask, uint32_t gw, uint32_t oif);

int iftable_init(void);
void iftable_fini(void);

int iftable_dump(FILE *outfd);
int iftable_up(unsigned int index);
#endif
