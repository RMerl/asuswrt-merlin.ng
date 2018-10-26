/*
 * <:label-BRCM:2012:NONE:standard
 *
 * :>
 * */

#include "bcm63xx_util.h"
#include "btrm_if.h"
#include "bcm_map.h"
#include "bcm_otp.h"
#include "lib_byteorder.h"
#if !defined (_BCM963381_)
#include "bcm63xx_sotp.h"
#endif
#include "bcm63xx_potp_sotp_util.h"
#include "bcm_sec.h"

#define BTRM_OTP_READ_TIMEOUT_CNTR              0x10000

/**********************************************************************
 *  static int read_potp(uint32_t row, uint32_t *pRval)
 *  
 *  Input parameters: 
 *      row   - row number of potp to read
 *      pRval - pointer to uint32_t container to be loaded with data
 *      
 *  Return value:
 *      0 - everything worked as expected
 *      1 - not good
 *
 ********************************************************************* */
int read_potp(uint32_t row, uint32_t *pRval)
{
   int rs = -1;
   uint32_t cntr = BTRM_OTP_READ_TIMEOUT_CNTR;
   volatile uint32_t *pCtrl0Reg = (uint32_t *)(JTAG_OTP_BASE + JTAG_OTP_GENERAL_CTRL_0);
   volatile uint32_t *pCtrl1Reg = (uint32_t *)(JTAG_OTP_BASE + JTAG_OTP_GENERAL_CTRL_1);
   volatile uint32_t *pCtrl2Reg = (uint32_t *)(JTAG_OTP_BASE + JTAG_OTP_GENERAL_CTRL_2);
   volatile uint32_t *pCtrl3Reg = (uint32_t *)(JTAG_OTP_BASE + JTAG_OTP_GENERAL_CTRL_3);
   volatile uint32_t *pStat0Reg = (uint32_t *)(JTAG_OTP_BASE + JTAG_OTP_GENERAL_STATUS_0);
   volatile uint32_t *pStat1Reg = (uint32_t *)(JTAG_OTP_BASE + JTAG_OTP_GENERAL_STATUS_1);

   /* Clear OTP controller regs */
   *pCtrl0Reg = 0;
   *pCtrl1Reg &= ~0x00000001;
   *pCtrl2Reg = 0; 
   *pCtrl3Reg = 0; 
   cfe_usleep(5);

   /* turn on the cpu mode */
   *pCtrl1Reg |= 0x00000001;

   /* Assume OTP read is going to timeout */
   rs = 1;

   /* set up the addr for the row of interest */
   *pCtrl3Reg = row;

   /* activate read word */
   *pCtrl0Reg = JTAG_OTP_GENERAL_CTRL_0_START | JTAG_OTP_GENERAL_CTRL_0_PROG_EN | JTAG_OTP_GENERAL_CTRL_0_ACCESS_MODE;

   /* Wait for the CMD_DONE bit to drop low which means the current operation has begun */
   while((*pStat1Reg)&JTAG_OTP_GENERAL_STATUS_1_CMD_DONE)
   {
      cntr--;
      if (cntr == 0)
         break;
   }

   /* If cntr is 0, then situation 3) is occurring */
   /* Reset the count down */
   cntr = BTRM_OTP_READ_TIMEOUT_CNTR;

   /* wait for the otp word retrieval to complete */
   while(!((*pStat1Reg) & JTAG_OTP_GENERAL_STATUS_1_CMD_DONE)) {
      cntr--;
      if (cntr == 0)
         break;
   }

   if (cntr) {
      /* OTP read was successful */
      /* retrieve the data for the row */
      rs = 0;
      *pRval = *pStat0Reg;
#if defined (CFG_RAMAPP)
      printf("%s Got 0x%x\n",__func__,*pRval);
#endif
   }

   /* zero out the ctrl_0 reg */
   *pCtrl0Reg = 0;

   /* turn off the cpu mode */
   *pCtrl1Reg &= ~0x00000001;

   return rs;
}




#if defined (CFG_RAMAPP)
/**********************************************************************
 *  static int read_sotp(uint32_t section)
 *  
 *  Input parameters: 
 *      section - section of sotp to be read
 *      
 *  Return value:
 *      0 - everything worked as expected
 *      1 - not good
 *
 ********************************************************************* */

#if !defined (_BCM963381_)
int read_sotp(uint32_t section)
{
   uint32_t keyLenInWords = 8;
   uint32_t sotpKey[8], i;
   SotpKeyStatus res;
   int rs = 0;

   res = sotpReadSecKey( section, sotpKey, keyLenInWords);
   switch(res)
   {
      case SOTP_S_KEY_SUCCESS:
      {
         printf("\nSOTP_S_KEY_SUCCESS \n");
         printf("\nFor section %d\n\n", section);
         for (i=0; i < keyLenInWords; i++)
            printf("\nkey word %d: 0x%x \n", i, sotpKey[i]);
         break;
      }
      case SOTP_S_KEY_EMPTY:
      {
         printf("\nSection %d is empty \n", section);
         break;
      }
      case SOTP_E_KEY_UNDERRUN:
      {
         printf("\nSOTP_E_KEY_UNDERRUN \n");
         break;
      }
      case SOTP_E_KEY_CRC_MIS:
      {
         printf("\nSOTP_E_KEY_CRC_MIS \n");
         break;
      }
      case SOTP_E_KEY_ERROR:
      {
         printf("\nSOTP_E_KEY_ERROR \n");
         rs = 1;
         break;
      }
      default:
      {
         printf("\nunknown \n");
         rs = 1;
         break;
      }
   }

   return rs;
}



/**********************************************************************
 *  static int burn_sotp(uint32_t section, uint32_t *sotpKey)
 *  
 *  Input parameters: 
 *      section - section of sotp to be fused
 *      sotpKey - pointer to array of uint32 words to be fused
 *      
 *  Return value:
 *      0 - everything worked as expected
 *      1 - not good
 *
 ********************************************************************* */
int burn_sotp(uint32_t section, uint32_t *sotpKey)
{
   SotpKeyStatus res;
   uint32_t keyLenInWords = 8;
   int rs = 0;

   /* 32 bytes to the section of sotp */
   res = sotpWriteSecKey( section, sotpKey, keyLenInWords);
   switch(res)
   {
      case SOTP_S_KEY_SUCCESS:
      {
         printf("\nSOTP_S_KEY_SUCCESS \n");
         break;
      }
      case SOTP_E_KEY_BADPARAM:
      {
         printf("\nSOTP_E_KEY_BADPARAM \n");
         break;
      }
      case SOTP_E_KEY_OVERRUN:
      {
         printf("\nSOTP_E_KEY_OVERRUN \n");
         break;
      }
      case SOTP_E_KEY_ERROR:
      {
         printf("\nSOTP_E_KEY_ERROR \n");
         rs = 1;
         break;
      }
      default:
      {
         printf("\nunknown \n");
         rs = 1;
         break;
      }
   }
   return rs;
}

#endif /*_BCM963381_*/

#endif

/**********************************************************************
 *  static int burn_potp(uint32_t row, uint32_t val)
 *  
 *  Input parameters: 
 *      row   - row number of potp to fuse
 *      pRval - pointer to uint32_t data containing bits to fuse
 *      
 *  Return value:
 *      0 - everything worked as expected
 *      1 - not good
 *
 ********************************************************************* */
int burn_potp(uint32_t row, uint32_t val)
{
   int rs = 0;
   volatile uint32_t *pCtrl0Reg = (uint32_t *)(JTAG_OTP_BASE + JTAG_OTP_GENERAL_CTRL_0);
   volatile uint32_t *pCtrl1Reg = (uint32_t *)(JTAG_OTP_BASE + JTAG_OTP_GENERAL_CTRL_1);
   volatile uint32_t *pCtrl2Reg = (uint32_t *)(JTAG_OTP_BASE + JTAG_OTP_GENERAL_CTRL_2);
   volatile uint32_t *pCtrl3Reg = (uint32_t *)(JTAG_OTP_BASE + JTAG_OTP_GENERAL_CTRL_3);
   volatile uint32_t *pStat1Reg = (uint32_t *)(JTAG_OTP_BASE + JTAG_OTP_GENERAL_STATUS_1);

   uint32_t reg_rd_data;
   int i;
   uint32_t authVal[4] = {0xf,0x4,0x8,0xd};

   /* Clear OTP controller regs */
   *pCtrl0Reg = 0;
   *pCtrl1Reg &= ~0x00000001;
   *pCtrl2Reg = 0; 
   *pCtrl3Reg = 0; 
   cfe_usleep(5);

   /* turn on the cpu mode */
   *pCtrl1Reg |= 0x00000001;

   for (i=0;i<4;i++)
   {
      *pCtrl2Reg = authVal[i];
      *pCtrl0Reg = 0x00000001 | (0x1 << 2) | (0x1 << 21); // Start, OTP_ProgEnable, prog_en
      cfe_usleep(5);
      *pCtrl0Reg = 0x0;
      cfe_usleep(5);
   }
#ifdef CFG_RAMAPP
   printf("\nF48D auth value is programmed \n");
#endif

   reg_rd_data = *pStat1Reg;
   while((reg_rd_data & 0x2) == 0x0)
   {
      cfe_usleep(5);
      reg_rd_data = *pStat1Reg;
   }

   if ((reg_rd_data & 0x4) == 0x4) {
#ifdef CFG_RAMAPP
      printf(" ---------- OTP is in Program mode\n");
#endif
   } else {
#ifdef CFG_RAMAPP
      printf(" ---------- OTP is NOT in Program mode\n");
#endif

      /* turn off the cpu mode */
      *pCtrl1Reg &= ~0x00000001;

      rs = -1;
      return rs;
   }

   *pCtrl2Reg = val;
   *pCtrl3Reg = row;

   /* Activate Program Word */
   *pCtrl0Reg = (0x1 | (0xa << 1) | (0x2 << 22) | (0x1 << 21)); /* Start, write word, access mode 2, prog_en */
   reg_rd_data = *pStat1Reg;
   while((reg_rd_data & 0x2) == 0x0)
   {
      cfe_usleep(5);
      reg_rd_data = *pStat1Reg;
   }
   *pCtrl0Reg = 0x0;

#ifdef CFG_RAMAPP
   printf(" ---------- command_done is high \n");
#endif
   /* turn off the cpu mode */
   *pCtrl1Reg &= ~0x00000001;

   return rs;
}

#if !defined(CFG_RAMAPP)
int potp_set_brom_mode()
{
	int res = burn_potp(OTP_BRCM_BTRM_BOOT_ENABLE_ROW,
			(0x1<<OTP_BRCM_BTRM_BOOT_ENABLE_SHIFT)&OTP_BRCM_BTRM_BOOT_ENABLE_MASK);
	if (!res ) {
    		burn_potp(
#if (BOARD_SEC_ARCH==SEC_ARCH_GEN2)
				OTP_CUST_BTRM_BOOT_ENABLE_FUSE_ROW, 
#else
				OTP_CUST_BTRM_BOOT_ENABLE_ROW,
#endif
			(0x7<<OTP_CUST_BTRM_BOOT_ENABLE_SHIFT)&OTP_CUST_BTRM_BOOT_ENABLE_MASK);
	}
	return res;
}
#if (BOARD_SEC_ARCH==SEC_ARCH_GEN2)
int potp_set_brom_opmode()
{
        return burn_potp(OTP_CUST_OP_INUSE_FUSE_ROW, (0x1<<OTP_CUST_OP_INUSE_SHIFT)&OTP_CUST_OP_INUSE_MASK);
}

int potp_set_oid(uint16_t oid)
{
        return burn_potp(OTP_CUST_OP_MRKTID_ROW, (oid<<OTP_CUST_OP_MRKTID_SHIFT)&OTP_CUST_OP_MRKTID_MASK);
}

int read_potp_oid(uint16_t *oid)
{
    uint32_t rval = 0;
    int res;
    /* Fuse demonstration MID into potp  (row 24, bits 0 thru 15) (ecc row) */
    /* mid = 0x3412 for gen2 bootrom with arm proc, and 0x1234 for mips and gen3 arm */
    /* gen3 bootrom contains htons() */
    res = read_potp(OTP_CUST_OP_MRKTID_ROW, &rval);
    if (!res) { 
        *oid = (rval&OTP_CUST_OP_MRKTID_MASK)>>OTP_CUST_OP_MRKTID_SHIFT;
    }
    return res;
}
#endif

#endif

/**********************************************************************
 *  static int burn_potp_mid(uint16_t mid)
 *  
 *  Input parameters: 
 *      mid - 16 bit market identifier to be fused
 *      
 *  Return value:
 *      0 - everything worked as expected
 *      1 - not good
 *
 ********************************************************************* */
int burn_potp_mid(uint16_t mid)
{

    /* Fuse demonstration MID into potp  (row 24, bits 0 thru 15) (ecc row) */
    /* mid = 0x3412 for gen2 bootrom with arm proc, and 0x1234 for mips and gen3 arm */
    /* gen3 bootrom contains htons() */
    return burn_potp(OTP_CUST_MFG_MRKTID_ROW, mid);
}

int read_potp_mid(uint16_t *mid)
{
    uint32_t rval = 0;
    int res;
    /* Fuse demonstration MID into potp  (row 24, bits 0 thru 15) (ecc row) */
    /* mid = 0x3412 for gen2 bootrom with arm proc, and 0x1234 for mips and gen3 arm */
    /* gen3 bootrom contains htons() */
    res = read_potp(OTP_CUST_MFG_MRKTID_ROW, &rval);
    if (!res) { 
        *mid = (rval&OTP_CUST_MFG_MRKTID_MASK)>>OTP_CUST_MFG_MRKTID_SHIFT;
    }
    return res;
}
