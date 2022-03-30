#include <common.h>
#include <fdt_support.h>
#include <environment.h>
#include <stdlib.h>
#include "tpl_params.h"

void update_uboot_fdt(void *fdt_addr, tpl_params *tplp);
void ft_update_cpu_nodes(void* dtb_ptr, bd_t *bd);

#define ENV_RDP1			PARAM1_BASE_ADDR_STR
#define ENV_RDP2 			PARAM2_BASE_ADDR_STR
#define ENV_DHD1			DHD_BASE_ADDR_STR	
#define ENV_DHD2			DHD_BASE_ADDR_STR_1
#define ENV_DHD3			DHD_BASE_ADDR_STR_2
#define ENV_BUFMEM			BUFMEM_BASE_ADDR_STR
#define ENV_RNRMEM			RNRMEM_BASE_ADDR_STR

#define QUAD_CPUS 4
#define DUAL_CPUS 2
#define ONE_CPU   1

