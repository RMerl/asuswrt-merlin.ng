// SPDX-License-Identifier: GPL-2.0+

#include <common.h>
#include <linux/errno.h>
#include <asm/mach-imx/video.h>

#ifdef CONFIG_IMX_HDMI
#include <asm/arch/mxc_hdmi.h>
#include <asm/io.h>

int detect_hdmi(struct display_info_t const *dev)
{
	struct hdmi_regs *hdmi	= (struct hdmi_regs *)HDMI_ARB_BASE_ADDR;
	return readb(&hdmi->phy_stat0) & HDMI_DVI_STAT;
}
#endif

int board_video_skip(void)
{
	int i;
	int ret = 0;
	char const *panel = env_get("panel");

	if (!panel) {
		for (i = 0; i < display_count; i++) {
			struct display_info_t const *dev = displays+i;
			if (dev->detect && dev->detect(dev)) {
				panel = dev->mode.name;
				printf("auto-detected panel %s\n", panel);
				break;
			}
		}
		if (!panel) {
			panel = displays[0].mode.name;
			printf("No panel detected: default to %s\n", panel);
			i = 0;
		}
	} else {
		for (i = 0; i < display_count; i++) {
			if (!strcmp(panel, displays[i].mode.name))
				break;
		}
	}

	if (i < display_count) {
		ret = ipuv3_fb_init(&displays[i].mode, displays[i].di ? 1 : 0,
				    displays[i].pixfmt);
		if (!ret) {
			if (displays[i].enable)
				displays[i].enable(displays + i);

			printf("Display: %s (%ux%u)\n",
			       displays[i].mode.name,
			       displays[i].mode.xres,
			       displays[i].mode.yres);

#ifdef CONFIG_IMX_HDMI
			if (!strcmp(displays[i].mode.name, "HDMI"))
				imx_enable_hdmi_phy();
#endif
		} else
			printf("LCD %s cannot be configured: %d\n",
			       displays[i].mode.name, ret);
	} else {
		printf("unsupported panel %s\n", panel);
		return -EINVAL;
	}

	return ret;
}

int ipu_displays_init(void)
{
	return board_video_skip();
}
