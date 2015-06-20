#pragma once

#include "heightmap.hpp"
#include "ray.hpp"
#include "scene.hpp"

template <class t_cell_type>
class t_quadtree_cell_scene_node {
public:
	t_ray_intersection trace_ray(t_const_ray ray) const {
		if (m_leaf != 0)
			return (m_leaf->trace_ray(ray));

		t_ray_intersection result;
		t_ray traced_ray = ray;

		float tmin_x = (m_xmin - ray.pos().x()) / ray.dir().x();
		float tmax_x = (m_xmax - ray.pos().x()) / ray.dir().x();
		float tmin_y = (m_ymin - ray.pos().y()) / ray.dir().y();
		float tmax_y = (m_ymax - ray.pos().y()) / ray.dir().y();

		if (tmin_x > tmax_x)
			std::swap(tmin_x, tmax_x);
		if (tmin_y > tmax_y)
			std::swap(tmin_y, tmax_y);

		if (tmin_x > tmax_y || tmin_y > tmax_x)
			return result;

		if (ray.dir().z() < 0.0f) {
			const float tmin_z = (m_max_height - ray.pos().z()) / ray.dir().z();

			if (tmin_z > tmax_x || tmin_z > tmax_y) {
				return result;
			}
		} else if (ray.dir().z() > 0.0f) {
			const float tmax_z = (m_max_height - ray.pos().z()) / ray.dir().z();

			if (tmin_x > tmax_z || tmin_y > tmax_z) {
				return result;
			}
		} else {
			if (ray.pos().z() > m_max_height) {
				return result;
			}
		}

		for (int i = 0; i < 4; i++) {
			if (m_children[i] != 0) {
				const t_ray_intersection hit = m_children[i]->trace_ray(traced_ray);

				if (hit.valid()) {
					result = hit;
					traced_ray.tmax() = hit.time();
				}
			}
		}

		return result;
	}

	bool trace_shadow_ray(t_const_ray ray) const {
		return ((trace_ray(ray)).valid());
	}


	static t_quadtree_cell_scene_node<t_cell_type>* create_from_heightmap(const t_heightmap& heightmap) {
		return (create_from_heightmap(heightmap, 0, heightmap.width() - 1, 0, heightmap.height() - 1));
	}

    static t_quadtree_cell_scene_node<t_cell_type>* create_from_heightmap(const t_heightmap& heightmap, size_t xmin, size_t xmax, size_t ymin, size_t ymax) {
		t_quadtree_cell_scene_node<t_cell_type>* result = new t_quadtree_cell_scene_node<t_cell_type>();
		result->m_xmin = xmin;
		result->m_xmax = xmax;
		result->m_ymin = ymin;
		result->m_ymax = ymax;
		result->m_children[0] = 0;
		result->m_children[1] = 0;
		result->m_children[2] = 0;
		result->m_children[3] = 0;
		result->m_leaf = 0;

		if (xmax > (xmin + 1)) {
			if (ymax > (ymin + 1)) {
				// 4-node
				const int xmid = (xmin + xmax) >> 1;
				const int ymid = (ymin + ymax) >> 1;

				result->m_children[0] = create_from_heightmap(heightmap, xmin, xmid, ymin, ymid);
				result->m_children[1] = create_from_heightmap(heightmap, xmid, xmax, ymin, ymid);
				result->m_children[2] = create_from_heightmap(heightmap, xmin, xmid, ymid, ymax);
				result->m_children[3] = create_from_heightmap(heightmap, xmid, xmax, ymid, ymax);
				result->m_min_height = std::min(result->m_children[0]->m_min_height, result->m_children[1]->m_min_height);
				result->m_min_height = std::min(result->m_min_height, result->m_children[2]->m_min_height);
				result->m_min_height = std::min(result->m_min_height, result->m_children[3]->m_min_height);
				result->m_max_height = std::max(result->m_children[0]->m_max_height, result->m_children[1]->m_max_height);
				result->m_max_height = std::max(result->m_max_height, result->m_children[2]->m_max_height);
				result->m_max_height = std::max(result->m_max_height, result->m_children[3]->m_max_height);
			} else {
				// 2-node, split on x
				const int xmid = (xmin + xmax) >> 1;

				result->m_children[0] = create_from_heightmap(heightmap, xmin, xmid, ymin, ymax);
				result->m_children[1] = create_from_heightmap(heightmap, xmid, xmax, ymin, ymax);
				result->m_min_height = std::min(result->m_children[0]->m_min_height, result->m_children[1]->m_min_height);
				result->m_max_height = std::max(result->m_children[0]->m_max_height, result->m_children[1]->m_max_height);
			}
		} else {
			if (ymax > (ymin + 1)) {
				// 2-node, split on y
				const int ymid = (ymin + ymax) >> 1;

				result->m_children[0] = create_from_heightmap(heightmap, xmin, xmax, ymin, ymid);
				result->m_children[3] = create_from_heightmap(heightmap, xmin, xmax, ymid, ymax);
				result->m_min_height = std::min(result->m_children[0]->m_min_height, result->m_children[3]->m_min_height);
				result->m_max_height = std::max(result->m_children[0]->m_max_height, result->m_children[3]->m_max_height);
			} else {
				// leaf node
				result->m_leaf = new t_cell_type();
				result->m_leaf->set_from_heightmap(heightmap, xmin, ymin);
				result->m_min_height = result->m_leaf->get_min_height();
				result->m_max_height = result->m_leaf->get_max_height();
			}
		}

		return result;
	}

private:
	t_quadtree_cell_scene_node<t_cell_type>* m_children[4];
	t_cell_type* m_leaf;

	float m_min_height;
	float m_max_height;

	size_t m_xmin, m_xmax;
	size_t m_ymin, m_ymax;
};



template <class t_cell_type>
class t_quadtree_cell_scene: public t_scene {
public:
	~t_quadtree_cell_scene() {}

	void assign_heightmap(const t_heightmap& heightmap) {
		// construct tree
		m_root = t_quadtree_cell_scene_node<t_cell_type>::create_from_heightmap(heightmap); 
	}

	t_color shade_hit(const t_ray_intersection& hit) const {
		return (t_color((0.5f * hit.sn().x() + 0.5f), (0.5f * hit.sn().y() + 0.5f), (0.5f * hit.sn().z() + 0.5f)));
	}

	t_color trace_ray(t_const_ray ray) const {
		return (shade_hit(m_root->trace_ray(ray)));
	}

	bool trace_shadow_ray(t_const_ray ray) const {
		return (m_root->trace_shadow_ray(ray));
	}

private:
	t_quadtree_cell_scene_node<t_cell_type>* m_root;
};

