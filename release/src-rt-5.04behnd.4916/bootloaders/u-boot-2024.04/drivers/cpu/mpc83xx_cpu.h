/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2018
 * Mario Six, Guntermann & Drunck GmbH, mario.six@gdsys.cc
 */

#ifndef _MPC83XX_CPU_H_
#define _MPC83XX_CPU_H_

/**
 * enum e300_type - Identifiers for e300 cores
 * @E300C1:       Identifier for e300c1 cores
 * @E300C2:       Identifier for e300c2 cores
 * @E300C3:       Identifier for e300c3 cores
 * @E300C4:       Identifier for e300c4 cores
 * @E300_UNKNOWN: Identifier for unknown e300 cores
 */
enum e300_type {
	E300C1,
	E300C2,
	E300C3,
	E300C4,
	E300_UNKNOWN,
};

/* Array mapping the e300 core types to their human-readable names */
static const char * const e300_names[] = {
	[E300C1] = "e300c1",
	[E300C2] = "e300c2",
	[E300C3] = "e300c3",
	[E300C4] = "e300c4",
	[E300_UNKNOWN] = "Unknown e300",
};

/**
 * enum mpc83xx_cpu_family - Identifiers for MPC83xx CPU families
 * @FAMILY_830X:    Identifier for the MPC830x CPU family
 * @FAMILY_831X:    Identifier for the MPC831x CPU family
 * @FAMILY_832X:    Identifier for the MPC832x CPU family
 * @FAMILY_834X:    Identifier for the MPC834x CPU family
 * @FAMILY_836X:    Identifier for the MPC836x CPU family
 * @FAMILY_837X:    Identifier for the MPC837x CPU family
 * @FAMILY_UNKNOWN: Identifier for an unknown MPC83xx CPU family
 */
enum mpc83xx_cpu_family {
	FAMILY_830X,
	FAMILY_831X,
	FAMILY_832X,
	FAMILY_834X,
	FAMILY_836X,
	FAMILY_837X,
	FAMILY_UNKNOWN,
};

/**
 * enum mpc83xx_cpu_type - Identifiers for MPC83xx CPU types
 * @TYPE_8308:      Identifier for the MPC8308 CPU type
 * @TYPE_8309:      Identifier for the MPC8309 CPU type
 * @TYPE_8311:      Identifier for the MPC8311 CPU type
 * @TYPE_8313:      Identifier for the MPC8313 CPU type
 * @TYPE_8314:      Identifier for the MPC8314 CPU type
 * @TYPE_8315:      Identifier for the MPC8315 CPU type
 * @TYPE_8321:      Identifier for the MPC8321 CPU type
 * @TYPE_8323:      Identifier for the MPC8323 CPU type
 * @TYPE_8343:      Identifier for the MPC8343 CPU type
 * @TYPE_8347_TBGA: Identifier for the MPC8347 CPU type (Tape Ball Grid Array
 *		    version)
 * @TYPE_8347_PBGA: Identifier for the MPC8347 CPU type (Plastic Ball Grid Array
 *		    version)
 * @TYPE_8349:      Identifier for the MPC8349 CPU type
 * @TYPE_8358_TBGA: Identifier for the MPC8358 CPU type (Tape Ball Grid Array
 *		    version)
 * @TYPE_8358_PBGA: Identifier for the MPC8358 CPU type (Plastic Ball Grid Array
 *		    version)
 * @TYPE_8360:      Identifier for the MPC8360 CPU type
 * @TYPE_8377:      Identifier for the MPC8377 CPU type
 * @TYPE_8378:      Identifier for the MPC8378 CPU type
 * @TYPE_8379:      Identifier for the MPC8379 CPU type
 * @TYPE_UNKNOWN:   Identifier for an unknown MPC83xx CPU type
 */
enum mpc83xx_cpu_type {
	TYPE_8308,
	TYPE_8309,
	TYPE_8311,
	TYPE_8313,
	TYPE_8314,
	TYPE_8315,
	TYPE_8321,
	TYPE_8323,
	TYPE_8343,
	TYPE_8347_TBGA,
	TYPE_8347_PBGA,
	TYPE_8349,
	TYPE_8358_TBGA,
	TYPE_8358_PBGA,
	TYPE_8360,
	TYPE_8377,
	TYPE_8378,
	TYPE_8379,
	TYPE_UNKNOWN,
};

/* Array mapping the MCP83xx CPUs to their human-readable names */
static const char * const cpu_type_names[] = {
	[TYPE_8308] = "8308",
	[TYPE_8309] = "8309",
	[TYPE_8311] = "8311",
	[TYPE_8313] = "8313",
	[TYPE_8314] = "8314",
	[TYPE_8315] = "8315",
	[TYPE_8321] = "8321",
	[TYPE_8323] = "8323",
	[TYPE_8343] = "8343",
	[TYPE_8347_TBGA] = "8347_TBGA",
	[TYPE_8347_PBGA] = "8347_PBGA",
	[TYPE_8349] = "8349",
	[TYPE_8358_TBGA] = "8358_TBGA",
	[TYPE_8358_PBGA] = "8358_PBGA",
	[TYPE_8360] = "8360",
	[TYPE_8377] = "8377",
	[TYPE_8378] = "8378",
	[TYPE_8379] = "8379",
	[TYPE_UNKNOWN] = "Unknown CPU",
};

#endif /* !_MPC83XX_CPU_H_ */
