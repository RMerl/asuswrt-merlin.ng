/*
<:copyright-BRCM:2013:DUAL/GPL:standard

   Copyright (c) 2013 Broadcom 
   All Rights Reserved

Unless you and Broadcom execute a separate written software license
agreement governing use of this software, this software is licensed
to you under the terms of the GNU General Public License version 2
(the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
with the following added to such license:

   As a special exception, the copyright holders of this software give
   you permission to link this software with independent modules, and
   to copy and distribute the resulting executable under terms of your
   choice, provided that you also meet, for each linked independent
   module, the terms and conditions of the license of that module.
   An independent module is a module which is not derived from this
   software.  The special exception does not apply to any modifications
   of the software.

Not withstanding the above, under no circumstances may you combine
this software in any way with any other Broadcom software provided
under a license other than the GPL, without Broadcom's express prior
written consent.

:>
*/

#include <linux/string.h>
#include <linux/workqueue.h>
#include <linux/delay.h>
#include <stdarg.h>
#include "pmc_drv.h"
#include "bcm_map_part.h"
#include "BPCM.h"

#define KEYHOLE_IDX 1
#include "pmc_direct_access_pmc3.h"

#define MAX_BPCM_DEVICES    64
#define MAX_PVTMON_DEVICES  1
#define MAX_MAPPED_DEVICES  (MAX_BPCM_DEVICES + MAX_PVTMON_DEVICES)
#define MIN_AVS_MV          860	 // Vavs = 0.86V
#define MAX_AVS_MV          1035 // Vavs = 1.035V
#define FSADC_MV            880  // 0.88V Full-scale ADC voltage (page 10 of 28nm spec)
#define MV2ADC(mv)			((1024 * 7 * (mv))/(FSADC_MV * 10))  // derived from formula on page 10 of 16nm spec (sel = 3'b011)
#define ADC2MV(adc)			(((adc) * FSADC_MV * 10)/(7 * 1024))

#define DAC_STEP_SIZE       1
#define SSB_D_1p0V_SW       0
#define SSB_D_1p0V_REG      6

#define MV2SWREGDAC(mv)     (((1000 * (mv)) - 450000)/3125)
#define SWREGDAC2MV(dc)     ((450000 + ((dc) * 3125))/1000)

#define MIN_SWREG_DAC_CODE	MV2SWREGDAC(850)  // 0.85V
#define MAX_SWREG_DAC_CODE	MV2SWREGDAC(1250) // 1.25V
#define MIN_PVTMON_DAC_CODE	50		// 0.85V
#define MAX_PVTMON_DAC_CODE	973		// 1.05V

#define MIN_DAC_CODE (use_swreg ? MIN_SWREG_DAC_CODE : MIN_PVTMON_DAC_CODE)
#define MAX_DAC_CODE (use_swreg ? MAX_SWREG_DAC_CODE : MAX_PVTMON_DAC_CODE)
#define DAC_CHANGE_DELAY_MS		30      // wait this long after changing the DAC setting

static int use_swreg = 1;

typedef struct DeviceInfo {
    uint32 fPresent    : 1;	// whether or not this address is currently present
    uint32 fDevType    : 4;	// see AVS3_BPCM.h::BPCM_CAPABILITES_REG for types of PMB devices
    uint32 fIsClassic  : 1;    // treat this device as a classic BPCM slave
    uint32 fBus        : 4;    // values from bus enum above
    uint32 fDevAddr    : 9;	// architecture is limited to 512 device addresses
    uint32 fNumZones   : 8;    // architecture is limited to 512 zones/device
    uint32 fReserved   : 4;     
} TDeviceInfo;	// 4 bytes/device

enum {
    kBUS_BPCM0 = 0,     // there are a maximum of 8 PMB buses (0-7)
    kBUS_BPCM1,
    kBUS_BPCM2,
    kBUS_BPCM3,
    kBUS_BPCM4,
    kBUS_BPCM5,
    kBUS_BPCM6,
    kBUS_BPCM7,
	kBUS_SSB,
    kBUS_LOCAL,         // deprecated in PMB3
	kBUS_LAST_DEV = 15	// indicates the final device in the map
};

// error codes
enum {
    NO_ERROR = 0,
    INVALID_ISLAND,
    INVALID_DEVICE,
    INVALID_ZONE,
    INVALID_STATE,
    INVALID_COMMAND,
    LOG_EMPTY,
    INVALID_PARAM,
    BPCM_READ_TIMEOUT,
    INVALID_BUS,
    INVALID_QUEUE_NUMBER,
    QUEUE_NOT_AVAILABLE,
    INVALID_TOKEN_SIZE,
    INVALID_WATERMARKS,
    INSUFFIENT_QSM_MEMORY_AVAILABLE,
    INVALID_BOOT_COMMAND,
    BPCM_WRITE_TIMEOUT,
    CMD_TABLE_FULL,
    CMD_TABLE_LOCKED,
    BPCM_SLAVE_ERROR,
};

typedef struct {
    uint16        avs_min_adc;
    uint16        avs_max_adc;
    TDeviceInfo     device_map[MAX_MAPPED_DEVICES+1];   // 40 devices * 4 bytes each = 160 bytes
    TDeviceInfo     *pvtmon_map;           // 1 island * 4 bytes/island = 4 bytes
} pmc_ctx_t;

void track_AVS(struct work_struct *);
static pmc_ctx_t g_pmc_ctx;
static struct workqueue_struct *avs_workqueue;
static struct delayed_work tracking_task;
static DECLARE_DELAYED_WORK(tracking_task, track_AVS);

#define SET_ADDR(sw, reg)  (((sw) << 5 | ((reg) & 0x1f)) & 0x2ff)

static int is_swreg_locked(void)
{
    return PMC->ssbMasterCtrl.ssbmSwLock.Bits.lock_bit;
}

static uint16 get_swreg(uint8 sw, uint8 reg)
{
    PMC->ssbMasterCtrl.ssbmControl.Bits.ssb_en = 1;

    PMC->ssbMasterCtrl.ssbmControl.Bits.ssb_cmd = kSSBRead;
    PMC->ssbMasterCtrl.ssbmControl.Bits.ssb_addr = SET_ADDR(sw, reg);
    PMC->ssbMasterCtrl.ssbmControl.Bits.ssb_add_pre = 1;
    PMC->ssbMasterCtrl.ssbmControl.Bits.ssb_en = 1;
    PMC->ssbMasterCtrl.ssbmControl.Bits.ssb_start = 1;

    while (PMC->ssbMasterCtrl.ssbmControl.Bits.ssb_start)
    {
        ; /* Do nothing */
    }
    return PMC->ssbMasterCtrl.ssbmRdData.Bits.data;
}

static void set_swreg(uint8 sw, uint8 reg, uint16 val)
{
    PMC->ssbMasterCtrl.ssbmControl.Bits.ssb_en = 1;

    PMC->ssbMasterCtrl.ssbmWrData.Bits.data = val;

    PMC->ssbMasterCtrl.ssbmControl.Bits.ssb_cmd = kSSBWrite;
    PMC->ssbMasterCtrl.ssbmControl.Bits.ssb_addr = SET_ADDR(sw, reg);
    PMC->ssbMasterCtrl.ssbmControl.Bits.ssb_add_pre = 1;
    PMC->ssbMasterCtrl.ssbmControl.Bits.ssb_en = 1;
    PMC->ssbMasterCtrl.ssbmControl.Bits.ssb_start = 1;

    while (PMC->ssbMasterCtrl.ssbmControl.Bits.ssb_start)
    {
        ; /* Do nothing */
    }
}

#define PMC3_LOG_SIZE           (0x4000) 
#define PMC3_LOG_START_ADDRESS  (0x040fc000)

static uint32 wraparound_skip_offset = 0;
static uint32 wr_offset = 0;

static void put_str(const char *str)
{
    char *dst = NULL;
    while (*str)
    {
        dst = (char *)phys_to_virt(PMC3_LOG_START_ADDRESS + wr_offset);
        *dst = *str;
        str++;
        wr_offset++;
        if (wr_offset == PMC3_LOG_SIZE)
            wr_offset = wraparound_skip_offset;
    }
}

static void set_wraparound_offset(void)
{
    wraparound_skip_offset = wr_offset;
}

void printf_log(char *format, ...)
{
    char str[256];
    static int first = 1; 
    va_list va_args;
	char *virt = phys_to_virt(PMC3_LOG_START_ADDRESS);

    if (first)
    {
        first = 0;
        memset((void *)virt, 0, PMC3_LOG_SIZE);
        *((uint32 *)(virt-4)) = 0xc0ffee55;
    }

    va_start(va_args, format);
    vsprintf(&str[0], format, va_args);
    va_end(va_args);

    put_str(str);
}

static void InitDevice(TDeviceInfo *devPtr, int devType, int addr, int bus)
{
    *(uint32*)devPtr  = 0;
    devPtr->fDevType = devType;
    devPtr->fDevAddr = addr;
    devPtr->fBus = bus;
}

static void StartDiscovery(void)
{
    PMB_CONFIG_REG cfgreg;

    // kick off H/W device discovery
    cfgreg.Reg32 = PMC->pmb.config.Reg32;
    cfgreg.Bits.startDiscovery = 1;
    PMC->pmb.config.Reg32 = cfgreg.Reg32;
}

static void InitMemory(void)
{
    TDeviceInfo *di;
    int ix;

    memset(&g_pmc_ctx, 0, sizeof(pmc_ctx_t));
    g_pmc_ctx.avs_min_adc = MV2ADC(MIN_AVS_MV);
    g_pmc_ctx.avs_max_adc = MV2ADC(MAX_AVS_MV);
 
    // initialize device tables
    di = g_pmc_ctx.device_map;
    for( ix = 0; ix < MAX_MAPPED_DEVICES + 1; ix++, di++ )
    {
        InitDevice(di, kPMB_NO_DEVICE, 0, kBUS_LAST_DEV);
    }
}

static void MapDevice(TDeviceInfo *devPtr, BPCM_ID_REG *id, int bus)
{
    // following InitDevice, *devPtr will be valid for fAddr, fBus
    InitDevice(devPtr, id->Bits.devType, id->Bits.pmbAddr, bus);

    // post-initialization
    devPtr->fPresent = 1;
    devPtr->fIsClassic = 0;

    printf_log("AVS: Found device: Type: %d Bus: %d Addr 0x%x\n", devPtr->fDevType, devPtr->fBus, devPtr->fDevAddr);
}

static void MapSystem(void)
{
    uint16 totalDevices = 0;
    int addr;
    TDeviceInfo *di = g_pmc_ctx.device_map;
    uint16 max_client_id;
    uint8 pmb_map[255]={};
    int rc;

    // spin waiting for H/W discovery to complete
    while( PMC->pmb.config.Bits.discoveryBusy ) ;

    max_client_id = PMC->pmb.config.Bits.maxClientId;
    memcpy((uint32 *)pmb_map, (const uint32 *)PMC->pmb.map, max_client_id + 1);


    printf_log("Dump scan map: %d max_client_id\n", max_client_id);
    printf_log("      0 1 2 3 4 5 6 7 8 9 A B C D E F");
    for( addr = 0; addr <= max_client_id; addr++ )
    {
        if (addr % 16 == 0)
            printf_log("\n0x%03X", addr);
        printf_log(" %d", pmb_map[addr]);
    }
    printf_log("\n");

    for( addr = 0; addr <= max_client_id; addr++ )
    {
        int bus;
        BPCM_ID_REG id;

        rc = NO_ERROR;
        // H/W indicates a device with this address exists on the specified bus
        if (!pmb_map[addr])
        {
            continue;
        }

        bus = pmb_map[addr] - 1;
        rc = read_bpcm_reg_direct((addr| (bus << PMB_BUS_ID_SHIFT)), BPCM_OFFSET(id_reg), &id.Reg32);
        if( id.Reg32 != 0 && rc != BPCM_READ_TIMEOUT )
        {
            MapDevice(di, &id, bus);
            if( id.Bits.devType == kPMB_PVTMON )
            {
                // save this pointer in the g_pmc_ctx.pvtmon_map unless the target location has already been filled
                if(!g_pmc_ctx.pvtmon_map)
                    g_pmc_ctx.pvtmon_map = di;
                else
                    continue;
            }
            else if( id.Bits.devType == kPMB_TMON_INTERNAL || id.Bits.devType == kPMB_TMON_EXTERNAL )
            {
                continue; 
            }
            di++;
            totalDevices++;
        }
        if( totalDevices == MAX_MAPPED_DEVICES )
            break;
    }
}

static uint16 GetDac(void)
{
    if (use_swreg)
    {
        uint16 val;
        val = get_swreg(SSB_D_1p0V_SW, SSB_D_1p0V_REG);
        return (val >> 8) & 0xff;
    }
    else
    {
        TDeviceInfo *di = g_pmc_ctx.pvtmon_map;
        APVTMON_DATA_REG target;
        read_bpcm_reg_direct(di->fDevAddr | (di->fBus <<PMB_BUS_ID_SHIFT), PVTMON_OFFSET(adc_data), &target.Reg32);
        return target.Bits.dac_data;
    }
}

static int SetDac(int value)
{
    
    if( value > MAX_DAC_CODE )
    {
        value = MAX_DAC_CODE;
    } 
    else if( value < MIN_DAC_CODE )
    {
        value = MIN_DAC_CODE;
    }

    printf_log("DAC: %d\n", value);
    if (use_swreg)
    {
        uint16 val;
     
        val = get_swreg(SSB_D_1p0V_SW, SSB_D_1p0V_REG);

        val &= ~0xff00;
        val |= value << 8;
        set_swreg(SSB_D_1p0V_SW, SSB_D_1p0V_REG, val);

        /*Toggle bit 1 of reg 0 to force new values to be loaded */
        val = get_swreg(SSB_D_1p0V_SW, 0);

        val &= ~0x2;
        set_swreg(SSB_D_1p0V_SW, 0, val);
        val |= 0x2;
        set_swreg(SSB_D_1p0V_SW, 0, val);
        val &= ~0x2;
        set_swreg(SSB_D_1p0V_SW, 0, val);
    }
    else
    {
        TDeviceInfo *di = g_pmc_ctx.pvtmon_map;
        APVTMON_DATA_REG target;

        read_bpcm_reg_direct(di->fDevAddr | (di->fBus <<PMB_BUS_ID_SHIFT), PVTMON_OFFSET(adc_data), &target.Reg32);
        target.Bits.dac_data = value;
        write_bpcm_reg_direct(di->fDevAddr | (di->fBus <<PMB_BUS_ID_SHIFT), PVTMON_OFFSET(adc_data), target.Reg32);
    }
    return value;
}

static int ReadPVT(int index)
{
    APVTMON_ACCUM_REG target;
    TDeviceInfo *devPtr = g_pmc_ctx.pvtmon_map;
    int devAddr = devPtr->fDevAddr | (devPtr->fBus << PMB_BUS_ID_SHIFT);

    read_bpcm_reg_direct(devAddr, PVTMON_OFFSET(acq_accum_regs[index]), &target.Reg32);
    while(!target.Bits.valid)
    {
        // the value SHOULD be valid immediatly, but just in case...
        read_bpcm_reg_direct(devAddr, PVTMON_OFFSET(acq_accum_regs[index]), &target.Reg32);
    }
    return target.Bits.value;
}

static void dump_system_status(void)
{
    uint32 dac = GetDac();

    printf_log("PVTMON: 1VD AVS closed at %dmV\n", pmc_convert_pvtmon(kV_VIN, ReadPVT(kV_VIN)));
    printf_log("PVTMON: Current Die temp is %dmC\n", pmc_convert_pvtmon(kTEMPERATURE, ReadPVT(kTEMPERATURE)));
    printf_log("%s: Current DAC value %d = %dmV\n", use_swreg ? "SWREG ":"PVTMON", dac, use_swreg ? SWREGDAC2MV(dac):ADC2MV(dac));
    if (is_swreg_locked())
    {
        printf_log("SWREG is locked in range %d ... %d\n", PMC->ssbMasterCtrl.ssbmThHiLo.Bits.swreg_th_lo,
            PMC->ssbMasterCtrl.ssbmThHiLo.Bits.swreg_th_hi);
    }
}

static void AdjustDac(int delta)
{
    uint32 new_val, cur_val, adc, temp;

    new_val = cur_val = GetDac();
    adc = ReadPVT(kV_VIN);
    temp = ReadPVT(kTEMPERATURE);
    if( adc > g_pmc_ctx.avs_min_adc && adc < g_pmc_ctx.avs_max_adc )
    {
        // voltage is currently within absolute voltage limits...
        // go ahead and apply the change
        new_val = cur_val + delta;
    } 
    else
    {
        // voltage is already either right on a limit or is outside the absolute voltage limits...
        // in the latter case, apply a correction to try to get back within limits
        if( adc < g_pmc_ctx.avs_min_adc ) 
            new_val = cur_val + 2 * DAC_STEP_SIZE;
        else if( adc > g_pmc_ctx.avs_max_adc ) 
            new_val = cur_val - 2 * DAC_STEP_SIZE;
    }

    printf_log("cur_dac %u curV %umV curTemp %dmC delta %d new_dac %u\n", cur_val, pmc_convert_pvtmon(kV_VIN, adc),
        pmc_convert_pvtmon(kTEMPERATURE, temp), delta, new_val);

    // if a change from the current value is indicated...
    if( new_val != cur_val )
    {
        SetDac(new_val);
        mdelay(DAC_CHANGE_DELAY_MS);
    }
}

void track_AVS(struct work_struct *p)
{
    TDeviceInfo *di = g_pmc_ctx.device_map;
    int too_hi = 0, too_lo = 0, total_ars = 0;
    int rc;

    while( di->fBus != kBUS_LAST_DEV )
    {
        if(di->fDevType == kPMB_ARS)
        {
            BPCM_AVS_ROSC_CONTROL_REG ctlReg;
            int dev_addr = di->fDevAddr | (di->fBus << PMB_BUS_ID_SHIFT);

            rc = read_bpcm_reg_direct(dev_addr, ARS_OFFSET(rosc_control), &ctlReg.Reg32);
            if (rc || !ctlReg.Bits.valid_h || !ctlReg.Bits.valid_s)
                continue; /* ReRead the value */
            total_ars++;

            if( ctlReg.Bits.alert_s || ctlReg.Bits.alert_h )
            {
                BPCM_AVS_ROSC_COUNT countReg;
                ECTR_THRESH_REG threshReg_s, threshReg_h;

                rc = read_bpcm_reg_direct(dev_addr, ARS_OFFSET(rosc_count), &countReg.Reg32);
                rc = read_bpcm_reg_direct(dev_addr, ARS_OFFSET(rosc_thresh_s), &threshReg_s.Reg32);
                rc = read_bpcm_reg_direct(dev_addr, ARS_OFFSET(rosc_thresh_h), &threshReg_h.Reg32);

                if( countReg.Bits.count_s <= threshReg_s.Bits.thresh_lo || 
                    countReg.Bits.count_h <= threshReg_h.Bits.thresh_lo )
                {
                    too_lo = 1;
                    break;
                }
                else if( countReg.Bits.count_s >= threshReg_s.Bits.thresh_hi &&
                    countReg.Bits.count_h >= threshReg_h.Bits.thresh_hi )
                {
                    too_hi++;
                }                   

                /* To cleanup the alert_s/h bits the threshold enable bits need to be toggled */
                if (ctlReg.Bits.alert_h)
                    ctlReg.Bits.thresh_en_h = 0;
                if (ctlReg.Bits.alert_s)
                    ctlReg.Bits.thresh_en_s = 0;
                rc = write_bpcm_reg_direct(dev_addr, ARS_OFFSET(rosc_control), ctlReg.Reg32);

                ctlReg.Bits.thresh_en_s = 1;
                ctlReg.Bits.thresh_en_h = 1;
                rc = write_bpcm_reg_direct(dev_addr, ARS_OFFSET(rosc_control), ctlReg.Reg32);
            }
        }
        di++;
    }
    if( too_lo )
    {
        printf_log("AVS:Raise V\n");
        AdjustDac(1 * DAC_STEP_SIZE);
    }
    else if( too_hi == total_ars )
    {
        printf_log("AVS:Low V\n");
        AdjustDac(-1 * DAC_STEP_SIZE);
    }
    queue_delayed_work(avs_workqueue, &tracking_task, CONFIG_HZ/2);
}

void pmc_tracking_init(void)
{
    if (!is_swreg_locked())
    {
        printk("============== ATTENTION !!!!!! =====================\n");
        printk("      OTP IS NOT CONFIGURED CORECTLY!\n");
        printk("          Running with NO AVS \n");
        printk("=====================================================\n");
#if 0
        while (1);
#else
        return;
#endif
    }

    StartDiscovery();
    printf_log("AVS:InitMemory\n");
    InitMemory();
    printf_log("AVS:MapSystem\n");
    MapSystem();
    dump_system_status();

    printf_log("AVS:Schedule tracking\n");
    avs_workqueue = create_workqueue("AVS_TRACKING");
    queue_delayed_work(avs_workqueue, &tracking_task, 5*CONFIG_HZ);
    set_wraparound_offset();
}
