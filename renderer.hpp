#pragma once

#include <vector>
#include <boost/thread.hpp>

#include "scene.hpp"
#include "tri_cell.hpp"
#include "vector.hpp"
#include "camera.hpp"

class t_renderer {
public:
	t_renderer();
	~t_renderer();

	const t_camera* get_camera() const { return m_camera; }

	void spawn_threads();
	void read_config(const char* filename);

	void set_viewport(size_t x, size_t y);
	void display();
	void idle();

	void keyboard_down(unsigned char key, int x, int y);
	void keyboard_up(unsigned char key, int x, int y);

	void mouse_state(int button, int state, int x, int y);
	void mouse_motion(int x, int y);

	#if (USE_STANDARD_GLUT != 1)
	bool want_quit() const { return m_quit_tracing; }
	#endif

private:
	void trace_rays(size_t xmin, size_t xmax, size_t ymin, size_t ymax);
	void trace_ray_columns(size_t xmin, size_t xmax, size_t ymin, size_t ymax);
	void trace_ray_slope_columns(size_t xmin, size_t xmax, size_t ymin, size_t ymax);

	int64_t m_epoch_tick;
	int64_t m_frame_tick;

	size_t m_thread_count;
	size_t m_frame_count;
	size_t m_scene_type;

	float camera_azim_angle;
	float camera_elev_angle;
	float camera_move_dist;
	float light_azim_angle;
	float light_elev_angle;

	float camera_transl_speed;
	float camera_rotate_speed;
	float light_rotate_speed;

	int m_last_mouse_x, m_last_mouse_y;
	int m_diff_mouse_x, m_diff_mouse_y;

	float m_mouse_scale;
	int m_mouse_button;

	volatile bool m_quit_tracing;
	bool m_trace_columns;

	// worker threads
	std::vector<boost::thread*> m_threads;
	boost::barrier* m_barrier;

	t_camera* m_camera;
	t_scene* m_scene;
};

