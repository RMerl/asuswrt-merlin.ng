#ifndef _UAPI_ASM_X86_SIGCONTEXT_H
#define _UAPI_ASM_X86_SIGCONTEXT_H

#include <linux/compiler.h>
#include <linux/types.h>

#define FP_XSTATE_MAGIC1	0x46505853U
#define FP_XSTATE_MAGIC2	0x46505845U
#define FP_XSTATE_MAGIC2_SIZE	sizeof(FP_XSTATE_MAGIC2)

/*
 * bytes 464..511 in the current 512byte layout of fxsave/fxrstor frame
 * are reserved for SW usage. On cpu's supporting xsave/xrstor, these bytes
 * are used to extended the fpstate pointer in the sigcontext, which now
 * includes the extended state information along with fpstate information.
 *
 * Presence of FP_XSTATE_MAGIC1 at the beginning of this SW reserved
 * area and FP_XSTATE_MAGIC2 at the end of memory layout
 * (extended_size - FP_XSTATE_MAGIC2_SIZE) indicates the presence of the
 * extended state information in the memory layout pointed by the fpstate
 * pointer in sigcontext.
 */
struct _fpx_sw_bytes {
	__u32 magic1;		/* FP_XSTATE_MAGIC1 */
	__u32 extended_size;	/* total size of the layout referred by
				 * fpstate pointer in the sigcontext.
				 */
	__u64 xstate_bv;
				/* feature bit mask (including fp/sse/extended
				 * state) that is present in the memory
				 * layout.
				 */
	__u32 xstate_size;	/* actual xsave state size, based on the
				 * features saved in the layout.
				 * 'extended_size' will be greater than
				 * 'xstate_size'.
				 */
	__u32 padding[7];	/*  for future use. */
};

#ifdef __i386__
/*
 * As documented in the iBCS2 standard..
 *
 * The first part of "struct _fpstate" is just the normal i387
 * hardware setup, the extra "status" word is used to save the
 * coprocessor status word before entering the handler.
 *
 * Pentium III FXSR, SSE support
 *	Gareth Hughes <gareth@valinux.com>, May 2000
 *
 * The FPU state data structure has had to grow to accommodate the
 * extended FPU state required by the Streaming SIMD Extensions.
 * There is no documented standard to accomplish this at the moment.
 */
struct _fpreg {
	unsigned short significand[4];
	unsigned short exponent;
};

struct _fpxreg {
	unsigned short significand[4];
	unsigned short exponent;
	unsigned short padding[3];
};

struct _xmmreg {
	unsigned long element[4];
};

struct _fpstate {
	/* Regular FPU environment */
	unsigned long	cw;
	unsigned long	sw;
	unsigned long	tag;
	unsigned long	ipoff;
	unsigned long	cssel;
	unsigned long	dataoff;
	unsigned long	datasel;
	struct _fpreg	_st[8];
	unsigned short	status;
	unsigned short	magic;		/* 0xffff = regular FPU data only */

	/* FXSR FPU environment */
	unsigned long	_fxsr_env[6];	/* FXSR FPU env is ignored */
	unsigned long	mxcsr;
	unsigned long	reserved;
	struct _fpxreg	_fxsr_st[8];	/* FXSR FPU reg data is ignored */
	struct _xmmreg	_xmm[8];
	unsigned long	padding1[44];

	union {
		unsigned long	padding2[12];
		struct _fpx_sw_bytes sw_reserved; /* represents the extended
						   * state info */
	};
};

#define X86_FXSR_MAGIC		0x0000

#ifndef __KERNEL__
/*
 * User-space might still rely on the old definition:
 */
struct sigcontext {
	unsigned short gs, __gsh;
	unsigned short fs, __fsh;
	unsigned short es, __esh;
	unsigned short ds, __dsh;
	unsigned long edi;
	unsigned long esi;
	unsigned long ebp;
	unsigned long esp;
	unsigned long ebx;
	unsigned long edx;
	unsigned long ecx;
	unsigned long eax;
	unsigned long trapno;
	unsigned long err;
	unsigned long eip;
	unsigned short cs, __csh;
	unsigned long eflags;
	unsigned long esp_at_signal;
	unsigned short ss, __ssh;
	struct _fpstate __user *fpstate;
	unsigned long oldmask;
	unsigned long cr2;
};
#endif /* !__KERNEL__ */

#else /* __i386__ */

/* FXSAVE frame */
/* Note: reserved1/2 may someday contain valuable data. Always save/restore
   them when you change signal frames. */
struct _fpstate {
	__u16	cwd;
	__u16	swd;
	__u16	twd;		/* Note this is not the same as the
				   32bit/x87/FSAVE twd */
	__u16	fop;
	__u64	rip;
	__u64	rdp;
	__u32	mxcsr;
	__u32	mxcsr_mask;
	__u32	st_space[32];	/* 8*16 bytes for each FP-reg */
	__u32	xmm_space[64];	/* 16*16 bytes for each XMM-reg  */
	__u32	reserved2[12];
	union {
		__u32	reserved3[12];
		struct _fpx_sw_bytes sw_reserved; /* represents the extended
						   * state information */
	};
};

#ifndef __KERNEL__
/*
 * User-space might still rely on the old definition:
 */
struct sigcontext {
	__u64 r8;
	__u64 r9;
	__u64 r10;
	__u64 r11;
	__u64 r12;
	__u64 r13;
	__u64 r14;
	__u64 r15;
	__u64 rdi;
	__u64 rsi;
	__u64 rbp;
	__u64 rbx;
	__u64 rdx;
	__u64 rax;
	__u64 rcx;
	__u64 rsp;
	__u64 rip;
	__u64 eflags;		/* RFLAGS */
	__u16 cs;
	__u16 gs;
	__u16 fs;
	__u16 __pad0;
	__u64 err;
	__u64 trapno;
	__u64 oldmask;
	__u64 cr2;
	struct _fpstate __user *fpstate;	/* zero when no FPU context */
#ifdef __ILP32__
	__u32 __fpstate_pad;
#endif
	__u64 reserved1[8];
};
#endif /* !__KERNEL__ */

#endif /* !__i386__ */

struct _xsave_hdr {
	__u64 xstate_bv;
	__u64 reserved1[2];
	__u64 reserved2[5];
};

struct _ymmh_state {
	/* 16 * 16 bytes for each YMMH-reg */
	__u32 ymmh_space[64];
};

/*
 * Extended state pointed by the fpstate pointer in the sigcontext.
 * In addition to the fpstate, information encoded in the xstate_hdr
 * indicates the presence of other extended state information
 * supported by the processor and OS.
 */
struct _xstate {
	struct _fpstate fpstate;
	struct _xsave_hdr xstate_hdr;
	struct _ymmh_state ymmh;
	/* new processor state extensions go here */
};

#endif /* _UAPI_ASM_X86_SIGCONTEXT_H */
