/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2018
 * Mario Six, Guntermann & Drunck GmbH, mario.six@gdsys.cc
 */

#ifndef _MPC83XX_SOC_H_
#define _MPC83XX_SOC_H_

enum soc_type {
	SOC_MPC8308,
	SOC_MPC8309,
	SOC_MPC8313,
	SOC_MPC8315,
	SOC_MPC832X,
	SOC_MPC8349,
	SOC_MPC8360,
	SOC_MPC8379,
};

bool mpc83xx_has_sdhc(int type)
{
	return (type == SOC_MPC8308) ||
	       (type == SOC_MPC8309) ||
	       (type == SOC_MPC8379);
}

bool mpc83xx_has_tsec(int type)
{
	return (type == SOC_MPC8308) ||
	       (type == SOC_MPC8313) ||
	       (type == SOC_MPC8315) ||
	       (type == SOC_MPC8349) ||
	       (type == SOC_MPC8379);
}

bool mpc83xx_has_pcie1(int type)
{
	return (type == SOC_MPC8308) ||
	       (type == SOC_MPC8315) ||
	       (type == SOC_MPC8379);
}

bool mpc83xx_has_pcie2(int type)
{
	return (type == SOC_MPC8315) ||
	       (type == SOC_MPC8379);
}

bool mpc83xx_has_sata(int type)
{
	return (type == SOC_MPC8315) ||
	       (type == SOC_MPC8379);
}

bool mpc83xx_has_pci(int type)
{
	return type != SOC_MPC8308;
}

bool mpc83xx_has_second_i2c(int type)
{
	return (type != SOC_MPC8315) &&
	       (type != SOC_MPC832X);
}

bool mpc83xx_has_quicc_engine(int type)
{
	return (type == SOC_MPC8309) ||
	       (type == SOC_MPC832X) ||
	       (type == SOC_MPC8360);
}

#endif /* _MPC83XX_SOC_H_ */
