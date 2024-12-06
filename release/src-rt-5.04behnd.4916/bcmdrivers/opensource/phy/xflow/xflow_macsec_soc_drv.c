/***********************************************************************
 *
 *  Copyright (c) 2021  Broadcom Corporation
 *  All Rights Reserved
 *
 * <:label-BRCM:2012:DUAL/GPL:standard
 * 
 * Unless you and Broadcom execute a separate written software license
 * agreement governing use of this software, this software is licensed
 * to you under the terms of the GNU General Public License version 2
 * (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
 * with the following added to such license:
 * 
 *    As a special exception, the copyright holders of this software give
 *    you permission to link this software with independent modules, and
 *    to copy and distribute the resulting executable under terms of your
 *    choice, provided that you also meet, for each linked independent
 *    module, the terms and conditions of the license of that module.
 *    An independent module is a module which is not derived from this
 *    software.  The special exception does not apply to any modifications
 *    of the software.
 * 
 * Not withstanding the above, under no circumstances may you combine
 * this software in any way with any other Broadcom software provided
 * under a license other than the GPL, without Broadcom's express prior
 * written consent.
 * 
 * :>
 *
 ************************************************************************/

#include "macsec_defs.h"
#include "macsec_dev.h"
#include "macsec_types.h"
#include "macsec_macros.h"

#include "bchp_regs_int.h"

extern soc_driver_t soc_driver_bcm68880;

/*
 * Function:
 *      soc_attach
 * Purpose:
 *      Initialize the soc_control_t structure for a device,
 *      allocating all memory and semaphores required.
 * Parameters:
 *      unit - StrataSwitch unit 
 *      detach - Callback function called on detach.
 * Returns:
 *      SOC_E_XXX
 * Notes:
 *      No chip initialization is done other than masking all interrupts,
 *      see soc_init or soc_reset_init.
 */
int soc_attach(int unit)
{
    soc_control_t       *soc;
    int                 rv = 0;

    /*
     * Allocate soc_control and soc_persist if not already.
     */
    if (SOC_CONTROL(unit) == NULL) 
    {
        SOC_CONTROL(unit) = sal_alloc(sizeof (soc_control_t), "soc_control");
        if (SOC_CONTROL(unit) == NULL)
        {
            return SOC_E_MEMORY;
        }
        sal_memset(SOC_CONTROL(unit), 0, sizeof (soc_control_t));
    }

    soc = SOC_CONTROL(unit);

    /*
     * Instantiate the driver -- Verify chip revision matches driver
     * compilation revision.
     */
    soc->chip_driver = &soc_driver_bcm68880;

    if (soc->chip_driver == NULL)
        return SOC_E_UNAVAIL;
    
    return rv;
}

int soc_macsec_hw_init(int unit, int macsec_ing_byp, int macsec_egr_byp)
{
    unsigned int rval;
    int rv = BCM_E_NONE;
    soc_ubus_reg_t reg;
    soc_ubus_field_t fld;
    
    rval = 0;
    reg = MACSEC_HW_RESET_CONTROLreg;
    fld = MEM_TABLE_INIT_fld;
    soc_ubus_reg_field32_set(unit, reg, &rval, fld, 1);
    fld = MIB_INIT_fld;
    soc_ubus_reg_field32_set(unit, reg, &rval, fld, 1);
    fld = TCAM_INIT_fld;
    soc_ubus_reg_field32_set(unit, reg, &rval, fld, 1);

    rv = soc_ubus_reg32_set(unit, reg, REG_PORT_ANY, rval);
    if (SOC_FAILURE(rv))
        return rv;

    fld = ISEC_MIB_SA_INIT_DONE_fld;

    do 
    {
        rv = soc_ubus_reg32_get(unit, reg, REG_PORT_ANY, &rval);
        
        if (SOC_FAILURE(rv))
        {
            return rv;
        }
        if (soc_ubus_reg32_field_get(unit, reg, rval, fld))
        {
        #if 0
            rval = 0;
            rv = soc_ubus_reg32_set(unit, reg, 0, 0, rval);
            if (SOC_FAILURE(rv))
            {
                return rv;
            }
        #endif
            break;
        }
            
    } while (TRUE);
    
    return rv;
}

/* Init */
int _soc_xflow_macsec_init(int unit)
{
#if 0 //TODO MSN - Needs sorting
    uint32 rval, val;
    soc_mem_t tcam[] = {ISEC_SC_TCAMm, ISEC_SP_TCAMm};
    int idx;

    if (soc_feature(unit, soc_feature_xflow_macsec_inline))
        return BCM_E_NONE;

    rval = 0;
    
    if (soc_feature(unit, soc_feature_xflow_macsec_olp))
    {
        if (SOC_REG_FIELD_VALID(unit, SP_GLB_CTRLr, LOOPBACK_ENf))
        {
            soc_reg_field_set(unit, SP_GLB_CTRLr, &rval, LOOPBACK_ENf, 1);
        }
        if (SOC_REG_FIELD_VALID(unit, SP_GLB_CTRLr, PORT_MODEf))
        {
            soc_reg_field_set(unit, SP_GLB_CTRLr, &rval, PORT_MODEf, 0);
        }
    }
    else
    {
        if (SOC_REG_FIELD_VALID(unit, SP_GLB_CTRLr, LOOPBACK_ENf))
        {
            soc_reg_field_set(unit, SP_GLB_CTRLr, &rval, LOOPBACK_ENf, 0);
        }
        if (SOC_REG_FIELD_VALID(unit, SP_GLB_CTRLr, PORT_MODEf))
        {
            soc_reg_field_set(unit, SP_GLB_CTRLr, &rval, PORT_MODEf, 3);
        }
        if (SOC_REG_FIELD_VALID(unit, SP_GLB_CTRLr, DUAL_CYCLE_MODEf))
        {
            soc_reg_field_set(unit, SP_GLB_CTRLr, &rval, DUAL_CYCLE_MODEf, 1);
        }
    }

    num_sa_per_sc[unit] = soc_property_get(unit, spn_XFLOW_MACSEC_SECURE_CHAN_TO_NUM_SECURE_ASSOC, 1);
    
    if ((num_sa_per_sc[unit] != 4) && (num_sa_per_sc[unit] != 1))
        num_sa_per_sc[unit] = 1;

    if (num_sa_per_sc[unit] == 4)
        soc_reg_field_set(unit, SP_GLB_CTRLr, &rval, SC_MODEf, 1);
    
    soc_reg_field_set(unit, SP_GLB_CTRLr, &rval, SOFT_RESETf, 1);
    SOC_IF_ERROR_RETURN (WRITE_SP_GLB_CTRLr(unit, rval));
    sal_usleep(1000);
    soc_reg_field_set(unit, SP_GLB_CTRLr, &rval, SOFT_RESETf, 0);
    SOC_IF_ERROR_RETURN (WRITE_SP_GLB_CTRLr(unit, rval));

    rval = 0;
    soc_reg_field_set(unit, ESEC_CONFIGr, &rval, ESEC_MEM_INITf, 1);
    SOC_IF_ERROR_RETURN(WRITE_ESEC_CONFIGr(unit, rval));
    soc_reg_field_set(unit, ESEC_CONFIGr, &rval, ESEC_MEM_INITf, 0);
    SOC_IF_ERROR_RETURN(WRITE_ESEC_CONFIGr(unit, rval));

    rval = 0;
    soc_reg_field_set(unit, ISEC_CNTRLr, &rval, ISEC_MEM_INITf, 1);
    SOC_IF_ERROR_RETURN(WRITE_ISEC_CNTRLr(unit, rval));
    soc_reg_field_set(unit, ISEC_CNTRLr, &rval, ISEC_MEM_INITf, 0);
    SOC_IF_ERROR_RETURN(WRITE_ISEC_CNTRLr(unit, rval));

    /* Clear the TCAMs */
    for (idx = 0 ; idx < COUNTOF(tcam) ; idx++)
    {
        if (SOC_MEM_IS_VALID(unit, tcam[idx]))
        {
            BCM_IF_ERROR_RETURN(soc_mem_clear(unit, tcam[idx], MEM_BLOCK_ALL, TRUE));
        }
    }

    rval = 0;
    if (SOC_REG_IS_VALID(unit, MACSEC_CREDIT_ENABLEr))
    {
        soc_reg_field_set(unit, MACSEC_CREDIT_ENABLEr, &rval, ENABLEf, 0);
        SOC_IF_ERROR_RETURN(WRITE_MACSEC_CREDIT_ENABLEr(unit, rval));
        soc_reg_field_set(unit, MACSEC_CREDIT_ENABLEr, &rval, ENABLEf, 1);
        SOC_IF_ERROR_RETURN(WRITE_MACSEC_CREDIT_ENABLEr(unit, rval));
    }

    //init_usec_timeout = 1000;
    
    SOC_IF_ERROR_RETURN(READ_ESEC_CONFIGr(unit, &rval));
    if (SOC_REG_FIELD_VALID(unit, ESEC_CONFIGr, BBRC_ENABLEf))
    {
        soc_reg_field_set(unit, ESEC_CONFIGr, &rval, BBRC_ENABLEf, 1);
    }
    
    if (SOC_REG_FIELD_VALID(unit, ESEC_CONFIGr, STAGECOUNTf))
    {
        soc_reg_field_set(unit, ESEC_CONFIGr, &rval, STAGECOUNTf, 0x20);
    }

    val = soc_property_get(unit, spn_XFLOW_MACSEC_ENCRYPT_FAIL_SWITCH_TO_CPU, 0);
    if (val > 1)
    {
            return BCM_E_CONFIG;
    }

    soc_reg_field_set(unit, ESEC_CONFIGr, &rval, ESEC_DROPFAILEDPKTSf, ((!val) & 0x1));
    SOC_IF_ERROR_RETURN(WRITE_ESEC_CONFIGr(unit, rval));

    //soc_timeout_init(&to, init_usec_timeout, 0);
    do
    {
        SOC_IF_ERROR_RETURN(READ_ESEC_CONFIGr(unit, &rval));
        if (soc_reg_field_get(unit, ESEC_CONFIGr, rval, ESEC_INIT_DONEf))
        {
            soc_reg_field_set(unit, ESEC_CONFIGr, &rval, ESEC_MEM_INITf, 0);
            SOC_IF_ERROR_RETURN(WRITE_ESEC_CONFIGr(unit, rval));
            break;
        }

        //if (soc_timeout_check(&to))
        //{
        //    return BCM_E_TIMEOUT;
        //}
    } while (TRUE);

    SOC_IF_ERROR_RETURN(READ_ISEC_CNTRLr(unit, &rval));
    if (SOC_REG_FIELD_VALID(unit, ISEC_CNTRLr, DROP_CONTROL_FRAME_ENf))
    {
        soc_reg_field_set(unit, ISEC_CNTRLr, &rval, DROP_CONTROL_FRAME_ENf, 1);
    }

    soc_reg_field_set(unit, ISEC_CNTRLr, &rval, INGRESS_TCAM_MISS_PKT_DROP_ENf, 1);
    soc_reg_field_set(unit, ISEC_CNTRLr, &rval, INVALIDATE_SA_ENf, 0x1);
    val = soc_property_get(unit, spn_XFLOW_MACSEC_DECRYPT_PAD_THRESHOLD, 0);
    
    if (val != 0)
    {
        if ((val < 60) || (val > 127))
        {
            return BCM_E_CONFIG;
        }
        soc_reg_field_set(unit, ISEC_CNTRLr, &rval, PAD_ENf, 1);
        soc_reg_field_set(unit, ISEC_CNTRLr, &rval, PAD_THRESHOLDf, val);
    }
    
    val = soc_property_get(unit, spn_XFLOW_MACSEC_DECRYPT_SECTAG_C1E0_ERROR, 0);
    if (val > 1)
    {
        return BCM_E_CONFIG;
    }
    soc_reg_field_set(unit, ISEC_CNTRLr, &rval, C_E_ERROR_ENf, val);

    val = soc_property_get(unit, spn_XFLOW_MACSEC_DECRYPT_FAIL_SWITCH_TO_CPU, 0);
    if (val > 1)
    {
        return BCM_E_CONFIG;
    }

    soc_reg_field_set(unit, ISEC_CNTRLr, &rval, ISEC_DROPFAILEDPKTSf, ((!val) & 0x1));
    if (!SAL_BOOT_BCMSIM && !SAL_BOOT_XGSSIM)
    {
        soc_reg_field_set(unit, ISEC_CNTRLr, &rval, SC_TCAM_SW_ENCODEf, 0x1);
        soc_reg_field_set(unit, ISEC_CNTRLr, &rval, SP_TCAM_SW_ENCODEf, 0x1);
    }
    
    if (SOC_REG_FIELD_VALID(unit, ISEC_CNTRLr, CRC_APPEND_ENf))
    {
        soc_reg_field_set(unit, ISEC_CNTRLr, &rval, CRC_APPEND_ENf, 0x1);
    }

    if (soc_property_get(unit, spn_XFLOW_MACSEC_SKIP_DECRYPT_PKT_PARSER, 0))
    {
        soc_reg_field_set(unit, ISEC_CNTRLr, &rval, SKIP_DECRYPT_PKT_PARSERf, 0x1);
    }

    SOC_IF_ERROR_RETURN(WRITE_ISEC_CNTRLr(unit, rval));
    
    if (!(SAL_BOOT_BCMSIM || SAL_BOOT_XGSSIM))
    {
        soc_timeout_init(&to, init_usec_timeout, 0);
        do {
            SOC_IF_ERROR_RETURN(READ_ISEC_CNTRLr(unit, &rval));
            if (soc_reg_field_get(unit, ISEC_CNTRLr, rval, ISEC_INIT_DONEf))
            {
                soc_reg_field_set(unit, ISEC_CNTRLr, &rval, ISEC_MEM_INITf, 0);
                SOC_IF_ERROR_RETURN(WRITE_ISEC_CNTRLr(unit, rval));
                break;
            }
            if (soc_timeout_check(&to))
            {
                return BCM_E_TIMEOUT;
            }
        } while (TRUE);
    }
#endif
    return BCM_E_NONE;
}
