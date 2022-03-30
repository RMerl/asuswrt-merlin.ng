
void bcm_set_pinmux(unsigned int pin_num, unsigned int mux_num);
int bcm_init_pinmux(void);
void bcm_init_pinmux_interface(unsigned int interface);
int bcm_pinmux_update_optled_map(unsigned short led_gpio_num, unsigned int muxinfo);
