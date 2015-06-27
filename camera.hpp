#pragma once

#include <vector>

#include "color.hpp"
#include "vector.hpp"

enum {
	CAM_FWD_DIR = 0,
	CAM_RGT_DIR = 1,
	CAM_UPW_DIR = 2,
};

class t_camera {
public:
	t_camera() {
		m_fov = 0.0f;

		m_view_size_x = 0;
		m_view_size_y = 0;

		update(t_vector(1.0f, 1.0f, -1.0f).normalize_xyz());
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

	void update(const t_vector& dir) {
		// (re)compose the camera's coordinate-system
		m_dir[CAM_FWD_DIR] = dir;
		m_dir[CAM_RGT_DIR] = (m_dir[CAM_FWD_DIR] ^ t_vector(0.0f, 0.0f, 1.0f)).normalize_xyz();
		m_dir[CAM_UPW_DIR] = (m_dir[CAM_RGT_DIR] ^ m_dir[CAM_FWD_DIR]).normalize_xyz();
	}

	const t_vector& pos(              ) const { return m_pos;      }
	const t_vector& dir(size_t idx = 0) const { return m_dir[idx]; }
	      t_vector& pos(              )       { return m_pos;      }
	      t_vector& dir(size_t idx = 0)       { return m_dir[idx]; }

	float  fov() const { return m_fov; }
	float& fov()       { return m_fov; }

	float aspect() const { return (m_view_size_x * 1.0f / m_view_size_y); }

	size_t get_view_size_x() const { return m_view_size_x; }
	size_t get_view_size_y() const { return m_view_size_y; }

private:
	t_vector m_pos;
	t_vector m_dir[3];

	// horizontal FOV in radians
	float m_fov;

	size_t m_view_size_x;
	size_t m_view_size_y;

private:
	std::vector<t_color> m_image;
};

