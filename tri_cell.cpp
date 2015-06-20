#include "tri_cell.hpp"

t_vector t_tri_cell::shading_normal(float relx, float rely) const {
	const t_vector x_s0 = (1.0f - rely) * m_sn00 + rely * m_sn01;
	const t_vector x_s1 = (1.0f - rely) * m_sn10 + rely * m_sn11;

	t_vector result = (1.0f - relx) * x_s0 + relx * x_s1;
	result.normalize_xyz();
	return result;
}



t_ray_intersection t_tri_cell::trace_negative(t_const_ray ray) const {
	const t_vector diff = ray.pos() - t_vector(m_x, m_y, m_z00);

	const float dist = diff * m_gn0;
	const float d = (m_gn0 * ray.dir());

	if (d < 0.0f) {
		const float t = -dist / d;

		if (ray.is_in_range(t)) {
			const t_vector hit = ray.point(t);

			const float relx = hit.x() - m_x;
			const float rely = hit.y() - m_y;

			if (relx >= 0.0f && rely >= 0.0f && relx + rely <= 1.0f) {
				return (t_ray_intersection(hit, m_gn0, shading_normal(relx, rely), t));
			}
		}
	}

	return (t_ray_intersection());
}

t_ray_intersection t_tri_cell::trace_positive(t_const_ray ray) const {
	const t_vector diff = ray.pos() - t_vector(m_x + 1.0f, m_y + 1.0f, m_z11);

	const float dist = diff * m_gn1;
	const float d = (m_gn1 * ray.dir());

	if (d < 0.0f) {
		const float t = -dist / d;

		if (ray.is_in_range(t)) {
			const t_vector hit = ray.point(t);

			const float relx = hit.x() - m_x;
			const float rely = hit.y() - m_y;

			if (relx <= 1.0f && rely <= 1.0f && relx + rely >= 1.0f) {
				return (t_ray_intersection(hit, m_gn1, shading_normal(relx, rely), t));
			}
		}
	}

	return (t_ray_intersection());
}



t_ray_intersection t_tri_cell::trace_negative_slope(t_const_ray ray) const {
	const float dz = ray.dir().z() - ray.dir().x() * m_dx0 - ray.dir().y() * m_dy0;

	if (dz > 0.0f)
		return (t_ray_intersection());

	const float diffz = m_z00 - ray.pos().z();
	const float t = diffz / dz;

	if (t > 0.0f) {
		const t_vector hit = ray.point(t);

		const float relx = hit.x() - m_x;
		const float rely = hit.y() - m_y;

		if (relx >= 0.0f && rely >= 0.0f && (relx + rely) <= 1.0f) {
			return (t_ray_intersection(hit, m_gn0, shading_normal(relx, rely), t));
		}
	}

	return (t_ray_intersection());
}

t_ray_intersection t_tri_cell::trace_positive_slope(t_const_ray ray) const {
	const float dz = ray.dir().z() - ray.dir().x() * m_dx1 - ray.dir().y() * m_dy1;

	if (dz > 0.0f)
		return (t_ray_intersection());

	const float diffz = m_z11 - ray.pos().z();
	const float t = diffz / dz;

	if (t > 0.0f) {
		const t_vector hit = ray.point(t);
		const float relx = hit.x() - m_x;
		const float rely = hit.y() - m_y;

		if (relx <= 1.0f && rely <= 1.0f && (relx + rely) >= 1.0f) {
			return (t_ray_intersection(hit, m_gn1, shading_normal(relx, rely), t));
		}
	}

	return (t_ray_intersection());
}



t_ray_intersection t_tri_cell::trace_ray(t_const_ray ray) const {
	if ((ray.dir().x() + ray.dir().y()) > 0.0f) {
		const t_ray_intersection result = trace_negative(ray);

		if (result.valid())
			return result;

		return (trace_positive(ray));
	} else {
		const t_ray_intersection result = trace_positive(ray);

		if (result.valid())
			return result;

		return (trace_negative(ray));
	}
}

t_ray_intersection t_tri_cell::trace_slope_ray(t_const_ray slope_ray) const {
	if ((slope_ray.dir().x() + slope_ray.dir().y()) > 0.0f) {
		const t_ray_intersection result = trace_negative_slope(slope_ray);

		if (result.valid())
			return result;

		return (trace_positive_slope(slope_ray));
	} else {
		const t_ray_intersection result = trace_positive_slope(slope_ray);

		if (result.valid())
			return result;

		return (trace_negative_slope(slope_ray));
	}
}

bool t_tri_cell::trace_shadow_ray(t_const_ray ray) const {
	// TODO: optimize?
	return ((trace_ray(ray)).valid());
}

void t_tri_cell::set_from_heightmap(const t_heightmap& heightmap, size_t x, size_t y) {
	m_x = x;
	m_y = y;

	// corner heights
	m_z00 = heightmap.at(x,     y    );
	m_z10 = heightmap.at(x + 1, y    );
	m_z01 = heightmap.at(x,     y + 1);
	m_z11 = heightmap.at(x + 1, y + 1);

	m_max_height = std::max(std::max(m_z00, m_z10), std::max(m_z01, m_z11));
	m_min_height = std::min(std::min(m_z00, m_z10), std::min(m_z01, m_z11));

	// delta heights
	m_dx0 = m_z10 - m_z00;
	m_dx1 = m_z11 - m_z01;
	m_dy0 = m_z01 - m_z00;
	m_dy1 = m_z11 - m_z10;

	const t_vector x0 = t_vector(1.0f, 0.0f, m_dx0);
	const t_vector y0 = t_vector(0.0f, 1.0f, m_dy0);
	const t_vector x1 = t_vector(1.0f, 0.0f, m_dx1);
	const t_vector y1 = t_vector(0.0f, 1.0f, m_dy1);

	m_gn0 = x0 ^ y0;
	m_gn1 = x1 ^ y1;
	m_gn0.normalize_xyz();
	m_gn1.normalize_xyz();

	float dx = m_dx0;
	float dy = m_dy0;

	t_vector vx;
	t_vector vy;

	// n_s00
	if (x > 0) {
		dx = (heightmap.at(x + 1, y) - heightmap.at(x - 1, y)) * 0.5f;
	} else {
		dx = m_dx0;
	}

	if (y > 0) {
		dy = (heightmap.at(x, y + 1) - heightmap.at(x, y - 1)) * 0.5f;
	} else {
		dy = m_dy0;
	}

	vx = t_vector(1.0f, 0.0f, dx);
	vy = t_vector(0.0f, 1.0f, dy);
	m_sn00 = vx ^ vy;
	m_sn00.normalize_xyz();


	// n_s10
	if (m_x < heightmap.width() - 2) {
		dx = (heightmap.at(x + 2, y) - heightmap.at(x, y)) * 0.5f;
	} else {
		dx = m_dx0;
	}

	if (y > 0) {
		dy = (heightmap.at(x + 1, y + 1) - heightmap.at(x + 1, y - 1)) * 0.5f;
	} else {
		dy = m_dy1;
	}

	vx = t_vector(1.0f, 0.0f, dx);
	vy = t_vector(0.0f, 1.0f, dy);
	m_sn10 = vx ^ vy;
	m_sn10.normalize_xyz();


	// n _s01
	if (x > 0) {
		dx = (heightmap.at(x + 1, y + 1) - heightmap.at(x - 1, y + 1)) * 0.5f;
	} else {
		dx = m_dx1;
	}

	if (y < heightmap.height() - 2) {
		dy = (heightmap.at(x, y + 2) - heightmap.at(x, y)) * 0.5f;
	} else {
		dy = m_dy0;
	}

	vx = t_vector(1.0f, 0.0f, dx);
	vy = t_vector(0.0f, 1.0f, dy);
	m_sn01 = vx ^ vy;
	m_sn01.normalize_xyz();


	// n_s11
	if (x < heightmap.width() - 2) {
		dx = (heightmap.at(x + 2, y + 1) - heightmap.at(x, y + 1)) * 0.5f;
	} else {
		dx = m_dx1;
	}

	if (y < heightmap.height() - 2) {
		dy = (heightmap.at(x + 1, y + 2) - heightmap.at(x + 1, y)) * 0.5f;
	} else {
		dy = m_dy1;
	}

	vx = t_vector(1.0f, 0.0f, dx);
	vy = t_vector(0.0f, 1.0f, dy);
	m_sn11 = vx ^ vy;
	m_sn11.normalize_xyz();
}

