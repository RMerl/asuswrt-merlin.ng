	.arch armv8-a+crc
	.file	"armv8_disable_mmu.c"
	.text
	.comm	sctlr,4,4
	.comm	el,4,4
	.align	2
	.global	dcache_disable
	.type	dcache_disable, %function
dcache_disable:
.LFB0:
	.cfi_startproc
#APP
// 11 "armv8_disable_mmu.c" 1
	mrs x1, CurrentEL
// 0 "" 2
#NO_APP
	adrp	x0, el
	add	x0, x0, :lo12:el
	str	w1, [x0]
	adrp	x0, el
	add	x0, x0, :lo12:el
	ldr	w0, [x0]
	lsr	w1, w0, 2
	adrp	x0, el
	add	x0, x0, :lo12:el
	str	w1, [x0]
	adrp	x0, el
	add	x0, x0, :lo12:el
	ldr	w0, [x0]
	cmp	w0, 1
	bne	.L2
#APP
// 15 "armv8_disable_mmu.c" 1
	mrs x1, sctlr_el1
// 0 "" 2
#NO_APP
	adrp	x0, sctlr
	add	x0, x0, :lo12:sctlr
	str	w1, [x0]
	b	.L3
.L2:
	adrp	x0, el
	add	x0, x0, :lo12:el
	ldr	w0, [x0]
	cmp	w0, 2
	bne	.L4
#APP
// 17 "armv8_disable_mmu.c" 1
	mrs x1, sctlr_el2
// 0 "" 2
#NO_APP
	adrp	x0, sctlr
	add	x0, x0, :lo12:sctlr
	str	w1, [x0]
	b	.L3
.L4:
#APP
// 19 "armv8_disable_mmu.c" 1
	mrs x1, sctlr_el3
// 0 "" 2
#NO_APP
	adrp	x0, sctlr
	add	x0, x0, :lo12:sctlr
	str	w1, [x0]
.L3:
	adrp	x0, sctlr
	add	x0, x0, :lo12:sctlr
	ldr	w0, [x0]
	and	w0, w0, 4
	cmp	w0, 0
	beq	.L10
	adrp	x0, sctlr
	add	x0, x0, :lo12:sctlr
	ldr	w1, [x0]
	mov	w0, -6
	and	w1, w1, w0
	adrp	x0, sctlr
	add	x0, x0, :lo12:sctlr
	str	w1, [x0]
	adrp	x0, sctlr
	add	x0, x0, :lo12:sctlr
	ldr	w0, [x0]
	and	w1, w0, -4097
	adrp	x0, sctlr
	add	x0, x0, :lo12:sctlr
	str	w1, [x0]
	adrp	x0, el
	add	x0, x0, :lo12:el
	ldr	w0, [x0]
	cmp	w0, 1
	bne	.L7
	adrp	x0, sctlr
	add	x0, x0, :lo12:sctlr
	ldr	w0, [x0]
#APP
// 30 "armv8_disable_mmu.c" 1
	msr sctlr_el1, x0
// 0 "" 2
#NO_APP
	b	.L8
.L7:
	adrp	x0, el
	add	x0, x0, :lo12:el
	ldr	w0, [x0]
	cmp	w0, 2
	bne	.L9
	adrp	x0, sctlr
	add	x0, x0, :lo12:sctlr
	ldr	w0, [x0]
#APP
// 32 "armv8_disable_mmu.c" 1
	msr sctlr_el2, x0
// 0 "" 2
#NO_APP
	b	.L8
.L9:
	adrp	x0, sctlr
	add	x0, x0, :lo12:sctlr
	ldr	w0, [x0]
#APP
// 34 "armv8_disable_mmu.c" 1
	msr sctlr_el3, x0
// 0 "" 2
#NO_APP
.L8:
#APP
// 36 "armv8_disable_mmu.c" 1
	isb
// 0 "" 2
#NO_APP
	b	.L1
.L10:
	nop
.L1:
	ret
	.cfi_endproc
.LFE0:
	.size	dcache_disable, .-dcache_disable
	.ident	"GCC: (Buildroot 2019.11.1) 9.2.0"
	.section	.note.GNU-stack,"",@progbits
