#include <GL/glut.h>
#include "camera.hpp"

void t_camera::draw_image_pixel(size_t x, size_t y, const t_color& c) {
	glColor3f(c.r(), c.g(), c.b());
	glVertex2f(x, y);
}

void t_camera::draw_image() {
	glBegin(GL_POINTS);

	for (size_t y = 0; y < m_view_size_y; y++) {
		for (size_t x = 0; x < m_view_size_x; x++) {
			draw_image_pixel(x, y, m_image[y * m_view_size_x + x]);
		}
	}

	glEnd();
}

