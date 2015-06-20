#include <GL/glut.h>

#include "renderer.hpp"

static t_renderer* renderer = 0;



void display() { renderer->display(); }
void idle() { renderer->idle(); }
#if (USE_STANDARD_GLUT == 1)
void kill() { delete renderer; renderer = 0; }
#endif

static void keyboard_down(unsigned char key, int x, int y) { renderer->keyboard_down(key, x, y); }
static void keyboard_up(unsigned char key, int x, int y) { renderer->keyboard_up(key, x, y); }
static void mouse_state(int button, int state, int x, int y) { renderer->mouse_state(button, state, x, y); }
static void mouse_motion(int x, int y) { renderer->mouse_motion(x, y); }



int main(int argc, char** argv) {
	#if (USE_STANDARD_GLUT == 1)
	atexit(kill);
	#endif

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);

	renderer = new t_renderer();
	renderer->read_config((argc > 1)? argv[1]: "config.txt");
	renderer->spawn_threads();

	glutInitWindowPosition(100, 100);
	glutInitWindowSize((renderer->get_camera())->get_view_size_x(), (renderer->get_camera())->get_view_size_y());
	glutCreateWindow("prayground");

	// needs to be done after glut*Window (without using reshape)
	renderer->set_viewport((renderer->get_camera())->get_view_size_x(), (renderer->get_camera())->get_view_size_y());

	glutIgnoreKeyRepeat(GLUT_KEY_REPEAT_OFF); 
	glutDisplayFunc(display);
	glutIdleFunc(idle);	
	glutKeyboardFunc(keyboard_down);
	glutKeyboardUpFunc(keyboard_up);
	glutMouseFunc(mouse_state);
	glutMotionFunc(mouse_motion);

	#if (USE_STANDARD_GLUT == 1)
	glutMainLoop();
	#else
	while (!renderer->want_quit()) {
		glutMainLoopEvent();
	}

	delete renderer;
	#endif

	return 0;
}

