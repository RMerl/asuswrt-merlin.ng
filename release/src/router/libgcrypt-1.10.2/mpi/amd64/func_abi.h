#include <config.h>

#include "asm-common-amd64.h"

#ifdef USE_MS_ABI
 /* Store registers and move four first input arguments from MS ABI to
  * SYSV ABI.  */
 #define FUNC_ENTRY() \
	CFI_STARTPROC(); \
	pushq %rsi; \
	CFI_PUSH(%rsi); \
	pushq %rdi; \
	CFI_PUSH(%rdi); \
	movq %rdx, %rsi; \
	movq %rcx, %rdi; \
	movq %r8, %rdx; \
	movq %r9, %rcx;

 /* Restore registers.  */
 #define FUNC_EXIT() \
	popq %rdi; \
	CFI_POP(%rdi); \
	popq %rsi; \
	CFI_POP(%rsi); \
	ret_spec_stop; \
	CFI_ENDPROC();
#else
 #define FUNC_ENTRY() \
	CFI_STARTPROC();

 #define FUNC_EXIT() \
	ret_spec_stop; \
	CFI_ENDPROC();
#endif
