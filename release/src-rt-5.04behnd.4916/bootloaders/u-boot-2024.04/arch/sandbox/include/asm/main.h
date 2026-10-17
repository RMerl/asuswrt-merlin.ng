/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2022 Google, Inc.
 * Written by Andrew Scull <ascull@google.com>
 */

#ifndef __ASM_SANDBOX_MAIN_H
#define __ASM_SANDBOX_MAIN_H

/**
 * sandbox_main() - main entrypoint for sandbox
 *
 * @argc:	the number of arguments passed to the program
 * @argv:	array of argc+1 pointers, of which the last one is null
 */
int sandbox_main(int argc, char *argv[]);

#endif /* __ASM_SANDBOX_MAIN_H */
