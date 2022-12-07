#pragma once

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
	colLightGray,
	colGray,
	colDarkGray,
};

using ColorRGB = uint16_t;

constexpr ColorRGB colorRGB(uint8_t r, uint8_t g, uint8_t b)
{
	return static_cast<ColorRGB>(((r & 0x1f) << 11) | ((g & 0x3f) << 5) | (b & 0x1f));
}

constexpr const ColorRGB rgbBlack		= colorRGB( 0,  0,  0);
constexpr const ColorRGB rgbWhite		= colorRGB(31, 63, 31);
constexpr const ColorRGB rgbRed			= colorRGB(31,  0,  0);
constexpr const ColorRGB rgbGreen		= colorRGB( 0, 63,  0);
constexpr const ColorRGB rgbBlue		= colorRGB( 0,  0, 31);
constexpr const ColorRGB rgbCyan		= colorRGB( 0, 63, 31);
constexpr const ColorRGB rgbMagenta		= colorRGB(31,  0, 31);
constexpr const ColorRGB rgbYellow		= colorRGB(31, 63,  0);
constexpr const ColorRGB rgbOrange		= colorRGB(31, 32,  0);
constexpr const ColorRGB rgbLightGray	= colorRGB(24, 48, 24);
constexpr const ColorRGB rgbGray		= colorRGB(16, 32, 16);
constexpr const ColorRGB rgbDarkGray	= colorRGB( 8, 16,  8);

constexpr static const ColorRGB Color2RGBMap[] =
{
	rgbBlack,
	rgbWhite,
	rgbRed,
	rgbGreen,
	rgbBlue,
	rgbCyan,
	rgbMagenta,
	rgbYellow,
	rgbOrange,
	rgbLightGray,
	rgbGray,
	rgbDarkGray,
};

constexpr inline ColorRGB color2rgb(const Color col)
{
	return Color2RGBMap[col];
}

template <typename Canvas, typename ColorT>
void draw_pixel(Canvas& canvas, Coord x, Coord y, ColorT color)
{
	[[maybe_unused]] typename Canvas::Transaction t;

	canvas.pixel(x, y, color);
}

template <typename Canvas, typename ColorT>
void draw_circle(Canvas& canvas, Coord x0, Coord y0, Coord r, ColorT color)
{
	[[maybe_unused]] typename Canvas::Transaction t;

	int16_t f = 1 - r;
	int16_t ddF_x = 1;
	int16_t ddF_y = -2 * r;
	int16_t x = 0;
	int16_t y = r;

	canvas.pixel(x0, y0 + r, color);
	canvas.pixel(x0, y0 - r, color);
	canvas.pixel(x0 + r, y0, color);
	canvas.pixel(x0 - r, y0, color);

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

		canvas.pixel(x0 + x, y0 + y, color);
		canvas.pixel(x0 - x, y0 + y, color);
		canvas.pixel(x0 + x, y0 - y, color);
		canvas.pixel(x0 - x, y0 - y, color);
		canvas.pixel(x0 + y, y0 + x, color);
		canvas.pixel(x0 - y, y0 + x, color);
		canvas.pixel(x0 + y, y0 - x, color);
		canvas.pixel(x0 - y, y0 - x, color);
	}
}

template <typename Canvas, typename ColorT>
void fill_circle(Canvas& canvas, Coord x0, Coord y0, Coord r, ColorT color)
{
	[[maybe_unused]] typename Canvas::Transaction t;

	canvas.vline(x0, y0 - r, 2 * r + 1, color);

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
			canvas.vline(x0 + x, y0 - y, 2 * y + 1, color);
			canvas.vline(x0 - x, y0 - y, 2 * y + 1, color);
		}

		if (y != py)
		{
			canvas.vline(x0 + py, y0 - px, 2 * px + 1, color);
			canvas.vline(x0 - py, y0 - px, 2 * px + 1, color);
			py = y;
		}
		px = x;
	}
}

template <typename T>
void swap(T& a1, T& a2)
{
	T temp = a1;
	a1 = a2;
	a2 = temp;
}

template <typename Canvas, typename ColorT>
void draw_line(Canvas& canvas, Coord x0, Coord y0, Coord x1, Coord y1, const ColorT color)
{
	[[maybe_unused]] typename Canvas::Transaction t;

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
			canvas.pixel(y0, x0, color);
		else
			canvas.pixel(x0, y0, color);

		err -= dy;
		if (err < 0)
		{
			y0 += ystep;
			err += dx;
		}
	}
}

template <typename Canvas, typename ColorT>
void fill_rect(Canvas& canvas, Coord x0, Coord y0, Coord w, Coord h, ColorT color)
{
	[[maybe_unused]] typename Canvas::Transaction t;

	for (Coord x = x0; x < x0 + w; x++)
		for (Coord y = x0; y < y0 + h; y++)
			canvas.pixel(x, y, color);
}

template <typename Canvas, typename ColorT>
void draw_raster(Canvas& canvas, const uint8_t* raster, Coord x0, Coord y0, Coord w, Coord h, ColorT color, ColorT bgcolor)
{
	[[maybe_unused]] typename Canvas::Transaction t;

	Color curr_color = bgcolor;

	Coord x = x0;
	Coord y = y0;

	while (true)
	{
		const uint8_t num_pixels = pgm_read_byte(raster++);

		// EOF?
		if (num_pixels == 0)
			break;

		for (uint8_t i = 0; i < num_pixels; i++)
		{
			canvas.pixel(x, y, curr_color);

			if (++x == x0 + w)
			{
				x = x0;
				++y;
			}
		}

		if (curr_color == color)
			curr_color = bgcolor;
		else
			curr_color = color;
	}
}

template <typename Canvas, typename ColorT>
void fill(Canvas& c, ColorT col)
{
	fill_rect(c, 0, 0, Canvas::Width, Canvas::Height, col);
}

template <Coord W, Coord H>
class WindowRGB
{
public:

	static const Coord Width = W;
	static const Coord Height = H;

	struct Transaction {};

	WindowRGB(ColorRGB colbgnd)
	{
		for (ColorRGB& pixel : buffer)
			pixel = colbgnd;
	}

	WindowRGB(Color colbgnd)
		: WindowRGB(color2rgb(colbgnd))
	{}

	void pixel(Coord x, Coord y, ColorRGB color)
	{
		if (x < Width  &&  y < Height)
			buffer[y * Width + x] = color;
	}

	void pixel(Coord x, Coord y, Color color)
	{
		pixel(x, y, color2rgb(color));
	}

	void vline(Coord x, Coord y, Coord len, ColorRGB color)
	{
		for (Coord y1 = y; y1 < y + len; ++y1)
			pixel(x, y1, color);
	}

	void vline(Coord x, Coord y, Coord len, Color color)
	{
		vline(x, y, len, color2rgb(color));
	}

private:

	ColorRGB	buffer[Width * Height];
};

template <Coord W, Coord H>
class Window
{
public:

	static const Coord Width = W;
	static const Coord Height = H;

	struct Transaction {};

	Window(Color colbgnd)
	{
		for (TwoPixels& pixels : buffer)
			pixels = TwoPixels(colbgnd);
	}

	void vline(Coord x, Coord y, Coord len, Color color)
	{
		for (Coord y1 = y; y1 < y + len; ++y1)
			pixel(x, y1, color);
	}

	void pixel(Coord x, Coord y, Color color)
	{
		if (x < Width  &&  y < Height)
		{
			const size_t ndx = Width * y + x;
			if (ndx & 1)
				buffer[ndx / 2].second = color;
			else
				buffer[ndx / 2].first = color;
		}
	}

	Color get_color(Coord x, Coord y)
	{
		const size_t ndx = Width * y + x;
		if (ndx & 1)
			return static_cast<Color>(buffer[ndx / 2].second);

		return static_cast<Color>(buffer[ndx / 2].first);
	}

private:

	struct TwoPixels
	{
		unsigned int	first : 4;
		unsigned int	second : 4;

		TwoPixels() = default;

		TwoPixels(Color col)
			: first(col), second(col)
		{}
	};

	TwoPixels buffer[(Width * Height + 1) / 2];
};
