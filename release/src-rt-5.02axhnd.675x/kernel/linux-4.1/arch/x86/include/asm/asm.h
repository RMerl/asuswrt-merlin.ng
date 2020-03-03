#ifndef _ASM_X86_ASM_H
#define _ASM_X86_ASM_H

#ifdef __ASSEMBLY__
# define __ASM_FORM(x)	x
# define __ASM_FORM_RAW(x)     x
# define __ASM_FORM_COMMA(x) x,
#else
# define __ASM_FORM(x)	" " #x " "
# define __ASM_FORM_RAW(x)     #x
# define __ASM_FORM_COMMA(x) " " #x ","
#endif

#ifdef CONFIG_X86_32
# define __ASM_SEL(a,b) __ASM_FORM(a)
# define __ASM_SEL_RAW(a,b) __ASM_FORM_RAW(a)
#else
# define __ASM_SEL(a,b) __ASM_FORM(b)
# define __ASM_SEL_RAW(a,b) __ASM_FORM_RAW(b)
#endif

#define __ASM_SIZE(inst, ...)	__ASM_SEL(inst##l##__VA_ARGS__, \
					  inst##q##__VA_ARGS__)
#define __ASM_REG(reg)         __ASM_SEL_RAW(e##reg, r##reg)

#define _ASM_PTR	__ASM_SEL(.long, .quad)
#define _ASM_ALIGN	__ASM_SEL(.balign 4, .balign 8)

#define _ASM_MOV	__ASM_SIZE(mov)
#define _ASM_INC	__ASM_SIZE(inc)
#define _ASM_DEC	__ASM_SIZE(dec)
#define _ASM_ADD	__ASM_SIZE(add)
#define _ASM_SUB	__ASM_SIZE(sub)
#define _ASM_XADD	__ASM_SIZE(xadd)

#define _ASM_AX		__ASM_REG(ax)
#define _ASM_BX		__ASM_REG(bx)
#define _ASM_CX		__ASM_REG(cx)
#define _ASM_DX		__ASM_REG(dx)
#define _ASM_SP		__ASM_REG(sp)
#define _ASM_BP		__ASM_REG(bp)
#define _ASM_SI		__ASM_REG(si)
#define _ASM_DI		__ASM_REG(di)

/* Exception table entry */
#ifdef __ASSEMBLY__
# define _ASM_EXTABLE_HANDLE(from, to, handler)			\
	.pushsection "__ex_table","a" ;				\
	.balign 4 ;						\
	.long (from) - . ;					\
	.long (to) - . ;					\
	.long (handler) - . ;					\
	.popsection

# define _ASM_EXTABLE(from, to)					\
	_ASM_EXTABLE_HANDLE(from, to, ex_handler_default)

# define _ASM_EXTABLE_FAULT(from, to)				\
	_ASM_EXTABLE_HANDLE(from, to, ex_handler_fault)

# define _ASM_EXTABLE_EX(from, to)				\
	_ASM_EXTABLE_HANDLE(from, to, ex_handler_ext)

# define _ASM_NOKPROBE(entry)					\
	.pushsection "_kprobe_blacklist","aw" ;			\
	_ASM_ALIGN ;						\
	_ASM_PTR (entry);					\
	.popsection
#else
# define _EXPAND_EXTABLE_HANDLE(x) #x
# define _ASM_EXTABLE_HANDLE(from, to, handler)			\
	" .pushsection \"__ex_table\",\"a\"\n"			\
	" .balign 4\n"						\
	" .long (" #from ") - .\n"				\
	" .long (" #to ") - .\n"				\
	" .long (" _EXPAND_EXTABLE_HANDLE(handler) ") - .\n"	\
	" .popsection\n"

# define _ASM_EXTABLE(from, to)					\
	_ASM_EXTABLE_HANDLE(from, to, ex_handler_default)

# define _ASM_EXTABLE_FAULT(from, to)				\
	_ASM_EXTABLE_HANDLE(from, to, ex_handler_fault)

# define _ASM_EXTABLE_EX(from, to)				\
	_ASM_EXTABLE_HANDLE(from, to, ex_handler_ext)

/* For C file, we already have NOKPROBE_SYMBOL macro */
#endif

#ifndef __ASSEMBLY__
/*
 * This output constraint should be used for any inline asm which has a "call"
 * instruction.  Otherwise the asm may be inserted before the frame pointer
 * gets set up by the containing function.  If you forget to do this, objtool
 * may print a "call without frame pointer save/setup" warning.
 */
register unsigned long current_stack_pointer asm(_ASM_SP);
#define ASM_CALL_CONSTRAINT "+r" (current_stack_pointer)
#endif

#endif /* _ASM_X86_ASM_H */
