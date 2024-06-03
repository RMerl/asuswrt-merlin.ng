#ifndef __KGDB_H__
#define __KGDB_H__

#include <asm/ptrace.h>

#define KGDBERR_BADPARAMS	1
#define KGDBERR_NOTHEXDIG	2
#define KGDBERR_MEMFAULT	3
#define KGDBERR_NOSPACE		4
#define KGDBERR_ALIGNFAULT	5

#define KGDBDATA_MAXREGS	8
#define KGDBDATA_MAXPRIV	8

#define KGDBEXIT_TYPEMASK	0xff

#define KGDBEXIT_KILL		0
#define KGDBEXIT_CONTINUE	1
#define KGDBEXIT_SINGLE		2

#define KGDBEXIT_WITHADDR	0x100

typedef
	struct {
		int num;
		unsigned long val;
	}
kgdb_reg;

typedef
	struct {
		int sigval;
		int extype;
		unsigned long exaddr;
		int nregs;
		kgdb_reg regs[KGDBDATA_MAXREGS];
		unsigned long private[KGDBDATA_MAXPRIV];
	}
kgdb_data;

/* these functions are provided by the generic kgdb support */
extern void kgdb_init(void);
extern void kgdb_error(int);
extern int kgdb_output_string(const char *, unsigned int);
extern void breakpoint(void);

/* these functions are provided by the platform specific kgdb support */
extern void kgdb_flush_cache_range(void *, void *);
extern void kgdb_flush_cache_all(void);
extern int kgdb_setjmp(long *);
extern void kgdb_longjmp(long *, int);
extern void kgdb_enter(struct pt_regs *, kgdb_data *);
extern void kgdb_exit(struct pt_regs *, kgdb_data *);
extern int kgdb_getregs(struct pt_regs *, char *, int);
extern void kgdb_putreg(struct pt_regs *, int, char *, int);
extern void kgdb_putregs(struct pt_regs *, char *, int);
extern int kgdb_trap(struct pt_regs *);
extern void kgdb_breakpoint(int argc, char * const argv[]);

/* these functions are provided by the platform serial driver */
extern void kgdb_serial_init(void);
extern int getDebugChar(void);
extern void putDebugChar(int);
extern void putDebugStr(const char *);
extern void kgdb_interruptible(int);

/* this is referenced in the trap handler for the platform */
extern int (*debugger_exception_handler)(struct pt_regs *);

#endif /* __KGDB_H__ */
