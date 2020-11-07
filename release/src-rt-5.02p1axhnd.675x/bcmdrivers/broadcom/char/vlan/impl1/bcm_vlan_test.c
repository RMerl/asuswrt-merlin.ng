/*
<:copyright-BRCM:2011:proprietary:standard

   Copyright (c) 2011 Broadcom 
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
*/
//**************************************************************************
// File Name  : bcm_vlan_test.c
//
// Description: Broadcom VLAN Interface Driver test
//               
//**************************************************************************

#include "bcm_vlan_local.h"
#include "bcm_vlan_dev.h"
#include <linux/blog_rule.h>

#define vlanTestPrint(_fmt, _arg...)            \
    printk("VLAN TEST : " _fmt, ##_arg)

#define vlanTestError(_fmt, _arg...)                            \
    BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "TEST : " _fmt, ##_arg)


#if defined(CC_BCM_VLAN_FLOW) && defined(CONFIG_BLOG)
int bcmVlan_blogVlanFlows(Blog_t *blog_p,
                          struct net_device *rxVlanDev,
                          struct net_device *txVlanDev);

static void vlanMcastTest1(struct net_device *rxVlanDev, struct net_device *txVlanDev)
{
    int ret;
    Blog_t *blog_p;
    blogRule_t *blogRule_p;
    blogRuleAction_t blogRuleAction;
    int blogRuleCount;

    blog_p = (Blog_t *)kmalloc(sizeof(Blog_t), GFP_KERNEL);
    if(blog_p == NULL)
    {
        vlanTestError("Failed to allocate Blog!");
        return;
    }

    memset(blog_p, 0, sizeof(Blog_t));

    /* create master blog rule */
    blogRule_p = blog_rule_alloc();
    if(blogRule_p == NULL)
    {
        vlanTestError("Failed to allocate Blog Rule!");
        return;
    }
    blog_rule_init(blogRule_p);

    memset(&blogRuleAction, 0, sizeof(blogRuleAction_t));
    blogRuleAction.cmd = BLOG_RULE_CMD_POP_PPPOE_HDR;

    ret = blog_rule_add_action(blogRule_p, &blogRuleAction);
    vlanTestPrint("blog_rule_add_action = <%d>\n", ret);

    /* attach master blog rule to the blog */
    blog_p->blogRule_p = blogRule_p;

    /* Rx Fwd Info */
    blog_p->rx.info.phyHdr = BLOG_GPONPHY;
    blog_p->rx.info.channel = 10;
    /* Tx Fwd Info */
    blog_p->tx.info.phyHdr = BLOG_ENETPHY;
//    blog_p->tx.info.channel = 1;
    /* Tuple */
//    blog_p->rx.tuple.daddr = 0xE80A0A01; /* SSM : 232.10.10.1 */
//    blog_p->rx.tuple.saddr = 0x0A0A0A01; /* 10.10.10.1 */

    ret = bcmVlan_blogVlanFlows(blog_p, rxVlanDev, txVlanDev);
    vlanTestPrint("bcmVlan_blogVlanFlows = <%d>\n", ret);


    /* free blog rules */

    blogRuleCount = blog_rule_free_list(blog_p);

    vlanTestPrint("Found <%u> Blog Rules\n", blogRuleCount);

    /* free blog */

    kfree(blog_p);
}

static void vlanMcastTest2(struct net_device *rxVlanDev, struct net_device *txVlanDev, int sync)
{
    int ret;
    Blog_t *blog_p;
    blogRule_t *blogRule_p;
    blogRuleAction_t blogRuleAction;
    int blogRuleCount;

    blog_p = (Blog_t *)kmalloc(sizeof(Blog_t), GFP_KERNEL);
    if(blog_p == NULL)
    {
        vlanTestError("Failed to allocate Blog!");
        return;
    }

    memset(blog_p, 0, sizeof(Blog_t));

    /* create master blog rule */
    blogRule_p = blog_rule_alloc();
    if(blogRule_p == NULL)
    {
        vlanTestError("Failed to allocate Blog Rule!");
        return;
    }
    blog_rule_init(blogRule_p);

    memset(&blogRuleAction, 0, sizeof(blogRuleAction_t));
    blogRuleAction.cmd = BLOG_RULE_CMD_POP_PPPOE_HDR;

    ret = blog_rule_add_action(blogRule_p, &blogRuleAction);
    vlanTestPrint("blog_rule_add_action = <%d>\n", ret);

    /* attach master blog rule to the blog */
    blog_p->blogRule_p = blogRule_p;

    /* Rx Fwd Info */
    blog_p->rx.info.phyHdr = BLOG_GPONPHY;
    blog_p->rx.info.channel = 10;
    /* Tx Fwd Info */
    blog_p->tx.info.phyHdr = BLOG_ENETPHY;

    ret = bcmVlan_blogVlanFlows(blog_p, rxVlanDev, txVlanDev);
    vlanTestPrint("bcmVlan_blogVlanFlows = <%d>\n", ret);


    /* free blog rules */

    blogRuleCount = blog_rule_free_list(blog_p);

    vlanTestPrint("Found <%u> Blog Rules\n", blogRuleCount);

    /* free blog */

    kfree(blog_p);
}

void vlanMcastTest3(struct net_device *rxVlanDev, struct net_device *txVlanDev)
{
    vlanMcastTest2(rxVlanDev, txVlanDev, 1);
}

#endif /* CC_BCM_VLAN_FLOW */

void bcmVlan_runTest(bcmVlan_iocRunTest_t *iocRunTest_p)
{
    struct net_device *rxVlanDev;
    struct net_device *txVlanDev;

    vlanTestPrint("Test <%u>\n", iocRunTest_p->testNbr);

    rxVlanDev = dev_get_by_name(&init_net, iocRunTest_p->rxVlanDevName);
    if(rxVlanDev != NULL)
    {
        vlanTestPrint("rxVlanDev: %s\n", rxVlanDev->name);
    }
    else
    { 
        vlanTestPrint("rxVlanDev: %s\n", iocRunTest_p->rxVlanDevName);
    }

    txVlanDev = dev_get_by_name(&init_net, iocRunTest_p->txVlanDevName);
    if(txVlanDev != NULL)
    {
        vlanTestPrint("txVlanDev: %s\n", txVlanDev->name);
    }
    else
    {
        vlanTestPrint("txVlanDev: %s\n", iocRunTest_p->txVlanDevName);
    }

#if defined(CC_BCM_VLAN_FLOW)
    switch(iocRunTest_p->testNbr)
    {
        case 1:
            vlanMcastTest1(rxVlanDev, txVlanDev);
            break;

        case 2:
            vlanMcastTest2(rxVlanDev, txVlanDev, 0);
            break;

        case 3:
            vlanMcastTest3(rxVlanDev, txVlanDev);
            break;

        default:
            vlanTestPrint("Invalid testNbr <%d>\n", iocRunTest_p->testNbr);
    }
#endif

    if(rxVlanDev != NULL)
    {
        dev_put(rxVlanDev);
    }

    if(txVlanDev != NULL)
    {
        dev_put(txVlanDev);
    }

    vlanTestPrint("Done!\n");
}
