#include "rom_main.h"
#include "lib_crc.h"
#include "jffs2.h"
#include "bcm_ubi.h"
#include "lib_byteorder.h"
#define je16_to_cpu(x) ((x).v16)
#define je32_to_cpu(x) ((x).v32)


void read_memcfg_from_nvram(uint32_t *memcfg);
void nvram_read(PNVRAM_DATA nvram_data);
