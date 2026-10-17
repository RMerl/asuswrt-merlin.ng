/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2016 Imagination Technologies
 */

#ifndef __BOARD_BOSTON_LCD_H__
#define __BOARD_BOSTON_LCD_H__

/**
 * lowlevel_display() - Display a message on Boston's LCD
 * @msg: The string to display
 *
 * Display the string @msg on the 7 character LCD display of the Boston board.
 * This is typically used for debug or to present some form of status
 * indication to the user, allowing faults to be identified when things go
 * wrong early enough that the UART isn't up.
 */
void lowlevel_display(const char msg[static 8]);

#endif /* __BOARD_BOSTON_LCD_H__ */
