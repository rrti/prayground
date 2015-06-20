#pragma once
#include "cell.hpp"

// represents a two-triangle quad with fixed tesellation direction
class t_tri_cell: public t_cell {
public:
	float get_max_height() const { return m_max_height; }
	float get_min_height() const { return m_min_height; }

	t_ray_intersection trace_ray(t_const_ray ray) const;
	t_ray_intersection trace_slope_ray(t_const_ray slope_ray) const;

	bool trace_shadow_ray(t_const_ray ray) const;

	void set_from_heightmap(const t_heightmap& heightmap, size_t x, size_t y);

private:
	t_vector shading_normal(float relx, float rely) const;

	t_ray_intersection trace_negative(t_const_ray ray) const;
	t_ray_intersection trace_positive(t_const_ray ray) const;

	t_ray_intersection trace_negative_slope(t_const_ray ray) const;
	t_ray_intersection trace_positive_slope(t_const_ray ray) const;

private:
	float m_min_height;
	float m_max_height;

	// geometric normals
	t_vector m_gn0;
	t_vector m_gn1;

	// shading normals
	t_vector m_sn00;
	t_vector m_sn10;
	t_vector m_sn01;
	t_vector m_sn11;

	// grid coordinates
	float m_x, m_y;
	// corner heights
	float m_z00, m_z10, m_z01, m_z11;
	// delta heights
	float m_dx0, m_dx1, m_dy0, m_dy1;
};

