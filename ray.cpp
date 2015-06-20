#include "ray.hpp"

bool t_ray::time_in_rect(float& tmin, float& tmax,  float xmin, float xmax, float ymin, float ymax) const {
	float tmin_x = (xmin - m_pos.x()) / m_dir.x();
	float tmax_x = (xmax - m_pos.x()) / m_dir.x();

	float tmin_y = (ymin - m_pos.y()) / m_dir.y();
	float tmax_y = (ymax - m_pos.y()) / m_dir.y();

	if (tmin_x > tmax_x)
		std::swap(tmin_x, tmax_x);
	if (tmax_x < 0.0f)
		return false;

	if (tmin_y > tmax_y)
		std::swap(tmin_y, tmax_y);
	if (tmax_y < 0.0f)
		return false;

	tmin = std::max(tmin_x, tmin_y);
	tmax = std::min(tmax_x, tmax_y);

	if (tmin > tmax)
		return false;

	if (tmin < 0.0f)
		tmin = 0.0f;

    return true;
}

