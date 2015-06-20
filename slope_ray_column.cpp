#include "slope_ray_column.hpp"

t_ray t_slope_ray_column::get_ray(int index) const {
	const t_vector dir = t_vector(m_xdir, m_ydir, m_zdirs[index]).normalize_xyz();
	const t_ray ray = t_ray(m_pos, dir);
	return ray;
}


#if 0
bool t_slope_ray_column::rect_indices(t_const_vec min, t_const_vec max, int& start, int& end) const {
	float tmin = 0.0f;
	float tmax = 0.0f;

	if (!m_pos.time_in_rect(tmin, tmax, m_xdir, m_ydir, min.x(), max.x(), min.y(), max.y()))
		return false;

	const float zdif_max = max.z() - m_pos.z();
	const float zdif_min = min.z() - m_pos.z();

	float zdir_max = 0.0f;
	float zdir_min = m_zdirs[start];

	if (zdif_max > 0.0f) {
		zdir_max = zdif_max / tmin;
	} else {
		zdir_max = zdif_max / tmax;
	}

	if (zdif_min > 0.0f) {
		zdir_max = zdif_min / tmin;
	} else {
		zdir_max = zdif_min / tmax;
	}

	// TODO: optimize?
	for (++start; start < end; start++) {
		if (m_zdirs[start] > zdir_min)
			break;
	}

	for (int i = start; i < end; i++) {
		if (m_zdirs[i] > zdir_max)
			end = i;
	}

	return true;
}
#endif

