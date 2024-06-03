#include <common.h>
#include <exports.h>
#include <linux/compiler.h>

#define FO(x) offsetof(struct jt_funcs, x)

#if defined(CONFIG_X86)
/*
 * x86 does not have a dedicated register to store the pointer to
 * the global_data. Thus the jump table address is stored in a
 * global variable, but such approach does not allow for execution
 * from flash memory. The global_data address is passed as argv[-1]
 * to the application program.
 */
static struct jt_funcs *jt;
gd_t *global_data;

#define EXPORT_FUNC(f, a, x, ...) \
	asm volatile (			\
"	.globl " #x "\n"		\
#x ":\n"				\
"	movl	%0, %%eax\n"		\
"	movl	jt, %%ecx\n"		\
"	jmp	*(%%ecx, %%eax)\n"	\
	: : "i"(FO(x)) : "eax", "ecx");
#elif defined(CONFIG_PPC)
/*
 * r2 holds the pointer to the global_data, r11 is a call-clobbered
 * register
 */
#define EXPORT_FUNC(f, a, x, ...) \
	asm volatile (			\
"	.globl " #x "\n"		\
#x ":\n"				\
"	lwz	%%r11, %0(%%r2)\n"	\
"	lwz	%%r11, %1(%%r11)\n"	\
"	mtctr	%%r11\n"		\
"	bctr\n"				\
	: : "i"(offsetof(gd_t, jt)), "i"(FO(x)) : "r11");
#elif defined(CONFIG_ARM)
#ifdef CONFIG_ARM64
/*
 * x18 holds the pointer to the global_data, x9 is a call-clobbered
 * register
 */
#define EXPORT_FUNC(f, a, x, ...) \
	asm volatile (			\
"	.globl " #x "\n"		\
#x ":\n"				\
"	ldr	x9, [x18, %0]\n"		\
"	ldr	x9, [x9, %1]\n"		\
"	br	x9\n"		\
	: : "i"(offsetof(gd_t, jt)), "i"(FO(x)) : "x9");
#else
/*
 * r9 holds the pointer to the global_data, ip is a call-clobbered
 * register
 */
#define EXPORT_FUNC(f, a, x, ...) \
	asm volatile (			\
"	.globl " #x "\n"		\
#x ":\n"				\
"	ldr	ip, [r9, %0]\n"		\
"	ldr	pc, [ip, %1]\n"		\
	: : "i"(offsetof(gd_t, jt)), "i"(FO(x)) : "ip");
#endif
#elif defined(CONFIG_MIPS)
#ifdef CONFIG_CPU_MIPS64
/*
 * k0 ($26) holds the pointer to the global_data; t9 ($25) is a call-
 * clobbered register that is also used to set gp ($26). Note that the
 * jr instruction also executes the instruction immediately following
 * it; however, GCC/mips generates an additional `nop' after each asm
 * statement
 */
#define EXPORT_FUNC(f, a, x, ...) \
	asm volatile (			\
"	.globl " #x "\n"		\
#x ":\n"				\
"	ld	$25, %0($26)\n"		\
"	ld	$25, %1($25)\n"		\
"	jr	$25\n"			\
        : : "i"(offsetof(gd_t, jt)), "i"(FO(x)) : "t9");
#else
/*
 * k0 ($26) holds the pointer to the global_data; t9 ($25) is a call-
 * clobbered register that is also used to set gp ($26). Note that the
 * jr instruction also executes the instruction immediately following
 * it; however, GCC/mips generates an additional `nop' after each asm
 * statement
 */
#define EXPORT_FUNC(f, a, x, ...) \
	asm volatile (			\
"	.globl " #x "\n"		\
#x ":\n"				\
"	lw	$25, %0($26)\n"		\
"	lw	$25, %1($25)\n"		\
"	jr	$25\n"			\
	: : "i"(offsetof(gd_t, jt)), "i"(FO(x)) : "t9");
#endif
#elif defined(CONFIG_NIOS2)
/*
 * gp holds the pointer to the global_data, r8 is call-clobbered
 */
#define EXPORT_FUNC(f, a, x, ...) \
	asm volatile (			\
"	.globl " #x "\n"		\
#x ":\n"				\
"	movhi	r8, %%hi(%0)\n"		\
"	ori	r8, r0, %%lo(%0)\n"	\
"	add	r8, r8, gp\n"		\
"	ldw	r8, 0(r8)\n"		\
"	ldw	r8, %1(r8)\n"		\
"	jmp	r8\n"			\
	: : "i"(offsetof(gd_t, jt)), "i"(FO(x)) : "gp");
#elif defined(CONFIG_M68K)
/*
 * d7 holds the pointer to the global_data, a0 is a call-clobbered
 * register
 */
#define EXPORT_FUNC(f, a, x, ...) \
	asm volatile (			\
"	.globl " #x "\n"		\
#x ":\n"				\
"	move.l	%%d7, %%a0\n"		\
"	adda.l	%0, %%a0\n"		\
"	move.l	(%%a0), %%a0\n"		\
"	adda.l	%1, %%a0\n"		\
"	move.l	(%%a0), %%a0\n"		\
"	jmp	(%%a0)\n"			\
	: : "i"(offsetof(gd_t, jt)), "i"(FO(x)) : "a0");
#elif defined(CONFIG_MICROBLAZE)
/*
 * r31 holds the pointer to the global_data. r5 is a call-clobbered.
 */
#define EXPORT_FUNC(f, a, x, ...)				\
	asm volatile (				\
"	.globl " #x "\n"			\
#x ":\n"					\
"	lwi	r5, r31, %0\n"			\
"	lwi	r5, r5, %1\n"			\
"	bra	r5\n"				\
	: : "i"(offsetof(gd_t, jt)), "i"(FO(x)) : "r5");
#elif defined(CONFIG_SH)
/*
 * r13 holds the pointer to the global_data. r1 is a call clobbered.
 */
#define EXPORT_FUNC(f, a, x, ...)					\
	asm volatile (					\
		"	.align	2\n"			\
		"	.globl " #x "\n"		\
		#x ":\n"				\
		"	mov	r13, r1\n"		\
		"	add	%0, r1\n"		\
		"	mov.l @r1, r2\n"	\
		"	add	%1, r2\n"		\
		"	mov.l @r2, r1\n"	\
		"	jmp	@r1\n"			\
		"	nop\n"				\
		"	nop\n"				\
		: : "i"(offsetof(gd_t, jt)), "i"(FO(x)) : "r1", "r2");
#elif defined(CONFIG_NDS32)
/*
 * r16 holds the pointer to the global_data. gp is call clobbered.
 * not support reduced register (16 GPR).
 */
#define EXPORT_FUNC(f, a, x, ...) \
	asm volatile (			\
"	.globl " #x "\n"		\
#x ":\n"				\
"	lwi	$r16, [$gp + (%0)]\n"	\
"	lwi	$r16, [$r16 + (%1)]\n"	\
"	jr	$r16\n"			\
	: : "i"(offsetof(gd_t, jt)), "i"(FO(x)) : "$r16");
#elif defined(CONFIG_RISCV)
/*
 * gp holds the pointer to the global_data. t0 is call clobbered.
 */
#ifdef CONFIG_ARCH_RV64I
#define EXPORT_FUNC(f, a, x, ...)	\
	asm volatile (			\
"	.globl " #x "\n"		\
#x ":\n"				\
"	ld	t0, %0(gp)\n"		\
"	ld	t0, %1(t0)\n"		\
"	jr	t0\n"			\
	: : "i"(offsetof(gd_t, jt)), "i"(FO(x)) : "t0");
#else
#define EXPORT_FUNC(f, a, x, ...)	\
	asm volatile (			\
"	.globl " #x "\n"		\
#x ":\n"				\
"	lw	t0, %0(gp)\n"		\
"	lw	t0, %1(t0)\n"		\
"	jr	t0\n"			\
	: : "i"(offsetof(gd_t, jt)), "i"(FO(x)) : "t0");
#endif
#elif defined(CONFIG_ARC)
/*
 * r25 holds the pointer to the global_data. r10 is call clobbered.
  */
#define EXPORT_FUNC(f, a, x, ...) \
	asm volatile( \
"	.align 4\n" \
"	.globl " #x "\n" \
#x ":\n" \
"	ld	r10, [r25, %0]\n" \
"	ld	r10, [r10, %1]\n" \
"	j	[r10]\n" \
	: : "i"(offsetof(gd_t, jt)), "i"(FO(x)) : "r10");
#elif defined(CONFIG_XTENSA)
/*
 * Global data ptr is in global_data, jump table ptr is in jt.
 * Windowed ABI: Jump just past 'entry' in target and adjust stack frame
 * (extract stack frame size from target 'entry' instruction).
 */

static void **jt;

#if defined(__XTENSA_CALL0_ABI__)
#define EXPORT_FUNC(f, a, x, ...)	\
	asm volatile (			\
"	.extern jt\n"			\
"	.globl " #x "\n"		\
"	.align 4\n"			\
#x ":\n"				\
"	l32i	a8, %0, 0\n"		\
"	l32i	a8, a8, %1\n"		\
"	jx	a8\n"			\
	: : "r"(jt), "i" (FO(x)) : "a8");
#elif defined(__XTENSA_WINDOWED_ABI__)
#if XCHAL_HAVE_BE
# define SFT "8"
#else
# define SFT "12"
#endif
#define EXPORT_FUNC(f, a, x, ...)	\
	asm volatile (			\
"	.extern jt\n"			\
"	.globl " #x "\n"		\
"	.align 4\n"			\
#x ":\n"				\
"	entry	sp, 16\n"		\
"	l32i	a8, %0, 0\n"		\
"	l32i	a8, a8, %1\n"		\
"	l32i	a9, a8, 0\n"		\
"	extui	a9, a9, " SFT ", 12\n"	\
"	subx8	a9, a9, sp\n"		\
"	movi	a10, 16\n"		\
"	sub	a9, a10, a9\n"		\
"	movsp	sp, a9\n"		\
"	addi	a8, a8, 3\n"		\
"	jx	a8\n"			\
	: : "r"(jt), "i" (FO(x)) : "a8", "a9", "a10");
#else
#error Unsupported Xtensa ABI
#endif
#else
/*"	addi	$sp, $sp, -24\n"	\
"	br	$r16\n"			\*/

#error stubs definition missing for this architecture
#endif

/* This function is necessary to prevent the compiler from
 * generating prologue/epilogue, preparing stack frame etc.
 * The stub functions are special, they do not use the stack
 * frame passed to them, but pass it intact to the actual
 * implementation. On the other hand, asm() statements with
 * arguments can be used only inside the functions (gcc limitation)
 */
#if GCC_VERSION < 30400
static
#endif /* GCC_VERSION */
void __attribute__((unused)) dummy(void)
{
#include <_exports.h>
}

#include <asm/sections.h>

void app_startup(char * const *argv)
{
	char *cp = __bss_start;

	/* Zero out BSS */
	while (cp < _end)
		*cp++ = 0;

#if defined(CONFIG_X86)
	/* x86 does not have a dedicated register for passing global_data */
	global_data = (gd_t *)argv[-1];
	jt = global_data->jt;
#endif
}

#undef EXPORT_FUNC
