/* For __FreeBSD_kernel_version */
#include <osreldate.h>
#define __FreeBSD_version __FreeBSD_kernel_version

/* For <vm/vm.h> */
#include <sys/_types.h>

#define INP_NEXT_SYMBOL inp_next

#if __FreeBSD_kernel_version >= 3
#define freebsd3
#endif
#if __FreeBSD_kernel_version >= 4
#define freebsd4
#endif
#if __FreeBSD_kernel_version >= 5
#define freebsd5
#endif
#if __FreeBSD_kernel_version >= 6
#define freebsd6
#endif
#if __FreeBSD_kernel_version >= 7
#define freebsd7
#endif
#if __FreeBSD_kernel_version >= 8
#define freebsd8
#endif
#if __FreeBSD_kernel_version >= 9
#define freebsd9
#endif
#if __FreeBSD_kernel_version >= 10
#define freebsd10
#endif
#if __FreeBSD_kernel_version >= 11
#define freebsd11
#endif
#if __FreeBSD_kernel_version >= 12
#define freebsd12
#endif
#if __FreeBSD_kernel_version >= 13
#define freebsd13
#endif
#if __FreeBSD_kernel_version >= 14
#define freebsd14
#endif
