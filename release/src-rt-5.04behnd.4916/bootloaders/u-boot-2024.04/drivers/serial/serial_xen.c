// SPDX-License-Identifier: GPL-2.0
/*
 * (C) 2018 NXP
 * (C) 2020 EPAM Systems Inc.
 */
#include <common.h>
#include <cpu_func.h>
#include <dm.h>
#include <serial.h>
#include <watchdog.h>
#include <asm/global_data.h>

#include <linux/bug.h>

#include <xen/hvm.h>
#include <xen/events.h>

#include <xen/interface/sched.h>
#include <xen/interface/hvm/hvm_op.h>
#include <xen/interface/hvm/params.h>
#include <xen/interface/io/console.h>
#include <xen/interface/io/ring.h>

DECLARE_GLOBAL_DATA_PTR;

u32 console_evtchn;

/*
 * struct xen_uart_priv - Structure representing a Xen UART info
 * @intf:    Console I/O interface for Xen guest OSes
 * @evtchn:  Console event channel
 */
struct xen_uart_priv {
	struct xencons_interface *intf;
	u32 evtchn;
};

int xen_serial_setbrg(struct udevice *dev, int baudrate)
{
	return 0;
}

static int xen_serial_probe(struct udevice *dev)
{
	struct xen_uart_priv *priv = dev_get_priv(dev);
	u64 val = 0;
	unsigned long gfn;
	int ret;

	ret = hvm_get_parameter(HVM_PARAM_CONSOLE_EVTCHN, &val);
	if (ret < 0 || val == 0)
		return ret;

	priv->evtchn = val;
	console_evtchn = val;

	ret = hvm_get_parameter(HVM_PARAM_CONSOLE_PFN, &val);
	if (ret < 0)
		return ret;

	if (!val)
		return -EINVAL;

	gfn = val;
	priv->intf = (struct xencons_interface *)(gfn << XEN_PAGE_SHIFT);

	return 0;
}

static int xen_serial_pending(struct udevice *dev, bool input)
{
	struct xen_uart_priv *priv = dev_get_priv(dev);
	struct xencons_interface *intf = priv->intf;

	if (!input || intf->in_cons == intf->in_prod)
		return 0;

	return 1;
}

static int xen_serial_getc(struct udevice *dev)
{
	struct xen_uart_priv *priv = dev_get_priv(dev);
	struct xencons_interface *intf = priv->intf;
	XENCONS_RING_IDX cons;
	char c;

	while (intf->in_cons == intf->in_prod)
		mb(); /* wait */

	cons = intf->in_cons;
	mb();			/* get pointers before reading ring */

	c = intf->in[MASK_XENCONS_IDX(cons++, intf->in)];

	mb();			/* read ring before consuming */
	intf->in_cons = cons;

	notify_remote_via_evtchn(priv->evtchn);

	return c;
}

static int __write_console(struct udevice *dev, const char *data, int len)
{
	struct xen_uart_priv *priv = dev_get_priv(dev);
	struct xencons_interface *intf = priv->intf;
	XENCONS_RING_IDX cons, prod;
	int sent = 0;

	cons = intf->out_cons;
	prod = intf->out_prod;
	mb(); /* Update pointer */

	WARN_ON((prod - cons) > sizeof(intf->out));

	while ((sent < len) && ((prod - cons) < sizeof(intf->out)))
		intf->out[MASK_XENCONS_IDX(prod++, intf->out)] = data[sent++];

	mb(); /* Update data before pointer */
	intf->out_prod = prod;

	if (sent)
		notify_remote_via_evtchn(priv->evtchn);

	return sent;
}

static int write_console(struct udevice *dev, const char *data, int len)
{
	/*
	 * Make sure the whole buffer is emitted, polling if
	 * necessary.  We don't ever want to rely on the hvc daemon
	 * because the most interesting console output is when the
	 * kernel is crippled.
	 */
	while (len) {
		int sent = __write_console(dev, data, len);

		data += sent;
		len -= sent;

		if (unlikely(len))
			HYPERVISOR_sched_op(SCHEDOP_yield, NULL);
	}

	return 0;
}

static int xen_serial_putc(struct udevice *dev, const char ch)
{
	write_console(dev, &ch, 1);

	return 0;
}

static const struct dm_serial_ops xen_serial_ops = {
	.putc = xen_serial_putc,
	.getc = xen_serial_getc,
	.pending = xen_serial_pending,
};

#if CONFIG_IS_ENABLED(OF_CONTROL)
static const struct udevice_id xen_serial_ids[] = {
	{ .compatible = "xen,xen" },
	{ }
};
#endif

U_BOOT_DRIVER(serial_xen) = {
	.name			= "serial_xen",
	.id			= UCLASS_SERIAL,
#if CONFIG_IS_ENABLED(OF_CONTROL)
	.of_match		= xen_serial_ids,
#endif
	.priv_auto	= sizeof(struct xen_uart_priv),
	.probe			= xen_serial_probe,
	.ops			= &xen_serial_ops,
#if !CONFIG_IS_ENABLED(OF_CONTROL)
	.flags			= DM_FLAG_PRE_RELOC,
#endif
};
