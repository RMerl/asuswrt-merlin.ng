#ifndef _IMQ_H
#define _IMQ_H

/* IFMASK (16 device indexes, 0 to 15) and flag(s) fit in 5 bits */
#define IMQ_F_BITS	5

#define IMQ_F_IFMASK	0x0f
#define IMQ_F_ENQUEUE	0x10

#define IMQ_MAX_DEVS	(IMQ_F_IFMASK + 1)

#endif /* _IMQ_H */
