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
#include "bcm63xx_sotp.h"
#include "bcm63xx_potp_sotp_util.h"


extern int yesno(void);



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
            res = read_potp(OTP_CUST_MFG_MRKTID_ROW, &otpRowData);
            if (res == 0)
               printf("\nmid is 0x%x \n", otpRowData & 0xffff);
            else
               printf("\nError: cannot read otp row %d\n",OTP_CUST_MFG_MRKTID_ROW);

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
         res = read_potp(OTP_BRCM_BTRM_BOOT_ENABLE_ROW, &otpRowData);
         if ((res == 0) && (! (otpRowData & 0x8)))
         res = burn_potp(OTP_BRCM_BTRM_BOOT_ENABLE_ROW, 0x8);

         /* Enable customer btrm-enable potp bits (row 18, bits 15,16,17) (not an ecc row) */
         res = burn_potp(OTP_CUST_BTRM_BOOT_ENABLE_ROW, 0x38000);
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

    brcm_cmd_addcmd("otpcfg",
           otpcfg,
           NULL,
           "Reading/fusing potp/sotp bits ",
           "otpcfg",
           "",
           BRCM_UI_CMD_TYPE_RESTRICTED);

    return( 0 );
}
