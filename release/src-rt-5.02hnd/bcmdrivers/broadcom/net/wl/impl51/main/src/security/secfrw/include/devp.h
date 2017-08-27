/*****************************************************************************
 * Device (private)
 *****************************************************************************
*/

#if !defined(__DEVP_H__)
#define __DEVP_H__


struct cfg_ctx;
struct il2;
struct iwlss;

struct dev {
	int (*init)(struct cfg_ctx *, const void *priv);
	void (*deinit)(struct cfg_ctx *);

	const struct il2 *il2;
	const struct iwlss *iwlss;
};

#define DEV_INITIALIZER_LIST(f1,f2,f3,f4) {(f1),(f2),(f3),(f4)}


#endif /* !defined(__DEVP_H__) */
