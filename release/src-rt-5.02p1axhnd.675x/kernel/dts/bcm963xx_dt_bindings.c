#ifndef __DT_BINDINGS_BCM63XX_H
#define __DT_BINDINGS_BCM63XX_H

#include "bcm963xx/bcm_hwdefs.h"

#define __stringify_1(x...)   #x
#define __stringify(x...)  __stringify_1(x)
#define build_clause(x1,x2,x3)  x1 x2 x3
#define build_string_clause(x1,x2,x3)  x1 x2 __stringify(x3)

#define MB(N)        ((N)*0x100000)
#define MBALIGN(N)   ((N+0xFFFFF)&0xFFF00000)

#define	DRAM_BASE_END     (DRAM_BASE+DRAM_DEF_SIZE)

#if defined (CONFIG_BCM_ADSL) 
#include "softdsl/AdslCoreDefs.h"
build_clause(#define,"ADSL_SDRAM_IMAGE_SIZE",MBALIGN(ADSL_SDRAM_IMAGE_SIZE))
#define DRAM_OFFSET_ADSL  (DRAM_BASE_END-MBALIGN(ADSL_SDRAM_IMAGE_SIZE))
build_clause(#define,"DRAM_OFFSET_ADSL",DRAM_OFFSET_ADSL)
#else
#define DRAM_OFFSET_ADSL  DRAM_BASE_END
#endif

#if defined (CONFIG_BCM960333)
#define DRAM_OFFSET_PLC   (DRAM_BASE_END-MBALIGN(PLC_SDRAM_SIZE))
build_clause(#define,"DRAM_OFFSET_PLC",DRAM_OFFSET_PLC)
build_clause(#define,"PLC_SDRAM_SIZE",MBALIGN(PLC_SDRAM_SIZE))
#endif

#if defined (CONFIG_BCM_RDPA) 

#if defined(CONFIG_RDP_PARAM1_SIZE)
#if MB(CONFIG_RDP_PARAM1_SIZE) < RDP_PARAM1_DEF_DDR_SIZE
#warning "RDP PARAM1 size in profile smaller than the default value in bcm_hwdefs.h. Use header file value!"
#define RDP_PARAM1_DDR_SIZE RDP_PARAM1_DEF_DDR_SIZE
#else
#define RDP_PARAM1_DDR_SIZE MB(CONFIG_RDP_PARAM1_SIZE)
#endif
#else
#define RDP_PARAM1_DDR_SIZE RDP_PARAM1_DEF_DDR_SIZE
#endif

#if defined(CONFIG_RDP_PARAM2_SIZE)
#if MB(CONFIG_RDP_PARAM2_SIZE) < RDP_PARAM2_DEF_DDR_SIZE
#warning "RDP PARAM2 size in profile smaller than the default value in bcm_hwdefs.h. Use header file value!"
#define RDP_PARAM2_DDR_SIZE RDP_PARAM2_DEF_DDR_SIZE
#else
#define RDP_PARAM2_DDR_SIZE MB(CONFIG_RDP_PARAM2_SIZE)
#endif
#else
#define RDP_PARAM2_DDR_SIZE RDP_PARAM2_DEF_DDR_SIZE
#endif

#define DRAM_OFFSET_RDP_PARAM1  (DRAM_OFFSET_ADSL-RDP_PARAM1_DDR_SIZE)
#define DRAM_OFFSET_RDP_PARAM2  (DRAM_OFFSET_RDP_PARAM1-RDP_PARAM2_DDR_SIZE)

build_clause(#define,"RDP_PARAM1_DDR_SIZE",RDP_PARAM1_DDR_SIZE)
build_clause(#define,"RDP_PARAM2_DDR_SIZE",RDP_PARAM2_DDR_SIZE)
build_clause(#define,"DRAM_OFFSET_RDP_PARAM1",DRAM_OFFSET_RDP_PARAM1)
build_clause(#define,"DRAM_OFFSET_RDP_PARAM2",DRAM_OFFSET_RDP_PARAM2)

#endif

#if defined (CONFIG_BCM_DHD_RUNNER) 

#if defined(CONFIG_DHD_PARAM1_SIZE)
#define DHD_PARAM1_DDR_SIZE MB(CONFIG_DHD_PARAM1_SIZE)
#else
#define DHD_PARAM1_DDR_SIZE 0x0
#endif
#if defined(CONFIG_DHD_PARAM2_SIZE)
#define DHD_PARAM2_DDR_SIZE MB(CONFIG_DHD_PARAM2_SIZE)
#else
#define DHD_PARAM2_DDR_SIZE 0x0
#endif
#if defined(CONFIG_DHD_PARAM3_SIZE)
#define DHD_PARAM3_DDR_SIZE MB(CONFIG_DHD_PARAM3_SIZE)
#else
#define DHD_PARAM3_DDR_SIZE 0x0
#endif

build_clause(#define,"DHD_PARAM1_DDR_SIZE",DHD_PARAM1_DDR_SIZE)
build_clause(#define,"DHD_PARAM2_DDR_SIZE",DHD_PARAM2_DDR_SIZE)
build_clause(#define,"DHD_PARAM3_DDR_SIZE",DHD_PARAM3_DDR_SIZE)

#endif

build_clause(#define,"DRAM_BASE",DRAM_BASE)
build_clause(#define,"DRAM_DEF_SIZE",DRAM_DEF_SIZE)

#if defined (OOPSLOG_PARTITION_NAME)
build_string_clause(#define,"OOPSLOG_PARTITION_NAME",OOPSLOG_PARTITION_NAME)
#endif

#endif
