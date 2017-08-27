/*****************************************************************************
 * Per-port Cfg
 *****************************************************************************
*/

#if !defined(__PP_CFG_H__)
#define __PP_CFG_H__


/* return the per-port data for given context */
extern struct pp_dat *
cfg_ctx_pp_dat(struct cfg_ctx *ctx);

/* return service data for given context */
extern void *
cfg_ctx_svc_dat(struct cfg_ctx *ctx);

/* initialize given context to zero */
extern void
cfg_ctx_zero(struct cfg_ctx *ctx);

/* return the sizeof the given context */
extern size_t
cfg_ctx_sizeof(void);


#endif /* !defined(__PP_CFG_H__) */
