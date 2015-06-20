#pragma once

#include "ray.hpp"
#include "vector.hpp"

class t_ray_column {
public:
	// note: <dirs> must be sorted in ascending order
	t_ray_column(t_const_vec origin, t_vector* dirs, float xdir, float ydir, int num_rays) {
		m_origin = origin;
		m_dirs = dirs;
		m_xdir = xdir;
		m_ydir = ydir;
		m_num_rays = num_rays;
	}

	const t_vector& pos() const { return m_origin; }
	const t_vector* dirs() const { return m_dirs; }

	float xdir() const { return m_xdir; }
	float ydir() const { return m_ydir; }

	int num_rays() const { return m_num_rays; }

private:
	t_vector m_origin;
	t_vector* m_dirs;

	float m_xdir;
	float m_ydir;

	int m_num_rays;
};

