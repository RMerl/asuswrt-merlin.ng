/*
 * PowerPC atomic operations
 */

#ifndef _ASM_PPC_ATOMIC_H_
#define _ASM_PPC_ATOMIC_H_

#ifdef CONFIG_SMP
typedef struct { volatile int counter; } atomic_t;
#else
typedef struct { int counter; } atomic_t;
#endif

#define ATOMIC_INIT(i)	{ (i) }

#define atomic_read(v)		((v)->counter)
#define atomic_set(v,i)		(((v)->counter) = (i))

extern void atomic_clear_mask(unsigned long mask, unsigned long *addr);
extern void atomic_set_mask(unsigned long mask, unsigned long *addr);

static __inline__ int atomic_add_return(int a, atomic_t *v)
{
	int t;

	__asm__ __volatile__("\n\
1:	lwarx	%0,0,%3\n\
	add	%0,%2,%0\n\
	stwcx.	%0,0,%3\n\
	bne-	1b"
	: "=&r" (t), "=m" (*v)
	: "r" (a), "r" (v), "m" (*v)
	: "cc");

	return t;
}

static __inline__ int atomic_sub_return(int a, atomic_t *v)
{
	int t;

	__asm__ __volatile__("\n\
1:	lwarx	%0,0,%3\n\
	subf	%0,%2,%0\n\
	stwcx.	%0,0,%3\n\
	bne-	1b"
	: "=&r" (t), "=m" (*v)
	: "r" (a), "r" (v), "m" (*v)
	: "cc");

	return t;
}

static __inline__ int atomic_inc_return(atomic_t *v)
{
	int t;

	__asm__ __volatile__("\n\
1:	lwarx	%0,0,%2\n\
	addic	%0,%0,1\n\
	stwcx.	%0,0,%2\n\
	bne-	1b"
	: "=&r" (t), "=m" (*v)
	: "r" (v), "m" (*v)
	: "cc");

	return t;
}

static __inline__ int atomic_dec_return(atomic_t *v)
{
	int t;

	__asm__ __volatile__("\n\
1:	lwarx	%0,0,%2\n\
	addic	%0,%0,-1\n\
	stwcx.	%0,0,%2\n\
	bne	1b"
	: "=&r" (t), "=m" (*v)
	: "r" (v), "m" (*v)
	: "cc");

	return t;
}

#define atomic_add(a, v)		((void) atomic_add_return((a), (v)))
#define atomic_sub(a, v)		((void) atomic_sub_return((a), (v)))
#define atomic_sub_and_test(a, v)	(atomic_sub_return((a), (v)) == 0)
#define atomic_inc(v)			((void) atomic_inc_return((v)))
#define atomic_dec(v)			((void) atomic_dec_return((v)))
#define atomic_dec_and_test(v)		(atomic_dec_return((v)) == 0)

#endif /* _ASM_PPC_ATOMIC_H_ */
