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
#include "bcm_pinmux.h"
#define set_pinmux bcm_set_pinmux


