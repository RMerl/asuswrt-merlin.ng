/*
 * $Copyright Open Broadcom Corporation$
 *
 * $Id: dpsta.h 300516 2011-12-04 17:39:44Z rnuti $
 */

#ifndef _DPSTA_H_
#define _DPSTA_H_

typedef enum {
	DPSTA_MODE_PSTA = 1,
	DPSTA_MODE_DWDS = 2,
	DPSTA_MODE_WET = 3
} dpsta_mode_e;

typedef struct psta_if psta_if_t;

/* Proxy STA instance data and exported functions */
typedef struct psta_if_api {
	void	*psta;
	void	*bsscfg;
	bool	(*is_ds_sta)(void *psta, struct ether_addr *ea);
	void	*(*psta_find)(void *psta, uint8 *ea);
	bool	(*bss_auth)(void *bsscfg);
	dpsta_mode_e	mode;
} psta_if_api_t;

extern psta_if_t *dpsta_register(uint32 unit, psta_if_api_t *inst);
extern int32 dpsta_recv(void *p);

#endif /* _DPSTA_H_ */
