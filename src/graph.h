#pragma once

#include <stdlib.h>

using Coord = uint8_t;

enum Color : uint8_t
{
	colBlack,
	colWhite,
	colRed,
	colGreen,
	colBlue,
	colCyan,
	colMagenta,
	colYellow,
	colOrange,
};


template <typename Canvas>
void draw_pixel(Canvas& canvas, Coord x, Coord y, Color color)
{
	canvas.draw_pixel(x, y, color);
}

template <typename Canvas>
void draw_circle(Canvas& canvas, Coord x0, Coord y0, Coord r, Color color)
{
	int16_t f = 1 - r;
	int16_t ddF_x = 1;
	int16_t ddF_y = -2 * r;
	int16_t x = 0;
	int16_t y = r;

	canvas.start();

	canvas.send_pixel(x0, y0 + r, color);
	canvas.send_pixel(x0, y0 - r, color);
	canvas.send_pixel(x0 + r, y0, color);
	canvas.send_pixel(x0 - r, y0, color);

	while (x < y)
	{
		if (f >= 0)
		{
			y--;
			ddF_y += 2;
			f += ddF_y;
		}

		x++;
		ddF_x += 2;
		f += ddF_x;

		canvas.send_pixel(x0 + x, y0 + y, color);
		canvas.send_pixel(x0 - x, y0 + y, color);
		canvas.send_pixel(x0 + x, y0 - y, color);
		canvas.send_pixel(x0 - x, y0 - y, color);
		canvas.send_pixel(x0 + y, y0 + x, color);
		canvas.send_pixel(x0 - y, y0 + x, color);
		canvas.send_pixel(x0 + y, y0 - x, color);
		canvas.send_pixel(x0 - y, y0 - x, color);
	}

	canvas.finish();
}

template <typename Canvas>
void fill_circle(Canvas& canvas, Coord x0, Coord y0, Coord r, Color color)
{
	canvas.start();

	canvas.send_vline(x0, y0 - r, 2 * r + 1, color);

	int16_t f = 1 - r;
	int16_t ddF_x = 1;
	int16_t ddF_y = -2 * r;
	int16_t x = 0;
	int16_t y = r;
	int16_t px = x;
	int16_t py = y;

	while (x < y)
	{
		if (f >= 0)
		{
			y--;
			ddF_y += 2;
			f += ddF_y;
		}

		x++;
		ddF_x += 2;
		f += ddF_x;

		// These checks avoid double-drawing certain lines, important
		// for the SSD1306 library which has an INVERT drawing mode.
		if (x < y + 1)
		{
			canvas.send_vline(x0 + x, y0 - y, 2 * y + 1, color);
			canvas.send_vline(x0 - x, y0 - y, 2 * y + 1, color);
		}

		if (y != py)
		{
			canvas.send_vline(x0 + py, y0 - px, 2 * px + 1, color);
			canvas.send_vline(x0 - py, y0 - px, 2 * px + 1, color);
			py = y;
		}
		px = x;
	}

	canvas.finish();
}

template <typename T>
void swap(T& a1, T& a2)
{
	T temp = a1;
	a1 = a2;
	a2 = temp;
}

template <typename Canvas>
void draw_line(Canvas& canvas, uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, const Color color)
{
	canvas.start();
	
	const bool steep = abs(y1 - y0) > abs(x1 - x0);

	if (steep)
	{
		swap(x0, y0);
		swap(x1, y1);
	}

	if (x0 > x1)
	{
		swap(x0, x1);
		swap(y0, y1);
	}

	int16_t dx = x1 - x0;
	int16_t dy = abs(y1 - y0);

	int16_t err = dx / 2;
	int16_t ystep;

	if (y0 < y1)
		ystep = 1;
	else
		ystep = -1;

	for (; x0 <= x1; x0++)
	{
		if (steep)
			canvas.send_pixel(y0, x0, color);
		else
			canvas.send_pixel(x0, y0, color);

		err -= dy;
		if (err < 0)
		{
			y0 += ystep;
			err += dx;
		}
	}

	canvas.finish();
}

template <typename Canvas>
void fill_rect(Canvas& canvas, Coord x0, Coord y0, Coord w, Coord h, Color color)
{
	canvas.start();
	for (Coord x = x0; x < x0 + w; x++)
		for (Coord y = x0; y < y0 + h; y++)
			canvas.send_pixel(x, y, color);

	canvas.finish();
}

template <typename Canvas>
void draw_raster(Canvas& canvas, const uint8_t* raster, Coord x, Coord y, Coord w, Coord h, Color color, Color bgcolor)
{
	canvas.start();

	canvas.set_addr_window(x, y, w, h);

	Color curr_color = bgcolor;
	while (true)
	{
		const uint8_t num_pixels = pgm_read_byte(raster++);

		if (num_pixels == 0)
			break;

		canvas.send_colors(curr_color, num_pixels);

		if (curr_color == color)
			curr_color = bgcolor;
		else
			curr_color = color;
	}

	canvas.finish();
}
