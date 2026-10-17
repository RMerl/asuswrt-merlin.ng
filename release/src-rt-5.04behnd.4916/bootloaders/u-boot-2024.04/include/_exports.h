/*
 * You need to use #ifdef around functions that may not exist
 * in the final configuration (such as i2c).
 * use a dummyfunction as first parameter to EXPORT_FUNC.
 * As an example see the CONFIG_CMD_I2C section below
 */
#ifndef EXPORT_FUNC
#define EXPORT_FUNC(a, b, c, ...)
#endif
	EXPORT_FUNC(get_version, unsigned long, get_version, void)
	EXPORT_FUNC(getchar, int, getc, void)
	EXPORT_FUNC(tstc, int, tstc, void)
	EXPORT_FUNC(putc, void, putc, const char)
	EXPORT_FUNC(puts, void, puts, const char *)
#ifdef CONFIG_CONSOLE_FLUSH_SUPPORT
	EXPORT_FUNC(flush, void, flush, void)
#endif
	EXPORT_FUNC(printf, int, printf, const char*, ...)
#if (defined(CONFIG_X86) && !defined(CONFIG_X86_64)) || defined(CONFIG_PPC)
	EXPORT_FUNC(irq_install_handler, void, install_hdlr,
		    int, interrupt_handler_t, void*)

	EXPORT_FUNC(irq_free_handler, void, free_hdlr, int)
#else
	EXPORT_FUNC(dummy, void, install_hdlr, void)
	EXPORT_FUNC(dummy, void, free_hdlr, void)
#endif
	EXPORT_FUNC(malloc, void *, malloc, size_t)
#if !CONFIG_IS_ENABLED(SYS_MALLOC_SIMPLE)
	EXPORT_FUNC(free, void, free, void *)
#endif
	EXPORT_FUNC(udelay, void, udelay, unsigned long)
	EXPORT_FUNC(get_timer, unsigned long, get_timer, unsigned long)
	EXPORT_FUNC(vprintf, int, vprintf, const char *, va_list)
	EXPORT_FUNC(do_reset, int, do_reset, struct cmd_tbl *,
		    int , int , char * const [])
	EXPORT_FUNC(env_get, char  *, env_get, const char*)
	EXPORT_FUNC(env_set, int, env_set, const char *, const char *)
	EXPORT_FUNC(simple_strtoul, unsigned long, simple_strtoul,
		    const char *, char **, unsigned int)
	EXPORT_FUNC(strict_strtoul, int, strict_strtoul,
		    const char *, unsigned int , unsigned long *)
	EXPORT_FUNC(simple_strtol, long, simple_strtol,
		    const char *, char **, unsigned int)
	EXPORT_FUNC(strcmp, int, strcmp, const char *cs, const char *ct)
#if defined(CONFIG_CMD_I2C) && !CONFIG_IS_ENABLED(DM_I2C)
	EXPORT_FUNC(i2c_write, int, i2c_write, uchar, uint, int , uchar * , int)
	EXPORT_FUNC(i2c_read, int, i2c_read, uchar, uint, int , uchar * , int)
#else
	EXPORT_FUNC(dummy, void, i2c_write, void)
	EXPORT_FUNC(dummy, void, i2c_read, void)
#endif

#if !defined(CONFIG_CMD_SPI) || defined(CONFIG_DM_SPI)
	EXPORT_FUNC(dummy, void, spi_setup_slave, void)
	EXPORT_FUNC(dummy, void, spi_free_slave, void)
#else
	EXPORT_FUNC(spi_setup_slave, struct spi_slave *, spi_setup_slave,
		    unsigned int, unsigned int, unsigned int, unsigned int)
	EXPORT_FUNC(spi_free_slave, void, spi_free_slave, struct spi_slave *)
#endif
#ifndef CONFIG_CMD_SPI
	EXPORT_FUNC(dummy, void, spi_claim_bus, void)
	EXPORT_FUNC(dummy, void, spi_release_bus, void)
	EXPORT_FUNC(dummy, void, spi_xfer, void)
#else
	EXPORT_FUNC(spi_claim_bus, int, spi_claim_bus, struct spi_slave *)
	EXPORT_FUNC(spi_release_bus, void, spi_release_bus, struct spi_slave *)
	EXPORT_FUNC(spi_xfer, int, spi_xfer, struct spi_slave *,
		    unsigned int, const void *, void *, unsigned long)
#endif
	EXPORT_FUNC(ustrtoul, unsigned long, ustrtoul,
		    const char *, char **, unsigned int)
	EXPORT_FUNC(ustrtoull, unsigned long long, ustrtoull,
		    const char *, char **, unsigned int)
	EXPORT_FUNC(strcpy, char *, strcpy, char *dest, const char *src)
	EXPORT_FUNC(mdelay, void, mdelay, unsigned long msec)
	EXPORT_FUNC(memset, void *, memset, void *, int, size_t)
#ifdef CONFIG_PHY_AQUANTIA
	EXPORT_FUNC(mdio_get_current_dev, struct mii_dev *,
		    mdio_get_current_dev, void)
	EXPORT_FUNC(phy_find_by_mask, struct phy_device *, phy_find_by_mask,
		    struct mii_dev *bus, unsigned phy_mask)
	EXPORT_FUNC(mdio_phydev_for_ethname, struct phy_device *,
		    mdio_phydev_for_ethname, const char *ethname)
	EXPORT_FUNC(miiphy_set_current_dev, int, miiphy_set_current_dev,
		    const char *devname)
#endif
