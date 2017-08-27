/*****************************************************************************
 * wpa service
 *****************************************************************************
*/

#if !defined(__WPA_SVC_H__)
#define __WPA_SVC_H__


struct cfg_ctx;
struct cfg_ctx_set_cfg;
struct wpa_dat;
struct wpa_svc_dat;


/*
 * supplicant
*/

extern int
wpa_sup_svc_init(struct cfg_ctx *ctx);

extern int
wpa_sup_svc_deinit(struct cfg_ctx *ctx);

/*
 * authenticator
*/

extern int
wpa_auth_svc_init(struct cfg_ctx *ctx);

extern int
wpa_auth_svc_deinit(struct cfg_ctx *ctx);

/*
 * common
*/

extern int
wpa_svc_cfg(struct cfg_ctx *, const struct cfg_ctx_set_cfg *);

extern struct wpa_dat *
wpa_svc_wpa_dat(struct wpa_svc_dat *);


#endif /* !defined(__WPA_SVC_H__) */
