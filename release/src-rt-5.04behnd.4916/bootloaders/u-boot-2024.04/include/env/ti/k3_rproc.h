/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2019 Texas Instruments Incorporated - https://www.ti.com
 *
 * rproc environment variable definitions for various TI K3 SoCs.
 */

#ifndef __TI_RPROC_H
#define __TI_RPROC_H

/*
 * should contain a list of <rproc_id fw_name> tuplies,
 * override in board config files with the actual list
 */
#define DEFAULT_RPROCS ""

#ifdef CONFIG_CMD_REMOTEPROC
#define EXTRA_ENV_RPROC_SETTINGS					\
	"dorprocboot=0\0"						\
	"boot_rprocs="							\
		"if test ${dorprocboot} -eq 1 && test ${boot} = mmc; then "\
			"rproc init;"					\
			"run boot_rprocs_mmc;"				\
		"fi;\0"							\
	"rproc_load_and_boot_one="					\
		"if load mmc ${bootpart} $loadaddr ${rproc_fw}; then "	\
			"if rproc load ${rproc_id} ${loadaddr} ${filesize}; then "\
				"rproc start ${rproc_id};"		\
			"fi;"						\
		"fi\0"							\
	"boot_rprocs_mmc="						\
		"env set rproc_id;"					\
		"env set rproc_fw;"					\
		"for i in ${rproc_fw_binaries} ; do "			\
			"if test -z \"${rproc_id}\" ; then "		\
				"env set rproc_id $i;"			\
			"else "						\
				"env set rproc_fw $i;"			\
				"run rproc_load_and_boot_one;"		\
				"env set rproc_id;"			\
				"env set rproc_fw;"			\
			"fi;"						\
		"done\0"						\
	"rproc_fw_binaries="						\
		DEFAULT_RPROCS						\
		"\0"
#else
#define EXTRA_ENV_RPROC_SETTINGS					\
	"boot_rprocs= \0"
#endif /* CONFIG_CMD_REMOTEPROC */

#endif /* __TI_RPROC_H */
