#pragma once

#include "vector.hpp"

class t_ray {
public:
	t_ray() {}
	t_ray(t_const_vec pos, t_const_vec dir, float tmax = -1.0f) {
		m_pos = pos;
		m_dir = dir;
		m_tmax = tmax;
	}

	t_vector point(float t) const { return (m_pos + m_dir * t); }

	bool is_in_range(float t) const { return (t > 0.0f && (t < m_tmax || m_tmax < 0.0f)); }
	bool time_in_rect(float& tmin, float& tmax,  float xmin, float xmax, float ymin, float ymax) const;

	float time_to_x(float x) const { return ((x - m_pos.x()) / m_dir.x()); }
	float time_to_y(float y) const { return ((y - m_pos.y()) / m_dir.y()); }


	const t_vector& pos() const { return m_pos; }
	const t_vector& dir() const { return m_dir; }

	float& tmax() { return m_tmax; }

private:
	t_vector m_pos; // origin
	t_vector m_dir; // direction

	// defines our segment length
	float m_tmax;
};

