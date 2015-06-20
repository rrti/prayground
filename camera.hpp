#pragma once

#include <vector>

#include "color.hpp"
#include "vector.hpp"

class t_camera {
public:
	t_camera() {
		set_image_size(640, 480);

		m_fov = 90.0f;
		m_pos = t_vector(0.0f, 0.0f,  8.0f);
		m_dir = t_vector(1.0f, 1.0f, -1.0f);
		m_dir.normalize_xyz();
	}

	void set_image_size(size_t x, size_t y) {
		m_view_size_x = x;
		m_view_size_y = y;

		if ((x * y) != m_image.size()) {
			m_image.clear();
			m_image.resize(x * y);
		}
	}

	void set_image_pixel(size_t x, size_t y, const t_color& c) {
		m_image[y * m_view_size_x + x] = c;
	}

	void draw_image_pixel(size_t x, size_t y, const t_color& c);
	void draw_image();

	const t_vector& pos() const { return m_pos; }
	const t_vector& dir() const { return m_dir; }
	      t_vector& pos()       { return m_pos; }
	      t_vector& dir()       { return m_dir; }

	float  fov() const { return m_fov; }
	float& fov()       { return m_fov; }

	float aspect() const { return (m_view_size_x * 1.0f / m_view_size_y); }

	size_t get_view_size_x() const { return m_view_size_x; }
	size_t get_view_size_y() const { return m_view_size_y; }

private:
	t_vector m_pos;
	t_vector m_dir;

	// horizontal FOV in radians
	float m_fov;

	size_t m_view_size_x;
	size_t m_view_size_y;

private:
	std::vector<t_color> m_image;
};

