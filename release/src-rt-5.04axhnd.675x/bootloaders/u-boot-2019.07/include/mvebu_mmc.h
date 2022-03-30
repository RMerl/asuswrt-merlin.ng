/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Marvell MMC/SD/SDIO driver
 *
 * (C) Copyright 2012
 * Marvell Semiconductor <www.marvell.com>
 * Written-by: Maen Suleiman, Gerald Kerma
 */

#ifndef __MVEBU_MMC_H__
#define __MVEBU_MMC_H__

/* needed for the mmc_cfg definition */
#include <mmc.h>

#define MMC_BLOCK_SIZE				512

/*
 * Clock rates
 */

#define MVEBU_MMC_CLOCKRATE_MAX			50000000
#define MVEBU_MMC_BASE_DIV_MAX			0x7ff
#define MVEBU_MMC_BASE_FAST_CLOCK		CONFIG_SYS_TCLK
#define MVEBU_MMC_BASE_FAST_CLK_100		100000000
#define MVEBU_MMC_BASE_FAST_CLK_200		200000000

/* SDIO register */
#define SDIO_SYS_ADDR_LOW			0x000
#define SDIO_SYS_ADDR_HI			0x004
#define SDIO_BLK_SIZE				0x008
#define SDIO_BLK_COUNT				0x00c
#define SDIO_ARG_LOW				0x010
#define SDIO_ARG_HI				0x014
#define SDIO_XFER_MODE				0x018
#define SDIO_CMD				0x01c
#define SDIO_RSP(i)				(0x020 + ((i)<<2))
#define SDIO_RSP0				0x020
#define SDIO_RSP1				0x024
#define SDIO_RSP2				0x028
#define SDIO_RSP3				0x02c
#define SDIO_RSP4				0x030
#define SDIO_RSP5				0x034
#define SDIO_RSP6				0x038
#define SDIO_RSP7				0x03c
#define SDIO_BUF_DATA_PORT			0x040
#define SDIO_RSVED				0x044
#define SDIO_HW_STATE				0x048
#define SDIO_PRESENT_STATE0			0x048
#define SDIO_PRESENT_STATE1			0x04c
#define SDIO_HOST_CTRL				0x050
#define SDIO_BLK_GAP_CTRL			0x054
#define SDIO_CLK_CTRL				0x058
#define SDIO_SW_RESET				0x05c
#define SDIO_NOR_INTR_STATUS			0x060
#define SDIO_ERR_INTR_STATUS			0x064
#define SDIO_NOR_STATUS_EN			0x068
#define SDIO_ERR_STATUS_EN			0x06c
#define SDIO_NOR_INTR_EN			0x070
#define SDIO_ERR_INTR_EN			0x074
#define SDIO_AUTOCMD12_ERR_STATUS		0x078
#define SDIO_CURR_BYTE_LEFT			0x07c
#define SDIO_CURR_BLK_LEFT			0x080
#define SDIO_AUTOCMD12_ARG_LOW			0x084
#define SDIO_AUTOCMD12_ARG_HI			0x088
#define SDIO_AUTOCMD12_INDEX			0x08c
#define SDIO_AUTO_RSP(i)			(0x090 + ((i)<<2))
#define SDIO_AUTO_RSP0				0x090
#define SDIO_AUTO_RSP1				0x094
#define SDIO_AUTO_RSP2				0x098
#define SDIO_CLK_DIV				0x128

#define WINDOW_CTRL(i)				(0x108 + ((i) << 3))
#define WINDOW_BASE(i)				(0x10c + ((i) << 3))

/* SDIO_PRESENT_STATE */
#define CARD_BUSY				(1 << 1)
#define CMD_INHIBIT				(1 << 0)
#define CMD_TXACTIVE				(1 << 8)
#define CMD_RXACTIVE				(1 << 9)
#define CMD_FIFO_EMPTY				(1 << 13)
#define CMD_AUTOCMD12ACTIVE			(1 << 14)
#define CMD_BUS_BUSY				(CMD_AUTOCMD12ACTIVE |	\
						CMD_RXACTIVE |	\
						CMD_TXACTIVE |	\
						CMD_INHIBIT |	\
						CARD_BUSY)

/*
 * SDIO_CMD
 */

#define SDIO_CMD_RSP_NONE			(0 << 0)
#define SDIO_CMD_RSP_136			(1 << 0)
#define SDIO_CMD_RSP_48				(2 << 0)
#define SDIO_CMD_RSP_48BUSY			(3 << 0)

#define SDIO_CMD_CHECK_DATACRC16		(1 << 2)
#define SDIO_CMD_CHECK_CMDCRC			(1 << 3)
#define SDIO_CMD_INDX_CHECK			(1 << 4)
#define SDIO_CMD_DATA_PRESENT			(1 << 5)
#define SDIO_UNEXPECTED_RESP			(1 << 7)

#define SDIO_CMD_INDEX(x)			((x) << 8)

/*
 * SDIO_XFER_MODE
 */

#define SDIO_XFER_MODE_STOP_CLK			(1 << 5)
#define SDIO_XFER_MODE_HW_WR_DATA_EN		(1 << 1)
#define SDIO_XFER_MODE_AUTO_CMD12		(1 << 2)
#define SDIO_XFER_MODE_INT_CHK_EN		(1 << 3)
#define SDIO_XFER_MODE_TO_HOST			(1 << 4)
#define SDIO_XFER_MODE_DMA			(0 << 6)

/*
 * SDIO_HOST_CTRL
 */

#define SDIO_HOST_CTRL_PUSH_PULL_EN		(1 << 0)

#define SDIO_HOST_CTRL_CARD_TYPE_MEM_ONLY	(0 << 1)
#define SDIO_HOST_CTRL_CARD_TYPE_IO_ONLY	(1 << 1)
#define SDIO_HOST_CTRL_CARD_TYPE_IO_MEM_COMBO	(2 << 1)
#define SDIO_HOST_CTRL_CARD_TYPE_IO_MMC		(3 << 1)
#define SDIO_HOST_CTRL_CARD_TYPE_MASK		(3 << 1)

#define SDIO_HOST_CTRL_BIG_ENDIAN		(1 << 3)
#define SDIO_HOST_CTRL_LSB_FIRST		(1 << 4)
#define SDIO_HOST_CTRL_DATA_WIDTH_1_BIT		(0 << 9)
#define SDIO_HOST_CTRL_DATA_WIDTH_4_BITS	(1 << 9)
#define SDIO_HOST_CTRL_HI_SPEED_EN		(1 << 10)

#define SDIO_HOST_CTRL_TMOUT_MAX		0xf
#define SDIO_HOST_CTRL_TMOUT_MASK		(0xf << 11)
#define SDIO_HOST_CTRL_TMOUT(x)			((x) << 11)
#define SDIO_HOST_CTRL_TMOUT_EN			(1 << 15)

/*
 * SDIO_SW_RESET
 */

#define SDIO_SW_RESET_NOW			(1 << 8)

/*
 * Normal interrupt status bits
 */

#define SDIO_NOR_ERROR				(1 << 15)
#define SDIO_NOR_UNEXP_RSP			(1 << 14)
#define SDIO_NOR_AUTOCMD12_DONE			(1 << 13)
#define SDIO_NOR_SUSPEND_ON			(1 << 12)
#define SDIO_NOR_LMB_FF_8W_AVAIL		(1 << 11)
#define SDIO_NOR_LMB_FF_8W_FILLED		(1 << 10)
#define SDIO_NOR_READ_WAIT_ON			(1 << 9)
#define SDIO_NOR_CARD_INT			(1 << 8)
#define SDIO_NOR_READ_READY			(1 << 5)
#define SDIO_NOR_WRITE_READY			(1 << 4)
#define SDIO_NOR_DMA_INI			(1 << 3)
#define SDIO_NOR_BLK_GAP_EVT			(1 << 2)
#define SDIO_NOR_XFER_DONE			(1 << 1)
#define SDIO_NOR_CMD_DONE			(1 << 0)

/*
 * Error status bits
 */

#define SDIO_ERR_CRC_STATUS			(1 << 14)
#define SDIO_ERR_CRC_STARTBIT			(1 << 13)
#define SDIO_ERR_CRC_ENDBIT			(1 << 12)
#define SDIO_ERR_RESP_TBIT			(1 << 11)
#define SDIO_ERR_XFER_SIZE			(1 << 10)
#define SDIO_ERR_CMD_STARTBIT			(1 << 9)
#define SDIO_ERR_AUTOCMD12			(1 << 8)
#define SDIO_ERR_DATA_ENDBIT			(1 << 6)
#define SDIO_ERR_DATA_CRC			(1 << 5)
#define SDIO_ERR_DATA_TIMEOUT			(1 << 4)
#define SDIO_ERR_CMD_INDEX			(1 << 3)
#define SDIO_ERR_CMD_ENDBIT			(1 << 2)
#define SDIO_ERR_CMD_CRC			(1 << 1)
#define SDIO_ERR_CMD_TIMEOUT			(1 << 0)
/* enable all for polling */
#define SDIO_POLL_MASK				0xffff

/*
 * CMD12 error status bits
 */

#define SDIO_AUTOCMD12_ERR_NOTEXE		(1 << 0)
#define SDIO_AUTOCMD12_ERR_TIMEOUT		(1 << 1)
#define SDIO_AUTOCMD12_ERR_CRC			(1 << 2)
#define SDIO_AUTOCMD12_ERR_ENDBIT		(1 << 3)
#define SDIO_AUTOCMD12_ERR_INDEX		(1 << 4)
#define SDIO_AUTOCMD12_ERR_RESP_T_BIT		(1 << 5)
#define SDIO_AUTOCMD12_ERR_RESP_STARTBIT	(1 << 6)

#define MMC_RSP_PRESENT				(1 << 0)
/* 136 bit response */
#define MMC_RSP_136				(1 << 1)
/* expect valid crc */
#define MMC_RSP_CRC				(1 << 2)
/* card may send busy */
#define MMC_RSP_BUSY				(1 << 3)
/* response contains opcode */
#define MMC_RSP_OPCODE				(1 << 4)

#define MMC_BUSMODE_OPENDRAIN			1
#define MMC_BUSMODE_PUSHPULL			2

#define MMC_BUS_WIDTH_1				0
#define MMC_BUS_WIDTH_4				2
#define MMC_BUS_WIDTH_8				3

/* Can the host do 4 bit transfers */
#define MMC_CAP_4_BIT_DATA			(1 << 0)
/* Can do MMC high-speed timing */
#define MMC_CAP_MMC_HIGHSPEED			(1 << 1)
/* Can do SD high-speed timing */
#define MMC_CAP_SD_HIGHSPEED			(1 << 2)
/* Can signal pending SDIO IRQs */
#define MMC_CAP_SDIO_IRQ			(1 << 3)
/* Talks only SPI protocols */
#define MMC_CAP_SPI				(1 << 4)
/* Needs polling for card-detection */
#define MMC_CAP_NEEDS_POLL			(1 << 5)
/* Can the host do 8 bit transfers */
#define MMC_CAP_8_BIT_DATA			(1 << 6)

/* Nonremovable e.g. eMMC */
#define MMC_CAP_NONREMOVABLE			(1 << 8)
/* Waits while card is busy */
#define MMC_CAP_WAIT_WHILE_BUSY			(1 << 9)
/* Allow erase/trim commands */
#define MMC_CAP_ERASE				(1 << 10)
/* can support DDR mode at 1.8V */
#define MMC_CAP_1_8V_DDR			(1 << 11)
/* can support DDR mode at 1.2V */
#define MMC_CAP_1_2V_DDR			(1 << 12)
/* Can power off after boot */
#define MMC_CAP_POWER_OFF_CARD			(1 << 13)
/* CMD14/CMD19 bus width ok */
#define MMC_CAP_BUS_WIDTH_TEST			(1 << 14)
/* Host supports UHS SDR12 mode */
#define MMC_CAP_UHS_SDR12			(1 << 15)
/* Host supports UHS SDR25 mode */
#define MMC_CAP_UHS_SDR25			(1 << 16)
/* Host supports UHS SDR50 mode */
#define MMC_CAP_UHS_SDR50			(1 << 17)
/* Host supports UHS SDR104 mode */
#define MMC_CAP_UHS_SDR104			(1 << 18)
/* Host supports UHS DDR50 mode */
#define MMC_CAP_UHS_DDR50			(1 << 19)
/* Host supports Driver Type A */
#define MMC_CAP_DRIVER_TYPE_A			(1 << 23)
/* Host supports Driver Type C */
#define MMC_CAP_DRIVER_TYPE_C			(1 << 24)
/* Host supports Driver Type D */
#define MMC_CAP_DRIVER_TYPE_D			(1 << 25)
/* CMD23 supported. */
#define MMC_CAP_CMD23				(1 << 30)
/* Hardware reset */
#define MMC_CAP_HW_RESET			(1 << 31)

struct mvebu_mmc_cfg {
	u32	mvebu_mmc_base;
	u32	mvebu_mmc_clk;
	u8	max_bus_width;
	struct mmc_config cfg;
};

/*
 * Functions prototypes
 */

int mvebu_mmc_init(bd_t *bis);

#endif /* __MVEBU_MMC_H__ */
