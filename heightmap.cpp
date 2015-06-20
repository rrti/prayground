#include <algorithm>
#include <cfloat>
#include <vector>

#include "lib/FreeImage.h"
#include "heightmap.hpp"

void t_heightmap::set_data(FIBITMAP* source, float scale) {
	m_xsize = FreeImage_GetWidth(source);
	m_ysize = FreeImage_GetHeight(source);

	if (m_data != 0)
		delete_data();

	m_data = new float[m_xsize * m_ysize];
    
	for (size_t y = 0; y < m_ysize; y++) {
		for (size_t x = 0; x < m_xsize; x++) {
			RGBQUAD rawColor;
			FreeImage_GetPixelColor(source, x, y, &rawColor);

			at(x, y) = (rawColor.rgbRed / 255.0f + rawColor.rgbGreen / 255.0f + rawColor.rgbBlue / 255.0f) * scale / 3.0f;
		}
	}
}

void t_heightmap::set_data(const t_heightmap& heightmap) {
	m_xsize = heightmap.width();
	m_ysize = heightmap.height();

	if (m_data != 0)
		delete_data();

	m_data = new float[m_xsize * m_ysize];

	for (size_t y = 0; y < m_ysize; y++) {
		for (size_t x = 0; x < m_xsize; x++) {
			at(x, y) = heightmap.at(x, y);
		}
	}
}


float t_heightmap::get_max_height_x(size_t x, size_t ymin, size_t ymax) const {
	float result = -FLT_MAX;

	for (size_t y = ymin; y < ymax; y++)
		result = std::max(result, at(x, y));

	return result;
}

float t_heightmap::get_min_height_x(size_t x, size_t ymin, size_t ymax) const {
	float result = FLT_MAX;

	for (size_t y = ymin; y < ymax; y++)
		result = std::min(result, at(x, y));

	return result;
}


float t_heightmap::get_max_height_y(size_t y, size_t xmin, size_t xmax) const {
	float result = -FLT_MAX;

	for (size_t x = xmin; x < xmax; x++)
		result = std::max(result, at(x, y));

	return result;
}

float t_heightmap::get_min_height_y(size_t y, size_t xmin, size_t xmax) const {
	float result = FLT_MAX;

	for (size_t x = xmin; x < xmax; x++)
		result = std::min(result, at(x, y));

	return result;
}


void t_heightmap::get_opt_split_x(float& score, size_t& split,  size_t xmin, size_t xmax, size_t ymin, size_t ymax) const {
	const size_t dx = xmax - xmin;
	const size_t dy = ymax - ymin;

	std::vector<float> max_height_pos(dx - 2);
	std::vector<float> min_height_pos(dx - 2);

	std::vector<float> max_height_neg(dx - 2);
	std::vector<float> min_height_neg(dx - 2);

	float max_height = get_max_height_x(xmin, ymin, ymax);
	float min_height = get_min_height_x(xmin, ymin, ymax);

	for (size_t i = 1; i < dx - 1; i++) {
		const float cur_max_height = get_max_height_x(xmin + i, ymin, ymax);
		const float cur_min_height = get_min_height_x(xmin + i, ymin, ymax);

		max_height_pos[i - 1] = (max_height = std::max(max_height, cur_max_height));
		min_height_pos[i - 1] = (min_height = std::min(min_height, cur_min_height));
	}

	max_height = get_max_height_x(xmax - 1, ymin, ymax);
	min_height = get_min_height_x(xmax - 1, ymin, ymax);

	for (size_t i = dx - 2; i >= 1; i--) {
		const float cur_max_height = get_max_height_x(xmin + i, ymin, ymax);
		const float cur_min_height = get_min_height_x(xmin + i, ymin, ymax);

		max_height_neg[i - 1] = (max_height = std::max(max_height, cur_max_height));
		min_height_neg[i - 1] = (min_height = std::min(min_height, cur_min_height));
	}

	score = FLT_MAX;

	for (size_t i = 1; i < dx - 1; i++) {
		const float dif_height_pos = (max_height_pos[i - 1] - min_height_pos[i - 1]);
		const float dif_height_neg = (max_height_neg[i - 1] - min_height_neg[i - 1]);

		const float edge_pos = i + dy;
		const float edge_neg = dx - 1 - i + dy;
		const float area_pos = i * dy;
		const float area_neg = (dx - 1 - i) * dy;

		const float score_pos = dif_height_pos * edge_pos + area_pos;
		const float score_neg = dif_height_neg * edge_neg + area_neg;
		const float currScore = score_pos * area_pos + score_neg * area_neg;

		if (currScore < score) {
			score = currScore;
			split = xmin + i;
		}
	}
}

void t_heightmap::get_opt_split_y(float& score, size_t& split,  size_t xmin, size_t xmax, size_t ymin, size_t ymax) const {
	const size_t dy = ymax - ymin;
	const size_t dx = xmax - xmin;

	std::vector<float> max_height_pos(dy - 2);
	std::vector<float> min_height_pos(dy - 2);

	std::vector<float> max_height_neg(dy - 2);
	std::vector<float> min_height_neg(dy - 2);

	float max_height = get_max_height_y(ymin, xmin, xmax);
	float min_height = get_min_height_y(ymin, xmin, xmax);

	for (size_t i = 1; i < dy - 1; i++) {
		const float cur_max_height = get_max_height_y(ymin + i, xmin, xmax);
		const float cur_min_height = get_min_height_y(ymin + i, xmin, xmax);

		max_height_pos[i - 1] = (max_height = std::max(max_height, cur_max_height));
		min_height_pos[i - 1] = (min_height = std::min(min_height, cur_min_height));
	}

	max_height = get_max_height_y(ymax - 1, xmin, xmax);
	min_height = get_min_height_y(ymax - 1, xmin, xmax);

	for (size_t i = dy - 2; i >= 1; i--) {
		const float cur_max_height = get_max_height_y(ymin + i, xmin, xmax);
		const float cur_min_height = get_min_height_y(ymin + i, xmin, xmax);

		max_height_neg[i - 1] = (max_height = std::max(max_height, cur_max_height));
		min_height_neg[i - 1] = (min_height = std::min(min_height, cur_min_height));
	}

	score = FLT_MAX;

	for (size_t i = 1; i < dy - 1; i++) {
		const float dif_height_pos = (max_height_pos[i - 1] - min_height_pos[i - 1]);
		const float dif_height_neg = (max_height_neg[i - 1] - min_height_neg[i - 1]);

		const float edge_pos = i + dx;
		const float edge_neg = dy - 1 - i + dx;
		const float area_pos = i * dx;
		const float area_neg = (dy - 1 - i) * dx;

		const float score_pos = dif_height_pos * edge_pos + area_pos;
		const float score_neg = dif_height_neg * edge_neg + area_neg;
		const float currScore = score_pos * area_pos + score_neg * area_neg;

		if (currScore < score) {
			score = currScore;
			split = ymin + i;
		}
	}
}

