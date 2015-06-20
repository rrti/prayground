#pragma once

#include "common.hpp"
#include "heightmap.hpp"
#include "ray.hpp"
#include "ray_intersection.hpp"

class t_cell {
public:
	virtual float get_max_height() const = 0;
	virtual float get_min_height() const = 0;

	// traces a ray into this cell; returns the intersection
	virtual t_ray_intersection trace_ray(t_const_ray ray) const = 0;
	virtual t_ray_intersection trace_slope_ray(t_const_ray slope_ray) const = 0;

	// traces a shadow ray into this cell; returns true iff there is a collision
	virtual bool trace_shadow_ray(t_const_ray ray) const = 0;

	virtual void set_from_heightmap(const t_heightmap& heightmap, size_t x, size_t y) = 0;
};

