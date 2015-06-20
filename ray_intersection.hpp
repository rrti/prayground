#pragma once

#include "color.hpp"
#include "vector.hpp"

class t_ray_intersection {
public:
	t_ray_intersection(float t = -1.0f) {
		m_time = t;
	}

	t_ray_intersection(t_const_vec pos, t_const_vec n, float t) {
		m_pos = pos;
		m_gn = n;
		m_sn = n;
		m_time = t;
	}

	t_ray_intersection(t_const_vec pos, t_const_vec gn, t_const_vec sn, float t) {
		m_pos = pos;
		m_gn = gn;
		m_sn = sn;
		m_time = t;
	}

	const t_vector& pos() const { return m_pos; }
	const t_vector& gn() const { return m_gn; }
	const t_vector& sn() const { return m_sn; }

	float time() const { return m_time; }

	bool valid() const { return (time() >= 0.0f); }

	bool operator < (const t_ray_intersection& intersection) const {
		return (valid() && (!intersection.valid() || m_time < intersection.time()));
	}
	bool operator <= (const t_ray_intersection& intersection) const {
		if (valid())
			return (!intersection.valid() || m_time <= intersection.time());

		return (!intersection.valid());
	}

private:
	t_vector m_pos;
	// geometric normal
	t_vector m_gn;
	// shading normal
	t_vector m_sn;

	// parametric time of intersection
	float m_time;
};

