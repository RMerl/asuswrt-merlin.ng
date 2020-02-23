/*
 * <:copyright-BRCM:2019:DUAL/GPL:standard
 * 
 *    Copyright (c) 2019 Broadcom 
 *    All Rights Reserved
 * 
 * Unless you and Broadcom execute a separate written software license
 * agreement governing use of this software, this software is licensed
 * to you under the terms of the GNU General Public License version 2
 * (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
 * with the following added to such license:
 * 
 *    As a special exception, the copyright holders of this software give
 *    you permission to link this software with independent modules, and
 *    to copy and distribute the resulting executable under terms of your
 *    choice, provided that you also meet, for each linked independent
 *    module, the terms and conditions of the license of that module.
 *    An independent module is a module which is not derived from this
 *    software.  The special exception does not apply to any modifications
 *    of the software.
 * 
 * Not withstanding the above, under no circumstances may you combine
 * this software in any way with any other Broadcom software provided
 * under a license other than the GPL, without Broadcom's express prior
 * written consent.
 * 
 * :>
 */


#include <bdmf_system.h>

#ifdef XRDP
/* Allocate/release memory */
bdmf_phys_addr_t g_rsv_phys_base_addr = 0;
void *g_rsv_base_addr = NULL;
uint32_t g_rsv_alloc_total_size = 0;
static uint32_t g_rsv_allocated = 0;

bdmf_int_parm_t g_int_params[MAX_INT_NUM] = {};

#ifndef XRDP_EMULATION
static int g_fd_ddr = -1;
#endif

inline bdmf_phys_addr_t rsv_virt_to_phys(volatile void *vir_addr)
{
    long offset = (uint8_t *)vir_addr - (uint8_t *)g_rsv_base_addr;
    if (offset < 0)
        BUG();

    return (bdmf_phys_addr_t)(g_rsv_phys_base_addr + offset);
}

inline void *rsv_phys_to_virt(bdmf_phys_addr_t phys_addr)
{
    long offset = (long)phys_addr - (long)g_rsv_phys_base_addr;
    if (offset < 0)
        BUG();

    return (void *)((uint8_t *)g_rsv_base_addr + offset);
}

void *bdmf_alloc_rsv(int size, bdmf_phys_addr_t *phys_addr)
{
    void *p;

    if (!g_rsv_base_addr)
        return NULL;
    if ((g_rsv_allocated + size) > g_rsv_alloc_total_size)
        return NULL;

    p = g_rsv_base_addr + g_rsv_allocated;
    g_rsv_allocated += size;

    if (phys_addr)
        *phys_addr = rsv_virt_to_phys(p);
    return p;
}

void bdmf_rsv_mem_stats(void)
{
    bdmf_print("total ddr size confiugred: 0x%08X\n", g_rsv_alloc_total_size);
    bdmf_print("base addr virt:            0x%p\n", g_rsv_base_addr);
    bdmf_print("total ddr size allocated:  0x%08X\n", g_rsv_allocated);
}

#ifndef XRDP_EMULATION
void *bdmf_rsv_mem_init(void)
{
    int flags, i;
    struct stat sbuf;
    
    g_rsv_phys_base_addr = RUNNER_TABLES_BASE_ADDR;
    g_rsv_alloc_total_size = DDR_RSRVED_SEGMENT_SIZE;
    
    flags = O_RDWR;
    if (stat("ddr.mem", &sbuf) == -1)
        flags |= O_CREAT;
    if ((g_fd_ddr = open("ddr.mem", flags, S_IRWXU | S_IRWXG | S_IRWXO)) == -1)
    {
        bdmf_print("open ddr.mem failed\n");
        return NULL;
    }
    if (ftruncate(g_fd_ddr, DDR_RSRVED_SEGMENT_SIZE))
    {
        bdmf_print("ftruncate ddr.mem failed\n");
        return NULL;
    }

    g_rsv_base_addr = mmap((caddr_t)0, DDR_RSRVED_SEGMENT_SIZE, PROT_WRITE | PROT_READ, MAP_SHARED, g_fd_ddr, 0);
    if (g_rsv_base_addr == (caddr_t)(-1))
        return NULL;

    for (i = 0; i < DDR_RSRVED_SEGMENT_SIZE / 4;)
    {
    	*(uint32_t *)(g_rsv_base_addr + i) = 0xdead;
    	i++;
    	*(uint32_t *)(g_rsv_base_addr + i) = 0xbeef;
    	i++;
    }
    /* memset(g_rsv_base_addr, 0, DDR_RSRVED_SEGMENT_SIZE); */

    bdmf_rsv_mem_stats();

    return g_rsv_base_addr;
}
#else
void *bdmf_rsv_mem_init(void)
{
    g_rsv_phys_base_addr = RUNNER_TABLES_BASE_ADDR;
    g_rsv_alloc_total_size = DDR_RSRVED_SEGMENT_SIZE;
    g_rsv_base_addr = (void *)g_rsv_phys_base_addr;

    return g_rsv_base_addr;
}
#endif

void bdmf_rsv_mem_destroy(void)
{
#ifndef XRDP_EMULATION
    bdmf_print("bdmf_alloc_destroy\n");
    if (g_rsv_base_addr)
    {
        if (munmap(g_rsv_base_addr, DDR_RSRVED_SEGMENT_SIZE) == -1)
            bdmf_print_error("error while unmapping ddr.mem\n");
    }
    if (g_fd_ddr > 0)
        close(g_fd_ddr);
#endif
}

void *kmem_cache_alloc(struct kmem_cache *cachep, uint32_t flags)
{
    return bdmf_alloc_rsv(SKB_ALLOC_LEN, NULL);
}
#endif

int bdmf_int_connect(int irq, int cpu, int flags,
    int (*isr)(int irq, void *priv), const char *name, void *priv)
{
#ifdef XRDP
	g_int_params[irq].irq = irq;
	g_int_params[irq].priv = priv;
	g_int_params[irq].int_cb = isr;
#endif
    return 0;
}

void bdmf_int_disconnect(int irq, void *priv)
{
#ifdef XRDP
	g_int_params[irq].irq = 0;
	g_int_params[irq].priv = NULL;
	g_int_params[irq].int_cb = NULL;
#endif
}

uint32_t jiffies = 0;

int bdmf_task_create(const char *name, int priority, int stack,
                int (*handler)(void *arg), void *arg, bdmf_task *ptask)
{
    int rc;
    rc = pthread_create(ptask, NULL, (void *(*)(void *))handler, arg);
    return rc ? BDMF_ERR_SYSCALL_ERR : 0;
}

int bdmf_task_destroy(bdmf_task task)
{
    int rc;
    void *res;
    pthread_cancel(task);
    rc = pthread_join(task, &res);
    return (rc || (res != PTHREAD_CANCELED)) ? BDMF_ERR_SYSCALL_ERR : 0;
}

/*
 * Shared memory mapping
 */
#ifndef XRDP_EMULATION
void *bdmf_mmap(const char *fname, uint32_t size)
{
    void *map;
    int fd;
    struct stat stat;
    int rc;

    fd = shm_open(fname, O_RDWR | O_CREAT, 0x1ff);
    if (fd <= 0)
    {
        bdmf_print_error("Failed to open shm file %s\n", fname);
        return NULL;
    }

    rc = fstat(fd, &stat);
    if (rc == -1)
    {
        close(fd);
        bdmf_print_error("Failed to fstat shm file %s\n", fname);
        return NULL;
    }

    if (stat.st_size < size)
    {
        /* stretch file */
        rc = lseek(fd, size-1, SEEK_SET);
        rc = (rc < 0) ? rc : write(fd, "", 1);
        if (rc == -1)
        {
            close(fd);
            bdmf_print_error("Failed to stretch shm file %s to %u bytes\n", fname, size);
            return NULL;
        }
    }

     /* Now the file is ready to be mmapped.
      */
     map = mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
     close(fd);
     if (map == MAP_FAILED)
     {
         bdmf_print_error("Error mapping the file %s\n", fname);
         return NULL;
     }
     return map;
}
#endif

/*
 * Interrupt handling
 */
struct bdmf_irq
{
    f_bdmf_irq_cb cb;
    void *data;
};
static struct bdmf_irq bdmf_irq_handlers[BDMFSYS_IRQ__NUM_OF];

static void bdmf_irq_sigaction(int sig, siginfo_t *info, void *dummy)
{
    int irq;

    irq = info->si_int;
    assert((unsigned)irq < BDMFSYS_IRQ__NUM_OF);
    //bdmf_print("%s: got irq%d\n", __FUNCTION__, irq);
    if (!bdmf_irq_handlers[irq].cb)
    {
        bdmf_print_error("irq%d is not connected\n", irq);
        return;
    }
    bdmf_irq_handlers[irq].cb(irq, bdmf_irq_handlers[irq].data);
}

int bdmf_irq_connect(int irq, f_bdmf_irq_cb cb, void *data)
{
    static int signal_connected;
    if ((unsigned)irq >= BDMFSYS_IRQ__NUM_OF)
        return BDMF_ERR_PARM;
    if (bdmf_irq_handlers[irq].cb)
        return BDMF_ERR_ALREADY;
    if (!signal_connected)
    {
        struct sigaction act;
        int rc;
        memset(&act, 0, sizeof(act));
        act.sa_flags = SA_SIGINFO;
        act.sa_sigaction = bdmf_irq_sigaction;
        rc = sigaction(SIGUSR1, &act, NULL);
        if (rc)
            return BDMF_ERR_INTERNAL;
        signal_connected = 1;
    }
    bdmf_irq_handlers[irq].cb = cb;
    bdmf_irq_handlers[irq].data = data;
    bdmf_print("%s: connected irq%d\n", __FUNCTION__, irq);
    return 0;
}

int bdmf_irq_free(int irq, f_bdmf_irq_cb cb, void *data)
{
    if ((unsigned)irq >= BDMFSYS_IRQ__NUM_OF)
        return BDMF_ERR_PARM;
    if ((bdmf_irq_handlers[irq].cb != cb) ||
        (bdmf_irq_handlers[irq].data != data))
        return BDMF_ERR_NOT_CONNECTED;
    bdmf_irq_handlers[irq].cb = NULL;
    bdmf_irq_handlers[irq].data = NULL;
    return 0;
}

void bdmf_irq_raise(int irq)
{
    union sigval value;
    int pid = getpid();
    int rc;

again:
    value.sival_int = irq;
    rc = sigqueue(pid, SIGUSR1, value);
    if (rc)
    {
        bdmf_print_error("failed to raise irq%d on pid %d. rc=%d\n", irq, pid, rc);
        if (rc == ESRCH)
            goto again;
    }
}

/*
 * Recursive mutex support
 */

typedef struct {
    int initialized; /* Should overlap with 'initialized' member in struct bdmf_ta_mutex */
    pthread_t self;
    int count;
    pthread_mutex_t m;
} bdmf_sim_ta_mutex;

static pthread_mutex_t ta_mutex_lock = PTHREAD_MUTEX_INITIALIZER;

void bdmf_ta_mutex_init(bdmf_ta_mutex *pmutex)
{
    bdmf_sim_ta_mutex *tam = (bdmf_sim_ta_mutex *)pmutex;
    BUG_ON(sizeof(bdmf_sim_ta_mutex) > sizeof(bdmf_ta_mutex));
    tam->self = -1;
    tam->count = 0;
    pthread_mutex_init(&tam->m, NULL);
    tam->initialized = 1;
}

void bdmf_ta_mutex_delete(bdmf_ta_mutex *pmutex)
{
    bdmf_sim_ta_mutex *tam = (bdmf_sim_ta_mutex *)pmutex;
    pthread_mutex_destroy(&tam->m);
}

int bdmf_ta_mutex_lock(bdmf_ta_mutex *pmutex)
{
    bdmf_sim_ta_mutex *tam = (bdmf_sim_ta_mutex *)pmutex;

    if (!tam->initialized)
        bdmf_ta_mutex_init(pmutex);
    pthread_mutex_lock(&ta_mutex_lock);
    if (tam->self == pthread_self())
    {
        ++tam->count;
        pthread_mutex_unlock(&ta_mutex_lock);
        return 0;
    }
    pthread_mutex_unlock(&ta_mutex_lock);

    /* not-recurring request */
    pthread_mutex_lock(&tam->m);

    tam->self = pthread_self();
    tam->count = 1;

    return 0;
}

void bdmf_ta_mutex_unlock(bdmf_ta_mutex *pmutex)
{
    bdmf_sim_ta_mutex *tam = (bdmf_sim_ta_mutex *)pmutex;

    if (!tam->initialized)
        bdmf_ta_mutex_init(pmutex);
    BUG_ON(tam->self != pthread_self());
    BUG_ON(tam->count < 1);
    if (--tam->count == 0)
    {
        tam->self = -1;
        pthread_mutex_unlock(&tam->m);
    }
}

static uint32_t sysb_headroom[bdmf_sysb_type__num_of];

/** Set headroom size for system buffer
 * \param[in]   sysb_type   System buffer type
 * \param[in]   headroom    Headroom size
 */
void bdmf_sysb_headroom_size_set(bdmf_sysb_type sysb_type, uint32_t headroom)
{
    sysb_headroom[sysb_type] = headroom;
}

/** Allocate system buffer.
 * \param[in]   sysb_type   System buffer type
 * \param[in]   length      Data length
 * \return system buffer pointer.
 * If the function returns NULL, caller is responsible for "data" deallocation
 */
bdmf_sysb bdmf_sysb_alloc(bdmf_sysb_type sysb_type, uint32_t length)
{
    if (sysb_type == bdmf_sysb_skb)
    {
        struct sk_buff *skb = dev_alloc_skb(length + sysb_headroom[bdmf_sysb_skb]);
        if (!skb)
            return NULL;
        skb_reserve(skb, sysb_headroom[bdmf_sysb_skb]);
        return (bdmf_sysb)skb;
    }
    return NULL;
}

/*
 * Timer support
 * Not implemented
 */

void bdmf_timer_init(bdmf_timer_t *timer, bdmf_timer_cb_t cb, unsigned long priv)
{

}

int bdmf_timer_start(bdmf_timer_t *timer, uint32_t ticks)
{
    return 0;
}

void bdmf_timer_stop(bdmf_timer_t *timer)
{

}

void bdmf_timer_delete(bdmf_timer_t *timer)
{

}

uint32_t bdmf_ms_to_ticks(uint32_t ms)
{
    return ms * 1000;
}

void *bdmf_ioremap(bdmf_phys_addr_t phys_addr, size_t size __attribute__((unused)))
{
    return (void *)phys_addr;
}

#ifndef XRDP
void *bdmf_alloc_uncached(int size, bdmf_phys_addr_t *phys_addr_p)
{
    *phys_addr_p = (bdmf_phys_addr_t)(long)(bdmf_alloc(size));

    return (void *)((long)*phys_addr_p);
}
#endif
