unsigned int bcm_gpio_get_dir(unsigned int gpio_num);
void bcm_gpio_set_dir(unsigned int gpio_num, unsigned int dir);
unsigned int bcm_gpio_get_data(unsigned int gpio_num);
void bcm_gpio_set_data(unsigned int gpio_num, unsigned int data);
#if defined(CONFIG_BCM960333) || defined(_BCM960333_)
unsigned int bcm_gpio_get_funcmode(unsigned int gpio_num);
void bcm_gpio_set_funcmode(unsigned int gpio_num, unsigned int mode);
#endif
