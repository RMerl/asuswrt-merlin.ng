/*
 * access_logging.c
 */

#ifdef CONFIG_GPL_RDP

#include "access_macros.h"
#include "access_logging.h"

#ifdef _CFE_
extern void cfe_usleep(int);
#define xrdp_usleep(_a) cfe_usleep(_a)
#else
#error Need to define xrdp_usleep for the platform
#endif

#define ACCESS_LOG_PRINT(_args...)  printf(_args);

/* #define ACCESS_LOG_DEBUG_ENABLE */

#ifdef ACCESS_LOG_DEBUG_ENABLE
#define ACCESS_LOG_DEBUG(_args...)  ACCESS_LOG_PRINT(_args)
#else
#define ACCESS_LOG_DEBUG(_args...)
#endif

extern uintptr_t rdp_runner_core_addr[];

extern int access_log_restore(const access_log_tuple_t *entry_array)
{
    int rc = 0;
    const access_log_tuple_t *entry = entry_array;

    while (1)
    {
        if (entry->op == ACCESS_LOG_OP_WRITE || entry->op == ACCESS_LOG_OP_MWRITE)
        {
            if (entry->size == 4)
                *(volatile uint32_t *)entry->addr = (uint32_t)entry->value;
            else if (entry->size == 2)
                *(volatile uint16_t *)entry->addr = (uint16_t)entry->value;
            else if (entry->size == 1)
                *(volatile uint8_t *)entry->addr = (uint8_t)entry->value;
            else
            {
                ACCESS_LOG_PRINT("!!!!!!! op=%d: invalid entry size %u\n", entry->op, entry->size);
                rc = -1;
                break;
            }
        }
        else if (entry->op == ACCESS_LOG_OP_SLEEP)
        {
            ACCESS_LOG_PRINT("Sleep %u...\n", entry->value);
            xrdp_usleep(entry->value);
        }
        else if (entry->op == ACCESS_LOG_OP_MEMSET)
        {
            ACCESS_LOG_DEBUG("MEMSET(0x%lx, %u, %u)\n", entry->addr, entry->value, entry->size);
            MEMSET((void *)entry->addr, entry->value, entry->size);
        }
        else if (entry->op == ACCESS_LOG_OP_MEMSET_32)
        {
            ACCESS_LOG_DEBUG("MEMSET_32(0x%lx, %u, %u)\n", entry->addr, entry->value, entry->size);
            MEMSET_32((void *)entry->addr, entry->value, entry->size);
        }
        else if (entry->op == ACCESS_LOG_OP_SET_CORE_ADDR)
        {
            ACCESS_LOG_DEBUG("SET_CORE_ADDR[%u] = 0x%lx\n", entry->value, entry->addr);
            rdp_runner_core_addr[entry->value] = entry->addr;
        }
        else if (entry->op == ACCESS_LOG_OP_STOP)
        {
            break;
        }
        else
        {
            ACCESS_LOG_PRINT("!!!!!!! op=%d: invalid operation\n", entry->op);
            rc = -1;
            break;
        }
        ++entry;
    }

    if (entry)
    {
        ACCESS_LOG_PRINT("%s: %d entries processed\n", __FUNCTION__, (int)(entry - entry_array));
    }

    return rc;
}

#else

#ifdef CONFIG_GPL_RDP_GEN

int access_log_enable;

#endif

#endif /* #ifdef CONFIG_GPL_RDP */

