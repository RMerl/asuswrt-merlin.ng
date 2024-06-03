// SPDX-License-Identifier: GPL-2.0+
/*
 * efi_selftest_bitblt
 *
 * Copyright (c) 2017 Heinrich Schuchardt <xypron.glpk@gmx.de>
 *
 * Test the block image transfer in the graphical output protocol.
 * An animated submarine is shown.
 */

#include <efi_selftest.h>

#define WIDTH	200
#define HEIGHT	120
#define DEPTH	 60

static const struct efi_gop_pixel BLACK =	{  0,   0,   0, 0};
static const struct efi_gop_pixel RED =		{  0,   0, 255, 0};
static const struct efi_gop_pixel ORANGE =	{  0, 128, 255, 0};
static const struct efi_gop_pixel YELLOW =	{  0, 255, 255, 0};
static const struct efi_gop_pixel GREEN =	{  0, 255,   0, 0};
static const struct efi_gop_pixel DARK_BLUE =	{128,   0,   0, 0};
static const struct efi_gop_pixel LIGHT_BLUE =	{255, 192, 192, 0};

static struct efi_boot_services *boottime;
static efi_guid_t efi_gop_guid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
static struct efi_gop *gop;
static struct efi_gop_pixel *bitmap;
static struct efi_event *event;
static efi_uintn_t xpos;

static void ellipse(efi_uintn_t x, efi_uintn_t y,
		    efi_uintn_t x0, efi_uintn_t y0,
		    efi_uintn_t x1, efi_uintn_t y1,
		    const struct efi_gop_pixel col, struct efi_gop_pixel *pix)
{
	efi_uintn_t xm = x0 + x1;
	efi_uintn_t ym = y0 + y1;
	efi_uintn_t dx = x1 - x0 + 1;
	efi_uintn_t dy = y1 - y0 + 1;

	if (dy * dy * (2 * x - xm) * (2 * x - xm) +
	    dx * dx * (2 * y - ym) * (2 * y - ym) <= dx * dx * dy * dy)
		*pix = col;
}

static void rectangle(efi_uintn_t x, efi_uintn_t y,
		      efi_uintn_t x0, efi_uintn_t y0,
		      efi_uintn_t x1, efi_uintn_t y1,
		      const struct efi_gop_pixel col, struct efi_gop_pixel *pix)
{
	if (x >= x0 && y >= y0 && x <= x1 && y <= y1)
		*pix = col;
}

/*
 * Notification function, copies image to video.
 * The position is incremented in each call.
 *
 * @event	notified event
 * @context	pointer to the notification count
 */
static void EFIAPI notify(struct efi_event *event, void *context)
{
	efi_uintn_t *pos = context;
	efi_uintn_t dx, sx, width;

	if (!pos)
		return;

	/* Increment position */
	*pos += 5;
	if (*pos >= WIDTH + gop->mode->info->width)
		*pos = 0;

	width = WIDTH;
	dx = *pos - WIDTH;
	sx = 0;
	if (*pos >= gop->mode->info->width) {
		width = WIDTH +  gop->mode->info->width - *pos;
	} else if (*pos < WIDTH) {
		dx = 0;
		sx = WIDTH - *pos;
		width = *pos;
	}

	/* Copy image to video */
	gop->blt(gop, bitmap, EFI_BLT_BUFFER_TO_VIDEO, sx, 0, dx, DEPTH,
		 width, HEIGHT, WIDTH * sizeof(struct efi_gop_pixel));
}

/*
 * Setup unit test.
 *
 * @handle:	handle of the loaded image
 * @systable:	system table
 * @return:	EFI_ST_SUCCESS for success
 */
static int setup(const efi_handle_t handle,
		 const struct efi_system_table *systable)
{
	efi_status_t ret;
	struct efi_gop_pixel pix;
	efi_uintn_t x, y;

	boottime = systable->boottime;

	/* Create event */
	ret = boottime->create_event(EVT_TIMER | EVT_NOTIFY_SIGNAL,
				     TPL_CALLBACK, notify, (void *)&xpos,
				     &event);
	if (ret != EFI_SUCCESS) {
		efi_st_error("could not create event\n");
		return EFI_ST_FAILURE;
	}

	/* Get graphical output protocol */
	ret = boottime->locate_protocol(&efi_gop_guid, NULL, (void **)&gop);
	if (ret != EFI_SUCCESS) {
		gop = NULL;
		efi_st_printf("Graphical output protocol is not available.\n");
		return EFI_ST_SUCCESS;
	}

	/* Prepare image of submarine */
	ret = boottime->allocate_pool(EFI_LOADER_DATA,
				      sizeof(struct efi_gop_pixel) *
				      WIDTH * HEIGHT, (void **)&bitmap);
	if (ret != EFI_SUCCESS) {
		efi_st_error("Out of memory\n");
		return EFI_ST_FAILURE;
	}
	for (y = 0; y < HEIGHT; ++y) {
		for (x = 0; x < WIDTH; ++x) {
			pix = DARK_BLUE;

			/* Propeller */
			ellipse(x, y, 35, 55, 43, 75, BLACK, &pix);
			ellipse(x, y, 36, 56, 42, 74, LIGHT_BLUE, &pix);

			ellipse(x, y, 35, 75, 43, 95, BLACK, &pix);
			ellipse(x, y, 36, 76, 42, 94, LIGHT_BLUE, &pix);

			/* Shaft */
			rectangle(x, y, 35, 73, 100, 77, BLACK, &pix);

			/* Periscope */
			ellipse(x, y, 120, 10, 160, 50, BLACK, &pix);
			ellipse(x, y, 121, 11, 159, 59, YELLOW, &pix);
			ellipse(x, y, 130, 20, 150, 40, BLACK, &pix);
			ellipse(x, y, 131, 21, 149, 49, DARK_BLUE, &pix);
			rectangle(x, y, 135, 10, 160, 50, DARK_BLUE, &pix);
			ellipse(x, y, 132, 10, 138, 20, BLACK, &pix);
			ellipse(x, y, 133, 11, 139, 19, RED, &pix);

			/* Rudder */
			ellipse(x, y, 45, 40, 75, 70, BLACK, &pix);
			ellipse(x, y, 46, 41, 74, 69, ORANGE, &pix);
			ellipse(x, y, 45, 80, 75, 109, BLACK, &pix);
			ellipse(x, y, 46, 81, 74, 108, RED, &pix);

			/* Bridge */
			ellipse(x, y, 100, 30, 120, 50, BLACK, &pix);
			ellipse(x, y, 101, 31, 119, 49, GREEN, &pix);
			ellipse(x, y, 140, 30, 160, 50, BLACK, &pix);
			ellipse(x, y, 141, 31, 159, 49, GREEN, &pix);
			rectangle(x, y, 110, 30, 150, 50, BLACK, &pix);
			rectangle(x, y, 110, 31, 150, 50, GREEN, &pix);

			/* Hull */
			ellipse(x, y, 50, 40, 199, 109, BLACK, &pix);
			ellipse(x, y, 51, 41, 198, 108, LIGHT_BLUE, &pix);

			/* Port holes */
			ellipse(x, y, 79, 57, 109, 82, BLACK, &pix);
			ellipse(x, y, 80, 58, 108, 81, LIGHT_BLUE, &pix);
			ellipse(x, y, 83, 61, 105, 78, BLACK, &pix);
			ellipse(x, y, 84, 62, 104, 77, YELLOW, &pix);
			/*
			 * This port hole is created by copying
			 * ellipse(x, y, 119, 57, 149, 82, BLACK, &pix);
			 * ellipse(x, y, 120, 58, 148, 81, LIGHT_BLUE, &pix);
			 * ellipse(x, y, 123, 61, 145, 78, BLACK, &pix);
			 * ellipse(x, y, 124, 62, 144, 77, YELLOW, &pix);
			 */
			ellipse(x, y, 159, 57, 189, 82, BLACK, &pix);
			ellipse(x, y, 160, 58, 188, 81, LIGHT_BLUE, &pix);
			ellipse(x, y, 163, 61, 185, 78, BLACK, &pix);
			ellipse(x, y, 164, 62, 184, 77, YELLOW, &pix);

			bitmap[WIDTH * y + x] = pix;
		}
	}

	return EFI_ST_SUCCESS;
}

/*
 * Tear down unit test.
 *
 * @return:	EFI_ST_SUCCESS for success
 */
static int teardown(void)
{
	efi_status_t ret;

	if (bitmap) {
		ret = boottime->free_pool(bitmap);
		if (ret != EFI_SUCCESS) {
			efi_st_error("FreePool failed\n");
			return EFI_ST_FAILURE;
		}
	}
	if (event) {
		ret = boottime->close_event(event);
		event = NULL;
		if (ret != EFI_SUCCESS) {
			efi_st_error("could not close event\n");
			return EFI_ST_FAILURE;
		}
	}
	return EFI_ST_SUCCESS;
}

/*
 * Execute unit test.
 *
 * @return:	EFI_ST_SUCCESS for success
 */
static int execute(void)
{
	u32 max_mode;
	efi_status_t ret;
	struct efi_gop_mode_info *info;

	if (!gop)
		return EFI_ST_SUCCESS;

	if (!gop->mode) {
		efi_st_error("EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE missing\n");
		return EFI_ST_FAILURE;
	}
	info = gop->mode->info;
	max_mode = gop->mode->max_mode;
	if (!max_mode) {
		efi_st_error("No graphical mode available\n");
		return EFI_ST_FAILURE;
	}

	/* Fill background */
	ret = gop->blt(gop, bitmap, EFI_BLT_VIDEO_FILL, 0, 0, 0, 0,
		       info->width, info->height, 0);
	if (ret != EFI_SUCCESS) {
		efi_st_error("EFI_BLT_VIDEO_FILL failed\n");
		return EFI_ST_FAILURE;
	}

	/* Copy image to video */
	ret = gop->blt(gop, bitmap, EFI_BLT_BUFFER_TO_VIDEO, 0, 0, 0, DEPTH,
		       WIDTH, HEIGHT, 0);
	if (ret != EFI_SUCCESS) {
		efi_st_error("EFI_BLT_BUFFER_TO_VIDEO failed\n");
		return EFI_ST_FAILURE;
	}

	/* Copy left port hole */
	ret = gop->blt(gop, bitmap, EFI_BLT_VIDEO_TO_VIDEO,
		       79, 57 + DEPTH, 119, 57 + DEPTH,
		       31, 26, 0);
	if (ret != EFI_SUCCESS) {
		efi_st_error("EFI_BLT_VIDEO_TO_VIDEO failed\n");
		return EFI_ST_FAILURE;
	}

	/* Copy port holes back to buffer */
	ret = gop->blt(gop, bitmap, EFI_BLT_VIDEO_TO_BLT_BUFFER,
		       94, 57 + DEPTH, 94, 57,
		       90, 26, WIDTH * sizeof(struct efi_gop_pixel));
	if (ret != EFI_SUCCESS) {
		efi_st_error("EFI_BLT_VIDEO_TO_BLT_BUFFER failed\n");
		return EFI_ST_FAILURE;
	}

	/* Set 250ms timer */
	xpos = WIDTH;
	ret = boottime->set_timer(event, EFI_TIMER_PERIODIC, 250000);
	if (ret != EFI_SUCCESS) {
		efi_st_error("Could not set timer\n");
		return EFI_ST_FAILURE;
	}

	con_out->set_cursor_position(con_out, 0, 0);
	con_out->set_attribute(con_out, EFI_WHITE | EFI_BACKGROUND_BLUE);
	efi_st_printf("The submarine should have three yellow port holes.\n");
	efi_st_printf("Press any key to continue");
	efi_st_get_key();
	con_out->set_attribute(con_out, EFI_LIGHTGRAY);
	efi_st_printf("\n");

	return EFI_ST_SUCCESS;
}

EFI_UNIT_TEST(bitblt) = {
	.name = "block image transfer",
	.phase = EFI_EXECUTE_BEFORE_BOOTTIME_EXIT,
	.setup = setup,
	.execute = execute,
	.teardown = teardown,
	.on_request = true,
};
