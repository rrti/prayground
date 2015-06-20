#pragma once

#include "ray.hpp"
#include "vector.hpp"

class t_slope_ray_column {
public:
	// note: <zdirs> must be sorted in ascending order
	t_slope_ray_column(t_const_vec pos, float xdir, float ydir, float* zdirs, int num_rays) {
		m_pos = pos;
		m_ray_xy = t_ray(pos, t_vector(xdir, ydir, 0.0f));
		m_xdir = xdir;
		m_ydir = ydir;
		m_zdirs = zdirs;
		m_num_rays = num_rays;
	}

	t_ray get_ray(int index) const;
	t_ray get_slope_ray(int index) const {
		return (t_ray(m_pos, t_vector(m_xdir, m_ydir, m_zdirs[index])));
	}

	bool rect_indices(t_const_vec min, t_const_vec max, int& start, int& end) const;

	const t_vector& pos() const { return m_pos; }
	const t_ray& ray() const { return m_ray_xy; }

	float xdir() const { return m_xdir; }
	float ydir() const { return m_ydir; }
	float* zdirs() const { return m_zdirs; }

	int num_rays() const { return m_num_rays; }

private:
	t_vector m_pos;
	t_ray m_ray_xy;

	float m_xdir;
	float m_ydir;
	float* m_zdirs;

	int m_num_rays;
};

