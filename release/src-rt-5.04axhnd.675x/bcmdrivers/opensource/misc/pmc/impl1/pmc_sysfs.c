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

#include <linux/gfp.h>
#include <linux/kobject.h>
#include <linux/slab.h>
#include <linux/sysfs.h>
#include "pmc_drv.h"
#include "BPCM.h"
#include "bcm_ubus4.h"
#include <linux/delay.h>
#include <linux/irqflags.h>
#include <asm/io.h>
#include <linux/dma-mapping.h>

#ifdef PVTMON_REG
int mVolts = 0;
int dacValue = 0;
#endif

/* block common register templates */
static const struct attribute blockreg_attrs[] = {
        { "id",                  0644 }, /* offset = 0x00, actual offset =  0 */
        { "capabilities",        0644 }, /* offset = 0x04, actual offset =  1 */
#if defined(PMC_IMPL_3_X) || defined(PMC_ON_HOSTCPU)
        { "rsvd2",               0644 }, /* offset = 0x08, actual offset = 2 */
        { "rsvd3",               0644 }, /* offset = 0x0c, actual offset = 3 */
        { "control",             0644 }, /* offset = 0x10, actual offset = 4 */
        { "sr_control",          0644 }, /* offset = 0x14, actual offset = 5 */
        { "rsvd6",               0644 }, /* offset = 0x18, actual offset = 6 */
        { "rsvd7",               0644 }, /* offset = 0x1c, actual offset = 7 */
        { "client_sp0",          0644 }, /* offset = 0x20, actual offset = 8 */
        { "client_sp1",          0644 }, /* offset = 0x24, actual offset = 9 */
        { "client_sp2",          0644 }, /* offset = 0x28, actual offset = 10 */
        { "client_sp3",          0644 }, /* offset = 0x2c, actual offset = 11 */
        { "client_sp4",          0644 }, /* offset = 0x30, actual offset = 12 */ // 63146 bpcm_vdsl afe_config0
        { "client_sp5",          0644 }, /* offset = 0x34, actual offset = 13 */ // 63146 bpcm_vdsl afe_config1
        { "client_sp6",          0644 }, /* offset = 0x38, actual offset = 14 */
        { "client_sp7",          0644 }, /* offset = 0x3c, actual offset = 15 */
        { "client_sp8",          0644 }, /* offset = 0x40, actual offset = 16 */ // 63178 bpcm_vdsl mips_cntl
        { "client_sp9",          0644 }, /* offset = 0x44, actual offset = 17 */ // 63178 bpcm_vdsl phy_cntl
        { "client_sp10",         0644 }, /* offset = 0x48, actual offset = 18 */ // 63178 bpcm_vdsl afe_cntl
#else
        { "control",             0644 }, /* offset = 0x08, actual offset =  2 */
        { "status",              0644 }, /* offset = 0x0c, actual offset =  3 */
        { "rosc_control",        0644 }, /* offset = 0x10, actual offset =  4 */
        { "rosc_thresh_h",       0644 }, /* offset = 0x14, actual offset =  5 */
        { "rosc_thresh_s",       0644 }, /* offset = 0x18, actual offset =  6 */
        { "rosc_count",          0644 }, /* offset = 0x1c, actual offset =  7 */
        { "pwd_control",         0644 }, /* offset = 0x20, actual offset =  8 */
        { "pwd_accum_control",   0644 }, /* offset = 0x24, actual offset =  9 */
        { "sr_control",          0644 }, /* offset = 0x28, actual offset = 10 */
        { "global_control",      0644 }, /* offset = 0x2c, actual offset = 11 */
        { "misc_control",        0644 }, /* offset = 0x30, actual offset = 12 */
        { "rsvd13",              0644 }, /* offset = 0x34, actual offset = 13 */
        { "rsvd14",              0644 }, /* offset = 0x38, actual offset = 14 */
        { "rsvd15",              0644 }, /* offset = 0x3c, actual offset = 15 */
#endif
};

/* block common register templates */
static const struct attribute zonereg_attrs[] = {
        { "control",             0644 },
        { "config1",             0644 },
        { "config2",             0644 },
        { "freq_scalar_control", 0644 },
};

/* attribute subclass with device and zone */
struct bpcm_attribute {
        struct attribute attr;
        ssize_t (*show)(struct kobject *kobj, struct kobj_attribute *attr,
                        char *buf);
        ssize_t (*store)(struct kobject *kobj, struct kobj_attribute *attr,
                         const char *buf, size_t count);
        int devno;
        int zoneno;
        int islandno;
        int regno;
#if defined(PMC_UCBID)
        int ucbid;
#endif
};

static ssize_t power_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
        struct bpcm_attribute *pmc = (struct bpcm_attribute *) attr;
        unsigned zone = pmc->zoneno >= 0 ? pmc->zoneno : 0;
        BPCM_PWR_ZONE_N_CONTROL control;
        int rc;

        rc = ReadZoneRegister(pmc->devno, zone, BPCMZoneRegOffset(control), &control.Reg32);
        if (rc)
                return sprintf(buf, "name %s, dev %d, error %x\n", pmc->attr.name, pmc->devno, rc);
        if (control.Reg32 == 0xdeaddead)
                return sprintf(buf, "%x\n", (unsigned) control.Reg32);
        return sprintf(buf, "%d\n", control.Bits.pwr_on_state);
}

static ssize_t power_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
        struct bpcm_attribute *pmc = (struct bpcm_attribute *) attr;
        unsigned value;

        if (kstrtouint(buf, 0, &value)) {
                printk("%s: not a number\n", buf);
                return -EEXIST;
        }

        if (value) {
                if (pmc->zoneno < 0)
                {
                        PowerOnDevice(pmc->devno);
#if defined(PMC_UCBID)
                        ubus_register_port(pmc->ucbid & UCBID_MASK);
                        if (pmc->ucbid & UCB_HAS_MST_SLV)
                            ubus_register_port((pmc->ucbid & UCBID_MASK)+1);
#endif
                }
                else
                        PowerOnZone(pmc->devno, pmc->zoneno);
        } else {
                if (pmc->zoneno < 0)
                {
#if defined(PMC_UCBID)
                        ubus_deregister_port(pmc->ucbid);
                        if (pmc->ucbid & UCB_HAS_MST_SLV)
                            ubus_deregister_port((pmc->ucbid & UCBID_MASK)+1);
#endif
                        PowerOffDevice(pmc->devno, 0); // dev, repower
                }
                else
                        PowerOffZone(pmc->devno, pmc->zoneno);
        }
        return count;
}

static ssize_t reset_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
        struct bpcm_attribute *pmc = (struct bpcm_attribute *) attr;
        unsigned zone = pmc->zoneno >= 0 ? pmc->zoneno : 0;
        BPCM_PWR_ZONE_N_CONTROL control;
        int rc;

        rc = ReadZoneRegister(pmc->devno, zone, BPCMZoneRegOffset(control), &control.Reg32);
        if (rc)
                return sprintf(buf, "name %s, dev %d, error %x\n", pmc->attr.name, pmc->devno, rc);
        if (control.Reg32 == 0xdeaddead)
                return sprintf(buf, "%x\n", (unsigned) control.Reg32);
        return sprintf(buf, "%d\n", control.Bits.reset_state);
}

static ssize_t reset_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
        struct bpcm_attribute *pmc = (struct bpcm_attribute *) attr;
        unsigned value;

        if (kstrtouint(buf, 0, &value)) {
                printk("%s: not a number\n", buf);
                return -EEXIST;
        }
        if (value) {
                if (pmc->zoneno < 0)
                        ResetDevice(pmc->devno);
                else
                        ResetZone(pmc->devno, pmc->zoneno);
        }
        return count;
}

/* node templates */
static const struct bpcm_attribute pseudo_attrs[] = {
        { { "power",       0644, }, power_show,       power_store },
        { { "reset",       0644, }, reset_show,       reset_store },
};

static ssize_t regs_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
        struct bpcm_attribute *pmc = (struct bpcm_attribute *) attr;
        uint32_t value;
        int rc;

        if (pmc->zoneno < 0)
                rc = ReadBPCMRegister(pmc->devno, pmc->regno, &value);
        else
                rc = ReadZoneRegister(pmc->devno, pmc->zoneno, pmc->regno, &value);
        if (rc == 0)
                return sprintf(buf, "%x\n", (unsigned) value);
        else
                return sprintf(buf, "name %s, dev %d, error %x\n", pmc->attr.name, pmc->devno, rc);
}

static ssize_t regs_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
        struct bpcm_attribute *pmc = (struct bpcm_attribute *) attr;
        uint32_t value;
        int rc;

        if (kstrtou32(buf, 0, (u32 *)&value)) {
                printk("%s: not a number\n", buf);
                return -EEXIST;
        }

        if (pmc->zoneno < 0)
                rc = WriteBPCMRegister(pmc->devno, pmc->regno, value);
        else
                rc = WriteZoneRegister(pmc->devno, pmc->zoneno, pmc->regno, value);
        if (rc)
                return 0;
        return count;
}

static int pmc_sysfs_init_zones(const struct bpcm_device *dev, struct kobject *kobj)
{
        unsigned z;
        unsigned r;
        int error;

        for (z = 0; z < dev->zones; z++) {
                unsigned char name[8];
                struct kobject *zoneobj;
                struct bpcm_attribute *pmcattr;

                /* create subdirectory for each zone */
                snprintf(name, sizeof name, "%s%d", "zone", z);
                zoneobj = kobject_create_and_add(name, kobj);
                if (zoneobj == 0)
                        return -ENOENT;

                /* create zone's power and reset nodes */
                r = ARRAY_SIZE(pseudo_attrs);
                pmcattr = kmalloc(r * sizeof *pmcattr, GFP_KERNEL);
                while (r--) {
                        pmcattr[r] = pseudo_attrs[r];
                        sysfs_attr_init(&pmcattr[r].attr);
                        pmcattr[r].devno = dev->devno;
                        pmcattr[r].zoneno = z;
                        pmcattr[r].regno = -1;
                        error = sysfs_create_file(zoneobj, (struct attribute *)&pmcattr[r]);
                }

                /* create some register nodes */
                r = ARRAY_SIZE(zonereg_attrs);
                pmcattr = kmalloc(r * sizeof *pmcattr, GFP_KERNEL);
                while (r--) {
                        pmcattr[r].attr = zonereg_attrs[r];
                        sysfs_attr_init(&pmcattr[r].attr);
                        pmcattr[r].store = regs_store;
                        pmcattr[r].show = regs_show;
                        pmcattr[r].devno = dev->devno;
                        pmcattr[r].zoneno = z;
                        pmcattr[r].regno = r;
                        error = sysfs_create_file(zoneobj, (struct attribute *)&pmcattr[r]);
                }
        }

        return error;
}

static int pmc_sysfs_init_block(const struct bpcm_device *dev, struct kobject *kobj)
{
        struct bpcm_attribute *pmcattr;
        struct kobject *blkobj;
        unsigned r;
        int error;

        /* create subdirectory for each block */
        blkobj = kobject_create_and_add(dev->name, kobj);
        if (blkobj == 0)
                return -ENOMEM;

        /* create block's power and reset nodes */
        if (dev->zones) {
                r = ARRAY_SIZE(pseudo_attrs);
                pmcattr = kmalloc(r * sizeof *pmcattr, GFP_KERNEL);
                while (r--) {
                        pmcattr[r] = pseudo_attrs[r];
                        sysfs_attr_init(&pmcattr[r].attr);
                        pmcattr[r].devno = dev->devno;
#if defined(PMC_UCBID)
                        pmcattr[r].ucbid = dev->ucbid;
#endif
                        pmcattr[r].zoneno = -1;
                        pmcattr[r].regno = -1;
                        error = sysfs_create_file(blkobj, (struct attribute *)&pmcattr[r]);
                }
        }

        /* create some register nodes */
        r = ARRAY_SIZE(blockreg_attrs);
        pmcattr = kmalloc(r * sizeof *pmcattr, GFP_KERNEL);
        while (r--) {
                pmcattr[r].attr = blockreg_attrs[r];
                sysfs_attr_init(&pmcattr[r].attr);
                pmcattr[r].store = regs_store;
                pmcattr[r].show = regs_show;
                pmcattr[r].devno = dev->devno;
                pmcattr[r].zoneno = -1;
                pmcattr[r].regno = r;
                error = sysfs_create_file(blkobj, (struct attribute *)&pmcattr[r]);
        }

        return pmc_sysfs_init_zones(dev, blkobj);
}

static ssize_t revision_show(struct kobject *kobj, struct kobj_attribute *kattr, char *buf)
{
        struct attribute *attr = (struct attribute *)kattr;
        int revision, change;
        int rc;

        rc = GetRevision(&change, &revision);
        if (rc != kPMC_NO_ERROR)
                return sprintf(buf, "name %s, error %x\n", attr->name, rc);
        return sprintf(buf, "%x-%d\n", revision, change);
}

// append new valid value into the buf and return the final len
static ssize_t pvtmon_convert(char *buf, int sel, int value)
{
	static const char *name[] = {
		[kTEMPERATURE] = "DieTemp",
		[kV_0p85_0]    = "V0.85_1",
		[kV_0p85_1]    = "V0.85_2",
		[kV_VIN]       = "VIN",
		[kV_1p00_1]    = "V1.0",
		[kV_1p80]      = "V1.8",
		[kV_3p30]      = "V3.3",
		[kTEST]        = "Vtest",
	};
	int v, i, f, len = 0;

	while (*buf) {
		len++;
		buf++;
	}

	if (sel < kTEMPERATURE || sel > kTEST)
		return len;

	v = pmc_convert_pvtmon(sel, value);
	i = v / 1000;
	f = (v > 0 ? v : -v) % 1000;

	return len + sprintf(buf, "%s: %d.%03d %c\n",
			name[sel], i, f, sel ? 'V' : 'C');
}

static ssize_t pvtmon_show(struct kobject *kobj, struct kobj_attribute *kattr, char *buf)
{
        struct bpcm_attribute *pmc = (struct bpcm_attribute *) kattr;
        int rc, value;

        rc = GetPVT(pmc->regno, pmc->islandno, &value);
        if (rc != kPMC_NO_ERROR)
                return sprintf(buf, "name %s, error %x\n", pmc->attr.name, rc);

        return pvtmon_convert(buf, pmc->regno, value);
}

static ssize_t clock_show(struct kobject *kobj, struct kobj_attribute *kattr, char *buf)
{
#if defined(__AARCH64EL__)
        unsigned long time1, time2, ctrl, flags = 0;
        /* Enable cycle counter */
        asm volatile("MRS %0, PMCNTENSET_EL0" : "=r" (ctrl));
        ctrl |= 0x80000000;
        asm volatile("MSR PMCNTENSET_EL0, %0" :: "r" (ctrl));

        /* Enable PMU */
        ctrl = 0x1;
        asm volatile("MSR PMCR_EL0, %0" :: "r" (ctrl));

        /* Count CPU cycle for 1 ms to get the CPU frequency */
        local_irq_save(flags);
        asm volatile("MRS %0, PMCCNTR_EL0" : "=r" (time1));
        mdelay(1);
        asm volatile("MRS %0, PMCCNTR_EL0" : "=r" (time2));
        local_irq_restore(flags);

        return sprintf (buf, "CPU Frequency %d MHz\n", (unsigned int)(time2 - time1) / 1000);
#else
        return 0;
#endif
}

static ssize_t clock_set(struct kobject *kobj, struct kobj_attribute *kattr, const char *buf, size_t count)
{
#if defined(PMC_CLOCKSET_SUPPORT)
        unsigned mdiv0;
        PLL_CHCFG_REG pll_ch01_cfg;

        if (kstrtouint(buf, 0, &mdiv0)) {
                printk("%s: not a number\n", buf);
                return -EEXIST;
        }

        ReadBPCMRegister(PMB_ADDR_BIU_PLL, PLLBPCMRegOffset(ch01_cfg), &pll_ch01_cfg.Reg32);

        if (mdiv0 == 0) {
                printk("CPU is running with clock divider %d\n", pll_ch01_cfg.Bits.mdiv0);
                return count;
        }

        if (pll_ch01_cfg.Bits.mdiv0 != mdiv0) {
                pll_ch01_cfg.Bits.mdiv0 = mdiv0;
                WriteBPCMRegister(PMB_ADDR_BIU_PLL, PLLBPCMRegOffset(ch01_cfg), pll_ch01_cfg.Reg32);
                mdelay(1);
                pll_ch01_cfg.Bits.mdiv_override0 = 1;
                WriteBPCMRegister(PMB_ADDR_BIU_PLL, PLLBPCMRegOffset(ch01_cfg), pll_ch01_cfg.Reg32);
                mdelay(1);
        }
#else
        printk ("Not implemented\n");
#endif
        return count;
}

#ifdef PVTMON_REG
static ssize_t volts_save (void *pvtreg)
{
        unsigned int ix, accum, data, sel = 3;

        pvtmon_regs *pvtmon = (pvtmon_regs*)pvtreg;
        /* Disable auto scan */
        pvtmon->ascan_config &= ~1;

        /* Select voltage + clock_en + power_on */
        iowrite32(sel << 4 | 1 << 3 | 1 << 1, &pvtmon->control);
        udelay(50);
        /* Select voltage + clock_en + power_on + un-reset */
        iowrite32(sel << 4 | 1 << 3 | 1 << 1 | 1 << 0, &pvtmon->control);
        /* Read once to make stale */
        ioread32(&pvtmon->adc_data);
        /* Confirm, not to read stale data */
        for (ix = 0; ix < 10; ix++) {
              do {
                    data = ioread32(&pvtmon->adc_data);
              }while (data & (1 << 15));  /* wait for non-stale data */
        }
        /* Read 32 times and then average them up */
        accum = 0;
        for (ix = 0; ix < 8; ix++) {
              do {
                    data = ioread32(&pvtmon->adc_data);
              }while (data & (1 << 15));  /* wait for non-stale data */
              accum += data & 0x3FF;      /* accumulate */
        }
        mVolts = ((accum / 8) * 880 * 10) / (7 * 1024);
        /* Enable auto scan */
        pvtmon->ascan_config |= 1;
        return 0;
}
#endif

static ssize_t volts_show (struct kobject *kobj, struct kobj_attribute *kattr, char *buf)
{
#ifdef PVTMON_REG
        return sprintf(buf, "Input voltage %d(dac) %d(mv)\n", dacValue, mVolts);
#else
        return 0;
#endif
}

static ssize_t volts_set (struct kobject *kobj, struct kobj_attribute *kattr, const char *buf, size_t count)
{
#ifdef PVTMON_REG
        pvtmon_regs *pvtmon;
        unsigned int address, value, cfg;
        char s_addr[16];
        char s_val[16];

        sscanf(buf, "%s %s", s_addr, s_val);
        kstrtouint (s_addr, 0, &address);
        kstrtouint (s_val, 0, &value);
        pvtmon = (pvtmon_regs *)ioremap_nocache(address, 0x100);

        dacValue = value;
        if (value) {
              /* Write the DAC code */
              iowrite32(value, &pvtmon->vref_data);
              /* Set PVTMON in AVS mode (3) to be able to control the DAC voltage */
              cfg = (ioread32(&pvtmon->cfg_lo) & ~(0x7 << 10)) | 3 << 10;
              iowrite32(cfg, &pvtmon->cfg_lo);
        }
        else{
              printk ("DAC value %d\n", ioread32(&pvtmon->vref_data));
        }

        volts_save((void*)pvtmon);
        iounmap((void*)pvtmon);
#endif
        return count;
}

#if defined (PMC_CPUTEMP_SUPPORT)
static ssize_t cputemp_show(struct kobject *kobj, struct kobj_attribute *kattr, char *buf)
{
	int v = readl(g_pmc->bac_cpu_base + BAC_CPU_THERM_OFFSET);

	if (!(v & (1<<31)))
		return sprintf(buf, "failed to get cpu_temp\n");

	v = pmc_convert_pvtmon(kTEMPERATURE, v & 0x3ff);
	return sprintf(buf, "cpu_temp: %d.%03d C\n",
			v / 1000, (v > 0 ? v : -v) % 1000);
}
#endif

#if defined(PMC_GETALL_RO_SUPPORT)
struct bpcm
{
    int addr;
    int bus;
};

char *common_csv_header = "Temp,VIN";
struct bpcm map[]={{37,0},{45,0},{48,0},{49,1},{50,1}, {51,1}, {52,1}, {53,1}, {54,1}, {55,1}, {56,1}, {57,1}, {58,1}, {59,1}, {60,0}, {61, 0}};
char *csv_header = "ORION_0(37)SVT,ORION_0(37)HVT,ORION_1(45)SVT,ORION_1(45)HVT,PERIPH(48)SVT,PERIPH(48)HVT,WAN(49)SVT,"            "WAN(49)HVT,XRDP(50)SVT,XRDP(50)HVT,XRDP_RC0(51)SVT,XRDP_RC0(51)HVT,XRDP_RC1(52)SVT,XRDP_RC1(52)HVT,"
            "XRDP_RC2(53)SVT,XRDP_RC2(53)HVT,XRDP_RC3(54)SVT,XRDP_RC3(54)HVT,XRDP_RC4(55)SVT,XRDP_RC4(55)HVT,"
            "XRDP_RC5(56)SVT,XRDP_RC5(56)HVT,XRDP_RC6(57)SVT,XRDP_RC6(57)HVT,XRDP_RC7(58)SVT,XRDP_RC7(58)HVT,"
            "PCIE(59)SVT,PCIE(59)HVT,USB(60)SVT,USB(60)HVT,MEMC(61)SVT,MEMC(61)HVT,"
            "SVT THRESH HI,SVT THRESH LO,HVT THRESH HI,HVT THRESH LO, LVT TRESH HI,LVT TRESH LO\n";
#define ROSC_COUNT_OFF 11
#define ROSC_CONFIG_OFF 8
static int event_counters_enabled = 0;
static ssize_t get_roscs(struct kobject *kobj, struct kobj_attribute *kattr, char *buf)
{
    int i;
    int rc;
    int temperature;
    int vin;
    int val, val0;
    int valin, valin0;
    uint32_t target;
    uint32_t num_bpcms = sizeof(map)/sizeof(struct bpcm);
    uint32_t thresh_s, thresh_h, thresh_l;

    if(!event_counters_enabled)
    {
        for (i=0; i < num_bpcms; i++)
        {
            target = 0xffff00ff;
            rc = WriteBPCMRegister((map[i].bus<<PMB_BUS_ID_SHIFT) | map[i].addr, ROSC_CONFIG_OFF, target);
        }
        event_counters_enabled = 1;

        sprintf(buf,"%s,%s",common_csv_header, csv_header);
    }
    else
    {
        sprintf(buf, "\n");
    }

    rc = ReadBPCMRegister((map[1].bus<<PMB_BUS_ID_SHIFT) | map[1].addr,  9, &thresh_h);
    rc = ReadBPCMRegister((map[0].bus<<PMB_BUS_ID_SHIFT) | map[0].addr, 10, &thresh_s);
    rc = ReadBPCMRegister((map[0].bus<<PMB_BUS_ID_SHIFT) | map[0].addr,  9, &thresh_l);
    rc = GetPVT(0, 0, &temperature);

    val = ((41335000 - (49055 * temperature))/100000);
    val0 = ((41335000 - (49055 * temperature))%100000);

    rc = GetPVT(3, 0, &vin);
    valin = (880 * vin * 10) / (7 * 1024)/1000;
    valin0 = (880 * vin * 10) / (7 * 1024)%1000;

    sprintf(buf, "%s%d.%03d,%d.%03d", buf,val, val0, valin, valin0);

    for (i=0; i < num_bpcms; i++)
    {
        target = 0;
        rc = ReadBPCMRegister((map[i].bus<<PMB_BUS_ID_SHIFT) | map[i].addr, ROSC_COUNT_OFF, &target);

        sprintf(buf, "%s,%04d,%04d", buf, target&0xffff,(target&0xffff0000)>>16 );
    }
    return sprintf(buf, "%s,%d,%d,%d,%d,%d,%d\n",buf,(thresh_s & 0xffff0000)>>16, thresh_s &
        0xffff, (thresh_h & 0xffff0000)>>16, thresh_h & 0xffff, (thresh_l & 0xffff0000)>>16, thresh_l & 0xffff);
}

static ssize_t get_all_roscs(struct kobject *kobj, struct kobj_attribute *kattr, char *buf)
{
    uint32_t ix, sh_ix;
    uint32_t phys = 0x4ffbc00;
    uint32_t *shmem = phys_to_virt(phys);
    uint32_t rc;

    rc = GetAllROs(phys);
    if (rc != kPMC_NO_ERROR)
    {
        return rc;  
    }

    sprintf(buf, "PVTMON: \n");
    for (sh_ix = 0; sh_ix < 7; sh_ix++)
    {
        pvtmon_convert(buf, sh_ix, shmem[sh_ix]);
    }

    sprintf(buf, "%s Central ROs: \n", buf);
    for (ix = 0; ix < 36; ix++, sh_ix++)
    {
        sprintf(buf, "%s %d) %u;\n", buf, ix, shmem[sh_ix]);
    }

    sprintf(buf, "%s ARS ROs: \n", buf);
    ix = 0;
    while(shmem[sh_ix] != 0xfffffffe)
    {
        sprintf(buf, "%s %d) SVT %d, HVT %d;\n", buf, ix++, shmem[sh_ix], shmem[sh_ix + 1]);
        sh_ix +=2;
    }
    return sprintf(buf, "%s DONE\n", buf);
}

#endif
static const struct bpcm_attribute root_attrs[] = {
        { { "revision",    0444, }, revision_show,    NULL          },
        { { "select0",     0444, }, pvtmon_show,      NULL, 0, 0, 0, kTEMPERATURE },
        { { "select1",     0444, }, pvtmon_show,      NULL, 0, 0, 0, kV_0p85_0 },
        { { "select2",     0444, }, pvtmon_show,      NULL, 0, 0, 0, kV_0p85_1 },
        { { "select3",     0444, }, pvtmon_show,      NULL, 0, 0, 0, kV_VIN },
        { { "select4",     0444, }, pvtmon_show,      NULL, 0, 0, 0, kV_1p00_1 },
        { { "select5",     0444, }, pvtmon_show,      NULL, 0, 0, 0, kV_1p80 },
        { { "select6",     0444, }, pvtmon_show,      NULL, 0, 0, 0, kV_3p30 },
        { { "select9",     0444, }, pvtmon_show,      NULL, 0, 0, 1, kV_VIN },
        { { "select10",    0644, }, clock_show,       clock_set, 0, 0, 0, 0 },
        { { "select11",    0644, }, volts_show,       volts_set, 0, 0, 0, 0 },
#if defined (PMC_CPUTEMP_SUPPORT)
        { { "cpu_temp",    0644, }, cputemp_show,     NULL, 0, 0, 0, 0 },
#endif
#if defined(PMC_GETALL_RO_SUPPORT)
        { { "get_roscs",   0444, }, get_roscs ,        NULL, 0, 0, 0, 0 },
        { { "get_all_roscs",   0444, }, get_all_roscs , NULL, 0, 0, 0, 0 },
#endif
};

#if defined(PMC_IMPL_3_X) || defined(PMC_ON_HOSTCPU)
#ifdef PMC_LOG_IN_DTCM
#define PMC_LOG_BUF_SZ	CFG_BOOT_PMC_LOG_SIZE
#define get_pmc_log_start()  phys_to_virt(CFG_BOOT_PMC_LOG_ADDR)
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#ifndef time_is_after_jiffies64
#define time_is_after_jiffies64(a) time_before64(get_jiffies_64(), a)
#endif
#elif defined(PMC_ON_HOSTCPU) // #ifdef PMC_LOG_IN_DTCM
char *get_pmc_log_start(void);
#define PMC_LOG_BUF_SZ 0x4000	
#else
#define get_pmc_log_start() NULL
#define PMC_LOG_BUF_SZ	128
#endif // #ifdef PMC_LOG_IN_DTCM
static ssize_t pmc_log_read(struct file *file, struct kobject *kobj,
		struct bin_attribute *battr, char *buf, loff_t pos, size_t cnt)
{
	char *virt = get_pmc_log_start();

#ifdef PMC_LOG_IN_DTCM
	unsigned short *log_len = (unsigned short *) virt;
	u64 stop_time;
	volatile Pmc *pmc = (volatile Pmc *)g_pmc->pmc_base;

	if (!*log_len) {
		pmc->ctrl.hostMboxOut = 1; // request sync dtcm log
		stop_time = get_jiffies_64() + msecs_to_jiffies(2000);
		while (time_is_after_jiffies64(stop_time))
			pmc_save_log_item();
		pmc->ctrl.hostMboxOut = 0; // ignore dtcm log
	}

	*log_len = MIN(*log_len, CFG_BOOT_PMC_LOG_SIZE - 2);
	if (pos < 0 || pos >= *log_len) {
		*log_len = 0;
		return 0;
	}
	cnt = MIN(cnt, *log_len - pos);
	memcpy(buf, virt + sizeof(*log_len) + pos, cnt);
#else // #ifdef PMC_LOG_IN_DTCM

	if (virt) {
		memcpy(buf, virt + pos, cnt);
	} else {
		memset(buf, 0, cnt);
		if (pos == 0)
			snprintf(buf, cnt, "Log is not enabled in PMC\n");
	}
#endif // #ifdef PMC_LOG_IN_DTCM
	return cnt;
}

#if defined(PMC_ON_HOSTCPU)
void set_console_output(int value);

static ssize_t pmc_log_write(struct file *file, struct kobject *kobj,
		struct bin_attribute *battr, char *buf, loff_t pos, size_t cnt)
{
        unsigned console_en;
        if (kstrtouint(buf, 0, &console_en)) {
                printk("%s: not a number\n", buf);
                return -EEXIST;
        }

        set_console_output(console_en);
        return cnt;
};

static BIN_ATTR_RW(pmc_log, PMC_LOG_BUF_SZ);
#else
static BIN_ATTR_RO(pmc_log, PMC_LOG_BUF_SZ);
#endif
#endif

#ifdef CONFIG_PM
extern struct kobject *power_kobj; 
#else
struct kobject *power_kobj = NULL;
#endif

static int __init pmc_sysfs_init(void)
{
        struct bpcm_attribute *pmcattr;
        struct kobject *bpcmkobj;
        unsigned d;
        int error;

        /* create power object ourselves? */
        if (!power_kobj) {
                power_kobj = kobject_create_and_add("power", NULL);
                if (!power_kobj)
                        return -ENOENT;
        }

        /* create bpcm subdirectory */
        bpcmkobj = kobject_create_and_add("bpcm", power_kobj);
        if (bpcmkobj == 0)
                return -ENOMEM;

        /* create root nodes */
        d = ARRAY_SIZE(root_attrs);
        pmcattr = kmalloc(d * sizeof *pmcattr, GFP_KERNEL);
        while (d--) {
                pmcattr[d] = root_attrs[d];
                sysfs_attr_init(&pmcattr[d].attr);
                error = sysfs_create_file(bpcmkobj, (struct attribute *)&pmcattr[d]);
        }
#if defined(PMC_IMPL_3_X) || defined(PMC_ON_HOSTCPU) 
        sysfs_create_bin_file(bpcmkobj, &bin_attr_pmc_log);
#endif

        for (d = 0; d < ARRAY_SIZE(bpcm_devs); d++)
                pmc_sysfs_init_block(&bpcm_devs[d], bpcmkobj);
        return 0;
}

late_initcall(pmc_sysfs_init);
