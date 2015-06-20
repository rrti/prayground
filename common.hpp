#pragma once

// without barriers, performance doubles
// but thread-safety goes out the window
//
// TODO: switch to a producer/consumer pattern?
#define USE_BARRIERS 1
#define USE_REF_ARGS 1

#define USE_STANDARD_GLUT 1

#define RAY_JITTER_RIGHT 0.5f
#define RAY_TEST_EPSILON 0.001f

class t_ray;
class t_vector;

#if (USE_REF_ARGS == 1)
typedef const t_ray& t_const_ray;
typedef const t_vector& t_const_vec;
#else
typedef const t_ray t_const_ray;
typedef const t_vector t_const_vec;
#endif

