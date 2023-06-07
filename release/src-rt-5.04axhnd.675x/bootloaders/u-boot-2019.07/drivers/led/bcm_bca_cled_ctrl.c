// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2019 Broadcom
 */
/*
 *  
 */
#include <common.h>
#include <dm.h>
#include <dm/ofnode.h>
#include <errno.h>
#include <led.h>
#include <asm/io.h>

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
    int (*sw_led_get)(int led_num);
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
    uint8_t max_supported_leds;
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

static int cled_sw_led_get(int led_num)
{
    uint32_t led_mask = 1 << led_num;
    if (*(volatile uint32_t *)(bca_cled->led_regs[CLED_SW_STATUS]) & led_mask)
        return 1;

    return 0;
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

static int cled_legacy_sw_led_get(int led_num)
{
    uint32_t led_mask = 1 << led_num;
    if (*(volatile uint32_t *)(bca_cled->led_regs[CLED_SW_SET]) & led_mask)
        return 1;

    return 0;
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
    .sw_led_get = cled_sw_led_get,
}; 

static struct bcm_bca_cled_ops legacy_ops = {
    .sw_led_set = cled_legacy_sw_led_set,
    .sw_led_clr = cled_legacy_sw_led_clr,
    .sw_led_get = cled_legacy_sw_led_get,
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

static const struct udevice_id bca_cled_ctrl_of_match[] = {
	{ .compatible = "brcm,bca-cleds-ctrl", .data = (long unsigned int)&cled_set, },
	{ .compatible = "brcm,bca-cleds-ctrl,v2", .data = (long unsigned int)&cled_set_v2, },
	{ .compatible = "brcm,bca-cleds-ctrl,legacy", .data = (long unsigned int)&cled_legacy_set, },
	{ },
};

static int driver_check_compatible(const struct udevice_id *of_match,
				   const struct udevice_id **of_idp,
				   const char *compat)
{
	if (!of_match)
		return -ENOENT;

	while (of_match->compatible) {
		if (!strcmp(of_match->compatible, compat)) {
			*of_idp = of_match;
			return 0;
		}
		of_match++;
	}

	return -ENOENT;
}

static const ofnode of_parse_phandle(const ofnode handle, const char *phandle_name, int index)
{
    ofnode node = ofnode_null();
    struct ofnode_phandle_args out_args;

    if (ofnode_parse_phandle_with_args(handle, phandle_name, NULL, 0, index, &out_args) == 0)
    {
        node = out_args.node;
    }
    return node;
}

#if defined(XD4PRO) || defined(XT8PRO) || defined(BM68) || defined(XT8_V2) || defined(ET8PRO) || defined(ET8_V2) || defined(XC5) || defined(EBA63)
static void led_ch(uint32_t *init_mask, uint32_t led) {
    uint32_t mux_no = led / 4;
    uint32_t mux_index = led % 4;
    uint32_t val32 = init_mask[mux_no];

    val32 &= ~(0xFF << (mux_index * 8));
    val32 |= (led << (mux_index * 8));
    init_mask[mux_no] = val32;
}

static void init_led_out_mux_mask(uint32_t *init_mask) {
    uint32_t mask = (bca_cled->cntrl_ver == 1) ? 0x1F1F1F1F : 0x3F3F3F3F;
	int i;
    for (i=0; i<8; i++)
        init_mask[i] = mask;

    /* init LED 14, 15, 16, 29 */
    led_ch(init_mask, 14);
    led_ch(init_mask, 15);
    led_ch(init_mask, 16);
    led_ch(init_mask, 29);
}
#endif

static int bca_cled_ctrl_probe(struct udevice *dev)
{
    unsigned int val = 0;
	int ret;
    bool serial_msb_first = 0;
    bool serial_data_polarity_low = 0;
    uint32_t reg_val;
    ofnode serial_pinctrl;
    struct bcm_bca_cled_params_set *params;
    struct bcm_bca_cled_reg_map *reg_map;
    ulong data = 0;
    volatile uint32_t *mux_output = NULL;
    int i;

    data = dev_get_driver_data(dev);
    if (!data)
        return -ENODEV;

    params = (struct bcm_bca_cled_params_set*)data;

	bca_cled = dev_get_priv(dev);

    bca_cled->ops = params->ops;
    bca_cled->cntrl_ver = params->cntrl_ver;
    reg_map = params->reg_map;

    if(dev_read_u32u(dev, "nleds", &val))
    {
        printf("nleds property not present\n");
        ret = -EINVAL;
        goto error;
    }

    bca_cled->max_supported_leds = (uint8_t)val;
    printf("max supported leds %d[%d]\n", bca_cled->max_supported_leds, val);

    serial_pinctrl = of_parse_phandle(dev_ofnode(dev), "pinctrl-0", 0);

    if (serial_pinctrl.of_offset != -1)
    {

        if (dev_read_u32u(dev, "serial-shifters-installed", &val)) 
        {
            printf("The serial-shifters-installed property not present while Serial LED controller interface is configured\n");
            ret = -EINVAL;
            goto error;
        }

        bca_cled->serial_shifters_num = (uint8_t)val;

        serial_msb_first = dev_read_bool(dev, "serial-order-msb-first");
        serial_data_polarity_low =  dev_read_bool(dev, "serial-data-polarity-low");
        printf("Serial CLED interface found num shifters %d serial data polarity low %d\n",
            bca_cled->serial_shifters_num, serial_data_polarity_low);
    }
    else
    {
        printf(" Parallel CLED interface found\n");
    }

    while (reg_map->reg_name)
    {
        bca_cled->led_regs[reg_map->reg_idx] = dev_remap_addr_name(dev, reg_map->reg_name);
        if (bca_cled->led_regs[reg_map->reg_idx] == NULL)
        {
            printf("Failed to find %s resource\n", reg_map->reg_name);
            ret = -EINVAL;
            goto error;
        }
        reg_map++;
    }

    if (!dev_read_u32u(dev, "hw-polarity-quirk", &val)) 
    {
        *(volatile uint32_t *)(bca_cled->led_regs[CLED_HW_POLARITY]) = (uint32_t)val;
    }

    if (!dev_read_u32u(dev, "sw-polarity-quirk", &val)) 
    {
        *(volatile uint32_t *)(bca_cled->led_regs[CLED_SW_POLARITY]) = (uint32_t)val;
    }


    *(volatile uint32_t *)(bca_cled->led_regs[CLED_CTRL]) &= ~(0xa);
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
    if (mux_output)
    {
#if defined(XD4PRO) || defined(XT8PRO) || defined(BM68) || defined(XT8_V2) || defined(ET8PRO) || defined(ET8_V2) || defined(XC5) || defined(EBA63)
        uint32_t init_mask[8];
        init_led_out_mux_mask(init_mask);
#else
        uint32_t init_mask = (bca_cled->cntrl_ver == 1) ? 0x1F1F1F1F : 0x3F3F3F3F; 
#endif
#if defined(XD4PRO) || defined(XT8PRO) || defined(BM68) || defined(XT8_V2) || defined(ET8PRO) || defined(ET8_V2) || defined(XC5) || defined(EBA63)
        for (i = 0; i<8; i++)
            mux_output[i] = init_mask[i];
#else
        for (i = 0; i<8; i++)
            mux_output[i] = init_mask;
#endif
    }

    printf("BCA CLED Controller initialized\n");
    
    return 0;

error:
    return ret;
}

static int bca_cled_ctrl_bind(struct udevice *parent)
{
    const char *compat;
    const struct udevice_id *id;
	ofnode node;
    int ret;

    compat = dev_read_prop(parent, "compatible", NULL);
    ret = driver_check_compatible(bca_cled_ctrl_of_match, &id, compat);
    if (ret)
        return -ENODEV;

    parent->driver_data = id->data;

	dev_for_each_subnode(node, parent) {
		struct led_uc_plat *uc_plat;
		struct udevice *dev;
		const char *label;
		int ret;

		ret = device_bind_driver_to_node(parent, "bcm-bca-leds", ofnode_get_name(node),
            node, &dev);
		if (ret)
        {
            if (ret != -ENODEV)
                printf("failed to bind node %s ret %d\n", ofnode_get_name(node), ret);
            continue;
        }

		label = ofnode_read_string(node, "label");
        if (label)
        {
            uc_plat = dev_get_uclass_platdata(dev);
            uc_plat->label = label;
        }
	}

	return 0;
}

U_BOOT_DRIVER(bcm_bca_cled_ctrl_driver) = {
    .name = "bcm-bca-cled-ctrl",
	.id = UCLASS_LED,
	.probe = bca_cled_ctrl_probe,
    .bind = bca_cled_ctrl_bind,
    .of_match = bca_cled_ctrl_of_match,
	.priv_auto_alloc_size = sizeof(struct bcm_bca_cled_ctrl),
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

static int setup_crossbar(ofnode np, unsigned int led_num)
{
    uint32_t crossbar_output;
    uint8_t mux_idx = 0;
    uint32_t mux_mask = (bca_cled->cntrl_ver == 1) ? 0x1f : 0x3f;
    uint32_t mux_val = 0;
    volatile uint32_t *mux_output;

    if(ofnode_read_u32(np, "crossbar-output", &crossbar_output))
    {
        printf( "crossbar-output property not present\n");
        return -EINVAL;
    }

    if (bca_cled->mux_maped & (1 << crossbar_output))
    {
        printf( "crossbar-output is already used\n");
        return -EINVAL;
    }

    mux_idx = (crossbar_output >> 2);
    mux_mask = mux_mask <<((crossbar_output & 0x3) << 3);
    mux_val = led_num<<((crossbar_output &0x3)<<3);

    mux_output = (volatile uint32_t *)bca_cled->led_regs[CLED_MUX];
    mux_output[mux_idx] &= ~mux_mask;
    mux_output[mux_idx] |= mux_val;
    bca_cled->mux_maped |= (1 << crossbar_output);

    return crossbar_output;
}

int bca_cled_setup_serial(unsigned int led_num, unsigned int polarity, unsigned int is_hw, 
    const struct udevice *dev, bool is_crossbar)
{
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
        printf("requested LED %d is out of supported range(%d)\n", led_num,
            bca_cled->max_supported_leds);
        return -EINVAL;
    }

    if (bca_cled->serial_shifters_num == 0)
    {
        printf("Serial LED%d is requested, but no serial LED interface defined\n", led_num);
        return -EINVAL;
    }

    if (is_crossbar)
    {
        int output_serial_ch = setup_crossbar(dev_ofnode(dev), led_num);
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

    bca_cled->active_serial_led_count++;
    if (bca_cled->active_serial_led_count > (bca_cled->serial_shifters_num * 8))
    {
        bca_cled->active_serial_led_count--;
        printf("The number of registered serial LEDs is bigger than supported by this configuration\n" );
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

    for (i = 0; i < 32 && missed_pins; i++)
    {
        if (bca_cled->serial_led_map & (1 << i))
            continue;
        led_map |= (1 << i);
        missed_pins--;
    }

    *(volatile uint32_t *)(bca_cled->led_regs[CLED_SERIAL_SHIFT_SEL]) = led_map;
    return return_led_num;
}

int bca_cled_setup_parallel(unsigned int led_num, int polarity, int is_hw, struct udevice *dev, bool is_crossbar)
{
    uint32_t led_mask = 1 << led_num;
    volatile uint32_t *polarity_reg;
    ofnode led_pinctrl;
    int return_led_num = led_num;
  
    if(!bca_cled)
        return -ENODEV;

    if (led_num > bca_cled->max_supported_leds)
    {
        printf("requested LED %d is out of supported range(%d)\n", led_num,
            bca_cled->max_supported_leds);
        return -EINVAL;
    }
    if(is_crossbar)
    {
        int crossbar_output;
        led_pinctrl = of_parse_phandle(dev_ofnode(dev), "pinctrl-0", 0);
        if (led_pinctrl.of_offset == -1)
        {
            printf("requested parallel LED %d does not define proper pinctrl-0\n", 
                led_num);
            return -EINVAL;
        }
        crossbar_output = setup_crossbar(led_pinctrl, led_num);
        if (crossbar_output < 0)
            return -EINVAL;
        if (bca_cled->cntrl_ver > 1)
        {
            led_mask = 1 << crossbar_output;
            return_led_num = crossbar_output;
        }
    }

    polarity_reg = (volatile uint32_t *)bca_cled->led_regs[CLED_PARALLEL_POLARITY];


    if (is_hw)
        *(volatile uint32_t *)(bca_cled->led_regs[CLED_HW_EN]) |= led_mask;

    if (polarity)
        *polarity_reg |= led_mask;
    else
        *polarity_reg &= ~(led_mask);

    return return_led_num;
}

int bca_cled_get_value(unsigned int led_num)
{
    if(!bca_cled)
        return -ENODEV;

    if (led_num > bca_cled->max_supported_leds)
    {
        printf("requested LED %d is out of supported range(%d)\n", led_num,
            bca_cled->max_supported_leds);
        return -EINVAL;
    }
    
    return bca_cled->ops->sw_led_get(led_num);
}

int bca_cled_set_value(unsigned int led_num, unsigned int value)
{
    if(!bca_cled)
        return -ENODEV;

    if (led_num > bca_cled->max_supported_leds)
    {
        printf("requested LED %d is out of supported range(%d)\n", led_num,
            bca_cled->max_supported_leds);
        return -EINVAL;
    }
    if (value)
        bca_cled->ops->sw_led_set(led_num);
    else
        bca_cled->ops->sw_led_clr(led_num);
    
    return 0;
}

int bca_cled_set_brightness(unsigned int led_num, unsigned int value)
{
    uint32_t led_mask = 1 << led_num;
    volatile struct cled_cfg *led_config;

    if(!bca_cled)
        return -ENODEV;

    if (led_num > bca_cled->max_supported_leds)
    {
        printf("requested LED %d is out of supported range(%d)\n", led_num,
            bca_cled->max_supported_leds);
        return -EINVAL;
    }

    led_config = (volatile struct cled_cfg *)(bca_cled->led_regs[CLED_XX_CONFIG]);

    led_config[led_num].cfg0.Bits.bright_ctrl = value;
    *(volatile uint32_t *)(bca_cled->led_regs[CLED_CH_ACT]) = led_mask;
    
    return 0;
}

int bca_cled_set_flash_rate(unsigned int led_num, unsigned int value)
{
    uint32_t led_mask = 1 << led_num;
    volatile struct cled_cfg *led_config;

    if(!bca_cled)
        return -ENODEV;

    if (led_num > bca_cled->max_supported_leds)
    {
        printf("requested LED %d is out of supported range(%d)\n", led_num,
            bca_cled->max_supported_leds);
        return -EINVAL;
    }

    led_config = (volatile struct cled_cfg *)(bca_cled->led_regs[CLED_XX_CONFIG]);

    led_config[led_num].cfg0.Bits.flash_ctrl = value;
    *(volatile uint32_t *)(bca_cled->led_regs[CLED_CH_ACT]) = led_mask;
    
    return 0;
}

