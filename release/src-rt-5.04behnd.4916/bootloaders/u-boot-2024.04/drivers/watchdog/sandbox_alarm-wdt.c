#include <common.h>
#include <dm.h>
#include <os.h>
#include <wdt.h>

struct alarm_wdt_priv {
	unsigned int timeout_sec;
};

static void alarm_handler(int sig)
{
	const char *msg = "!!! ALARM !!!\n";

	os_write(2, msg, strlen(msg));
	os_fd_restore();
	os_set_alarm_handler(NULL);
	os_raise_sigalrm();
}

static int alarm_wdt_start(struct udevice *dev, u64 timeout, ulong flags)
{
	struct alarm_wdt_priv *priv = dev_get_priv(dev);
	unsigned int sec;

	timeout = DIV_ROUND_UP(timeout, 1000);
	sec = min_t(u64, UINT_MAX, timeout);
	priv->timeout_sec = sec;

	os_alarm(0);
	os_set_alarm_handler(alarm_handler);
	os_alarm(sec);

	return 0;
}

static int alarm_wdt_stop(struct udevice *dev)
{
	os_alarm(0);
	os_set_alarm_handler(NULL);

	return 0;
}

static int alarm_wdt_reset(struct udevice *dev)
{
	struct alarm_wdt_priv *priv = dev_get_priv(dev);

	os_alarm(priv->timeout_sec);

	return 0;
}

static int alarm_wdt_expire_now(struct udevice *dev, ulong flags)
{
	alarm_handler(0);

	return 0;
}


static const struct wdt_ops alarm_wdt_ops = {
	.start = alarm_wdt_start,
	.reset = alarm_wdt_reset,
	.stop = alarm_wdt_stop,
	.expire_now = alarm_wdt_expire_now,
};

static const struct udevice_id alarm_wdt_ids[] = {
	{ .compatible = "sandbox,alarm-wdt" },
	{}
};

U_BOOT_DRIVER(alarm_wdt_sandbox) = {
	.name = "alarm_wdt_sandbox",
	.id = UCLASS_WDT,
	.of_match = alarm_wdt_ids,
	.ops = &alarm_wdt_ops,
	.priv_auto = sizeof(struct alarm_wdt_priv),
};
