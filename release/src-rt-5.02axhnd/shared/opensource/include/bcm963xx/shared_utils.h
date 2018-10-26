#include "boardparms.h"

unsigned int UtilGetChipRev(void);
unsigned int UtilGetChipId(void);
unsigned int util_get_chip_id(void); /*wrapper for RDPA compilation*/
unsigned int UtilGetChipIsLP(void);
char *UtilGetChipName(char *buf, int len);
int UtilGetChipIsPinCompatible(void); 
int UtilGetLanLedGpio(int n, unsigned short *gpio_num);
int UtilGetLan1LedGpio(unsigned short *gpio_num);
int UtilGetLan2LedGpio(unsigned short *gpio_num);
int UtilGetLan3LedGpio(unsigned short *gpio_num);
int UtilGetLan4LedGpio(unsigned short *gpio_num);
#if defined (CONFIG_BCM96838) || defined(_BCM96838_) 
unsigned int gpio_get_dir(unsigned int gpio_num);
void gpio_set_dir(unsigned int gpio_num, unsigned int dir);
unsigned int gpio_get_data(unsigned int gpio_num);
void gpio_set_data(unsigned int gpio_num, unsigned int data);
void set_pinmux(unsigned int pin_num, unsigned int mux_num);
#else
#include "bcm_pinmux.h"
#define set_pinmux bcm_set_pinmux
#endif


