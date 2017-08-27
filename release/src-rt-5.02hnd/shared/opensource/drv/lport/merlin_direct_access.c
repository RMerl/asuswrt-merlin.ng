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
#define MERLIN_DBG(a)  printk(a)
#else
#define MERLIN_DBG(a)  xprintf(a)
#endif
#else
#define MERLIN_DBG(a)  
#endif

#ifndef _CFE_
#define MERLIN_ERR  printk
#else
#define MERLIN_ERR  xprintf
#endif

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

#define MERLIN_READ_REG(dev_type, lane_index, reg_addr, mask, shift, value)       \
    do {                                                                         \
        uint32_t addr=0;                                             \
        addr = ((dev_type)<<MMD_TYPE_OFFSET)|(((uint32_t)(lane_index%MERLIN_LANES_PER_CORE))<<LANE_ADDRESS_OFFSET)|(reg_addr); \
        read_serdes_reg((lane_index)/MERLIN_LANES_PER_CORE, addr, (mask), (value));\
        *value >>= shift;\
      } while (0);

#define MERLIN_WRITE_REG(dev_type, lane_index, reg_addr, mask, shift, value)        \
    do {    \
        uint32_t addr=0;  \
        addr = ((dev_type)<<MMD_TYPE_OFFSET)|(((uint32_t)(lane_index%MERLIN_LANES_PER_CORE))<<LANE_ADDRESS_OFFSET)|(reg_addr);    \
        write_serdes_reg((lane_index)/MERLIN_LANES_PER_CORE, addr, (uint16_t)(mask), ((value) << (shift)));    \
     } while (0);

typedef struct{

    LPORT_PORT_MUX_SELECT lane_type[MERLIN_MAX_LANE_NUMBER];
    SERDES_AN_MODE an_mode[MERLIN_MAX_LANE_NUMBER];
}merlin_config_status_t;

merlin_config_status_t g_merlin_status = {};

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

    UDELAY(300);
    
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
    return 0;
}

static int merlin_direct_core_init(E_MERLIN_ID merlin_id, E_MERLIN_VCO pll)
{
    int rc;
    uint32_t lane_id;
    int i;

    rc = merlin_direct_microcode_load(merlin_id, merlin_mptwo_ucode_image, MERLIN_MPTWO_UCODE_IMAGE_SIZE);
    if (rc)
    {
        MERLIN_ERR("Failed to load microcode to serdes%d %d\n",merlin_id, rc);
        return rc;
    }

    if (pll == MERLIN_VCO_9375_MHZ)
    {
        for (i = 0; i < 2; i++)
        {
            lane_id = (merlin_id * MERLIN_LANES_PER_CORE) + i;
            MERLIN_WRITE_REG(0x3, lane_id, 0x9201, MASK_ALL_BITS_16, 0 , 0x2999); /*os_mode_cl36*/
            MERLIN_WRITE_REG(0x3, lane_id, 0x9202, MASK_ALL_BITS_16, 0 , 0x6620); /*pmd_osr_mode*/
            MERLIN_WRITE_REG(0x3, lane_id, 0x9270, MASK_BITS_BETWEEN(0, 13), 0, 0x3); /* reg2p5G_ClockCount0 */
            MERLIN_WRITE_REG(0x3, lane_id, 0x9271, MASK_BITS_BETWEEN(0, 12), 0, 0x1); /* reg2p5G_CGC */
            MERLIN_WRITE_REG(0x3, lane_id, 0x9272, MASK_ALL_BITS_16, 0, 0x102); /*reg2p5G_loopcount0 */
            MERLIN_WRITE_REG(0x3, lane_id, 0x9273, MASK_BITS_BETWEEN(0, 12), 0, 0x6); /* reg2p5G_loopcount1 */
            MERLIN_WRITE_REG(0x3, lane_id, 0x926a, MASK_BITS_BETWEEN(0, 13), 0, 0x19); /* reg1G_ClockCount0 */
            MERLIN_WRITE_REG(0x3, lane_id, 0x926b, MASK_BITS_BETWEEN(0, 12), 0, 0x4); /* reg1G_CGC */
            MERLIN_WRITE_REG(0x3, lane_id, 0x926c, MASK_ALL_BITS_16, 0, 0x105); /*reg1G_loopcount0 */
            MERLIN_WRITE_REG(0x3, lane_id, 0x926d, MASK_BITS_BETWEEN(0, 12), 0, 0xa); /* reg1G_loopcount1 */
            MERLIN_WRITE_REG(0x3, lane_id, 0x9232, MASK_BITS_BETWEEN(0, 8), 0, 0x3); /* reg1G_modulo */
            MERLIN_WRITE_REG(0x3, lane_id, 0x9265, MASK_BITS_BETWEEN(0, 13), 0, 0x177); /* reg100M_ClockCount0 */
            MERLIN_WRITE_REG(0x3, lane_id, 0x9268, MASK_BITS_BETWEEN(0, 12), 0, 0x17); /* reg100M_CGC */
            MERLIN_WRITE_REG(0x3, lane_id, 0x9266, MASK_ALL_BITS_16, 0, 0x100); /*reg100M_loopcount0 */
            MERLIN_WRITE_REG(0x3, lane_id, 0x9269, MASK_BITS_BETWEEN(13, 15), 13, 0x0); /* reg100M_loopcount1_hi */
            MERLIN_WRITE_REG(0x3, lane_id, 0x9268, MASK_BITS_BETWEEN(13, 15), 13, 0x0); /* reg100M_loopcount1_lo */
            MERLIN_WRITE_REG(0x3, lane_id, 0x9267, MASK_BITS_BETWEEN(0, 13), 0, 0x4b); /* reg100M_PCS_ClockCount0*/
            MERLIN_WRITE_REG(0x3, lane_id, 0x9269, MASK_BITS_BETWEEN(0, 12), 0, 0x25); /* reg100M_PCS_CGC */
            MERLIN_WRITE_REG(0x3, lane_id, 0x9231, MASK_BITS_BETWEEN(0, 8), 0, 0x16); /* reg100M_modulo */
            MERLIN_WRITE_REG(0x3, lane_id, 0x9260, MASK_BITS_BETWEEN(0, 13), 0, 0x753); /* reg10M_ClockCount0 */
            MERLIN_WRITE_REG(0x3, lane_id, 0x9263, MASK_BITS_BETWEEN(0, 12), 0, 0xea); /* reg10M_CGC */
            MERLIN_WRITE_REG(0x3, lane_id, 0x9261, MASK_ALL_BITS_16, 0, 0x100); /*reg10M_loopcount0 */
            MERLIN_WRITE_REG(0x3, lane_id, 0x9264, MASK_BITS_BETWEEN(13, 15), 13, 0x0); /* reg10M_loopcount1_hi */
            MERLIN_WRITE_REG(0x3, lane_id, 0x9263, MASK_BITS_BETWEEN(13, 15), 13, 0x0); /* reg10M_loopcount1_lo */
            MERLIN_WRITE_REG(0x3, lane_id, 0x9262, MASK_BITS_BETWEEN(0, 13), 0, 0x4b); /* reg10M_PCS_ClockCount0 */
            MERLIN_WRITE_REG(0x3, lane_id, 0x9264, MASK_BITS_BETWEEN(0, 12), 0, 0x25); /* reg10M_PCS_CGC */
            MERLIN_WRITE_REG(0x3, lane_id, 0x9230, MASK_BITS_BETWEEN(0, 8), 0, 0xe9); /* reg10M_modulo */
        }
    }

    lane_id = merlin_id * MERLIN_LANES_PER_CORE;

    MERLIN_WRITE_REG(0x1, lane_id, 0xd0b9, MASK_BIT(0), 0 , 0x0);

    if (pll == MERLIN_VCO_9375_MHZ)
    {
        MERLIN_WRITE_REG(0x1, lane_id, 0xd0b3, MASK_BITS_BETWEEN(11, 15), 11, 0x1a);
    }
    else
    {
        MERLIN_WRITE_REG(0x1, lane_id, 0xd0b3, MASK_BITS_BETWEEN(11, 15), 11, 0x4);
        MERLIN_WRITE_REG(0x1, lane_id, 0xd0b6, MASK_BITS_BETWEEN(12, 15), 12, 0x0);
        MERLIN_WRITE_REG(0x1, lane_id, 0xd0b7, MASK_BITS_BETWEEN(0, 13), 0 , 0x1000);
        MERLIN_WRITE_REG(0x1, lane_id, 0xd0b8, MASK_BITS_BETWEEN(0, 14), 0 , 0x6cce);
        MERLIN_WRITE_REG(0x1, lane_id, 0xd0b9, MASK_BITS_BETWEEN(0, 2), 0 , 0x0003);
    }
    MERLIN_WRITE_REG(0x3, lane_id, 0x9100, MASK_BITS_BETWEEN(13, 15), 13, 0x6);
    MERLIN_WRITE_REG(0x1, lane_id, 0xd0e3, MASK_BIT(1), 1 , 0x1); /* for lane 0 of current core*/
    MERLIN_WRITE_REG(0x1, (lane_id+1), 0xd0e3, MASK_BIT(1), 1 , 0x1);/*for lane 1 of current core*/

    /*AN speed up needed for IEEE CL37 AN mode*/
    MERLIN_WRITE_REG(0x3, lane_id, 0x9107, MASK_BIT(15), 15, 0x1); /*tick_override*/
    MERLIN_WRITE_REG(0x3, lane_id, 0x9107, MASK_BITS_BETWEEN(0, 14), 0, 0x1); /*tick_nummerator_upper*/
    MERLIN_WRITE_REG(0x3, lane_id, 0x9108, MASK_BITS_BETWEEN(2, 11), 2, 0x1); /*tick_denummerator*/
    MERLIN_WRITE_REG(0x3, lane_id, 0x9250, MASK_ALL_BITS_16, 0, 0x1F); /*cl37_restart_timer*/
    MERLIN_WRITE_REG(0x3, lane_id, 0x9251, MASK_ALL_BITS_16, 0, 0x1F); /*cl37_ack_timer*/
    MERLIN_WRITE_REG(0x3, lane_id, 0x9252, MASK_ALL_BITS_16, 0, 0x0); /*cl37_error_timer*/
    MERLIN_WRITE_REG(0x3, lane_id, 0x9253, MASK_ALL_BITS_16, 0, 0x5); /*cl73_break_link_timer*/
    MERLIN_WRITE_REG(0x3, lane_id, 0x9256, MASK_ALL_BITS_16, 0, 0x1f); /*cl73_link_up*/
    MERLIN_WRITE_REG(0x3, lane_id, 0x925c, MASK_ALL_BITS_16, 0, 0xf); /*ignore_link_timer*/
    MERLIN_WRITE_REG(0x3, lane_id, 0x924a, MASK_ALL_BITS_16, 0, 0xf); /*tx_reset_timer_period */

    return 0;
}

static int merlin_direct_deassert_core(E_MERLIN_ID merlin_id)
{
    uint32_t lane_id;

    lane_id = merlin_id * MERLIN_LANES_PER_CORE;
    //deassert core
    MERLIN_WRITE_REG(0x1, lane_id, 0xd0f2, MASK_ALL_BITS_16, 0, 0x1);

    return 0;
}

static int merlin_direct_lane_init(E_MERLIN_LANE lane_id, lport_serdes_cfg_s *serdes_cfg)
{
    g_merlin_status.lane_type[lane_id] = serdes_cfg->prt_mux_sel;
    g_merlin_status.an_mode[lane_id] = serdes_cfg->autoneg_mode;

    switch (serdes_cfg->prt_mux_sel)
    {
    case PORT_XFI:
        MERLIN_WRITE_REG(0x3, lane_id, 0xc30b, MASK_BIT(11), 11, 0x0); /*creadit_sw_en*/
        MERLIN_WRITE_REG(0x3, lane_id, 0xc30b, MASK_BIT(6), 6, 0x1); /*SW_actual_speed_force_en*/
        MERLIN_WRITE_REG(0x3, lane_id, 0xc30b, MASK_BITS_BETWEEN(0, 5), 0, 0xf); /* SW_actual_speed */
       break;
    case PORT_SGMII_SLAVE:
    case PORT_SGMII_1000BASE_X:
        switch (serdes_cfg->autoneg_mode)
        {
        case MERLIN_AN_NONE:
            /* Force 1G */
            MERLIN_WRITE_REG(0x3, lane_id, 0xc30b, MASK_BIT(11), 11, 0x0); /*creadit_sw_en*/
            MERLIN_WRITE_REG(0x3, lane_id, 0xc30b, MASK_BIT(6), 6, 0x1); /*SW_actual_speed_force_en*/
            MERLIN_WRITE_REG(0x3, lane_id, 0xc30b, MASK_BITS_BETWEEN(0, 5), 0, 0x2); /* SW_actual_speed */
            break; 
        case MERLIN_AN_IEEE_CL37:
            MERLIN_WRITE_REG(0x3, lane_id, 0xc30b, MASK_BITS_BETWEEN(12,13), 12, 0x1); /*use_ieee_reg_ctrl_sel*/
            MERLIN_WRITE_REG(0x0, lane_id, 0x0004, MASK_BIT(5), 5, 0x1); /* dplex */
            MERLIN_WRITE_REG(0x0, lane_id, 0x0000, MASK_BIT(12), 12, 0x1); /*AN enable*/
            MERLIN_WRITE_REG(0x3, lane_id, 0xc30b, MASK_BIT(11), 11, 0x0); /*credit_sw_en*/
            MERLIN_WRITE_REG(0x3, lane_id, 0xc30b, MASK_BIT(6), 6, 0x0); /*SW_actual_speed_force_en*/
            break;
        case MERLIN_AN_USER_CL37:
            MERLIN_WRITE_REG(0x3, lane_id, 0xc30b, MASK_BITS_BETWEEN(12,13), 12, 0x2); /*use_ieee_reg_ctrl_sel*/
            MERLIN_WRITE_REG(0x3, lane_id, 0xc481, MASK_BIT(4), 4, 0x1); /* dplex */
            MERLIN_WRITE_REG(0x3, lane_id, 0xc480, MASK_BIT(6), 6, 0x1); /*AN enable*/
            MERLIN_WRITE_REG(0x3, lane_id, 0xc30b, MASK_BIT(11), 11, 0x0); /*credit_sw_en*/
            MERLIN_WRITE_REG(0x3, lane_id, 0xc30b, MASK_BIT(6), 6, 0x0); /*SW_actual_speed_force_en*/
            break;
        case MERLIN_AN_SGMII_MASTER:
            MERLIN_WRITE_REG(0x3, lane_id, 0xc30b, MASK_BITS_BETWEEN(12,13), 12, 0x2); /*use_ieee_reg_ctrl_sel*/
            MERLIN_WRITE_REG(0x3, lane_id, 0xc481, MASK_BIT(2), 2, 0x1); /* dplex */
            MERLIN_WRITE_REG(0x3, lane_id, 0xc481, MASK_BITS_BETWEEN(0,1), 0, 0x2); /* speed */
            MERLIN_WRITE_REG(0x3, lane_id, 0xc481, MASK_BIT(9), 9, 0x1); /* mater */
            MERLIN_WRITE_REG(0x3, lane_id, 0xc480, MASK_BITS_BETWEEN(6,7), 6, 0x3); /*AN enable*/
            MERLIN_WRITE_REG(0x3, lane_id, 0xc30b, MASK_BIT(11), 11, 0x0); /*credit_sw_en*/
            MERLIN_WRITE_REG(0x3, lane_id, 0xc30b, MASK_BIT(6), 6, 0x0); /*SW_actual_speed_force_en*/
            break;
        case MERLIN_AN_SGMII_SLAVE:
            MERLIN_WRITE_REG(0x3, lane_id, 0xc30b, MASK_BITS_BETWEEN(12,13), 12, 0x2); /*use_ieee_reg_ctrl_sel*/
            MERLIN_WRITE_REG(0x3, lane_id, 0xc480, MASK_BITS_BETWEEN(6,7), 6, 0x3); /*AN enable*/
            MERLIN_WRITE_REG(0x3, lane_id, 0xc30b, MASK_BIT(11), 11, 0x0); /*credit_sw_en*/
            MERLIN_WRITE_REG(0x3, lane_id, 0xc30b, MASK_BIT(6), 6, 0x0); /*SW_actual_speed_force_en*/
            break;
        }
        break;
    case PORT_HSGMII:
        MERLIN_WRITE_REG(0x3, lane_id, 0xc30b, MASK_BIT(11), 11, 0x0); /*creadit_sw_en*/
        MERLIN_WRITE_REG(0x3, lane_id, 0xc30b, MASK_BIT(6), 6, 0x1); /*SW_actual_speed_force_en*/
        MERLIN_WRITE_REG(0x3, lane_id, 0xc30b, MASK_BITS_BETWEEN(0, 5), 0, 0x3); /* SW_actual_speed */
       break;
    default:
        return -1;
    }

    MERLIN_WRITE_REG(0x3, lane_id, 0xc457, MASK_BIT(0), 0, 0x1); /*rx_rstb_lane*/
    MERLIN_WRITE_REG(0x3, lane_id, 0xc30b, MASK_BIT(7), 7, 0x1); /*mac_credentable*/
    MERLIN_WRITE_REG(0x3, lane_id, 0xc433, MASK_BIT(1), 1, 0x1); /*tx_rstb_lane*/
    MERLIN_WRITE_REG(0x3, lane_id, 0xc433, MASK_BIT(0), 0, 0x1); /*enable_tx_lane*/
 
    return 0;
}

static int merlin_direct_lane_datapath_reset(E_MERLIN_LANE lane_id)
{
    MERLIN_WRITE_REG(0x3, lane_id, 0xc457, ~0xfffe, 0, 0x0);
    MERLIN_WRITE_REG(0x3, lane_id, 0xc433, ~0xfffc, 0, 0x0);
    MERLIN_WRITE_REG(0x3, lane_id, 0xc457, ~0xfffe, 0, 0x1);
    MERLIN_WRITE_REG(0x3, lane_id, 0xc433, ~0xfffc, 0, 0x3);

    MERLIN_WRITE_REG(0x1, lane_id, 0xd081, MASK_BIT(1), 1, 0x1);
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

static int merlin_direct_change_speed(E_MERLIN_LANE lane_id, LPORT_PORT_RATE port_rate)
{
    MERLIN_WRITE_REG(0x3, lane_id, 0xc30b, MASK_BIT(11), 11, 0x0); /*creadit_sw_en*/
    MERLIN_WRITE_REG(0x3, lane_id, 0xc30b, MASK_BIT(6), 6, 0x1); /*SW_actual_speed_force_en*/
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

    MERLIN_WRITE_REG(0x3, lane_id, 0xc457, MASK_BIT(0), 0, 0x1); /*rx_rstb_lane*/
    MERLIN_WRITE_REG(0x3, lane_id, 0xc30b, MASK_BIT(7), 7, 0x1); /*mac_credentable*/
    MERLIN_WRITE_REG(0x3, lane_id, 0xc433, MASK_BIT(1), 1, 0x1); /*tx_rstb_lane*/
    MERLIN_WRITE_REG(0x3, lane_id, 0xc433, MASK_BIT(0), 0, 0x1); /*enable_tx_lane*/
 
    MERLIN_WRITE_REG(0x1, lane_id, 0xd081, MASK_BIT(1), 1, 0x1);
    return 0;
}

int merlin_direct_lane_config_get(E_MERLIN_LANE lane_index, lport_serdes_cfg_s *serdes_cfg)
{
    serdes_cfg->prt_mux_sel = g_merlin_status.lane_type[lane_index];
    serdes_cfg->autoneg_mode = g_merlin_status.an_mode[lane_index];

    switch(g_merlin_status.lane_type[lane_index])
    {
        case PORT_XFI:
            serdes_cfg->speed = LPORT_RATE_10G;
            break;
        case PORT_HSGMII:
            serdes_cfg->speed = LPORT_RATE_2500MB;
            break;
        case PORT_SGMII_SLAVE:
        case PORT_SGMII_1000BASE_X:
            serdes_cfg->speed = LPORT_RATE_1000MB;
            break;
        default:
            serdes_cfg->prt_mux_sel = PORT_UNAVAIL;
            break;
    }
    
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
           Default fixed pattern  8 ones / 8 zeros
           Enable fix pattern + default fixed pattern
        */
        MERLIN_WRITE_REG(1, lane_index, 0xD0E0, MASK_BITS_BETWEEN(0 , 15), 0, 0x3);  
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

    /* Enable prbs_generator control with prbs type  //PRBS7|PRBS9|PRBS11|PRBS15|PRBS23|PRBS31|PRBS58 + prbs_en bit 0*/
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
           MERLIN_READ_REG(data->reg_data.device_type, lane_index, data->reg_data.reg_addr, 0xffff, 0,
                           &data->reg_data.reg_value);
            break;
        case MERLIN_CMD_REG_WRITE:
            MERLIN_WRITE_REG(data->reg_data.device_type, lane_index, data->reg_data.reg_addr, 0xffff, 0,
                data->reg_data.reg_value);
            break;
        case MERLIN_CMD_LOOPBACK_SET:
            ret = merlin_direct_loopback_set(lane_index, &data->loopback);
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
    merlin_callbacks.merlin_init  = merlin_direct_core_init;
    merlin_callbacks.merlin_datapath_reset = merlin_direct_lane_datapath_reset;
    merlin_callbacks.merlin_set_cfg = merlin_direct_lane_init;
    merlin_callbacks.merlin_get_cfg = merlin_direct_lane_config_get;
    merlin_callbacks.merlin_get_status = merlin_direct_get_status;
    merlin_callbacks.merlin_post_init = merlin_direct_deassert_core;
    merlin_callbacks.merlin_change_speed = merlin_direct_change_speed;
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
