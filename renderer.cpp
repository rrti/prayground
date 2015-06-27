#include <cassert>
#include <cmath>
#include <cstdio>
#include <ctime>
#include <fstream>
#include <sstream>

#include <boost/chrono.hpp>
#include <GL/glut.h>

#include "lib/FreeImage.h"

#include "common.hpp"
#include "renderer.hpp"
#include "directional_light.hpp"
#include "linear_cell_scene.hpp"
#include "quadtree_cell_scene.hpp"
#include "kdtree_cell_scene.hpp"

#if 0
struct t_thread_state {
private:
	boost::thread* m_thread;
	boost::mutex* m_mutex;

	// produced sub-frame image chunks
	std::list<t_bitmap> m_chunks;
};
#endif

enum {
	SCENETYPE_LINEAR   = 0,
	SCENETYPE_QUADTREE = 1,
	SCENETYPE_KDTREE   = 2,
};



static int64_t get_tick() {
	const boost::chrono::high_resolution_clock::time_point cur_time = boost::chrono::high_resolution_clock::now();
	const boost::chrono::nanoseconds run_time = boost::chrono::duration_cast<boost::chrono::nanoseconds>(cur_time.time_since_epoch());
	return (run_time.count());
}

static float deg2rad(float x) { return (x * (M_PI / 180.0f)); }
static float rnd_flt() { return (float(rand()) / RAND_MAX); }



t_renderer::t_renderer() {
	m_epoch_tick = get_tick();
	m_frame_tick = get_tick();

	m_thread_count = boost::thread::hardware_concurrency();
	m_frame_count = 0;
	m_scene_type = SCENETYPE_KDTREE;

	m_barrier = 0;
	m_camera = new t_camera();
	m_scene = 0;

	camera_azim_angle = 0.0f;
	camera_elev_angle = 0.0f; 
	camera_move_dist  = 0.0f;
	light_azim_angle  = 0.0f;
	light_elev_angle  = 0.0f;

	camera_transl_speed = 100.00f;
	camera_rotate_speed =   0.25f;
	light_rotate_speed  =   0.25f;

	m_mouse_scale = 0.005f;
	m_mouse_button = -1;

	m_quit_tracing = false;
	m_trace_columns = true;

	const time_t raw_time = time(0);
	const tm* loc_time = localtime(&raw_time);
	printf("[%s] %s", __FUNCTION__, asctime(loc_time));

	FreeImage_Initialise();
}

t_renderer::~t_renderer() {
	const int64_t render_time = get_tick() - m_epoch_tick; // ns
	const  double render_rate = m_frame_count / (render_time * 1e-9);

	// threads waiting at a barrier are joinable
	for (size_t n = 0; n < m_threads.size(); n++) {
		assert(m_threads[n]->joinable());
		m_threads[n]->join();
		delete m_threads[n];
	}

	FreeImage_DeInitialise();

	printf("[%s]\n", __FUNCTION__);
	printf("\tframe-count: %lu\n", m_frame_count);
	printf("\trender-time: %gsec\n", (render_time * 1e-9));
	printf("\trender-rate: %gfps\n", render_rate);

	delete m_barrier;
	delete m_camera;
	delete m_scene;
}


void t_renderer::spawn_threads() {
	if (m_thread_count <= 1) {
		// dummy for the ST case
		m_barrier = new boost::barrier(1);
		return;
	}

	assert(m_thread_count != 0);
	assert((m_thread_count % 2) == 0);

	m_barrier = new boost::barrier(m_thread_count + 1);
	m_threads.resize(m_thread_count, NULL);

	size_t num_threads_x = 0;
	size_t num_threads_y = 0;

	// spawn threads to perform the actual raytracing in lockstep
	// these will be cranked by display() calls in the main-thread
	switch (m_thread_count) {
		case  2: {
			num_threads_x = 1;
			num_threads_y = 2;
		} break;

		case  4: {
			num_threads_x = 2;
			num_threads_y = 2;
		} break;

		case  8: {
			num_threads_x = 2;
			num_threads_y = 4;
		} break;

		case 16: {
			num_threads_x = 4;
			num_threads_y = 4;
		} break;

		case 32: {
			num_threads_x = 4;
			num_threads_y = 8;
		} break;

		case 64: {
			num_threads_x = 8;
			num_threads_y = 8;
		} break;

		case 128: {
			num_threads_x = 16;
			num_threads_y =  8;
		} break;

		case 256: {
			num_threads_x = 16;
			num_threads_y = 16;
		} break;
	}

	for (size_t y = 0; y < num_threads_y; y++) {
		for (size_t x = 0; x < num_threads_x; x++) {
			const int xmin = (m_camera->get_view_size_x() / num_threads_x) * (x + 0), xmax = (m_camera->get_view_size_x() / num_threads_x) * (x + 1);
			const int ymin = (m_camera->get_view_size_y() / num_threads_y) * (y + 0), ymax = (m_camera->get_view_size_y() / num_threads_y) * (y + 1);

			if (m_trace_columns) {
				m_threads[y * num_threads_x + x] = new boost::thread(&t_renderer::trace_ray_columns, this,  xmin, xmax, ymin, ymax);
			} else {
				m_threads[y * num_threads_x + x] = new boost::thread(&t_renderer::trace_rays, this,  xmin, xmax, ymin, ymax);
			}
		}
	}
}


void t_renderer::set_viewport(size_t x, size_t y) {
	m_camera->set_image_size(x, y);

	glViewport(0, 0, x, y);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, x, 0, y);
}



void t_renderer::read_config(const char* filename) {
	std::ifstream is;
	std::stringstream ss;

	std::string line;
	std::string oper;

	is.open(filename);

	struct t_scene_data {
	public:
		std::vector<t_heightmap> m_images;
		std::vector<t_light*> m_lights;
	};

	t_scene_data scene_data;

	while ((std::getline(is, line)).good()) {
		assert(!line.empty());

		if (line[0] == '\n')
			continue;
		if (line[0] == '\r')
			continue;
		if (line[0] == '#')
			continue;

		ss << line;
		ss >> oper; // extract key

		if (oper ==   "num_threads") { ss >> m_thread_count; continue; }
		if (oper ==    "scene_type") { ss >> m_scene_type; continue; }
		if (oper == "trace_columns") { ss >> m_trace_columns; continue; }

		if (oper == "camera_pos") { ss >> m_camera->pos(); continue; }
		if (oper == "camera_dir") { ss >> m_camera->dir(); m_camera->dir().normalize_xyz(); continue; }
		if (oper == "camera_fov") { ss >> m_camera->fov(); m_camera->fov() = deg2rad(m_camera->fov()); continue; }

		if (oper == "camera_mov_speed") { ss >> camera_transl_speed; continue; }
		if (oper == "camera_rot_speed") { ss >> camera_rotate_speed; continue; }
		if (oper == "mouse_sensitivity") { ss >> m_mouse_scale; continue; }

		if (oper == "view_size") {
			size_t vsx; ss >> vsx;
			size_t vsy; ss >> vsy;

			m_camera->set_image_size(vsx, vsy);
			continue;
		}

		if (oper == "map_image") {
			std::string name; ss >> name;
			float scale; ss >> scale;

			scene_data.m_images.emplace_back(t_heightmap());
			scene_data.m_images.back().set_data(FreeImage_Load(FIF_PNG, name.c_str()), scale);
			continue;
		}

		if (oper == "light_source") {
			t_vector dir;
			t_color color;

			ss >> dir;
			ss >> color;

			scene_data.m_lights.push_back(new t_directional_light(dir.normalize_xyz(), color));
			continue;
		}
	}

	is.close();


	switch (m_scene_type) {
		case SCENETYPE_LINEAR:   { m_scene = new   t_linear_cell_scene<t_tri_cell>(); } break;
		case SCENETYPE_QUADTREE: { m_scene = new t_quadtree_cell_scene<t_tri_cell>(); } break;
		case SCENETYPE_KDTREE:   { m_scene = new   t_kdtree_cell_scene<t_tri_cell>(); } break;
	}

	assert(m_scene != 0);
	assert(!scene_data.m_images.empty());
	assert(!scene_data.m_lights.empty());

	m_scene->assign_heightmap(scene_data.m_images.back());

	for (size_t n = 0; n < scene_data.m_lights.size(); n++) {
		m_scene->assign_light_source(scene_data.m_lights[n]);
	}
}



void t_renderer::display() {
	const int64_t elapsed_time_ns = get_tick() - m_frame_tick; // ns
	const  double elapsed_time_s = std::min(1.0, elapsed_time_ns * 1e-9); // s

	m_frame_tick = get_tick();

	m_camera->pos() += (m_camera->dir() * camera_move_dist * elapsed_time_s);
	m_camera->dir().rotate_z(camera_azim_angle * elapsed_time_s);
	m_camera->dir().rotate_xy(camera_elev_angle * elapsed_time_s);
	m_camera->update(m_camera->dir());
	m_scene->modify_light_source(0, light_azim_angle * elapsed_time_s, light_elev_angle * elapsed_time_s);

	switch (m_mouse_button) {
		case GLUT_LEFT_BUTTON: {
			m_camera->dir().rotate_z(-m_diff_mouse_x * m_mouse_scale);
			m_camera->dir().rotate_xy(-m_diff_mouse_y * m_mouse_scale);
			m_camera->update(m_camera->dir());
		} break;
		case GLUT_RIGHT_BUTTON: {
			m_scene->modify_light_source(0, -m_diff_mouse_x * m_mouse_scale, -m_diff_mouse_y * m_mouse_scale);
		} break;
	}

	m_diff_mouse_x = 0;
	m_diff_mouse_y = 0;

	glClear(GL_COLOR_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();


	if (!m_threads.empty()) {
		#if (USE_BARRIERS == 1)
		// signal each thread to start its iteration
		m_barrier->wait();
		// wait for each thread to finish its iteration
		m_barrier->wait();
		#endif
	} else {
		if (m_trace_columns) {
			trace_ray_columns(0, m_camera->get_view_size_x(), 0, m_camera->get_view_size_y());
		} else {
			trace_rays(0, m_camera->get_view_size_x(), 0, m_camera->get_view_size_y());
		}
	}

	// show the composite result
	m_camera->draw_image();


	// glFlush();
	glutSwapBuffers();

	m_frame_count++;
}

void t_renderer::idle() {
	#if (USE_STANDARD_GLUT == 1)
	if (m_quit_tracing) {
		// glutLeaveMainLoop is not part of the standard GLUT lib
		// another non-default option would be glutMainLoopEvent
		exit(0);
	} else {
		glutPostRedisplay();
	}
	#else
	glutPostRedisplay();
	#endif
}



void t_renderer::keyboard_down(unsigned char key, int, int) {
	switch (key) {
		case 'x': { m_quit_tracing = true; } break;

		case 'a': { camera_azim_angle =  camera_rotate_speed; } break;
		case 'd': { camera_azim_angle = -camera_rotate_speed; } break;
		case 'w': { camera_elev_angle =  camera_rotate_speed; } break;
		case 's': { camera_elev_angle = -camera_rotate_speed; } break;

		case 'e': { camera_move_dist =  camera_transl_speed; } break;
		case 'q': { camera_move_dist = -camera_transl_speed; } break;

		case 'j': { light_azim_angle =  light_rotate_speed; } break;
		case 'l': { light_azim_angle = -light_rotate_speed; } break;
		case 'i': { light_elev_angle =  light_rotate_speed; } break;
		case 'k': { light_elev_angle = -light_rotate_speed; } break;
	}
}

void t_renderer::keyboard_up(unsigned char key, int, int) {
	switch (key) {
		case 'a': { camera_azim_angle = 0.0f; } break;
		case 'd': { camera_azim_angle = 0.0f; } break;
		case 'w': { camera_elev_angle = 0.0f; } break;
		case 's': { camera_elev_angle = 0.0f; } break;

		case 'e': { camera_move_dist = 0.0f; } break;
		case 'q': { camera_move_dist = 0.0f; } break;

		case 'j': { light_azim_angle = 0.0f; } break;
		case 'l': { light_azim_angle = 0.0f; } break;
		case 'i': { light_elev_angle = 0.0f; } break;
		case 'k': { light_elev_angle = 0.0f; } break;
	}
}

void t_renderer::mouse_state(int button, int state, int x, int y) {
	switch (state) {
		case GLUT_DOWN: {
			m_mouse_button = button;
			m_last_mouse_x = x;
			m_last_mouse_y = y;
		} break;

		case GLUT_UP: {
			m_mouse_button = -1;
		} break;
	}
}

void t_renderer::mouse_motion(int x, int y) {
	if (m_mouse_button >= 0) {
		m_diff_mouse_x = x - m_last_mouse_x;
		m_diff_mouse_y = y - m_last_mouse_y;
	}

	m_last_mouse_x = x;
	m_last_mouse_y = y;
}



void t_renderer::trace_rays(size_t xmin, size_t xmax, size_t ymin, size_t ymax) {
	const float fscale = std::tan(m_camera->fov() * 0.5f);
	const float aspect = m_camera->aspect();

	while (!m_quit_tracing) {
		#if (USE_BARRIERS == 1)
		m_barrier->wait();
		#endif

		const t_vector& cam_fwd_dir = m_camera->dir(CAM_FWD_DIR);
		const t_vector& cam_rgt_dir = m_camera->dir(CAM_RGT_DIR);
		const t_vector& cam_upw_dir = m_camera->dir(CAM_UPW_DIR);

		for (size_t y = ymin; y < ymax; y++) {
			const float yrel = (y * 1.0f / m_camera->get_view_size_y()) - 0.5f;
			const t_vector pxl_up_dir = cam_upw_dir * (yrel / aspect);

			for (size_t x = xmin; x < xmax; x++) {
				const float xrel = (x * 1.0f / m_camera->get_view_size_x()) - 0.5f;

				const t_vector pxl_rgt_dir = cam_rgt_dir * (xrel * fscale);
				const t_vector pxl_ray_dir = (cam_fwd_dir + pxl_up_dir + pxl_rgt_dir).normalize_xyz();

				m_camera->set_image_pixel(x, y, m_scene->trace_ray(t_ray(m_camera->pos(), pxl_ray_dir)));
			}
		}

		#if (USE_BARRIERS == 1)
		m_barrier->wait();
		#endif

		if (m_threads.empty()) {
			break;
		}
	}
}

void t_renderer::trace_ray_columns(size_t xmin, size_t xmax, size_t ymin, size_t ymax) {
	// a larger FOV means the rays will fan out wider, and moves
	// our (virtual) image-plane closer to the camera's position
	const float fscale = std::tan(m_camera->fov() * 0.5f);
	const float aspect = m_camera->aspect();

	// inefficient in ST mode
	std::vector<t_color> pxls(ymax - ymin);
	std::vector<t_vector> dirs(ymax - ymin);

	while (!m_quit_tracing) {
		#if (USE_BARRIERS == 1)
		m_barrier->wait();
		#endif

		const t_vector& cam_fwd_dir = m_camera->dir(CAM_FWD_DIR);
		const t_vector& cam_rgt_dir = m_camera->dir(CAM_RGT_DIR);
		const t_vector& cam_upw_dir = m_camera->dir(CAM_UPW_DIR);

		for (size_t x = xmin; x < xmax; x++) {
			const float xrel = (x * 1.0f / m_camera->get_view_size_x()) - 0.5f;
			const t_vector pxl_rgt_dir = cam_rgt_dir * (xrel * fscale);

			t_vector col_dir = cam_fwd_dir + pxl_rgt_dir;

			col_dir.z() = 0.0f;
			col_dir.normalize_xyz();

			// for each pixel in the column, set its image-plane direction
			for (size_t y = ymin; y < ymax; y++) {
				const float yrel = (y * 1.0f / m_camera->get_view_size_y()) - 0.5f;

				const t_vector pxl_up_dir = cam_upw_dir * (yrel * fscale / aspect);
				const t_vector pxl_ray_dir = (cam_fwd_dir + pxl_up_dir + pxl_rgt_dir).normalize_xyz();

				dirs[y - ymin] = pxl_ray_dir;
			}

			// trace the column of directions as one contiguous element
			m_scene->trace_ray_column(t_ray_column(m_camera->pos(), &dirs[0], col_dir.x(), col_dir.y(), ymax - ymin), &pxls[0]);

			for (size_t y = ymin; y < ymax; y++) {
				m_camera->set_image_pixel(x, y, pxls[y - ymin]);
			}
		}

		#if (USE_BARRIERS == 1)
		m_barrier->wait();
		#endif

		if (m_threads.empty()) {
			break;
		}
	}
}

void t_renderer::trace_ray_slope_columns(size_t xmin, size_t xmax, size_t ymin, size_t ymax) {
	const float fscale = std::tan(m_camera->fov() * 0.5f);
	const float aspect = m_camera->aspect();

	std::vector<t_color> pixels(ymax - ymin);
	std::vector<float> slopes(ymax - ymin);

	while (!m_quit_tracing) {
		#if (USE_BARRIERS == 1)
		m_barrier->wait();
		#endif

		const t_vector& cam_fwd_dir = m_camera->dir(CAM_FWD_DIR);
		const t_vector& cam_rgt_dir = m_camera->dir(CAM_RGT_DIR);
		const t_vector& cam_upw_dir = m_camera->dir(CAM_UPW_DIR);

		// for each pixel in the column, set its vertical slope
		for (size_t y = ymin; y < ymax; y++) {
			const float yrel = (y * 1.0f / m_camera->get_view_size_y()) - 0.5f;

			const t_vector pxl_up_dir = cam_upw_dir * (yrel / aspect);
			const t_vector col_fwd_dir = cam_fwd_dir + pxl_up_dir;

			slopes[y - ymin] = col_fwd_dir.get_slope();
		}

		for (size_t x = xmin; x < xmax; x++) {
			const float xrel = ((x * 1.0f + RAY_JITTER_RIGHT * (rnd_flt() - 0.5f)) / m_camera->get_view_size_x()) - 0.5f;

			const t_vector pxl_rgt_dir = cam_rgt_dir * (xrel * fscale);
			const t_vector pxl_col_dir = (cam_fwd_dir + pxl_rgt_dir).normalize_xy();

			// trace the column of slopes as one contiguous element
			// FIXME: vertical line in SS is not in general vertical in WS, needs multisampling
			m_scene->trace_slope_ray_column(t_slope_ray_column(m_camera->pos(), pxl_col_dir.x(), pxl_col_dir.y(), &slopes[0], ymax - ymin), &pixels[0]);

			for (size_t y = ymin; y < ymax; y++) {
				m_camera->set_image_pixel(x, y, pixels[y - ymin]);
			}
		}

		#if (USE_BARRIERS == 1)
		m_barrier->wait();
		#endif

		if (m_threads.empty()) {
			break;
		}
	}
}

