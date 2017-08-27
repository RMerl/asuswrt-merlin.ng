/*****************************************************************************
 * Wireless User Tools
 *
 * Layer-2 Bind Method Declarations
 *****************************************************************************
*/

#if !defined(__IL2B_H__)
#define __IL2B_H__


/* forwards */

struct l2;

/* methods */

extern void *
l2_bind(void *, const struct l2 *, int (*)(void *arg, void *frame, int), \
		void *arg);
			 
extern int
l2_unbind(void *, void *ref, void *);


#endif /* !defined(__IL2B_H__) */
