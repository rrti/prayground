#pragma once

#include "light.hpp"

class t_directional_light: public t_light {
public:
	t_directional_light(const t_vector& dir, const t_color& clr) {
		m_dir = dir;
		m_clr = clr;
	}

	t_vector get_direction(const t_vector& /*pos*/) const { return m_dir; }
	t_color get_color() const { return m_clr; }

	void rotate(float yaw, float pitch) {
		m_dir.rotate_z(yaw);
		m_dir.rotate_xy(pitch);
	}

private:
	t_vector m_dir;
	t_color m_clr;
};

