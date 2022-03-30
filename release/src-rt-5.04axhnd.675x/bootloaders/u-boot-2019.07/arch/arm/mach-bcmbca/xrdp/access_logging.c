/*
 * access_logging.c
 */

#ifdef CONFIG_BCMBCA_XRDP_GPL

#include <common.h>
#include "access_macros.h"
#include "access_logging.h"

#define xrdp_usleep(_a) udelay(_a)

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
	int i;
	volatile uint32_t *p_addr;
	uint32_t start_val;
	uint32_t adaptive_val;
	const access_log_tuple_t *entry = entry_array;
	uint32_t addr;
	uint32_t op;
	uint32_t size;
	uint32_t value;
	addr_op_size_st addr_op_size;

	while (1) {
		addr_op_size.value = entry->addr_op_size;
		op = addr_op_size.op_code;
		size = addr_op_size.size;
		addr = addr_op_size.addr + 0x82000000;
		if ((op == ACCESS_LOG_OP_WRITE) ||
		    (op == ACCESS_LOG_OP_MWRITE) ||
		    (op == ACCESS_LOG_OP_WRITE_6BYTE_ADDR)) {
			if (op == ACCESS_LOG_OP_WRITE_6BYTE_ADDR)
				addr-= 0x82000000;

			if (size == 4)
				*(volatile uint32_t *)((uintptr_t)addr) = (uint32_t)(entry->value);
			else if (size == 2)
				*(volatile uint16_t *)((uintptr_t)addr) = (uint16_t)(entry->value);
			else if (size == 1)
				*(volatile uint8_t *)((uintptr_t)addr) = (uint8_t)(entry->value);
			else {
				ACCESS_LOG_PRINT("!!!!!!! op=%d: invalid entry size %u\n", op, size);
				rc = -1;
				break;
			}
			ACCESS_LOG_DEBUG("write op(%d) size(%d) addr 0x%x data 0x%x\n", op, size, addr, entry->value);
		} else if (op == ACCESS_LOG_OP_SLEEP) {
			ACCESS_LOG_PRINT("Sleep %u...\n", entry->value);
			xrdp_usleep(entry->value);
		} else if (op == ACCESS_LOG_OP_MEMSET) {
			size = entry->value;
			++entry;
			addr = entry->addr_op_size;
			value = entry->value;
			ACCESS_LOG_DEBUG("MEMSET(0x%x, %u, %u)\n", addr, value, size);
			MEMSET((void *)(uintptr_t)addr, value, size);
		} else if (op == ACCESS_LOG_OP_MEMSET_32) {
			size = entry->value;
			++entry;
			addr = entry->addr_op_size;
			value = entry->value;
			ACCESS_LOG_DEBUG("MEMSET_32(0x%x, %u, %u)\n", addr, value, size);
			MEMSET_32((void *)(uintptr_t)addr, value, size);
		} else if (op == ACCESS_LOG_OP_MEMSET_ADAPTIVE_32) {
			size = entry->value;
			++entry;
			addr = entry->addr_op_size;
			value = entry->value;
			p_addr = (volatile uint32_t *)(uintptr_t)addr;
			start_val = value & 0xffff;
			adaptive_val = (value >> 16) & 0xffff;;
			ACCESS_LOG_DEBUG("MEMSET_ADAPTIVE_32(0x%lx, %u %u, %u)\n", addr, start_val, adaptive_val, size);

			for (i = 0; i < size; i++) {
				p_addr[i] = (uint32_t)start_val;
				start_val += adaptive_val;
			}
		} else if (op == ACCESS_LOG_OP_SET_CORE_ADDR) {
			ACCESS_LOG_DEBUG("SET_CORE_ADDR[%u] = 0x%lx\n", entry->value, addr);
			rdp_runner_core_addr[entry->value] = addr;
		} else if (op == ACCESS_LOG_OP_STOP)
			break;
		else {
			ACCESS_LOG_PRINT("!!!!!!! op=%d: invalid operation\n", op);
			rc = -1;
			break;
		}
		++entry;
	}

	if (entry)
		ACCESS_LOG_PRINT("%s: %d entries processed\n", __FUNCTION__, (int)(entry - entry_array));

	return rc;
}
#else

int access_log_enable;

#endif /* #ifdef CONFIG_BCMBCA_XRDP_GPL */

