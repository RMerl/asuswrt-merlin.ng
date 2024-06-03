/* SPDX-License-Identifier: GPL-2.0+
 *
 *  Copyright 2019 Broadcom Ltd.
 */

#include <common.h>
#include <dm.h>
#include <net.h>

#include "rdp_cpu_ring.h"
#include "unimac_drv.h"
#include "data_path_init.h"
#include "bcm_ethsw.h"
#include "pmc_rdp.h"
#include "clk_rst.h"
#include "mmu_map_v7.h"

extern int rdp_post_init(void);
extern void rdp_enable_ubus_masters(void);

#define DATA_PATH_INIT(PARAMS)  data_path_init(PARAMS)
#define DATA_PATH_GO()          data_path_go()
#define DATA_PATH_SHUTDOWN()    (data_path_shutdown() | pmc_rdp_shut_down())

#define TM_DEF_SIZE             0x1400000
#define MC_DEF_SIZE             0x0400000


#define MAX_RX_BUFSIZE        2048

struct bcmenet_softc {
	uint32_t init;
	uint8_t hwaddr[14];
	uint8_t macId;
	uint8_t phyId;
	uint8_t rx_buf[MAX_RX_BUFSIZE];

	struct udevice *sf2_devp;
	bcm_ethsw_ops_t *sf2_ops;
};

phys_addr_t ddr_tm_base_address_phys;
phys_addr_t ddr_multicast_base_address_phys;
unsigned char *ddr_tm_base_address;
unsigned char *ddr_multicast_base_address;

static int allocate_rdp_memory(void)
{
	int rc = 0;

	/* In CFE, cached addresses and physical addresses are the same address. */
	if (gd->ram_size > 256*SZ_1M) {
		ddr_tm_base_address_phys = (phys_addr_t)(256*SZ_1M - TM_DEF_SIZE);
		ddr_multicast_base_address_phys = (phys_addr_t)(256*SZ_1M - TM_DEF_SIZE - MC_DEF_SIZE);
		mmu_set_region_dcache_behaviour(ddr_tm_base_address_phys, TM_DEF_SIZE, 
			SECTION_ATTR_NONCACHED_MEM);
		mmu_set_region_dcache_behaviour(ddr_multicast_base_address_phys, MC_DEF_SIZE, 
			SECTION_ATTR_NONCACHED_MEM);
#if 0
		rc = fdt_add_mem_rsv((void *)gd->fdt_blob, ddr_tm_base_address_phys, TM_DEF_SIZE);
		rc != fdt_add_mem_rsv((void *)gd->fdt_blob, ddr_multicast_base_address_phys, MC_DEF_SIZE);
		if (rc) 
			return rc;
#endif
		printf("\nWARNING: Memory region from 0x%08x - 0x%08x reserved used for runner\n", 
		       256*SZ_1M - MC_DEF_SIZE - TM_DEF_SIZE, 256*SZ_1M-1);
	} else {
		ddr_tm_base_address_phys = noncached_alloc(TM_DEF_SIZE, 0x200000);
		ddr_multicast_base_address_phys = noncached_alloc(MC_DEF_SIZE, 0x200000);
	}

	ddr_tm_base_address = (void *) ddr_tm_base_address_phys;
	ddr_multicast_base_address = (void *) ddr_multicast_base_address_phys;

	if (ddr_tm_base_address == NULL || ddr_multicast_base_address == NULL) {
		printf("%s:%d Error allocating memory for Runner\n",
		       __FUNCTION__, __LINE__);
		return -1;
	}

	memset(ddr_tm_base_address, 0x00, TM_DEF_SIZE);
	memset(ddr_multicast_base_address, 0x00, MC_DEF_SIZE);

	return rc;
}

static int internal_open(struct bcmenet_softc *softc)
{
	S_DPI_CFG DpiBasicConfig;
	int errorCode;

	/* TBD.  This section is temporary to ensure that packets goto EMAC 1.
	 *       CFE configures the SF2 in unmanaged mode. The board parameter
	 *       entry is for managed mode.  A board parameter entry for unmanaged
	 *       mode needs to be added.
	 */
	softc->macId = 1;

	/* Initialize SF2 */
	if (softc->sf2_devp)
		softc->sf2_ops->open(softc->sf2_devp);

	errorCode = pmc_rdp_init();
	if (errorCode != DPI_RC_OK) {
		printf("%s:%d pmc_rdp_init error(error code = %d)\n",
		       __FUNCTION__, __LINE__, errorCode);
		return -1;
	}

	rdp_enable_ubus_masters();

	errorCode = get_rdp_freq((unsigned int *)&DpiBasicConfig.runner_freq);
	if (errorCode != DPI_RC_OK) {
		printf("%s:%d get_rdp_freq error(error code = %d)\n",
		       __FUNCTION__, __LINE__, errorCode);
		return -1;
	}

	DpiBasicConfig.mtu_size = 1536;
	DpiBasicConfig.headroom_size = 0;
	DpiBasicConfig.runner_tm_base_addr = (uint32_t) ddr_tm_base_address;
	DpiBasicConfig.runner_tm_base_addr_phys =
	    (uint32_t) ddr_tm_base_address_phys;
	DpiBasicConfig.runner_tm_size =
	  TM_DEF_SIZE / (1024 * 1024);
	DpiBasicConfig.runner_mc_base_addr =
	    (uint32_t) ddr_multicast_base_address;
	DpiBasicConfig.runner_mc_base_addr_phys =
	    (uint32_t) ddr_multicast_base_address_phys;
	DpiBasicConfig.runner_mc_size =
	    MC_DEF_SIZE / (1024 * 1024);

	errorCode = DATA_PATH_INIT(&DpiBasicConfig);
	if (errorCode != DPI_RC_OK) {
		printf("%s:%d DATA_PATH_INIT error(error code = %d)\n",
		       __FUNCTION__, __LINE__, errorCode);
		return -1;
	}

	errorCode = rdp_post_init();
	if (errorCode != DPI_RC_OK) {
		printf("%s:%d rdp_post_init error(error code = %d)\n",
		       __FUNCTION__, __LINE__, errorCode);
		return -1;
	}

	errorCode = DATA_PATH_GO();
	if (errorCode != DPI_RC_OK) {
		printf("%s:%d DATA_PATH_GO error(error code = %d)\n",
		       __FUNCTION__, __LINE__, errorCode);
		return -1;
	}

	return 0;
}

static int bcmbca_rdp_eth_send(struct udevice *dev, void *buffer, int length)
{
	struct bcmenet_softc *priv = dev_get_priv(dev);
	unsigned char txbuf[60];
	BL_LILAC_RDD_ERROR_DTE errorCode;

	/* add padding to get 64 bytes frame (4bytes is CRC added by MAC).
	   padding is zeroes up to 60 bytes */
	if (length < 60) {
		memcpy(txbuf, buffer, length);
		memset(txbuf + length, 0, 60 - length);
	}

	if (length < 60) {
		/* call runner driver to send the packet - driver will copy the data */
		errorCode = rdd_cpu_tx_write_eth_packet(txbuf, 60,
							BL_LILAC_RDD_EMAC_ID_0 +
							priv->macId, 0,
							BL_LILAC_RDD_QUEUE_0);
	} else {
		errorCode = rdd_cpu_tx_write_eth_packet(buffer, length,
							BL_LILAC_RDD_EMAC_ID_0 +
							priv->macId, 0,
							BL_LILAC_RDD_QUEUE_0);
	}

	if (errorCode != BL_LILAC_RDD_OK) {
		printf("%s: MAC=%d tx error = %d, len=%d\n", __FUNCTION__,
		       priv->macId, errorCode, length);
	}

	return 0;
}

static int bcmbca_rdp_eth_recv(struct udevice *dev, int flags, uchar ** packetp)
{

	BL_LILAC_RDD_ERROR_DTE errorCode;
	CPU_RX_PARAMS cpuRx;
	struct bcmenet_softc *priv = dev_get_priv(dev);

	cpuRx.data_ptr = (void *)priv->rx_buf;
	/* call runner driver to receive the packet - driver will copy the data */
	//waiting = rdp_cpu_ring_get_queued(BL_LILAC_RDD_CPU_RX_QUEUE_0);
	//printf("There are %d packets waiting to be read\n",waiting);
	errorCode =
	    rdp_cpu_ring_read_packet_copy(BL_LILAC_RDD_CPU_RX_QUEUE_0, &cpuRx);
	if ((errorCode != BL_LILAC_RDD_OK)
	    && (errorCode != BL_LILAC_RDD_ERROR_CPU_RX_QUEUE_EMPTY)) {
		printf("NET: RDD error receive : %d len=%d reason=%d\n",
		       errorCode, (int)cpuRx.packet_size, cpuRx.reason);
		return -EIO;
	}

	if (!cpuRx.packet_size
	    && errorCode != BL_LILAC_RDD_ERROR_CPU_RX_QUEUE_EMPTY) {
		printf("%s: Get zero length packet from RDD\n", __FUNCTION__);
		return -EIO;
	}

	if (errorCode == BL_LILAC_RDD_ERROR_CPU_RX_QUEUE_EMPTY) {
		return -EAGAIN;
	}

	if (errorCode != BL_LILAC_RDD_OK) {
		return -EIO;
	}

	*packetp = priv->rx_buf;
	return cpuRx.packet_size;
}

static int bcmbca_rdp_eth_start(struct udevice *dev)
{
	int rc;
	bdmf_phys_addr_t ring_head;
	struct bcmenet_softc *priv = dev_get_priv(dev);

	debug("bcmbca_rdp_eth_start called %s\n", dev->name);

	if (priv->init)
		return 0;

	rc = internal_open(priv);
	if (rc) {
		printk("internal open falled rc %d \n", rc);
		return rc;
	}

	rc = rdp_cpu_ring_create_ring(BL_LILAC_RDD_CPU_RX_QUEUE_0,
				      rdpa_ring_data, 32, &ring_head,
				      BCM_PKTBUF_SIZE, NULL, 0);
	if (rc) {
		printk("Error creating CPU Rx Ring rc %d\n", rc);
		return rc;
	}

	rc = rdd_ring_init(BL_LILAC_RDD_CPU_RX_QUEUE_0, 0,
			   VIRT_TO_PHYS(ring_head), 32,
			   sizeof(RDD_CPU_RX_DESCRIPTOR_DTS),
			   BL_LILAC_RDD_CPU_RX_QUEUE_0, 0, 0, 0);
	if (rc) {
		printk("Error rdd_ring_init CPU Rx Ring rc %d\n", rc);
		return rc;
	}

	priv->init = 1;
	return 0;
}

static void bcmbca_rdp_eth_stop(struct udevice *dev)
{
	debug("bcmbca_rdp_eth_stop called %s\n", dev->name);

	return;
}

static int bcmbca_rdp_eth_read_mac(struct udevice *dev)
{
	return 0;
}

static int bcmbca_rdp_eth_probe(struct udevice *dev)
{
	struct udevice *sf2_devp;
	struct bcmenet_softc *priv = dev_get_priv(dev);
	int size;
	uint phandle_id;
	const fdt32_t *list;


	debug("bcmbca_rdp_eth_probe called %s\n", dev->name);

	priv->sf2_devp = NULL;
	list = dev_read_prop(dev, "ethsw", &size);

	if (size > 0) {
		phandle_id = fdt32_to_cpu(*list);
		if (!uclass_get_device_by_phandle_id (UCLASS_NOP, phandle_id, &sf2_devp)) {
			printf("sf2 found, should have been probed name %s driver name %s\n", 
			sf2_devp->name, sf2_devp->driver->name);
			priv->sf2_devp = sf2_devp;
			priv->sf2_ops = dev_get_priv(sf2_devp);
			priv->sf2_ops->init(priv->sf2_devp);
		}
	}

	priv->init = 0;

	if (allocate_rdp_memory() != 0) {
		printf("%s:%d allocate rdp memory failed\n",
		       __FUNCTION__, __LINE__);
		return -1;
	}

	return 0;
}

static int bcmbca_rdp_eth_remove(struct udevice *dev)
{
	int rc = 0;
	struct bcmenet_softc *priv = dev_get_priv(dev);

	debug("bcmbca_rdp_eth_removed called %s\n", dev->name);

	if (priv->sf2_devp)
		priv->sf2_ops->close(priv->sf2_devp);

	if (!priv->init)
		return rc;

	rc = rdp_cpu_ring_delete_ring(BL_LILAC_RDD_CPU_RX_QUEUE_0);
	if (rc) {
		printk("Error deleting CPU Rx Ring \n");
		return -1;
	}

	rc = DATA_PATH_SHUTDOWN();

	return rc;
}

static const struct eth_ops bcmbca_rdp_eth_ops = {
	.start = bcmbca_rdp_eth_start,
	.send = bcmbca_rdp_eth_send,
	.recv = bcmbca_rdp_eth_recv,
	.stop = bcmbca_rdp_eth_stop,
	.read_rom_hwaddr = bcmbca_rdp_eth_read_mac,
};

static const struct udevice_id bcmbca_rdp_eth_ids[] = {
	{.compatible = "brcm,bcmbca-rdp"},
	{}
};

U_BOOT_DRIVER(eth_bcmbca_rdp) = {
	.name	= "eth_bcmbca_rdp",
	.id	= UCLASS_ETH,
	.flags	= DM_REMOVE_ACTIVE_ALL,
	.of_match = bcmbca_rdp_eth_ids,
	.probe	= bcmbca_rdp_eth_probe,
	.remove	= bcmbca_rdp_eth_remove,
	.ops	= &bcmbca_rdp_eth_ops,
	.priv_auto_alloc_size = sizeof(struct bcmenet_softc),
	.platdata_auto_alloc_size = sizeof(struct eth_pdata),
};
