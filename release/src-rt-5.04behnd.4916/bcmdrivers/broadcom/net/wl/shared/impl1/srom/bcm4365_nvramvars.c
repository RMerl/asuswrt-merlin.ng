#include <typedefs.h>

char wl_nvramvars_4365[] =
	"ledbh9=0x88\0"
#if defined(DSLCPE) && defined(DISABLE_CTDMA_4GEN1PCIE)
	"ctdma=0x0\0"
#endif /* DSLCPE && DISABLE_CTDMA_4GEN1PCIE */
	"END\0";
