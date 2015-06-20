#pragma once

#include <iostream>

class t_color {
public:
	t_color() { r() = 0.0f; g() = 0.0f; b() = 0.0f; }
	t_color(float _r, float _g, float _b) {
		r() = _r;
		g() = _g;
		b() = _b;
	}

	// t_color operator + (const t_color& v) const;
	// t_color operator - (const t_color& v) const;
	t_color operator * (const t_color& v) const { return (t_color(r() * v.r(), g() * v.g(), b() * v.b())); }

	t_color& operator += (const t_color& v) { r() += v.r(); g() += v.g(); b() += v.b(); return *this; }
	t_color& operator -= (const t_color& v) { r() -= v.r(); g() -= v.g(); b() -= v.b(); return *this; }

	friend t_color operator * (const t_color& v, float f) { return (t_color(v.r() * f, v.g() * f, v.b() * f)); }
	friend t_color operator * (float f, const t_color& v) { return (t_color(v.r() * f, v.g() * f, v.b() * f)); }

	t_color& operator *= (float f) { r() *= f; g() *= f; b() *= f; return *this; }
	t_color& operator /= (float f) { r() /= f; g() /= f; b() /= f; return *this; }

	friend std::ostream& operator << (std::ostream& os, t_color& c) { os << c.r() << " " << c.g() << " " << c.b(); return os; }
	friend std::istream& operator >> (std::istream& is, t_color& c) { is >> c.r()        >> c.g()        >> c.b(); return is; }

	float r() const { return m_r; }
	float g() const { return m_g; }
	float b() const { return m_b; }

	float& r() { return m_r; }
	float& g() { return m_g; }
	float& b() { return m_b; }

private:
	float m_r, m_g, m_b;
};

