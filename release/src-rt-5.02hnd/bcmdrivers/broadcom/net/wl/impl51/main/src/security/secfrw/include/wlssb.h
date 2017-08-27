/*****************************************************************************
 * Wireless User Tools
 *
 * Wireless Bind Method Declarations
 *****************************************************************************
*/

#if !defined(__IWLSSB_H__)
#define __IWLSSB_H__


/* methods */

extern void *
wlss_bind(void *, void (*interest_vector)(void *ctx, void *priv), \
		  int (*)(void *arg, void *event, int),void *arg);

extern int
wlss_unbind(void *, void *ref);


#endif /* !defined(__IWLSSB_H__) */
