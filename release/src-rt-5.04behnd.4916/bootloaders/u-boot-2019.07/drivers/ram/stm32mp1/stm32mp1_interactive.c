// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright (C) 2019, STMicroelectronics - All Rights Reserved
 */

#include <common.h>
#include <console.h>
#include <cli.h>
#include <clk.h>
#include <malloc.h>
#include <ram.h>
#include <reset.h>
#include "stm32mp1_ddr.h"
#include "stm32mp1_tests.h"

DECLARE_GLOBAL_DATA_PTR;

enum ddr_command {
	DDR_CMD_HELP,
	DDR_CMD_INFO,
	DDR_CMD_FREQ,
	DDR_CMD_RESET,
	DDR_CMD_PARAM,
	DDR_CMD_PRINT,
	DDR_CMD_EDIT,
	DDR_CMD_STEP,
	DDR_CMD_NEXT,
	DDR_CMD_GO,
	DDR_CMD_TEST,
	DDR_CMD_TUNING,
	DDR_CMD_UNKNOWN,
};

const char *step_str[] = {
	[STEP_DDR_RESET] = "DDR_RESET",
	[STEP_CTL_INIT] = "DDR_CTRL_INIT_DONE",
	[STEP_PHY_INIT] = "DDR PHY_INIT_DONE",
	[STEP_DDR_READY] = "DDR_READY",
	[STEP_RUN] = "RUN"
};

enum ddr_command stm32mp1_get_command(char *cmd, int argc)
{
	const char *cmd_string[DDR_CMD_UNKNOWN] = {
		[DDR_CMD_HELP] = "help",
		[DDR_CMD_INFO] = "info",
		[DDR_CMD_FREQ] = "freq",
		[DDR_CMD_RESET] = "reset",
		[DDR_CMD_PARAM] = "param",
		[DDR_CMD_PRINT] = "print",
		[DDR_CMD_EDIT] = "edit",
		[DDR_CMD_STEP] = "step",
		[DDR_CMD_NEXT] = "next",
		[DDR_CMD_GO] = "go",
#ifdef CONFIG_STM32MP1_DDR_TESTS
		[DDR_CMD_TEST] = "test",
#endif
#ifdef CONFIG_STM32MP1_DDR_TUNING
		[DDR_CMD_TUNING] = "tuning",
#endif
	};
	/* min and max number of argument */
	const char cmd_arg[DDR_CMD_UNKNOWN][2] = {
		[DDR_CMD_HELP] = { 0, 0 },
		[DDR_CMD_INFO] = { 0, 255 },
		[DDR_CMD_FREQ] = { 0, 1 },
		[DDR_CMD_RESET] = { 0, 0 },
		[DDR_CMD_PARAM] = { 0, 2 },
		[DDR_CMD_PRINT] = { 0, 1 },
		[DDR_CMD_EDIT] = { 2, 2 },
		[DDR_CMD_STEP] = { 0, 1 },
		[DDR_CMD_NEXT] = { 0, 0 },
		[DDR_CMD_GO] = { 0, 0 },
#ifdef CONFIG_STM32MP1_DDR_TESTS
		[DDR_CMD_TEST] = { 0, 255 },
#endif
#ifdef CONFIG_STM32MP1_DDR_TUNING
		[DDR_CMD_TUNING] = { 0, 255 },
#endif
	};
	int i;

	for (i = 0; i < DDR_CMD_UNKNOWN; i++)
		if (!strcmp(cmd, cmd_string[i])) {
			if (argc - 1 < cmd_arg[i][0]) {
				printf("no enought argument (min=%d)\n",
				       cmd_arg[i][0]);
				return DDR_CMD_UNKNOWN;
			} else if (argc - 1 > cmd_arg[i][1]) {
				printf("too many argument (max=%d)\n",
				       cmd_arg[i][1]);
				return DDR_CMD_UNKNOWN;
			} else {
				return i;
			}
		}

	printf("unknown command %s\n", cmd);
	return DDR_CMD_UNKNOWN;
}

static void stm32mp1_do_usage(void)
{
	const char *usage = {
		"commands:\n\n"
		"help                       displays help\n"
		"info                       displays DDR information\n"
		"info  <param> <val>        changes DDR information\n"
		"      with <param> = step, name, size or speed\n"
		"freq                       displays the DDR PHY frequency in kHz\n"
		"freq  <freq>               changes the DDR PHY frequency\n"
		"param [type|reg]           prints input parameters\n"
		"param <reg> <val>          edits parameters in step 0\n"
		"print [type|reg]           dumps registers\n"
		"edit <reg> <val>           modifies one register\n"
		"step                       lists the available step\n"
		"step <n>                   go to the step <n>\n"
		"next                       goes to the next step\n"
		"go                         continues the U-Boot SPL execution\n"
		"reset                      reboots machine\n"
#ifdef CONFIG_STM32MP1_DDR_TESTS
		"test [help] | <n> [...]    lists (with help) or executes test <n>\n"
#endif
#ifdef CONFIG_STM32MP1_DDR_TUNING
		"tuning [help] | <n> [...]  lists (with help) or execute tuning <n>\n"
#endif
		"\nwith for [type|reg]:\n"
		"  all registers if absent\n"
		"  <type> = ctl, phy\n"
		"           or one category (static, timing, map, perf, cal, dyn)\n"
		"  <reg> = name of the register\n"
	};

	puts(usage);
}

static bool stm32mp1_check_step(enum stm32mp1_ddr_interact_step step,
				enum stm32mp1_ddr_interact_step expected)
{
	if (step != expected) {
		printf("invalid step %d:%s expecting %d:%s\n",
		       step, step_str[step],
		       expected,
		       step_str[expected]);
		return false;
	}
	return true;
}

static void stm32mp1_do_info(struct ddr_info *priv,
			     struct stm32mp1_ddr_config *config,
			     enum stm32mp1_ddr_interact_step step,
			     int argc, char * const argv[])
{
	unsigned long value;
	static char *ddr_name;

	if (argc == 1) {
		printf("step = %d : %s\n", step, step_str[step]);
		printf("name = %s\n", config->info.name);
		printf("size = 0x%x\n", config->info.size);
		printf("speed = %d kHz\n", config->info.speed);
		return;
	}

	if (argc < 3) {
		printf("no enought parameter\n");
		return;
	}
	if (!strcmp(argv[1], "name")) {
		u32 i, name_len = 0;

		for (i = 2; i < argc; i++)
			name_len += strlen(argv[i]) + 1;
		if (ddr_name)
			free(ddr_name);
		ddr_name = malloc(name_len);
		config->info.name = ddr_name;
		if (!ddr_name) {
			printf("alloc error, length %d\n", name_len);
			return;
		}
		strcpy(ddr_name, argv[2]);
		for (i = 3; i < argc; i++) {
			strcat(ddr_name, " ");
			strcat(ddr_name, argv[i]);
		}
		printf("name = %s\n", ddr_name);
		return;
	}
	if (!strcmp(argv[1], "size")) {
		if (strict_strtoul(argv[2], 16, &value) < 0) {
			printf("invalid value %s\n", argv[2]);
		} else {
			config->info.size = value;
			printf("size = 0x%x\n", config->info.size);
		}
		return;
	}
	if (!strcmp(argv[1], "speed")) {
		if (strict_strtoul(argv[2], 10, &value) < 0) {
			printf("invalid value %s\n", argv[2]);
		} else {
			config->info.speed = value;
			printf("speed = %d kHz\n", config->info.speed);
			value = clk_get_rate(&priv->clk);
			printf("DDRPHY = %ld kHz\n", value / 1000);
		}
		return;
	}
	printf("argument %s invalid\n", argv[1]);
}

static bool stm32mp1_do_freq(struct ddr_info *priv,
			     int argc, char * const argv[])
{
	unsigned long ddrphy_clk;

	if (argc == 2) {
		if (strict_strtoul(argv[1], 0, &ddrphy_clk) < 0) {
			printf("invalid argument %s", argv[1]);
			return false;
		}
		if (clk_set_rate(&priv->clk, ddrphy_clk * 1000)) {
			printf("ERROR: update failed!\n");
			return false;
		}
	}
	ddrphy_clk = clk_get_rate(&priv->clk);
	printf("DDRPHY = %ld kHz\n", ddrphy_clk / 1000);
	if (argc == 2)
		return true;
	return false;
}

static void stm32mp1_do_param(enum stm32mp1_ddr_interact_step step,
			      const struct stm32mp1_ddr_config *config,
			      int argc, char * const argv[])
{
	switch (argc) {
	case 1:
		stm32mp1_dump_param(config, NULL);
		break;
	case 2:
		if (stm32mp1_dump_param(config, argv[1]))
			printf("invalid argument %s\n",
			       argv[1]);
		break;
	case 3:
		if (!stm32mp1_check_step(step, STEP_DDR_RESET))
			return;
		stm32mp1_edit_param(config, argv[1], argv[2]);
		break;
	}
}

static void stm32mp1_do_print(struct ddr_info *priv,
			      int argc, char * const argv[])
{
	switch (argc) {
	case 1:
		stm32mp1_dump_reg(priv, NULL);
		break;
	case 2:
		if (stm32mp1_dump_reg(priv, argv[1]))
			printf("invalid argument %s\n",
			       argv[1]);
		break;
	}
}

static int stm32mp1_do_step(enum stm32mp1_ddr_interact_step step,
			    int argc, char * const argv[])
{
	int i;
	unsigned long value;

	switch (argc) {
	case 1:
		for (i = 0; i < ARRAY_SIZE(step_str); i++)
			printf("%d:%s\n", i, step_str[i]);
		break;

	case 2:
		if ((strict_strtoul(argv[1], 0,
				    &value) < 0) ||
				    value >= ARRAY_SIZE(step_str)) {
			printf("invalid argument %s\n",
			       argv[1]);
			goto end;
		}

		if (value != STEP_DDR_RESET &&
		    value <= step) {
			printf("invalid target %d:%s, current step is %d:%s\n",
			       (int)value, step_str[value],
			       step, step_str[step]);
			goto end;
		}
		printf("step to %d:%s\n",
		       (int)value, step_str[value]);
		return (int)value;
	};

end:
	return step;
}

#if defined(CONFIG_STM32MP1_DDR_TESTS) || defined(CONFIG_STM32MP1_DDR_TUNING)
static const char * const s_result[] = {
		[TEST_PASSED] = "Pass",
		[TEST_FAILED] = "Failed",
		[TEST_ERROR] = "Error"
};

static void stm32mp1_ddr_subcmd(struct ddr_info *priv,
				int argc, char *argv[],
				const struct test_desc array[],
				const int array_nb)
{
	int i;
	unsigned long value;
	int result;
	char string[50] = "";

	if (argc == 1) {
		printf("%s:%d\n", argv[0], array_nb);
		for (i = 0; i < array_nb; i++)
			printf("%d:%s:%s\n",
			       i, array[i].name, array[i].usage);
		return;
	}
	if (argc > 1 && !strcmp(argv[1], "help")) {
		printf("%s:%d\n", argv[0], array_nb);
		for (i = 0; i < array_nb; i++)
			printf("%d:%s:%s:%s\n", i,
			       array[i].name, array[i].usage, array[i].help);
		return;
	}

	if ((strict_strtoul(argv[1], 0, &value) <  0) ||
	    value >= array_nb) {
		sprintf(string, "invalid argument %s",
			argv[1]);
		result = TEST_FAILED;
		goto end;
	}

	if (argc > (array[value].max_args + 2)) {
		sprintf(string, "invalid nb of args %d, max %d",
			argc - 2, array[value].max_args);
		result = TEST_FAILED;
		goto end;
	}

	printf("execute %d:%s\n", (int)value, array[value].name);
	clear_ctrlc();
	result = array[value].fct(priv->ctl, priv->phy,
				  string, argc - 2, &argv[2]);

end:
	printf("Result: %s [%s]\n", s_result[result], string);
}
#endif

bool stm32mp1_ddr_interactive(void *priv,
			      enum stm32mp1_ddr_interact_step step,
			      const struct stm32mp1_ddr_config *config)
{
	const char *prompt = "DDR>";
	char buffer[CONFIG_SYS_CBSIZE];
	char *argv[CONFIG_SYS_MAXARGS + 1];	/* NULL terminated */
	int argc;
	static int next_step = -1;

	if (next_step < 0 && step == STEP_DDR_RESET) {
#ifdef CONFIG_STM32MP1_DDR_INTERACTIVE_FORCE
		gd->flags &= ~(GD_FLG_SILENT |
			       GD_FLG_DISABLE_CONSOLE);
		next_step = STEP_DDR_RESET;
#else
		unsigned long start = get_timer(0);

		while (1) {
			if (tstc() && (getc() == 'd')) {
				next_step = STEP_DDR_RESET;
				break;
			}
			if (get_timer(start) > 100)
				break;
		}
#endif
	}

	debug("** step %d ** %s / %d\n", step, step_str[step], next_step);

	if (next_step < 0)
		return false;

	if (step < 0 || step > ARRAY_SIZE(step_str)) {
		printf("** step %d ** INVALID\n", step);
		return false;
	}

	printf("%d:%s\n", step, step_str[step]);
	printf("%s\n", prompt);

	if (next_step > step)
		return false;

	while (next_step == step) {
		cli_readline_into_buffer(prompt, buffer, 0);
		argc = cli_simple_parse_line(buffer, argv);
		if (!argc)
			continue;

		switch (stm32mp1_get_command(argv[0], argc)) {
		case DDR_CMD_HELP:
			stm32mp1_do_usage();
			break;

		case DDR_CMD_INFO:
			stm32mp1_do_info(priv,
					 (struct stm32mp1_ddr_config *)config,
					 step, argc, argv);
			break;

		case DDR_CMD_FREQ:
			if (stm32mp1_do_freq(priv, argc, argv))
				next_step = STEP_DDR_RESET;
			break;

		case DDR_CMD_RESET:
			do_reset(NULL, 0, 0, NULL);
			break;

		case DDR_CMD_PARAM:
			stm32mp1_do_param(step, config, argc, argv);
			break;

		case DDR_CMD_PRINT:
			stm32mp1_do_print(priv, argc, argv);
			break;

		case DDR_CMD_EDIT:
			stm32mp1_edit_reg(priv, argv[1], argv[2]);
			break;

		case DDR_CMD_GO:
			next_step = STEP_RUN;
			break;

		case DDR_CMD_NEXT:
			next_step = step + 1;
			break;

		case DDR_CMD_STEP:
			next_step = stm32mp1_do_step(step, argc, argv);
			break;

#ifdef CONFIG_STM32MP1_DDR_TESTS
		case DDR_CMD_TEST:
			if (!stm32mp1_check_step(step, STEP_DDR_READY))
				continue;
			stm32mp1_ddr_subcmd(priv, argc, argv, test, test_nb);
			break;
#endif

#ifdef CONFIG_STM32MP1_DDR_TUNING
		case DDR_CMD_TUNING:
			if (!stm32mp1_check_step(step, STEP_DDR_READY))
				continue;
			stm32mp1_ddr_subcmd(priv, argc, argv,
					    tuning, tuning_nb);
			break;
#endif

		default:
			break;
		}
	}
	return next_step == STEP_DDR_RESET;
}
