#include "cmdlist_arch.h"
#include "cmdlist.h"

#if !(defined(RDP_ARCH_BOARD) || defined (RDP_ARCH_QEMU_SIM))
#error "non board compilation"
#endif

static struct kmem_cache *cmdlist_cache_p = NULL;

void *cmdlist_buffer_alloc(void **phys_addr_pp)
{
    void *buffer_p = kmem_cache_alloc(cmdlist_cache_p, GFP_ATOMIC);

    if(buffer_p)
    {
        *phys_addr_pp = (void *)VIRT_TO_PHYS(buffer_p);
    }

    __logDebug("buffer_p %p, phys_addr_p %px", buffer_p, *phys_addr_pp);

    return buffer_p;
}

void cmdlist_buffer_free(void *buffer_p)
{
    __logDebug("buffer_p %px", buffer_p);

    kmem_cache_free(cmdlist_cache_p, buffer_p);
}

int __init cmdlist_buffer_construct(void)
{
    cmdlist_cache_p = kmem_cache_create("cmdlist_buffer",
                                        CMDLIST_BUFFER_SIZE,
                                        0, /* align */
                                        SLAB_HWCACHE_ALIGN, /* flags */
                                        NULL); /* ctor */
    if(cmdlist_cache_p == NULL)
    {
        __logError("Unable to create Buffer Cache\n");

        return -ENOMEM;
    }

    return 0;
}

void __exit cmdlist_buffer_destruct(void)
{
    kmem_cache_destroy(cmdlist_cache_p);
}

void cmdlist_buffer_flush(void *buffer_p)
{
    cache_flush_len(buffer_p, CMDLIST_BUFFER_SIZE);
}
