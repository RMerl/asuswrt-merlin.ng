/* Framework for configuring and reading PHY devices
 * Based on code in sungem_phy.c and gianfar_phy.c
 *
 * Author: Andy Fleming
 *
 * Copyright (c) 2004 Freescale Semiconductor, Inc.
 * Copyright (c) 2006, 2007  Maciej W. Rozycki
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 *
 */

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/errno.h>
#include <linux/unistd.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/mii.h>
#include <linux/ethtool.h>
#include <linux/phy.h>
#include <linux/timer.h>
#include <linux/workqueue.h>
#include <linux/mdio.h>
#include <linux/io.h>
#include <linux/uaccess.h>
#include <linux/atomic.h>

#include <asm/irq.h>

static const char *phy_speed_to_str(int speed)
{
	switch (speed) {
	case SPEED_10:
		return "10Mbps";
	case SPEED_100:
		return "100Mbps";
	case SPEED_1000:
		return "1Gbps";
	case SPEED_2500:
		return "2.5Gbps";
	case SPEED_10000:
		return "10Gbps";
	case SPEED_UNKNOWN:
		return "Unknown";
	default:
		return "Unsupported (update phy.c)";
	}
}

/**
 * phy_print_status - Convenience function to print out the current phy status
 * @phydev: the phy_device struct
 */
void phy_print_status(struct phy_device *phydev)
{
	if (phydev->link) {
		netdev_info(phydev->attached_dev,
			"Link is Up - %s/%s - flow control %s\n",
			phy_speed_to_str(phydev->speed),
			DUPLEX_FULL == phydev->duplex ? "Full" : "Half",
			phydev->pause ? "rx/tx" : "off");
	} else	{
		netdev_info(phydev->attached_dev, "Link is Down\n");
	}
}
EXPORT_SYMBOL(phy_print_status);

/**
 * phy_clear_interrupt - Ack the phy device's interrupt
 * @phydev: the phy_device struct
 *
 * If the @phydev driver has an ack_interrupt function, call it to
 * ack and clear the phy device's interrupt.
 *
 * Returns 0 on success or < 0 on error.
 */
static int phy_clear_interrupt(struct phy_device *phydev)
{
	if (phydev->drv->ack_interrupt)
		return phydev->drv->ack_interrupt(phydev);

	return 0;
}

/**
 * phy_config_interrupt - configure the PHY device for the requested interrupts
 * @phydev: the phy_device struct
 * @interrupts: interrupt flags to configure for this @phydev
 *
 * Returns 0 on success or < 0 on error.
 */
static int phy_config_interrupt(struct phy_device *phydev, u32 interrupts)
{
	phydev->interrupts = interrupts;
	if (phydev->drv->config_intr)
		return phydev->drv->config_intr(phydev);

	return 0;
}


/**
 * phy_aneg_done - return auto-negotiation status
 * @phydev: target phy_device struct
 *
 * Description: Return the auto-negotiation status from this @phydev
 * Returns > 0 on success or < 0 on error. 0 means that auto-negotiation
 * is still pending.
 */
static inline int phy_aneg_done(struct phy_device *phydev)
{
	if (phydev->drv->aneg_done)
		return phydev->drv->aneg_done(phydev);

	/* Avoid genphy_aneg_done() if the Clause 45 PHY does not
	 * implement Clause 22 registers
	 */
	if (phydev->is_c45 && !(phydev->c45_ids.devices_in_package & BIT(0)))
		return -EINVAL;

	return genphy_aneg_done(phydev);
}

/* A structure for mapping a particular speed and duplex
 * combination to a particular SUPPORTED and ADVERTISED value
 */
struct phy_setting {
	int speed;
	int duplex;
	u32 setting;
};

/* A mapping of all SUPPORTED settings to speed/duplex */
static const struct phy_setting settings[] = {
	{
		.speed = SPEED_10000,
		.duplex = DUPLEX_FULL,
		.setting = SUPPORTED_10000baseKR_Full,
	},
	{
		.speed = SPEED_10000,
		.duplex = DUPLEX_FULL,
		.setting = SUPPORTED_10000baseKX4_Full,
	},
	{
		.speed = SPEED_10000,
		.duplex = DUPLEX_FULL,
		.setting = SUPPORTED_10000baseT_Full,
	},
	{
		.speed = SPEED_2500,
		.duplex = DUPLEX_FULL,
		.setting = SUPPORTED_2500baseX_Full,
	},
	{
		.speed = SPEED_1000,
		.duplex = DUPLEX_FULL,
		.setting = SUPPORTED_1000baseKX_Full,
	},
	{
		.speed = SPEED_1000,
		.duplex = DUPLEX_FULL,
		.setting = SUPPORTED_1000baseT_Full,
	},
	{
		.speed = SPEED_1000,
		.duplex = DUPLEX_HALF,
		.setting = SUPPORTED_1000baseT_Half,
	},
	{
		.speed = SPEED_100,
		.duplex = DUPLEX_FULL,
		.setting = SUPPORTED_100baseT_Full,
	},
	{
		.speed = SPEED_100,
		.duplex = DUPLEX_HALF,
		.setting = SUPPORTED_100baseT_Half,
	},
	{
		.speed = SPEED_10,
		.duplex = DUPLEX_FULL,
		.setting = SUPPORTED_10baseT_Full,
	},
	{
		.speed = SPEED_10,
		.duplex = DUPLEX_HALF,
		.setting = SUPPORTED_10baseT_Half,
	},
};

#define MAX_NUM_SETTINGS ARRAY_SIZE(settings)

/**
 * phy_find_setting - find a PHY settings array entry that matches speed & duplex
 * @speed: speed to match
 * @duplex: duplex to match
 *
 * Description: Searches the settings array for the setting which
 *   matches the desired speed and duplex, and returns the index
 *   of that setting.  Returns the index of the last setting if
 *   none of the others match.
 */
static inline unsigned int phy_find_setting(int speed, int duplex)
{
	unsigned int idx = 0;

	while (idx < ARRAY_SIZE(settings) &&
	       (settings[idx].speed != speed || settings[idx].duplex != duplex))
		idx++;

	return idx < MAX_NUM_SETTINGS ? idx : MAX_NUM_SETTINGS - 1;
}

/**
 * phy_find_valid - find a PHY setting that matches the requested features mask
 * @idx: The first index in settings[] to search
 * @features: A mask of the valid settings
 *
 * Description: Returns the index of the first valid setting less
 *   than or equal to the one pointed to by idx, as determined by
 *   the mask in features.  Returns the index of the last setting
 *   if nothing else matches.
 */
static inline unsigned int phy_find_valid(unsigned int idx, u32 features)
{
	while (idx < MAX_NUM_SETTINGS && !(settings[idx].setting & features))
		idx++;

	return idx < MAX_NUM_SETTINGS ? idx : MAX_NUM_SETTINGS - 1;
}

/**
 * phy_check_valid - check if there is a valid PHY setting which matches
 *		     speed, duplex, and feature mask
 * @speed: speed to match
 * @duplex: duplex to match
 * @features: A mask of the valid settings
 *
 * Description: Returns true if there is a valid setting, false otherwise.
 */
static inline bool phy_check_valid(int speed, int duplex, u32 features)
{
	unsigned int idx;

	idx = phy_find_valid(phy_find_setting(speed, duplex), features);

	return settings[idx].speed == speed && settings[idx].duplex == duplex &&
		(settings[idx].setting & features);
}

/**
 * phy_sanitize_settings - make sure the PHY is set to supported speed and duplex
 * @phydev: the target phy_device struct
 *
 * Description: Make sure the PHY is set to supported speeds and
 *   duplexes.  Drop down by one in this order:  1000/FULL,
 *   1000/HALF, 100/FULL, 100/HALF, 10/FULL, 10/HALF.
 */
static void phy_sanitize_settings(struct phy_device *phydev)
{
	u32 features = phydev->supported;
	unsigned int idx;

	/* Sanitize settings based on PHY capabilities */
	if ((features & SUPPORTED_Autoneg) == 0)
		phydev->autoneg = AUTONEG_DISABLE;

	idx = phy_find_valid(phy_find_setting(phydev->speed, phydev->duplex),
			features);

	phydev->speed = settings[idx].speed;
	phydev->duplex = settings[idx].duplex;
}

/**
 * phy_ethtool_sset - generic ethtool sset function, handles all the details
 * @phydev: target phy_device struct
 * @cmd: ethtool_cmd
 *
 * A few notes about parameter checking:
 * - We don't set port or transceiver, so we don't care what they
 *   were set to.
 * - phy_start_aneg() will make sure forced settings are sane, and
 *   choose the next best ones from the ones selected, so we don't
 *   care if ethtool tries to give us bad values.
 */
int phy_ethtool_sset(struct phy_device *phydev, struct ethtool_cmd *cmd)
{
	u32 speed = ethtool_cmd_speed(cmd);

	if (cmd->phy_address != phydev->addr)
		return -EINVAL;

	/* We make sure that we don't pass unsupported values in to the PHY */
	cmd->advertising &= phydev->supported;

	/* Verify the settings we care about. */
	if (cmd->autoneg != AUTONEG_ENABLE && cmd->autoneg != AUTONEG_DISABLE)
		return -EINVAL;

	if (cmd->autoneg == AUTONEG_ENABLE && cmd->advertising == 0)
		return -EINVAL;

	if (cmd->autoneg == AUTONEG_DISABLE &&
	    ((speed != SPEED_1000 &&
	      speed != SPEED_100 &&
	      speed != SPEED_10) ||
	     (cmd->duplex != DUPLEX_HALF &&
	      cmd->duplex != DUPLEX_FULL)))
		return -EINVAL;

	phydev->autoneg = cmd->autoneg;

	phydev->speed = speed;

	phydev->advertising = cmd->advertising;

	if (AUTONEG_ENABLE == cmd->autoneg)
		phydev->advertising |= ADVERTISED_Autoneg;
	else
		phydev->advertising &= ~ADVERTISED_Autoneg;

	phydev->duplex = cmd->duplex;

	/* Restart the PHY */
	phy_start_aneg(phydev);

	return 0;
}
EXPORT_SYMBOL(phy_ethtool_sset);

int phy_ethtool_gset(struct phy_device *phydev, struct ethtool_cmd *cmd)
{
	cmd->supported = phydev->supported;

	cmd->advertising = phydev->advertising;
	cmd->lp_advertising = phydev->lp_advertising;

	ethtool_cmd_speed_set(cmd, phydev->speed);
	cmd->duplex = phydev->duplex;
	if (phydev->interface == PHY_INTERFACE_MODE_MOCA)
		cmd->port = PORT_BNC;
	else
		cmd->port = PORT_MII;
	cmd->phy_address = phydev->addr;
	cmd->transceiver = phy_is_internal(phydev) ?
		XCVR_INTERNAL : XCVR_EXTERNAL;
	cmd->autoneg = phydev->autoneg;

	return 0;
}
EXPORT_SYMBOL(phy_ethtool_gset);

/**
 * phy_mii_ioctl - generic PHY MII ioctl interface
 * @phydev: the phy_device struct
 * @ifr: &struct ifreq for socket ioctl's
 * @cmd: ioctl cmd to execute
 *
 * Note that this function is currently incompatible with the
 * PHYCONTROL layer.  It changes registers without regard to
 * current state.  Use at own risk.
 */
int phy_mii_ioctl(struct phy_device *phydev, struct ifreq *ifr, int cmd)
{
	struct mii_ioctl_data *mii_data = if_mii(ifr);
	u16 val = mii_data->val_in;
	bool change_autoneg = false;

	switch (cmd) {
	case SIOCGMIIPHY:
		mii_data->phy_id = phydev->addr;
		/* fall through */

	case SIOCGMIIREG:
		mii_data->val_out = mdiobus_read(phydev->bus, mii_data->phy_id,
						 mii_data->reg_num);
		return 0;

	case SIOCSMIIREG:
		if (mii_data->phy_id == phydev->addr) {
			switch (mii_data->reg_num) {
			case MII_BMCR:
				if ((val & (BMCR_RESET | BMCR_ANENABLE)) == 0) {
					if (phydev->autoneg == AUTONEG_ENABLE)
						change_autoneg = true;
					phydev->autoneg = AUTONEG_DISABLE;
					if (val & BMCR_FULLDPLX)
						phydev->duplex = DUPLEX_FULL;
					else
						phydev->duplex = DUPLEX_HALF;
					if (val & BMCR_SPEED1000)
						phydev->speed = SPEED_1000;
					else if (val & BMCR_SPEED100)
						phydev->speed = SPEED_100;
					else phydev->speed = SPEED_10;
				}
				else {
					if (phydev->autoneg == AUTONEG_DISABLE)
						change_autoneg = true;
					phydev->autoneg = AUTONEG_ENABLE;
				}
				break;
			case MII_ADVERTISE:
				phydev->advertising = mii_adv_to_ethtool_adv_t(val);
				change_autoneg = true;
				break;
			default:
				/* do nothing */
				break;
			}
		}

		mdiobus_write(phydev->bus, mii_data->phy_id,
			      mii_data->reg_num, val);

		if (mii_data->reg_num == MII_BMCR &&
		    val & BMCR_RESET)
			return phy_init_hw(phydev);

		if (change_autoneg)
			return phy_start_aneg(phydev);

		return 0;

	case SIOCSHWTSTAMP:
		if (phydev->drv->hwtstamp)
			return phydev->drv->hwtstamp(phydev, ifr);
		/* fall through */

	default:
		return -EOPNOTSUPP;
	}
}
EXPORT_SYMBOL(phy_mii_ioctl);

/**
 * phy_start_aneg - start auto-negotiation for this PHY device
 * @phydev: the phy_device struct
 *
 * Description: Sanitizes the settings (if we're not autonegotiating
 *   them), and then calls the driver's config_aneg function.
 *   If the PHYCONTROL Layer is operating, we change the state to
 *   reflect the beginning of Auto-negotiation or forcing.
 */
int phy_start_aneg(struct phy_device *phydev)
{
	int err;

	mutex_lock(&phydev->lock);

	if (AUTONEG_DISABLE == phydev->autoneg)
		phy_sanitize_settings(phydev);

	/* Invalidate LP advertising flags */
	phydev->lp_advertising = 0;

	err = phydev->drv->config_aneg(phydev);
	if (err < 0)
		goto out_unlock;

	if (phydev->state != PHY_HALTED) {
		if (AUTONEG_ENABLE == phydev->autoneg) {
			phydev->state = PHY_AN;
			phydev->link_timeout = PHY_AN_TIMEOUT;
		} else {
			phydev->state = PHY_FORCING;
			phydev->link_timeout = PHY_FORCE_TIMEOUT;
		}
	}

out_unlock:
	mutex_unlock(&phydev->lock);
	return err;
}
EXPORT_SYMBOL(phy_start_aneg);

/**
 * phy_start_machine - start PHY state machine tracking
 * @phydev: the phy_device struct
 *
 * Description: The PHY infrastructure can run a state machine
 *   which tracks whether the PHY is starting up, negotiating,
 *   etc.  This function starts the timer which tracks the state
 *   of the PHY.  If you want to maintain your own state machine,
 *   do not call this function.
 */
void phy_start_machine(struct phy_device *phydev)
{
	queue_delayed_work(system_power_efficient_wq, &phydev->state_queue, HZ);
}

/**
 * phy_stop_machine - stop the PHY state machine tracking
 * @phydev: target phy_device struct
 *
 * Description: Stops the state machine timer, sets the state to UP
 *   (unless it wasn't up yet). This function must be called BEFORE
 *   phy_detach.
 */
void phy_stop_machine(struct phy_device *phydev)
{
	cancel_delayed_work_sync(&phydev->state_queue);

	mutex_lock(&phydev->lock);
	if (phydev->state > PHY_UP && phydev->state != PHY_HALTED)
		phydev->state = PHY_UP;
	mutex_unlock(&phydev->lock);

	/* Now we can run the state machine synchronously */
	phy_state_machine(&phydev->state_queue.work);
}

/**
 * phy_error - enter HALTED state for this PHY device
 * @phydev: target phy_device struct
 *
 * Moves the PHY to the HALTED state in response to a read
 * or write error, and tells the controller the link is down.
 * Must not be called from interrupt context, or while the
 * phydev->lock is held.
 */
static void phy_error(struct phy_device *phydev)
{
	mutex_lock(&phydev->lock);
	phydev->state = PHY_HALTED;
	mutex_unlock(&phydev->lock);
}

/**
 * phy_interrupt - PHY interrupt handler
 * @irq: interrupt line
 * @phy_dat: phy_device pointer
 *
 * Description: When a PHY interrupt occurs, the handler disables
 * interrupts, and schedules a work task to clear the interrupt.
 */
static irqreturn_t phy_interrupt(int irq, void *phy_dat)
{
	struct phy_device *phydev = phy_dat;

	if (PHY_HALTED == phydev->state)
		return IRQ_NONE;		/* It can't be ours.  */

	/* The MDIO bus is not allowed to be written in interrupt
	 * context, so we need to disable the irq here.  A work
	 * queue will write the PHY to disable and clear the
	 * interrupt, and then reenable the irq line.
	 */
	disable_irq_nosync(irq);
	atomic_inc(&phydev->irq_disable);

	queue_work(system_power_efficient_wq, &phydev->phy_queue);

	return IRQ_HANDLED;
}

/**
 * phy_enable_interrupts - Enable the interrupts from the PHY side
 * @phydev: target phy_device struct
 */
static int phy_enable_interrupts(struct phy_device *phydev)
{
	int err = phy_clear_interrupt(phydev);

	if (err < 0)
		return err;

	return phy_config_interrupt(phydev, PHY_INTERRUPT_ENABLED);
}

/**
 * phy_disable_interrupts - Disable the PHY interrupts from the PHY side
 * @phydev: target phy_device struct
 */
static int phy_disable_interrupts(struct phy_device *phydev)
{
	int err;

	/* Disable PHY interrupts */
	err = phy_config_interrupt(phydev, PHY_INTERRUPT_DISABLED);
	if (err)
		goto phy_err;

	/* Clear the interrupt */
	err = phy_clear_interrupt(phydev);
	if (err)
		goto phy_err;

	return 0;

phy_err:
	phy_error(phydev);

	return err;
}

/**
 * phy_start_interrupts - request and enable interrupts for a PHY device
 * @phydev: target phy_device struct
 *
 * Description: Request the interrupt for the given PHY.
 *   If this fails, then we set irq to PHY_POLL.
 *   Otherwise, we enable the interrupts in the PHY.
 *   This should only be called with a valid IRQ number.
 *   Returns 0 on success or < 0 on error.
 */
int phy_start_interrupts(struct phy_device *phydev)
{
	atomic_set(&phydev->irq_disable, 0);
	if (request_irq(phydev->irq, phy_interrupt, 0, "phy_interrupt",
			phydev) < 0) {
		pr_warn("%s: Can't get IRQ %d (PHY)\n",
			phydev->bus->name, phydev->irq);
		phydev->irq = PHY_POLL;
		return 0;
	}

	return phy_enable_interrupts(phydev);
}
EXPORT_SYMBOL(phy_start_interrupts);

/**
 * phy_stop_interrupts - disable interrupts from a PHY device
 * @phydev: target phy_device struct
 */
int phy_stop_interrupts(struct phy_device *phydev)
{
	int err = phy_disable_interrupts(phydev);

	if (err)
		phy_error(phydev);

	free_irq(phydev->irq, phydev);

	/* Cannot call flush_scheduled_work() here as desired because
	 * of rtnl_lock(), but we do not really care about what would
	 * be done, except from enable_irq(), so cancel any work
	 * possibly pending and take care of the matter below.
	 */
	cancel_work_sync(&phydev->phy_queue);
	/* If work indeed has been cancelled, disable_irq() will have
	 * been left unbalanced from phy_interrupt() and enable_irq()
	 * has to be called so that other devices on the line work.
	 */
	while (atomic_dec_return(&phydev->irq_disable) >= 0)
		enable_irq(phydev->irq);

	return err;
}
EXPORT_SYMBOL(phy_stop_interrupts);

/**
 * phy_change - Scheduled by the phy_interrupt/timer to handle PHY changes
 * @work: work_struct that describes the work to be done
 */
void phy_change(struct work_struct *work)
{
	struct phy_device *phydev =
		container_of(work, struct phy_device, phy_queue);

	if (phydev->drv->did_interrupt &&
	    !phydev->drv->did_interrupt(phydev))
		goto ignore;

	if (phy_disable_interrupts(phydev))
		goto phy_err;

	mutex_lock(&phydev->lock);
	if ((PHY_RUNNING == phydev->state) || (PHY_NOLINK == phydev->state))
		phydev->state = PHY_CHANGELINK;
	mutex_unlock(&phydev->lock);

	atomic_dec(&phydev->irq_disable);
	enable_irq(phydev->irq);

	/* Reenable interrupts */
	if (PHY_HALTED != phydev->state &&
	    phy_config_interrupt(phydev, PHY_INTERRUPT_ENABLED))
		goto irq_enable_err;

	/* reschedule state queue work to run as soon as possible */
	cancel_delayed_work_sync(&phydev->state_queue);
	queue_delayed_work(system_power_efficient_wq, &phydev->state_queue, 0);
	return;

ignore:
	atomic_dec(&phydev->irq_disable);
	enable_irq(phydev->irq);
	return;

irq_enable_err:
	disable_irq(phydev->irq);
	atomic_inc(&phydev->irq_disable);
phy_err:
	phy_error(phydev);
}

/**
 * phy_stop - Bring down the PHY link, and stop checking the status
 * @phydev: target phy_device struct
 */
void phy_stop(struct phy_device *phydev)
{
	mutex_lock(&phydev->lock);

	if (PHY_HALTED == phydev->state)
		goto out_unlock;

	if (phy_interrupt_is_valid(phydev)) {
		/* Disable PHY Interrupts */
		phy_config_interrupt(phydev, PHY_INTERRUPT_DISABLED);

		/* Clear any pending interrupts */
		phy_clear_interrupt(phydev);
	}

	phydev->state = PHY_HALTED;

out_unlock:
	mutex_unlock(&phydev->lock);

	/* Cannot call flush_scheduled_work() here as desired because
	 * of rtnl_lock(), but PHY_HALTED shall guarantee phy_change()
	 * will not reenable interrupts.
	 */
}
EXPORT_SYMBOL(phy_stop);

/**
 * phy_start - start or restart a PHY device
 * @phydev: target phy_device struct
 *
 * Description: Indicates the attached device's readiness to
 *   handle PHY-related work.  Used during startup to start the
 *   PHY, and after a call to phy_stop() to resume operation.
 *   Also used to indicate the MDIO bus has cleared an error
 *   condition.
 */
void phy_start(struct phy_device *phydev)
{
	bool do_resume = false;
	int err = 0;

	mutex_lock(&phydev->lock);

	switch (phydev->state) {
	case PHY_STARTING:
		phydev->state = PHY_PENDING;
		break;
	case PHY_READY:
		phydev->state = PHY_UP;
		break;
	case PHY_HALTED:
		/* make sure interrupts are re-enabled for the PHY */
		err = phy_enable_interrupts(phydev);
		if (err < 0)
			break;

		phydev->state = PHY_RESUMING;
		do_resume = true;
		break;
	default:
		break;
	}
	mutex_unlock(&phydev->lock);

	/* if phy was suspended, bring the physical link up again */
	if (do_resume)
		phy_resume(phydev);
}
EXPORT_SYMBOL(phy_start);

/**
 * phy_state_machine - Handle the state machine
 * @work: work_struct that describes the work to be done
 */
void phy_state_machine(struct work_struct *work)
{
	struct delayed_work *dwork = to_delayed_work(work);
	struct phy_device *phydev =
			container_of(dwork, struct phy_device, state_queue);
	bool needs_aneg = false, do_suspend = false;
	int err = 0;

	mutex_lock(&phydev->lock);

	if (phydev->drv->link_change_notify)
		phydev->drv->link_change_notify(phydev);

	switch (phydev->state) {
	case PHY_DOWN:
	case PHY_STARTING:
	case PHY_READY:
	case PHY_PENDING:
		break;
	case PHY_UP:
		needs_aneg = true;

		phydev->link_timeout = PHY_AN_TIMEOUT;

		break;
	case PHY_AN:
		err = phy_read_status(phydev);
		if (err < 0)
			break;

		/* If the link is down, give up on negotiation for now */
		if (!phydev->link) {
			phydev->state = PHY_NOLINK;
			netif_carrier_off(phydev->attached_dev);
			phydev->adjust_link(phydev->attached_dev);
			break;
		}

		/* Check if negotiation is done.  Break if there's an error */
		err = phy_aneg_done(phydev);
		if (err < 0)
			break;

		/* If AN is done, we're running */
		if (err > 0) {
			phydev->state = PHY_RUNNING;
			netif_carrier_on(phydev->attached_dev);
			phydev->adjust_link(phydev->attached_dev);

		} else if (0 == phydev->link_timeout--)
			needs_aneg = true;
		break;
	case PHY_NOLINK:
		err = phy_read_status(phydev);
		if (err)
			break;

		if (phydev->link) {
			if (AUTONEG_ENABLE == phydev->autoneg) {
				err = phy_aneg_done(phydev);
				if (err < 0)
					break;

				if (!err) {
					phydev->state = PHY_AN;
					phydev->link_timeout = PHY_AN_TIMEOUT;
					break;
				}
			}
			phydev->state = PHY_RUNNING;
			netif_carrier_on(phydev->attached_dev);
			phydev->adjust_link(phydev->attached_dev);
		}
		break;
	case PHY_FORCING:
		err = genphy_update_link(phydev);
		if (err)
			break;

		if (phydev->link) {
			phydev->state = PHY_RUNNING;
			netif_carrier_on(phydev->attached_dev);
		} else {
			if (0 == phydev->link_timeout--)
				needs_aneg = true;
		}

		phydev->adjust_link(phydev->attached_dev);
		break;
	case PHY_RUNNING:
		/* Only register a CHANGE if we are
		 * polling or ignoring interrupts
		 */
		if (!phy_interrupt_is_valid(phydev))
			phydev->state = PHY_CHANGELINK;
		break;
	case PHY_CHANGELINK:
		err = phy_read_status(phydev);
		if (err)
			break;

		if (phydev->link) {
			phydev->state = PHY_RUNNING;
			netif_carrier_on(phydev->attached_dev);
		} else {
			phydev->state = PHY_NOLINK;
			netif_carrier_off(phydev->attached_dev);
		}

		phydev->adjust_link(phydev->attached_dev);

		if (phy_interrupt_is_valid(phydev))
			err = phy_config_interrupt(phydev,
						   PHY_INTERRUPT_ENABLED);
		break;
	case PHY_HALTED:
		if (phydev->link) {
			phydev->link = 0;
			netif_carrier_off(phydev->attached_dev);
			phydev->adjust_link(phydev->attached_dev);
			do_suspend = true;
		}
		break;
	case PHY_RESUMING:
		if (AUTONEG_ENABLE == phydev->autoneg) {
			err = phy_aneg_done(phydev);
			if (err < 0)
				break;

			/* err > 0 if AN is done.
			 * Otherwise, it's 0, and we're  still waiting for AN
			 */
			if (err > 0) {
				err = phy_read_status(phydev);
				if (err)
					break;

				if (phydev->link) {
					phydev->state = PHY_RUNNING;
					netif_carrier_on(phydev->attached_dev);
				} else	{
					phydev->state = PHY_NOLINK;
				}
				phydev->adjust_link(phydev->attached_dev);
			} else {
				phydev->state = PHY_AN;
				phydev->link_timeout = PHY_AN_TIMEOUT;
			}
		} else {
			err = phy_read_status(phydev);
			if (err)
				break;

			if (phydev->link) {
				phydev->state = PHY_RUNNING;
				netif_carrier_on(phydev->attached_dev);
			} else	{
				phydev->state = PHY_NOLINK;
			}
			phydev->adjust_link(phydev->attached_dev);
		}
		break;
	}

	mutex_unlock(&phydev->lock);

	if (needs_aneg)
		err = phy_start_aneg(phydev);
	else if (do_suspend)
		phy_suspend(phydev);

	if (err < 0)
		phy_error(phydev);

	queue_delayed_work(system_power_efficient_wq, &phydev->state_queue,
			   PHY_STATE_TIME * HZ);
}

void phy_mac_interrupt(struct phy_device *phydev, int new_link)
{
	cancel_work_sync(&phydev->phy_queue);
	phydev->link = new_link;
	schedule_work(&phydev->phy_queue);
}
EXPORT_SYMBOL(phy_mac_interrupt);

static inline void mmd_phy_indirect(struct mii_bus *bus, int prtad, int devad,
				    int addr)
{
	/* Write the desired MMD Devad */
	bus->write(bus, addr, MII_MMD_CTRL, devad);

	/* Write the desired MMD register address */
	bus->write(bus, addr, MII_MMD_DATA, prtad);

	/* Select the Function : DATA with no post increment */
	bus->write(bus, addr, MII_MMD_CTRL, (devad | MII_MMD_CTRL_NOINCR));
}

/**
 * phy_read_mmd_indirect - reads data from the MMD registers
 * @phydev: The PHY device bus
 * @prtad: MMD Address
 * @devad: MMD DEVAD
 * @addr: PHY address on the MII bus
 *
 * Description: it reads data from the MMD registers (clause 22 to access to
 * clause 45) of the specified phy address.
 * To read these register we have:
 * 1) Write reg 13 // DEVAD
 * 2) Write reg 14 // MMD Address
 * 3) Write reg 13 // MMD Data Command for MMD DEVAD
 * 3) Read  reg 14 // Read MMD data
 */
int phy_read_mmd_indirect(struct phy_device *phydev, int prtad,
				 int devad, int addr)
{
	struct phy_driver *phydrv = phydev->drv;
	int value = -1;

	if (phydrv->read_mmd_indirect == NULL) {
		mmd_phy_indirect(phydev->bus, prtad, devad, addr);

		/* Read the content of the MMD's selected register */
		value = phydev->bus->read(phydev->bus, addr, MII_MMD_DATA);
	} else {
		value = phydrv->read_mmd_indirect(phydev, prtad, devad, addr);
	}
	return value;
}
EXPORT_SYMBOL(phy_read_mmd_indirect);

/**
 * phy_write_mmd_indirect - writes data to the MMD registers
 * @phydev: The PHY device
 * @prtad: MMD Address
 * @devad: MMD DEVAD
 * @addr: PHY address on the MII bus
 * @data: data to write in the MMD register
 *
 * Description: Write data from the MMD registers of the specified
 * phy address.
 * To write these register we have:
 * 1) Write reg 13 // DEVAD
 * 2) Write reg 14 // MMD Address
 * 3) Write reg 13 // MMD Data Command for MMD DEVAD
 * 3) Write reg 14 // Write MMD data
 */
void phy_write_mmd_indirect(struct phy_device *phydev, int prtad,
				   int devad, int addr, u32 data)
{
	struct phy_driver *phydrv = phydev->drv;

	if (phydrv->write_mmd_indirect == NULL) {
		mmd_phy_indirect(phydev->bus, prtad, devad, addr);

		/* Write the data into MMD's selected register */
		phydev->bus->write(phydev->bus, addr, MII_MMD_DATA, data);
	} else {
		phydrv->write_mmd_indirect(phydev, prtad, devad, addr, data);
	}
}
EXPORT_SYMBOL(phy_write_mmd_indirect);

/**
 * phy_init_eee - init and check the EEE feature
 * @phydev: target phy_device struct
 * @clk_stop_enable: PHY may stop the clock during LPI
 *
 * Description: it checks if the Energy-Efficient Ethernet (EEE)
 * is supported by looking at the MMD registers 3.20 and 7.60/61
 * and it programs the MMD register 3.0 setting the "Clock stop enable"
 * bit if required.
 */
int phy_init_eee(struct phy_device *phydev, bool clk_stop_enable)
{
	/* According to 802.3az,the EEE is supported only in full duplex-mode.
	 * Also EEE feature is active when core is operating with MII, GMII
	 * or RGMII (all kinds). Internal PHYs are also allowed to proceed and
	 * should return an error if they do not support EEE.
	 */
	if ((phydev->duplex == DUPLEX_FULL) &&
	    ((phydev->interface == PHY_INTERFACE_MODE_MII) ||
	    (phydev->interface == PHY_INTERFACE_MODE_GMII) ||
	    (phydev->interface >= PHY_INTERFACE_MODE_RGMII &&
	     phydev->interface <= PHY_INTERFACE_MODE_RGMII_TXID) ||
	     phy_is_internal(phydev))) {
		int eee_lp, eee_cap, eee_adv;
		u32 lp, cap, adv;
		int status;

		/* Read phy status to properly get the right settings */
		status = phy_read_status(phydev);
		if (status)
			return status;

		/* First check if the EEE ability is supported */
		eee_cap = phy_read_mmd_indirect(phydev, MDIO_PCS_EEE_ABLE,
						MDIO_MMD_PCS, phydev->addr);
		if (eee_cap <= 0)
			goto eee_exit_err;

		cap = mmd_eee_cap_to_ethtool_sup_t(eee_cap);
		if (!cap)
			goto eee_exit_err;

		/* Check which link settings negotiated and verify it in
		 * the EEE advertising registers.
		 */
		eee_lp = phy_read_mmd_indirect(phydev, MDIO_AN_EEE_LPABLE,
					       MDIO_MMD_AN, phydev->addr);
		if (eee_lp <= 0)
			goto eee_exit_err;

		eee_adv = phy_read_mmd_indirect(phydev, MDIO_AN_EEE_ADV,
						MDIO_MMD_AN, phydev->addr);
		if (eee_adv <= 0)
			goto eee_exit_err;

		adv = mmd_eee_adv_to_ethtool_adv_t(eee_adv);
		lp = mmd_eee_adv_to_ethtool_adv_t(eee_lp);
		if (!phy_check_valid(phydev->speed, phydev->duplex, lp & adv))
			goto eee_exit_err;

		if (clk_stop_enable) {
			/* Configure the PHY to stop receiving xMII
			 * clock while it is signaling LPI.
			 */
			int val = phy_read_mmd_indirect(phydev, MDIO_CTRL1,
							MDIO_MMD_PCS,
							phydev->addr);
			if (val < 0)
				return val;

			val |= MDIO_PCS_CTRL1_CLKSTOP_EN;
			phy_write_mmd_indirect(phydev, MDIO_CTRL1,
					       MDIO_MMD_PCS, phydev->addr,
					       val);
		}

		return 0; /* EEE supported */
	}
eee_exit_err:
	return -EPROTONOSUPPORT;
}
EXPORT_SYMBOL(phy_init_eee);

/**
 * phy_get_eee_err - report the EEE wake error count
 * @phydev: target phy_device struct
 *
 * Description: it is to report the number of time where the PHY
 * failed to complete its normal wake sequence.
 */
int phy_get_eee_err(struct phy_device *phydev)
{
	return phy_read_mmd_indirect(phydev, MDIO_PCS_EEE_WK_ERR,
				     MDIO_MMD_PCS, phydev->addr);
}
EXPORT_SYMBOL(phy_get_eee_err);

/**
 * phy_ethtool_get_eee - get EEE supported and status
 * @phydev: target phy_device struct
 * @data: ethtool_eee data
 *
 * Description: it reportes the Supported/Advertisement/LP Advertisement
 * capabilities.
 */
int phy_ethtool_get_eee(struct phy_device *phydev, struct ethtool_eee *data)
{
	int val;

	/* Get Supported EEE */
	val = phy_read_mmd_indirect(phydev, MDIO_PCS_EEE_ABLE,
				    MDIO_MMD_PCS, phydev->addr);
	if (val < 0)
		return val;
	data->supported = mmd_eee_cap_to_ethtool_sup_t(val);

	/* Get advertisement EEE */
	val = phy_read_mmd_indirect(phydev, MDIO_AN_EEE_ADV,
				    MDIO_MMD_AN, phydev->addr);
	if (val < 0)
		return val;
	data->advertised = mmd_eee_adv_to_ethtool_adv_t(val);

	/* Get LP advertisement EEE */
	val = phy_read_mmd_indirect(phydev, MDIO_AN_EEE_LPABLE,
				    MDIO_MMD_AN, phydev->addr);
	if (val < 0)
		return val;
	data->lp_advertised = mmd_eee_adv_to_ethtool_adv_t(val);

	return 0;
}
EXPORT_SYMBOL(phy_ethtool_get_eee);

/**
 * phy_ethtool_set_eee - set EEE supported and status
 * @phydev: target phy_device struct
 * @data: ethtool_eee data
 *
 * Description: it is to program the Advertisement EEE register.
 */
int phy_ethtool_set_eee(struct phy_device *phydev, struct ethtool_eee *data)
{
	int val = ethtool_adv_to_mmd_eee_adv_t(data->advertised);

	phy_write_mmd_indirect(phydev, MDIO_AN_EEE_ADV, MDIO_MMD_AN,
			       phydev->addr, val);

	return 0;
}
EXPORT_SYMBOL(phy_ethtool_set_eee);

int phy_ethtool_set_wol(struct phy_device *phydev, struct ethtool_wolinfo *wol)
{
	if (phydev->drv->set_wol)
		return phydev->drv->set_wol(phydev, wol);

	return -EOPNOTSUPP;
}
EXPORT_SYMBOL(phy_ethtool_set_wol);

void phy_ethtool_get_wol(struct phy_device *phydev, struct ethtool_wolinfo *wol)
{
	if (phydev->drv->get_wol)
		phydev->drv->get_wol(phydev, wol);
}
EXPORT_SYMBOL(phy_ethtool_get_wol);
