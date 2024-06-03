
#include "rdp_drv_sbpm.h"
void drv_sbpm_default_val_init(void)
{
    uint32_t sp_high = 0, sp_low = 0, ug_high = 0, ug_low = 0;

    sp_low = RU_FIELD_SET(0, SBPM, REGS_SBPM_SP_RNR_LOW, SBPM_SP_RNR_LOW, sp_low, SBPM_SP_RNR_LOW_INIT_VAL);
    sp_high = RU_FIELD_SET(0, SBPM, REGS_SBPM_SP_RNR_HIGH, SBPM_SP_RNR_HIGH, sp_high, SBPM_SP_RNR_HIGH_INIT_VAL);
    ug_low = RU_FIELD_SET(0, SBPM, REGS_SBPM_UG_MAP_LOW, SBPM_UG_MAP_LOW, ug_low, SBPM_UG_MAP_LOW_INIT_VAL);
    ug_high = RU_FIELD_SET(0, SBPM, REGS_SBPM_UG_MAP_HIGH, SBPM_UG_MAP_HIGH, ug_high, SBPM_UG_MAP_HIGH_INIT_VAL);

    RU_REG_WRITE(0, SBPM, REGS_SBPM_SP_RNR_LOW, sp_low);
    RU_REG_WRITE(0, SBPM, REGS_SBPM_SP_RNR_HIGH, sp_high);
    RU_REG_WRITE(0, SBPM, REGS_SBPM_UG_MAP_LOW, ug_low);
    RU_REG_WRITE(0, SBPM, REGS_SBPM_UG_MAP_HIGH, ug_high);
}

#define INVALID_CHIP_ID 0xffffffff
unsigned int UtilGetChipRev(void)
{
#if defined(CONFIG_BCM96878)
    return 0xB0;
#endif 
    return INVALID_CHIP_ID;
}

