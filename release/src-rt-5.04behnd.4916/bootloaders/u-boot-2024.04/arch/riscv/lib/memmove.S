/* SPDX-License-Identifier: GPL-2.0 */

#include <linux/linkage.h>
#include <asm/asm.h>

ENTRY(__memmove)
WEAK(memmove)
	/*
	 * Here we determine if forward copy is possible. Forward copy is
	 * preferred to backward copy as it is more cache friendly.
	 *
	 * If a0 >= a1, t0 gives their distance, if t0 >= a2 then we can
	 *   copy forward.
	 * If a0 < a1, we can always copy forward. This will make t0 negative,
	 *   so a *unsigned* comparison will always have t0 >= a2.
	 *
	 * For forward copy we just delegate the task to memcpy.
	 */
	sub	t0, a0, a1
	bltu	t0, a2, 1f
	tail	__memcpy
1:

	/*
	 * Register allocation for code below:
	 * a0 - end of uncopied dst
	 * a1 - end of uncopied src
	 * t0 - start of uncopied dst
	 */
	mv	t0, a0
	add	a0, a0, a2
	add	a1, a1, a2

	/*
	 * Use bytewise copy if too small.
	 *
	 * This threshold must be at least 2*SZREG to ensure at least one
	 * wordwise copy is performed. It is chosen to be 16 because it will
	 * save at least 7 iterations of bytewise copy, which pays off the
	 * fixed overhead.
	 */
	li	a3, 16
	bltu	a2, a3, .Lbyte_copy_tail

	/*
	 * Bytewise copy first to align t0 to word boundary.
	 */
	andi	a2, a0, ~(SZREG-1)
	beq	a0, a2, 2f
1:
	addi	a1, a1, -1
	lb	a5, 0(a1)
	addi	a0, a0, -1
	sb	a5, 0(a0)
	bne	a0, a2, 1b
2:

	/*
	 * Now a0 is word-aligned. If a1 is also word aligned, we could perform
	 * aligned word-wise copy. Otherwise we need to perform misaligned
	 * word-wise copy.
	 */
	andi	a3, a1, SZREG-1
	bnez	a3, .Lmisaligned_word_copy

	/* Wordwise copy */
	addi	t0, t0, SZREG-1
	bleu	a0, t0, 2f
1:
	addi	a1, a1, -SZREG
	REG_L	a5, 0(a1)
	addi	a0, a0, -SZREG
	REG_S	a5, 0(a0)
	bgtu	a0, t0, 1b
2:
	addi	t0, t0, -(SZREG-1)

.Lbyte_copy_tail:
	/*
	 * Bytewise copy anything left.
	 */
	beq	a0, t0, 2f
1:
	addi	a1, a1, -1
	lb	a5, 0(a1)
	addi	a0, a0, -1
	sb	a5, 0(a0)
	bne	a0, t0, 1b
2:

	mv	a0, t0
	ret

.Lmisaligned_word_copy:
	/*
	 * Misaligned word-wise copy.
	 * For misaligned copy we still perform word-wise copy, but we need to
	 * use the value fetched from the previous iteration and do some shifts.
	 * This is safe because we wouldn't access more words than necessary.
	 */

	/* Calculate shifts */
	slli	t3, a3, 3
	sub	t4, x0, t3 /* negate is okay as shift will only look at LSBs */

	/* Load the initial value and align a1 */
	andi	a1, a1, ~(SZREG-1)
	REG_L	a5, 0(a1)

	addi	t0, t0, SZREG-1
	/* At least one iteration will be executed here, no check */
1:
	sll	a4, a5, t4
	addi	a1, a1, -SZREG
	REG_L	a5, 0(a1)
	srl	a2, a5, t3
	or	a2, a2, a4
	addi	a0, a0, -SZREG
	REG_S	a2, 0(a0)
	bgtu	a0, t0, 1b

	/* Update pointers to correct value */
	addi	t0, t0, -(SZREG-1)
	add	a1, a1, a3

	j	.Lbyte_copy_tail

END(__memmove)
