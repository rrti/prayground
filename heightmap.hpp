#pragma once

#include <cstddef>

class FIBITMAP;
class t_heightmap {
public:
	t_heightmap() { m_xsize = 0; m_ysize = 0; m_data = 0; }
	t_heightmap(FIBITMAP* source, float scale) { set_data(source, scale); }
	~t_heightmap() { delete_data(); }

	size_t width() const { return m_xsize; }
	size_t height() const { return m_ysize; }

	float  at(size_t x, size_t y) const { return m_data[y * m_xsize + x]; }
	float& at(size_t x, size_t y)       { return m_data[y * m_xsize + x]; }

	void set_data(FIBITMAP* source, float scale);
	void set_data(const t_heightmap& heightmap);
	void delete_data() { delete[] m_data; m_data = 0; }

	// determines split position for e.g. kd-trees
	void get_opt_split_x(float& score, size_t& split,  size_t xmin, size_t xmax, size_t ymin, size_t ymax) const;
	void get_opt_split_y(float& score, size_t& split,  size_t xmin, size_t xmax, size_t ymin, size_t ymax) const;

private:
	float get_max_height_x(size_t x, size_t ymin, size_t ymax) const;
	float get_min_height_x(size_t x, size_t ymin, size_t ymax) const;
	float get_max_height_y(size_t y, size_t xmin, size_t xmax) const;
	float get_min_height_y(size_t y, size_t xmin, size_t xmax) const;

private:
	size_t m_xsize;
	size_t m_ysize;
	float* m_data;
};

