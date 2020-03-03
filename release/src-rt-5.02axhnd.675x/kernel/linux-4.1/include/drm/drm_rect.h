/*
 * Copyright (C) 2011-2013 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef DRM_RECT_H
#define DRM_RECT_H

/**
 * DOC: rect utils
 *
 * Utility functions to help manage rectangular areas for
 * clipping, scaling, etc. calculations.
 */

/**
 * struct drm_rect - two dimensional rectangle
 * @x1: horizontal starting coordinate (inclusive)
 * @x2: horizontal ending coordinate (exclusive)
 * @y1: vertical starting coordinate (inclusive)
 * @y2: vertical ending coordinate (exclusive)
 */
struct drm_rect {
	int x1, y1, x2, y2;
};

/**
 * drm_rect_adjust_size - adjust the size of the rectangle
 * @r: rectangle to be adjusted
 * @dw: horizontal adjustment
 * @dh: vertical adjustment
 *
 * Change the size of rectangle @r by @dw in the horizontal direction,
 * and by @dh in the vertical direction, while keeping the center
 * of @r stationary.
 *
 * Positive @dw and @dh increase the size, negative values decrease it.
 */
static inline void drm_rect_adjust_size(struct drm_rect *r, int dw, int dh)
{
	r->x1 -= dw >> 1;
	r->y1 -= dh >> 1;
	r->x2 += (dw + 1) >> 1;
	r->y2 += (dh + 1) >> 1;
}

/**
 * drm_rect_translate - translate the rectangle
 * @r: rectangle to be tranlated
 * @dx: horizontal translation
 * @dy: vertical translation
 *
 * Move rectangle @r by @dx in the horizontal direction,
 * and by @dy in the vertical direction.
 */
static inline void drm_rect_translate(struct drm_rect *r, int dx, int dy)
{
	r->x1 += dx;
	r->y1 += dy;
	r->x2 += dx;
	r->y2 += dy;
}

/**
 * drm_rect_downscale - downscale a rectangle
 * @r: rectangle to be downscaled
 * @horz: horizontal downscale factor
 * @vert: vertical downscale factor
 *
 * Divide the coordinates of rectangle @r by @horz and @vert.
 */
static inline void drm_rect_downscale(struct drm_rect *r, int horz, int vert)
{
	r->x1 /= horz;
	r->y1 /= vert;
	r->x2 /= horz;
	r->y2 /= vert;
}

/**
 * drm_rect_width - determine the rectangle width
 * @r: rectangle whose width is returned
 *
 * RETURNS:
 * The width of the rectangle.
 */
static inline int drm_rect_width(const struct drm_rect *r)
{
	return r->x2 - r->x1;
}

/**
 * drm_rect_height - determine the rectangle height
 * @r: rectangle whose height is returned
 *
 * RETURNS:
 * The height of the rectangle.
 */
static inline int drm_rect_height(const struct drm_rect *r)
{
	return r->y2 - r->y1;
}

/**
 * drm_rect_visible - determine if the the rectangle is visible
 * @r: rectangle whose visibility is returned
 *
 * RETURNS:
 * %true if the rectangle is visible, %false otherwise.
 */
static inline bool drm_rect_visible(const struct drm_rect *r)
{
	return drm_rect_width(r) > 0 && drm_rect_height(r) > 0;
}

/**
 * drm_rect_equals - determine if two rectangles are equal
 * @r1: first rectangle
 * @r2: second rectangle
 *
 * RETURNS:
 * %true if the rectangles are equal, %false otherwise.
 */
static inline bool drm_rect_equals(const struct drm_rect *r1,
				   const struct drm_rect *r2)
{
	return r1->x1 == r2->x1 && r1->x2 == r2->x2 &&
		r1->y1 == r2->y1 && r1->y2 == r2->y2;
}

bool drm_rect_intersect(struct drm_rect *r, const struct drm_rect *clip);
bool drm_rect_clip_scaled(struct drm_rect *src, struct drm_rect *dst,
			  const struct drm_rect *clip,
			  int hscale, int vscale);
int drm_rect_calc_hscale(const struct drm_rect *src,
			 const struct drm_rect *dst,
			 int min_hscale, int max_hscale);
int drm_rect_calc_vscale(const struct drm_rect *src,
			 const struct drm_rect *dst,
			 int min_vscale, int max_vscale);
int drm_rect_calc_hscale_relaxed(struct drm_rect *src,
				 struct drm_rect *dst,
				 int min_hscale, int max_hscale);
int drm_rect_calc_vscale_relaxed(struct drm_rect *src,
				 struct drm_rect *dst,
				 int min_vscale, int max_vscale);
void drm_rect_debug_print(const struct drm_rect *r, bool fixed_point);
void drm_rect_rotate(struct drm_rect *r,
		     int width, int height,
		     unsigned int rotation);
void drm_rect_rotate_inv(struct drm_rect *r,
			 int width, int height,
			 unsigned int rotation);

#endif
