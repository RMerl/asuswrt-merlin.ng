/***********************************************************************/
/*                                                                     */
/*   MODULE:  merlin_direct_access.c                                   */
/*   DATE:    01/03/2016                                               */
/*   PURPOSE: Callback functions for Lport                             */
/*                                                                     */
/***********************************************************************/


#ifdef _CFE_
#include "lib_types.h"
#include "lib_printf.h"
#else
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/string.h>
#include <linux/delay.h>
#endif

#include "lport_defs.h"
#include "lport_drv.h"
#include "serdes_access.h"
/* Microcode */
#include "merlin_direct_ucode_image.h"

#ifndef _CFE_
#include <asm/delay.h>
#define UDELAY(_a) udelay(_a)
#else
extern void cfe_usleep(int usec);
#define UDELAY(_a) cfe_usleep(_a)
#endif

#ifdef MERLIN_DEBUG
#ifndef _CFE_
#define MERLIN_DBG  printk
#else
#define MERLIN_DBG  xprintf
#endif
#else
#define MERLIN_DBG(...)
#endif

#ifndef _CFE_
#define MERLIN_ERR  printk
#else
#define MERLIN_ERR  xprintf
#endif

#define MERLIN_INFO MERLIN_ERR

#define MERLIN_LANES_PER_CORE   2
#define MERLIN_MAX_CORE_NUMBER  2
#define MERLIN_MAX_LANE_NUMBER  MERLIN_LANES_PER_CORE*MERLIN_MAX_CORE_NUMBER
/*RMIC/PMI address*/
#define MMD_TYPE_OFFSET      27
#define LANE_ADDRESS_OFFSET  16

#define MASK_BIT(a) (1<<(a))
#define MASK_BITS_TO(x) ((1<<(x))-1)
#define MASK_BITS_BETWEEN(l, h) (MASK_BITS_TO(((h)+1)) & ~(MASK_BITS_TO(l)))
#define MASK_ALL_BITS_16 (0xFFFF)

#define MARLIN_SET_BIT(reg, posn)    ((reg) | (1L << (posn)))
#define MARLIN_CLEAR_BIT(reg, posn)  ((reg) & ~(1L << (posn)))

#define RAM_CORE_BASE   0x0050
#define RAM_LANE_BASE   0x0400
#define RAM_CORE_SIZE   0x0040
#define RAM_LANE_SIZE   0x0100

static int merlin_direct_tx_cfg_set(E_MERLIN_LANE lane_index, merlin_tx_cfg_t *tx_cfg);

static inline void MERLIN_READ_REG(uint16_t dev_type, E_MERLIN_LANE lane_id, uint16_t reg_addr, uint16_t mask, uint16_t shift, uint16_t *value)
{
    uint32_t addr = 0;

    addr = ((dev_type) << MMD_TYPE_OFFSET) | (((uint32_t)(lane_id % MERLIN_LANES_PER_CORE)) << LANE_ADDRESS_OFFSET) | reg_addr;
    read_serdes_reg(lane_id / MERLIN_LANES_PER_CORE, addr, mask, value);
    *value >>= shift;
}

static inline void MERLIN_WRITE_REG(uint16_t dev_type, E_MERLIN_LANE lane_id, uint16_t reg_addr, uint16_t mask, uint16_t shift, uint16_t value)
{
    uint32_t addr = 0;

    addr = ((dev_type) << MMD_TYPE_OFFSET) | (((uint32_t)(lane_id % MERLIN_LANES_PER_CORE)) << LANE_ADDRESS_OFFSET) | reg_addr;
    write_serdes_reg(lane_id / MERLIN_LANES_PER_CORE, addr, mask, value << shift);
}

static inline void MERLIN_RAM_ADDR(E_MERLIN_LANE lane_id, uint16_t size, uint16_t addr)
{
    if (lane_id % MERLIN_LANES_PER_CORE)
        addr += RAM_LANE_SIZE;

    MERLIN_WRITE_REG(0x1, lane_id, 0xd202, MASK_BITS_BETWEEN(7, 8), 7, 0x2); /* ram memory access */
    MERLIN_WRITE_REG(0x1, lane_id, 0xd202, MASK_BITS_BETWEEN(9, 9), 9, size > 1 ? 0x00 : 0x01); /* access mode word/byte */
    MERLIN_WRITE_REG(0x1, lane_id, 0xd201, MASK_BITS_BETWEEN(0, 15), 0, addr); /* ram address */
}

static inline void MERLIN_READ_RAM(E_MERLIN_LANE lane_id, uint16_t size, uint16_t addr, uint16_t *value)
{
    MERLIN_RAM_ADDR(lane_id, size, addr);
    MERLIN_READ_REG(0x1, lane_id, 0xd204, MASK_BITS_BETWEEN(0, 15), 0, value); /* read ram data */

    if (size > 1)
        *value = ((*value & 0xff) << 8) | (*value >> 8);
}

static inline void MERLIN_WRITE_RAM(E_MERLIN_LANE lane_id, uint16_t size, uint16_t addr, uint16_t value)
{
    if (size > 1)
        value = ((value & 0xff) << 8) | (value >> 8);

    MERLIN_RAM_ADDR(lane_id, size, addr);
    MERLIN_WRITE_REG(0x1, lane_id, 0xd203, MASK_BITS_BETWEEN(0, 15), 0, value); /* write ram data */
}

static int merlin_direct_microcode_load(E_MERLIN_ID merlin_id, uint8_t *code_image, uint16_t code_len)
{
    uint32_t lane_id;
    uint16_t is_init_done;
    uint16_t padded_len, data, count = 0;
    uint8_t lsb_data;
    uint16_t load_result0 = 0;
    uint16_t load_result1 = 0;

    lane_id = merlin_id * MERLIN_LANES_PER_CORE;

    MERLIN_WRITE_REG(0x1, lane_id, 0xd0f2, MASK_BIT(6), 6, 0x1); /*uc_active*/
    MERLIN_WRITE_REG(0x1, lane_id, 0xd0f2, MASK_BIT(0), 0, 0x1); /*core_dp_s_rstb*/
    MERLIN_WRITE_REG(0x1, lane_id, 0xd20d, MASK_BIT(1), 1, 0x1); /*micro_system_reset_n*/
    MERLIN_WRITE_REG(0x1, lane_id, 0xd20d, MASK_BIT(0), 0, 0x1); /*micro_system_clk_en*/
    MERLIN_WRITE_REG(0x1, lane_id, 0xd202, MASK_BIT(15), 15, 0x1); /*init_cmd*/

    UDELAY(10000);

    MERLIN_READ_REG(0x1, lane_id, 0xd205, MASK_BIT(15), 15, &is_init_done);
    if (!is_init_done)
    {
        MERLIN_ERR("Failed to init serdes %d engine\n", merlin_id);
        return -1;
    }

    padded_len = (code_len + 7) & 0xfff8 ; /* number of bytes to be writen, have to be multiple of 4 */ 

    MERLIN_WRITE_REG(0x1, lane_id, 0xd200, MASK_ALL_BITS_16, 0, padded_len - 1); /*ram_cnt*/
    MERLIN_WRITE_REG(0x1, lane_id, 0xd201, MASK_ALL_BITS_16, 0, 0x0); /*ram_addr*/
    MERLIN_WRITE_REG(0x1, lane_id, 0xd202, MASK_BIT(1), 1, 0x0); /*stop*/
    MERLIN_WRITE_REG(0x1, lane_id, 0xd202, MASK_BIT(3), 3, 0x1); /*write */
    MERLIN_WRITE_REG(0x1, lane_id, 0xd202, MASK_BIT(0), 0, 0x1); /*run*/
    for (count = 0; count < padded_len; count+=2)
    {
        lsb_data = (count < code_len) ? code_image[count] : 0x0;
        data = ((count + 1) < code_len) ? code_image[count + 1] : 0x0;
        data = (data << 8) | lsb_data;

        MERLIN_WRITE_REG(0x1, lane_id, 0xd203, MASK_ALL_BITS_16, 0, data);
    }

    MERLIN_WRITE_REG(0x1, lane_id, 0xd202, MASK_BIT(3), 3, 0x0); /*write */
    MERLIN_WRITE_REG(0x1, lane_id, 0xd202, MASK_BIT(0), 0, 0x0); /*run*/
    MERLIN_WRITE_REG(0x1, lane_id, 0xd202, MASK_BIT(1), 1, 0x1); /* stop */
    /* Verify microcode load */
    MERLIN_READ_REG(0x1, lane_id, 0xd205, MASK_BIT(0), 0, &load_result0);
    MERLIN_READ_REG(0x1, lane_id, 0xd205, MASK_BIT(1), 1, &load_result1);
    if (load_result0 || load_result1)
    {
        MERLIN_ERR("Failed load firmware ERR0: %d ERR1: %d\n", load_result0, load_result1);
        return -2;
    }

    MERLIN_WRITE_REG(0x1, lane_id, 0xd202, MASK_BIT(4), 4, 0x1); /* mdio_dw8051_reset_n */
    UDELAY(10000);

    return 0;
}

static int merlin_direct_core_init(E_MERLIN_ID merlin_id, E_MERLIN_VCO pll)
{
    int rc;
    uint32_t lane_id;

    rc = merlin_direct_microcode_load(merlin_id, merlin_mptwo_ucode_image, MERLIN_MPTWO_UCODE_IMAGE_SIZE);
    if (rc)
    {
        MERLIN_ERR("Failed to load microcode to serdes%d %d\n",merlin_id, rc);
        return rc;
    }

    lane_id = merlin_id * MERLIN_LANES_PER_CORE;

    MERLIN_WRITE_REG(0x1, lane_id, 0xffdc, MASK_BITS_BETWEEN(0, 4), 0, 0xa); /* broadcast address */
    /* PMD Setup 50MHz */
    MERLIN_WRITE_REG(0x1, lane_id, 0xd0b9, MASK_BIT(0), 0, 0x0); /* mmd_resetb */
    MERLIN_WRITE_REG(0x1, lane_id, 0xd0b3, MASK_BITS_BETWEEN(11, 15), 11, 0x4); /* div */
    MERLIN_WRITE_REG(0x1, lane_id, 0xd0b6, MASK_BITS_BETWEEN(12, 15), 12, 0x0); /* i_ndiv_frac_l */
    MERLIN_WRITE_REG(0x1, lane_id, 0xd0b7, MASK_BITS_BETWEEN(0, 13), 0, 0x1000); /* i_ndiv_frac_h */
    MERLIN_WRITE_REG(0x1, lane_id, 0xd0b8, MASK_BIT(14), 14, 0x1); /* i_pll_sdm_pwrdnb */
    MERLIN_WRITE_REG(0x1, lane_id, 0xd0b8, MASK_BIT(13), 13, 0x1); /* mmd_en */
    MERLIN_WRITE_REG(0x1, lane_id, 0xd0b8, MASK_BIT(12), 12, 0x0); /* mmd_prsc4or5pwdb */
    MERLIN_WRITE_REG(0x1, lane_id, 0xd0b8, MASK_BIT(11), 11, 0x1); /* mmd_prsc8or9pwdb */
    MERLIN_WRITE_REG(0x1, lane_id, 0xd0b8, MASK_BIT(10), 10, 0x1); /* mmd_div_range */
    MERLIN_WRITE_REG(0x1, lane_id, 0xd0b8, MASK_BITS_BETWEEN(0, 9), 0, 0xce); /* i_ndiv_int */
    MERLIN_WRITE_REG(0x1, lane_id, 0xd0b9, MASK_BITS_BETWEEN(0, 2), 0, 0x3); /* i_pll_frac_mode, mmd_resetb */
    MERLIN_WRITE_REG(0x3, lane_id, 0x9100, MASK_BITS_BETWEEN(13, 15), 13, 0x6); /* refclk_sel */

    /* AN Timer Speed Up */
    MERLIN_WRITE_REG(0x3, lane_id, 0x9107, MASK_BITS_BETWEEN(15,15), 15, 0x00); /* tick_override */
    MERLIN_WRITE_REG(0x3, lane_id, 0x9250, MASK_BITS_BETWEEN(0,15), 0, 0x029a); /* cl73_restart_timer */
    MERLIN_WRITE_REG(0x3, lane_id, 0x9251, MASK_BITS_BETWEEN(0,15), 0, 0x029a); /* cl37_ack_timer */
    MERLIN_WRITE_REG(0x3, lane_id, 0x9252, MASK_BITS_BETWEEN(0,15), 0, 0xa000); /* cl37_error_timer */
    MERLIN_WRITE_REG(0x3, lane_id, 0x9253, MASK_BITS_BETWEEN(0,15), 0, 0x10ee); /* cl73_break_link_timer */
    MERLIN_WRITE_REG(0x3, lane_id, 0x9254, MASK_BITS_BETWEEN(0,15), 0, 0xa000); /* cl73_error_timer */
    MERLIN_WRITE_REG(0x3, lane_id, 0x9255, MASK_BITS_BETWEEN(0,15), 0, 0x0bb8); /* cl73_pd_dme_lock_timer */
    MERLIN_WRITE_REG(0x3, lane_id, 0x9256, MASK_BITS_BETWEEN(0,15), 0, 0x1b00); /* cl73_link_up_timer */
    MERLIN_WRITE_REG(0x3, lane_id, 0x9257, MASK_BITS_BETWEEN(0,15), 0, 0x8235); /* link_fail_inhibit_timer_cl72 */
    MERLIN_WRITE_REG(0x3, lane_id, 0x9258, MASK_BITS_BETWEEN(0,15), 0, 0x8235); /* link_fail_inhibit_timer_ncl72 */
    MERLIN_WRITE_REG(0x3, lane_id, 0x925a, MASK_BITS_BETWEEN(0,15), 0, 0x8235); /* cl72_max_wait_timer */
    MERLIN_WRITE_REG(0x3, lane_id, 0x925c, MASK_BITS_BETWEEN(0,15), 0, 0x000f); /* ignore_link_timer_period */

    /* Datapath Reset */
    MERLIN_WRITE_REG(0x1, lane_id, 0xd0f2, MASK_BIT(0), 0, 0x1); /* core_dp_s_rstb */
    UDELAY(10000);

    /* afe_hw_version */
    MERLIN_WRITE_RAM(lane_id, 1, RAM_CORE_BASE + 0x0f, 0x1);

    return 0;
}

#pragma pack(push,1)
typedef struct {
    uint16_t lane_cfg_from_pcs:1;
    uint16_t an_enabled:1;
    uint16_t dfe_on:1;
    uint16_t force_brdfe_on:1;
    uint16_t media_type:2;
    uint16_t unreliable_los:1;
    uint16_t scrambling_dis:1;
    uint16_t cl72_auto_polarity_en:1;
    uint16_t cl72_restart_timeout_en:1;
    uint16_t reserved:6;
} lane_config_t;
#pragma pack(pop)

static int merlin_direct_lane_init(E_MERLIN_LANE lane_id, LPORT_PORT_MUX_SELECT mux_sel)
{
    merlin_tx_cfg_t tx_cfg = {};
    lane_config_t lane_conf = {};

    /* Change Speed */
    MERLIN_WRITE_REG(0x1, lane_id, 0xd082, MASK_BITS_BETWEEN(0, 15), 0, 0x0000);
    MERLIN_WRITE_REG(0x3, lane_id, 0x0000, MASK_BIT(15), 15, 0x1); /* lane reset */
    MERLIN_WRITE_REG(0x1, lane_id, 0xd081, MASK_BIT(1), 1, 0x0); /* ln_dp_s_rstb */
    MERLIN_WRITE_REG(0x1, lane_id, 0xd0e3, MASK_BIT(1), 1, 0x1); /* byppass pmd tx oversampling */

    switch (mux_sel)
    {
    case PORT_SGMII:
        MERLIN_WRITE_REG(0x3, lane_id, 0xc30b, MASK_BIT(11), 11, 0x0); /* credit_sw_en */
        MERLIN_WRITE_REG(0x3, lane_id, 0xc30b, MASK_BIT(6), 6, 0x1); /* SW_actual_speed_force_en */
        MERLIN_WRITE_REG(0x3, lane_id, 0xc30b, MASK_BITS_BETWEEN(0, 5), 0, 0x2); /* SW_actual_speed */
        break; 
    case PORT_SGMII_AN_IEEE_CL37:
        MERLIN_WRITE_REG(0x3, lane_id, 0xc30b, MASK_BITS_BETWEEN(12,13), 12, 0x1); /* use_ieee_reg_ctrl_sel */
        MERLIN_WRITE_REG(0x0, lane_id, 0x0004, MASK_BIT(5), 5, 0x1); /* duplex */
        MERLIN_WRITE_REG(0x0, lane_id, 0x0000, MASK_BIT(12), 12, 0x1); /* AN enable */
        MERLIN_WRITE_REG(0x3, lane_id, 0xc30b, MASK_BIT(11), 11, 0x0); /* credit_sw_en */
        MERLIN_WRITE_REG(0x3, lane_id, 0xc30b, MASK_BIT(6), 6, 0x0); /* SW_actual_speed_force_en */
        break;
    case PORT_SGMII_AN_USER_CL37:
        MERLIN_WRITE_REG(0x3, lane_id, 0xc30b, MASK_BITS_BETWEEN(12,13), 12, 0x2); /*use_ieee_reg_ctrl_sel */
        MERLIN_WRITE_REG(0x3, lane_id, 0xc481, MASK_BIT(4), 4, 0x1); /* duplex */
        MERLIN_WRITE_REG(0x3, lane_id, 0xc480, MASK_BIT(6), 6, 0x1); /* AN enable */
        MERLIN_WRITE_REG(0x3, lane_id, 0xc30b, MASK_BIT(11), 11, 0x0); /* credit_sw_en */
        MERLIN_WRITE_REG(0x3, lane_id, 0xc30b, MASK_BIT(6), 6, 0x0); /* SW_actual_speed_force_en */
        break;
    case PORT_SGMII_AN_MASTER:
        MERLIN_WRITE_REG(0x3, lane_id, 0xc30b, MASK_BITS_BETWEEN(12,13), 12, 0x2); /* use_ieee_reg_ctrl_sel */
        MERLIN_WRITE_REG(0x3, lane_id, 0xc481, MASK_BIT(2), 2, 0x1); /* duplex */
        MERLIN_WRITE_REG(0x3, lane_id, 0xc481, MASK_BITS_BETWEEN(0,1), 0, 0x2); /* speed */
        MERLIN_WRITE_REG(0x3, lane_id, 0xc481, MASK_BIT(9), 9, 0x1); /* mater */
        MERLIN_WRITE_REG(0x3, lane_id, 0xc480, MASK_BITS_BETWEEN(6,7), 6, 0x3); /* AN enable */
        MERLIN_WRITE_REG(0x3, lane_id, 0xc30b, MASK_BIT(11), 11, 0x0); /* credit_sw_en */
        MERLIN_WRITE_REG(0x3, lane_id, 0xc30b, MASK_BIT(6), 6, 0x0); /* SW_actual_speed_force_en */
        break;
    case PORT_SGMII_AN_SLAVE:
        MERLIN_WRITE_REG(0x3, lane_id, 0xc30b, MASK_BITS_BETWEEN(12,13), 12, 0x2); /* use_ieee_reg_ctrl_sel */
        MERLIN_WRITE_REG(0x3, lane_id, 0xc480, MASK_BITS_BETWEEN(6,7), 6, 0x3); /* AN enable */
        MERLIN_WRITE_REG(0x3, lane_id, 0xc30b, MASK_BIT(11), 11, 0x0); /* credit_sw_e n*/
        MERLIN_WRITE_REG(0x3, lane_id, 0xc30b, MASK_BIT(6), 6, 0x0); /* SW_actual_speed_force_en */
        break;
    case PORT_HSGMII:
        MERLIN_WRITE_REG(0x3, lane_id, 0xc30b, MASK_BIT(11), 11, 0x0); /* credit_sw_en */
        MERLIN_WRITE_REG(0x3, lane_id, 0xc30b, MASK_BIT(6), 6, 0x1); /* SW_actual_speed_force_en */
        MERLIN_WRITE_REG(0x3, lane_id, 0xc30b, MASK_BITS_BETWEEN(0, 5), 0, 0x3); /* SW_actual_speed */
        MERLIN_WRITE_REG(0x3, lane_id, 0x9270, MASK_BITS_BETWEEN(0, 15), 0, 0x0021); /* Reg2p5G_ClockCount0 */
        MERLIN_WRITE_REG(0x3, lane_id, 0x9233, MASK_BITS_BETWEEN(0, 15), 0, 0x0002); /* Reg2p5G_modulo */
        break;
    case PORT_SFI:
    case PORT_XFI:
        MERLIN_WRITE_REG(0x3, lane_id, 0xc30b, MASK_BIT(11), 11, 0x0); /* credit_sw_en */
        MERLIN_WRITE_REG(0x3, lane_id, 0xc30b, MASK_BIT(6), 6, 0x1); /* SW_actual_speed_force_en */
        MERLIN_WRITE_REG(0x3, lane_id, 0xc30b, MASK_BITS_BETWEEN(0, 5), 0, 0xf); /* SW_actual_speed */
        break;
    default:
        MERLIN_WRITE_REG(0x3, lane_id, 0xc457, MASK_BITS_BETWEEN(0, 15), 0, 0x0000);
        MERLIN_WRITE_REG(0x3, lane_id, 0xc30b, MASK_BITS_BETWEEN(0, 15), 0, 0x1002);
        MERLIN_WRITE_REG(0x3, lane_id, 0xc433, MASK_BITS_BETWEEN(0, 15), 0, 0x01c8);
        MERLIN_WRITE_REG(0x1, lane_id, 0xd081, MASK_BITS_BETWEEN(0, 15), 0, 0x0001);
        MERLIN_WRITE_REG(0x1, lane_id, 0xd082, MASK_BITS_BETWEEN(0, 15), 0, 0x0033);
        return 0;
    }

    MERLIN_WRITE_REG(0x3, lane_id, 0xc457, MASK_BIT(0), 0, 0x1); /* rx_rstb_lane */
    MERLIN_WRITE_REG(0x3, lane_id, 0xc30b, MASK_BIT(7), 7, 0x1); /* mac_creditenable */
    MERLIN_WRITE_REG(0x3, lane_id, 0xc433, MASK_BIT(1), 1, 0x1); /* tx_rstb_lane */
    MERLIN_WRITE_REG(0x3, lane_id, 0xc433, MASK_BIT(0), 0, 0x1); /* enable_tx_lane */

    /* LPI pass-through */
    MERLIN_WRITE_REG(0x3, lane_id, 0xc450, MASK_BIT(2), 2, 0x1); /* lpi_enable */

    /* Transmitter Configuration */
    switch (mux_sel)
    {
    case PORT_SGMII:
    case PORT_SGMII_AN_MASTER:
    case PORT_SGMII_AN_SLAVE:
        tx_cfg = (merlin_tx_cfg_t){ 0, 40, 0, 0, 1, 1, 1, 1, 4, 0 };
        break;
    case PORT_SGMII_AN_IEEE_CL37:
    case PORT_SGMII_AN_USER_CL37:
        tx_cfg = (merlin_tx_cfg_t){ 2, 44, 11, 2, 1, 1, 1, 1, 5, 0 };
        break;
    case PORT_HSGMII:
        tx_cfg = (merlin_tx_cfg_t){ 0, 60, 0, 0, 1, 1, 1, 1, 5, 4 };
        break;
    case PORT_SFI:
        tx_cfg = (merlin_tx_cfg_t){ 2, 44, 11, 2, 0, 1, 1, 1, 5, 0 };
        break;
    case PORT_XFI:
        tx_cfg = (merlin_tx_cfg_t){ 0, 46, 8, 0, 0, 1, 1, 1, 5, 0 };
        break;
    default:
       return -1;
    }

    merlin_direct_tx_cfg_set(lane_id, &tx_cfg);

    /* Lane configuration (RAM Variables) */
    MERLIN_READ_RAM(lane_id, 2, RAM_LANE_BASE + 0x00, (uint16_t *)&lane_conf);

    switch (mux_sel)
    {
    case PORT_SGMII:
        break; 
    case PORT_SGMII_AN_MASTER:
    case PORT_SGMII_AN_SLAVE:
        lane_conf.an_enabled = 1;
        lane_conf.lane_cfg_from_pcs = 1;
        break;
    case PORT_SGMII_AN_IEEE_CL37:
    case PORT_SGMII_AN_USER_CL37:
        lane_conf.media_type = 2;
        lane_conf.an_enabled = 1;
        lane_conf.lane_cfg_from_pcs = 1;
        break;
    case PORT_HSGMII:
        break;
    case PORT_XFI:
        break;
    case PORT_SFI:
        lane_conf.dfe_on = 1;
        lane_conf.media_type = 2;
        lane_conf.unreliable_los = 0;
        break;
    default:
        return -1;
    }

    MERLIN_WRITE_RAM(lane_id, 2, RAM_LANE_BASE + 0x00, *(uint16_t *)&lane_conf);

    /* Datapath Reset */
    MERLIN_WRITE_REG(0x1, lane_id, 0xd081, MASK_BIT(1), 1, 0x1); /* ln_dp_s_rstb */
    UDELAY(10000);

    return 0;
}

static int merlin_direct_get_status(E_MERLIN_LANE lane_id, lport_port_status_s *port_status)
{
    uint16_t port_speed;
    uint16_t port_up;

    MERLIN_READ_REG(0x3, lane_id, 0xc474, MASK_BIT(1), 1, &port_up);
    MERLIN_READ_REG(0x3, lane_id, 0xc30b, MASK_BITS_BETWEEN(0, 5), 0, &port_speed);
    port_status->port_up = port_up;
    switch (port_speed)
    {
    case 0:
        port_status->rate = LPORT_RATE_10MB;
        break;
    case 1:
        port_status->rate = LPORT_RATE_100MB;
        break;
    case 2:
        port_status->rate = LPORT_RATE_1000MB;
        break;
    case 3:
        port_status->rate = LPORT_RATE_2500MB;
        break;
    case 4:
        port_status->rate = LPORT_RATE_2500MB;
        break;
    case 0xD:
    case 0x34:
        port_status->rate = LPORT_RATE_1000MB;
        break;
    case 0xf:
    case 0x1b:
    case 0x1f:
    case 0x33:
        port_status->rate = LPORT_RATE_10G;
        break;
    default:
        port_status->rate = LPORT_RATE_UNKNOWN;
        break;
    }
    port_status->duplex = LPORT_FULL_DUPLEX; 
    return 0;
}

static int merlin_direct_speed_set(E_MERLIN_LANE lane_id, LPORT_PORT_RATE port_rate)
{
    MERLIN_WRITE_REG(0x1, lane_id, 0xd081, MASK_BIT(1), 1, 0x0); /* ln_dp_s_rstb */
    UDELAY(10000);

    switch (port_rate)
    {
    case LPORT_RATE_10G:
        MERLIN_WRITE_REG(0x3, lane_id, 0xc30b, MASK_BITS_BETWEEN(0, 5), 0, 0xf); /* SW_actual_speed */
        break;
    case LPORT_RATE_2500MB:
        MERLIN_WRITE_REG(0x3, lane_id, 0xc30b, MASK_BITS_BETWEEN(0, 5), 0, 0x3); /* SW_actual_speed */
        break;
    case LPORT_RATE_1000MB:
        MERLIN_WRITE_REG(0x3, lane_id, 0xc30b, MASK_BITS_BETWEEN(0, 5), 0, 0x2); /* SW_actual_speed */
        break;
    case LPORT_RATE_100MB:
        MERLIN_WRITE_REG(0x3, lane_id, 0xc30b, MASK_BITS_BETWEEN(0, 5), 0, 0x1); /* SW_actual_speed */
        break;
    default:
        MERLIN_ERR("Trying to set invalid port speed %d \n", port_rate);
        return -1;
        break;
    }

    MERLIN_WRITE_REG(0x1, lane_id, 0xd081, MASK_BIT(1), 1, 0x1); /* ln_dp_s_rstb */
    UDELAY(10000);

    return 0;
}

static int merlin_direct_lane_status_get(uint16_t lane_index, merlin_status_t *status)
{
    MERLIN_READ_REG(3, lane_index, 0xc441, MASK_BIT(8), 8, &status->tx_LOCAL_FAULT);
    MERLIN_READ_REG(3, lane_index, 0xc441, MASK_BIT(7), 7, &status->tx_REMOTE_FAULT);
    MERLIN_READ_REG(3, lane_index, 0xc470, MASK_BIT(3), 3, &status->PMD_LOCK);
    MERLIN_READ_REG(3, lane_index, 0xc470, MASK_BIT(0), 0, &status->signal_ok);
    MERLIN_READ_REG(3, lane_index, 0xc474, MASK_BIT(8), 8, &status->rx_LOCAL_FAULT);
    MERLIN_READ_REG(3, lane_index, 0xc474, MASK_BIT(7), 7, &status->rx_REMOTE_FAULT);
    MERLIN_READ_REG(3, lane_index, 0xc474, MASK_BIT(1), 1, &status->rx_LINK_STATUS);
    MERLIN_READ_REG(3, lane_index, 0xc474, MASK_BIT(0), 0, &status->rx_SYNC_STATUS);
    MERLIN_READ_REG(1, lane_index, 0xd128, MASK_BIT(9), 9, &status->pll_lock);

    return 0;
}

static int merlin_direct_lane_stats_get(uint16_t lane_index, merlin_stats_t *stats)
{
    MERLIN_READ_REG(3, lane_index, 0xc468, MASK_BITS_BETWEEN(0, 7), 0, &stats->kcode66ErrCount);
    MERLIN_READ_REG(3, lane_index, 0xc468, MASK_BITS_BETWEEN(8, 15), 8, &stats->sync66ErrCount);
    MERLIN_READ_REG(3, lane_index, 0xc469, MASK_BITS_BETWEEN(2, 9), 2, &stats->cl49ieee_errored_blocks);
    MERLIN_READ_REG(3, lane_index, 0xc46e, MASK_BITS_BETWEEN(0, 7), 0, &stats->BER_count_per_ln);
    MERLIN_READ_REG(3, lane_index, 0x9227, MASK_BITS_BETWEEN(0, 7), 0, &stats->cl49_invalid_sh_cnt);
    MERLIN_READ_REG(3, lane_index, 0x9227, MASK_BITS_BETWEEN(8, 15), 8, &stats->cl49_valid_sh_cnt);

    return 0;
}

static int merlin_direct_lane_prbs_stats_get(uint16_t lane_index, merlin_prbs_stats_t *stats)
{
    uint16_t reg_lo, reg_hi;

    reg_lo = 0;
    MERLIN_READ_REG(1, lane_index, 0xD0D9, MASK_ALL_BITS_16, 0, &reg_lo);
    stats->prbs_lock_status = (reg_lo & 0x1);//1- PRBS checker is locked 

    reg_lo = reg_hi = 0; 
    MERLIN_READ_REG(1, lane_index, 0xD0DA, MASK_ALL_BITS_16, 0, &reg_hi);
    MERLIN_READ_REG(1, lane_index, 0xD0DB, MASK_ALL_BITS_16, 0, &reg_lo);

    stats->prbs_lock_lost =  (uint16_t)(reg_hi >> 15);

    // Clear bit 15 it is prbs lock lost status bit;
    reg_hi = MARLIN_CLEAR_BIT(reg_hi, 15);

    stats->prbs_error = ((uint32_t)(reg_hi) << 16) | (uint32_t)reg_lo;

    return 0;
}

static int merlin_direct_lane_prbs_generate(uint16_t lane_index, uint16_t* p_prbs_type)
{
    uint32_t prog_patten_gen_mode = 
        (*p_prbs_type == SERDES_PRBS_PATTERN_TYPE_8180) ? 1 : 0; //bit pattern 8 ones / 8 zeros 
    uint16_t reg_val = 1;//Enable prbs_generator(bit 0)

    if (prog_patten_gen_mode)
    {
        /* In case that prbs generator was enable  */
        MERLIN_WRITE_REG(1, lane_index, 0xD0E1, MASK_BIT(0), 0, 0x0);

        /*
         * Default fixed pattern  8 ones / 8 zeros
         * Enable fix pattern + default fixed pattern
         */
        MERLIN_WRITE_REG(1, lane_index, 0xD0E0, MASK_BITS_BETWEEN(0, 15), 0, 0x3);  
    }
    else
    {
        /* Disable fixed pattern */
        MERLIN_WRITE_REG(1, lane_index, 0xD0E0, MASK_BIT(0), 0, 0x0);

        reg_val |= (*p_prbs_type << 1);
        /* Enable prbs_generator(bit 0) with prbs type: PRBS7|PRBS9|PRBS11|PRBS15|PRBS23|PRBS31|PRBS58 */
        MERLIN_WRITE_REG(1, lane_index, 0xD0E1, MASK_BITS_BETWEEN(0,3), 0, reg_val);
    } 	

    return 0;
}


static int merlin_direct_lane_prbs_monitor(uint16_t lane_index, uint16_t* p_prbs_type)
{
    uint16_t reg_val = 1;//Enable prbs_generator(bit 0)

    reg_val |= (*p_prbs_type << 1);

    /* Enable prbs_generator control with prbs type  //PRBS7|PRBS9|PRBS11|PRBS15|PRBS23|PRBS31|PRBS58 + prbs_en bit 0 */
    MERLIN_WRITE_REG(1, lane_index, 0xD0D1, MASK_BITS_BETWEEN(0,3), 0, reg_val);

    return 0;
}


static int merlin_direct_core_id_get(uint16_t lane_index, merlin_serdes_id_t *id)
{
    MERLIN_READ_REG(3, lane_index, 0x910e, MASK_BITS_BETWEEN(14, 15), 14, &id->rev_letter);
    MERLIN_READ_REG(3, lane_index, 0x910e, MASK_BITS_BETWEEN(11, 13), 11, &id->rev_number);
    MERLIN_READ_REG(3, lane_index, 0x910e, MASK_BITS_BETWEEN(9, 10), 9, &id->bonding);
    MERLIN_READ_REG(3, lane_index, 0x910e, MASK_BITS_BETWEEN(6, 8), 6, &id->tech_proc);
    MERLIN_READ_REG(3, lane_index, 0x910e, MASK_BITS_BETWEEN(0, 5), 0, &id->model_number);
    return 0;
}

static int merlin_direct_loopback_set(E_MERLIN_LANE lane_index, merlin_loopback_set_t *in)
{
    int ret = 0;
    uint16_t lane_map;

    switch(in->mode)
    {
    case MERLIN_PCS_LOCAL_LOOPBACK:
        MERLIN_READ_REG(3, lane_index, 0x9109, MASK_BITS_BETWEEN(4, 7), 4, &lane_map);
        if(!in->enable)
            lane_map |= 1<<lane_index;
        else
            lane_map &= ~(1<<lane_index);
        MERLIN_WRITE_REG(1, lane_index, 0xd0c1, MASK_BIT(7), 7, (in->enable != 0));
        MERLIN_WRITE_REG(3, lane_index, 0x9109, MASK_BITS_BETWEEN(4, 7), 4, lane_map);
        break;
    case MERLIN_PCS_REMOTE_LOOPBACK:
        MERLIN_READ_REG(3, lane_index, 0x9109, MASK_BITS_BETWEEN(12, 15), 12, &lane_map);
        if(!in->enable)
            lane_map |= 1<<lane_index;
        else
            lane_map &= ~(1<<lane_index);
        MERLIN_WRITE_REG(3, lane_index, 0x9109, MASK_BITS_BETWEEN(12, 15), 12, lane_map);
        break;
    case MERLIN_PMD_LOCAL_LOOPBACK:
        MERLIN_WRITE_REG(1, lane_index, 0xd0c1, MASK_BIT(7), 7, (in->enable != 0));
        MERLIN_WRITE_REG(1, lane_index, 0xd0d2, MASK_BIT(0), 0, in->enable);
        break;
    case MERLIN_PMD_REMOTE_LOOPBACK:
        if (in->enable)
        {
            /*locks Loop timing */
            MERLIN_WRITE_REG(1, lane_index, 0xd075, MASK_BIT(2), 2, 0);
            MERLIN_WRITE_REG(1, lane_index, 0xd070, MASK_BIT(0), 0, 1);
            MERLIN_WRITE_REG(1, lane_index, 0xd070, MASK_BIT(1), 1, 1);
            UDELAY(25);
            MERLIN_WRITE_REG(1, lane_index, 0xd070, MASK_BIT(2), 2, 1);
            MERLIN_WRITE_REG(1, lane_index, 0xd0e2, MASK_BIT(0), 0, 1);
            UDELAY(50);
        }
        else
        {
            MERLIN_WRITE_REG(1, lane_index, 0xd0e2, MASK_BIT(0), 0, 0);
            MERLIN_WRITE_REG(1, lane_index, 0xd070, MASK_BIT(2), 2, 0);
            MERLIN_WRITE_REG(1, lane_index, 0xd070, MASK_BIT(1), 1, 0);
            MERLIN_WRITE_REG(1, lane_index, 0xd070, MASK_BIT(0), 0, 0);
            MERLIN_WRITE_REG(1, lane_index, 0xd075, MASK_BIT(2), 2, 1);
        }
        break;
    default:
        MERLIN_ERR("loopback mode %d is not supported\n", in->mode);
        ret = -1;
        break;
    }

    return ret;
}

static int merlin_direct_tx_cfg_set(E_MERLIN_LANE lane_index, merlin_tx_cfg_t *tx_cfg)
{
    uint32_t parameters_valid = 0;

    if ((tx_cfg->pre > 10) || (tx_cfg->pre < 0))
        MERLIN_ERR("ERR_CODE_TXFIR_PRE_INVALID\n");
    else if ((tx_cfg->main > 60) || (tx_cfg->main < 0))
        MERLIN_ERR("ERR_CODE_TXFIR_MAIN_INVALID\n");
    else if (((tx_cfg->post2 != 0) && (tx_cfg->post1 > 18)) || (tx_cfg->post1 < 0))
        MERLIN_ERR("ERR_CODE_TXFIR_POST1_POST2_INVALID\n");
    else if (((tx_cfg->post2 == 0) && (tx_cfg->post1 > 23)) || (tx_cfg->post1 < 0))
        MERLIN_ERR("ERR_CODE_TXFIR_POST1_POST2_INVALID\n");
    else if (tx_cfg->post2 > 5)
        MERLIN_ERR("ERR_CODE_TXFIR_POST2_INVALID\n");
    else if (tx_cfg->main < (tx_cfg->pre + tx_cfg->post1 + tx_cfg->post2 + 1))
        MERLIN_ERR("ERR_CODE_TXFIR_SUM_LIMIT\n");
    else if ((tx_cfg->pre + tx_cfg->main + tx_cfg->post1 + tx_cfg->post2) > 60)
        MERLIN_ERR("ERR_CODE_TXFIR_SUM_LIMIT\n");
    else if ((tx_cfg->post2 != 0) && ((tx_cfg->pre + tx_cfg->post1) > 22))
        MERLIN_ERR("ERR_CODE_TXFIR_SUM_LIMIT\n");
    else if ((tx_cfg->post2 == 0) && ((tx_cfg->pre + tx_cfg->post1) > 27))
        MERLIN_ERR("ERR_CODE_TXFIR_SUM_LIMIT\n");
    else if ((tx_cfg->post2to1 < 0) || tx_cfg->post2to1 > 1)
        MERLIN_ERR("ERR_CODE_POST2TO1_INVALID\n");
    else if ((tx_cfg->en_pre < 0) || tx_cfg->en_pre > 1)
        MERLIN_ERR("ERR_CODE_EN_PRE_INVALID\n");
    else if ((tx_cfg->en_post1 < 0) || tx_cfg->en_post1 > 1)
        MERLIN_ERR("ERR_CODE_EN_POST1_INVALID\n");
    else if ((tx_cfg->en_post2 < 0) || tx_cfg->en_post2 > 1)
        MERLIN_ERR("ERR_CODE_EN_POST2_INVALID\n");
    else if ((tx_cfg->tx_refcalshunt < 0) || tx_cfg->tx_refcalshunt > 15)
        MERLIN_ERR("ERR_CODE_TX_REFCALSHUNT_INVALID\n");
    else if ((tx_cfg->hpf < 0) || tx_cfg->hpf > 15)
        MERLIN_ERR("ERR_CODE_HPF_INVALID\n");
    else
        parameters_valid = 1;

    if (parameters_valid != 1)
        return -1;

    MERLIN_WRITE_REG(1, lane_index, 0xd110, MASK_BITS_BETWEEN(0, 3), 0, tx_cfg->pre);
    MERLIN_WRITE_REG(1, lane_index, 0xd111, MASK_BITS_BETWEEN(0, 5), 0, tx_cfg->main);
    MERLIN_WRITE_REG(1, lane_index, 0xd110, MASK_BITS_BETWEEN(5, 9), 5, tx_cfg->post1);
    MERLIN_WRITE_REG(1, lane_index, 0xd111, MASK_BITS_BETWEEN(7, 10), 7, tx_cfg->post2);
    MERLIN_WRITE_REG(1, lane_index, 0xd0a5, MASK_BIT(15), 15, tx_cfg->post2to1);
    MERLIN_WRITE_REG(1, lane_index, 0xd0a5, MASK_BIT(14), 14, tx_cfg->en_pre);
    MERLIN_WRITE_REG(1, lane_index, 0xd0a5, MASK_BIT(13), 13, tx_cfg->en_post1);
    MERLIN_WRITE_REG(1, lane_index, 0xd0a5, MASK_BIT(12), 12, tx_cfg->en_post2);
    MERLIN_WRITE_REG(1, lane_index, 0xd0a3, MASK_BITS_BETWEEN(2, 5), 2, tx_cfg->tx_refcalshunt);
    MERLIN_WRITE_REG(1, lane_index, 0xd0a2, MASK_BITS_BETWEEN(0, 3), 0, tx_cfg->hpf);

    return 0;
}

static int merlin_direct_tx_cfg_get(E_MERLIN_LANE lane_index, merlin_tx_cfg_t *tx_cfg)
{
    MERLIN_READ_REG(1, lane_index, 0xd110, MASK_BITS_BETWEEN(0, 3), 0, &tx_cfg->pre);
    MERLIN_READ_REG(1, lane_index, 0xd111, MASK_BITS_BETWEEN(0, 5), 0, &tx_cfg->main);
    MERLIN_READ_REG(1, lane_index, 0xd110, MASK_BITS_BETWEEN(5, 9), 5, &tx_cfg->post1);
    MERLIN_READ_REG(1, lane_index, 0xd111, MASK_BITS_BETWEEN(7, 10), 7, &tx_cfg->post2);
    MERLIN_READ_REG(1, lane_index, 0xd0a5, MASK_BIT(15), 15, &tx_cfg->post2to1);
    MERLIN_READ_REG(1, lane_index, 0xd0a5, MASK_BIT(14), 14, &tx_cfg->en_pre);
    MERLIN_READ_REG(1, lane_index, 0xd0a5, MASK_BIT(13), 13, &tx_cfg->en_post1);
    MERLIN_READ_REG(1, lane_index, 0xd0a5, MASK_BIT(12), 12, &tx_cfg->en_post2);
    MERLIN_READ_REG(1, lane_index, 0xd0a3, MASK_BITS_BETWEEN(2, 5), 2, &tx_cfg->tx_refcalshunt);
    MERLIN_READ_REG(1, lane_index, 0xd0a2, MASK_BITS_BETWEEN(0, 3), 0, &tx_cfg->hpf);

    return 0;
}

static int  merlin_direct_ioctl(E_MERLIN_LANE lane_index, merlin_command_t cmd, merlin_control_t *data)
{
    int ret = 0;

    switch(cmd)
    {
    case MERLIN_CMD_STATUS_GET:
        ret = merlin_direct_lane_status_get(lane_index, &data->status);
        break;
    case MERLIN_CMD_STATS_GET:
        ret = merlin_direct_lane_stats_get(lane_index, &data->stats);
        break;
    case MERLIN_CMD_PRBS_GENRATE:
        ret = merlin_direct_lane_prbs_generate(lane_index, &data->prbs_type);
        break;
    case MERLIN_CMD_PRBS_MONITOR:
        ret = merlin_direct_lane_prbs_monitor(lane_index, &data->prbs_type);
        break;
    case MERLIN_CMD_PRBS_STATS_GET:
        ret = merlin_direct_lane_prbs_stats_get(lane_index, &data->prbs_stats);
        break;
    case MERLIN_CMD_FORCE_LINK_DOWN:
        MERLIN_WRITE_REG(1, lane_index, 0x09, 0x1, 1+lane_index, 0x0);
        break;
    case MERLIN_CMD_FORCE_LINK_UP:
        MERLIN_WRITE_REG(1, lane_index, 0x09, 0x1, 1+lane_index, 0x1);
        break;
    case MERLIN_CMD_ID:
        ret = merlin_direct_core_id_get(lane_index, &data->serdes_id);
        break;
    case MERLIN_CMD_REG_READ:
        if (data->reg_data.device_type == 9)
            MERLIN_READ_RAM(lane_index, 2, data->reg_data.reg_addr, &data->reg_data.reg_value);
        else
            MERLIN_READ_REG(data->reg_data.device_type, lane_index, data->reg_data.reg_addr, 0xffff, 0, &data->reg_data.reg_value);
        break;
    case MERLIN_CMD_REG_WRITE:
        if (data->reg_data.device_type == 9)
            MERLIN_WRITE_RAM(lane_index, 2, data->reg_data.reg_addr, data->reg_data.reg_value);
        else
            MERLIN_WRITE_REG(data->reg_data.device_type, lane_index, data->reg_data.reg_addr, 0xffff, 0, data->reg_data.reg_value);
        break;
    case MERLIN_CMD_LOOPBACK_SET:
        ret = merlin_direct_loopback_set(lane_index, &data->loopback);
        break;
    case MERLIN_CMD_TXCFG_SET:
        ret = merlin_direct_tx_cfg_set(lane_index, &data->tx_cfg);
        break;
    case MERLIN_CMD_TXCFG_GET:
        ret = merlin_direct_tx_cfg_get(lane_index, &data->tx_cfg);
        break;
    default:
        MERLIN_ERR("Command %d is not supported\n", cmd);
        ret = -1;
        break;
    }

    return ret;
}

extern merlin_sdk_cb_s merlin_callbacks;
void lport_serdes_drv_register(void)
{
    merlin_callbacks.merlin_core_init = merlin_direct_core_init;
    merlin_callbacks.merlin_lane_init = merlin_direct_lane_init;
    merlin_callbacks.merlin_speed_set = merlin_direct_speed_set;
    merlin_callbacks.merlin_get_status = merlin_direct_get_status;
    merlin_callbacks.merlin_ioctl = merlin_direct_ioctl;
}

#ifndef _CFE_
int __init lport_serdes_drv_init(void)
{
    lport_serdes_drv_register();
    return 0;
}


postcore_initcall(lport_serdes_drv_init)
#endif
