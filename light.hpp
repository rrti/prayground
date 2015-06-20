#pragma once

#include "color.hpp"
#include "vector.hpp"

class t_light {
public:
	// get the direction and distance to this light from pos
	virtual t_vector get_direction(const t_vector& pos) const = 0;
	virtual t_color get_color() const = 0;

	// modify the direction of this light
	virtual void rotate(float yaw, float pitch) = 0;
};

