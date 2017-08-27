/*****************************************************************************
 * Wireless User Tools
 *
 * Layer-2 helpers
 *****************************************************************************
*/

#if !defined(__L2_H__)
#define __L2_H__


/* forwards */

struct l2;

/* methods */

extern int
l2_tx(void *, const void *, const size_t);

extern const struct l2 *
eapol_type();

extern const struct l2 *
common_eapol_type(int index);


#endif /* !defined(__L2_H__) */
