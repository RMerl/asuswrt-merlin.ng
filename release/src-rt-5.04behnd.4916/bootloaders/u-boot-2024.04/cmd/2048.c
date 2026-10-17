// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: © 2014 Maurits van der Schee

/* Console version of the game "2048" for GNU/Linux */

#include <common.h>
#include <cli.h>
#include <command.h>
#include <rand.h>
#include <linux/delay.h>

#define SIZE 4
static uint score;

static void getColor(uint value, char *color, size_t length)
{
	u8 original[] = {
		8, 255, 1, 255, 2, 255, 3, 255,
		4, 255, 5, 255, 6, 255, 7, 255,
		9, 0, 10, 0, 11, 0, 12, 0, 13,
		0, 14, 0, 255, 0, 255, 0};
	u8 *scheme = original;
	u8 *background = scheme + 0;
	u8 *foreground = scheme + 1;

	if (value > 0) {
		while (value >>= 1) {
			if (background + 2 < scheme + sizeof(original)) {
				background += 2;
				foreground += 2;
			}
		}
	}
	snprintf(color, length, "\033[38;5;%d;48;5;%dm", *foreground,
		 *background);
}

static void drawBoard(u16 board[SIZE][SIZE])
{
	int x, y;
	char color[40], reset[] = "\033[0m";

	printf("\033[H");
	printf("2048.c %17d pts\n\n", score);

	for (y = 0; y < SIZE; y++) {
		for (x = 0; x < SIZE; x++) {
			getColor(board[x][y], color, 40);
			printf("%s", color);
			printf("       ");
			printf("%s", reset);
		}
		printf("\n");
		for (x = 0; x < SIZE; x++) {
			getColor(board[x][y], color, 40);
			printf("%s", color);
			if (board[x][y] != 0) {
				char s[8];
				s8 t;

				snprintf(s, 8, "%u", board[x][y]);
				t = 7 - strlen(s);
				printf("%*s%s%*s", t - t / 2, "", s, t / 2, "");
			} else {
				printf("   ·   ");
			}
			printf("%s", reset);
		}
		printf("\n");
		for (x = 0; x < SIZE; x++) {
			getColor(board[x][y], color, 40);
			printf("%s", color);
			printf("       ");
			printf("%s", reset);
		}
		printf("\n");
	}
	printf("\n");
	printf("        ←, ↑, →, ↓ or q        \n");
	printf("\033[A");
}

static int8_t findTarget(u16 array[SIZE], int x, int stop)
{
	int t;

	/* if the position is already on the first, don't evaluate */
	if (x == 0)
		return x;
	for (t = x - 1; t >= 0; t--) {
		if (array[t]) {
			if (array[t] != array[x]) {
				/* merge is not possible, take next position */
				return t + 1;
			}
			return t;
		}

		/* we should not slide further, return this one */
		if (t == stop)
			return t;
	}
	/* we did not find a */
	return x;
}

static bool slideArray(u16 array[SIZE])
{
	bool success = false;
	int x, t, stop = 0;

	for (x = 0; x < SIZE; x++) {
		if (array[x] != 0) {
			t = findTarget(array, x, stop);
			/*
			 * if target is not original position, then move or
			 * merge
			 */
			if (t != x) {
				/*
				 * if target is not zero, set stop to avoid
				 * double merge
				 */
				if (array[t]) {
					score += array[t] + array[x];
					stop = t + 1;
				}
				array[t] += array[x];
				array[x] = 0;
				success = true;
			}
		}
	}
	return success;
}

static void rotateBoard(u16 board[SIZE][SIZE])
{
	s8 i, j, n = SIZE;
	int tmp;

	for (i = 0; i < n / 2; i++) {
		for (j = i; j < n - i - 1; j++) {
			tmp = board[i][j];
			board[i][j] = board[j][n - i - 1];
			board[j][n - i - 1] = board[n - i - 1][n - j - 1];
			board[n - i - 1][n - j - 1] = board[n - j - 1][i];
			board[n - j - 1][i] = tmp;
		}
	}
}

static bool moveUp(u16 board[SIZE][SIZE])
{
	bool success = false;
	int x;

	for (x = 0; x < SIZE; x++)
		success |= slideArray(board[x]);

	return success;
}

static bool moveLeft(u16 board[SIZE][SIZE])
{
	bool success;

	rotateBoard(board);
	success = moveUp(board);
	rotateBoard(board);
	rotateBoard(board);
	rotateBoard(board);
	return success;
}

static bool moveDown(u16 board[SIZE][SIZE])
{
	bool success;

	rotateBoard(board);
	rotateBoard(board);
	success = moveUp(board);
	rotateBoard(board);
	rotateBoard(board);
	return success;
}

static bool moveRight(u16 board[SIZE][SIZE])
{
	bool success;

	rotateBoard(board);
	rotateBoard(board);
	rotateBoard(board);
	success = moveUp(board);
	rotateBoard(board);
	return success;
}

static bool findPairDown(u16 board[SIZE][SIZE])
{
	bool success = false;
	int x, y;

	for (x = 0; x < SIZE; x++) {
		for (y = 0; y < SIZE - 1; y++) {
			if (board[x][y] == board[x][y + 1])
				return true;
		}
	}

	return success;
}

static int16_t countEmpty(u16 board[SIZE][SIZE])
{
	int x, y;
	int count = 0;

	for (x = 0; x < SIZE; x++) {
		for (y = 0; y < SIZE; y++) {
			if (board[x][y] == 0)
				count++;
		}
	}
	return count;
}

static bool gameEnded(u16 board[SIZE][SIZE])
{
	bool ended = true;

	if (countEmpty(board) > 0)
		return false;
	if (findPairDown(board))
		return false;
	rotateBoard(board);
	if (findPairDown(board))
		ended = false;
	rotateBoard(board);
	rotateBoard(board);
	rotateBoard(board);

	return ended;
}

static void addRandom(u16 board[SIZE][SIZE])
{
	int x, y;
	int r, len = 0;
	u16 n, list[SIZE * SIZE][2];

	for (x = 0; x < SIZE; x++) {
		for (y = 0; y < SIZE; y++) {
			if (board[x][y] == 0) {
				list[len][0] = x;
				list[len][1] = y;
				len++;
			}
		}
	}

	if (len > 0) {
		r = rand() % len;
		x = list[r][0];
		y = list[r][1];
		n = ((rand() % 10) / 9 + 1) * 2;
		board[x][y] = n;
	}
}

static int test(void)
{
	u16 array[SIZE];
	u16 data[] = {
		0, 0, 0, 2,	2, 0, 0, 0,
		0, 0, 2, 2,	4, 0, 0, 0,
		0, 2, 0, 2,	4, 0, 0, 0,
		2, 0, 0, 2,	4, 0, 0, 0,
		2, 0, 2, 0,	4, 0, 0, 0,
		2, 2, 2, 0,	4, 2, 0, 0,
		2, 0, 2, 2,	4, 2, 0, 0,
		2, 2, 0, 2,	4, 2, 0, 0,
		2, 2, 2, 2,	4, 4, 0, 0,
		4, 4, 2, 2,	8, 4, 0, 0,
		2, 2, 4, 4,	4, 8, 0, 0,
		8, 0, 2, 2,	8, 4, 0, 0,
		4, 0, 2, 2,	4, 4, 0, 0
	};
	u16 *in, *out;
	u16 t, tests;
	int i;
	bool success = true;

	tests = (sizeof(data) / sizeof(data[0])) / (2 * SIZE);
	for (t = 0; t < tests; t++) {
		in = data + t * 2 * SIZE;
		out = in + SIZE;
		for (i = 0; i < SIZE; i++)
			array[i] = in[i];
		slideArray(array);
		for (i = 0; i < SIZE; i++) {
			if (array[i] != out[i])
				success = false;
		}
		if (!success) {
			for (i = 0; i < SIZE; i++)
				printf("%d ", in[i]);
			printf(" = > ");
			for (i = 0; i < SIZE; i++)
				printf("%d ", array[i]);
			printf("expected ");
			for (i = 0; i < SIZE; i++)
				printf("%d ", in[i]);
			printf(" = > ");
			for (i = 0; i < SIZE; i++)
				printf("%d ", out[i]);
			printf("\n");
			break;
		}
	}
	if (success)
		printf("All %u tests executed successfully\n", tests);

	return !success;
}

static int do_2048(struct cmd_tbl *cmdtp, int flag, int argc,
		   char *const argv[])
{
	struct cli_ch_state cch_s, *cch = &cch_s;
	u16 board[SIZE][SIZE];
	bool success;

	if (argc == 2 && strcmp(argv[1], "test") == 0)
		return test();

	score = 0;

	printf("\033[?25l\033[2J\033[H");

	memset(board, 0, sizeof(board));
	addRandom(board);
	addRandom(board);
	drawBoard(board);
	cli_ch_init(cch);
	while (true) {
		int c;

		c = cli_ch_process(cch, 0);
		if (!c) {
			c = getchar();
			c = cli_ch_process(cch, c);
		}
		switch (c) {
		case CTL_CH('b'): /* left arrow */
			success = moveLeft(board);
			break;
		case CTL_CH('f'): /* right arrow */
			success = moveRight(board);
			break;
		case CTL_CH('p'):/* up arrow */
			success = moveUp(board);
			break;
		case CTL_CH('n'): /* down arrow */
			success = moveDown(board);
			break;
		default:
			success = false;
		}
		if (success) {
			drawBoard(board);
			mdelay(150);
			addRandom(board);
			drawBoard(board);
			if (gameEnded(board)) {
				printf("         GAME OVER          \n");
				break;
			}
		}
		if (c == 'q') {
			printf("            QUIT            \n");
			break;
		}
	}

	printf("\033[?25h");

	return 0;
}

U_BOOT_CMD(
	2048,	2,	1,	do_2048,
	"The 2048 game",
	"Use your arrow keys to move the tiles. When two tiles with "
	"the same number touch, they merge into one!"
);
