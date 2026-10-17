#include <arm_acle.h>
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include <lib/mmio.h>
#include <lib/utils_def.h>
#include <platform_def.h>

#include <lib/smccc.h>
#include <services/trng_svc.h>
#include <smccc_helpers.h>
#include <plat/common/platform.h>
#include <bcm_rng.h>

DEFINE_SVC_UUID2(_plat_trng_uuid,
	0x12345678, 0x7448, 0x4083, 0x9d, 0x16,
	0xe3, 0xfa, 0xb9, 0xf1, 0x73, 0xbc
);
uuid_t plat_trng_uuid;


bool plat_get_entropy(uint64_t *out)
{
	rng_get_rand((uint8_t*)out, sizeof(uint64_t));
	return true;
}


void plat_entropy_setup(void)
{
	plat_trng_uuid = _plat_trng_uuid;
	rng_init();
	rng_lock();
}
