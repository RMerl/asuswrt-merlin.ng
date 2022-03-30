#include <common.h>


#define PINMUX_ADDR_SHIFT       0
#define PINMUX_ADDR_MASK        (0xfff<<PINMUX_ADDR_SHIFT)
#define PINMUX_DATA_SHIFT       12
#define PINMUX_DATA_MASK        (0x3f<<PINMUX_DATA_SHIFT)

#define LOAD_MUX_REG_CMD        0x21
#define LOAD_PAD_CTRL_CMD       0x22
#define LOAD_SELECT_CMD         0x23

/*===================================================*/

typedef struct pinctrl_reg {
	uint32_t TestPortBlockDataMSB;
	uint32_t TestPortBlockDataLSB;
	uint32_t TestPortCmd;
	uint32_t DiagReadBack;
	uint32_t DiagReadBackHi;
}pinctrl_reg;

/* pinmux function */
int bcmbca_pinmux_set (volatile struct pinctrl_reg *regp, int pin, int func);

