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
#include "bcm_sec_auth.h"

#define BTRM_OTP_READ_TIMEOUT_CNTR              0x10000

/* CPU soft lock only needed for 63178/47622*/
static inline void otp_lock(void)
{
#ifdef JTAG_OTP_GENERAL_CPU_SOFT_LOCK
   volatile uint32_t *cpuSoftLock = (volatile uint32_t *)(JTAG_OTP_BASE + JTAG_OTP_GENERAL_CPU_SOFT_LOCK);
   *cpuSoftLock = 1;
   while ((*cpuSoftLock&JTAG_OTP_GENERAL_CPU_SOFT_LOCK_MASK)==0);
#endif
}

static inline void otp_unlock(void)
{
#ifdef JTAG_OTP_GENERAL_CPU_SOFT_LOCK
   *(volatile uint32_t *)(JTAG_OTP_BASE + JTAG_OTP_GENERAL_CPU_SOFT_LOCK) = 0;
#endif
}


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
   rs = bcm_otp_get_row(row, pRval);

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
   int rs;
   rs = bcm_otp_fuse_row(row, val);
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
