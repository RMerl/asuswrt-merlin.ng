/*
 *   sysctl() interface
 *     e.g. BSD/OS, NetBSD, OpenBSD, later Darwin releases
 */
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-features.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include <net-snmp/agent/hardware/cpu.h>

#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

#if defined(__FreeBSD__)
#include <sys/resource.h>
#if !defined(CPUSTATES)
#include <sys/dkstat.h>
#endif
#else
#include <sys/sched.h>
#endif

#include <sys/param.h>
#include <sys/sysctl.h>
#include <sys/vmmeter.h>
#ifdef HAVE_VM_VM_PARAM_H
#include <vm/vm_param.h>
#endif
#ifdef HAVE_VM_VM_EXTERN_H
#include <vm/vm_extern.h>
#endif
#ifdef HAVE_UVM_UVM_EXTERN_H
#include <uvm/uvm_extern.h>
#endif

netsnmp_feature_require(hardware_cpu_copy_stats)

void _cpu_copy_stats( netsnmp_cpu_info *cpu );

    /*
     * Initialise the list of CPUs on the system
     *   (including descriptions)
     */
void init_cpu_sysctl( void ) {
    int               i, n;
    size_t            siz;
    int               ncpu_mib[]  = { CTL_HW, HW_NCPU };
#if !(defined(__NetBSD__) && ( defined(__i386__) || defined(__x86_64__) ) )
    int               model_mib[] = { CTL_HW, HW_MODEL };
#endif
    char              descr[ SNMP_MAXBUF ];
    netsnmp_cpu_info  *cpu = netsnmp_cpu_get_byIdx( -1, 1 );
    strcpy(cpu->name, "Overall CPU statistics");

    siz = sizeof(n);
    sysctl(ncpu_mib, 2, &n, &siz, NULL, 0);
    if ( n <= 0 )
        n = 1;   /* Single CPU system */
    siz = sizeof(descr);
#if defined(__NetBSD__) && ( defined(__i386__) || defined(__x86_64__) )
    sysctlbyname("machdep.cpu_brand", descr, &siz, NULL, 0);
#else
    sysctl(model_mib, 2, descr, &siz, NULL, 0);
#endif
    for ( i = 0; i < n; i++ ) {
        cpu = netsnmp_cpu_get_byIdx( i, 1 );
        cpu->status = 2;  /* running */
        sprintf( cpu->name,  "cpu%d", i );
        sprintf( cpu->descr, "%s", descr );
    }
    cpu_num = n;
}


#if defined(__NetBSD__)
#define NETSNMP_CPU_STATS uint64_t
#else
#define NETSNMP_CPU_STATS long
#endif

#if defined(__NetBSD__)
#define NETSNMP_KERN_CPU  KERN_CP_TIME
#define NETSNMP_KERN_MCPU
#define NETSNMP_KERN_MCPU_TYPE NETSNMP_CPU_STATS
#elif defined(KERN_CPUSTATS)              /* BSDi */
#define NETSNMP_KERN_CPU  KERN_CPUSTATS
#elif defined(KERN_CPTIME2)                /* OpenBSD */
#define NETSNMP_KERN_CPU  KERN_CPTIME
#define NETSNMP_KERN_MCPU
#define NETSNMP_KERN_MCPU_TYPE NETSNMP_CPU_STATS
#elif defined(KERN_CPTIME)                /* OpenBSD */
#define NETSNMP_KERN_CPU  KERN_CPTIME

#elif defined(__FreeBSD__)
#define NETSNMP_KERN_MCPU 1    /* Enable support for multi-cpu stats. Valid for FreeBSD >=6.4, >=7.1, >=8.0 and beyond */
#define NETSNMP_KERN_MCPU_TYPE NETSNMP_CPU_STATS

#else
#error "No CPU statistics sysctl token"
#endif

/*
   Need to check details before enabling this!
#if defined(KERN_MPCPUSTATS)
#define NETSNMP_KERN_MCPU  KERN_MPCPUSTATS
#define NETSNMP_KERN_MCPU_TYPE  struct mpcpustats
#elif defined(KERN_MP_CPUSTATS)
#define NETSNMP_KERN_MCPU  KERN_MP_CPUSTATS
#define NETSNMP_KERN_MCPU_TYPE  struct cpustats
#endif
 */

#if defined(VM_UVMEXP2) || defined(VM_UVMEXP) 
    #define NS_VM_INTR		intrs
    #define NS_VM_SWTCH		swtch
    #define NS_VM_PAGEIN	pageins
    #define NS_VM_PAGEOUT	pdpageouts
#ifdef HAVE_STRUCT_UVMEXP_PGSWAPIN
    #define NS_VM_SWAPIN	pgswapin
    #define NS_VM_SWAPOUT	pgswapout
#else
    #define NS_VM_SWAPIN	swapins
    #define NS_VM_SWAPOUT	swapouts
#endif

#if defined(VM_UVMEXP2)                   /* NetBSD 1.6+ */
#define NETSNMP_VM_STATS       VM_UVMEXP2
#define NETSNMP_VM_STATS_TYPE  struct uvmexp_sysctl
#else /* VM_UVMEXP */                     /* OpenBSD 3+, NetBSD 1.6+ */
#define NETSNMP_VM_STATS       VM_UVMEXP
#define NETSNMP_VM_STATS_TYPE  struct uvmexp
#endif  /* VM_UVMEXP2 || VM_UVMEXP */

#elif defined(__FreeBSD__)                /* FreeBSD */
#define NETSNMP_VM_STATS       VM_METER
#define NETSNMP_VM_STATS_TYPE  struct vmmeter
    #define NS_VM_INTR		v_intr
    #define NS_VM_SWTCH		v_swtch
    #define NS_VM_PAGEIN	v_swappgsin
    #define NS_VM_PAGEOUT	v_swappgsout
    #define NS_VM_SWAPIN	v_swapin
    #define NS_VM_SWAPOUT	v_swapout

#elif defined(VM_METER)                   /* OpenBSD, NetBSD */
#define NETSNMP_VM_STATS       VM_METER
#define NETSNMP_VM_STATS_TYPE  struct vmtotal

#elif defined(VM_CNT)                     /* BSDi */
#define NETSNMP_VM_STATS       VM_CNT
#define NETSNMP_VM_STATS_TYPE  struct vmmeter
    #define NS_VM_INTR		v_intr
    #define NS_VM_SWTCH		v_swtch
    #undef  NS_VM_PAGEIN
    #undef  NS_VM_PAGEOUT
    #define NS_VM_SWAPIN	v_swpin
    #define NS_VM_SWAPOUT	v_swpout
#endif


    /*
     * Load the latest CPU usage statistics
     */
int netsnmp_cpu_arch_load( netsnmp_cache *cache, void *magic ) {

    /*
     * Strictly speaking, BSDi ought to use
     *    "struct cpustats  cpu_stats"
     * but this array was used in the previous code, and
     *   is correct for the {Open,Net}BSD versions too.
     * Don't fight it, Dave - go with the flow....
     */
    NETSNMP_CPU_STATS cpu_stats[CPUSTATES];
    size_t         cpu_size  = sizeof(cpu_stats);
#if !defined(__FreeBSD__) && !defined(__NetBSD__)
    int            cpu_mib[] = { CTL_KERN, NETSNMP_KERN_CPU };
#endif
#ifdef __FreeBSD__
    static int     cp_times = -1;
#endif
#ifdef KERN_CPTIME2
    int            mcpu_mib[] = { CTL_KERN, KERN_CPTIME2, 0 };
#endif
#ifdef NETSNMP_KERN_MCPU 
    int            i;
    int            act_cpu = cpu_num;
    NETSNMP_KERN_MCPU_TYPE *mcpu_stats;
    size_t         mcpu_size;
#endif
    NETSNMP_VM_STATS_TYPE mem_stats;
    int            mem_mib[] = { CTL_VM, NETSNMP_VM_STATS };
    size_t         mem_size  = sizeof(NETSNMP_VM_STATS_TYPE);
    netsnmp_cpu_info *cpu = netsnmp_cpu_get_byIdx( -1, 0 );

#if defined(__FreeBSD__) || defined(__NetBSD__)
    sysctlbyname("kern.cp_time", cpu_stats, &cpu_size, NULL, 0);
#else
    sysctl(cpu_mib, 2,  cpu_stats, &cpu_size, NULL, 0);
#endif
    cpu->user_ticks = cpu_stats[CP_USER];
    cpu->nice_ticks = cpu_stats[CP_NICE];
    cpu->sys2_ticks = cpu_stats[CP_SYS]+cpu_stats[CP_INTR];
    cpu->kern_ticks = cpu_stats[CP_SYS];
    cpu->idle_ticks = cpu_stats[CP_IDLE];
    cpu->intrpt_ticks = cpu_stats[CP_INTR];
        /* wait_ticks, sirq_ticks unused */
    
        /*
         * Interrupt/Context Switch statistics
         *   XXX - Do these really belong here ?
         */
    sysctl(mem_mib, 2, &mem_stats, &mem_size, NULL, 0);
    cpu->nInterrupts  = mem_stats.NS_VM_INTR;
    cpu->nCtxSwitches = mem_stats.NS_VM_SWTCH;
    cpu->swapIn       = mem_stats.NS_VM_SWAPIN;
    cpu->swapOut      = mem_stats.NS_VM_SWAPOUT;
#ifdef NS_VM_PAGEIN
    cpu->pageIn       = mem_stats.NS_VM_PAGEIN;
#endif
#ifdef NS_VM_PAGEOUT
    cpu->pageOut      = mem_stats.NS_VM_PAGEOUT;
#endif

#ifdef NETSNMP_KERN_MCPU
#if defined(KERN_CPTIME2)
    mcpu_size  = cpu_num*sizeof(cpu_stats);
    mcpu_stats = malloc(mcpu_size);
#elif defined(__NetBSD__)
    mcpu_size  = cpu_num*sizeof(cpu_stats);
    mcpu_stats = malloc(mcpu_size);
    sysctlbyname("kern.cp_time", mcpu_stats, &mcpu_size, NULL, 0);
#elif defined(__FreeBSD__)
    if (cp_times == -1) {
	int ret = sysctlbyname("kern.cp_times", NULL, &mcpu_size, NULL, 0);
	cp_times = ret == -1 ? 0 : 1;
    }
    if (cp_times) {
	sysctlbyname("kern.cp_times", NULL, &mcpu_size, NULL, 0);
	mcpu_stats = malloc(mcpu_size);
	sysctlbyname("kern.cp_times", mcpu_stats, &mcpu_size, NULL, 0);
    }
    else {
	mcpu_size  = sizeof(cpu_stats);
	mcpu_stats = malloc(mcpu_size);
	sysctlbyname("kern.cp_time", mcpu_stats, &mcpu_size, NULL, 0);
	act_cpu = 1;
    }
#endif
    for ( i = 0; i < act_cpu; i++ ) {
        cpu = netsnmp_cpu_get_byIdx( i, 0 );
        /* XXX - per-CPU statistics - mcpu_mib[i].??? */
#ifdef KERN_CPTIME2
	mcpu_mib[2] = i;
	sysctl(mcpu_mib, 3, mcpu_stats+i*CPUSTATES, &mcpu_size, NULL, 0);
#endif
	/* Almost copy & paste of previous cpu stats stuff :) */
	cpu->user_ticks = mcpu_stats[(i*CPUSTATES)+CP_USER];
	cpu->nice_ticks = mcpu_stats[(i*CPUSTATES)+CP_NICE];
	cpu->sys2_ticks = mcpu_stats[(i*CPUSTATES)+CP_SYS]+mcpu_stats[(i*CPUSTATES)+CP_INTR];
	cpu->kern_ticks = mcpu_stats[(i*CPUSTATES)+CP_SYS];
	cpu->idle_ticks = mcpu_stats[(i*CPUSTATES)+CP_IDLE];
	cpu->intrpt_ticks = mcpu_stats[(i*CPUSTATES)+CP_INTR];
	    /* wait_ticks, sirq_ticks unused */

	    /*
	     * Interrupt/Context Switch statistics
	     *   XXX - Do these really belong here ?
	     */

	/* There's no real need to execute another sysctl()
	 *
	 * sysctl(mem_mib, 2, &mem_stats, &mem_size, NULL, 0);
	 */
	cpu->nInterrupts  = mem_stats.NS_VM_INTR;
	cpu->nCtxSwitches = mem_stats.NS_VM_SWTCH;
	cpu->swapIn       = mem_stats.NS_VM_SWAPIN;
	cpu->swapOut      = mem_stats.NS_VM_SWAPOUT;
#ifdef NS_VM_PAGEIN
	cpu->pageIn       = mem_stats.NS_VM_PAGEIN;
#endif
#ifdef NS_VM_PAGEOUT
	cpu->pageOut      = mem_stats.NS_VM_PAGEOUT;
#endif
    }
    free(mcpu_stats);
#else	/* NETSNMP_KERN_MCPU */
        /* Copy "overall" figures to cpu0 entry */
    _cpu_copy_stats( cpu );
#endif  /* NETSNMP_KERN_MCPU */

    return 0;
}
