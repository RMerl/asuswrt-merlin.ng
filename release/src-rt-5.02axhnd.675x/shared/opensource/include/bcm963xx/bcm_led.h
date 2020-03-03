
#if !defined(_BCM_LED_H_)
#define _BCM_LED_H_

#define BCM_LED_OFF 0
#define BCM_LED_ON  1

void bcm_common_led_init(void);
void bcm_led_driver_set(unsigned short num, unsigned short state);
void bcm_led_driver_toggle(unsigned short num);
void bcm_common_led_setAllSoftLedsOff(void);
void bcm_common_led_setInitial(void);
short * bcm_led_driver_get_optled_map(void);
void bcm_ethsw_led_init(void);
void bcm_led_zero_flash_rate(int channel);
void bcm_led_set_source(unsigned int serial_sel, unsigned int hwled_sel);

#endif  /* _BCM_LED_H_ */


