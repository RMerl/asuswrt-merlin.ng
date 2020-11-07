#ifndef _SELINUX_DSO_H
#define _SELINUX_DSO_H	1

#ifdef SHARED
# define hidden __attribute__ ((visibility ("hidden")))
# define hidden_proto(fct) __hidden_proto (fct, fct##_internal)
# define __hidden_proto(fct, internal)	\
     extern __typeof (fct) internal;	\
     extern __typeof (fct) fct __asm (#internal) hidden;
# if defined(__alpha__) || defined(__mips__)
#  define hidden_def(fct) \
     asm (".globl " #fct "\n" #fct " = " #fct "_internal");
# else
#  define hidden_def(fct) \
     asm (".globl " #fct "\n.set " #fct ", " #fct "_internal");
#endif
#else
# define hidden
# define hidden_proto(fct)
# define hidden_def(fct)
#endif

#endif
