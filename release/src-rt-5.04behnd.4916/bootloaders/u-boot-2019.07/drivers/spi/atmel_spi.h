/*
 * Register definitions for the Atmel AT32/AT91 SPI Controller
 */

/* Register offsets */
#define ATMEL_SPI_CR			0x0000
#define ATMEL_SPI_MR			0x0004
#define ATMEL_SPI_RDR			0x0008
#define ATMEL_SPI_TDR			0x000c
#define ATMEL_SPI_SR			0x0010
#define ATMEL_SPI_IER			0x0014
#define ATMEL_SPI_IDR			0x0018
#define ATMEL_SPI_IMR			0x001c
#define ATMEL_SPI_CSR(x)		(0x0030 + 4 * (x))
#define ATMEL_SPI_VERSION		0x00fc

/* Bits in CR */
#define ATMEL_SPI_CR_SPIEN		BIT(0)
#define ATMEL_SPI_CR_SPIDIS		BIT(1)
#define ATMEL_SPI_CR_SWRST		BIT(7)
#define ATMEL_SPI_CR_LASTXFER		BIT(24)

/* Bits in MR */
#define ATMEL_SPI_MR_MSTR		BIT(0)
#define ATMEL_SPI_MR_PS			BIT(1)
#define ATMEL_SPI_MR_PCSDEC		BIT(2)
#define ATMEL_SPI_MR_FDIV		BIT(3)
#define ATMEL_SPI_MR_MODFDIS		BIT(4)
#define ATMEL_SPI_MR_WDRBT		BIT(5)
#define ATMEL_SPI_MR_LLB		BIT(7)
#define ATMEL_SPI_MR_PCS(x)		(((x) & 15) << 16)
#define ATMEL_SPI_MR_DLYBCS(x)		((x) << 24)

/* Bits in RDR */
#define ATMEL_SPI_RDR_RD(x)		(x)
#define ATMEL_SPI_RDR_PCS(x)		((x) << 16)

/* Bits in TDR */
#define ATMEL_SPI_TDR_TD(x)		(x)
#define ATMEL_SPI_TDR_PCS(x)		((x) << 16)
#define ATMEL_SPI_TDR_LASTXFER		BIT(24)

/* Bits in SR/IER/IDR/IMR */
#define ATMEL_SPI_SR_RDRF		BIT(0)
#define ATMEL_SPI_SR_TDRE		BIT(1)
#define ATMEL_SPI_SR_MODF		BIT(2)
#define ATMEL_SPI_SR_OVRES		BIT(3)
#define ATMEL_SPI_SR_ENDRX		BIT(4)
#define ATMEL_SPI_SR_ENDTX		BIT(5)
#define ATMEL_SPI_SR_RXBUFF		BIT(6)
#define ATMEL_SPI_SR_TXBUFE		BIT(7)
#define ATMEL_SPI_SR_NSSR		BIT(8)
#define ATMEL_SPI_SR_TXEMPTY		BIT(9)
#define ATMEL_SPI_SR_SPIENS		BIT(16)

/* Bits in CSRx */
#define ATMEL_SPI_CSRx_CPOL		BIT(0)
#define ATMEL_SPI_CSRx_NCPHA		BIT(1)
#define ATMEL_SPI_CSRx_CSAAT		BIT(3)
#define ATMEL_SPI_CSRx_BITS(x)		((x) << 4)
#define ATMEL_SPI_CSRx_SCBR(x)		((x) << 8)
#define ATMEL_SPI_CSRx_SCBR_MAX		GENMASK(7, 0)
#define ATMEL_SPI_CSRx_DLYBS(x)		((x) << 16)
#define ATMEL_SPI_CSRx_DLYBCT(x)	((x) << 24)

/* Bits in VERSION */
#define ATMEL_SPI_VERSION_REV(x)	((x) & 0xfff)
#define ATMEL_SPI_VERSION_MFN(x)	((x) << 16)

/* Constants for CSRx:BITS */
#define ATMEL_SPI_BITS_8		0
#define ATMEL_SPI_BITS_9		1
#define ATMEL_SPI_BITS_10		2
#define ATMEL_SPI_BITS_11		3
#define ATMEL_SPI_BITS_12		4
#define ATMEL_SPI_BITS_13		5
#define ATMEL_SPI_BITS_14		6
#define ATMEL_SPI_BITS_15		7
#define ATMEL_SPI_BITS_16		8

struct atmel_spi_slave {
	struct spi_slave slave;
	void		*regs;
	u32		mr;
};

static inline struct atmel_spi_slave *to_atmel_spi(struct spi_slave *slave)
{
	return container_of(slave, struct atmel_spi_slave, slave);
}

/* Register access macros */
#define spi_readl(as, reg)					\
	readl(as->regs + ATMEL_SPI_##reg)
#define spi_writel(as, reg, value)				\
	writel(value, as->regs + ATMEL_SPI_##reg)

#if !defined(CONFIG_SYS_SPI_WRITE_TOUT)
#define CONFIG_SYS_SPI_WRITE_TOUT	(5 * CONFIG_SYS_HZ)
#endif
