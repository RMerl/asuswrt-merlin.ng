/*
 * <:label-BRCM:2012:NONE:standard
 *
 * :>
 * */

#include "bcm63xx_util.h"
#include "btrm_if.h"
#include "bcm_common.h"
#include "bcm_otp.h"
#include "bcm63xx_sotp.h"

#define BTRM_OTP_READ_TIMEOUT_CNTR              0x10000

extern int yesno(void);



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
static int read_potp(uint32_t row, uint32_t *pRval)
{
   int                  rs;
   uint32_t             cntr = BTRM_OTP_READ_TIMEOUT_CNTR;
   uint32_t             *pCtrl0Reg = (uint32_t *)(JTAG_OTP_BASE + JTAG_OTP_GENERAL_CTRL_0);
   uint32_t             *pCtrl1Reg = (uint32_t *)(JTAG_OTP_BASE + JTAG_OTP_GENERAL_CTRL_1);
   uint32_t             *pCtrl3Reg = (uint32_t *)(JTAG_OTP_BASE + JTAG_OTP_GENERAL_CTRL_3);
   uint32_t             *pStat0Reg = (uint32_t *)(JTAG_OTP_BASE + JTAG_OTP_GENERAL_STATUS_0);
   uint32_t             *pStat1Reg = (uint32_t *)(JTAG_OTP_BASE + JTAG_OTP_GENERAL_STATUS_1);

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
   while(!((*pStat1Reg) & JTAG_OTP_GENERAL_STATUS_1_CMD_DONE))
   {
      cntr--;
      if (cntr == 0)
         break;
   }

   if (cntr)
   {
      /* OTP read was successful */
      /* retrieve the data for the row */
      rs = 0;
      *pRval = *pStat0Reg;
   }

   /* zero out the ctrl_0 reg */
   *pCtrl0Reg = 0;

   /* turn off the cpu mode */
   *pCtrl1Reg &= ~0x00000001;

   return rs;
}



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
static int burn_potp(uint32_t row, uint32_t val)
{
   int rs = 0;
   uint32_t *pCtrl0Reg = (uint32_t *)(JTAG_OTP_BASE + JTAG_OTP_GENERAL_CTRL_0);
   uint32_t *pCtrl1Reg = (uint32_t *)(JTAG_OTP_BASE + JTAG_OTP_GENERAL_CTRL_1);
   uint32_t *pCtrl2Reg = (uint32_t *)(JTAG_OTP_BASE + JTAG_OTP_GENERAL_CTRL_2);
   uint32_t *pCtrl3Reg = (uint32_t *)(JTAG_OTP_BASE + JTAG_OTP_GENERAL_CTRL_3);
   uint32_t *pStat1Reg = (uint32_t *)(JTAG_OTP_BASE + JTAG_OTP_GENERAL_STATUS_1);

   uint32_t reg_rd_data;
   int i;
   uint32_t authVal[4] = {0xf,0x4,0x8,0xd};

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
   printf("\nF48D auth value is programmed \n");


   reg_rd_data = *pStat1Reg;
   while((reg_rd_data & 0x2) == 0x0)
   {
      cfe_usleep(5);
      reg_rd_data = *pStat1Reg;
   }

   if ((reg_rd_data & 0x4) == 0x4)
      printf(" ---------- OTP is in Program mode\n");
   else
   {
      printf(" ---------- OTP is NOT in Program mode\n");

      /* turn off the cpu mode */
      *pCtrl1Reg &= ~0x00000001;

      rs = 1;
      return rs;
   }

   *pCtrl2Reg = val;
   *pCtrl3Reg = row;

   /* Activate Program Word */
   *pCtrl0Reg = (0x1 | (0xa << 1) | (0x2 << 22) | (0x1 << 21)); // Start, write word, access mode 2, prog_en
   reg_rd_data = *pStat1Reg;
   while((reg_rd_data & 0x2) == 0x0)
   {
      cfe_usleep(5);
      reg_rd_data = *pStat1Reg;
   }
   *pCtrl0Reg = 0x0;
   printf(" ---------- command_done is high \n");

   /* turn off the cpu mode */
   *pCtrl1Reg &= ~0x00000001;

   return rs;
}



#if 0
/**********************************************************************
 *  static int read_potp_all(ui_cmdline_t *cmd,int argc,char *argv[])
 *  
 *  Input parameters: 
 *      none
 *      
 *  Return value:
 *      0 - everything worked as expected
 *      1 - not good
 *
 ********************************************************************* */
static int read_potp_all(ui_cmdline_t *cmd,int argc,char *argv[])
{
   uint32_t otpRowData;
   int res = 0;

   res = read_potp(17, &otpRowData);
   if (res == 0)
      printf("\nRow 17 data is 0x%x ", otpRowData);

   res = read_potp(18, &otpRowData);
   if (res == 0)
      printf("\nRow 18 data is 0x%x ", otpRowData);

   res = read_potp(24, &otpRowData);
   if (res == 0)
      printf("\nRow 24 data is 0x%x ", otpRowData);

   otpRowData = otp_is_boot_secure();
   printf("\notp_is_boot_secure() returned 0x%x \n", otpRowData);

#if !defined(_BCM94908_) && !defined(_BCM96858_)
   otpRowData = otp_is_btrm_enabled();
   printf("\notp_is_btrm_enabled() returned 0x%x ", otpRowData);
#else
   otpRowData = bcm_otp_is_btrm_boot();
   printf("\nbcm_otp_is_btrm_boot() returned 0x%x ", otpRowData);

   otpRowData = bcm_otp_is_boot_secure();
   printf("\nbcm_otp_is_boot_secure() returned 0x%x ", otpRowData);

   otpRowData = bcm_otp_is_boot_mfg_secure();
   printf("\nbcm_otp_is_boot_mfg_secure() returned 0x%x \n", otpRowData);
#endif

   return res;
}
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
static int burn_potp_mid(uint16_t mid)
{
    int res;

    /* Fuse demonstration MID into potp  (row 24, bits 0 thru 15) (ecc row) */
    /* mid = 0x3412 for gen2 bootrom with arm proc, and 0x1234 for mips and gen3 arm */
    /* gen3 bootrom contains htons() */
#if defined(_BCM963138_) || defined(_BCM963148_)
    mid = htons(mid);
#endif
    res = burn_potp(24, mid);
    return res;
}



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
static int read_sotp(uint32_t section)
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
static int burn_sotp(uint32_t section, uint32_t *sotpKey)
{
   SotpKeyStatus res;
   uint32_t keyLenInWords = 8;
   int rs = 0;

#if 0
   /* section 8 demo credentials are as follows */
   uint32_t sotpKey[] = { 0x1a2b3c4d, 0x5e6f1a2b, \
                          0x3c4d5e6f, 0x1a2b3c4d, \
                          0xa0b0c0d0, 0xe0f0a1b1, \
                          0xc1d1e1f1, 0xa2b2c2d2 };

   /* section 9 demo credentials are as follows */
   uint32_t sotpKey[] = { 0x247b8f52, 0x3cb62da2, \
                          0x76879ba4, 0x16569049, \
                          0x16163f27, 0x90513f68, \
                          0xeb337653, 0x3bdced4e };
#endif

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



/**********************************************************************
 *  static int otpcfg(ui_cmdline_t *cmd,int argc,char *argv[])
 *  
 *  Input parameters: 
 *      code says it all
 *      
 *  Return value:
 *      0 - everything worked as expected
 *      1 - not good
 *
 ********************************************************************* */
static int otpcfg(ui_cmdline_t *cmd,int argc,char *argv[])
{
   /* demo mid */
   uint16_t mid = 0x1234;

   /* demo kroe-fld */
   uint32_t sotpSec8Key[] = { 0x1a2b3c4d, 0x5e6f1a2b, \
                              0x3c4d5e6f, 0x1a2b3c4d, \
                              0xa0b0c0d0, 0xe0f0a1b1, \
                              0xc1d1e1f1, 0xa2b2c2d2 };

   /* demo hmid+rot-fld-pub */
   uint32_t sotpSec9Key[] = { 0x247b8f52, 0x3cb62da2, \
                              0x76879ba4, 0x16569049, \
                              0x16163f27, 0x90513f68, \
                              0xeb337653, 0x3bdced4e };

   int res = 0;
   uint32_t otpRowData;

   char *pFirstArg  = NULL;
   char *pSecondArg = NULL;
   char *pThirdArg  = NULL;

   uint32_t isMfgSecure = bcm_otp_is_boot_mfg_secure();
   uint32_t isFldSecure = bcm_otp_is_boot_secure();

   if (argc > 0) 
      pFirstArg  = cmd_getarg(cmd, 0);

   if (argc > 1)
      pSecondArg = cmd_getarg(cmd, 1);

   if (argc > 2)
      pThirdArg  = cmd_getarg(cmd, 2);

   if (isFldSecure)
   {
      /* field-secure mode */
      if ((! strcmp(pFirstArg, "get")) && (! strcmp(pSecondArg, "mode")))
      {
         printf("\nSoC is in field-secure mode\n");
      }
      else
      {
         printf("\n");
         printf("Usage: otpcfg get mode\n");
         printf("       otpcfg help\n\n");
      }
   }
   else if (isMfgSecure)
   {

      /* mfg-secure mode */
      if (! strcmp(pFirstArg, "get"))
      {
         if (! strcmp(pSecondArg, "mode"))
         {
            printf("\nSoC is in manufacturing-secure mode\n");
            return res;
         }
         if (! strcmp(pSecondArg, "mid"))
         {
            /* mid is bottom 16 bits of row 24 */
            res = read_potp(24, &otpRowData);
            if (res == 0)
               printf("\nmid is 0x%x \n", otpRowData & 0xffff);
            else
               printf("\nError: cannot read otp row 24\n");

            return res;
         }
         if (! strcmp(pSecondArg, "kroe-fld"))
         {
            /* kroe-fld is in section 8 of sotp */
            res = read_sotp(8);
            return res;
         }
         if (! strcmp(pSecondArg, "hmid+rot-fld-pub"))
         {
            /* hmid+rot-fld-pub is in section 9 of sotp */
            res = read_sotp(9);
            return res;
         }
      }

      if (! strcmp(pFirstArg, "set")) 
      {
         if (! strcmp(pSecondArg, "mid"))
         {
            res = burn_potp_mid(mid);
            return res;
         }
         if (! strcmp(pSecondArg, "kroe-fld"))
         {
            /* kroe-fld goes into section 8 of sotp */
            res = burn_sotp( 8, sotpSec8Key);
            return res;
         }
         if (! strcmp(pSecondArg, "hmid+rot-fld-pub"))
         {
            /* hmid+rot-fld-pub goes into section 9 of sotp */
            res = burn_sotp( 9, sotpSec9Key);
            return res;
         }
      }
      
      printf("\n");
      printf("Usage: otpcfg set mid\n");
      printf("       otpcfg set kroe-fld\n");
      printf("       otpcfg set hmid+rot-fld-pub\n");
      printf("       otpcfg get mid\n");
      printf("       otpcfg get kroe-fld\n");
      printf("       otpcfg get hmid+rot-fld-pub\n");
      printf("       otpcfg get mode\n");
      printf("       otpcfg help\n\n");

   }
   else
   {
      /* Unsecure mode */
      if ((! strcmp(pFirstArg, "get")) && (! strcmp(pSecondArg, "mode")))
      {
         printf("\nSoC is in unsecure mode\n");
      }
      else if ((! strcmp(pFirstArg, "set")) && (! strcmp(pSecondArg, "mode")) && (! strcmp(pThirdArg, "secure")))
      {
         printf("\nConfigure the SoC to be in mfg-secure mode?");
         if (yesno())
            return res;

         printf("\nNow think carefully. There is no going back! This SoC will permanently be configured for secure boot.\n");
         printf("Do you really want to configure the SoC to be in mfg-secure mode?");
         if (yesno())
            return res;

         /* Read brcm btrm-enable potp bit to see if it needs to be set */
         res = read_potp(17, &otpRowData);
         if ((res == 0) && (! (otpRowData & 0x8)))
            res = burn_potp(17, 0x8);

         /* Enable customer btrm-enable potp bits (row 18, bits 15,16,17) (not an ecc row) */
         res = burn_potp(18, 0x38000);
	 if (res == 0)
            printf("\nAfter the next reboot, the SoC will be in mfg-secure mode\n");
      }
      else
      {
         printf("\n");
         printf("Usage: otpcfg set mode secure\n");
         printf("       otpcfg get mode\n");
         printf("       otpcfg help\n\n");
      }
   }

   return res;
}



/**********************************************************************
 *  int ui_init_otp_cmds(void) 
 *  
 *  Input parameters: 
 *      none
 *      
 *  Return value:
 *      0 - everything worked as expected
 *
 ********************************************************************* */
int ui_init_otp_cmds(void) {

#if 0
    cmd_addcmd("potpr",
           read_potp_all,
           NULL,
           "Read potp rows 17,18, and 24 ",
           "potpr",
           "");
#endif

    cmd_addcmd("otpcfg",
           otpcfg,
           NULL,
           "Reading/fusing potp/sotp bits ",
           "otpcfg",
           "");

    return( 0 );
}
