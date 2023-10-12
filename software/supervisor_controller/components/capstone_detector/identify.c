/* quirc - QR-code recognition library
 * Copyright (C) 2010-2012 Daniel Beer <dlbeer@gmail.com>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <limits.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#ifdef QUIRC_USE_TGMATH
#include <tgmath.h>
#else
#include <math.h>
#endif // QUIRC_USE_TGMATH
#include "quirc_internal.h"

// This value is faster and gives better results than original otsu adaptative filter
const uint8_t PIXEL_THRESHOLD = 50;

/************************************************************************
 * Linear algebra routines
 */

static void perspective_setup(quirc_float_t *c,
			      const struct quirc_point *rect,
			      quirc_float_t w, quirc_float_t h)
{
	quirc_float_t x0 = rect[0].x;
	quirc_float_t y0 = rect[0].y;
	quirc_float_t x1 = rect[1].x;
	quirc_float_t y1 = rect[1].y;
	quirc_float_t x2 = rect[2].x;
	quirc_float_t y2 = rect[2].y;
	quirc_float_t x3 = rect[3].x;
	quirc_float_t y3 = rect[3].y;

	quirc_float_t wden = w * (x2*y3 - x3*y2 + (x3-x2)*y1 + x1*(y2-y3));
	quirc_float_t hden = h * (x2*y3 + x1*(y2-y3) - x3*y2 + (x3-x2)*y1);

	c[0] = (x1*(x2*y3-x3*y2) + x0*(-x2*y3+x3*y2+(x2-x3)*y1) +
		x1*(x3-x2)*y0) / wden;
	c[1] = -(x0*(x2*y3+x1*(y2-y3)-x2*y1) - x1*x3*y2 + x2*x3*y1
		 + (x1*x3-x2*x3)*y0) / hden;
	c[2] = x0;
	c[3] = (y0*(x1*(y3-y2)-x2*y3+x3*y2) + y1*(x2*y3-x3*y2) +
		x0*y1*(y2-y3)) / wden;
	c[4] = (x0*(y1*y3-y2*y3) + x1*y2*y3 - x2*y1*y3 +
		y0*(x3*y2-x1*y2+(x2-x3)*y1)) / hden;
	c[5] = y0;
	c[6] = (x1*(y3-y2) + x0*(y2-y3) + (x2-x3)*y1 + (x3-x2)*y0) / wden;
	c[7] = (-x2*y3 + x1*y3 + x3*y2 + x0*(y1-y2) - x3*y1 + (x2-x1)*y0) /
		hden;
}

static void perspective_map(const quirc_float_t *c,
			    quirc_float_t u, quirc_float_t v, struct quirc_point *ret)
{
	quirc_float_t den = c[6]*u + c[7]*v + 1.0;
	quirc_float_t x = (c[0]*u + c[1]*v + c[2]) / den;
	quirc_float_t y = (c[3]*u + c[4]*v + c[5]) / den;

	ret->x = (int) rint(x);
	ret->y = (int) rint(y);
}

static void perspective_unmap(const quirc_float_t *c,
			      const struct quirc_point *in,
			      quirc_float_t *u, quirc_float_t *v)
{
	quirc_float_t x = in->x;
	quirc_float_t y = in->y;
	quirc_float_t den = -c[0]*c[7]*y + c[1]*c[6]*y + (c[3]*c[7]-c[4]*c[6])*x +
		c[0]*c[4] - c[1]*c[3];

	*u = -(c[1]*(y-c[5]) - c[2]*c[7]*y + (c[5]*c[7]-c[4])*x + c[2]*c[4]) /
		den;
	*v = (c[0]*(y-c[5]) - c[2]*c[6]*y + (c[5]*c[6]-c[3])*x + c[2]*c[3]) /
		den;
}

/************************************************************************
 * Span-based floodfill routine
 */

typedef void (*span_func_t)(void *user_data, int y, int left, int right);

static void flood_fill_line(struct quirc *q, int x, int y,
			    int from, int to,
			    span_func_t func, void *user_data,
			    int *leftp, int *rightp)
{
	quirc_pixel_t *row;
	int left;
	int right;
	int i;

	row = q->pixels + y * q->w;
	QUIRC_ASSERT(row[x] == from);

	left = x;
	right = x;

	while (left > 0 && row[left - 1] == from)
		left--;

	while (right < q->w - 1 && row[right + 1] == from)
		right++;

	/* Fill the extent */
	for (i = left; i <= right; i++)
		row[i] = to;

	/* Return the processed range */
	*leftp = left;
	*rightp = right;

	if (func)
		func(user_data, y, left, right);
}

static struct quirc_flood_fill_vars *flood_fill_call_next(
			struct quirc *q,
			quirc_pixel_t *row,
			int from, int to,
			span_func_t func, void *user_data,
			struct quirc_flood_fill_vars *vars,
			int direction)
{
	int *leftp;

	if (direction < 0) {
		leftp = &vars->left_up;
	} else {
		leftp = &vars->left_down;
	}

	while (*leftp <= vars->right) {
		if (row[*leftp] == from) {
			struct quirc_flood_fill_vars *next_vars;
			int next_left;

			/* Set up the next context */
			next_vars = vars + 1;
			next_vars->y = vars->y + direction;

			/* Fill the extent */
			flood_fill_line(q,
					*leftp,
					next_vars->y,
					from, to,
					func, user_data,
					&next_left,
					&next_vars->right);
			next_vars->left_down = next_left;
			next_vars->left_up = next_left;

			return next_vars;
		}
		(*leftp)++;
	}
	return NULL;
}

static void flood_fill_seed(struct quirc *q,
			    int x0, int y0,
			    int from, int to,
			    span_func_t func, void *user_data)
{
	struct quirc_flood_fill_vars *const stack = q->flood_fill_vars;
	const size_t stack_size = q->num_flood_fill_vars;
	const struct quirc_flood_fill_vars *const last_vars =
	    &stack[stack_size - 1];

	QUIRC_ASSERT(from != to);
	QUIRC_ASSERT(q->pixels[y0 * q->w + x0] == from);

	struct quirc_flood_fill_vars *next_vars;
	int next_left;

	/* Set up the first context  */
	next_vars = stack;
	next_vars->y = y0;

	/* Fill the extent */
	flood_fill_line(q, x0, next_vars->y, from, to,
			func, user_data,
			&next_left, &next_vars->right);
	next_vars->left_down = next_left;
	next_vars->left_up = next_left;

	while (true) {
		struct quirc_flood_fill_vars * const vars = next_vars;
		quirc_pixel_t *row;

		if (vars == last_vars) {
			/*
			 * "Stack overflow".
			 * Just stop and return.
			 * This can be caused by very complex shapes in
			 * the image, which is not likely a part of
			 * a valid QR code anyway.
			 */
			break;
		}

		/* Seed new flood-fills */
		if (vars->y > 0) {
			row = q->pixels + (vars->y - 1) * q->w;

			next_vars = flood_fill_call_next(q, row,
							 from, to,
							 func, user_data,
							 vars, -1);
			if (next_vars != NULL) {
				continue;
			}
		}

		if (vars->y < q->h - 1) {
			row = q->pixels + (vars->y + 1) * q->w;

			next_vars = flood_fill_call_next(q, row,
							 from, to,
							 func, user_data,
							 vars, 1);
			if (next_vars != NULL) {
				continue;
			}
		}

		if (vars > stack) {
			/* Restore the previous context */
			next_vars = vars - 1;
			continue;
		}

		/* We've done. */
		break;
	}
}

static void area_count(void *user_data, int y, int left, int right)
{
	((struct quirc_region *)user_data)->count += right - left + 1;
}

static int region_code(struct quirc *q, int x, int y)
{
	int pixel;
	struct quirc_region *box;
	int region;

	if (x < 0 || y < 0 || x >= q->w || y >= q->h)
		return -1;

	pixel = q->pixels[y * q->w + x];

	if (pixel >= QUIRC_PIXEL_REGION)
		return pixel;

	if (pixel == QUIRC_PIXEL_WHITE)
		return -1;

	if (q->num_regions >= QUIRC_MAX_REGIONS)
		return -1;

	region = q->num_regions;
	box = &q->regions[q->num_regions++];

	memset(box, 0, sizeof(*box));

	box->seed.x = x;
	box->seed.y = y;
	box->capstone = -1;

	flood_fill_seed(q, x, y, pixel, region, area_count, box);

	return region;
}

struct polygon_score_data {
	struct quirc_point	ref;

	int			scores[4];
	struct quirc_point	*corners;
};

static void find_one_corner(void *user_data, int y, int left, int right)
{
	struct polygon_score_data *psd =
		(struct polygon_score_data *)user_data;
	int xs[2] = {left, right};
	int dy = y - psd->ref.y;
	int i;

	for (i = 0; i < 2; i++) {
		int dx = xs[i] - psd->ref.x;
		int d = dx * dx + dy * dy;

		if (d > psd->scores[0]) {
			psd->scores[0] = d;
			psd->corners[0].x = xs[i];
			psd->corners[0].y = y;
		}
	}
}

static void find_other_corners(void *user_data, int y, int left, int right)
{
	struct polygon_score_data *psd =
		(struct polygon_score_data *)user_data;
	int xs[2] = {left, right};
	int i;

	for (i = 0; i < 2; i++) {
		int up = xs[i] * psd->ref.x + y * psd->ref.y;
		int right = xs[i] * -psd->ref.y + y * psd->ref.x;
		int scores[4] = {up, right, -up, -right};
		int j;

		for (j = 0; j < 4; j++) {
			if (scores[j] > psd->scores[j]) {
				psd->scores[j] = scores[j];
				psd->corners[j].x = xs[i];
				psd->corners[j].y = y;
			}
		}
	}
}

static void find_region_corners(struct quirc *q,
				int rcode, const struct quirc_point *ref,
				struct quirc_point *corners)
{
	struct quirc_region *region = &q->regions[rcode];
	struct polygon_score_data psd;
	int i;

	memset(&psd, 0, sizeof(psd));
	psd.corners = corners;

	memcpy(&psd.ref, ref, sizeof(psd.ref));
	psd.scores[0] = -1;
	flood_fill_seed(q, region->seed.x, region->seed.y,
			rcode, QUIRC_PIXEL_BLACK,
			find_one_corner, &psd);

	psd.ref.x = psd.corners[0].x - psd.ref.x;
	psd.ref.y = psd.corners[0].y - psd.ref.y;

	for (i = 0; i < 4; i++)
		memcpy(&psd.corners[i], &region->seed,
		       sizeof(psd.corners[i]));

	i = region->seed.x * psd.ref.x + region->seed.y * psd.ref.y;
	psd.scores[0] = i;
	psd.scores[2] = -i;
	i = region->seed.x * -psd.ref.y + region->seed.y * psd.ref.x;
	psd.scores[1] = i;
	psd.scores[3] = -i;

	flood_fill_seed(q, region->seed.x, region->seed.y,
			QUIRC_PIXEL_BLACK, rcode,
			find_other_corners, &psd);
}

static void record_capstone(struct quirc *q, int ring, int stone)
{
	struct quirc_region *stone_reg = &q->regions[stone];
	struct quirc_region *ring_reg = &q->regions[ring];
	struct quirc_capstone *capstone;
	int cs_index;

	if (q->num_capstones >= QUIRC_MAX_CAPSTONES)
		return;

	cs_index = q->num_capstones;
	capstone = &q->capstones[q->num_capstones++];

	memset(capstone, 0, sizeof(*capstone));

	capstone->ring = ring;
	capstone->stone = stone;
	stone_reg->capstone = cs_index;
	ring_reg->capstone = cs_index;

	/* Find the corners of the ring */
	find_region_corners(q, ring, &stone_reg->seed, capstone->corners);

	/* Set up the perspective transform and find the center */
	perspective_setup(capstone->c, capstone->corners, 7.0, 7.0);
	perspective_map(capstone->c, 3.5, 3.5, &capstone->center);
}

static void test_capstone(struct quirc *q, unsigned int x, unsigned int y,
			  unsigned int *pb)
{
	int ring_right = region_code(q, x - pb[4], y);
	int stone = region_code(q, x - pb[4] - pb[3] - pb[2], y);
	int ring_left = region_code(q, x - pb[4] - pb[3] -
				    pb[2] - pb[1] - pb[0],
				    y);
	struct quirc_region *stone_reg;
	struct quirc_region *ring_reg;
	unsigned int ratio;

	if (ring_left < 0 || ring_right < 0 || stone < 0)
		return;

	/* Left and ring of ring should be connected */
	if (ring_left != ring_right)
		return;

	/* Ring should be disconnected from stone */
	if (ring_left == stone)
		return;

	stone_reg = &q->regions[stone];
	ring_reg = &q->regions[ring_left];

	/* Already detected */
	if (stone_reg->capstone >= 0 || ring_reg->capstone >= 0)
		return;

	/* Ratio should ideally be 37.5 */
	ratio = stone_reg->count * 100 / ring_reg->count;
	if (ratio < 10 || ratio > 70)
		return;

	record_capstone(q, ring_left, stone);
}

static void finder_scan(struct quirc *q, unsigned int y)
{
	quirc_pixel_t *row = q->pixels + y * q->w;
	unsigned int x;
	int last_color = 0;
	unsigned int run_length = 0;
	unsigned int run_count = 0;
	unsigned int pb[5];

	memset(pb, 0, sizeof(pb));
	for (x = 0; x < q->w; x++) {
		int color = row[x] ? 1 : 0;

		if (x && color != last_color) {
			memmove(pb, pb + 1, sizeof(pb[0]) * 4);
			pb[4] = run_length;
			run_length = 0;
			run_count++;

			if (!color && run_count >= 5) {
				const int scale = 16;
				static const unsigned int check[5] = {1, 1, 3, 1, 1};
				unsigned int avg, err;
				unsigned int i;
				int ok = 1;

				avg = (pb[0] + pb[1] + pb[3] + pb[4]) * scale / 4;
				err = avg * 3 / 4;

				for (i = 0; i < 5; i++)
					if (pb[i] * scale < check[i] * avg - err ||
					    pb[i] * scale > check[i] * avg + err)
						ok = 0;

				if (ok)
					test_capstone(q, x, y, pb);
			}
		}

		run_length++;
		last_color = color;
	}
}

static void pixels_setup(struct quirc *q)
{
	if (QUIRC_PIXEL_ALIAS_IMAGE) {
		q->pixels = (quirc_pixel_t *)q->image;
	}

	uint8_t* source = q->image;
	quirc_pixel_t* dest = q->pixels;
	int length = q->w * q->h;
	while (length--) {
		uint8_t value = *source++;
		*dest++ = (value < PIXEL_THRESHOLD) ? QUIRC_PIXEL_BLACK : QUIRC_PIXEL_WHITE;
	}
}

uint8_t *quirc_begin(struct quirc *q, int *w, int *h)
{
	q->num_regions = QUIRC_PIXEL_REGION;
	q->num_capstones = 0;

	if (w)
		*w = q->w;
	if (h)
		*h = q->h;

	return q->image;
}

void quirc_end(struct quirc *q)
{
	pixels_setup(q);

	for (int i = 0; i < q->h; i++)
		finder_scan(q, i);

}
