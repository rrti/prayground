#pragma once

#include <cfloat>
#include <vector>

#include "ray.hpp"
#include "ray_column.hpp"
#include "ray_intersection.hpp"
#include "heightmap.hpp"
#include "scene.hpp"

template <class t_cell_type>
class t_kdtree_cell_scene;

template <class t_cell_type>
class t_kdtree_cell_scene_node {
public:
	// traces a ray into the scene; returns the intersection
	t_ray_intersection trace_ray(t_const_ray ray, float tmin, float tmax, float zmin) const {
		t_ray_intersection result;

		if (zmin > m_max_height)
			return result;

		if (m_leaf != 0)
			return (m_leaf->trace_ray(ray));

		t_kdtree_cell_scene_node<t_cell_type>* min_child = 0;
		t_kdtree_cell_scene_node<t_cell_type>* max_child = 0;
		float t_split = 0.0f;

		find_split(ray, min_child, max_child, t_split);

		if (tmin <= t_split) {
			float tmax_neg = tmax;
			float zmin_neg = zmin;

			if (t_split < tmax) {
				tmax_neg = t_split;

				if (ray.dir().z() < 0.0f) {
					zmin_neg = ray.pos().z() + ray.dir().z() * tmax_neg;
				}
			}

			result = min_child->trace_ray(ray, tmin, tmax_neg, zmin_neg);
		}

		if (result.valid())
			return result;

		if (t_split <= tmax) {
			float tmin_pos = tmin;
			float zmin_pos = zmin;

			if (t_split > tmin) {
				tmin_pos = t_split;

				if (ray.dir().z() > 0.0f) {
					zmin_pos = ray.pos().z() + ray.dir().z() * tmin_pos;
				}
			}

			result = max_child->trace_ray(ray, tmin_pos, tmax, zmin_pos);
		}

		return result;
	}

	// traces a shadow ray; returns true iff there is a collision
	bool trace_shadow_ray(t_const_ray ray, float tmin, float tmax, float zmin, float zmax) const {
		if (zmin > (m_max_height - RAY_TEST_EPSILON))	
			return false;
		if (zmax < (m_min_height + RAY_TEST_EPSILON))
			return true;

		if (m_leaf != 0)
			return (m_leaf->trace_shadow_ray(ray));

		t_kdtree_cell_scene_node<t_cell_type>* min_child = 0;
		t_kdtree_cell_scene_node<t_cell_type>* max_child = 0;
		float t_split = 0.0f;

		find_split(ray, min_child, max_child, t_split);

		if (tmin <= t_split) {
			float tmax_neg = tmax;
			float zmin_neg = zmin;
			float zmax_neg = zmax;

			if (t_split < tmax) {
				tmax_neg = t_split;

				if (ray.dir().z() < 0.0f) {
					zmin_neg = ray.pos().z() + ray.dir().z() * tmax_neg;
				} else {
					zmax_neg = ray.pos().z() + ray.dir().z() * tmax_neg;
				}
			}

			if (min_child->trace_shadow_ray(ray, tmin, tmax_neg, zmin_neg, zmax_neg)) {
				return true;
			}
		}

		if (t_split <= tmax) {
			float tmin_pos = tmin;
			float zmin_pos = zmin;
			float zmax_pos = zmax;

			if (t_split > tmin) {
				tmin_pos = t_split;

				if (ray.dir().z() > 0.0f) {
					zmin_pos = ray.pos().z() + ray.dir().z() * tmin_pos;
				} else {
					zmax_pos = ray.pos().z() + ray.dir().z() * tmin_pos;
				}
			}

			if (max_child->trace_shadow_ray(ray, tmin_pos, tmax, zmin_pos, zmax_pos)) {
				return true;
			}
		}

		return false;
	}

	// traces a slope-column beginning at the <start>-th ray
	int trace_slope_ray_column(t_ray_intersection* results, const t_slope_ray_column& slope_ray_column, float tmin, float tmax, int start) const {
		float zmin = slope_ray_column.pos().z() + tmin * slope_ray_column.zdirs()[start];
		float zmax = slope_ray_column.pos().z() + tmax * slope_ray_column.zdirs()[start];

		zmin = std::min(zmin, zmax);

		if (zmin > m_max_height)
			return start;

		if (m_leaf != 0) {
			for (; start < slope_ray_column.num_rays(); start++) {
				const t_ray_intersection result = m_leaf->trace_ray(slope_ray_column.get_ray(start));

				if (result.valid()) {
					results[start] = result;
				} else {
					break;
				}
			}

			return start;
		}

		t_kdtree_cell_scene_node<t_cell_type>* min_child;
		t_kdtree_cell_scene_node<t_cell_type>* max_child;
		float t_split;

		find_split(slope_ray_column.ray(), min_child, max_child, t_split);

		if (tmin <= t_split) {
			start = min_child->trace_slope_ray_column(results, slope_ray_column, tmin, ((t_split < tmax)? t_split: tmax), start);
		}

		if (t_split <= tmax) {
			start = max_child->trace_slope_ray_column(results, slope_ray_column, ((t_split > tmin)? t_split: tmin), tmax, start);
		}

		return start;
	}

    
	static t_kdtree_cell_scene_node<t_cell_type>* create_from_heightmap(const t_heightmap& heightmap,  size_t xmin, size_t xmax, size_t ymin, size_t ymax) {
		t_kdtree_cell_scene_node<t_cell_type>* result = new t_kdtree_cell_scene_node<t_cell_type>();

		if (xmax == xmin + 1 && ymax == ymin + 1) {
			// leaf node
			result->m_lft_child = 0;
			result->m_rgt_child = 0;
			result->m_leaf = new t_cell_type();
			result->m_leaf->set_from_heightmap(heightmap, xmin, ymin);
			result->m_min_height = result->m_leaf->get_min_height();
			result->m_max_height = result->m_leaf->get_max_height();
			return result;
		}

		// non-leaf nodes
		result->m_leaf = 0;

		// choose an axis and split-coordinates such that surface area is minimized
		// top surface area is constant with split choice, only worry about sides
		float score_x = FLT_MAX;
		float score_y = FLT_MAX;

		size_t split_x = 0;
		size_t split_y = 0;

		if (xmax > (xmin + 1)) {
			// placeholder: even split
			split_x = (xmax + xmin) / 2;
			score_x = ymax - ymin;

			// heightmap.get_opt_split_x(score_x, split_x, xmin, xmax + 1, ymin, ymax + 1);
		}

		if (ymax > (ymin + 1)) {
			// placeholder: even split
			split_y = (ymin + ymax) / 2;
			score_y = xmax - xmin;

			// heightmap.get_opt_split_y(score_y, split_y, xmin, xmax + 1, ymin, ymax + 1);
		}

		if (score_x < score_y) {
			result->m_lft_child = create_from_heightmap(heightmap, xmin, split_x, ymin, ymax);
			result->m_rgt_child = create_from_heightmap(heightmap, split_x, xmax, ymin, ymax);
			result->m_split_coor = split_x;
			result->m_split_axis = false;
		} else {
			result->m_lft_child = create_from_heightmap(heightmap, xmin, xmax, ymin, split_y);
			result->m_rgt_child = create_from_heightmap(heightmap, xmin, xmax, split_y, ymax);
			result->m_split_coor = split_y;
			result->m_split_axis = true;
		}

		result->m_max_height = std::max(result->m_lft_child->m_max_height, result->m_rgt_child->m_max_height);
		result->m_min_height = std::min(result->m_lft_child->m_min_height, result->m_rgt_child->m_min_height);

		return result;
	}

private:
	void find_split(
		t_const_ray ray,
		t_kdtree_cell_scene_node<t_cell_type>*& min_child, // near
		t_kdtree_cell_scene_node<t_cell_type>*& max_child, // far
		float& t_split
	) const {
		if (m_split_axis) {
			t_split = ray.time_to_y(m_split_coor);

			if (ray.dir().y() > 0.0f) {
				min_child = m_lft_child;
				max_child = m_rgt_child;
			} else {
				min_child = m_rgt_child;
				max_child = m_lft_child;
			}
		} else {
			t_split = ray.time_to_x(m_split_coor);

			if (ray.dir().x() > 0.0f) {
				min_child = m_lft_child;
				max_child = m_rgt_child;
			} else {
				min_child = m_rgt_child;
				max_child = m_lft_child;
			}
		}
	}

private:
	friend t_kdtree_cell_scene<t_cell_type>;

	t_kdtree_cell_scene_node<t_cell_type>* m_lft_child; // "left" subtree (< split)
	t_kdtree_cell_scene_node<t_cell_type>* m_rgt_child; // "right" subtree (> split)
	t_cell_type* m_leaf; // null if not leaf node

	size_t m_split_coor;

	float m_min_height;
	float m_max_height;

	bool m_split_axis; // true = y axis
};



template <class t_cell_type>
class t_kdtree_cell_scene: public t_scene {
public:
	~t_kdtree_cell_scene() {}

	void assign_heightmap(const t_heightmap& heightmap) {
		m_xmax = heightmap.width() - 1;
		m_ymax = heightmap.height() - 1;
		m_root = t_kdtree_cell_scene_node<t_cell_type>::create_from_heightmap(heightmap, 0, m_xmax, 0, m_ymax);
	}


	void assign_light_source(t_light* light) {
		m_light_sources.push_back(light);
	}

	void modify_light_source(size_t idx, float yaw, float pitch) {
		m_light_sources[idx]->rotate(yaw, pitch);
	}


	t_color trace_ray(t_const_ray ray) const {
		float tmin = 0.0f;
		float tmax = 0.0f;

		if (!ray.time_in_rect(tmin, tmax, 0, m_xmax, 0, m_ymax))
			return (t_color(0.0f, 0.0f, 0.0f));

		float zmin = ray.pos().z() + tmin * ray.dir().z();
		float zmax = ray.pos().z() + tmax * ray.dir().z();

		zmin = std::min(zmin, zmax);

		return (shade_hit(m_root->trace_ray(ray, tmin, tmax, zmin)));
	}

	bool trace_shadow_ray(t_const_ray ray) const {
		float tmin = 0.0f;
		float tmax = 0.0f;

		if (!ray.time_in_rect(tmin, tmax,  0, m_xmax, 0, m_ymax))
			return false;

		float zmin = ray.pos().z() + tmin * ray.dir().z();
		float zmax = ray.pos().z() + tmax * ray.dir().z();

		if (zmin > zmax)
			std::swap(zmin, zmax);

		return (m_root->trace_shadow_ray(ray, tmin, tmax, zmin, zmax));
	}

	void trace_slope_ray_column(const t_slope_ray_column& slope_ray_column, t_color* results) const {
		std::vector<t_ray_intersection> hits(slope_ray_column.num_rays());

		float tmin = 0.0f;
		float tmax = 0.0f;

		if (!(slope_ray_column.ray()).time_in_rect(tmin, tmax,  0, m_xmax, 0, m_ymax)) {
			for (int i = 0; i < slope_ray_column.num_rays(); i++) {
				results[i] *= 0.0f;
			}

			return;
		}

		m_root->trace_slope_ray_column(&hits[0], slope_ray_column, tmin, tmax, 0);

		for (int i = 0; i < slope_ray_column.num_rays(); i++) {
			results[i] = shade_hit(hits[i]);
		}
	}

private:
	t_color shade_hit(const t_ray_intersection& hit) const {
		t_color result;

		if (hit.valid()) {
			// convert the normal to a diffuse RGB color (TODO: read in a diffuse albedo texture)
			const t_color albedo = t_color((0.5f * hit.sn().x() + 0.5f), (0.5f * hit.sn().y() + 0.5f), (0.5f * hit.sn().z() + 0.5f));

			// we only trace shadow secondary rays, so fake global illumination
			result += (albedo * 0.25f);

			for (size_t n = 0; n < m_light_sources.size(); n++) {
				const t_vector light_dir = m_light_sources[n]->get_direction(hit.pos());

				// calculate the strength of diffuse local illumination via dot(N, L)
				const float obliquity_g = hit.gn() * light_dir;
				const float obliquity_s = hit.sn() * light_dir;

				if (obliquity_s > 0.0f) {
					if (obliquity_g > 0.0f) {
						if (!trace_shadow_ray(t_ray(hit.pos(), light_dir))) {
							result += (m_light_sources[n]->get_color() * albedo * obliquity_s);
						}
					} else {
						const t_ray shadow_ray = t_ray(hit.pos() + light_dir, light_dir);

						float tmin = 0.0f;
						float tmax = 0.0f;

						if (shadow_ray.time_in_rect(tmin, tmax,  0, m_xmax, 0, m_ymax)) {
							const float zmin = shadow_ray.pos().z() + tmin * shadow_ray.dir().z();
							const float zmax = shadow_ray.pos().z() + tmax * shadow_ray.dir().z();

							const t_ray_intersection shadow_int = m_root->trace_ray(shadow_ray, tmin, tmax, std::min(zmin, zmax));

							if (!shadow_int.valid()) {
								result += (m_light_sources[n]->get_color() * albedo * obliquity_s);
							}
						} else {
							result += (m_light_sources[n]->get_color() * albedo * obliquity_s);
						}
					}
				}
			}
		}

		return result;
	}

private:
	// bounds of the tree-root
	size_t m_xmax;
	size_t m_ymax;

	t_kdtree_cell_scene_node<t_cell_type>* m_root;

	std::vector<t_light*> m_light_sources;
};

