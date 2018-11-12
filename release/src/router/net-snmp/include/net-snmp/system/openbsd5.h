/* openbsd5 is a superset of all since openbsd3 */
#include "openbsd4.h"
#define openbsd4 openbsd4

#undef HAVE_NET_IF_VAR_H

#if OpenBSD >= 201605
#undef INP_FIRST_SYMBOL
#define INP_FIRST_SYMBOL inpt_queue.tqh_first
#undef INP_NEXT_SYMBOL
#define INP_NEXT_SYMBOL inp_queue.tqe_next
#undef INP_PREV_SYMBOL
#define INP_PREV_SYMBOL inp_queue.tqe_prev
#endif

