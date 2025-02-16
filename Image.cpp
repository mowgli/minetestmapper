#include <cstdio>
#include <cerrno>
#include <cstring>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <gd.h>
#include <gdfontmb.h>

#include "Image.h"

#ifndef NDEBUG
#define SIZECHECK(x, y) check_bounds((x), (y), m_width, m_height)
#else
#define SIZECHECK(x, y) do {} while(0)
#endif

// ARGB but with inverted alpha

static inline int color2int(const Color &c)
{
	u8 a = (255 - c.a) * gdAlphaMax / 255;
	return (a << 24) | (c.r << 16) | (c.g << 8) | c.b;
}

static inline Color int2color(int c)
{
	Color c2;
	c2.b = c & 0xff;
	c2.g = (c >> 8) & 0xff;
	c2.r = (c >> 16) & 0xff;
	u8 a = (c >> 24) & 0xff;
	c2.a = 255 - (a*255 / gdAlphaMax);
	return c2;
}

#ifndef NDEBUG
static inline void check_bounds(int x, int y, int width, int height)
{
	if(x < 0 || x >= width) {
		std::ostringstream oss;
		oss << "Access outside image bounds (x), 0 < "
			<< x << " < " << width << " is false.";
		throw std::out_of_range(oss.str());
	}
	if(y < 0 || y >= height) {
		std::ostringstream oss;
		oss << "Access outside image bounds (y), 0 < "
			<< y << " < " << height << " is false.";
		throw std::out_of_range(oss.str());
	}
}
#endif


Image::Image(int width, int height) :
	m_width(width), m_height(height), m_image(nullptr)
{
	SIZECHECK(0, 0);
	m_image = gdImageCreateTrueColor(m_width, m_height);
}

Image::~Image()
{
	gdImageDestroy(m_image);
}

void Image::setPixel(int x, int y, const Color &c)
{
	SIZECHECK(x, y);
	m_image->tpixels[y][x] = color2int(c);
}

Color Image::getPixel(int x, int y)
{
	SIZECHECK(x, y);
	return int2color(m_image->tpixels[y][x]);
}

void Image::drawLine(int x1, int y1, int x2, int y2, const Color &c)
{
	SIZECHECK(x1, y1);
	SIZECHECK(x2, y2);
	gdImageLine(m_image, x1, y1, x2, y2, color2int(c));
}

void Image::drawText(int x, int y, const std::string &s, const Color &c)
{
	SIZECHECK(x, y);
	int brect[8];
	gdImageStringFT(m_image, &brect[0], color2int(c), "dejavu/DejaVuSans-Bold;freefont/FreeSansBold;msttcorefonts/Arial", 13., 0., x, y, s.c_str());
}

void Image::drawFilledRect(int x, int y, int w, int h, const Color &c)
{
	SIZECHECK(x, y);
	SIZECHECK(x + w - 1, y + h - 1);
	gdImageFilledRectangle(m_image, x, y, x + w - 1, y + h - 1, color2int(c));
}

void Image::drawCircle(int x, int y, int diameter, const Color &c)
{
	SIZECHECK(x, y);
	gdImageArc(m_image, x, y, diameter, diameter, 0, 360, color2int(c));
}

void Image::save(const std::string &filename)
{
#if (GD_MAJOR_VERSION == 2 && GD_MINOR_VERSION == 1 && GD_RELEASE_VERSION >= 1) || (GD_MAJOR_VERSION == 2 && GD_MINOR_VERSION > 1) || GD_MAJOR_VERSION > 2
	const char *f = filename.c_str();
	if (gdSupportsFileType(f, 1) == GD_FALSE)
		throw std::runtime_error("Image format not supported by gd");
	if (gdImageFile(m_image, f) == GD_FALSE)
		throw std::runtime_error("Error saving image");
#else
	if (filename.compare(filename.length() - 4, 4, ".png") != 0)
		throw std::runtime_error("Only PNG is supported");
	FILE *f = fopen(filename.c_str(), "wb");
	if (!f) {
		std::ostringstream oss;
		oss << "Error opening image file: " << std::strerror(errno);
		throw std::runtime_error(oss.str());
	}
	gdImagePng(m_image, f);
	fclose(f);
#endif
}
