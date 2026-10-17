// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2014-2016 Freescale Semiconductor, Inc.
 * Copyright 2017, 2023 NXP
 */

#include <common.h>
#include <cpu_func.h>
#include <dm/device_compat.h>
#include <fsl-mc/fsl_dpmac.h>
#include <fsl-mc/ldpaa_wriop.h>
#include <hwconfig.h>
#include <log.h>
#include <malloc.h>
#include <miiphy.h>
#include <net.h>
#include <phy.h>
#include <asm/io.h>
#include <asm/types.h>
#include <linux/bug.h>
#include <linux/compat.h>
#include <linux/delay.h>
#include <asm/global_data.h>
#include <net/ldpaa_eth.h>
#include "ldpaa_eth.h"

#ifdef CONFIG_PHYLIB
static void init_phy(struct udevice *dev)
{
	struct ldpaa_eth_priv *priv = dev_get_priv(dev);

	priv->phy = dm_eth_phy_connect(dev);

	if (!priv->phy)
		return;

	phy_config(priv->phy);
}
#endif

static void ldpaa_eth_collect_dpni_stats(struct udevice *dev, u64 *data)
{
	union dpni_statistics dpni_stats;
	int dpni_stats_page_size[DPNI_STATISTICS_CNT] = {
		sizeof(dpni_stats.page_0),
		sizeof(dpni_stats.page_1),
		sizeof(dpni_stats.page_2),
		sizeof(dpni_stats.page_3),
		sizeof(dpni_stats.page_4),
		sizeof(dpni_stats.page_5),
		sizeof(dpni_stats.page_6),
	};
	int j, k, num_cnt, err, i = 0;

	for (j = 0; j <= 6; j++) {
		/* We're not interested in pages 4 & 5 for now */
		if (j == 4 || j == 5)
			continue;
		err = dpni_get_statistics(dflt_mc_io, MC_CMD_NO_FLAGS,
					  dflt_dpni->dpni_handle,
					  j, 0, &dpni_stats);
		if (err) {
			memset(&dpni_stats, 0, sizeof(dpni_stats));
			printf("dpni_get_stats(%d) failed\n", j);
		}

		num_cnt = dpni_stats_page_size[j] / sizeof(u64);
		for (k = 0; k < num_cnt; k++)
			*(data + i++) = dpni_stats.raw.counter[k];
	}
}

static void ldpaa_eth_add_dpni_stats(struct udevice *dev, u64 *data)
{
	struct ldpaa_eth_priv *priv = dev_get_priv(dev);
	int i;

	for (i = 0; i < LDPAA_ETH_DPNI_NUM_STATS; i++)
		priv->dpni_stats[i] += data[i];
}

static void ldpaa_eth_collect_dpmac_stats(struct udevice *dev, u64 *data)
{
	struct ldpaa_eth_priv *priv = dev_get_priv(dev);
	int err, i;
	u64 value;

	for (i = 0; i < LDPAA_ETH_DPMAC_NUM_STATS; i++) {
		err = dpmac_get_counter(dflt_mc_io, MC_CMD_NO_FLAGS,
					priv->dpmac_handle, i,
					&value);
		if (err)
			printf("dpmac_get_counter(%d) failed\n", i);

		*(data + i) = value;
	}
}

static void ldpaa_eth_add_dpmac_stats(struct udevice *dev, u64 *data)
{
	struct ldpaa_eth_priv *priv = dev_get_priv(dev);
	int i;

	for (i = 0; i < LDPAA_ETH_DPMAC_NUM_STATS; i++)
		priv->dpmac_stats[i] += data[i];
}

#ifdef DEBUG
static void ldpaa_eth_dump_dpni_stats(struct udevice *dev, u64 *data)
{
	int i;

	printf("DPNI counters:\n");
	for (i = 0; i < LDPAA_ETH_DPNI_NUM_STATS; i++)
		printf("  %s: %llu\n", ldpaa_eth_dpni_stat_strings[i], data[i]);
}

static void ldpaa_eth_dump_dpmac_stats(struct udevice *dev, u64 *data)
{
	int i;

	printf("DPMAC counters:\n");
	for (i = 0; i < LDPAA_ETH_DPMAC_NUM_STATS; i++)
		printf("  %s: %llu\n", ldpaa_eth_dpmac_stat_strings[i], data[i]);
}
#endif

static void ldpaa_eth_rx(struct ldpaa_eth_priv *priv,
			 const struct dpaa_fd *fd)
{
	u64 fd_addr;
	uint16_t fd_offset;
	uint32_t fd_length;
	struct ldpaa_fas *fas;
	uint32_t status, err;
	u32 timeo = (CONFIG_SYS_HZ * 2) / 1000;
	u32 time_start;
	struct qbman_release_desc releasedesc;
	struct qbman_swp *swp = dflt_dpio->sw_portal;

	fd_addr = ldpaa_fd_get_addr(fd);
	fd_offset = ldpaa_fd_get_offset(fd);
	fd_length = ldpaa_fd_get_len(fd);

	debug("Rx frame:data addr=0x%p size=0x%x\n", (u64 *)fd_addr, fd_length);

	if (fd->simple.frc & LDPAA_FD_FRC_FASV) {
		/* Read the frame annotation status word and check for errors */
		fas = (struct ldpaa_fas *)
				((uint8_t *)(fd_addr) +
				dflt_dpni->buf_layout.private_data_size);
		status = le32_to_cpu(fas->status);
		if (status & LDPAA_ETH_RX_ERR_MASK) {
			printf("Rx frame error(s): 0x%08x\n",
			       status & LDPAA_ETH_RX_ERR_MASK);
			goto error;
		} else if (status & LDPAA_ETH_RX_UNSUPP_MASK) {
			printf("Unsupported feature in bitmask: 0x%08x\n",
			       status & LDPAA_ETH_RX_UNSUPP_MASK);
			goto error;
		}
	}

	debug("Rx frame: To Upper layer\n");
	net_process_received_packet((uint8_t *)(fd_addr) + fd_offset,
				    fd_length);

error:
	flush_dcache_range(fd_addr, fd_addr + LDPAA_ETH_RX_BUFFER_SIZE);
	qbman_release_desc_clear(&releasedesc);
	qbman_release_desc_set_bpid(&releasedesc, dflt_dpbp->dpbp_attr.bpid);
	time_start = get_timer(0);
	do {
		/* Release buffer into the QBMAN */
		err = qbman_swp_release(swp, &releasedesc, &fd_addr, 1);
	} while (get_timer(time_start) < timeo && err == -EBUSY);

	if (err == -EBUSY)
		printf("Rx frame: QBMAN buffer release fails\n");

	return;
}

static int ldpaa_eth_pull_dequeue_rx(struct udevice *dev,
				     int flags, uchar **packetp)
{
	struct ldpaa_eth_priv *priv = dev_get_priv(dev);
	const struct ldpaa_dq *dq;
	const struct dpaa_fd *fd;
	int i = 5, err = 0, status;
	u32 timeo = (CONFIG_SYS_HZ * 2) / 1000;
	u32 time_start;
	static struct qbman_pull_desc pulldesc;
	struct qbman_swp *swp = dflt_dpio->sw_portal;

	while (--i) {
		qbman_pull_desc_clear(&pulldesc);
		qbman_pull_desc_set_numframes(&pulldesc, 1);
		qbman_pull_desc_set_fq(&pulldesc, priv->rx_dflt_fqid);

		err = qbman_swp_pull(swp, &pulldesc);
		if (err < 0) {
			printf("Dequeue frames error:0x%08x\n", err);
			continue;
		}

		time_start = get_timer(0);

		 do {
			dq = qbman_swp_dqrr_next(swp);
		} while (get_timer(time_start) < timeo && !dq);

		if (dq) {
			/* Check for valid frame. If not sent a consume
			 * confirmation to QBMAN otherwise give it to NADK
			 * application and then send consume confirmation to
			 * QBMAN.
			 */
			status = (uint8_t)ldpaa_dq_flags(dq);
			if ((status & LDPAA_DQ_STAT_VALIDFRAME) == 0) {
				debug("Dequeue RX frames:");
				debug("No frame delivered\n");

				qbman_swp_dqrr_consume(swp, dq);
				continue;
			}

			fd = ldpaa_dq_fd(dq);

			/* Obtain FD and process it */
			ldpaa_eth_rx(priv, fd);
			qbman_swp_dqrr_consume(swp, dq);
			break;
		} else {
			err = -ENODATA;
			debug("No DQRR entries\n");
			break;
		}
	}

	return err;
}

static int ldpaa_eth_tx(struct udevice *dev, void *buf, int len)
{
	struct ldpaa_eth_priv *priv = dev_get_priv(dev);
	struct dpaa_fd fd;
	u64 buffer_start;
	int data_offset, err;
	u32 timeo = (CONFIG_SYS_HZ * 10) / 1000;
	u32 time_start;
	struct qbman_swp *swp = dflt_dpio->sw_portal;
	struct qbman_eq_desc ed;
	struct qbman_release_desc releasedesc;

	/* Setup the FD fields */
	memset(&fd, 0, sizeof(fd));

	data_offset = priv->tx_data_offset;

	do {
		err = qbman_swp_acquire(dflt_dpio->sw_portal,
					dflt_dpbp->dpbp_attr.bpid,
					&buffer_start, 1);
	} while (err == -EBUSY);

	if (err <= 0) {
		printf("qbman_swp_acquire() failed\n");
		return -ENOMEM;
	}

	debug("TX data: malloc buffer start=0x%p\n", (u64 *)buffer_start);

	memcpy(((uint8_t *)(buffer_start) + data_offset), buf, len);

	flush_dcache_range(buffer_start, buffer_start +
					LDPAA_ETH_RX_BUFFER_SIZE);

	ldpaa_fd_set_addr(&fd, (u64)buffer_start);
	ldpaa_fd_set_offset(&fd, (uint16_t)(data_offset));
	ldpaa_fd_set_bpid(&fd, dflt_dpbp->dpbp_attr.bpid);
	ldpaa_fd_set_len(&fd, len);

	fd.simple.ctrl = LDPAA_FD_CTRL_ASAL | LDPAA_FD_CTRL_PTA |
				LDPAA_FD_CTRL_PTV1;

	qbman_eq_desc_clear(&ed);
	qbman_eq_desc_set_no_orp(&ed, 0);
	qbman_eq_desc_set_qd(&ed, priv->tx_qdid, priv->tx_flow_id, 0);

	time_start = get_timer(0);

	while (get_timer(time_start) < timeo) {
		err = qbman_swp_enqueue(swp, &ed,
				(const struct qbman_fd *)(&fd));
		if (err != -EBUSY)
			break;
	}

	if (err < 0) {
		printf("error enqueueing Tx frame\n");
		goto error;
	}

	return err;

error:
	qbman_release_desc_clear(&releasedesc);
	qbman_release_desc_set_bpid(&releasedesc, dflt_dpbp->dpbp_attr.bpid);
	time_start = get_timer(0);
	do {
		/* Release buffer into the QBMAN */
		err = qbman_swp_release(swp, &releasedesc, &buffer_start, 1);
	} while (get_timer(time_start) < timeo && err == -EBUSY);

	if (err == -EBUSY)
		printf("TX data: QBMAN buffer release fails\n");

	return err;
}

static int ldpaa_get_dpmac_state(struct ldpaa_eth_priv *priv,
				 struct dpmac_link_state *state)
{
	phy_interface_t enet_if;
	struct phy_device *phydev = NULL;
	int err;

	/* let's start off with maximum capabilities */
	enet_if = wriop_get_enet_if(priv->dpmac_id);
	switch (enet_if) {
	case PHY_INTERFACE_MODE_XGMII:
		state->rate = SPEED_10000;
		break;
	default:
		state->rate = SPEED_1000;
		break;
	}

	state->up = 1;
	state->options |= DPMAC_LINK_OPT_AUTONEG;
	phydev = priv->phy;

	if (phydev) {
		err = phy_startup(phydev);
		if (err) {
			printf("%s: Could not initialize\n", phydev->dev->name);
			state->up = 0;
		} else if (phydev->link) {
			state->rate = min(state->rate, (uint32_t)phydev->speed);
			if (!phydev->duplex)
				state->options |= DPMAC_LINK_OPT_HALF_DUPLEX;
			if (!phydev->autoneg)
				state->options &= ~DPMAC_LINK_OPT_AUTONEG;
		} else {
			state->up = 0;
		}
	}

	if (!phydev)
		state->options &= ~DPMAC_LINK_OPT_AUTONEG;

	if (!state->up) {
		state->rate = 0;
		state->options = 0;
		return -ENOLINK;
	}

	return 0;
}

static int ldpaa_eth_open(struct udevice *dev)
{
	struct eth_pdata *plat = dev_get_plat(dev);
	struct ldpaa_eth_priv *priv = dev_get_priv(dev);
	struct dpmac_link_state	dpmac_link_state = { 0 };
#ifdef DEBUG
	struct dpni_link_state link_state;
#endif
	int err = 0;
	struct dpni_queue d_queue_cfg = { 0 };
	struct dpni_queue_id d_queue;

	if (eth_is_active(dev))
		return 0;

	if (get_mc_boot_status() != 0) {
		printf("ERROR (MC is not booted)\n");
		return -ENODEV;
	}

	if (get_dpl_apply_status() == 0) {
		printf("ERROR (DPL is deployed. No device available)\n");
		return -ENODEV;
	}

	/* DPMAC initialization */
	err = ldpaa_dpmac_setup(priv);
	if (err < 0)
		goto err_dpmac_setup;

	err = ldpaa_get_dpmac_state(priv, &dpmac_link_state);
	if (err < 0)
		goto err_dpmac_bind;

	/* DPMAC binding DPNI */
	err = ldpaa_dpmac_bind(priv);
	if (err)
		goto err_dpmac_bind;

	/* DPNI initialization */
	err = ldpaa_dpni_setup(priv);
	if (err < 0)
		goto err_dpni_setup;

	err = ldpaa_dpbp_setup();
	if (err < 0)
		goto err_dpbp_setup;

	/* DPNI binding DPBP */
	err = ldpaa_dpni_bind(priv);
	if (err)
		goto err_dpni_bind;

	err = dpni_add_mac_addr(dflt_mc_io, MC_CMD_NO_FLAGS,
				dflt_dpni->dpni_handle, plat->enetaddr, 0, 0, 0);
	if (err) {
		printf("dpni_add_mac_addr() failed\n");
		return err;
	}

	err = dpni_enable(dflt_mc_io, MC_CMD_NO_FLAGS, dflt_dpni->dpni_handle);
	if (err < 0) {
		printf("dpni_enable() failed\n");
		return err;
	}

	err = dpmac_set_link_state(dflt_mc_io, MC_CMD_NO_FLAGS,
				  priv->dpmac_handle, &dpmac_link_state);
	if (err < 0) {
		printf("dpmac_set_link_state() failed\n");
		return err;
	}

#ifdef DEBUG
	printf("DPMAC link status: %d - ", dpmac_link_state.up);
	dpmac_link_state.up == 0 ? printf("down\n") :
	dpmac_link_state.up == 1 ? printf("up\n") : printf("error state\n");

	err = dpni_get_link_state(dflt_mc_io, MC_CMD_NO_FLAGS,
				  dflt_dpni->dpni_handle, &link_state);
	if (err < 0) {
		printf("dpni_get_link_state() failed\n");
		return err;
	}

	printf("DPNI link status: %d - ", link_state.up);
	link_state.up == 0 ? printf("down\n") :
	link_state.up == 1 ? printf("up\n") : printf("error state\n");
#endif

	memset(&d_queue, 0, sizeof(struct dpni_queue));
	err = dpni_get_queue(dflt_mc_io, MC_CMD_NO_FLAGS,
			     dflt_dpni->dpni_handle, DPNI_QUEUE_RX,
			     0, 0, &d_queue_cfg, &d_queue);
	if (err) {
		printf("dpni_get_queue failed\n");
		goto err_get_queue;
	}

	priv->rx_dflt_fqid = d_queue.fqid;

	err = dpni_get_qdid(dflt_mc_io, MC_CMD_NO_FLAGS, dflt_dpni->dpni_handle,
			    DPNI_QUEUE_TX, &priv->tx_qdid);
	if (err) {
		printf("dpni_get_qdid() failed\n");
		goto err_qdid;
	}

	return dpmac_link_state.up;

err_qdid:
err_get_queue:
	dpni_disable(dflt_mc_io, MC_CMD_NO_FLAGS, dflt_dpni->dpni_handle);
err_dpni_bind:
	ldpaa_dpbp_free();
err_dpbp_setup:
	dpni_close(dflt_mc_io, MC_CMD_NO_FLAGS, dflt_dpni->dpni_handle);
err_dpni_setup:
err_dpmac_bind:
	dpmac_close(dflt_mc_io, MC_CMD_NO_FLAGS, priv->dpmac_handle);
	dpmac_destroy(dflt_mc_io,
		      dflt_dprc_handle,
		      MC_CMD_NO_FLAGS, priv->dpmac_id);
err_dpmac_setup:
	return err;
}

static void ldpaa_eth_stop(struct udevice *dev)
{
	struct ldpaa_eth_priv *priv = dev_get_priv(dev);
	struct phy_device *phydev = NULL;
	int err = 0;
	u64 *data;

	if (!eth_is_active(dev))
		return;

	data = kzalloc(sizeof(u64) * LDPAA_ETH_DPNI_NUM_STATS, GFP_KERNEL);
	if (data) {
		ldpaa_eth_collect_dpni_stats(dev, data);
		ldpaa_eth_add_dpni_stats(dev, data);
#ifdef DEBUG
		ldpaa_eth_dump_dpni_stats(dev, data);
#endif
	}
	kfree(data);

	data = kzalloc(sizeof(u64) * LDPAA_ETH_DPMAC_NUM_STATS, GFP_KERNEL);
	if (data) {
		ldpaa_eth_collect_dpmac_stats(dev, data);
		ldpaa_eth_add_dpmac_stats(dev, data);
#ifdef DEBUG
		ldpaa_eth_dump_dpmac_stats(dev, data);
#endif
	}
	kfree(data);

	err = dprc_disconnect(dflt_mc_io, MC_CMD_NO_FLAGS,
			      dflt_dprc_handle, &dpmac_endpoint);
	if (err < 0)
		printf("dprc_disconnect() failed dpmac_endpoint\n");

	err = dpmac_close(dflt_mc_io, MC_CMD_NO_FLAGS, priv->dpmac_handle);
	if (err < 0)
		printf("dpmac_close() failed\n");

	err = dpmac_destroy(dflt_mc_io,
			    dflt_dprc_handle,
			    MC_CMD_NO_FLAGS,
			    priv->dpmac_id);
	if (err < 0)
		printf("dpmac_destroy() failed\n");

	/* Stop Tx and Rx traffic */
	err = dpni_disable(dflt_mc_io, MC_CMD_NO_FLAGS, dflt_dpni->dpni_handle);
	if (err < 0)
		printf("dpni_disable() failed\n");

	phydev = priv->phy;
	if (phydev)
		phy_shutdown(phydev);

	/* Free DPBP handle and reset. */
	ldpaa_dpbp_free();

	dpni_reset(dflt_mc_io, MC_CMD_NO_FLAGS, dflt_dpni->dpni_handle);
	if (err < 0)
		printf("dpni_reset() failed\n");

	dpni_close(dflt_mc_io, MC_CMD_NO_FLAGS, dflt_dpni->dpni_handle);
	if (err < 0)
		printf("dpni_close() failed\n");
}

static void ldpaa_dpbp_drain_cnt(int count)
{
	uint64_t buf_array[7];
	void *addr;
	int ret, i;

	BUG_ON(count > 7);

	do {
		ret = qbman_swp_acquire(dflt_dpio->sw_portal,
					dflt_dpbp->dpbp_attr.bpid,
					buf_array, count);
		if (ret < 0) {
			printf("qbman_swp_acquire() failed\n");
			return;
		}
		for (i = 0; i < ret; i++) {
			addr = (void *)buf_array[i];
			debug("Free: buffer addr =0x%p\n", addr);
			free(addr);
		}
	} while (ret);
}

static void ldpaa_dpbp_drain(void)
{
	int i;
	for (i = 0; i < LDPAA_ETH_NUM_BUFS; i += 7)
		ldpaa_dpbp_drain_cnt(7);
}

static int ldpaa_bp_add_7(uint16_t bpid)
{
	uint64_t buf_array[7];
	u8 *addr;
	int i;
	struct qbman_release_desc rd;

	for (i = 0; i < 7; i++) {
		addr = memalign(LDPAA_ETH_BUF_ALIGN, LDPAA_ETH_RX_BUFFER_SIZE);
		if (!addr) {
			printf("addr allocation failed\n");
			goto err_alloc;
		}
		memset(addr, 0x00, LDPAA_ETH_RX_BUFFER_SIZE);
		flush_dcache_range((u64)addr,
				   (u64)(addr + LDPAA_ETH_RX_BUFFER_SIZE));

		buf_array[i] = (uint64_t)addr;
		debug("Release: buffer addr =0x%p\n", addr);
	}

release_bufs:
	/* In case the portal is busy, retry until successful.
	 * This function is guaranteed to succeed in a reasonable amount
	 * of time.
	 */

	do {
		mdelay(1);
		qbman_release_desc_clear(&rd);
		qbman_release_desc_set_bpid(&rd, bpid);
	} while (qbman_swp_release(dflt_dpio->sw_portal, &rd, buf_array, i));

	return i;

err_alloc:
	if (i)
		goto release_bufs;

	return 0;
}

static int ldpaa_dpbp_seed(uint16_t bpid)
{
	int i;
	int count;

	for (i = 0; i < LDPAA_ETH_NUM_BUFS; i += 7) {
		count = ldpaa_bp_add_7(bpid);
		if (count < 7)
			printf("Buffer Seed= %d\n", count);
	}

	return 0;
}

static int ldpaa_dpbp_setup(void)
{
	int err;

	err = dpbp_open(dflt_mc_io, MC_CMD_NO_FLAGS, dflt_dpbp->dpbp_attr.id,
			&dflt_dpbp->dpbp_handle);
	if (err) {
		printf("dpbp_open() failed\n");
		goto err_open;
	}

	err = dpbp_enable(dflt_mc_io, MC_CMD_NO_FLAGS, dflt_dpbp->dpbp_handle);
	if (err) {
		printf("dpbp_enable() failed\n");
		goto err_enable;
	}

	err = dpbp_get_attributes(dflt_mc_io, MC_CMD_NO_FLAGS,
				  dflt_dpbp->dpbp_handle,
				  &dflt_dpbp->dpbp_attr);
	if (err) {
		printf("dpbp_get_attributes() failed\n");
		goto err_get_attr;
	}

	err = ldpaa_dpbp_seed(dflt_dpbp->dpbp_attr.bpid);

	if (err) {
		printf("Buffer seeding failed for DPBP %d (bpid=%d)\n",
		       dflt_dpbp->dpbp_attr.id, dflt_dpbp->dpbp_attr.bpid);
		goto err_seed;
	}

	return 0;

err_seed:
err_get_attr:
	dpbp_disable(dflt_mc_io, MC_CMD_NO_FLAGS, dflt_dpbp->dpbp_handle);
err_enable:
	dpbp_close(dflt_mc_io, MC_CMD_NO_FLAGS, dflt_dpbp->dpbp_handle);
err_open:
	return err;
}

static void ldpaa_dpbp_free(void)
{
	ldpaa_dpbp_drain();
	dpbp_disable(dflt_mc_io, MC_CMD_NO_FLAGS, dflt_dpbp->dpbp_handle);
	dpbp_reset(dflt_mc_io, MC_CMD_NO_FLAGS, dflt_dpbp->dpbp_handle);
	dpbp_close(dflt_mc_io, MC_CMD_NO_FLAGS, dflt_dpbp->dpbp_handle);
}

static int ldpaa_dpmac_version_check(struct fsl_mc_io *mc_io,
				     struct ldpaa_eth_priv *priv)
{
	int error;
	uint16_t major_ver, minor_ver;

	error = dpmac_get_api_version(dflt_mc_io, 0,
					&major_ver,
					&minor_ver);
	if ((major_ver < DPMAC_VER_MAJOR) ||
	    (major_ver == DPMAC_VER_MAJOR && minor_ver < DPMAC_VER_MINOR)) {
		printf("DPMAC version mismatch found %u.%u,",
		       major_ver, minor_ver);
		printf("supported version is %u.%u\n",
		       DPMAC_VER_MAJOR, DPMAC_VER_MINOR);
		return error;
	}

	return error;
}

static int ldpaa_dpmac_setup(struct ldpaa_eth_priv *priv)
{
	int err = 0;
	struct dpmac_cfg dpmac_cfg;

	dpmac_cfg.mac_id = priv->dpmac_id;

	err = dpmac_create(dflt_mc_io,
			   dflt_dprc_handle,
			   MC_CMD_NO_FLAGS, &dpmac_cfg,
			   &priv->dpmac_id);
	if (err)
		printf("dpmac_create() failed\n");

	err = ldpaa_dpmac_version_check(dflt_mc_io, priv);
	if (err < 0) {
		printf("ldpaa_dpmac_version_check() failed: %d\n", err);
		goto err_version_check;
	}

	err = dpmac_open(dflt_mc_io,
			 MC_CMD_NO_FLAGS,
			 priv->dpmac_id,
			 &priv->dpmac_handle);
	if (err < 0) {
		printf("dpmac_open() failed: %d\n", err);
		goto err_open;
	}

	return err;

err_open:
err_version_check:
	dpmac_destroy(dflt_mc_io,
		      dflt_dprc_handle,
		      MC_CMD_NO_FLAGS, priv->dpmac_id);

	return err;
}

static int ldpaa_dpmac_bind(struct ldpaa_eth_priv *priv)
{
	int err = 0;
	struct dprc_connection_cfg dprc_connection_cfg = {
		/* If both rates are zero the connection */
		/* will be configured in "best effort" mode. */
		.committed_rate = 0,
		.max_rate = 0
	};

#ifdef DEBUG
	struct dprc_endpoint dbg_endpoint;
	int state = 0;
#endif

	memset(&dpmac_endpoint, 0, sizeof(struct dprc_endpoint));
	strcpy(dpmac_endpoint.type, "dpmac");
	dpmac_endpoint.id = priv->dpmac_id;

	memset(&dpni_endpoint, 0, sizeof(struct dprc_endpoint));
	strcpy(dpni_endpoint.type, "dpni");
	dpni_endpoint.id = dflt_dpni->dpni_id;

	err = dprc_connect(dflt_mc_io, MC_CMD_NO_FLAGS,
			     dflt_dprc_handle,
			     &dpmac_endpoint,
			     &dpni_endpoint,
			     &dprc_connection_cfg);
	if (err)
		printf("dprc_connect() failed\n");

#ifdef DEBUG
	err = dprc_get_connection(dflt_mc_io, MC_CMD_NO_FLAGS,
				    dflt_dprc_handle, &dpni_endpoint,
				    &dbg_endpoint, &state);
	printf("%s, DPMAC Type= %s\n", __func__, dbg_endpoint.type);
	printf("%s, DPMAC ID= %d\n", __func__, dbg_endpoint.id);
	printf("%s, DPMAC State= %d\n", __func__, state);

	memset(&dbg_endpoint, 0, sizeof(struct dprc_endpoint));
	err = dprc_get_connection(dflt_mc_io, MC_CMD_NO_FLAGS,
				    dflt_dprc_handle, &dpmac_endpoint,
				    &dbg_endpoint, &state);
	printf("%s, DPNI Type= %s\n", __func__, dbg_endpoint.type);
	printf("%s, DPNI ID= %d\n", __func__, dbg_endpoint.id);
	printf("%s, DPNI State= %d\n", __func__, state);
#endif
	return err;
}

static int ldpaa_dpni_setup(struct ldpaa_eth_priv *priv)
{
	int err;

	/* and get a handle for the DPNI this interface is associate with */
	err = dpni_open(dflt_mc_io, MC_CMD_NO_FLAGS, dflt_dpni->dpni_id,
			&dflt_dpni->dpni_handle);
	if (err) {
		printf("dpni_open() failed\n");
		goto err_open;
	}
	err = dpni_get_attributes(dflt_mc_io, MC_CMD_NO_FLAGS,
				  dflt_dpni->dpni_handle,
				  &dflt_dpni->dpni_attrs);
	if (err) {
		printf("dpni_get_attributes() failed (err=%d)\n", err);
		goto err_get_attr;
	}

	/* Configure our buffers' layout */
	dflt_dpni->buf_layout.options = DPNI_BUF_LAYOUT_OPT_PARSER_RESULT |
				   DPNI_BUF_LAYOUT_OPT_FRAME_STATUS |
				   DPNI_BUF_LAYOUT_OPT_PRIVATE_DATA_SIZE |
				   DPNI_BUF_LAYOUT_OPT_DATA_ALIGN;
	dflt_dpni->buf_layout.pass_parser_result = true;
	dflt_dpni->buf_layout.pass_frame_status = true;
	dflt_dpni->buf_layout.private_data_size = LDPAA_ETH_SWA_SIZE;
	/* HW erratum mandates data alignment in multiples of 256 */
	dflt_dpni->buf_layout.data_align = LDPAA_ETH_BUF_ALIGN;

	/* ...rx, ... */
	err = dpni_set_buffer_layout(dflt_mc_io, MC_CMD_NO_FLAGS,
				     dflt_dpni->dpni_handle,
				     DPNI_QUEUE_RX, &dflt_dpni->buf_layout);
	if (err) {
		printf("dpni_set_buffer_layout() failed");
		goto err_buf_layout;
	}

	/* ... tx, ... */
	/* remove Rx-only options */
	dflt_dpni->buf_layout.options &= ~(DPNI_BUF_LAYOUT_OPT_DATA_ALIGN |
				      DPNI_BUF_LAYOUT_OPT_PARSER_RESULT);
	err = dpni_set_buffer_layout(dflt_mc_io, MC_CMD_NO_FLAGS,
				     dflt_dpni->dpni_handle,
				     DPNI_QUEUE_TX, &dflt_dpni->buf_layout);
	if (err) {
		printf("dpni_set_buffer_layout() failed");
		goto err_buf_layout;
	}

	/* ... tx-confirm. */
	dflt_dpni->buf_layout.options &= ~DPNI_BUF_LAYOUT_OPT_PRIVATE_DATA_SIZE;
	err = dpni_set_buffer_layout(dflt_mc_io, MC_CMD_NO_FLAGS,
				     dflt_dpni->dpni_handle,
				     DPNI_QUEUE_TX_CONFIRM, &dflt_dpni->buf_layout);
	if (err) {
		printf("dpni_set_buffer_layout() failed");
		goto err_buf_layout;
	}

	/* Now that we've set our tx buffer layout, retrieve the minimum
	 * required tx data offset.
	 */
	err = dpni_get_tx_data_offset(dflt_mc_io, MC_CMD_NO_FLAGS,
				      dflt_dpni->dpni_handle,
				      &priv->tx_data_offset);
	if (err) {
		printf("dpni_get_tx_data_offset() failed\n");
		goto err_data_offset;
	}

	/* Warn in case TX data offset is not multiple of 64 bytes. */
	WARN_ON(priv->tx_data_offset % 64);

	/* Accomodate SWA space. */
	priv->tx_data_offset += LDPAA_ETH_SWA_SIZE;
	debug("priv->tx_data_offset=%d\n", priv->tx_data_offset);

	return 0;

err_data_offset:
err_buf_layout:
err_get_attr:
	dpni_close(dflt_mc_io, MC_CMD_NO_FLAGS, dflt_dpni->dpni_handle);
err_open:
	return err;
}

static int ldpaa_dpni_bind(struct ldpaa_eth_priv *priv)
{
	struct dpni_pools_cfg pools_params;
	struct dpni_queue tx_queue;
	int err = 0;

	memset(&pools_params, 0, sizeof(pools_params));
	pools_params.num_dpbp = 1;
	pools_params.pools[0].dpbp_id = (uint16_t)dflt_dpbp->dpbp_attr.id;
	pools_params.pools[0].buffer_size = LDPAA_ETH_RX_BUFFER_SIZE;
	err = dpni_set_pools(dflt_mc_io, MC_CMD_NO_FLAGS,
			     dflt_dpni->dpni_handle, &pools_params);
	if (err) {
		printf("dpni_set_pools() failed\n");
		return err;
	}

	memset(&tx_queue, 0, sizeof(struct dpni_queue));

	err = dpni_set_queue(dflt_mc_io, MC_CMD_NO_FLAGS,
			     dflt_dpni->dpni_handle,
			     DPNI_QUEUE_TX, 0, 0, 0, &tx_queue);

	if (err) {
		printf("dpni_set_queue() failed\n");
		return err;
	}

	err = dpni_set_tx_confirmation_mode(dflt_mc_io, MC_CMD_NO_FLAGS,
					    dflt_dpni->dpni_handle,
					    0, DPNI_CONF_DISABLE);
	if (err) {
		printf("dpni_set_tx_confirmation_mode() failed\n");
		return err;
	}

	return 0;
}

static int ldpaa_eth_probe(struct udevice *dev)
{
	struct ofnode_phandle_args phandle;

	/* Nothing to do if there is no "phy-handle" in the DTS node */
	if (dev_read_phandle_with_args(dev, "phy-handle", NULL,
				       0, 0, &phandle)) {
		return 0;
	}

	init_phy(dev);

	return 0;
}

uint32_t ldpaa_eth_get_dpmac_id(struct udevice *dev)
{
	int port_node = dev_of_offset(dev);

	return fdtdec_get_uint(gd->fdt_blob, port_node, "reg", -1);
}

static int ldpaa_eth_bind(struct udevice *dev)
{
	uint32_t dpmac_id;
	char eth_name[16];
	int phy_mode = -1;

	phy_mode = dev_read_phy_mode(dev);
	if (phy_mode == PHY_INTERFACE_MODE_NA) {
		dev_err(dev, "incorrect phy mode\n");
		return -EINVAL;
	}

	dpmac_id = ldpaa_eth_get_dpmac_id(dev);
	if (dpmac_id == -1) {
		dev_err(dev, "missing reg field from the dpmac node\n");
		return -EINVAL;
	}

	sprintf(eth_name, "DPMAC%d@%s", dpmac_id,
		phy_string_for_interface(phy_mode));
	device_set_name(dev, eth_name);

	return 0;
}

static int ldpaa_eth_of_to_plat(struct udevice *dev)
{
	struct ldpaa_eth_priv *priv = dev_get_priv(dev);

	priv->dpmac_id = ldpaa_eth_get_dpmac_id(dev);
	priv->phy_mode = dev_read_phy_mode(dev);

	return 0;
}

static int ldpaa_eth_get_sset_count(struct udevice *dev)
{
	return LDPAA_ETH_DPNI_NUM_STATS + LDPAA_ETH_DPMAC_NUM_STATS;
}

static void ldpaa_eth_get_strings(struct udevice *dev, u8 *data)
{
	u8 *p = data;
	int i;

	for (i = 0; i < LDPAA_ETH_DPNI_NUM_STATS; i++) {
		strlcpy(p, ldpaa_eth_dpni_stat_strings[i], ETH_GSTRING_LEN);
		p += ETH_GSTRING_LEN;
	}

	for (i = 0; i < LDPAA_ETH_DPMAC_NUM_STATS; i++) {
		strlcpy(p, ldpaa_eth_dpmac_stat_strings[i], ETH_GSTRING_LEN);
		p += ETH_GSTRING_LEN;
	}
}

static void ldpaa_eth_get_stats(struct udevice *dev, u64 *data)
{
	struct ldpaa_eth_priv *priv = dev_get_priv(dev);
	int i, j = 0;

	for (i = 0; i < LDPAA_ETH_DPNI_NUM_STATS; i++)
		*(data + j++) = priv->dpni_stats[i];

	for (i = 0; i < LDPAA_ETH_DPMAC_NUM_STATS; i++)
		*(data + j++) = priv->dpmac_stats[i];
}

static const struct eth_ops ldpaa_eth_ops = {
	.start = ldpaa_eth_open,
	.send = ldpaa_eth_tx,
	.recv = ldpaa_eth_pull_dequeue_rx,
	.stop = ldpaa_eth_stop,
	.get_sset_count = ldpaa_eth_get_sset_count,
	.get_strings = ldpaa_eth_get_strings,
	.get_stats = ldpaa_eth_get_stats,
};

static const struct udevice_id ldpaa_eth_of_ids[] = {
	{ .compatible = "fsl,qoriq-mc-dpmac" },
};

U_BOOT_DRIVER(ldpaa_eth) = {
	.name = LDPAA_ETH_DRIVER_NAME,
	.id = UCLASS_ETH,
	.of_match = ldpaa_eth_of_ids,
	.of_to_plat = ldpaa_eth_of_to_plat,
	.bind = ldpaa_eth_bind,
	.probe = ldpaa_eth_probe,
	.ops = &ldpaa_eth_ops,
	.priv_auto	= sizeof(struct ldpaa_eth_priv),
	.plat_auto	= sizeof(struct eth_pdata),
};
