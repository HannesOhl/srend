// TODO: 1) finish unit tests
//	 2) some calculations dont make
// 	    sense for given input types, check that
//       3)

#ifndef LALG_H
#define LALG_H
#include <stdint.h>
#include <math.h>
#include <stddef.h>

// vectors: types
typedef struct {
	uint32_t x;
	uint32_t y;
} V2u;

typedef struct {
	int32_t x;
	int32_t y;
} V2s;

typedef union {
	struct {
		float x;
		float y;
	};
	float arr[2];
} V2f;

typedef union {
	struct {
		uint32_t x;
		uint32_t y;
		uint32_t z;
	};
	uint32_t arr[3];
} V3u;

typedef union {
	struct {
		int32_t x;
		int32_t y;
		int32_t z;
	};
	int32_t arr[3];
} V3s;

typedef union {
	struct {
		float x;
		float y;
		float z;
	};
	float arr[3];
} V3f;

// vectors: addition (we exclude matrix-matrix addition)
static inline V3u add_3u(V3u a, V3u b) {
	return (V3u) {{ a.x + b.x, a.y + b.y, a.z + b.z }};
}
static inline V3s add_3s(V3s a, V3s b) {
	return (V3s) {{ a.x + b.x, a.y + b.y, a.z + b.z }};
}
static inline V3f add_3f(V3f a, V3f b) {
	return (V3f) {{ a.x + b.x, a.y + b.y, a.z + b.z }};
}
#define add(a, b)		\
	_Generic((a), 		\
		V3u: add_3u, 	\
		V3s: add_3s, 	\
		V3f: add_3f	\
	)((a), (b))

// vectors: subtraction (we exclude matrix-matrix subtraction)
static inline V3u sub_3u(V3u a, V3u b) {
	return (V3u) {{ a.x - b.x, a.y - b.y, a.z - b.z }};
}
static inline V3s sub_3s(V3s a, V3s b) {
	return (V3s) {{ a.x - b.x, a.y - b.y, a.z - b.z }};
}
static inline V3f sub_3f(V3f a, V3f b) {
	return (V3f) {{ a.x - b.x, a.y - b.y, a.z - b.z }};
}
#define sub(a, b)		\
	_Generic((a), 		\
		V3u: sub_3u, 	\
		V3s: sub_3s, 	\
		V3f: sub_3f	\
	)((a), (b))

// vectors: dot product
static inline uint32_t dot_2u(V2u a, V2u b) {
	return ( a.x * b.x + a.y * b.y );
}
static inline  int32_t dot_2s(V2s a, V2s b) {
	return ( a.x * b.x + a.y * b.y );
}
static inline    float dot_2f(V2f a, V2f b) {
	return ( a.x * b.x + a.y * b.y );
}
static inline uint32_t dot_3u(V3u a, V3u b) {
	return ( a.x * b.x + a.y * b.y + a.z * b.z );
}
static inline  int32_t dot_3s(V3s a, V3s b) {
	return ( a.x * b.x + a.y * b.y + a.z * b.z );
}
static inline    float dot_3f(V3f a, V3f b) {
	return ( a.x * b.x + a.y * b.y + a.z * b.z );
}
#define dot(a, b)		\
	_Generic((a), 		\
		V3u: dot_3u, 	\
		V3s: dot_3s, 	\
		V3f: dot_3f	\
	)((a), (b))

// vectors: cross product
static inline V3u cross_3u(V3u a, V3u b) {
	return (V3u) {{ a.y * b.z - a.z * b.y,
			a.z * b.x - a.x * b.z,
			a.x * b.y - a.y * b.x }};
}
static inline V3s cross_3s(V3s a, V3s b) {
	return (V3s) {{ a.y * b.z - a.z * b.y,
			a.z * b.x - a.x * b.z,
			a.x * b.y - a.y * b.x }};
}
static inline V3f cross_3f(V3f a, V3f b) {
	return (V3f) {{ a.y * b.z - a.z * b.y,
			a.z * b.x - a.x * b.z,
			a.x * b.y - a.y * b.x }};
}
#define cross(a, b)		\
	_Generic((a), 		\
		V3u: cross_3u, 	\
		V3s: cross_3s, 	\
		V3f: cross_3f	\
	)((a), (b))

// vectors: normalize
static inline V2f norm_2f(V2f a) {
	float l2 = a.x*a.x + a.y*a.y;
	if (l2 < 1e-8) return (V2f) {};
	float l_inv = 1.0f / sqrtf(l2);
	return (V2f) {{ a.x*l_inv, a.y*l_inv }};
}
static inline V3f norm_3f(V3f a) {
	float l2 = a.x*a.x + a.y*a.y + a.z*a.z;
	if (l2 < 1e-8) return (V3f) {};
	float l_inv = 1.0f / sqrtf(l2);
	return (V3f) {{ a.x*l_inv, a.y*l_inv, a.z*l_inv }};
}
#define norm(a)			\
	_Generic((a), 		\
		V2f: norm_2f,	\
		V3f: norm_3f	\
	)((a))

// vectors: scale
static inline V3u scale_3u(float fac, V3u a) {
	return (V3u) {{ fac * a.x, fac * a.y, fac * a.z }};
}
static inline V3s scale_3s(float fac, V3s a) {
	return (V3s) {{ fac * a.x, fac * a.y, fac * a.z }};
}
static inline V3f scale_3f(float fac, V3f a) {
	return (V3f) {{ fac * a.x, fac * a.y, fac * a.z }};
}
#define scale(a, b)		\
	_Generic((b), 		\
		V3u: scale_3u, 	\
		V3s: scale_3s, 	\
		V3f: scale_3f	\
	)((a), (b))

// vectors: intersection of line segment
static inline V3f intersect_z(V3f a, V3f b, float z) {
	float dz = b.z - a.z;
	if (dz*dz < 1e-8f) return a;
	float lambda = (z - a.z) / dz;
	return (V3f) {{ a.x + lambda * (b.x - a.x), a.y + lambda * (b.y - a.y), z }};
}

// matrices: types
typedef union {
	struct {
		uint32_t m00, m01, m02;
		uint32_t m10, m11, m12;
		uint32_t m20, m21, m22;
	};
	uint32_t arr[9];
} M3u;
typedef union {
	struct {
		float m00, m01, m02;
		float m10, m11, m12;
		float m20, m21, m22;
	};
	float arr[9];
} M3f;

// matrices: multplication matrix-vector and matrix-matrix
static inline V3u mul_m3u_v3u(M3u m, V3u a) {
	return (V3u) {{ m.arr[0]*a.arr[0] + m.arr[3]*a.arr[1] + m.arr[6]*a.arr[2],
			m.arr[1]*a.arr[0] + m.arr[4]*a.arr[1] + m.arr[7]*a.arr[2],
			m.arr[2]*a.arr[0] + m.arr[5]*a.arr[1] + m.arr[8]*a.arr[2] }};
}
static inline V3f mul_m3f_v3f(M3f m, V3f a) {
	return (V3f) {{ m.arr[0]*a.arr[0] + m.arr[3]*a.arr[1] + m.arr[6]*a.arr[2],
			m.arr[1]*a.arr[0] + m.arr[4]*a.arr[1] + m.arr[7]*a.arr[2],
			m.arr[2]*a.arr[0] + m.arr[5]*a.arr[1] + m.arr[8]*a.arr[2] }};
}
static inline M3u mul_m3u_m3u(M3u m, M3u n) {
	M3u res = {0};
	for (size_t i = 0; i < 3; i++) {
	for (size_t j = 0; j < 3; j++) {
		for (size_t k = 0; k < 3; k++) {
			res.arr[i + j*3] += m.arr[i + k*3] * n.arr[k + j*3];
		}
	}}
	return res;
}
static inline M3f mul_m3f_m3f(M3f m, M3f n) {
	M3f res = {0};
	for (size_t i = 0; i < 3; i++) {
	for (size_t j = 0; j < 3; j++) {
		for (size_t k = 0; k < 3; k++) {
			res.arr[i + j*3] += m.arr[i + k*3] * n.arr[k + j*3];
		}
	}}
	return res;
}
#define mul(a, b) 				\
	_Generic((a), 				\
		M3u: _Generic((b), 		\
			V3u: mul_m3u_v3u,	\
			M3u: mul_m3u_m3u	\
		),				\
		M3f: _Generic((b), 		\
			V3f: mul_m3f_v3f,	\
			M3f: mul_m3f_m3f	\
		)				\
	)((a), (b));

static inline V3f rot_rod_3f(V3f vector, V3f axis, float angle) {

	V3f res = {0};

	float cosine = cosf(angle);
	float sine   = sinf(angle);

	res = add( add(scale(cosine, vector), scale(sine, cross(axis, vector))),
		   scale((1.0f-cosine) * dot(axis, vector), axis) );

	return res;
}

// other types
typedef struct {
	V3f v1;
	V3f v2;
	V3f v3;
} Triangle;

// other utilities
static inline float maxf(float value, float max) {

	return value < max ? max : value;
}

#ifdef UNIT_TEST

#include <assert.h>

#define TYPE_CHECK(expression, T) _Generic((expression), T: true, default: false)

V3u TEST_u1 = { .x =   1u, .y =   2u, .z =   3u };
V3u TEST_u2 = { .x =   1u, .y =   2u, .z =   3u };
V3s TEST_s1 = { .x =   -1, .y =   -2, .z =   -3 };
V3s TEST_s2 = { .x =   -1, .y =   -2, .z =   -3 };
V3f TEST_f1 = { .x = 0.1f, .y = 0.2f, .z = 0.3f };
V3f TEST_f2 = { .x = 0.4f, .y = 0.5f, .z = 0.6f };

// begin with asserting return types
static_assert(TYPE_CHECK( add(TEST_u1, TEST_u2), V3u), "");
static_assert(TYPE_CHECK( add(TEST_s1, TEST_s2), V3s), "");
static_assert(TYPE_CHECK( add(TEST_f1, TEST_f2), V3f), "");

static_assert(TYPE_CHECK( sub(TEST_u1, TEST_u2), V3u), "");
static_assert(TYPE_CHECK( sub(TEST_s1, TEST_s2), V3s), "");
static_assert(TYPE_CHECK( sub(TEST_f1, TEST_f2), V3f), "");

static_assert(TYPE_CHECK( dot(TEST_u1, TEST_u2), uint32_t), "");
static_assert(TYPE_CHECK( dot(TEST_s1, TEST_s2),  int32_t), "");
static_assert(TYPE_CHECK( dot(TEST_f1, TEST_f2),    float), "");

static_assert(TYPE_CHECK( cross(TEST_u1, TEST_u2), V3u), "");
static_assert(TYPE_CHECK( cross(TEST_s1, TEST_s2), V3s), "");
static_assert(TYPE_CHECK( cross(TEST_f1, TEST_f2), V3f), "");

static_assert(TYPE_CHECK( norm(TEST_f1), V3f), "");

static bool LALG_test_add() {

	fprintf(stdout, "\tadd:\n");

	V3u res_u = add(TEST_u1, TEST_u2);
	assert(res_u.x == 2u && res_u.y == 4u && res_u.z == 6u);
	fprintf(stdout, "\t\tV3u passed!\n");

	V3s res_s = add(TEST_s1, TEST_s2);
	assert(res_s.x == -2 && res_s.y == -4 && res_s.z == -6);
	fprintf(stdout, "\t\tV3s passed!\n");

	float EPS = 1e-6f;
	V3f res_f = add(TEST_f1, TEST_f2);
	assert( fabsf(res_f.x-0.5f) < EPS &&
		fabsf(res_f.y-0.7f) < EPS &&
		fabsf(res_f.z-0.9f) < EPS);
	fprintf(stdout, "\t\tV3f passed!\n");

	return true;
}

static bool LALG_tests_run() {

	fprintf(stdout, "running all lalg.h unit tests:\n");
	LALG_test_add();
	return true;
}

#endif // UNIT_TEST

#endif // LALG_H

