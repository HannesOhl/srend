// TODO: 1) finish unit tests

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

// matrices: types
typedef union {
	struct {
		int32_t e00, e01, e02;
		int32_t e10, e11, e12;
		int32_t e20, e21, e22;
	};
	uint32_t arr[9];
} M3s;

typedef union {
	struct {
		uint32_t e00, e01, e02;
		uint32_t e10, e11, e12;
		uint32_t e20, e21, e22;
	};
	uint32_t arr[9];
} M3u;

typedef union {
	struct {
		float e00, e01, e02;
		float e10, e11, e12;
		float e20, e21, e22;
	};
	float arr[9];
} M3f;

// addition
static inline V3u add_v_3u(V3u a, V3u b) {
	return (V3u) {{ a.x + b.x, a.y + b.y, a.z + b.z }};
}
static inline V3s add_v_3s(V3s a, V3s b) {
	return (V3s) {{ a.x + b.x, a.y + b.y, a.z + b.z }};
}
static inline V3f add_v_3f(V3f a, V3f b) {
	return (V3f) {{ a.x + b.x, a.y + b.y, a.z + b.z }};
}
static inline M3u add_m_3u(M3u m, M3u n) {
	return (M3u) {{ m.e00+n.e00, m.e01+n.e01, m.e02+n.e02,
			m.e10+n.e10, m.e11+n.e11, m.e12+n.e12,
			m.e20+n.e20, m.e21+n.e21, m.e22+n.e22 }};
}
static inline M3s add_m_3s(M3s m, M3s n) {
	return (M3s) {{ m.e00+n.e00, m.e01+n.e01, m.e02+n.e02,
			m.e10+n.e10, m.e11+n.e11, m.e12+n.e12,
			m.e20+n.e20, m.e21+n.e21, m.e22+n.e22 }};
}
static inline M3f add_m_3f(M3f m, M3f n) {
	return (M3f) {{ m.e00+n.e00, m.e01+n.e01, m.e02+n.e02,
			m.e10+n.e10, m.e11+n.e11, m.e12+n.e12,
			m.e20+n.e20, m.e21+n.e21, m.e22+n.e22 }};
}
#define add(a, b)		\
	_Generic((a), 		\
		V3u: add_v_3u, 	\
		V3s: add_v_3s, 	\
		V3f: add_v_3f,	\
		M3u: add_m_3u,	\
		M3s: add_m_3s,	\
		M3f: add_m_3f	\
	)((a), (b))

// subtraction
static inline V3u sub_v_3u(V3u a, V3u b) {
	return (V3u) {{ a.x - b.x, a.y - b.y, a.z - b.z }};
}
static inline V3s sub_v_3s(V3s a, V3s b) {
	return (V3s) {{ a.x - b.x, a.y - b.y, a.z - b.z }};
}
static inline V3f sub_v_3f(V3f a, V3f b) {
	return (V3f) {{ a.x - b.x, a.y - b.y, a.z - b.z }};
}
static inline M3u sub_m_3u(M3u m, M3u n) {
	return (M3u) {{ m.e00-n.e00, m.e01-n.e01, m.e02-n.e02,
			m.e10-n.e10, m.e11-n.e11, m.e12-n.e12,
			m.e20-n.e20, m.e21-n.e21, m.e22-n.e22 }};
}
static inline M3s sub_m_3s(M3s m, M3s n) {
	return (M3s) {{ m.e00-n.e00, m.e01-n.e01, m.e02-n.e02,
			m.e10-n.e10, m.e11-n.e11, m.e12-n.e12,
			m.e20-n.e20, m.e21-n.e21, m.e22-n.e22 }};
}
static inline M3f sub_m_3f(M3f m, M3f n) {
	return (M3f) {{ m.e00-n.e00, m.e01-n.e01, m.e02-n.e02,
			m.e10-n.e10, m.e11-n.e11, m.e12-n.e12,
			m.e20-n.e20, m.e21-n.e21, m.e22-n.e22 }};
}
#define sub(a, b)		\
	_Generic((a), 		\
		V3u: sub_v_3u, 	\
		V3s: sub_v_3s, 	\
		V3f: sub_v_3f,	\
		M3u: sub_m_3u,	\
		M3s: sub_m_3s,	\
		M3f: sub_m_3f	\
	)((a), (b))

// dot product
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
static inline V3f scale_v_3f(float fac, V3f a) {
	return (V3f) {{ fac * a.x, fac * a.y, fac * a.z }};
}
static inline M3f scale_m_3f(float fac, M3f m) {
	return (M3f) {{ fac*m.e00, fac*m.e01, fac*m.e02,
			fac*m.e11, fac*m.e12, fac*m.e12,
			fac*m.e21, fac*m.e22, fac*m.e22 }};
}
#define scale(a, b)		 \
	_Generic((b), 		 \
		V3f: scale_v_3f, \
		M3f: scale_m_3f	 \
	)((a), (b))

// matrices: multplication matrix-vector and matrix-matrix
static inline V3u mul_m3u_v3u(M3u m, V3u a) {
	return (V3u) {{ m.e00*a.x + m.e10*a.y + m.e20*a.z,
			m.e01*a.x + m.e11*a.y + m.e21*a.z,
			m.e02*a.x + m.e12*a.y + m.e22*a.z }};
}
static inline V3s mul_m3s_v3s(M3s m, V3s a) {
	return (V3s) {{ m.e00*a.x + m.e10*a.y + m.e20*a.z,
			m.e01*a.x + m.e11*a.y + m.e21*a.z,
			m.e02*a.x + m.e12*a.y + m.e22*a.z }};
}
static inline V3f mul_m3f_v3f(M3f m, V3f a) {
	return (V3f) {{ m.e00*a.x + m.e10*a.y + m.e20*a.z,
			m.e01*a.x + m.e11*a.y + m.e21*a.z,
			m.e02*a.x + m.e12*a.y + m.e22*a.z }};
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
		M3s: _Generic((b), 		\
			V3s: mul_m3s_v3s,	\
			M3s: mul_m3s_m3s	\
		),				\
		M3f: _Generic((b), 		\
			V3f: mul_m3f_v3f,	\
			M3f: mul_m3f_m3f	\
		)				\
	)((a), (b));

static inline M3u transpose_3u(M3u m) {
	return (M3u) {{ m.e00, m.e10, m.e20,
			m.e01, m.e11, m.e21,
			m.e02, m.e12, m.e22 }};
}
static inline M3s transpose_3s(M3s m) {
	return (M3s) {{ m.e00, m.e10, m.e20,
			m.e01, m.e11, m.e21,
			m.e02, m.e12, m.e22 }};
}
static inline M3f transpose_3f(M3f m) {
	return (M3f) {{ m.e00, m.e10, m.e20,
			m.e01, m.e11, m.e21,
			m.e02, m.e12, m.e22 }};
}
#define transpose(a)  			\
	_Generic((a), 			\
		M3u: transpose_3u, 	\
		M3s: transpose_3s, 	\
		M3f: transpose_3f, 	\
	)

static inline V3f rot_ax_3f(V3f vector, V3f axis, float angle) {
	V3f res = {0};
	float cosine = cosf(angle);
	float sine   = sinf(angle);
	res = add( add(scale(cosine, vector), scale(sine, cross(axis, vector))),
		   scale((1.0f-cosine) * dot(axis, vector), axis) );

	return res;
}
#define rot_ax(a, b, c)	       \
	_Generic((a), 	       \
		V3f: rot_ax_3f \
	)((a), (b), (c))

// other types
typedef struct {
	V3f v1;
	V3f v2;
	V3f v3;
} Triangle;

// other utilities

static inline V3f intersect_z(V3f a, V3f b, float z) {
	float dz = b.z - a.z;
	if (dz*dz < 1e-8f) return a;
	float lambda = (z - a.z) / dz;
	return (V3f) {{ a.x + lambda * (b.x - a.x), a.y + lambda * (b.y - a.y), z }};
}

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

static bool LALG_test_sub() {

	fprintf(stdout, "\tsub:\n");

	V3u res_u = sub(TEST_u1, TEST_u2);
	assert(res_u.x == 0u && res_u.y == 0u && res_u.z == 0u);
	fprintf(stdout, "\t\tV3u passed!\n");

	V3s res_s = sub(TEST_s1, TEST_s2);
	assert(res_s.x == 0 && res_s.y == 0 && res_s.z == 0);
	fprintf(stdout, "\t\tV3s passed!\n");

	float EPS = 1e-6f;
	V3f res_f = sub(TEST_f1, TEST_f2);
	assert( fabsf(res_f.x-(-0.3f)) < EPS &&
		fabsf(res_f.y-(-0.3f)) < EPS &&
		fabsf(res_f.z-(-0.3f)) < EPS);
	fprintf(stdout, "\t\tV3f passed!\n");

	return true;
}

static bool LALG_tests_run() {

	fprintf(stdout, "running all lalg.h unit tests:\n");
	LALG_test_add();
	LALG_test_sub();
	return true;
}

#endif // UNIT_TEST

#endif // LALG_H

