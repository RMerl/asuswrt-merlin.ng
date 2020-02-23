/*  *********************************************************************
    *
    <:copyright-BRCM:2017:proprietary:standard
    
       Copyright (c) 2017 Broadcom 
       All Rights Reserved
    
     This program is the proprietary software of Broadcom and/or its
     licensors, and may only be used, duplicated, modified or distributed pursuant
     to the terms and conditions of a separate, written license agreement executed
     between you and Broadcom (an "Authorized License").  Except as set forth in
     an Authorized License, Broadcom grants no license (express or implied), right
     to use, or waiver of any kind with respect to the Software, and Broadcom
     expressly reserves all rights in and to the Software and all intellectual
     property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
     NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
     BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
    
     Except as expressly set forth in the Authorized License,
    
     1. This program, including its structure, sequence and organization,
        constitutes the valuable trade secrets of Broadcom, and you shall use
        all reasonable efforts to protect the confidentiality thereof, and to
        use this information only in connection with your use of Broadcom
        integrated circuit products.
    
     2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
        AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
        WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
        RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
        ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
        FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
        COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
        TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
        PERFORMANCE OF THE SOFTWARE.
    
     3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
        ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
        INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
        WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
        IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
        OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
        SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
        SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
        LIMITED REMEDY.
    :> 
    ********************************************************************* */
#include "rom_main.h"
#include "bcm_btrm_common.h"
#include "lib_crc.h"

extern int spi_nand_get_numsectors(void);
extern int spi_nand_read_buf(unsigned short blk, int offset, unsigned char *buffer, int len);
extern int spi_nand_get_sector_size(unsigned short sector);


typedef void (*func )(int offset, unsigned char *pSrc, uint32_t len);

void do_nothing(int offset, unsigned char *pSrc, uint32_t len)
{
}

func ppCopyFunc=do_nothing;


/*  *********************************************************************
    *  copy_from_mapped_flash()
    *      Perform a retrieval of flash content from a spi nor or nand device
    *      using direct access
    *
    *  Input parameters:
    *
    *  Return value:
    *
    ********************************************************************* */
static void copy_from_mapped_flash(int offset, unsigned char *pDst, uint32_t len)
{
   memcpy(pDst,(unsigned char *)BTRM_EXT_MEM_ADDR_SBI_REGION_BGN_NAND+offset, len);
}

static void copy_from_spi_flash(int offset, unsigned char *pDst, uint32_t len)
{
   spi_nand_read_buf(0, offset, (unsigned char *)pDst,  len);
}


static void select_flash_type(void)
{
    // See if we are using eMMC flash, SPI_NAND flash, SPI_NOR flash or NAND flash, or boot from ethernet
    uint32_t miscStrapBus = *(uint32_t *)(MISC_BASE + MISC_STRAP_BUS);
    uint32_t miscStrapBootSel = (miscStrapBus & MISC_STRAP_BUS_BOOTSEL_MASK) >> MISC_STRAP_BUS_BOOTSEL_SHIFT;

    // The STRAP_BOOT_SEL[5] bit in the misc strap bus register solely controls whether NAND flash is in use.
    if ( ! (miscStrapBootSel & MISC_STRAP_BUS_BOOT_NAND_MASK))
    {
        // Print out "NAND" because it is NAND flash
        board_setleds(*(unsigned *)"DNAN");
        NAND->NandCsNandXor = 1;
        ppCopyFunc = copy_from_mapped_flash;
    }
    else if (miscStrapBootSel ==  MISC_STRAP_BUS_BOOT_SPINAND)
    {
        // Print out "SNAN" because it is Spi NANd flash
        board_setleds(*(unsigned *)"NANS");
        // Initialize the SPI NAND flash driver
        rom_spi_nand_init();
        ppCopyFunc = copy_from_spi_flash;
    }
}


static int chksum_sbi(unsigned char *pSbi)
{
    // sbiLen is the length of the entire UBI image including the 4 byte CRC
    SbiUnauthHdrBeginning *pUHdr = (SbiUnauthHdrBeginning *)pSbi;
    uint32_t              sbiLen = pUHdr->sbiSize;
    uint32_t       unauthHdrSize = pUHdr->hdrLen;
    unsigned char     *pCrcStart = pSbi + unauthHdrSize;
    uint32_t              crcLen = sbiLen - (SEC_S_SIGNATURE * 2) - unauthHdrSize - CRC_LEN;
    uint32_t                 crc = CRC32_INIT_VALUE;



    // We are about to authenticate the UBI itself, print out "UBI?"
    board_setleds(*(unsigned *)"?IBU");
    // Perform the CRC calc
    // crc is only across the same data that the signature is across
    // therefore exclude the unauthenticated header, and the trailer
    crc = getCrc32(pCrcStart, crcLen, crc);

    if (memcmp(&crc, pSbi + sbiLen - CRC_LEN, CRC_LEN) != 0)
    {
        // we have failed the UBI image crc, print out "UBIF"
        board_setleds(*(unsigned *)"FIBU");
        return -1;
    }

    // we have passed the UBI image crc, print out "UBIP"
    board_setleds(*(unsigned *)"PIBU");
    return 0;
}

int sbi_hunt(void)
{
    struct hdr_chksum {
    SbiUnauthHdrBeginning  unauthHdr;
    uint32_t chksum;
    }__attribute__((__packed__)) hdchk;
    SbiUnauthHdrBeginning *pHdr;
    SbiAuthHdrBeginning   authHdrBgn;
    unsigned int hunt_start=0x10000, auth_hdr_offset=0;
    uint32_t crc = CRC32_INIT_VALUE;
    int rc=-1;


    select_flash_type();

    board_setleds(*(unsigned *)"?gMI");

    while(hunt_start < 0x100000)
    {
        ppCopyFunc(hunt_start, (unsigned char *)&hdchk,  sizeof(hdchk));
        if (hdchk.unauthHdr.magic_1 == BTRM_SBI_UNAUTH_MGC_NUM_1)
        {
            if (hdchk.unauthHdr.magic_2 == BTRM_SBI_UNAUTH_MGC_NUM_2)
            {
                board_setleds(*(unsigned *)"LGMI");
                //verify image
                // Print out "UHD?" which stands for "verifying Unauthenticated HeaDer"
                board_setleds(*(unsigned *)"?DHU");
                // Do a preliminary upper / lower boundary check of the size of the unauthenticated header
                if ((hdchk.unauthHdr.hdrLen >= sizeof(SbiUnauthHdrBeginning)) && (hdchk.unauthHdr.hdrLen < BTRM_SBI_UNAUTH_HDR_MAX_SIZE))
                {
                    // Perform a CRC calc on the header
                    pHdr=&hdchk.unauthHdr;
                    crc = CRC32_INIT_VALUE;
                    crc = getCrc32((uint8_t*)pHdr, hdchk.unauthHdr.hdrLen-sizeof(uint32_t), crc);
                    if (memcmp(&crc, ((uint8_t*)pHdr) + (hdchk.unauthHdr.hdrLen-sizeof(uint32_t)), CRC_LEN) == 0)
                    {
                        //
                        if (hdchk.unauthHdr.sbiSize < BTRM_SBI_IMAGE_MAX_ALLOWED_SIZE)
                        {
                            // Check that the sbi size value is at least big enough integer to cover the
                            // unauthenticated header and trailer.
                            if (hdchk.unauthHdr.sbiSize <= ((SEC_S_SIGNATURE * 2) + hdchk.unauthHdr.hdrLen + CRC_LEN))
                            {
                                // Print out "UHDF" which stands for "Unath HeaDer Failed"
                                board_setleds(*(unsigned *)"UDHU");    
                            }
                            else
                            {
                                //    
                                board_setleds(*(unsigned *)"PDHU");
                                //get the image 
                                auth_hdr_offset=hunt_start+hdchk.unauthHdr.hdrLen;
                                ppCopyFunc(auth_hdr_offset, (unsigned char *)&authHdrBgn,  sizeof(SbiAuthHdrBeginning));
                                // Retrieve the (untrusted) authHdrSize from the header itself
                                // Do a preliminary upper / lower boundary check of the authenticated header size
                                if ((authHdrBgn.hdrLen < sizeof(SbiAuthHdrBeginning)) || (authHdrBgn.hdrLen > BTRM_SBI_AUTH_HDR_MAX_SIZE))
                                {
                                    board_setleds(*(unsigned *)"fLOR");
                                }
                                else
                                {
                                    // Make sure that the COTs within the authenticated header are such that everything in
                                    // this header is word aligned. For example, mfg ROE COT version 2 should have 2 bytes of padding
                                    if (authHdrBgn.hdrLen % sizeof(uint32_t) != 0)
                                    {
                                        board_setleds(*(unsigned *)"FLOR");
                                    }
                                    else
                                    {
                                                                            ppCopyFunc(hunt_start,(unsigned char *)(BTRM_INT_MEM_SBI_LINK_ADDR_VIRT+0x1000), hdchk.unauthHdr.sbiSize);
                                        if(chksum_sbi((unsigned char *)(BTRM_INT_MEM_SBI_LINK_ADDR_VIRT+0x1000)) == 0)
                                        {
                                            memcpy((unsigned char *)BTRM_INT_MEM_SBI_LINK_ADDR_VIRT, 
                                                ((unsigned char*)BTRM_INT_MEM_SBI_LINK_ADDR_VIRT)+(0x1000+hdchk.unauthHdr.hdrLen+authHdrBgn.hdrLen),
                                                hdchk.unauthHdr.sbiSize);
                                            rc=0;

                                            break;
                                        }
                                    }    
                                }
                            }

                        }
                        else
                        {
                            board_setleds(*(unsigned *)"FTHU");
                        }
                    }
                    else
                    {
                        // Print out "UHDF" which stands for "Unauth HeaDer Failed", return failed
                        board_setleds(*(unsigned *)"FdHU");
                    }

                }
                else
                {
                    board_setleds(*(unsigned *)"FDHU");
                }

                
            }
        }
        hunt_start += 1024;
    }

    return rc;

}

/* read NAND precfe-rom from NAND flash */
void bootPreCfeImage(void)
{
#if defined (_BCM94908_)
    if(sbi_hunt() == 0)
    {
        cfe_launch(BTRM_INT_MEM_SBI_LINK_ADDR_PHYS);
        //should never return;
    }
    else
        board_setleds(*(unsigned*)"LIAF"); //preCFE FAIL
#else
    unsigned int* buf = (unsigned int*)0x82620000; 
    unsigned int* src = (unsigned int*)0xffe1143c;
    unsigned int read_len = 0;
    int i;

    for (i=33; i<=48; i++)
        bcm_set_pinmux(33, 1);

    rom_nand_flash_init();
    NAND->NandCsNandXor = 1;

    while (read_len < 131072/4)
    {
        buf[read_len] = src[read_len];
        read_len++;
    }
    cfe_launch(0x82620000);
#endif
    while(1);
}
