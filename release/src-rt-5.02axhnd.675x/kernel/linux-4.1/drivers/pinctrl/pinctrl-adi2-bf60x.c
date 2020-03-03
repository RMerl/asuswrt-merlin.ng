/*
 * Pinctrl Driver for ADI GPIO2 controller
 *
 * Copyright 2007-2013 Analog Devices Inc.
 *
 * Licensed under the GPLv2 or later
 */

#include <asm/portmux.h>
#include "pinctrl-adi2.h"

static const struct pinctrl_pin_desc adi_pads[] = {
	PINCTRL_PIN(0, "PA0"),
	PINCTRL_PIN(1, "PA1"),
	PINCTRL_PIN(2, "PA2"),
	PINCTRL_PIN(3, "PG3"),
	PINCTRL_PIN(4, "PA4"),
	PINCTRL_PIN(5, "PA5"),
	PINCTRL_PIN(6, "PA6"),
	PINCTRL_PIN(7, "PA7"),
	PINCTRL_PIN(8, "PA8"),
	PINCTRL_PIN(9, "PA9"),
	PINCTRL_PIN(10, "PA10"),
	PINCTRL_PIN(11, "PA11"),
	PINCTRL_PIN(12, "PA12"),
	PINCTRL_PIN(13, "PA13"),
	PINCTRL_PIN(14, "PA14"),
	PINCTRL_PIN(15, "PA15"),
	PINCTRL_PIN(16, "PB0"),
	PINCTRL_PIN(17, "PB1"),
	PINCTRL_PIN(18, "PB2"),
	PINCTRL_PIN(19, "PB3"),
	PINCTRL_PIN(20, "PB4"),
	PINCTRL_PIN(21, "PB5"),
	PINCTRL_PIN(22, "PB6"),
	PINCTRL_PIN(23, "PB7"),
	PINCTRL_PIN(24, "PB8"),
	PINCTRL_PIN(25, "PB9"),
	PINCTRL_PIN(26, "PB10"),
	PINCTRL_PIN(27, "PB11"),
	PINCTRL_PIN(28, "PB12"),
	PINCTRL_PIN(29, "PB13"),
	PINCTRL_PIN(30, "PB14"),
	PINCTRL_PIN(31, "PB15"),
	PINCTRL_PIN(32, "PC0"),
	PINCTRL_PIN(33, "PC1"),
	PINCTRL_PIN(34, "PC2"),
	PINCTRL_PIN(35, "PC3"),
	PINCTRL_PIN(36, "PC4"),
	PINCTRL_PIN(37, "PC5"),
	PINCTRL_PIN(38, "PC6"),
	PINCTRL_PIN(39, "PC7"),
	PINCTRL_PIN(40, "PC8"),
	PINCTRL_PIN(41, "PC9"),
	PINCTRL_PIN(42, "PC10"),
	PINCTRL_PIN(43, "PC11"),
	PINCTRL_PIN(44, "PC12"),
	PINCTRL_PIN(45, "PC13"),
	PINCTRL_PIN(46, "PC14"),
	PINCTRL_PIN(47, "PC15"),
	PINCTRL_PIN(48, "PD0"),
	PINCTRL_PIN(49, "PD1"),
	PINCTRL_PIN(50, "PD2"),
	PINCTRL_PIN(51, "PD3"),
	PINCTRL_PIN(52, "PD4"),
	PINCTRL_PIN(53, "PD5"),
	PINCTRL_PIN(54, "PD6"),
	PINCTRL_PIN(55, "PD7"),
	PINCTRL_PIN(56, "PD8"),
	PINCTRL_PIN(57, "PD9"),
	PINCTRL_PIN(58, "PD10"),
	PINCTRL_PIN(59, "PD11"),
	PINCTRL_PIN(60, "PD12"),
	PINCTRL_PIN(61, "PD13"),
	PINCTRL_PIN(62, "PD14"),
	PINCTRL_PIN(63, "PD15"),
	PINCTRL_PIN(64, "PE0"),
	PINCTRL_PIN(65, "PE1"),
	PINCTRL_PIN(66, "PE2"),
	PINCTRL_PIN(67, "PE3"),
	PINCTRL_PIN(68, "PE4"),
	PINCTRL_PIN(69, "PE5"),
	PINCTRL_PIN(70, "PE6"),
	PINCTRL_PIN(71, "PE7"),
	PINCTRL_PIN(72, "PE8"),
	PINCTRL_PIN(73, "PE9"),
	PINCTRL_PIN(74, "PE10"),
	PINCTRL_PIN(75, "PE11"),
	PINCTRL_PIN(76, "PE12"),
	PINCTRL_PIN(77, "PE13"),
	PINCTRL_PIN(78, "PE14"),
	PINCTRL_PIN(79, "PE15"),
	PINCTRL_PIN(80, "PF0"),
	PINCTRL_PIN(81, "PF1"),
	PINCTRL_PIN(82, "PF2"),
	PINCTRL_PIN(83, "PF3"),
	PINCTRL_PIN(84, "PF4"),
	PINCTRL_PIN(85, "PF5"),
	PINCTRL_PIN(86, "PF6"),
	PINCTRL_PIN(87, "PF7"),
	PINCTRL_PIN(88, "PF8"),
	PINCTRL_PIN(89, "PF9"),
	PINCTRL_PIN(90, "PF10"),
	PINCTRL_PIN(91, "PF11"),
	PINCTRL_PIN(92, "PF12"),
	PINCTRL_PIN(93, "PF13"),
	PINCTRL_PIN(94, "PF14"),
	PINCTRL_PIN(95, "PF15"),
	PINCTRL_PIN(96, "PG0"),
	PINCTRL_PIN(97, "PG1"),
	PINCTRL_PIN(98, "PG2"),
	PINCTRL_PIN(99, "PG3"),
	PINCTRL_PIN(100, "PG4"),
	PINCTRL_PIN(101, "PG5"),
	PINCTRL_PIN(102, "PG6"),
	PINCTRL_PIN(103, "PG7"),
	PINCTRL_PIN(104, "PG8"),
	PINCTRL_PIN(105, "PG9"),
	PINCTRL_PIN(106, "PG10"),
	PINCTRL_PIN(107, "PG11"),
	PINCTRL_PIN(108, "PG12"),
	PINCTRL_PIN(109, "PG13"),
	PINCTRL_PIN(110, "PG14"),
	PINCTRL_PIN(111, "PG15"),
};

static const unsigned uart0_pins[] = {
	GPIO_PD7, GPIO_PD8,
};

static const unsigned uart0_ctsrts_pins[] = {
	GPIO_PD9, GPIO_PD10,
};

static const unsigned uart1_pins[] = {
	GPIO_PG15, GPIO_PG14,
};

static const unsigned uart1_ctsrts_pins[] = {
	GPIO_PG10, GPIO_PG13,
};

static const unsigned rsi0_pins[] = {
	GPIO_PG3, GPIO_PG2, GPIO_PG0, GPIO_PE15, GPIO_PG5, GPIO_PG6,
};

static const unsigned eth0_pins[] = {
	GPIO_PC6, GPIO_PC7, GPIO_PC2, GPIO_PC0, GPIO_PC3, GPIO_PC1,
	GPIO_PB13, GPIO_PD6, GPIO_PC5, GPIO_PC4, GPIO_PB14, GPIO_PB15,
};

static const unsigned eth1_pins[] = {
	GPIO_PE10, GPIO_PE11, GPIO_PG3, GPIO_PG0, GPIO_PG2, GPIO_PE15,
	GPIO_PG5, GPIO_PE12, GPIO_PE13, GPIO_PE14, GPIO_PG6, GPIO_PC9,
};

static const unsigned spi0_pins[] = {
	GPIO_PD4, GPIO_PD2, GPIO_PD3,
};

static const unsigned spi1_pins[] = {
	GPIO_PD5, GPIO_PD14, GPIO_PD13,
};

static const unsigned twi0_pins[] = {
};

static const unsigned twi1_pins[] = {
};

static const unsigned rotary_pins[] = {
	GPIO_PG7, GPIO_PG11, GPIO_PG12,
};

static const unsigned can0_pins[] = {
	GPIO_PG1, GPIO_PG4,
};

static const unsigned smc0_pins[] = {
	GPIO_PA0, GPIO_PA1, GPIO_PA2, GPIO_PA3, GPIO_PA4, GPIO_PA5, GPIO_PA6,
	GPIO_PA7, GPIO_PA8, GPIO_PA9, GPIO_PB2, GPIO_PA10, GPIO_PA11,
	GPIO_PB3, GPIO_PA12, GPIO_PA13, GPIO_PA14, GPIO_PA15, GPIO_PB6,
	GPIO_PB7, GPIO_PB8, GPIO_PB10, GPIO_PB11, GPIO_PB0,
};

static const unsigned sport0_pins[] = {
	GPIO_PB5, GPIO_PB4, GPIO_PB9, GPIO_PB8, GPIO_PB7, GPIO_PB11,
};

static const unsigned sport1_pins[] = {
	GPIO_PE2, GPIO_PE5, GPIO_PD15, GPIO_PE4, GPIO_PE3, GPIO_PE1,
};

static const unsigned sport2_pins[] = {
	GPIO_PG4, GPIO_PG1, GPIO_PG9, GPIO_PG10, GPIO_PG7, GPIO_PB12,
};

static const unsigned ppi0_8b_pins[] = {
	GPIO_PF0, GPIO_PF1, GPIO_PF2, GPIO_PF3, GPIO_PF4, GPIO_PF5, GPIO_PF6,
	GPIO_PF7, GPIO_PF13, GPIO_PF14, GPIO_PF15,
	GPIO_PE6, GPIO_PE7, GPIO_PE8, GPIO_PE9,
};

static const unsigned ppi0_16b_pins[] = {
	GPIO_PF0, GPIO_PF1, GPIO_PF2, GPIO_PF3, GPIO_PF4, GPIO_PF5, GPIO_PF6,
	GPIO_PF7, GPIO_PF9, GPIO_PF10, GPIO_PF11, GPIO_PF12,
	GPIO_PF13, GPIO_PF14, GPIO_PF15,
	GPIO_PE6, GPIO_PE7, GPIO_PE8, GPIO_PE9,
};

static const unsigned ppi0_24b_pins[] = {
	GPIO_PF0, GPIO_PF1, GPIO_PF2, GPIO_PF3, GPIO_PF4, GPIO_PF5, GPIO_PF6,
	GPIO_PF7, GPIO_PF8, GPIO_PF9, GPIO_PF10, GPIO_PF11, GPIO_PF12,
	GPIO_PF13, GPIO_PF14, GPIO_PF15, GPIO_PE0, GPIO_PE1, GPIO_PE2,
	GPIO_PE3, GPIO_PE4, GPIO_PE5, GPIO_PE6, GPIO_PE7, GPIO_PE8,
	GPIO_PE9, GPIO_PD12, GPIO_PD15,
};

static const unsigned ppi1_8b_pins[] = {
	GPIO_PC0, GPIO_PC1, GPIO_PC2, GPIO_PC3, GPIO_PC4, GPIO_PC5, GPIO_PC6,
	GPIO_PC7, GPIO_PC8, GPIO_PB13, GPIO_PB14, GPIO_PB15, GPIO_PD6,
};

static const unsigned ppi1_16b_pins[] = {
	GPIO_PC0, GPIO_PC1, GPIO_PC2, GPIO_PC3, GPIO_PC4, GPIO_PC5, GPIO_PC6,
	GPIO_PC7, GPIO_PC9, GPIO_PC10, GPIO_PC11, GPIO_PC12,
	GPIO_PC13, GPIO_PC14, GPIO_PC15,
	GPIO_PB13, GPIO_PB14, GPIO_PB15, GPIO_PD6,
};

static const unsigned ppi2_8b_pins[] = {
	GPIO_PA0, GPIO_PA1, GPIO_PA2, GPIO_PA3, GPIO_PA4, GPIO_PA5, GPIO_PA6,
	GPIO_PA7, GPIO_PB0, GPIO_PB1, GPIO_PB2, GPIO_PB3,
};

static const unsigned ppi2_16b_pins[] = {
	GPIO_PA0, GPIO_PA1, GPIO_PA2, GPIO_PA3, GPIO_PA4, GPIO_PA5, GPIO_PA6,
	GPIO_PA7, GPIO_PA8, GPIO_PA9, GPIO_PA10, GPIO_PA11, GPIO_PA12,
	GPIO_PA13, GPIO_PA14, GPIO_PA15, GPIO_PB0, GPIO_PB1, GPIO_PB2,
};

static const unsigned lp0_pins[] = {
	GPIO_PB0, GPIO_PB1, GPIO_PA0, GPIO_PA1, GPIO_PA2, GPIO_PA3,
	GPIO_PA4, GPIO_PA5, GPIO_PA6, GPIO_PA7,
};

static const unsigned lp1_pins[] = {
	GPIO_PB3, GPIO_PB2, GPIO_PA8, GPIO_PA9, GPIO_PA10, GPIO_PA11,
	GPIO_PA12, GPIO_PA13, GPIO_PA14, GPIO_PA15,
};

static const unsigned lp2_pins[] = {
	GPIO_PE6, GPIO_PE7, GPIO_PF0, GPIO_PF1, GPIO_PF2, GPIO_PF3,
	GPIO_PF4, GPIO_PF5, GPIO_PF6, GPIO_PF7,
};

static const unsigned lp3_pins[] = {
	GPIO_PE9, GPIO_PE8, GPIO_PF8, GPIO_PF9, GPIO_PF10, GPIO_PF11,
	GPIO_PF12, GPIO_PF13, GPIO_PF14, GPIO_PF15,
};

static const unsigned short uart0_mux[] = {
	P_UART0_TX, P_UART0_RX,
	0
};

static const unsigned short uart0_ctsrts_mux[] = {
	P_UART0_RTS, P_UART0_CTS,
	0
};

static const unsigned short uart1_mux[] = {
	P_UART1_TX, P_UART1_RX,
	0
};

static const unsigned short uart1_ctsrts_mux[] = {
	P_UART1_RTS, P_UART1_CTS,
	0
};

static const unsigned short rsi0_mux[] = {
	P_RSI_DATA0, P_RSI_DATA1, P_RSI_DATA2, P_RSI_DATA3,
	P_RSI_CMD, P_RSI_CLK, 0
};

static const unsigned short eth0_mux[] = P_RMII0;
static const unsigned short eth1_mux[] = P_RMII1;

static const unsigned short spi0_mux[] = {
	P_SPI0_SCK, P_SPI0_MISO, P_SPI0_MOSI, 0
};

static const unsigned short spi1_mux[] = {
	P_SPI1_SCK, P_SPI1_MISO, P_SPI1_MOSI, 0
};

static const unsigned short twi0_mux[] = {
	P_TWI0_SCL, P_TWI0_SDA, 0
};

static const unsigned short twi1_mux[] = {
	P_TWI1_SCL, P_TWI1_SDA, 0
};

static const unsigned short rotary_mux[] = {
	P_CNT_CUD, P_CNT_CDG, P_CNT_CZM, 0
};

static const unsigned short sport0_mux[] = {
	P_SPORT0_ACLK, P_SPORT0_AFS, P_SPORT0_AD0, P_SPORT0_BCLK,
	P_SPORT0_BFS, P_SPORT0_BD0, 0,
};

static const unsigned short sport1_mux[] = {
	P_SPORT1_ACLK, P_SPORT1_AFS, P_SPORT1_AD0, P_SPORT1_BCLK,
	P_SPORT1_BFS, P_SPORT1_BD0, 0,
};

static const unsigned short sport2_mux[] = {
	P_SPORT2_ACLK, P_SPORT2_AFS, P_SPORT2_AD0, P_SPORT2_BCLK,
	P_SPORT2_BFS, P_SPORT2_BD0, 0,
};

static const unsigned short can0_mux[] = {
	P_CAN0_RX, P_CAN0_TX, 0
};

static const unsigned short smc0_mux[] = {
	P_A3, P_A4, P_A5, P_A6, P_A7, P_A8, P_A9, P_A10, P_A11, P_A12,
	P_A13, P_A14, P_A15, P_A16, P_A17, P_A18, P_A19, P_A20, P_A21,
	P_A22, P_A23, P_A24, P_A25, P_NORCK, 0,
};

static const unsigned short ppi0_8b_mux[] = {
	P_PPI0_D0, P_PPI0_D1, P_PPI0_D2, P_PPI0_D3,
	P_PPI0_D4, P_PPI0_D5, P_PPI0_D6, P_PPI0_D7,
	P_PPI0_CLK, P_PPI0_FS1, P_PPI0_FS2,
	0,
};

static const unsigned short ppi0_16b_mux[] = {
	P_PPI0_D0, P_PPI0_D1, P_PPI0_D2, P_PPI0_D3,
	P_PPI0_D4, P_PPI0_D5, P_PPI0_D6, P_PPI0_D7,
	P_PPI0_D8, P_PPI0_D9, P_PPI0_D10, P_PPI0_D11,
	P_PPI0_D12, P_PPI0_D13, P_PPI0_D14, P_PPI0_D15,
	P_PPI0_CLK, P_PPI0_FS1, P_PPI0_FS2,
	0,
};

static const unsigned short ppi0_24b_mux[] = {
	P_PPI0_D0, P_PPI0_D1, P_PPI0_D2, P_PPI0_D3,
	P_PPI0_D4, P_PPI0_D5, P_PPI0_D6, P_PPI0_D7,
	P_PPI0_D8, P_PPI0_D9, P_PPI0_D10, P_PPI0_D11,
	P_PPI0_D12, P_PPI0_D13, P_PPI0_D14, P_PPI0_D15,
	P_PPI0_D16, P_PPI0_D17, P_PPI0_D18, P_PPI0_D19,
	P_PPI0_D20, P_PPI0_D21, P_PPI0_D22, P_PPI0_D23,
	P_PPI0_CLK, P_PPI0_FS1, P_PPI0_FS2,
	0,
};

static const unsigned short ppi1_8b_mux[] = {
	P_PPI1_D0, P_PPI1_D1, P_PPI1_D2, P_PPI1_D3,
	P_PPI1_D4, P_PPI1_D5, P_PPI1_D6, P_PPI1_D7,
	P_PPI1_CLK, P_PPI1_FS1, P_PPI1_FS2,
	0,
};

static const unsigned short ppi1_16b_mux[] = {
	P_PPI1_D0, P_PPI1_D1, P_PPI1_D2, P_PPI1_D3,
	P_PPI1_D4, P_PPI1_D5, P_PPI1_D6, P_PPI1_D7,
	P_PPI1_D8, P_PPI1_D9, P_PPI1_D10, P_PPI1_D11,
	P_PPI1_D12, P_PPI1_D13, P_PPI1_D14, P_PPI1_D15,
	P_PPI1_CLK, P_PPI1_FS1, P_PPI1_FS2,
	0,
};

static const unsigned short ppi2_8b_mux[] = {
	P_PPI2_D0, P_PPI2_D1, P_PPI2_D2, P_PPI2_D3,
	P_PPI2_D4, P_PPI2_D5, P_PPI2_D6, P_PPI2_D7,
	P_PPI2_CLK, P_PPI2_FS1, P_PPI2_FS2,
	0,
};

static const unsigned short ppi2_16b_mux[] = {
	P_PPI2_D0, P_PPI2_D1, P_PPI2_D2, P_PPI2_D3,
	P_PPI2_D4, P_PPI2_D5, P_PPI2_D6, P_PPI2_D7,
	P_PPI2_D8, P_PPI2_D9, P_PPI2_D10, P_PPI2_D11,
	P_PPI2_D12, P_PPI2_D13, P_PPI2_D14, P_PPI2_D15,
	P_PPI2_CLK, P_PPI2_FS1, P_PPI2_FS2,
	0,
};

static const unsigned short lp0_mux[] = {
	P_LP0_CLK, P_LP0_ACK, P_LP0_D0, P_LP0_D1, P_LP0_D2,
	P_LP0_D3, P_LP0_D4, P_LP0_D5, P_LP0_D6, P_LP0_D7,
        0
};

static const unsigned short lp1_mux[] = {
	P_LP1_CLK, P_LP1_ACK, P_LP1_D0, P_LP1_D1, P_LP1_D2,
	P_LP1_D3, P_LP1_D4, P_LP1_D5, P_LP1_D6, P_LP1_D7,
        0
};

static const unsigned short lp2_mux[] = {
	P_LP2_CLK, P_LP2_ACK, P_LP2_D0, P_LP2_D1, P_LP2_D2,
	P_LP2_D3, P_LP2_D4, P_LP2_D5, P_LP2_D6, P_LP2_D7,
        0
};

static const unsigned short lp3_mux[] = {
	P_LP3_CLK, P_LP3_ACK, P_LP3_D0, P_LP3_D1, P_LP3_D2,
	P_LP3_D3, P_LP3_D4, P_LP3_D5, P_LP3_D6, P_LP3_D7,
        0
};

static const struct adi_pin_group adi_pin_groups[] = {
	ADI_PIN_GROUP("uart0grp", uart0_pins, uart0_mux),
	ADI_PIN_GROUP("uart0ctsrtsgrp", uart0_ctsrts_pins, uart0_ctsrts_mux),
	ADI_PIN_GROUP("uart1grp", uart1_pins, uart1_mux),
	ADI_PIN_GROUP("uart1ctsrtsgrp", uart1_ctsrts_pins, uart1_ctsrts_mux),
	ADI_PIN_GROUP("rsi0grp", rsi0_pins, rsi0_mux),
	ADI_PIN_GROUP("eth0grp", eth0_pins, eth0_mux),
	ADI_PIN_GROUP("eth1grp", eth1_pins, eth1_mux),
	ADI_PIN_GROUP("spi0grp", spi0_pins, spi0_mux),
	ADI_PIN_GROUP("spi1grp", spi1_pins, spi1_mux),
	ADI_PIN_GROUP("twi0grp", twi0_pins, twi0_mux),
	ADI_PIN_GROUP("twi1grp", twi1_pins, twi1_mux),
	ADI_PIN_GROUP("rotarygrp", rotary_pins, rotary_mux),
	ADI_PIN_GROUP("can0grp", can0_pins, can0_mux),
	ADI_PIN_GROUP("smc0grp", smc0_pins, smc0_mux),
	ADI_PIN_GROUP("sport0grp", sport0_pins, sport0_mux),
	ADI_PIN_GROUP("sport1grp", sport1_pins, sport1_mux),
	ADI_PIN_GROUP("sport2grp", sport2_pins, sport2_mux),
	ADI_PIN_GROUP("ppi0_8bgrp", ppi0_8b_pins, ppi0_8b_mux),
	ADI_PIN_GROUP("ppi0_16bgrp", ppi0_16b_pins, ppi0_16b_mux),
	ADI_PIN_GROUP("ppi0_24bgrp", ppi0_24b_pins, ppi0_24b_mux),
	ADI_PIN_GROUP("ppi1_8bgrp", ppi1_8b_pins, ppi1_8b_mux),
	ADI_PIN_GROUP("ppi1_16bgrp", ppi1_16b_pins, ppi1_16b_mux),
	ADI_PIN_GROUP("ppi2_8bgrp", ppi2_8b_pins, ppi2_8b_mux),
	ADI_PIN_GROUP("ppi2_16bgrp", ppi2_16b_pins, ppi2_16b_mux),
	ADI_PIN_GROUP("lp0grp", lp0_pins, lp0_mux),
	ADI_PIN_GROUP("lp1grp", lp1_pins, lp1_mux),
	ADI_PIN_GROUP("lp2grp", lp2_pins, lp2_mux),
	ADI_PIN_GROUP("lp3grp", lp3_pins, lp3_mux),
};

static const char * const uart0grp[] = { "uart0grp" };
static const char * const uart0ctsrtsgrp[] = { "uart0ctsrtsgrp" };
static const char * const uart1grp[] = { "uart1grp" };
static const char * const uart1ctsrtsgrp[] = { "uart1ctsrtsgrp" };
static const char * const rsi0grp[] = { "rsi0grp" };
static const char * const eth0grp[] = { "eth0grp" };
static const char * const eth1grp[] = { "eth1grp" };
static const char * const spi0grp[] = { "spi0grp" };
static const char * const spi1grp[] = { "spi1grp" };
static const char * const twi0grp[] = { "twi0grp" };
static const char * const twi1grp[] = { "twi1grp" };
static const char * const rotarygrp[] = { "rotarygrp" };
static const char * const can0grp[] = { "can0grp" };
static const char * const smc0grp[] = { "smc0grp" };
static const char * const sport0grp[] = { "sport0grp" };
static const char * const sport1grp[] = { "sport1grp" };
static const char * const sport2grp[] = { "sport2grp" };
static const char * const ppi0grp[] = { "ppi0_8bgrp",
					"ppi0_16bgrp",
					"ppi0_24bgrp" };
static const char * const ppi1grp[] = { "ppi1_8bgrp",
					"ppi1_16bgrp" };
static const char * const ppi2grp[] = { "ppi2_8bgrp",
					"ppi2_16bgrp" };
static const char * const lp0grp[] = { "lp0grp" };
static const char * const lp1grp[] = { "lp1grp" };
static const char * const lp2grp[] = { "lp2grp" };
static const char * const lp3grp[] = { "lp3grp" };

static const struct adi_pmx_func adi_pmx_functions[] = {
	ADI_PMX_FUNCTION("uart0", uart0grp),
	ADI_PMX_FUNCTION("uart0_ctsrts", uart0ctsrtsgrp),
	ADI_PMX_FUNCTION("uart1", uart1grp),
	ADI_PMX_FUNCTION("uart1_ctsrts", uart1ctsrtsgrp),
	ADI_PMX_FUNCTION("rsi0", rsi0grp),
	ADI_PMX_FUNCTION("eth0", eth0grp),
	ADI_PMX_FUNCTION("eth1", eth1grp),
	ADI_PMX_FUNCTION("spi0", spi0grp),
	ADI_PMX_FUNCTION("spi1", spi1grp),
	ADI_PMX_FUNCTION("twi0", twi0grp),
	ADI_PMX_FUNCTION("twi1", twi1grp),
	ADI_PMX_FUNCTION("rotary", rotarygrp),
	ADI_PMX_FUNCTION("can0", can0grp),
	ADI_PMX_FUNCTION("smc0", smc0grp),
	ADI_PMX_FUNCTION("sport0", sport0grp),
	ADI_PMX_FUNCTION("sport1", sport1grp),
	ADI_PMX_FUNCTION("sport2", sport2grp),
	ADI_PMX_FUNCTION("ppi0", ppi0grp),
	ADI_PMX_FUNCTION("ppi1", ppi1grp),
	ADI_PMX_FUNCTION("ppi2", ppi2grp),
	ADI_PMX_FUNCTION("lp0", lp0grp),
	ADI_PMX_FUNCTION("lp1", lp1grp),
	ADI_PMX_FUNCTION("lp2", lp2grp),
	ADI_PMX_FUNCTION("lp3", lp3grp),
};

static const struct adi_pinctrl_soc_data adi_bf60x_soc = {
	.functions = adi_pmx_functions,
	.nfunctions = ARRAY_SIZE(adi_pmx_functions),
	.groups = adi_pin_groups,
	.ngroups = ARRAY_SIZE(adi_pin_groups),
	.pins = adi_pads,
	.npins = ARRAY_SIZE(adi_pads),
};

void adi_pinctrl_soc_init(const struct adi_pinctrl_soc_data **soc)
{
	*soc = &adi_bf60x_soc;
}
