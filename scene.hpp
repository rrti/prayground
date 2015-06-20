#pragma once

#include "common.hpp"
#include "color.hpp"
#include "heightmap.hpp"
#include "light.hpp"
#include "ray.hpp"
#include "ray_column.hpp"
#include "ray_intersection.hpp"
#include "slope_ray_column.hpp"

// represents a heightmap with imposed subdivision structure
// (e.g. a linear grid or a kd-tree) to accelerate raytracing
//
// NOTE: z-coordinate is "up"
//
class t_scene {
public:
	virtual ~t_scene() {}

	// assign a heightmap to this scene
	virtual void assign_heightmap(const t_heightmap&) = 0;


	// assign a user-controlled light
	virtual void assign_light_source(t_light*) {}

	// modify the user light
	virtual void modify_light_source(size_t, float, float) {}


	// traces a ray into the scene, returns the color
	virtual t_color trace_ray(t_const_ray) const = 0;

	// traces a ray-column into the scene
	virtual void trace_ray_column(const t_ray_column& ray_column, t_color* results) const {
		for (int i = 0; i < ray_column.num_rays(); i++) {
			results[i] = trace_ray(t_ray(ray_column.pos(), ray_column.dirs()[i]));
		}
	}

	// traces a slope-column into the scene
	virtual void trace_slope_ray_column(const t_slope_ray_column& slope_ray_column, t_color* results) const {
		for (int i = 0; i < slope_ray_column.num_rays(); i++) {
			results[i] = trace_ray(slope_ray_column.get_ray(i));
		}
	}

	// traces a shadow ray; returns true iff there is a collision
	virtual bool trace_shadow_ray(t_const_ray) const = 0;
};

