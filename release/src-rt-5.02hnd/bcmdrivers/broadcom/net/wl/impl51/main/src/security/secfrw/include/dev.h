/*****************************************************************************
 * Device
 *****************************************************************************
*/

#if !defined(__DEV_H__)
#define __DEV_H__

typedef struct _dev_info{
	char ifname[MAX_IF_NAME_SIZE+1];
	int bsscfg_index;
	int service;
}dev_info_t;


/* forwards */

struct cfg_ctx;
struct dev;

/* methods */

extern int
dev_init(struct cfg_ctx *, const void *priv);

extern void
dev_deinit(struct cfg_ctx *);

extern int
dev_cmp(const struct cfg_ctx *, const struct cfg_ctx *);

extern const struct dev *
dev_wlan(void);

extern int
dev_ifname(const struct cfg_ctx *ctx1, char *namebuf, int buflen);

extern int
dev_match(const struct cfg_ctx *ctx1, char *ifname, int bsscfg_index);

extern const struct dev *
dev_btamp(void);


#endif /* !defined(__DEV_H__) */
