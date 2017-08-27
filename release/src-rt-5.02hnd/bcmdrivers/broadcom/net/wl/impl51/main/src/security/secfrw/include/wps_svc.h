/*****************************************************************************
 * wps service
 *****************************************************************************
*/

#if !defined(__wps_svc_h__)
#define __wps_svc_h__


struct cfg_ctx;
struct cfg_ctx_set_cfg;
struct wps_dat;
struct wps_svc_dat;


extern int
wps_sup_svc_init(struct cfg_ctx *ctx);

extern int
wps_sup_svc_deinit(struct cfg_ctx *ctx);

extern int
wps_sup_svc_cfg(struct cfg_ctx *, const struct cfg_ctx_set_cfg *);

extern int
wps_auth_svc_init(struct cfg_ctx *ctx);

extern int
wps_auth_svc_deinit(struct cfg_ctx *ctx);

extern int
wps_auth_svc_cfg(struct cfg_ctx *, const struct cfg_ctx_set_cfg *);

extern struct wps_dat *
wps_svc_wps_dat(struct wps_svc_dat *);


#endif /* !defined(__wps_svc_h__) */
