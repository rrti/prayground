#pragma once

class FIBITMAP;
class t_bitmap {
public:
	struct t_rgb_color {
	public:
		t_rgb_color(uint8_t r = 0, uint8_t g = 0, uint8_t b = 0) {
			m_r = r;
			m_g = g;
			m_b = b;
		}

	private:
		uint8_t m_r;
		uint8_t m_g;
		uint8_t m_b;
	};


	t_bitmap() {
		m_xsize = 0;
		m_ysize = 0;
		m_data = 0;
	}

	t_bitmap(FIBITMAP* source) { set_data(source); }
	~t_bitmap() { delete_data(); }

	size_t width() const { return m_xsize; }
	size_t height() const { return m_ysize; }

	t_rgb_color& at(size_t x, size_t y) const { return m_data[y * m_xsize + x]; }

	void set_data(FIBITMAP* source);
	void delete_data() { delete[] m_data; m_data = 0; }

private:
	size_t m_xsize;
	size_t m_ysize;

	t_rgb_color* m_data;
};

