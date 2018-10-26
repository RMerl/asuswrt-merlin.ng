/*
<:copyright-BRCM:2012:DUAL/GPL:standard

   Copyright (c) 2012 Broadcom 
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


/** Includes. **/
#include "lib_types.h"
#include "lib_malloc.h"
#include "lib_string.h"
#include "lib_printf.h"

#include "cfe_iocb.h"
#include "cfe_ioctl.h"
#include "cfe_device.h"
#include "cfe_devfuncs.h"
#include "addrspace.h"
#include "cfe_timer.h"
#include "bcmtypes.h"
#include "boardparms.h"

#include "rdp_cpu_ring.h"
#include "phys_common_drv.h"
#include "unimac_drv.h"

#include "data_path_init.h"

#if defined(_BCM96838_) || defined(_BCM96848_)
#include "clk_rst.h"
#include "bcm_misc_hw_init.h"

static  S_DPI_CFG   DpiBasicConfig = {6,1536,0,0,0,0,0,0,0,0,0,0,2048,2};
#define DATA_PATH_INIT(PARAMS)  (rdp_pre_init() | data_path_init(PARAMS))
#define DATA_PATH_GO()          (data_path_go() | rdp_post_init())
#define DATA_PATH_SHUTDOWN()    rdp_shut_down()
#if defined(_BCM96838_)
#define ETH_NAME                "BCM6838 Ethernet"
#else
#define ETH_NAME                "BCM6848 Ethernet"
#endif
#endif

#if defined(_BCM963138_) || defined(_BCM963148_)
#include "bcm_ethsw.h"
#include "clk_rst.h"
#include "pmc_rdp.h"
extern int rdp_post_init(void);
extern int rdp_shut_down(void);
extern void rdp_enable_ubus_masters(void);
#define DATA_PATH_INIT(PARAMS)  data_path_init(PARAMS)
#define DATA_PATH_GO()          data_path_go()
#define DATA_PATH_SHUTDOWN()    (rdp_shut_down() | pmc_rdp_shut_down())
#define ETH_NAME                "BCM63138 Ethernet"
#endif


#define ETH_ALEN        6       /* Octets in one ethernet addr */


#if !defined(_BCM963138_) && !defined(_BCM963148_)
static const char* MAC_PHY_RATE_NAME[] =
{
	"10M Full",
	"10M Half",
	"100M Full",
	"100M Half",
	"1000M Full",
	"1000M Half",
	"2500M Full",
	"Down",
	"Error",
};
#endif /* !DSL */
typedef struct bcmenet_softc {
    uint8_t hwaddr[ETH_ALEN];
    int macId[BP_MAX_SWITCH_PORTS];
    int phyId[BP_MAX_SWITCH_PORTS];
    phy_rate_t phy_rate[BP_MAX_SWITCH_PORTS];
    uint32_t linkCheck;
} bcmenet_softc;

/** Prototypes. **/
static void bcm6xxx_impl2_ether_probe( cfe_driver_t * drv,    unsigned long probe_a,
                                 unsigned long probe_b, void * probe_ptr );
static int bcm6xxx_impl2_ether_open(cfe_devctx_t *ctx);
static int bcm6xxx_impl2_ether_read(cfe_devctx_t *ctx,iocb_buffer_t *buffer);
static int bcm6xxx_impl2_ether_inpstat(cfe_devctx_t *ctx,iocb_inpstat_t *inpstat);
static int bcm6xxx_impl2_ether_write(cfe_devctx_t *ctx,iocb_buffer_t *buffer);
static int bcm6xxx_impl2_ether_ioctl(cfe_devctx_t *ctx,iocb_buffer_t *buffer);
static int bcm6xxx_impl2_ether_close(cfe_devctx_t *ctx);
static int internal_open(bcmenet_softc * softc);

/** Variables. **/
const static cfe_devdisp_t bcm6xxx_impl2_ether_dispatch = {
    bcm6xxx_impl2_ether_open,
    bcm6xxx_impl2_ether_read,
    bcm6xxx_impl2_ether_inpstat,
    bcm6xxx_impl2_ether_write,
    bcm6xxx_impl2_ether_ioctl,
    bcm6xxx_impl2_ether_close,
    NULL,
    NULL
};

const cfe_driver_t bcm6xxx_impl2_enet = {
    ETH_NAME,
    "eth",
    CFE_DEV_NETWORK,
    &bcm6xxx_impl2_ether_dispatch,
    bcm6xxx_impl2_ether_probe
};

#if !defined(_BCM963138_) && !defined(_BCM963148_)
static void setUnimacRate(int macId, UNIMAC_SPEED rate, UNIMAC_DUPLEX mode)
{  
	mac_hwapi_set_speed(macId,rate);
	mac_hwapi_set_duplex(macId, mode);
}

/*=======================================================================*/
/* function   switch MAC  between the modes 10Mb, 100Mb and 1G           */
/* according to link status                                              */ 
/*=======================================================================*/
static phy_rate_t CheckAndSetLink(int macId, phy_rate_t cur_rate)
{
    phy_rate_t phyRate;

	phyRate = port_get_link_speed(macId);

	if(phyRate == cur_rate)
		return phyRate;

    switch( phyRate )
    {
    case PHY_RATE_2500_FULL:
        setUnimacRate(macId, UNIMAC_SPEED_2500, UNIMAC_DUPLEX_FULL);
        break;
    case PHY_RATE_1000_FULL:
        setUnimacRate(macId, UNIMAC_SPEED_1000, UNIMAC_DUPLEX_FULL);
        break;
    case PHY_RATE_1000_HALF:
        setUnimacRate(macId, UNIMAC_SPEED_1000, UNIMAC_DUPLEX_HALF);
        break;
    case PHY_RATE_100_FULL:
        setUnimacRate(macId, UNIMAC_SPEED_100, UNIMAC_DUPLEX_FULL);
        break;
    case PHY_RATE_100_HALF:
        setUnimacRate(macId, UNIMAC_SPEED_100, UNIMAC_DUPLEX_HALF);
        break;
    case PHY_RATE_10_FULL:
        setUnimacRate(macId, UNIMAC_SPEED_10, UNIMAC_DUPLEX_FULL);
        break;
    case PHY_RATE_10_HALF:
        setUnimacRate(macId, UNIMAC_SPEED_10, UNIMAC_DUPLEX_HALF);
        break;
    default:
        break;
    }

    return phyRate;
}
#endif /* !_BCM963138_ && !_BCM963148_ */

static int boardPorts = 0;

/** Functions. **/
#if !defined(_BCM963138_) && !defined(_BCM963148_)
static int internal_open(bcmenet_softc * softc)
{
	const ETHERNET_MAC_INFO *EnetInfo;
	int errorCode;
    int mac_id;

	EnetInfo = BpGetEthernetMacInfoArrayPtr();

    for ( mac_id = 0; mac_id <= BP_MAX_SWITCH_PORTS ; mac_id++)
    {
    	if(EnetInfo[0].sw.port_map & (1<<mac_id))
        {
            softc->macId[mac_id] = mac_id;
            softc->phyId[mac_id] = BpGetPhyAddr(0,mac_id);
            softc->phy_rate[mac_id] = PHY_RATE_LINK_DOWN;
	    boardPorts++;
        }
    }

    errorCode =  rdp_pre_init();
    if (errorCode != DPI_RC_OK) {
        printf("%s:%d rdp_pre_init error(error code = %d)\n", __FUNCTION__,__LINE__, errorCode);
        return -1;
    }

    errorCode = get_rdp_freq((unsigned int*)&DpiBasicConfig.runner_freq);
    if (errorCode != DPI_RC_OK)
    {
        printf("%s:%d get_rdp_freq error(error code = %d)\n", __FUNCTION__,__LINE__, errorCode);
        return -1;
    }
    DpiBasicConfig.enabled_port_map = EnetInfo[0].sw.port_map;
    errorCode =  DATA_PATH_INIT(&DpiBasicConfig);
    if (errorCode != DPI_RC_OK)
    {
        printf("%s:%d DATA_PATH_INIT error(error code = %d)\n", __FUNCTION__,__LINE__, errorCode);
        return -1;
    }

    errorCode =  DATA_PATH_GO();
    if (errorCode != DPI_RC_OK)
    {
        printf("%s:%d DATA_PATH_GO error(error code = %d)\n", __FUNCTION__,__LINE__, errorCode);
        return -1;
    }

    /*enable the available ports on board,
     * must call port_phy_init after oren_data_path_go
     * */
    for ( mac_id = 0; mac_id <= BP_MAX_SWITCH_PORTS ; mac_id++)
    {
    	if(EnetInfo[0].sw.port_map & (1<<mac_id) && 
            ((EnetInfo[0].sw.phy_id[mac_id] & MAC_IFACE) != MAC_IF_SERDES))
        {
            if (EnetInfo[0].sw.phy_id[mac_id] & MAC_MAC_IF)
                mac_hwapi_set_external_conf(mac_id, 0);

			/*in CFE the network stack allocate 1514 bytes for packet
			 * so we need to put the mtu 1514 + CRC = 1518           */
			mac_hwapi_set_tx_max_frame_len(mac_id,1518);
			mac_hwapi_set_rx_max_frame_len(mac_id,1518);
			port_phy_init(mac_id);
        }
    }

    softc->linkCheck = 0;

    return 0;
}

#else /* BCM63138 / BCM63148 */

static int internal_open(bcmenet_softc * softc)
{
    const int bcm63138_tm_def_ddr_size = 0x1400000;
    const int bcm63138_tm_mc_def_ddr_size = 0x0400000;

    unsigned char *ddr_tm_base_address_phys;
    unsigned char *ddr_multicast_base_address_phys;
    unsigned char *ddr_tm_base_address;
    unsigned char *ddr_multicast_base_address;
    S_DPI_CFG DpiBasicConfig;
    int errorCode;
    int mac_id;

    /* TBD.  This section is temporary to ensure that packets goto EMAC 1.
     *       CFE configures the SF2 in unmanaged mode. The board parameter
     *       entry is for managed mode.  A board parameter entry for unmanaged
     *       mode needs to be added.
     */
    boardPorts = 1;
    mac_id = 0;
    softc->macId[mac_id] = 1;
    softc->phyId[mac_id] = BpGetPhyAddr(0,1);
    softc->phy_rate[mac_id] = PHY_RATE_LINK_DOWN;

    /* Initialize SF2 */
    bcm_ethsw_open();

    /* In CFE, cached addresses and physical addresses are the same address. */
    ddr_tm_base_address_phys = (void*)(uint32_t)KMALLOC(bcm63138_tm_def_ddr_size, 0x200000);
    ddr_multicast_base_address_phys = (void*)(uint32_t)KMALLOC(bcm63138_tm_mc_def_ddr_size, 0x200000);
    ddr_tm_base_address = (void*)cache_to_uncache((uint32_t)ddr_tm_base_address_phys);
    ddr_multicast_base_address = (void*)cache_to_uncache((uint32_t)ddr_multicast_base_address_phys);

    memset(ddr_tm_base_address, 0x00, bcm63138_tm_def_ddr_size);
    memset(ddr_multicast_base_address, 0x00, bcm63138_tm_mc_def_ddr_size);

    if( ddr_tm_base_address == NULL || ddr_multicast_base_address == NULL )
    {
        printf("%s:%d Error allocating memory for Runner\n", __FUNCTION__,__LINE__);
        return -1;
    }

    errorCode = pmc_rdp_init();
    if (errorCode != DPI_RC_OK)
    {
        printf("%s:%d pmc_rdp_init error(error code = %d)\n", __FUNCTION__,__LINE__, errorCode);
        return -1;
    }

    rdp_enable_ubus_masters();

    errorCode = get_rdp_freq((unsigned int*)&DpiBasicConfig.runner_freq);
    if (errorCode != DPI_RC_OK)
    {
        printf("%s:%d get_rdp_freq error(error code = %d)\n", __FUNCTION__,__LINE__, errorCode);
        return -1;
    }

	DpiBasicConfig.mtu_size = 1536;
	DpiBasicConfig.headroom_size = 0;
    DpiBasicConfig.runner_tm_base_addr = (uint32_t) ddr_tm_base_address;
    DpiBasicConfig.runner_tm_base_addr_phys = (uint32_t) ddr_tm_base_address_phys;
    DpiBasicConfig.runner_tm_size = bcm63138_tm_def_ddr_size / (1024 * 1024);
    DpiBasicConfig.runner_mc_base_addr = (uint32_t) ddr_multicast_base_address;
    DpiBasicConfig.runner_mc_base_addr_phys = (uint32_t) ddr_multicast_base_address_phys;
    DpiBasicConfig.runner_mc_size = bcm63138_tm_mc_def_ddr_size / (1024 * 1024);

    errorCode = DATA_PATH_INIT(&DpiBasicConfig);
    if (errorCode != DPI_RC_OK)
    {
        printf("%s:%d DATA_PATH_INIT error(error code = %d)\n", __FUNCTION__,__LINE__, errorCode);
        return -1;
    }

    errorCode =  rdp_post_init();
    if (errorCode != DPI_RC_OK)
    {
        printf("%s:%d rdp_post_init error(error code = %d)\n", __FUNCTION__,__LINE__, errorCode);
        return -1;
    }

    errorCode = DATA_PATH_GO();
    if (errorCode != DPI_RC_OK)
    {
        printf("%s:%d DATA_PATH_GO error(error code = %d)\n", __FUNCTION__,__LINE__, errorCode);
        return -1;
    }

    softc->linkCheck = 0;

    return 0;
}
#endif

static void bcm6xxx_impl2_ether_probe( cfe_driver_t * drv,    unsigned long probe_a,
                                unsigned long probe_b, void * probe_ptr )
{
    bcmenet_softc * softc;
    char descr[100];

    softc = (bcmenet_softc *) KMALLOC( sizeof(bcmenet_softc), 0 );
    if( softc == NULL ) {
        xprintf( "Failed alloc softc mem\n" );
    } else {
        memset( softc, 0, sizeof(bcmenet_softc) );

        if (internal_open( softc ) == -1)
            xprintf("Failed init enet hw\n");
        else
        {
            /* temporarily comment out this line. It supposed to fix the
               uninitialized stack variable issue but some how this trigger
               issue in rdp init down the road */
            xsprintf(descr, "%s eth %d", drv->drv_description, probe_a );
            cfe_attach( drv, softc, NULL, descr );
        }
    }
}

#if 0
static void* cpu_ring_alloc_sysb(cpu_ring_sysb_type type, uint32_t *pDataPtr)
{
	void* ptr = KMALLOC(MAX_PACKET_SIZE_IN_DDR,16);
	*pDataPtr = (uint32_t)ptr;
	return ptr;
}
static void cpu_ring_free_sysb(void *pDataPtr)
{
	KFREE(pDataPtr);

}
#endif
#if defined(_BCM963138_) || defined(_BCM963148_)
static int bcm6xxx_impl2_ether_open(cfe_devctx_t *ctx)
{
    int rc;
    bdmf_phys_addr_t ring_head;

    rc = rdp_cpu_ring_create_ring(BL_LILAC_RDD_CPU_RX_QUEUE_0, rdpa_ring_data, 32,&ring_head, BCM_PKTBUF_SIZE, NULL, 0);
    if(rc)
    {
    	printk("Error creating CPU Rx Ring \n");
    	return rc;
    }

    rc = rdd_ring_init(BL_LILAC_RDD_CPU_RX_QUEUE_0, 0, VIRT_TO_PHYS(ring_head),
                        32,sizeof(RDD_CPU_RX_DESCRIPTOR_DTS),BL_LILAC_RDD_CPU_RX_QUEUE_0,0,0,0);
    if(rc)
    {
        printk("Error rdd_ring_init CPU Rx Ring \n");
        return rc;
    }
    return 0;
}
#else /* !_BCM963138_ && !_BCM963148_ */
static int bcm6xxx_impl2_ether_open(cfe_devctx_t *ctx)
{
    int rc,mac_id;
    bdmf_phys_addr_t ring_head;
    bcmenet_softc * softc = (bcmenet_softc *) ctx->dev_softc;
    const ETHERNET_MAC_INFO *EnetInfo;
    EnetInfo = BpGetEthernetMacInfoArrayPtr();

    rc = rdp_cpu_ring_create_ring(BL_LILAC_RDD_CPU_RX_QUEUE_0,rdpa_ring_data,32,&ring_head, BCM_PKTBUF_SIZE, NULL, 0);
    if(rc)
    {
    	printk("Error creating CPU Rx Ring \n");
    	return rc;
    }

    rc = rdd_ring_init(BL_LILAC_RDD_CPU_RX_QUEUE_0, 0, VIRT_TO_PHYS(ring_head),
                            32,sizeof(RDD_CPU_RX_DESCRIPTOR_DTS),BL_LILAC_RDD_CPU_RX_QUEUE_0,0,0,0);
    if(rc)
    {
        printk("Error rdd_ring_init CPU Rx Ring \n");
        return rc;
    }

    for ( mac_id = 0; mac_id <= BP_MAX_SWITCH_PORTS ; mac_id++)
    {
    	if(EnetInfo[0].sw.port_map & (1<<mac_id) && 
            ((EnetInfo[0].sw.phy_id[mac_id] & MAC_IFACE) != MAC_IF_SERDES))
        {
            softc->phy_rate[mac_id] = CheckAndSetLink(softc->macId[mac_id], softc->phy_rate[mac_id]);
            printf("Open PHY %d on MAC %d : link state = %s\n\r", softc->phyId[mac_id], softc->macId[mac_id],
                MAC_PHY_RATE_NAME[softc->phy_rate[mac_id]]);
        }
    }
    return 0;
}
#endif /* !_BCM963138_ && !_BCM963148_ */


static int bcm6xxx_impl2_ether_read( cfe_devctx_t * ctx, iocb_buffer_t * buffer )
{
    BL_LILAC_RDD_ERROR_DTE errorCode;
    //uint32_t				waiting;
    CPU_RX_PARAMS		cpuRx;
    cpuRx.data_ptr = (void *)buffer->buf_ptr;
	/* call runner driver to receive the packet - driver will copy the data */
	//waiting = rdp_cpu_ring_get_queued(BL_LILAC_RDD_CPU_RX_QUEUE_0);
	//printf("There are %d packets waiting to be read\n",waiting);
	errorCode =  rdp_cpu_ring_read_packet_copy(BL_LILAC_RDD_CPU_RX_QUEUE_0,&cpuRx);
#if !defined(_BCM963138_) && !defined(_BCM963148_)
	if(errorCode == BL_LILAC_RDD_ERROR_CPU_RX_QUEUE_EMPTY)
	{
        bcmenet_softc *softc = (bcmenet_softc *) ctx->dev_softc;
        phy_rate_t newRate;
        const ETHERNET_MAC_INFO *EnetInfo;
        int mac_id;
        EnetInfo = BpGetEthernetMacInfoArrayPtr();

        if (++softc->linkCheck > 1000) 
        {
            for ( mac_id = 0; mac_id <= BP_MAX_SWITCH_PORTS ; mac_id++)
            {
                if(EnetInfo[0].sw.port_map & (1<<mac_id) && 
                    ((EnetInfo[0].sw.phy_id[mac_id] & MAC_IFACE) != MAC_IF_SERDES))
                {
                    newRate = CheckAndSetLink(softc->macId[mac_id], softc->phy_rate[mac_id]);
                    if (softc->phy_rate[mac_id] != newRate)
                    {
                        printf("PHY %d on MAC %d : link state = %s\n\r", 
                            softc->phyId[mac_id], softc->macId[mac_id], 
                            MAC_PHY_RATE_NAME[newRate]);
                        softc->phy_rate[mac_id] = newRate;
                    }
                }
            }
        }
        return -1;
    }
#endif /* !_BCM963138_ && !_BCM963148_ */

    if((errorCode != BL_LILAC_RDD_OK) && (errorCode != BL_LILAC_RDD_ERROR_CPU_RX_QUEUE_EMPTY))
    {
        printf("NET: RDD error receive : %d len=%d reason=%d\n",errorCode,(int)cpuRx.packet_size,cpuRx.reason);
        return -1;
    }
    if(!cpuRx.packet_size && errorCode != BL_LILAC_RDD_ERROR_CPU_RX_QUEUE_EMPTY) {
        printf("%s: Get zero length packet from RDD\n",__FUNCTION__);
        return -1;
    }

    if(errorCode != 0)
    {
        buffer->buf_retlen = 0;
        return -1;
    }

    buffer->buf_retlen = cpuRx.packet_size;

    return 0;
}


static int bcm6xxx_impl2_ether_inpstat( cfe_devctx_t * ctx, iocb_inpstat_t * inpstat )
{
    // inp_status == 1 -> data available
    inpstat->inp_status = 1;
    return 0;
}


static int bcm6xxx_impl2_ether_write(cfe_devctx_t *ctx,iocb_buffer_t *buffer)
{

    bcmenet_softc *softc = (bcmenet_softc *) ctx->dev_softc;
    unsigned char txbuf[60];
    BL_LILAC_RDD_ERROR_DTE errorCode;

   	/* add padding to get 64 bytes frame (4bytes is CRC added by MAC).
       padding is zeroes up to 60 bytes */
    if (buffer->buf_length < 60) {
        memcpy(txbuf, buffer->buf_ptr, buffer->buf_length );
        memset(txbuf + buffer->buf_length, 0, 60 - buffer->buf_length);
    }
#if !defined(_BCM963138_) && !defined(_BCM963148_)
    const ETHERNET_MAC_INFO *EnetInfo;
    int mac_id;
    EnetInfo = BpGetEthernetMacInfoArrayPtr();

    for ( mac_id = 0; mac_id <= BP_MAX_SWITCH_PORTS ; mac_id++)
    {
        if(EnetInfo[0].sw.port_map & (1<<mac_id) && 
            ((EnetInfo[0].sw.phy_id[mac_id] & MAC_IFACE) != MAC_IF_SERDES))
        {
        if (buffer->buf_length < 60) 
        {
            /* call runner driver to send the packet - driver will copy the data */
            errorCode = rdd_cpu_tx_write_eth_packet(txbuf, 60,
                BL_LILAC_RDD_EMAC_ID_0+softc->macId[mac_id], 0, BL_LILAC_RDD_QUEUE_0);
        }
        else {
            errorCode = rdd_cpu_tx_write_eth_packet(buffer->buf_ptr, buffer->buf_length,
                BL_LILAC_RDD_EMAC_ID_0+softc->macId[mac_id], 0, BL_LILAC_RDD_QUEUE_0);
        }
        if (errorCode != BL_LILAC_RDD_OK) {
            printf("%s: MAC=%d tx error = %d, len=%d\n", __FUNCTION__,softc->macId[mac_id],errorCode,buffer->buf_length );
        }
        }
    }
#else
    int i;
    for (i=0; i< boardPorts; i++)
    {
        if (buffer->buf_length < 60) {
            /* call runner driver to send the packet - driver will copy the data */
            errorCode = rdd_cpu_tx_write_eth_packet(txbuf, 60,
                BL_LILAC_RDD_EMAC_ID_0+softc->macId[i], 0, BL_LILAC_RDD_QUEUE_0);
        }
        else {
            errorCode = rdd_cpu_tx_write_eth_packet(buffer->buf_ptr, buffer->buf_length,
                BL_LILAC_RDD_EMAC_ID_0+softc->macId[i], 0, BL_LILAC_RDD_QUEUE_0);
        }
        if (errorCode != BL_LILAC_RDD_OK) {
            printf("%s: MAC=%d tx error = %d, len=%d\n", __FUNCTION__,softc->macId[i],errorCode,buffer->buf_length );
        }
    }
#endif
    

    return 0;
}

static int bcm6xxx_impl2_ether_ioctl(cfe_devctx_t *ctx, iocb_buffer_t *buffer)
{
    bcmenet_softc *softc = (bcmenet_softc *) ctx->dev_softc;

    switch ((int)buffer->buf_ioctlcmd) {
    case IOCTL_ETHER_GETHWADDR:
        memcpy(buffer->buf_ptr, softc->hwaddr, sizeof(softc->hwaddr));
        return 0;

    case IOCTL_ETHER_SETHWADDR:
        memcpy(softc->hwaddr, buffer->buf_ptr, sizeof(softc->hwaddr));
        return 0;

    default:
        return -1;
    }
    return 0;
}

static int bcm6xxx_impl2_ether_close(cfe_devctx_t *ctx)
{
    int rc;

#if defined(_BCM963138_) && defined(_BCM963148_)
    bcm_ethsw_close();
#endif

    rc = rdp_cpu_ring_delete_ring(BL_LILAC_RDD_CPU_RX_QUEUE_0);
    if(rc)
    {
        printk("Error creating CPU Rx Ring \n");
        return rc;
    }
    rc = DATA_PATH_SHUTDOWN();
    return rc;
}

