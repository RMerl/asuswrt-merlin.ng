/*****************************************************************************
 * Cfg (private)
 *****************************************************************************
*/

#if !defined(__CFGP_H__)
#define __CFGP_H__


/* return the per-port data for given context */
extern struct pp_dat *
cfg_ctx_pp_dat(struct cfg_ctx *ctx);

/* return service data for given context */
extern void *
cfg_ctx_svc_dat(struct cfg_ctx *ctx);

/* initialize given context to zero */
extern void
cfg_ctx_zero(struct cfg_ctx *ctx);

/* return the size of the given context */
extern size_t
cfg_ctx_size_of(void);


#endif /* !defined(__CFGP_H__) */
