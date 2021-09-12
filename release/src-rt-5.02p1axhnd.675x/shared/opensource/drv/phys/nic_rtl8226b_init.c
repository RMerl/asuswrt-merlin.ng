
#include "os_dep.h"
#include "nic_rtl8226.h"
#include "bcm_gpio.h"

extern int _bus_read(uint32_t addr, uint16_t dev, uint16_t reg, uint16_t *val);
extern int _bus_write(uint32_t addr, uint16_t dev, uint16_t reg, uint16_t val);

#define MmdPhyRead _bus_read
#define MmdPhyWrite _bus_write

static const MMD_REG Rtl8226b_n0_ramcode[] =
{
	{ 31, 0xa436, 0XA016, },
    { 31, 0xa438, 0X0000, },
    { 31, 0xa436, 0XA012, },
    { 31, 0xa438, 0X0000, },
    { 31, 0xa436, 0XA014, },
    { 31, 0xa438, 0X1800, },
    { 31, 0xa438, 0X8010, },
    { 31, 0xa438, 0X1800, },
    { 31, 0xa438, 0X801a, },
    { 31, 0xa438, 0X1800, },
    { 31, 0xa438, 0X8024, },
    { 31, 0xa438, 0X1800, },
    { 31, 0xa438, 0X802f, },
    { 31, 0xa438, 0X1800, },
    { 31, 0xa438, 0X8050, },
    { 31, 0xa438, 0X1800, },
    { 31, 0xa438, 0X8050, },
    { 31, 0xa438, 0X1800, },
    { 31, 0xa438, 0X8050, },
    { 31, 0xa438, 0X1800, },
    { 31, 0xa438, 0X8050, },
    { 31, 0xa438, 0Xd093, },
    { 31, 0xa438, 0Xd1c4, },
    { 31, 0xa438, 0X1000, },
    { 31, 0xa438, 0X135c, },
    { 31, 0xa438, 0Xd704, },
    { 31, 0xa438, 0X5fbc, },
    { 31, 0xa438, 0Xd504, },
    { 31, 0xa438, 0Xc9f1, },
    { 31, 0xa438, 0X1800, },
    { 31, 0xa438, 0X0fc9, },
    { 31, 0xa438, 0Xbb50, },
    { 31, 0xa438, 0Xd505, },
    { 31, 0xa438, 0Xa202, },
    { 31, 0xa438, 0Xd504, }, 
    { 31, 0xa438, 0X8c0f, },
    { 31, 0xa438, 0Xd500, },
    { 31, 0xa438, 0X1000, },
    { 31, 0xa438, 0X1519, },
    { 31, 0xa438, 0X1800, },
    { 31, 0xa438, 0X1548, },
    { 31, 0xa438, 0X2f70, },
    { 31, 0xa438, 0X802a, },
    { 31, 0xa438, 0X2f73, },
    { 31, 0xa438, 0X156a, },
    { 31, 0xa438, 0X1800, },
    { 31, 0xa438, 0X155c, },
    { 31, 0xa438, 0Xd505, },
    { 31, 0xa438, 0Xa202, },
    { 31, 0xa438, 0Xd500, },
    { 31, 0xa438, 0X1800, },
    { 31, 0xa438, 0X1551, },
    { 31, 0xa438, 0Xc0c1, },
    { 31, 0xa438, 0Xc0c0, },
    { 31, 0xa438, 0Xd05a, },
    { 31, 0xa438, 0Xd1ba, },
    { 31, 0xa438, 0Xd701, },
    { 31, 0xa438, 0X2529, },
    { 31, 0xa438, 0X022a, },
    { 31, 0xa438, 0Xd0a7, },
    { 31, 0xa438, 0Xd1b9, },
    { 31, 0xa438, 0X1000, },
    { 31, 0xa438, 0X080e, },
    { 31, 0xa438, 0Xd701, },
    { 31, 0xa438, 0X408b, },
    { 31, 0xa438, 0X1000, },
    { 31, 0xa438, 0X0a65, },
    { 31, 0xa438, 0Xf003, },
    { 31, 0xa438, 0X1000, },
    { 31, 0xa438, 0X0a6b, },
    { 31, 0xa438, 0Xd701, },
    { 31, 0xa438, 0X1000, },
    { 31, 0xa438, 0X0920, },
    { 31, 0xa438, 0X1000, },
    { 31, 0xa438, 0X0915, },  
    { 31, 0xa438, 0X1000, },
    { 31, 0xa438, 0X0909, },
    { 31, 0xa438, 0X228f, },
    { 31, 0xa438, 0X8038, },
    { 31, 0xa438, 0X9801, },
    { 31, 0xa438, 0Xd71e, },
    { 31, 0xa438, 0X5d81, },
    { 31, 0xa438, 0Xd701, },
    { 31, 0xa438, 0X1800, },
    { 31, 0xa438, 0X022a, },    
    { 31, 0xa436, 0XA026, },
    { 31, 0xa438, 0Xffff, },
    { 31, 0xa436, 0XA024, },
    { 31, 0xa438, 0Xffff, },
    { 31, 0xa436, 0XA022, },
    { 31, 0xa438, 0Xffff, },
    { 31, 0xa436, 0XA020, },
    { 31, 0xa438, 0Xffff, },
    { 31, 0xa436, 0XA006, },
    { 31, 0xa438, 0X020a, },
    { 31, 0xa436, 0XA004, },
    { 31, 0xa438, 0X155b, },
    { 31, 0xa436, 0XA002, },
    { 31, 0xa438, 0X1542, },
    { 31, 0xa436, 0XA000, },
    { 31, 0xa438, 0X0fc7, },
    { 31, 0xa436, 0XA008, },
    { 31, 0xa438, 0X0f00, },


};


static const MMD_REG Rtl8226b_n1_ramcode[] =
{
    { 31, 0xa436, 0XA016, },
    { 31, 0xa438, 0X0010, },
    { 31, 0xa436, 0XA012, },
    { 31, 0xa438, 0X0000, },
    { 31, 0xa436, 0XA014, },
    { 31, 0xa438, 0X1800, },
    { 31, 0xa438, 0X8010, },
    { 31, 0xa438, 0X1800, },
    { 31, 0xa438, 0X801d, },
    { 31, 0xa438, 0X1800, },
    { 31, 0xa438, 0X802c, }, 
    { 31, 0xa438, 0X1800, }, 
    { 31, 0xa438, 0X802c, },
    { 31, 0xa438, 0X1800, },
    { 31, 0xa438, 0X802c, },
    { 31, 0xa438, 0X1800, },
    { 31, 0xa438, 0X802c, },
    { 31, 0xa438, 0X1800, },
    { 31, 0xa438, 0X802c, },
    { 31, 0xa438, 0X1800, },
    { 31, 0xa438, 0X802c, }, 
    { 31, 0xa438, 0Xd700, }, 
    { 31, 0xa438, 0X6090, },
    { 31, 0xa438, 0X60d1, },
    { 31, 0xa438, 0Xc95c, },
    { 31, 0xa438, 0Xf007, },
    { 31, 0xa438, 0X60b1, },
    { 31, 0xa438, 0Xc95a, },
    { 31, 0xa438, 0Xf004, },
    { 31, 0xa438, 0Xc956, },
    { 31, 0xa438, 0Xf002, }, 
    { 31, 0xa438, 0Xc94e, },
    { 31, 0xa438, 0X1800, },
    { 31, 0xa438, 0X00cd, },
    { 31, 0xa438, 0Xd700, },
    { 31, 0xa438, 0X6090, },
    { 31, 0xa438, 0X60d1, },
    { 31, 0xa438, 0Xc95c, },
    { 31, 0xa438, 0Xf007, },
    { 31, 0xa438, 0X60b1, },
    { 31, 0xa438, 0Xc95a, }, 
    { 31, 0xa438, 0Xf004, },
    { 31, 0xa438, 0Xc956, },
    { 31, 0xa438, 0Xf002, },
    { 31, 0xa438, 0Xc94e, },
    { 31, 0xa438, 0X1000, },
    { 31, 0xa438, 0X022a, },
    { 31, 0xa438, 0X1800, },
    { 31, 0xa438, 0X0132, },
    { 31, 0xa436, 0XA08E, },
    { 31, 0xa438, 0Xffff, },
    { 31, 0xa436, 0XA08C, },
    { 31, 0xa438, 0Xffff, },
    { 31, 0xa436, 0XA08A, },
    { 31, 0xa438, 0Xffff, },
    { 31, 0xa436, 0XA088, },
    { 31, 0xa438, 0Xffff, },
    { 31, 0xa436, 0XA086, },
    { 31, 0xa438, 0Xffff, },
    { 31, 0xa436, 0XA084, },
    { 31, 0xa438, 0Xffff, },
    { 31, 0xa436, 0XA082, },
    { 31, 0xa438, 0X012f, },
    { 31, 0xa436, 0XA080, },
    { 31, 0xa438, 0X00cc, },
    { 31, 0xa436, 0XA090, },
    { 31, 0xa438, 0X0103, },
};


static const MMD_REG Rtl8226b_n2_ramcode[] =
{
    { 31, 0xa436, 0XA016, },
    { 31, 0xa438, 0X0020, },
    { 31, 0xa436, 0XA012, },
    { 31, 0xa438, 0X0000, },
    { 31, 0xa436, 0XA014, },
    { 31, 0xa438, 0X1800, },
    { 31, 0xa438, 0X8010, },
    { 31, 0xa438, 0X1800, },
    { 31, 0xa438, 0X801e, },
    { 31, 0xa438, 0X1800, },
    { 31, 0xa438, 0X8026, },
    { 31, 0xa438, 0X1800, },
    { 31, 0xa438, 0X8026, },
    { 31, 0xa438, 0X1800, },
    { 31, 0xa438, 0X8026, },
    { 31, 0xa438, 0X1800, },
    { 31, 0xa438, 0X8026, },
    { 31, 0xa438, 0X1800, },
    { 31, 0xa438, 0X8026, },
    { 31, 0xa438, 0X1800, },
    { 31, 0xa438, 0X8026, },
    { 31, 0xa438, 0Xd107, },
    { 31, 0xa438, 0Xd042, },
    { 31, 0xa438, 0Xa404, },
    { 31, 0xa438, 0Xd700, },
    { 31, 0xa438, 0X5ff4, },
    { 31, 0xa438, 0X8280, },
    { 31, 0xa438, 0Xd700, },
    { 31, 0xa438, 0X6065, },
    { 31, 0xa438, 0Xd11d, },
    { 31, 0xa438, 0Xf002, },
    { 31, 0xa438, 0Xd123, },
    { 31, 0xa438, 0Xd040, },
    { 31, 0xa438, 0X1800, },
    { 31, 0xa438, 0X077f, },
    { 31, 0xa438, 0Xd104, },
    { 31, 0xa438, 0Xd040, },
    { 31, 0xa438, 0X0cf0, },
    { 31, 0xa438, 0X0c50, },
    { 31, 0xa438, 0Xd700, },
    { 31, 0xa438, 0X5ff4, },
    { 31, 0xa438, 0X1800, },
    { 31, 0xa438, 0X0a2e, },
    { 31, 0xa436, 0XA10E, },
    { 31, 0xa438, 0Xffff, },
    { 31, 0xa436, 0XA10C, },
    { 31, 0xa438, 0Xffff, },
    { 31, 0xa436, 0XA10A, },
    { 31, 0xa438, 0Xffff, },
    { 31, 0xa436, 0XA108, },
    { 31, 0xa438, 0Xffff, },
    { 31, 0xa436, 0XA106, },
    { 31, 0xa438, 0Xffff, },
    { 31, 0xa436, 0XA104, },
    { 31, 0xa438, 0Xffff, },
    { 31, 0xa436, 0XA102, },
    { 31, 0xa438, 0X0a2c, },
    { 31, 0xa436, 0XA100, },
    { 31, 0xa438, 0X077e, },
    { 31, 0xa436, 0XA110, },
    { 31, 0xa438, 0X0003, },    
};

static const MMD_REG Rtl8226b_uc2_ramcode[] =
{
    { 31, 0xa436, 0Xb87c, },
    { 31, 0xa438, 0X8625, },
    { 31, 0xa436, 0Xb87e, },
    { 31, 0xa438, 0Xaf86, },
    { 31, 0xa438, 0X3daf, },
    { 31, 0xa438, 0X8689, },
    { 31, 0xa438, 0Xaf86, },
    { 31, 0xa438, 0X89af, },
    { 31, 0xa438, 0X8689, },
    { 31, 0xa438, 0Xaf86, },
    { 31, 0xa438, 0X89af, },
    { 31, 0xa438, 0X8689, },
    { 31, 0xa438, 0Xaf86, },
    { 31, 0xa438, 0X89af, },
    { 31, 0xa438, 0X8689, },
    { 31, 0xa438, 0Xbf86, },
    { 31, 0xa438, 0X49d7, },
    { 31, 0xa438, 0X0040, },
    { 31, 0xa438, 0X0277, },
    { 31, 0xa438, 0X7daf, },
    { 31, 0xa438, 0X2727, },
    { 31, 0xa438, 0X0000, },
    { 31, 0xa438, 0X7205, },
    { 31, 0xa438, 0X0000, },
    { 31, 0xa438, 0X7208, },
    { 31, 0xa438, 0X0000, },
    { 31, 0xa438, 0X71f3, },
    { 31, 0xa438, 0X0000, },
    { 31, 0xa438, 0X71f6, },
    { 31, 0xa438, 0X0000, },
    { 31, 0xa438, 0X7229, },
    { 31, 0xa438, 0X0000, },
    { 31, 0xa438, 0X722c, },
    { 31, 0xa438, 0X0000, },
    { 31, 0xa438, 0X7217, },
    { 31, 0xa438, 0X0000, },
    { 31, 0xa438, 0X721a, },
    { 31, 0xa438, 0X0000, },
    { 31, 0xa438, 0X721d, },
    { 31, 0xa438, 0X0000, },
    { 31, 0xa438, 0X7211, },
    { 31, 0xa438, 0X0000, },
    { 31, 0xa438, 0X7220, },
    { 31, 0xa438, 0X0000, },
    { 31, 0xa438, 0X7214, },
    { 31, 0xa438, 0X0000, },
    { 31, 0xa438, 0X722f, },
    { 31, 0xa438, 0X0000, },
    { 31, 0xa438, 0X7223, },
    { 31, 0xa438, 0X0000, },
    { 31, 0xa438, 0X7232, },
    { 31, 0xa438, 0X0000, },
    { 31, 0xa438, 0X7226, },
    { 31, 0xa436, 0Xb85e, },
    { 31, 0xa438, 0X271E, },
    { 31, 0xa436, 0Xb860, },
    { 31, 0xa438, 0Xffff, },
    { 31, 0xa436, 0Xb862, },
    { 31, 0xa438, 0Xffff, },
    { 31, 0xa436, 0Xb864, },
    { 31, 0xa438, 0Xffff, },
    { 31, 0xa436, 0Xb886, },
    { 31, 0xa438, 0Xffff, },
    { 31, 0xa436, 0Xb888, },
    { 31, 0xa438, 0Xffff, },
    { 31, 0xa436, 0Xb88a, },
    { 31, 0xa438, 0Xffff, },
    { 31, 0xa436, 0Xb88c, },
    { 31, 0xa438, 0Xffff, },
    { 31, 0xa436, 0Xb838, },
    { 31, 0xa438, 0X0001, },

};

static const MMD_REG Rtl8226b_uc_ramcode[] =
{
    
};

static const MMD_REG Rtl8226b_data_ramcode[] =
{
   
};

static const MMD_REG Rtl8226b_normal_patch[] =
{
    { 31, 0xa46a, 0X0302, },// Lock Main
    { 31, 0xac46, 0Xb794, },
    { 31, 0xa412, 0X0200, },
    { 31, 0xa5d4, 0X0081, },
    { 31, 0xad30, 0X0a57, },
    { 31, 0xad30, 0X0a55, },
    { 31, 0xb87c, 0X80f5, },
    { 31, 0xb87e, 0X760e, },
    { 31, 0xb87c, 0X8107, },
    { 31, 0xb87e, 0X360e, },
    { 31, 0xb87c, 0X8551, },
    { 31, 0xb87e, 0X080e, },
    { 31, 0xbf00, 0Xb202, },
    { 31, 0xbf46, 0X0300, },
    { 31, 0xa436, 0X8044, },
    { 31, 0xa438, 0X240f, },
    { 31, 0xa436, 0X804a, },
    { 31, 0xa438, 0X240f, },
    { 31, 0xa436, 0X8050, },
    { 31, 0xa438, 0X240f, },
    { 31, 0xa436, 0X8056, },
    { 31, 0xa438, 0X240f, },
    { 31, 0xa436, 0X805c, },
    { 31, 0xa438, 0X240f, },
    { 31, 0xa436, 0X8062, },
    { 31, 0xa438, 0X240f, },
    { 31, 0xa436, 0X8068, },
    { 31, 0xa438, 0X240f, },
    { 31, 0xa436, 0X806e, },
    { 31, 0xa438, 0X240f, },
    { 31, 0xa436, 0X8074, },
    { 31, 0xa438, 0X240f, },
    { 31, 0xa436, 0X807a, },
    { 31, 0xa438, 0X240f, },
    { 31, 0xa46a, 0X0300, },// Release Lock Main 
};

static const MMD_REG Rtl8226b_isram_patch[] =
{
   
};

static BOOL
Rtl8226b_wait_for_bit(
    IN HANDLE hDevice,
    IN UINT16 dev,
    IN UINT16 addr,
    IN UINT16 mask,
    IN BOOL   set,
    IN UINT16 timeoutms)
{
    BOOL status = FAILURE;
    UINT16 phydata = 0;

    while (--timeoutms) {
        status = MmdPhyRead(hDevice, MMD_VEND2, addr, &phydata);
        if (FAILURE == status)
            goto exit;

        if (!set)
            phydata = ~phydata;

        if ((phydata & mask) == mask)
            return SUCCESS;

        udelay(1000);
    }

    printk("Timeout (dev=%02x addr=0x%02x mask=0x%02x timeout=%d)\n",
         dev, addr, mask, timeoutms);

exit:
    return FAILURE;
}


BOOLEAN
Rtl8226b_phy_init(
    IN HANDLE hDevice,
    IN PHY_LINK_ABILITY *pphylinkability,
    IN BOOL singlephy
    )
{
    BOOL status = FAILURE;
    UINT16 i = 0;   /* SW_SDK: use UINT16 instead of UINT8, for MMD_REG array may over 255 entries */
    UINT16 phydata = 0;
    UINT16 SerdesMode = 0;
    const UINT16 patchver = 0x0007, patchaddr = 0x8024;

    // Polling PHY Status
    status = Rtl8226b_wait_for_bit(hDevice, MMD_VEND2, 0xA420, 0x3, 1, 100);
    if (status != SUCCESS)
        goto exit;

    // MMD 31.0xA436[15:0] = 0x801E
    status = MmdPhyWrite(hDevice, MMD_VEND2, 0xA436, 0x801E);
    if (status != SUCCESS)
        goto exit;

    status = MmdPhyRead(hDevice, MMD_VEND2, 0xA438, &phydata);
    if (status != SUCCESS)
        goto exit;

    // Already patched.
    if (phydata == patchver)
    {
		status = SUCCESS;
        goto exit;
    }
    else
    {
        // Patch request & wait patch_rdy (for normal patch flow - Driver Initialize)
        // MMD 31.0xB820[4] = 1'b1     //(patch request)
        status = MmdPhyRead(hDevice, MMD_VEND2, 0xB820, &phydata);
        if (status != SUCCESS)
            goto exit;

        phydata |= BIT_4;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xB820, phydata);
        if (status != SUCCESS)
            goto exit;

        //wait for patch ready = 1 (MMD 31.0xB800[6])
        status = Rtl8226b_wait_for_bit(hDevice, MMD_VEND2, 0xB800, BIT_6, 1, 100);
        if (status != SUCCESS)
            goto exit;

        //Set patch_key & patch_lock
        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xA436, patchaddr);
        if (status != SUCCESS)
            goto exit;

        // MMD 31.0xA438[15:0] = 0x8601
        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xA438, 0x3701);
        if (status != SUCCESS)
            goto exit;

        // MMD 31.0xA436[15:0] = 0xB82E
        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xA436, 0xB82E);
        if (status != SUCCESS)
            goto exit;

        // MMD 31.0xA438[15:0] = 0x0001
        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xA438, 0x0001);
        if (status != SUCCESS)
            goto exit;

        // NC & UC patch
        status = MmdPhyRead(hDevice, MMD_VEND2, 0xB820, &phydata);
        if (status != SUCCESS)
            goto exit;

        phydata |= BIT_7;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xB820, phydata);
        if (status != SUCCESS)
            goto exit;

        // patch nc0
        for(i=0; i<sizeof(Rtl8226b_n0_ramcode)/sizeof(MMD_REG); i++)
        {
            status = MmdPhyWrite(hDevice, Rtl8226b_n0_ramcode[i].dev, Rtl8226b_n0_ramcode[i].addr, Rtl8226b_n0_ramcode[i].value);
            if (status != SUCCESS)
                goto exit;
        }

        // patch nc1
        for(i=0; i<sizeof(Rtl8226b_n1_ramcode)/sizeof(MMD_REG); i++)
        {
            status = MmdPhyWrite(hDevice, Rtl8226b_n1_ramcode[i].dev, Rtl8226b_n1_ramcode[i].addr, Rtl8226b_n1_ramcode[i].value);
            if (status != SUCCESS)
                goto exit;
        }


        // patch nc2
        for(i=0; i<sizeof(Rtl8226b_n2_ramcode)/sizeof(MMD_REG); i++)
        {
            status = MmdPhyWrite(hDevice, Rtl8226b_n2_ramcode[i].dev, Rtl8226b_n2_ramcode[i].addr, Rtl8226b_n2_ramcode[i].value);
            if (status != SUCCESS)
                goto exit;
        }

        // patch uc2
        for(i=0; i<sizeof(Rtl8226b_uc2_ramcode)/sizeof(MMD_REG); i++)
        {
            status = MmdPhyWrite(hDevice, Rtl8226b_uc2_ramcode[i].dev, Rtl8226b_uc2_ramcode[i].addr, Rtl8226b_uc2_ramcode[i].value);
           
			if (status != SUCCESS)
                goto exit;
        }

        // MMD 31.0xB820[7] = 1'b0
        status = MmdPhyRead(hDevice, MMD_VEND2, 0xB820, &phydata);
        if (status != SUCCESS)
            goto exit;

        phydata &= (~BIT_7);

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xB820, phydata);
        if (status != SUCCESS)
            goto exit;

        // patch uc
        for(i=0; i<sizeof(Rtl8226b_uc_ramcode)/sizeof(MMD_REG); i++)
        {
            status = MmdPhyWrite(hDevice, Rtl8226b_uc_ramcode[i].dev, Rtl8226b_uc_ramcode[i].addr, Rtl8226b_uc_ramcode[i].value);
            if (status != SUCCESS)
                goto exit;
        }

        // GPHY OCP 0xB896 bit[0] = 0x0
        status = MmdPhyRead(hDevice, MMD_VEND2, 0xB896, &phydata);
        if (status != SUCCESS)
            goto exit;
        
        phydata &= (~BIT_0);

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xB896, phydata);
        if (status != SUCCESS)
            goto exit;

        // GPHY OCP 0xB892 bit[15:8] = 0x0
        phydata = 0;
        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xB892, phydata);
        if (status != SUCCESS)
            goto exit;

        // patch ram code
        for(i=0; i<sizeof(Rtl8226b_data_ramcode)/sizeof(MMD_REG); i++)
        {
            status = MmdPhyWrite(hDevice, Rtl8226b_data_ramcode[i].dev, Rtl8226b_data_ramcode[i].addr, Rtl8226b_data_ramcode[i].value);
            if (status != SUCCESS)
                goto exit;
        }

        // GPHY OCP 0xB896 bit[0] = 0x1
        status = MmdPhyRead(hDevice, MMD_VEND2, 0xB896, &phydata);
        if (status != SUCCESS)
            goto exit;

        phydata |= BIT_0;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xB896, phydata);
        if (status != SUCCESS)
            goto exit;

        // Clear patch_key & patch_lock
        phydata = 0x0;
        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xA436, phydata);
        if (status != SUCCESS)
            goto exit;
		
        // MMD 31.0xA438[15:0] = 0x0000
        phydata = 0x0;
        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xA438, phydata);
        if (status != SUCCESS)
            goto exit;

        // MMD 31.0xB82E[0] = 1'b0
        status = MmdPhyRead(hDevice, MMD_VEND2, 0xB82E, &phydata);
        if (status != SUCCESS)
            goto exit;

        phydata &= (~BIT_0);
     
        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xB82E, phydata);
        if (status != SUCCESS)
            goto exit;

        // MMD 31.0xA436[15:0] = patch_key_addr
        phydata = patchaddr;
        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xA436, phydata);
        if (status != SUCCESS)
            goto exit;

        // MMD 31.0xA438[15:0] = 0x0000
        phydata = 0x0;
        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xA438, phydata);
        if (status != SUCCESS)
            goto exit;

        // Release patch request
        status = MmdPhyRead(hDevice, MMD_VEND2, 0xB820, &phydata);
        if (status != SUCCESS)
            goto exit;

        phydata &= (~BIT_4);

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xB820, phydata);
        if (status != SUCCESS)
            goto exit;
		
        status = Rtl8226b_wait_for_bit(hDevice, MMD_VEND2, 0xB800, BIT_6, 0, 100);
        if (status != SUCCESS)
            goto exit;

        for(i=0; i<sizeof(Rtl8226b_normal_patch)/sizeof(MMD_REG); i++)
        {
            status = MmdPhyWrite(hDevice, Rtl8226b_normal_patch[i].dev, Rtl8226b_normal_patch[i].addr, Rtl8226b_normal_patch[i].value);
            if (status != SUCCESS)
                goto exit;
        }
		
        printk("[%s-%d]: Set RTL8226B to SERDES %d mode \n",__FUNCTION__,__LINE__, SerdesMode);
        status = MmdPhyWrite(hDevice, MMD_VEND1, 0x75f3, 0x0);
        if (status != SUCCESS)
            goto exit;
		
		    
        status = MmdPhyWrite(hDevice, MMD_VEND1, 0x697a, SerdesMode);
        if (status != SUCCESS)
            goto exit;

        printk("[%s-%d]: Disable RTL8226B auto-negotiation \n",__FUNCTION__,__LINE__);
        status = MmdPhyWrite(hDevice, MMD_VEND1, 0x7588, 0x2);
        if (status != SUCCESS)
            goto exit;
        
            
        status = MmdPhyWrite(hDevice, MMD_VEND1, 0x7589, 0x71d0);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND1, 0x7587, 0x3);
        if (status != SUCCESS)
            goto exit;
		
 //ISRAM PATCH  
        if (singlephy)
        {
            for(i=0; i<sizeof(Rtl8226b_isram_patch)/sizeof(MMD_REG); i++)
            {
                status = MmdPhyWrite(hDevice, Rtl8226b_isram_patch[i].dev, Rtl8226b_isram_patch[i].addr, Rtl8226b_isram_patch[i].value);
                if (status != SUCCESS)
                    goto exit;
            }
        }


      status = MmdPhyWrite(hDevice, MMD_VEND1, 0x75B6, 0x0024);
        if (status != SUCCESS)
            goto exit;




  // Update patch version
        phydata = 0x801E;
        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xA436, phydata);
        if (status != SUCCESS)
            goto exit;

        // MMD 31.0xA438[15:0] =  driver_note_ver
        phydata = patchver;
        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xA438, phydata);
        if (status != SUCCESS)
            goto exit;
  
        {
            PHY_LINK_ABILITY phylinkability;
            memset(&phylinkability, 0x0, sizeof(phylinkability));

            phylinkability.Half_10 = TRUE;
            phylinkability.Full_10 = TRUE;

            phylinkability.Half_100 = TRUE;
            phylinkability.Full_100 = TRUE;

            phylinkability.Full_1000 = TRUE;

            phylinkability.adv_2_5G = TRUE;

            phylinkability.FC = TRUE;
            phylinkability.AsyFC = TRUE;

            status = Rtl8226_autoNegoAbility_set(hDevice, &phylinkability);
            if (status != SUCCESS)
                goto exit;
        }



        // PHYRST & Restart Nway
        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xA400, 0x9200);
        if (status != SUCCESS)
            goto exit;
    }

    // MMD 31.0xA430[15:0] = 0x019E
	printk("[%s-%d]: RTL8226B ignore broadcast\n",__FUNCTION__,__LINE__);
    status = MmdPhyWrite(hDevice, MMD_VEND2, 0xA430, 0x019E);
    if (status != SUCCESS)
        goto exit;

exit:
    return status;
}
