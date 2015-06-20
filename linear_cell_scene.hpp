#pragma once
#include "scene.hpp"

template <class t_cell_type>
class t_linear_cell_scene: public t_scene {
public:
	~t_linear_cell_scene() {}

	void assign_heightmap(const t_heightmap& heightmap) {
		m_ysize = heightmap.height() - 1;
		m_xsize = heightmap.width() - 1;

		m_cells = new t_cell_type[m_xsize * m_ysize];

		for (size_t y = 0; y < m_ysize; y++) {
			for (size_t x = 0; x < m_xsize; x++) {
				m_cells[y * m_xsize + x].set_from_heightmap(heightmap, x, y);
			}
		}
	}

	t_color shade_hit(const t_ray_intersection& hit) const {
		return (t_color((0.5f * hit.sn().x() + 0.5f), (0.5f * hit.sn().y() + 0.5f), (0.5f * hit.sn().z() + 0.5f)));
	}

	// search through the grid (note: not the best way)
	t_color trace_ray(t_const_ray ray) const {
		t_ray traced_ray = ray;
		t_ray_intersection traced_int;

		for (size_t y = 0; y < m_ysize; y++) {
			for (size_t x = 0; x < m_xsize; x++) {
				const t_ray_intersection hit = m_cells[y * m_xsize + x].trace_ray(traced_ray);

				if (hit.valid()) {
					traced_int = hit;
					traced_ray.tmax() = hit.time();
				}
			}
		}

		return (shade_hit(traced_int));
	}

	bool trace_shadow_ray(t_const_ray ray) const {
		bool hit = false;

		for (size_t y = 0; y < m_ysize; y++) {
			for (size_t x = 0; x < m_xsize; x++) {
				const t_ray_intersection ray_int = m_cells[y * m_xsize + x].trace_ray(ray);

				if ((hit = ray_int.valid())) {
					break;
				}
			}
		}

		return hit;
	}

private:
	size_t m_ysize;
	size_t m_xsize;

	t_cell_type* m_cells;
};

