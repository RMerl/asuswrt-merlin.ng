/*
 * <:copyright-BRCM:2019:DUAL/GPL:standard
 * 
 *    Copyright (c) 2019 Broadcom 
 *    All Rights Reserved
 * 
 * Unless you and Broadcom execute a separate written software license
 * agreement governing use of this software, this software is licensed
 * to you under the terms of the GNU General Public License version 2
 * (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
 * with the following added to such license:
 * 
 *    As a special exception, the copyright holders of this software give
 *    you permission to link this software with independent modules, and
 *    to copy and distribute the resulting executable under terms of your
 *    choice, provided that you also meet, for each linked independent
 *    module, the terms and conditions of the license of that module.
 *    An independent module is a module which is not derived from this
 *    software.  The special exception does not apply to any modifications
 *    of the software.
 * 
 * Not withstanding the above, under no circumstances may you combine
 * this software in any way with any other Broadcom software provided
 * under a license other than the GPL, without Broadcom's express prior
 * written consent.
 * 
 * :> 
 */

#include <linux/io.h>
#include <linux/leds.h>
#include <linux/of_device.h>
#include <linux/of_address.h>
#include <linux/module.h>
#include <linux/version.h>

#define ASUS_SKIP_CLED_PROBE 1

enum LP_CLED_REGS {
    CLED_CTRL = 0,
    CLED_HW_EN,
    CLED_SERIAL_SHIFT_SEL,
    CLED_HW_POLARITY,
    CLED_SW_SET,
    CLED_SW_POLARITY,
    CLED_CH_ACT,
    CLED_XX_CONFIG,
    CLED_SW_CLEAR,
    CLED_SW_STATUS,
    CLED_MUX,
    CLED_SERIAL_POLARITY,
    CLED_PARALLEL_POLARITY,
    CLED_MAX_REG,
};

struct bcm_bca_cled_ops {
    void (*sw_led_set)(int led_num);
    void (*sw_led_clr)(int led_num);
};

struct bcm_bca_cled_reg_map {
    char *reg_name;
    enum LP_CLED_REGS reg_idx;
};

struct bcm_bca_cled_params_set {
    struct bcm_bca_cled_ops *ops;
    struct bcm_bca_cled_reg_map *reg_map;
    uint8_t cntrl_ver;
};

struct bcm_bca_cled_ctrl {
    void __iomem *led_regs[CLED_MAX_REG];
    spinlock_t lock;
    uint8_t max_supported_leds;
    struct platform_device *pdev;
    uint8_t serial_shifters_num;
    uint32_t serial_led_map;
    uint8_t active_serial_led_count;
    uint32_t mux_maped;
    struct bcm_bca_cled_ops *ops;
    uint8_t cntrl_ver;
};

static struct bcm_bca_cled_ctrl *bca_cled = NULL;

static void cled_sw_led_set(int led_num)
{
    uint32_t led_mask = 1 << led_num;
    *(volatile uint32_t *)(bca_cled->led_regs[CLED_SW_SET]) = led_mask;
}

static void cled_sw_led_clr(int led_num)
{
    uint32_t led_mask = 1 << led_num;
    *(volatile uint32_t *)(bca_cled->led_regs[CLED_SW_CLEAR]) = led_mask;
}

static void cled_legacy_sw_led_set(int led_num)
{
    uint32_t led_mask = 1 << led_num;
    *(volatile uint32_t *)(bca_cled->led_regs[CLED_SW_SET]) |= led_mask;
}

static void cled_legacy_sw_led_clr(int led_num)
{
    uint32_t led_mask = 1 << led_num;
    *(volatile uint32_t *)(bca_cled->led_regs[CLED_SW_SET]) &= ~led_mask;
}

static struct bcm_bca_cled_reg_map cled_reg_map[] = {
    {"glbl_ctrl", CLED_CTRL},
    {"hw_en", CLED_HW_EN},
    {"ser_shift", CLED_SERIAL_SHIFT_SEL},
    {"hw_polarity", CLED_HW_POLARITY}, 
    {"sw_set", CLED_SW_SET},
    {"sw_polarity", CLED_SW_POLARITY},
    {"ch_activate", CLED_CH_ACT},
    {"ch_config", CLED_XX_CONFIG},
    {"sw_clear", CLED_SW_CLEAR}, 
    {"sw_status", CLED_SW_STATUS},
    {"out_mux", CLED_MUX},
    {"ser_polarity", CLED_SERIAL_POLARITY},
    {"par_polarity", CLED_PARALLEL_POLARITY},
    { },
}; 

static struct bcm_bca_cled_reg_map cled_legacy_reg_map[] = {
    {"glbl_ctrl", CLED_CTRL},
    {"hw_en", CLED_HW_EN},
    {"ser_shift", CLED_SERIAL_SHIFT_SEL},
    {"hw_polarity", CLED_HW_POLARITY}, 
    {"sw_set", CLED_SW_SET},
    {"sw_polarity", CLED_SW_POLARITY},
    {"ch_activate", CLED_CH_ACT},
    {"ch_config", CLED_XX_CONFIG},
    {"ser_polarity", CLED_SERIAL_POLARITY},
    {"par_polarity", CLED_PARALLEL_POLARITY},
    { },
};

static struct bcm_bca_cled_ops ops = {
    .sw_led_set = cled_sw_led_set,
    .sw_led_clr = cled_sw_led_clr,
}; 

static struct bcm_bca_cled_ops legacy_ops = {
    .sw_led_set = cled_legacy_sw_led_set,
    .sw_led_clr = cled_legacy_sw_led_clr,
}; 

static struct bcm_bca_cled_params_set cled_set_v2 = {
    .ops = &ops,
    .reg_map = &cled_reg_map[0],
    .cntrl_ver = 2,
};

static struct bcm_bca_cled_params_set cled_set = {
    .ops = &ops,
    .reg_map = &cled_reg_map[0],
    .cntrl_ver = 1,
};

static struct bcm_bca_cled_params_set cled_legacy_set = {
    .ops = &legacy_ops,
    .reg_map = &cled_legacy_reg_map[0],
    .cntrl_ver = 0,
};

static const struct of_device_id bca_cled_ctrl_of_match[] = {
	{ .compatible = "brcm,bca-cleds-ctrl", .data = (void *)&cled_set, },
	{ .compatible = "brcm,bca-cleds-ctrl,v2", .data = (void *)&cled_set_v2, },
	{ .compatible = "brcm,bca-cleds-ctrl,legacy", .data = (void *)&cled_legacy_set, },
	{ },
};

MODULE_DEVICE_TABLE(of, bca_cled_ctrl_of_match);

static int bca_cled_ctrl_probe(struct platform_device *pdev)
{
    struct device *dev = &pdev->dev;
    const struct of_device_id *match;
	struct resource *res;
    unsigned int val = 0;
	int ret;
    bool serial_msb_first = 0;
    bool serial_data_polarity_low = 0;
    uint32_t reg_val;
    struct device_node *serial_pinctrl;
    struct bcm_bca_cled_params_set *params;
    struct bcm_bca_cled_reg_map *reg_map;
    volatile uint32_t *mux_output = NULL;
    int i;

	match = of_match_device(bca_cled_ctrl_of_match, dev);
	if (!match)
    {
		dev_err(dev, "Failed to find CLED controller\n");
		return -ENODEV;
	}
    params = (struct bcm_bca_cled_params_set*)(match->data);

	bca_cled = devm_kzalloc(dev, sizeof(*bca_cled), GFP_KERNEL);
	if (!bca_cled)
    {
		dev_err(dev, "Failed to allocate memory for CLED controller\n");
        return -ENOMEM;
    }

    bca_cled->ops = params->ops;
    bca_cled->cntrl_ver = params->cntrl_ver;
    reg_map = params->reg_map;

    if(of_property_read_u32(dev->of_node, "nleds", &val))
    {
        dev_err(dev, "nleds property not present\n");
        ret = -EINVAL;
        goto error;
    }

    bca_cled->max_supported_leds = (uint8_t)val;
    dev_info(dev, "max supported leds %d[%d]\n", bca_cled->max_supported_leds, val);

    serial_pinctrl = of_parse_phandle(dev->of_node, "pinctrl-0", 0);

    if (serial_pinctrl)
    {

        if (of_property_read_u32(dev->of_node, "serial-shifters-installed", &val)) 
        {
            dev_err(dev, "The serial-shifters-installed property not present while Serial LED controller interface is configured\n");
            ret = -EINVAL;
            goto error;
        }

        bca_cled->serial_shifters_num = (uint8_t)val;

        serial_msb_first = of_property_read_bool(dev->of_node, "serial-order-msb-first");
        serial_data_polarity_low =  of_property_read_bool(dev->of_node, "serial-data-polarity-low");
        dev_info(dev, "Serial CLED interface found num shifters %d serial data polarity low %d\n",
            bca_cled->serial_shifters_num, serial_data_polarity_low);
    }
    else
    {
        dev_info(dev, " Parallel CLED interface found\n");
    }

    bca_cled->pdev = pdev;
    platform_set_drvdata(pdev, bca_cled);
    spin_lock_init(&bca_cled->lock);


    while (reg_map->reg_name)
    {
        res = platform_get_resource_byname(pdev, IORESOURCE_MEM, reg_map->reg_name);
        if (!res)
        {
            dev_err(dev, "Failed to find %s resource\n", reg_map->reg_name);
            ret = -EINVAL;
            goto error;
        }

        bca_cled->led_regs[reg_map->reg_idx] = devm_ioremap_resource(dev, res);
        if (IS_ERR(bca_cled->led_regs[reg_map->reg_idx])) 
        {
            dev_err(dev, "Failed to map the %s resource\n", reg_map->reg_name);
            ret = -ENXIO;
            goto error;
        }
        reg_map++;
    }
#ifdef ASUS_SKIP_CLED_PROBE
    goto asus;
#endif
    if (!of_property_read_u32(dev->of_node, "hw-polarity-quirk", &val)) 
    {
        *(volatile uint32_t *)(bca_cled->led_regs[CLED_HW_POLARITY]) = (uint32_t)val;
    }

    if (!of_property_read_u32(dev->of_node, "sw-polarity-quirk", &val)) 
    {
        *(volatile uint32_t *)(bca_cled->led_regs[CLED_SW_POLARITY]) = (uint32_t)val;
    }

    if (serial_data_polarity_low)
        reg_val = 0x8;
    else
        reg_val = 0xa;

    if (serial_msb_first)
        reg_val |= 0x10;
    else
        *(volatile uint32_t *)(bca_cled->led_regs[CLED_CTRL]) &= ~(0x10);

    *(volatile uint32_t *)(bca_cled->led_regs[CLED_CTRL]) |= reg_val;

    *(volatile uint32_t *)(bca_cled->led_regs[CLED_HW_EN]) = 0;
    *(volatile uint32_t *)(bca_cled->led_regs[CLED_SERIAL_POLARITY]) = 0;
    *(volatile uint32_t *)(bca_cled->led_regs[CLED_PARALLEL_POLARITY]) = 0;

    mux_output = (volatile uint32_t *)bca_cled->led_regs[CLED_MUX];
    if (mux_output) { 
        uint32_t init_mask = (bca_cled->cntrl_ver == 1) ? 0x1F1F1F1F : 0x3F3F3F3F; 
        for (i = 0; i<8; i++)
            mux_output[i] = init_mask;
    }

    dev_info(dev, "BCA CLED Controller initialized\n");
#ifdef ASUS_SKIP_CLED_PROBE
asus:
#endif
    of_platform_populate(dev->of_node, NULL, NULL, dev);

    return 0;

error:
    if (bca_cled)
    {
       devm_kfree(dev, bca_cled);
    }
    return ret;
}

static struct platform_driver bcm_bca_cled_ctrl_driver = {
	.probe = bca_cled_ctrl_probe,
	.driver = {
		.name = "bcm-bca-cled-ctrl",
		.of_match_table = bca_cled_ctrl_of_match,
	},
};

struct cled_cfg {
    union {
        struct {
            unsigned int mode:2;            /* [01]-[00] */
            unsigned int reserved1:1;       /* [02]-[02] */
            unsigned int flash_ctrl:3;      /* [05]-[03] */
            unsigned int bright_ctrl:8;     /* [13]-[06] */
            unsigned int repeat_cycle:1;    /* [14]-[14] */
            unsigned int change_dir:1;      /* [15]-[15] */
            unsigned int phas_1_bright:1;   /* [16]-[16] */
            unsigned int phas_2_bright:1;   /* [17]-[17] */
            unsigned int reserved2:2;       /* [19]-[18] */
            unsigned int init_delay:4;      /* [23]-[20] */
            unsigned int final_delay:4;     /* [27]-[24] */
            unsigned int color_blend_c:4;   /* [31]-[28] */
        } Bits;
        uint32_t Reg;
    }cfg0;
    union {
        struct {
            unsigned int b_step_1:4;        /* [03]-[00] */
            unsigned int t_step_1:4;        /* [07]-[04] */
            unsigned int n_step_1:4;        /* [11]-[08] */
            unsigned int b_step_2:4;        /* [15]-[12] */
            unsigned int t_step_2:4;        /* [19]-[16] */
            unsigned int n_step_2:4;        /* [23]-[20] */
            unsigned int reserved:8;        /* [31]-[24] */
        } Bits;
        uint32_t Reg;
    }cfg1;
    union {
        struct {
            unsigned int b_step_3:4;        /* [03]-[00] */
            unsigned int t_step_3:4;        /* [07]-[04] */
            unsigned int n_step_3:4;        /* [11]-[08] */
            unsigned int final_step:4;      /* [15]-[12] */
            unsigned int reserved:16;       /* [31]-[16] */
        } Bits;
        uint32_t Reg;
    }cfg2;
    union {
        struct {
            unsigned int phase_delay_1:16;  /* [15]-[00] */
            unsigned int phase_delay_2:16;  /* [31]-[16] */
        } Bits;
        uint32_t Reg;
    }cfg3;
};

static int setup_crossbar(const struct device_node *np, unsigned int led_num)
{
    uint32_t crossbar_output;
    uint8_t mux_idx = 0;
    uint32_t mux_mask = (bca_cled->cntrl_ver == 1) ? 0x1f : 0x3f;
    uint32_t mux_val = 0;
    volatile uint32_t *mux_output;
    unsigned long flags;

    if(of_property_read_u32(np, "crossbar-output", &crossbar_output))
    {
        dev_err(&bca_cled->pdev->dev, "crossbar-output property not present\n");
        return -EINVAL;
    }

    spin_lock_irqsave(&bca_cled->lock, flags);
    if (bca_cled->mux_maped & (1 << crossbar_output))
    {
        spin_unlock_irqrestore(&bca_cled->lock, flags);
        dev_err(&bca_cled->pdev->dev, "crossbar-output is already used\n");
        return -EINVAL;
    }

    mux_idx = (crossbar_output >> 2);
    mux_mask = mux_mask <<((crossbar_output & 0x3) << 3);
    mux_val = led_num<<((crossbar_output & 0x3)<<3);

    mux_output = (volatile uint32_t *)bca_cled->led_regs[CLED_MUX];
    mux_output[mux_idx] &= ~mux_mask;
    mux_output[mux_idx] |= mux_val;
    bca_cled->mux_maped |= (1 << crossbar_output);

    spin_unlock_irqrestore(&bca_cled->lock, flags);

    return crossbar_output;
}

int bca_cled_setup_serial(unsigned int led_num, unsigned int polarity, unsigned int is_hw, 
    const struct device_node *np, bool is_crossbar)
{
    unsigned long flags;
    uint32_t led_mask = 1 << led_num;
    volatile uint32_t *polarity_reg;
    uint32_t led_map = 0;
    uint8_t missed_pins;
    int i;
    int return_led_num = led_num;

    if(!bca_cled)
        return -ENODEV;

    if (led_num > bca_cled->max_supported_leds)
    {
        dev_err(&bca_cled->pdev->dev,"requested LED %d is out of supported range(%d)\n", led_num,
            bca_cled->max_supported_leds);
        return -EINVAL;
    }

    if (bca_cled->serial_shifters_num == 0)
    {
        dev_err(&bca_cled->pdev->dev,"Serial LED%d is requested, but no serial LED interface defined\n", led_num);
        return -EINVAL;
    }
    
    if (is_crossbar)
    {
        int output_serial_ch = setup_crossbar(np, led_num);

        if (output_serial_ch < 0)
            return -EINVAL;

        bca_cled->serial_led_map |= (1 << output_serial_ch);
        if (bca_cled->cntrl_ver > 1)
        {
            led_mask = (1 << output_serial_ch);
            return_led_num = output_serial_ch;
        }
    }
    else
    {
        bca_cled->serial_led_map |= led_mask;
    }

    polarity_reg = (volatile uint32_t *)bca_cled->led_regs[CLED_SERIAL_POLARITY];

    spin_lock_irqsave(&bca_cled->lock, flags);
    bca_cled->active_serial_led_count++;
    if (bca_cled->active_serial_led_count > (bca_cled->serial_shifters_num * 8))
    {
        bca_cled->active_serial_led_count--;
        spin_unlock_irqrestore(&bca_cled->lock, flags);
        dev_err(&bca_cled->pdev->dev,"The number of registered serial LEDs is bigger than supported by this configuration\n" );
        return -EINVAL;
    }

    if (is_hw)
        *(volatile uint32_t *)(bca_cled->led_regs[CLED_HW_EN]) |= led_mask;

    if (polarity)
        *polarity_reg |= led_mask;
    else
        *polarity_reg &= ~(led_mask);

    missed_pins = (bca_cled->serial_shifters_num * 8) - bca_cled->active_serial_led_count;

    led_map = bca_cled->serial_led_map;

    for (i = 0; i< 32 && missed_pins; i++)
    {
        if (bca_cled->serial_led_map & (1 << i))
            continue;
        led_map |= (1 << i);
        missed_pins--;
    }

    *(volatile uint32_t *)(bca_cled->led_regs[CLED_SERIAL_SHIFT_SEL]) = led_map; 

    spin_unlock_irqrestore(&bca_cled->lock, flags);
    return return_led_num;
}

int bca_cled_setup_parallel(unsigned int led_num, int polarity, int is_hw, const struct device_node *np, 
    bool is_crossbar)
{
    unsigned long flags;
    uint32_t led_mask = 1 << led_num;
    volatile uint32_t *polarity_reg;
    struct device_node *led_pinctrl;
    int return_led_num = led_num;
 
    if(!bca_cled)
        return -ENODEV;

    if(led_num > bca_cled->max_supported_leds)
    {
        dev_err(&bca_cled->pdev->dev,"requested LED %d is out of supported range(%d)\n", led_num,
            bca_cled->max_supported_leds);
        return -EINVAL;
    }

    if(is_crossbar)
    {
        int crossbar_output;
        led_pinctrl = of_parse_phandle(np, "pinctrl-0", 0);
        if(!led_pinctrl)
        {
            dev_err(&bca_cled->pdev->dev,"requested parallel LED %d does not define proper pinctrl-0\n", 
                led_num);
            return -EINVAL;
        }

        crossbar_output = setup_crossbar(led_pinctrl, led_num);
        if ( crossbar_output < 0)
            return -EINVAL;
        if (bca_cled->cntrl_ver > 1)
        {
            led_mask = 1 << crossbar_output;
            return_led_num = crossbar_output;
        }
    }
    polarity_reg = (volatile uint32_t *)bca_cled->led_regs[CLED_PARALLEL_POLARITY];

    spin_lock_irqsave(&bca_cled->lock, flags);

    if (is_hw)
        *(volatile uint32_t *)(bca_cled->led_regs[CLED_HW_EN]) |= led_mask;

    if (polarity)
        *polarity_reg |= led_mask;
    else
        *polarity_reg &= ~(led_mask);

    spin_unlock_irqrestore(&bca_cled->lock, flags);
    return return_led_num;
}

int bca_cled_set_value(unsigned int led_num, unsigned int value)
{
    unsigned long flags;
    
    if(!bca_cled)
        return -ENODEV;

    if (led_num > bca_cled->max_supported_leds)
    {
        dev_err(&bca_cled->pdev->dev,"requested LED %d is out of supported range(%d)\n", led_num,
            bca_cled->max_supported_leds);
        return -EINVAL;
    }
    spin_lock_irqsave(&bca_cled->lock, flags);
    if (value)
        bca_cled->ops->sw_led_set(led_num);
    else
        bca_cled->ops->sw_led_clr(led_num);
    
    spin_unlock_irqrestore(&bca_cled->lock, flags);
    
    return 0;
}

int bca_cled_set_brightness(unsigned int led_num, unsigned int value)
{
    unsigned long flags;
    uint32_t led_mask = 1 << led_num;
    volatile struct cled_cfg *led_config;

    if(!bca_cled)
        return -ENODEV;

    if (led_num > bca_cled->max_supported_leds)
    {
        dev_err(&bca_cled->pdev->dev,"requested LED %d is out of supported range(%d)\n", led_num,
            bca_cled->max_supported_leds);
        return -EINVAL;
    }

    led_config = (volatile struct cled_cfg *)(bca_cled->led_regs[CLED_XX_CONFIG]);

    spin_lock_irqsave(&bca_cled->lock, flags);
    led_config[led_num].cfg0.Bits.bright_ctrl = value;
    *(volatile uint32_t *)(bca_cled->led_regs[CLED_CH_ACT]) = led_mask;

    spin_unlock_irqrestore(&bca_cled->lock, flags);
    
    return 0;
}

int bca_cled_set_flash_rate(unsigned int led_num, unsigned int value)
{
    unsigned long flags;
    uint32_t led_mask = 1 << led_num;
    volatile struct cled_cfg *led_config;

    if(!bca_cled)
        return -ENODEV;

    if (led_num > bca_cled->max_supported_leds)
    {
        dev_err(&bca_cled->pdev->dev,"requested LED %d is out of supported range(%d)\n", led_num,
            bca_cled->max_supported_leds);
        return -EINVAL;
    }

    led_config = (volatile struct cled_cfg *)(bca_cled->led_regs[CLED_XX_CONFIG]);

    spin_lock_irqsave(&bca_cled->lock, flags);
    led_config[led_num].cfg0.Bits.flash_ctrl = value;
    *(volatile uint32_t *)(bca_cled->led_regs[CLED_CH_ACT]) = led_mask;

    spin_unlock_irqrestore(&bca_cled->lock, flags);
    
    return 0;
}

EXPORT_SYMBOL(bca_cled_setup_serial);
EXPORT_SYMBOL(bca_cled_setup_parallel);
EXPORT_SYMBOL(bca_cled_set_value);
EXPORT_SYMBOL(bca_cled_set_brightness);
EXPORT_SYMBOL(bca_cled_set_flash_rate);
    
static int __init bcmbca_cled_ctrl_drv_reg(void)
{
	return platform_driver_register(&bcm_bca_cled_ctrl_driver);
}

postcore_initcall(bcmbca_cled_ctrl_drv_reg);

MODULE_AUTHOR("Samyon Furman (samyon.furman@broadcom.com)");
MODULE_DESCRIPTION("Broadcom BCA CLED Controller Driver");
MODULE_LICENSE("GPL v2");
