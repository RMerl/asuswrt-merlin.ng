/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2015 Google, Inc
 */

#ifndef __video_console_h
#define __video_console_h

#include <video.h>

struct abuf;
struct video_priv;

#define VID_FRAC_DIV	256

#define VID_TO_PIXEL(x)	((x) / VID_FRAC_DIV)
#define VID_TO_POS(x)	((x) * VID_FRAC_DIV)

enum {
	/* cursor width in pixels */
	VIDCONSOLE_CURSOR_WIDTH		= 2,
};

/**
 * struct vidconsole_priv - uclass-private data about a console device
 *
 * Drivers must set up @rows, @cols, @x_charsize, @y_charsize in their probe()
 * method. Drivers may set up @xstart_frac if desired.
 *
 * @sdev:		stdio device, acting as an output sink
 * @xcur_frac:		Current X position, in fractional units (VID_TO_POS(x))
 * @ycur:		Current Y position in pixels (0=top)
 * @rows:		Number of text rows
 * @cols:		Number of text columns
 * @x_charsize:		Character width in pixels
 * @y_charsize:		Character height in pixels
 * @tab_width_frac:	Tab width in fractional units
 * @xsize_frac:		Width of the display in fractional units
 * @xstart_frac:	Left margin for the text console in fractional units
 * @last_ch:		Last character written to the text console on this line
 * @escape:		TRUE if currently accumulating an ANSI escape sequence
 * @escape_len:		Length of accumulated escape sequence so far
 * @col_saved:		Saved X position, in fractional units (VID_TO_POS(x))
 * @row_saved:		Saved Y position in pixels (0=top)
 * @escape_buf:		Buffer to accumulate escape sequence
 */
struct vidconsole_priv {
	struct stdio_dev sdev;
	int xcur_frac;
	int ycur;
	int rows;
	int cols;
	int x_charsize;
	int y_charsize;
	int tab_width_frac;
	int xsize_frac;
	int xstart_frac;
	int last_ch;
	/*
	 * ANSI escape sequences are accumulated character by character,
	 * starting after the ESC char (0x1b) until the entire sequence
	 * is consumed at which point it is acted upon.
	 */
	int escape;
	int escape_len;
	int row_saved;
	int col_saved;
	char escape_buf[32];
};

/**
 * struct vidfont_info - information about a font
 *
 * @name: Font name, e.g. nimbus_sans_l_regular
 */
struct vidfont_info {
	const char *name;
};

/**
 * struct vidconsole_colour - Holds colour information
 *
 * @colour_fg:	Foreground colour (pixel value)
 * @colour_bg:	Background colour (pixel value)
 */
struct vidconsole_colour {
	u32 colour_fg;
	u32 colour_bg;
};

/**
 * struct vidconsole_bbox - Bounding box of text
 *
 * This describes the bounding box of something, measured in pixels. The x0/y0
 * pair is inclusive; the x1/y2 pair is exclusive, meaning that it is one pixel
 * beyond the extent of the object
 *
 * @valid: Values are valid (bounding box is known)
 * @x0: left x position, in pixels from left side
 * @y0: top y position, in pixels from top
 * @x1: right x position + 1
 * @y1: botton y position + 1
 */
struct vidconsole_bbox {
	bool valid;
	int x0;
	int y0;
	int x1;
	int y1;
};

/**
 * struct vidconsole_ops - Video console operations
 *
 * These operations work on either an absolute console position (measured
 * in pixels) or a text row number (measured in rows, where each row consists
 * of an entire line of text - typically 16 pixels).
 */
struct vidconsole_ops {
	/**
	 * putc_xy() - write a single character to a position
	 *
	 * @dev:	Device to write to
	 * @x_frac:	Fractional pixel X position (0=left-most pixel) which
	 *		is the X position multipled by VID_FRAC_DIV.
	 * @y:		Pixel Y position (0=top-most pixel)
	 * @ch:		Character to write
	 * @return number of fractional pixels that the cursor should move,
	 * if all is OK, -EAGAIN if we ran out of space on this line, other -ve
	 * on error
	 */
	int (*putc_xy)(struct udevice *dev, uint x_frac, uint y, char ch);

	/**
	 * move_rows() - Move text rows from one place to another
	 *
	 * @dev:	Device to adjust
	 * @rowdst:	Destination text row (0=top)
	 * @rowsrc:	Source start text row
	 * @count:	Number of text rows to move
	 * @return 0 if OK, -ve on error
	 */
	int (*move_rows)(struct udevice *dev, uint rowdst, uint rowsrc,
			  uint count);

	/**
	 * set_row() - Set the colour of a text row
	 *
	 * Every pixel contained within the text row is adjusted
	 *
	 * @dev:	Device to adjust
	 * @row:	Text row to adjust (0=top)
	 * @clr:	Raw colour (pixel value) to write to each pixel
	 * @return 0 if OK, -ve on error
	 */
	int (*set_row)(struct udevice *dev, uint row, int clr);

	/**
	 * entry_start() - Indicate that text entry is starting afresh
	 *
	 * @dev:	Device to adjust
	 * Returns: 0 on success, -ve on error
	 *
	 * Consoles which use proportional fonts need to track the position of
	 * each character output so that backspace will return to the correct
	 * place. This method signals to the console driver that a new entry
	 * line is being start (e.g. the user pressed return to start a new
	 * command). The driver can use this signal to empty its list of
	 * positions.
	 */
	int (*entry_start)(struct udevice *dev);

	/**
	 * backspace() - Handle erasing the last character
	 *
	 * @dev:	Device to adjust
	 * Returns: 0 on success, -ve on error
	 *
	 * With proportional fonts the vidconsole uclass cannot itself erase
	 * the previous character. This optional method will be called when
	 * a backspace is needed. The driver should erase the previous
	 * character and update the cursor position (xcur_frac, ycur) to the
	 * start of the previous character.
	 *
	 * If not implement, default behaviour will work for fixed-width
	 * characters.
	 */
	int (*backspace)(struct udevice *dev);

	/**
	 * get_font() - Obtain information about a font (optional)
	 *
	 * @dev:	Device to check
	 * @seq:	Font number to query (0=first, 1=second, etc.)
	 * @info:	Returns font information on success
	 * Returns: 0 on success, -ENOENT if no such font
	 */
	int (*get_font)(struct udevice *dev, int seq,
			struct vidfont_info *info);

	/**
	 * get_font_size() - get the current font name and size
	 *
	 * @dev: vidconsole device
	 * @sizep: Place to put the font size (nominal height in pixels)
	 * Returns: Current font name
	 */
	const char *(*get_font_size)(struct udevice *dev, uint *sizep);

	/**
	 * select_font() - Select a particular font by name / size
	 *
	 * @dev:	Device to adjust
	 * @name:	Font name to use (NULL to use default)
	 * @size:	Font size to use (0 to use default)
	 * Returns: 0 on success, -ENOENT if no such font
	 */
	int (*select_font)(struct udevice *dev, const char *name, uint size);

	/**
	 * measure() - Measure the bounds of some text
	 *
	 * @dev:	Device to adjust
	 * @name:	Font name to use (NULL to use default)
	 * @size:	Font size to use (0 to use default)
	 * @text:	Text to measure
	 * @bbox:	Returns bounding box of text, assuming it is positioned
	 *		at 0,0
	 * Returns: 0 on success, -ENOENT if no such font
	 */
	int (*measure)(struct udevice *dev, const char *name, uint size,
		       const char *text, struct vidconsole_bbox *bbox);

	/**
	 * nominal() - Measure the expected width of a line of text
	 *
	 * Uses an average font width and nominal height
	 *
	 * @dev: Console device to use
	 * @name: Font name, NULL for default
	 * @size: Font size, ignored if @name is NULL
	 * @num_chars: Number of characters to use
	 * @bbox: Returns nounding box of @num_chars characters
	 * Returns: 0 if OK, -ve on error
	 */
	int (*nominal)(struct udevice *dev, const char *name, uint size,
		       uint num_chars, struct vidconsole_bbox *bbox);

	/**
	 * entry_save() - Save any text-entry information for later use
	 *
	 * Saves text-entry context such as a list of positions for each
	 * character in the string.
	 *
	 * @dev: Console device to use
	 * @buf: Buffer to hold saved data
	 * Return: 0 if OK, -ENOMEM if out of memory
	 */
	int (*entry_save)(struct udevice *dev, struct abuf *buf);

	/**
	 * entry_restore() - Restore text-entry information for current use
	 *
	 * Restores text-entry context such as a list of positions for each
	 * character in the string.
	 *
	 * @dev: Console device to use
	 * @buf: Buffer containing data to restore
	 * Return: 0 if OK, -ve on error
	 */
	int (*entry_restore)(struct udevice *dev, struct abuf *buf);

	/**
	 * set_cursor_visible() - Show or hide the cursor
	 *
	 * Shows or hides a cursor at the current position
	 *
	 * @dev: Console device to use
	 * @visible: true to show the cursor, false to hide it
	 * @x: X position in pixels
	 * @y: Y position in pixels
	 * @index: Character position (0 = at start)
	 * Return: 0 if OK, -ve on error
	 */
	int (*set_cursor_visible)(struct udevice *dev, bool visible,
				  uint x, uint y, uint index);
};

/* Get a pointer to the driver operations for a video console device */
#define vidconsole_get_ops(dev)  ((struct vidconsole_ops *)(dev)->driver->ops)

/**
 * vidconsole_get_font() - Obtain information about a font
 *
 * @dev:	Device to check
 * @seq:	Font number to query (0=first, 1=second, etc.)
 * @info:	Returns font information on success
 * Returns: 0 on success, -ENOENT if no such font, -ENOSYS if there is no such
 * method
 */
int vidconsole_get_font(struct udevice *dev, int seq,
			struct vidfont_info *info);

/**
 * vidconsole_select_font() - Select a particular font by name / size
 *
 * @dev:	Device to adjust
 * @name:	Font name to use (NULL to use default)
 * @size:	Font size to use (0 to use default)
 */
int vidconsole_select_font(struct udevice *dev, const char *name, uint size);

/*
 * vidconsole_measure() - Measuring the bounding box of some text
 *
 * @dev: Console device to use
 * @name: Font name, NULL for default
 * @size: Font size, ignored if @name is NULL
 * @text: Text to measure
 * @bbox: Returns nounding box of text
 * Returns: 0 if OK, -ve on error
 */
int vidconsole_measure(struct udevice *dev, const char *name, uint size,
		       const char *text, struct vidconsole_bbox *bbox);

/**
 * vidconsole_nominal() - Measure the expected width of a line of text
 *
 * Uses an average font width and nominal height
 *
 * @dev: Console device to use
 * @name: Font name, NULL for default
 * @size: Font size, ignored if @name is NULL
 * @num_chars: Number of characters to use
 * @bbox: Returns nounding box of @num_chars characters
 * Returns: 0 if OK, -ve on error
 */
int vidconsole_nominal(struct udevice *dev, const char *name, uint size,
		       uint num_chars, struct vidconsole_bbox *bbox);

/**
 * vidconsole_entry_save() - Save any text-entry information for later use
 *
 * Saves text-entry context such as a list of positions for each
 * character in the string.
 *
 * @dev: Console device to use
 * @buf: Buffer to hold saved data
 * Return: 0 if OK, -ENOMEM if out of memory
 */
int vidconsole_entry_save(struct udevice *dev, struct abuf *buf);

/**
 * entry_restore() - Restore text-entry information for current use
 *
 * Restores text-entry context such as a list of positions for each
 * character in the string.
 *
 * @dev: Console device to use
 * @buf: Buffer containing data to restore
 * Return: 0 if OK, -ve on error
 */
int vidconsole_entry_restore(struct udevice *dev, struct abuf *buf);

/**
 * vidconsole_set_cursor_visible() - Show or hide the cursor
 *
 * Shows or hides a cursor at the current position
 *
 * @dev: Console device to use
 * @visible: true to show the cursor, false to hide it
 * @x: X position in pixels
 * @y: Y position in pixels
 * @index: Character position (0 = at start)
 * Return: 0 if OK, -ve on error
 */
int vidconsole_set_cursor_visible(struct udevice *dev, bool visible,
				  uint x, uint y, uint index);

/**
 * vidconsole_push_colour() - Temporarily change the font colour
 *
 * @dev:	Device to adjust
 * @fg:		Foreground colour to select
 * @bg:		Background colour to select
 * @old:	Place to store the current colour, so it can be restored
 */
void vidconsole_push_colour(struct udevice *dev, enum colour_idx fg,
			    enum colour_idx bg, struct vidconsole_colour *old);

/**
 * vidconsole_pop_colour() - Restore the original colour
 *
 * @dev:	Device to adjust
 * @old:	Old colour to be restored
 */
void vidconsole_pop_colour(struct udevice *dev, struct vidconsole_colour *old);

/**
 * vidconsole_putc_xy() - write a single character to a position
 *
 * @dev:	Device to write to
 * @x_frac:	Fractional pixel X position (0=left-most pixel) which
 *		is the X position multipled by VID_FRAC_DIV.
 * @y:		Pixel Y position (0=top-most pixel)
 * @ch:		Character to write
 * Return: number of fractional pixels that the cursor should move,
 * if all is OK, -EAGAIN if we ran out of space on this line, other -ve
 * on error
 */
int vidconsole_putc_xy(struct udevice *dev, uint x, uint y, char ch);

/**
 * vidconsole_move_rows() - Move text rows from one place to another
 *
 * @dev:	Device to adjust
 * @rowdst:	Destination text row (0=top)
 * @rowsrc:	Source start text row
 * @count:	Number of text rows to move
 * Return: 0 if OK, -ve on error
 */
int vidconsole_move_rows(struct udevice *dev, uint rowdst, uint rowsrc,
			 uint count);

/**
 * vidconsole_set_row() - Set the colour of a text row
 *
 * Every pixel contained within the text row is adjusted
 *
 * @dev:	Device to adjust
 * @row:	Text row to adjust (0=top)
 * @clr:	Raw colour (pixel value) to write to each pixel
 * Return: 0 if OK, -ve on error
 */
int vidconsole_set_row(struct udevice *dev, uint row, int clr);

/**
 * vidconsole_entry_start() - Set the start position of a vidconsole line
 *
 * Marks the current cursor position as the start of a line
 *
 * @dev:	Device to adjust
 */
int vidconsole_entry_start(struct udevice *dev);

/**
 * vidconsole_put_char() - Output a character to the current console position
 *
 * Outputs a character to the console and advances the cursor. This function
 * handles wrapping to new lines and scrolling the console. Special
 * characters are handled also: \n, \r, \b and \t.
 *
 * The device always starts with the cursor at position 0,0 (top left). It
 * can be adjusted manually using vidconsole_position_cursor().
 *
 * @dev:	Device to adjust
 * @ch:		Character to write
 * Return: 0 if OK, -ve on error
 */
int vidconsole_put_char(struct udevice *dev, char ch);

/**
 * vidconsole_put_string() - Output a string to the current console position
 *
 * Outputs a string to the console and advances the cursor. This function
 * handles wrapping to new lines and scrolling the console. Special
 * characters are handled also: \n, \r, \b and \t.
 *
 * The device always starts with the cursor at position 0,0 (top left). It
 * can be adjusted manually using vidconsole_position_cursor().
 *
 * @dev:	Device to adjust
 * @str:	String to write
 * Return: 0 if OK, -ve on error
 */
int vidconsole_put_string(struct udevice *dev, const char *str);

/**
 * vidconsole_position_cursor() - Move the text cursor
 *
 * @dev:	Device to adjust
 * @col:	New cursor text column
 * @row:	New cursor text row
 * Return: 0 if OK, -ve on error
 */
void vidconsole_position_cursor(struct udevice *dev, unsigned col,
				unsigned row);

/**
 * vidconsole_clear_and_reset() - Clear the console and reset the cursor
 *
 * The cursor is placed at the start of the console
 *
 * @dev:	vidconsole device to adjust
 */
int vidconsole_clear_and_reset(struct udevice *dev);

/**
 * vidconsole_set_cursor_pos() - set cursor position
 *
 * The cursor is set to the new position and the start-of-line information is
 * updated to the same position, so that a newline will return to @x
 *
 * @dev:	video console device to update
 * @x:		x position from left in pixels
 * @y:		y position from top in pixels
 */
void vidconsole_set_cursor_pos(struct udevice *dev, int x, int y);

/**
 * vidconsole_list_fonts() - List the available fonts
 *
 * @dev: vidconsole device to check
 *
 * This shows a list of fonts known by this vidconsole. The list is displayed on
 * the console (not necessarily @dev but probably)
 */
void vidconsole_list_fonts(struct udevice *dev);

/**
 * vidconsole_get_font_size() - get the current font name and size
 *
 * @dev: vidconsole device
 * @sizep: Place to put the font size (nominal height in pixels)
 * @name: pointer to font name, a placeholder for result
 * Return: 0 if OK, -ENOSYS if not implemented in driver
 */
int vidconsole_get_font_size(struct udevice *dev, const char **name, uint *sizep);

#ifdef CONFIG_VIDEO_COPY
/**
 * vidconsole_sync_copy() - Sync back to the copy framebuffer
 *
 * This ensures that the copy framebuffer has the same data as the framebuffer
 * for a particular region. It should be called after the framebuffer is updated
 *
 * @from and @to can be in either order. The region between them is synced.
 *
 * @dev: Vidconsole device being updated
 * @from: Start/end address within the framebuffer (->fb)
 * @to: Other address within the frame buffer
 * Return: 0 if OK, -EFAULT if the start address is before the start of the
 *	frame buffer start
 */
int vidconsole_sync_copy(struct udevice *dev, void *from, void *to);

/**
 * vidconsole_memmove() - Perform a memmove() within the frame buffer
 *
 * This handles a memmove(), e.g. for scrolling. It also updates the copy
 * framebuffer.
 *
 * @dev: Vidconsole device being updated
 * @dst: Destination address within the framebuffer (->fb)
 * @src: Source address within the framebuffer (->fb)
 * @size: Number of bytes to transfer
 * Return: 0 if OK, -EFAULT if the start address is before the start of the
 *	frame buffer start
 */
int vidconsole_memmove(struct udevice *dev, void *dst, const void *src,
		       int size);
#else

#include <string.h>

static inline int vidconsole_sync_copy(struct udevice *dev, void *from,
				       void *to)
{
	return 0;
}

static inline int vidconsole_memmove(struct udevice *dev, void *dst,
				     const void *src, int size)
{
	memmove(dst, src, size);

	return 0;
}

#endif

#endif
