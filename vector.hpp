#pragma once

#include <cmath>
#include <iostream>
#include "common.hpp"

class t_vector {
public:
	t_vector(float _x = 0.0f, float _y = 0.0f, float _z = 0.0f) {
		x() = _x;
		y() = _y;
		z() = _z;
	}

	float magnitude_xyz() const;
	float magnitude_xy() const;
	float get_slope() const { return (z() / magnitude_xy()); }

	t_vector& normalize_xyz();
	t_vector& normalize_xy();

	t_vector& rotate_z(float angle);
	t_vector& rotate_xy(float angle);

	t_vector operator + (t_const_vec& v) const { return (t_vector(x() + v.x(), y() + v.y(), z() + v.z())); }
	t_vector operator - (t_const_vec& v) const { return (t_vector(x() - v.x(), y() - v.y(), z() - v.z())); }
	t_vector& operator += (t_const_vec& v) { x() += v.x(); y() += v.y(); z() += v.z(); return *this; }
	t_vector& operator -= (t_const_vec& v) { x() -= v.x(); y() -= v.y(); z() -= v.z(); return *this; }

	// inner product
	float operator * (t_const_vec& v) const { return ((x() * v.x()) + (y() * v.y()) + (z() * v.z())); }

	// outer product
	t_vector operator ^ (t_const_vec& v) const { return (t_vector((y() * v.z() - z() * v.y()), (z() * v.x() - x() * v.z()), (x() * v.y() - y() * v.x()))); }

	friend t_vector operator * (t_const_vec& v, float f) { return t_vector(v.x() * f, v.y() * f, v.z() * f); }
	friend t_vector operator / (t_const_vec& v, float f) { return t_vector(v.x() / f, v.y() / f, v.z() / f); }
	friend t_vector operator * (float f, t_const_vec& v) { return t_vector(v.x() * f, v.y() * f, v.z() * f); }
	friend t_vector operator / (float f, t_const_vec& v) { return t_vector(v.x() / f, v.y() / f, v.z() / f); }

	friend std::ostream& operator << (std::ostream& os, t_vector& v) { os << v.x() << " " << v.y() << " " << v.z(); return os; }
	friend std::istream& operator >> (std::istream& is, t_vector& v) { is >> v.x()        >> v.y()        >> v.z(); return is; }

	bool time_in_rect(float& tmin, float& tmax,  float xdir, float ydir,  float xmin, float xmax, float ymin, float ymax) const;

	float x() const { return m_x; }
	float y() const { return m_y; }
	float z() const { return m_z; }

	float& x() { return m_x; }
	float& y() { return m_y; }
	float& z() { return m_z; }

private:
	float m_x, m_y, m_z;
};

