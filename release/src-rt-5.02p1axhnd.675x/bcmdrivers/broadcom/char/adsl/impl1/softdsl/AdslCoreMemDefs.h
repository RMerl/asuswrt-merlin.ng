/*
<:copyright-broadcom

 Copyright (c) 2002 Broadcom Corporation
 All Rights Reserved
 No portions of this material may be reproduced in any form without the
 written permission of:
          Broadcom Corporation
          16215 Alton Parkway
          Irvine, California 92619
 All information contained in this document is Broadcom Corporation
 company private, proprietary, and trade secret.

:>
*/
/****************************************************************************/

#ifdef ADSL_ANNEXB
#define ADSL_PHY_XFACE_OFFSET 0x21F90
#define ADSL_PHY_SDRAM_BIAS 0x1A0000
#define ADSL_PHY_SDRAM_LINK_OFFSET 0x1A0000
#define ADSL_PHY_SDRAM_PAGE_SIZE 0x200000
#elif defined(ADSL_ANNEXC)
#define ADSL_PHY_XFACE_OFFSET 0x21F90
#else
#define ADSL_PHY_XFACE_OFFSET 0x21F90
#define ADSL_PHY_SDRAM_BIAS 0x1A0000
#define ADSL_PHY_SDRAM_LINK_OFFSET 0x1A0000
#define ADSL_PHY_SDRAM_PAGE_SIZE 0x200000
#endif



