#include "ray.hpp"
#include "vector.hpp"

float t_vector::magnitude_xyz() const { return (std::sqrt(x() * x() + y() * y() + z() * z())); }
float t_vector::magnitude_xy() const { return (std::sqrt(x() *x() + y() * y())); }


t_vector& t_vector::normalize_xyz() {
	const float s = magnitude_xyz();

	if (s > 0.0f) {
		x() /= s;
		y() /= s;
		z() /= s;
	}

	return *this;
}

t_vector& t_vector::normalize_xy() {
	const float s = magnitude_xy();

	if (s > 0.0f) {
		x() /= s;
		y() /= s;
	}

	return *this;
}


t_vector& t_vector::rotate_z(float angle) {
	const float cur_x = x();
	const float cur_y = y();

	const float cosAngle = std::cos(angle);
	const float sinAngle = std::sin(angle);

	// rotate CCW in the xy-plane around the world z-axis (z=up)
	x() = cur_x * cosAngle - cur_y * sinAngle;
	y() = cur_y * cosAngle + cur_x * sinAngle;

	// normalize_xy();
	return *this;
}

t_vector& t_vector::rotate_xy(float angle) {
	const float cosAngle = std::cos(angle);
	const float sinAngle = std::sin(angle);

	// calculate our length in the xy-plane
	const float cur_len_xy = magnitude_xy();
	const float new_len_xy = cur_len_xy * cosAngle - z() * sinAngle;

	// rotate around the world-axis formed by cross(this, world-z)
	x() = new_len_xy * x() / cur_len_xy;
	y() = new_len_xy * y() / cur_len_xy;
	z() = z() * cosAngle + cur_len_xy * sinAngle;

	// normalize_xyz();
	return *this;
}



bool t_vector::time_in_rect(float& tmin, float& tmax,  float xdir, float ydir,  float xmin, float xmax, float ymin, float ymax) const {
	const t_vector d = t_vector(xdir, ydir, 0.0f);
	const t_ray r = t_ray(*this, d);

	return (r.time_in_rect(tmin, tmax,  xmin, xmax, ymin, ymax));
}

