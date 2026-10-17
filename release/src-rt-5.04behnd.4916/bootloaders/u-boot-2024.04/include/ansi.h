/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2012
 * Pali Rohár <pali@kernel.org>
 */

/*
 * ANSI terminal
 */

#define ANSI_CURSOR_UP			"\e[%dA"
#define ANSI_CURSOR_DOWN		"\e[%dB"
#define ANSI_CURSOR_FORWARD		"\e[%dC"
#define ANSI_CURSOR_BACK		"\e[%dD"
#define ANSI_CURSOR_NEXTLINE		"\e[%dE"
#define ANSI_CURSOR_PREVIOUSLINE	"\e[%dF"
#define ANSI_CURSOR_COLUMN		"\e[%dG"
#define ANSI_CURSOR_POSITION		"\e[%d;%dH"
#define ANSI_CURSOR_SHOW		"\e[?25h"
#define ANSI_CURSOR_HIDE		"\e[?25l"
#define ANSI_CLEAR_CONSOLE		"\e[2J"
#define ANSI_CLEAR_LINE_TO_END		"\e[0K"
#define ANSI_CLEAR_LINE			"\e[2K"
#define ANSI_COLOR_RESET		"\e[0m"
#define ANSI_COLOR_REVERSE		"\e[7m"
