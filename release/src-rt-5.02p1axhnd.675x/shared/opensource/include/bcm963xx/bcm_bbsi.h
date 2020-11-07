
#if !defined(_BCM_BBSI_H_)
#define _BCM_BBSI_H_

/****************************/
/********** Macros **********/
/****************************/
#define BBSI_CMD_READ				0x80
#define BBSI_CMD_WRITE				0x81

#define BBSI_STATUS_REG_ADDR			6
#define BBSI_STATUS_CPURUNNING_SHIFT		0x6
#define BBSI_STATUS_CPURUNNING_MASK		0x1
#define BBSI_STATUS_HABREQ_SHIFT		0x5
#define BBSI_STATUS_HABREQ_MASK			0x1
#define BBSI_STATUS_BUSY_SHIFT			0x4
#define BBSI_STATUS_BUSY_MASK			0x1
#define BBSI_STATUS_RBUS_UNEXPTX_SHIFT		0x3
#define BBSI_STATUS_RBUS_UNEXPTX_MASK		0x1
#define BBSI_STATUS_RBUS_TIMEOUT_SHIFT		0x2
#define BBSI_STATUS_RBUS_TIMEOUT_MASK		0x1
#define BBSI_STATUS_ERROR_SHIFT			0x0
#define BBSI_STATUS_ERROR_MASK			0x1
#define BBSI_STATUS_FAILED			0xf

#define BBSI_CONFIG_REG_ADDR			7
#define BBSI_CONFIG_XFER_MODE_SHIFT		0x3
#define BBSI_CONFIG_XFER_MODE_MASK		0x3
#define BBSI_CONFIG_NO_RBUS_ADDR_INC_SHIFT	0x2
#define BBSI_CONFIG_NO_RBUS_ADDR_INC_MASK	0x1
#define BBSI_CONFIG_SPEC_READ_SHIFT		0x1
#define BBSI_CONFIG_SPEC_READ_MASK		0x1
#define BBSI_CONFIG_READ_RBUS_SHIFT		0x0
#define BBSI_CONFIG_READ_RBUS_MASK		0x1

#define BBSI_DATA0_REG_ADDR			0xc

#define BSSI_STATUS_RETRY 5

struct bbsi_t
{
   uint16_t spi_clk;
   uint16_t spi_cs;
   uint16_t spi_miso;
   uint16_t spi_mosi;
};

int is_bbsi_done(struct bbsi_t *bbsi);
int bbsi_read(struct bbsi_t *bbsi, uint32_t addr, uint32_t readlen, uint32_t *data);
int bbsi_write(struct bbsi_t *bbsi, uint32 addr, uint32 writelen, uint32 data);
void bbsi_init(struct bbsi_t *bbsi);


#endif  /* _BCM_BBSI_H_ */


