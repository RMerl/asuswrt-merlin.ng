unsigned int bcm_gpio_get_dir(unsigned int gpio_num);
void bcm_gpio_set_dir(unsigned int gpio_num, unsigned int dir);
unsigned int bcm_gpio_get_data(unsigned int gpio_num);
void bcm_gpio_set_data(unsigned int gpio_num, unsigned int data);

int bcm_common_gpio_init(void);
