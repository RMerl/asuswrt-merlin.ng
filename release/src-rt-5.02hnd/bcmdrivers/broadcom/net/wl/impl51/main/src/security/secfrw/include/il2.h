/*****************************************************************************
 * Wireless User Tools
 *
 * Layer-2 Method Declarations
 *****************************************************************************
*/

#if !defined(__IL2_H__)
#define __IL2_H__


/* forwards */

struct l2;

/* methods */

struct il2 {
	void *(*bind)(void *, const struct l2 *, \
				  int (*)(void *arg, void *, int), void *arg);

	int (*unbind)(void *, void *ref, void *svc_ctx);

	int (*tx)(void *, const void *ref, const size_t);

}; /* struct il2 */

#define IL2_INITIALIZER_LIST(f1,f2,f3) {f1,f2,f3}


#endif /* !defined(__IL2_H__) */
