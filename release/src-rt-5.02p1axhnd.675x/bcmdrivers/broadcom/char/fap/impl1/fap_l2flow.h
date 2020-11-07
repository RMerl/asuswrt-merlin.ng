#ifndef __FAP_L2FLOW_H_INCLUDED__
#define __FAP_L2FLOW_H_INCLUDED__

/*
<:copyright-broadcom

 Copyright (c) 2007 Broadcom Corporation
 All Rights Reserved
 No portions of this material may be reproduced in any form without the
 written permission of:
          Broadcom Corporation
          5300 California Avenue
          Irvine, California 92617
 All information contained in this document is Broadcom Corporation
 company private, proprietary, and trade secret.

:>
*/

#if defined(CONFIG_BCM_FAP_LAYER2)

int fapL2flow_activate(Blog_t *blog_p);
int fapL2flow_deactivate(fapPkt_flowHandle_t flowHandle);

#endif   /* CONFIG_BCM_FAP_LAYER2 */
#endif   /* __FAP_L2FLOW_H_INCLUDED__ */
