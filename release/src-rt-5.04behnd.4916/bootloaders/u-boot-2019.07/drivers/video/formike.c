// SPDX-License-Identifier: GPL-2.0
/*
 * LCD: Formike, TFT 4.3", 480x800, RGB24, KWH043ST20-F01, DriverIC NT35510-16
 * LCD initialization via SPI
 * Based on:
 *
 */
#include <common.h>
#include <errno.h>
#include <spi.h>

#define TAG_READ	0x80
#define TAG_WRITE	0x00

#define TAG_DATA	0x40
#define TAG_COMMAND	0x00

#define TAG_ADDR_H	0x20
#define TAG_ADDR_L	0x00

static int spi_write_tag_val(struct spi_slave *spi, unsigned char tag,
			     unsigned char val)
{
	unsigned long flags = SPI_XFER_BEGIN;
	u8 buf[2];
	int ret;

	buf[0] = tag;
	ret = spi_xfer(spi, 8, buf, NULL, flags);
	buf[0] = val;
	flags = SPI_XFER_END;
	ret = spi_xfer(spi, 8, buf, NULL, flags);

#ifdef KWH043ST20_F01_SPI_DEBUG
	printf("spi_write_tag_val: tag=%02X, val=%02X ret: %d\n",
	       tag, val, ret);
#endif /* KWH043ST20_F01_SPI_DEBUG */
	if (ret)
		debug("%s: Failed to send: %d\n", __func__, ret);

	return ret;
}

static void spi_write_dat(struct spi_slave *spi, unsigned int val)
{
	spi_write_tag_val(spi, TAG_WRITE|TAG_DATA, val);
}

static void spi_write_com(struct spi_slave *spi, unsigned int addr)
{
	spi_write_tag_val(spi, TAG_WRITE|TAG_COMMAND|TAG_ADDR_H,
			  (addr & 0xff00) >> 8);
	spi_write_tag_val(spi, TAG_WRITE|TAG_COMMAND|TAG_ADDR_L,
			  (addr & 0x00ff) >> 0);
}

int kwh043st20_f01_spi_startup(unsigned int bus, unsigned int cs,
	unsigned int max_hz, unsigned int spi_mode)
{
	struct spi_slave *spi;
	int ret;

	spi = spi_setup_slave(bus, cs, max_hz, spi_mode);
	if (!spi) {
		debug("%s: Failed to set up slave\n", __func__);
		return -1;
	}

	ret = spi_claim_bus(spi);
	if (ret) {
		debug("%s: Failed to claim SPI bus: %d\n", __func__, ret);
		goto err_claim_bus;
	}


	/* LV2 Page 1 enable */
	spi_write_com(spi, 0xF000);	spi_write_dat(spi, 0x55);
	spi_write_com(spi, 0xF001);	spi_write_dat(spi, 0xAA);
	spi_write_com(spi, 0xF002);	spi_write_dat(spi, 0x52);
	spi_write_com(spi, 0xF003);	spi_write_dat(spi, 0x08);
	spi_write_com(spi, 0xF004);	spi_write_dat(spi, 0x01);

	/* AVDD Set AVDD 5.2V */
	spi_write_com(spi, 0xB000);	spi_write_dat(spi, 0x0D);
	spi_write_com(spi, 0xB001);	spi_write_dat(spi, 0x0D);
	spi_write_com(spi, 0xB002);	spi_write_dat(spi, 0x0D);

	/* AVDD ratio */
	spi_write_com(spi, 0xB600);	spi_write_dat(spi, 0x34);
	spi_write_com(spi, 0xB601);	spi_write_dat(spi, 0x34);
	spi_write_com(spi, 0xB602);	spi_write_dat(spi, 0x34);

	/* AVEE  -5.2V */
	spi_write_com(spi, 0xB100);	spi_write_dat(spi, 0x0D);
	spi_write_com(spi, 0xB101);	spi_write_dat(spi, 0x0D);
	spi_write_com(spi, 0xB102);	spi_write_dat(spi, 0x0D);

	/* AVEE ratio */
	spi_write_com(spi, 0xB700);	spi_write_dat(spi, 0x35);
	spi_write_com(spi, 0xB701);	spi_write_dat(spi, 0x35);
	spi_write_com(spi, 0xB702);	spi_write_dat(spi, 0x35);

	/* VCL  -2.5V */
	spi_write_com(spi, 0xB200);	spi_write_dat(spi, 0x00);
	spi_write_com(spi, 0xB201);	spi_write_dat(spi, 0x00);
	spi_write_com(spi, 0xB202);	spi_write_dat(spi, 0x00);

	/* VCL ratio */
	spi_write_com(spi, 0xB800);	spi_write_dat(spi, 0x24);
	spi_write_com(spi, 0xB801);	spi_write_dat(spi, 0x24);
	spi_write_com(spi, 0xB802);	spi_write_dat(spi, 0x24);

	/* VGH 15V */
	spi_write_com(spi, 0xBF00);	spi_write_dat(spi, 0x01);
	spi_write_com(spi, 0xB300);	spi_write_dat(spi, 0x08);
	spi_write_com(spi, 0xB301);	spi_write_dat(spi, 0x08);
	spi_write_com(spi, 0xB302);	spi_write_dat(spi, 0x08);

	/* VGH ratio */
	spi_write_com(spi, 0xB900);	spi_write_dat(spi, 0x34);
	spi_write_com(spi, 0xB901);	spi_write_dat(spi, 0x34);
	spi_write_com(spi, 0xB902);	spi_write_dat(spi, 0x34);

	/* VGLX ratio */
	spi_write_com(spi, 0xBA00);	spi_write_dat(spi, 0x24);
	spi_write_com(spi, 0xBA01);	spi_write_dat(spi, 0x24);
	spi_write_com(spi, 0xBA02);	spi_write_dat(spi, 0x24);

	/* VGMP/VGSP 4.7V/0V */
	spi_write_com(spi, 0xBC00);	spi_write_dat(spi, 0x00);
	spi_write_com(spi, 0xBC01);	spi_write_dat(spi, 0x88);
	spi_write_com(spi, 0xBC02);	spi_write_dat(spi, 0x00);

	/* VGMN/VGSN -4.7V/0V */
	spi_write_com(spi, 0xBD00);	spi_write_dat(spi, 0x00);
	spi_write_com(spi, 0xBD01);	spi_write_dat(spi, 0x88);
	spi_write_com(spi, 0xBD02);	spi_write_dat(spi, 0x00);

	/* VCOM 1.525V */
	spi_write_com(spi, 0xBE00);	spi_write_dat(spi, 0x00);
	spi_write_com(spi, 0xBE01);	spi_write_dat(spi, 0x7A);

	/* Gamma Setting */
	spi_write_com(spi, 0xD100);	spi_write_dat(spi, 0x00);
	spi_write_com(spi, 0xD101);	spi_write_dat(spi, 0x05);
	spi_write_com(spi, 0xD102);	spi_write_dat(spi, 0x00);
	spi_write_com(spi, 0xD103);	spi_write_dat(spi, 0x15);
	spi_write_com(spi, 0xD104);	spi_write_dat(spi, 0x00);
	spi_write_com(spi, 0xD105);	spi_write_dat(spi, 0x30);
	spi_write_com(spi, 0xD106);	spi_write_dat(spi, 0x00);
	spi_write_com(spi, 0xD107);	spi_write_dat(spi, 0x47);
	spi_write_com(spi, 0xD108);	spi_write_dat(spi, 0x00);
	spi_write_com(spi, 0xD109);	spi_write_dat(spi, 0x5B);
	spi_write_com(spi, 0xD10A);	spi_write_dat(spi, 0x00);
	spi_write_com(spi, 0xD10B);	spi_write_dat(spi, 0x7D);
	spi_write_com(spi, 0xD10C);	spi_write_dat(spi, 0x00);
	spi_write_com(spi, 0xD10D);	spi_write_dat(spi, 0x9D);
	spi_write_com(spi, 0xD10E);	spi_write_dat(spi, 0x00);
	spi_write_com(spi, 0xD10F);	spi_write_dat(spi, 0xCC);
	spi_write_com(spi, 0xD110);	spi_write_dat(spi, 0x00);
	spi_write_com(spi, 0xD111);	spi_write_dat(spi, 0xF3);
	spi_write_com(spi, 0xD112);	spi_write_dat(spi, 0x01);
	spi_write_com(spi, 0xD113);	spi_write_dat(spi, 0x32);
	spi_write_com(spi, 0xD114);	spi_write_dat(spi, 0x01);
	spi_write_com(spi, 0xD115);	spi_write_dat(spi, 0x63);
	spi_write_com(spi, 0xD116);	spi_write_dat(spi, 0x01);
	spi_write_com(spi, 0xD117);	spi_write_dat(spi, 0xB1);
	spi_write_com(spi, 0xD118);	spi_write_dat(spi, 0x01);
	spi_write_com(spi, 0xD119);	spi_write_dat(spi, 0xF0);
	spi_write_com(spi, 0xD11A);	spi_write_dat(spi, 0x01);
	spi_write_com(spi, 0xD11B);	spi_write_dat(spi, 0xF2);
	spi_write_com(spi, 0xD11C);	spi_write_dat(spi, 0x02);
	spi_write_com(spi, 0xD11D);	spi_write_dat(spi, 0x2A);
	spi_write_com(spi, 0xD11E);	spi_write_dat(spi, 0x02);
	spi_write_com(spi, 0xD11F);	spi_write_dat(spi, 0x67);
	spi_write_com(spi, 0xD120);	spi_write_dat(spi, 0x02);
	spi_write_com(spi, 0xD121);	spi_write_dat(spi, 0x90);
	spi_write_com(spi, 0xD122);	spi_write_dat(spi, 0x02);
	spi_write_com(spi, 0xD123);	spi_write_dat(spi, 0xCB);
	spi_write_com(spi, 0xD124);	spi_write_dat(spi, 0x02);
	spi_write_com(spi, 0xD125);	spi_write_dat(spi, 0xF2);
	spi_write_com(spi, 0xD126);	spi_write_dat(spi, 0x03);
	spi_write_com(spi, 0xD127);	spi_write_dat(spi, 0x2A);
	spi_write_com(spi, 0xD128);	spi_write_dat(spi, 0x03);
	spi_write_com(spi, 0xD129);	spi_write_dat(spi, 0x51);
	spi_write_com(spi, 0xD12A);	spi_write_dat(spi, 0x03);
	spi_write_com(spi, 0xD12B);	spi_write_dat(spi, 0x80);
	spi_write_com(spi, 0xD12C);	spi_write_dat(spi, 0x03);
	spi_write_com(spi, 0xD12D);	spi_write_dat(spi, 0x9F);
	spi_write_com(spi, 0xD12E);	spi_write_dat(spi, 0x03);
	spi_write_com(spi, 0xD12F);	spi_write_dat(spi, 0xBE);
	spi_write_com(spi, 0xD130);	spi_write_dat(spi, 0x03);
	spi_write_com(spi, 0xD131);	spi_write_dat(spi, 0xF9);
	spi_write_com(spi, 0xD132);	spi_write_dat(spi, 0x03);
	spi_write_com(spi, 0xD133);	spi_write_dat(spi, 0xFF);

	spi_write_com(spi, 0xD200);	spi_write_dat(spi, 0x00);
	spi_write_com(spi, 0xD201);	spi_write_dat(spi, 0x05);
	spi_write_com(spi, 0xD202);	spi_write_dat(spi, 0x00);
	spi_write_com(spi, 0xD203);	spi_write_dat(spi, 0x15);
	spi_write_com(spi, 0xD204);	spi_write_dat(spi, 0x00);
	spi_write_com(spi, 0xD205);	spi_write_dat(spi, 0x30);
	spi_write_com(spi, 0xD206);	spi_write_dat(spi, 0x00);
	spi_write_com(spi, 0xD207);	spi_write_dat(spi, 0x47);
	spi_write_com(spi, 0xD208);	spi_write_dat(spi, 0x00);
	spi_write_com(spi, 0xD209);	spi_write_dat(spi, 0x5B);
	spi_write_com(spi, 0xD20A);	spi_write_dat(spi, 0x00);
	spi_write_com(spi, 0xD20B);	spi_write_dat(spi, 0x7D);
	spi_write_com(spi, 0xD20C);	spi_write_dat(spi, 0x00);
	spi_write_com(spi, 0xD20D);	spi_write_dat(spi, 0x9D);
	spi_write_com(spi, 0xD20E);	spi_write_dat(spi, 0x00);
	spi_write_com(spi, 0xD20F);	spi_write_dat(spi, 0xCC);
	spi_write_com(spi, 0xD210);	spi_write_dat(spi, 0x00);
	spi_write_com(spi, 0xD211);	spi_write_dat(spi, 0xF3);
	spi_write_com(spi, 0xD212);	spi_write_dat(spi, 0x01);
	spi_write_com(spi, 0xD213);	spi_write_dat(spi, 0x32);
	spi_write_com(spi, 0xD214);	spi_write_dat(spi, 0x01);
	spi_write_com(spi, 0xD215);	spi_write_dat(spi, 0x63);
	spi_write_com(spi, 0xD216);	spi_write_dat(spi, 0x01);
	spi_write_com(spi, 0xD217);	spi_write_dat(spi, 0xB1);
	spi_write_com(spi, 0xD218);	spi_write_dat(spi, 0x01);
	spi_write_com(spi, 0xD219);	spi_write_dat(spi, 0xF0);
	spi_write_com(spi, 0xD21A);	spi_write_dat(spi, 0x01);
	spi_write_com(spi, 0xD21B);	spi_write_dat(spi, 0xF2);
	spi_write_com(spi, 0xD21C);	spi_write_dat(spi, 0x02);
	spi_write_com(spi, 0xD21D);	spi_write_dat(spi, 0x2A);
	spi_write_com(spi, 0xD21E);	spi_write_dat(spi, 0x02);
	spi_write_com(spi, 0xD21F);	spi_write_dat(spi, 0x67);
	spi_write_com(spi, 0xD220);	spi_write_dat(spi, 0x02);
	spi_write_com(spi, 0xD221);	spi_write_dat(spi, 0x90);
	spi_write_com(spi, 0xD222);	spi_write_dat(spi, 0x02);
	spi_write_com(spi, 0xD223);	spi_write_dat(spi, 0xCB);
	spi_write_com(spi, 0xD224);	spi_write_dat(spi, 0x02);
	spi_write_com(spi, 0xD225);	spi_write_dat(spi, 0xF2);
	spi_write_com(spi, 0xD226);	spi_write_dat(spi, 0x03);
	spi_write_com(spi, 0xD227);	spi_write_dat(spi, 0x2A);
	spi_write_com(spi, 0xD228);	spi_write_dat(spi, 0x03);
	spi_write_com(spi, 0xD229);	spi_write_dat(spi, 0x51);
	spi_write_com(spi, 0xD22A);	spi_write_dat(spi, 0x03);
	spi_write_com(spi, 0xD22B);	spi_write_dat(spi, 0x80);
	spi_write_com(spi, 0xD22C);	spi_write_dat(spi, 0x03);
	spi_write_com(spi, 0xD22D);	spi_write_dat(spi, 0x9F);
	spi_write_com(spi, 0xD22E);	spi_write_dat(spi, 0x03);
	spi_write_com(spi, 0xD22F);	spi_write_dat(spi, 0xBE);
	spi_write_com(spi, 0xD230);	spi_write_dat(spi, 0x03);
	spi_write_com(spi, 0xD231);	spi_write_dat(spi, 0xF9);
	spi_write_com(spi, 0xD232);	spi_write_dat(spi, 0x03);
	spi_write_com(spi, 0xD233);	spi_write_dat(spi, 0xFF);

	spi_write_com(spi, 0xD300);	spi_write_dat(spi, 0x00);
	spi_write_com(spi, 0xD301);	spi_write_dat(spi, 0x05);
	spi_write_com(spi, 0xD302);	spi_write_dat(spi, 0x00);
	spi_write_com(spi, 0xD303);	spi_write_dat(spi, 0x15);
	spi_write_com(spi, 0xD304);	spi_write_dat(spi, 0x00);
	spi_write_com(spi, 0xD305);	spi_write_dat(spi, 0x30);
	spi_write_com(spi, 0xD306);	spi_write_dat(spi, 0x00);
	spi_write_com(spi, 0xD307);	spi_write_dat(spi, 0x47);
	spi_write_com(spi, 0xD308);	spi_write_dat(spi, 0x00);
	spi_write_com(spi, 0xD309);	spi_write_dat(spi, 0x5B);
	spi_write_com(spi, 0xD30A);	spi_write_dat(spi, 0x00);
	spi_write_com(spi, 0xD30B);	spi_write_dat(spi, 0x7D);
	spi_write_com(spi, 0xD30C);	spi_write_dat(spi, 0x00);
	spi_write_com(spi, 0xD30D);	spi_write_dat(spi, 0x9D);
	spi_write_com(spi, 0xD30E);	spi_write_dat(spi, 0x00);
	spi_write_com(spi, 0xD30F);	spi_write_dat(spi, 0xCC);
	spi_write_com(spi, 0xD310);	spi_write_dat(spi, 0x00);
	spi_write_com(spi, 0xD311);	spi_write_dat(spi, 0xF3);
	spi_write_com(spi, 0xD312);	spi_write_dat(spi, 0x01);
	spi_write_com(spi, 0xD313);	spi_write_dat(spi, 0x32);
	spi_write_com(spi, 0xD314);	spi_write_dat(spi, 0x01);
	spi_write_com(spi, 0xD315);	spi_write_dat(spi, 0x63);
	spi_write_com(spi, 0xD316);	spi_write_dat(spi, 0x01);
	spi_write_com(spi, 0xD317);	spi_write_dat(spi, 0xB1);
	spi_write_com(spi, 0xD318);	spi_write_dat(spi, 0x01);
	spi_write_com(spi, 0xD319);	spi_write_dat(spi, 0xF0);
	spi_write_com(spi, 0xD31A);	spi_write_dat(spi, 0x01);
	spi_write_com(spi, 0xD31B);	spi_write_dat(spi, 0xF2);
	spi_write_com(spi, 0xD31C);	spi_write_dat(spi, 0x02);
	spi_write_com(spi, 0xD31D);	spi_write_dat(spi, 0x2A);
	spi_write_com(spi, 0xD31E);	spi_write_dat(spi, 0x02);
	spi_write_com(spi, 0xD31F);	spi_write_dat(spi, 0x67);
	spi_write_com(spi, 0xD320);	spi_write_dat(spi, 0x02);
	spi_write_com(spi, 0xD321);	spi_write_dat(spi, 0x90);
	spi_write_com(spi, 0xD322);	spi_write_dat(spi, 0x02);
	spi_write_com(spi, 0xD323);	spi_write_dat(spi, 0xCB);
	spi_write_com(spi, 0xD324);	spi_write_dat(spi, 0x02);
	spi_write_com(spi, 0xD325);	spi_write_dat(spi, 0xF2);
	spi_write_com(spi, 0xD326);	spi_write_dat(spi, 0x03);
	spi_write_com(spi, 0xD327);	spi_write_dat(spi, 0x2A);
	spi_write_com(spi, 0xD328);	spi_write_dat(spi, 0x03);
	spi_write_com(spi, 0xD329);	spi_write_dat(spi, 0x51);
	spi_write_com(spi, 0xD32A);	spi_write_dat(spi, 0x03);
	spi_write_com(spi, 0xD32B);	spi_write_dat(spi, 0x80);
	spi_write_com(spi, 0xD32C);	spi_write_dat(spi, 0x03);
	spi_write_com(spi, 0xD32D);	spi_write_dat(spi, 0x9F);
	spi_write_com(spi, 0xD32E);	spi_write_dat(spi, 0x03);
	spi_write_com(spi, 0xD32F);	spi_write_dat(spi, 0xBE);
	spi_write_com(spi, 0xD330);	spi_write_dat(spi, 0x03);
	spi_write_com(spi, 0xD331);	spi_write_dat(spi, 0xF9);
	spi_write_com(spi, 0xD332);	spi_write_dat(spi, 0x03);
	spi_write_com(spi, 0xD333);	spi_write_dat(spi, 0xFF);

	spi_write_com(spi, 0xD400);	spi_write_dat(spi, 0x00);
	spi_write_com(spi, 0xD401);	spi_write_dat(spi, 0x05);
	spi_write_com(spi, 0xD402);	spi_write_dat(spi, 0x00);
	spi_write_com(spi, 0xD403);	spi_write_dat(spi, 0x15);
	spi_write_com(spi, 0xD404);	spi_write_dat(spi, 0x00);
	spi_write_com(spi, 0xD405);	spi_write_dat(spi, 0x30);
	spi_write_com(spi, 0xD406);	spi_write_dat(spi, 0x00);
	spi_write_com(spi, 0xD407);	spi_write_dat(spi, 0x47);
	spi_write_com(spi, 0xD408);	spi_write_dat(spi, 0x00);
	spi_write_com(spi, 0xD409);	spi_write_dat(spi, 0x5B);
	spi_write_com(spi, 0xD40A);	spi_write_dat(spi, 0x00);
	spi_write_com(spi, 0xD40B);	spi_write_dat(spi, 0x7D);
	spi_write_com(spi, 0xD40C);	spi_write_dat(spi, 0x00);
	spi_write_com(spi, 0xD40D);	spi_write_dat(spi, 0x9D);
	spi_write_com(spi, 0xD40E);	spi_write_dat(spi, 0x00);
	spi_write_com(spi, 0xD40F);	spi_write_dat(spi, 0xCC);
	spi_write_com(spi, 0xD410);	spi_write_dat(spi, 0x00);
	spi_write_com(spi, 0xD411);	spi_write_dat(spi, 0xF3);
	spi_write_com(spi, 0xD412);	spi_write_dat(spi, 0x01);
	spi_write_com(spi, 0xD413);	spi_write_dat(spi, 0x32);
	spi_write_com(spi, 0xD414);	spi_write_dat(spi, 0x01);
	spi_write_com(spi, 0xD415);	spi_write_dat(spi, 0x63);
	spi_write_com(spi, 0xD416);	spi_write_dat(spi, 0x01);
	spi_write_com(spi, 0xD417);	spi_write_dat(spi, 0xB1);
	spi_write_com(spi, 0xD418);	spi_write_dat(spi, 0x01);
	spi_write_com(spi, 0xD419);	spi_write_dat(spi, 0xF0);
	spi_write_com(spi, 0xD41A);	spi_write_dat(spi, 0x01);
	spi_write_com(spi, 0xD41B);	spi_write_dat(spi, 0xF2);
	spi_write_com(spi, 0xD41C);	spi_write_dat(spi, 0x02);
	spi_write_com(spi, 0xD41D);	spi_write_dat(spi, 0x2A);
	spi_write_com(spi, 0xD41E);	spi_write_dat(spi, 0x02);
	spi_write_com(spi, 0xD41F);	spi_write_dat(spi, 0x67);
	spi_write_com(spi, 0xD420);	spi_write_dat(spi, 0x02);
	spi_write_com(spi, 0xD421);	spi_write_dat(spi, 0x90);
	spi_write_com(spi, 0xD422);	spi_write_dat(spi, 0x02);
	spi_write_com(spi, 0xD423);	spi_write_dat(spi, 0xCB);
	spi_write_com(spi, 0xD424);	spi_write_dat(spi, 0x02);
	spi_write_com(spi, 0xD425);	spi_write_dat(spi, 0xF2);
	spi_write_com(spi, 0xD426);	spi_write_dat(spi, 0x03);
	spi_write_com(spi, 0xD427);	spi_write_dat(spi, 0x2A);
	spi_write_com(spi, 0xD428);	spi_write_dat(spi, 0x03);
	spi_write_com(spi, 0xD429);	spi_write_dat(spi, 0x51);
	spi_write_com(spi, 0xD42A);	spi_write_dat(spi, 0x03);
	spi_write_com(spi, 0xD42B);	spi_write_dat(spi, 0x80);
	spi_write_com(spi, 0xD42C);	spi_write_dat(spi, 0x03);
	spi_write_com(spi, 0xD42D);	spi_write_dat(spi, 0x9F);
	spi_write_com(spi, 0xD42E);	spi_write_dat(spi, 0x03);
	spi_write_com(spi, 0xD42F);	spi_write_dat(spi, 0xBE);
	spi_write_com(spi, 0xD430);	spi_write_dat(spi, 0x03);
	spi_write_com(spi, 0xD431);	spi_write_dat(spi, 0xF9);
	spi_write_com(spi, 0xD432);	spi_write_dat(spi, 0x03);
	spi_write_com(spi, 0xD433);	spi_write_dat(spi, 0xFF);

	spi_write_com(spi, 0xD500);	spi_write_dat(spi, 0x00);
	spi_write_com(spi, 0xD501);	spi_write_dat(spi, 0x05);
	spi_write_com(spi, 0xD502);	spi_write_dat(spi, 0x00);
	spi_write_com(spi, 0xD503);	spi_write_dat(spi, 0x15);
	spi_write_com(spi, 0xD504);	spi_write_dat(spi, 0x00);
	spi_write_com(spi, 0xD505);	spi_write_dat(spi, 0x30);
	spi_write_com(spi, 0xD506);	spi_write_dat(spi, 0x00);
	spi_write_com(spi, 0xD507);	spi_write_dat(spi, 0x47);
	spi_write_com(spi, 0xD508);	spi_write_dat(spi, 0x00);
	spi_write_com(spi, 0xD509);	spi_write_dat(spi, 0x5B);
	spi_write_com(spi, 0xD50A);	spi_write_dat(spi, 0x00);
	spi_write_com(spi, 0xD50B);	spi_write_dat(spi, 0x7D);
	spi_write_com(spi, 0xD50C);	spi_write_dat(spi, 0x00);
	spi_write_com(spi, 0xD50D);	spi_write_dat(spi, 0x9D);
	spi_write_com(spi, 0xD50E);	spi_write_dat(spi, 0x00);
	spi_write_com(spi, 0xD50F);	spi_write_dat(spi, 0xCC);
	spi_write_com(spi, 0xD510);	spi_write_dat(spi, 0x00);
	spi_write_com(spi, 0xD511);	spi_write_dat(spi, 0xF3);
	spi_write_com(spi, 0xD512);	spi_write_dat(spi, 0x01);
	spi_write_com(spi, 0xD513);	spi_write_dat(spi, 0x32);
	spi_write_com(spi, 0xD514);	spi_write_dat(spi, 0x01);
	spi_write_com(spi, 0xD515);	spi_write_dat(spi, 0x63);
	spi_write_com(spi, 0xD516);	spi_write_dat(spi, 0x01);
	spi_write_com(spi, 0xD517);	spi_write_dat(spi, 0xB1);
	spi_write_com(spi, 0xD518);	spi_write_dat(spi, 0x01);
	spi_write_com(spi, 0xD519);	spi_write_dat(spi, 0xF0);
	spi_write_com(spi, 0xD51A);	spi_write_dat(spi, 0x01);
	spi_write_com(spi, 0xD51B);	spi_write_dat(spi, 0xF2);
	spi_write_com(spi, 0xD51C);	spi_write_dat(spi, 0x02);
	spi_write_com(spi, 0xD51D);	spi_write_dat(spi, 0x2A);
	spi_write_com(spi, 0xD51E);	spi_write_dat(spi, 0x02);
	spi_write_com(spi, 0xD51F);	spi_write_dat(spi, 0x67);
	spi_write_com(spi, 0xD520);	spi_write_dat(spi, 0x02);
	spi_write_com(spi, 0xD521);	spi_write_dat(spi, 0x90);
	spi_write_com(spi, 0xD522);	spi_write_dat(spi, 0x02);
	spi_write_com(spi, 0xD523);	spi_write_dat(spi, 0xCB);
	spi_write_com(spi, 0xD524);	spi_write_dat(spi, 0x02);
	spi_write_com(spi, 0xD525);	spi_write_dat(spi, 0xF2);
	spi_write_com(spi, 0xD526);	spi_write_dat(spi, 0x03);
	spi_write_com(spi, 0xD527);	spi_write_dat(spi, 0x2A);
	spi_write_com(spi, 0xD528);	spi_write_dat(spi, 0x03);
	spi_write_com(spi, 0xD529);	spi_write_dat(spi, 0x51);
	spi_write_com(spi, 0xD52A);	spi_write_dat(spi, 0x03);
	spi_write_com(spi, 0xD52B);	spi_write_dat(spi, 0x80);
	spi_write_com(spi, 0xD52C);	spi_write_dat(spi, 0x03);
	spi_write_com(spi, 0xD52D);	spi_write_dat(spi, 0x9F);
	spi_write_com(spi, 0xD52E);	spi_write_dat(spi, 0x03);
	spi_write_com(spi, 0xD52F);	spi_write_dat(spi, 0xBE);
	spi_write_com(spi, 0xD530);	spi_write_dat(spi, 0x03);
	spi_write_com(spi, 0xD531);	spi_write_dat(spi, 0xF9);
	spi_write_com(spi, 0xD532);	spi_write_dat(spi, 0x03);
	spi_write_com(spi, 0xD533);	spi_write_dat(spi, 0xFF);

	spi_write_com(spi, 0xD600);	spi_write_dat(spi, 0x00);
	spi_write_com(spi, 0xD601);	spi_write_dat(spi, 0x05);
	spi_write_com(spi, 0xD602);	spi_write_dat(spi, 0x00);
	spi_write_com(spi, 0xD603);	spi_write_dat(spi, 0x15);
	spi_write_com(spi, 0xD604);	spi_write_dat(spi, 0x00);
	spi_write_com(spi, 0xD605);	spi_write_dat(spi, 0x30);
	spi_write_com(spi, 0xD606);	spi_write_dat(spi, 0x00);
	spi_write_com(spi, 0xD607);	spi_write_dat(spi, 0x47);
	spi_write_com(spi, 0xD608);	spi_write_dat(spi, 0x00);
	spi_write_com(spi, 0xD609);	spi_write_dat(spi, 0x5B);
	spi_write_com(spi, 0xD60A);	spi_write_dat(spi, 0x00);
	spi_write_com(spi, 0xD60B);	spi_write_dat(spi, 0x7D);
	spi_write_com(spi, 0xD60C);	spi_write_dat(spi, 0x00);
	spi_write_com(spi, 0xD60D);	spi_write_dat(spi, 0x9D);
	spi_write_com(spi, 0xD60E);	spi_write_dat(spi, 0x00);
	spi_write_com(spi, 0xD60F);	spi_write_dat(spi, 0xCC);
	spi_write_com(spi, 0xD610);	spi_write_dat(spi, 0x00);
	spi_write_com(spi, 0xD611);	spi_write_dat(spi, 0xF3);
	spi_write_com(spi, 0xD612);	spi_write_dat(spi, 0x01);
	spi_write_com(spi, 0xD613);	spi_write_dat(spi, 0x32);
	spi_write_com(spi, 0xD614);	spi_write_dat(spi, 0x01);
	spi_write_com(spi, 0xD615);	spi_write_dat(spi, 0x63);
	spi_write_com(spi, 0xD616);	spi_write_dat(spi, 0x01);
	spi_write_com(spi, 0xD617);	spi_write_dat(spi, 0xB1);
	spi_write_com(spi, 0xD618);	spi_write_dat(spi, 0x01);
	spi_write_com(spi, 0xD619);	spi_write_dat(spi, 0xF0);
	spi_write_com(spi, 0xD61A);	spi_write_dat(spi, 0x01);
	spi_write_com(spi, 0xD61B);	spi_write_dat(spi, 0xF2);
	spi_write_com(spi, 0xD61C);	spi_write_dat(spi, 0x02);
	spi_write_com(spi, 0xD61D);	spi_write_dat(spi, 0x2A);
	spi_write_com(spi, 0xD61E);	spi_write_dat(spi, 0x02);
	spi_write_com(spi, 0xD61F);	spi_write_dat(spi, 0x67);
	spi_write_com(spi, 0xD620);	spi_write_dat(spi, 0x02);
	spi_write_com(spi, 0xD621);	spi_write_dat(spi, 0x90);
	spi_write_com(spi, 0xD622);	spi_write_dat(spi, 0x02);
	spi_write_com(spi, 0xD623);	spi_write_dat(spi, 0xCB);
	spi_write_com(spi, 0xD624);	spi_write_dat(spi, 0x02);
	spi_write_com(spi, 0xD625);	spi_write_dat(spi, 0xF2);
	spi_write_com(spi, 0xD626);	spi_write_dat(spi, 0x03);
	spi_write_com(spi, 0xD627);	spi_write_dat(spi, 0x2A);
	spi_write_com(spi, 0xD628);	spi_write_dat(spi, 0x03);
	spi_write_com(spi, 0xD629);	spi_write_dat(spi, 0x51);
	spi_write_com(spi, 0xD62A);	spi_write_dat(spi, 0x03);
	spi_write_com(spi, 0xD62B);	spi_write_dat(spi, 0x80);
	spi_write_com(spi, 0xD62C);	spi_write_dat(spi, 0x03);
	spi_write_com(spi, 0xD62D);	spi_write_dat(spi, 0x9F);
	spi_write_com(spi, 0xD62E);	spi_write_dat(spi, 0x03);
	spi_write_com(spi, 0xD62F);	spi_write_dat(spi, 0xBE);
	spi_write_com(spi, 0xD630);	spi_write_dat(spi, 0x03);
	spi_write_com(spi, 0xD631);	spi_write_dat(spi, 0xF9);
	spi_write_com(spi, 0xD632);	spi_write_dat(spi, 0x03);
	spi_write_com(spi, 0xD633);	spi_write_dat(spi, 0xFF);

	/* LV2 Page 0 enable */
	spi_write_com(spi, 0xF000);	spi_write_dat(spi, 0x55);
	spi_write_com(spi, 0xF001);	spi_write_dat(spi, 0xAA);
	spi_write_com(spi, 0xF002);	spi_write_dat(spi, 0x52);
	spi_write_com(spi, 0xF003);	spi_write_dat(spi, 0x08);
	spi_write_com(spi, 0xF004);	spi_write_dat(spi, 0x00);

	/* Display control */
	spi_write_com(spi, 0xB100);	spi_write_dat(spi, 0xFC);
	spi_write_com(spi, 0xB101);	spi_write_dat(spi, 0x00);

	/* Source hold time */
	spi_write_com(spi, 0xB600);	spi_write_dat(spi, 0x05);

	/* Gate EQ control */
	spi_write_com(spi, 0xB700);	spi_write_dat(spi, 0x70);
	spi_write_com(spi, 0xB701);	spi_write_dat(spi, 0x70);

	/* Source EQ control (Mode 2) */
	spi_write_com(spi, 0xB800);	spi_write_dat(spi, 0x01);
	spi_write_com(spi, 0xB801);	spi_write_dat(spi, 0x05);
	spi_write_com(spi, 0xB802);	spi_write_dat(spi, 0x05);
	spi_write_com(spi, 0xB803);	spi_write_dat(spi, 0x05);

	/* Inversion mode  (Column) */
	spi_write_com(spi, 0xBC00);	spi_write_dat(spi, 0x00);
	spi_write_com(spi, 0xBC01);	spi_write_dat(spi, 0x00);
	spi_write_com(spi, 0xBC02);	spi_write_dat(spi, 0x00);

	/* Timing control 8phase dual side/4H/4delay/RST_EN */
	spi_write_com(spi, 0xC900);	spi_write_dat(spi, 0xD0);
	spi_write_com(spi, 0xC901);	spi_write_dat(spi, 0x82);
	spi_write_com(spi, 0xC902);	spi_write_dat(spi, 0x50);
	spi_write_com(spi, 0xC903);	spi_write_dat(spi, 0x50);
	spi_write_com(spi, 0xC904);	spi_write_dat(spi, 0x50);

	spi_write_com(spi, 0x3A00);	spi_write_dat(spi, 0x55);
	mdelay(120);
	spi_write_com(spi, 0x1100);
	mdelay(120);
	spi_write_com(spi, 0x2900);
	mdelay(120);
	/* spi_write_com(spi, 0x2100);	spi_write_dat(spi, 0x00); */
	spi_write_com(spi, 0x2C00);

	return 0;
err_claim_bus:
	spi_free_slave(spi);
	return -1;
}
