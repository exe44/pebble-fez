
//==============================================================================
// sqrt implement from Quake 3

#define SQRT_MAGIC_F 0x5f3759df

float q_sqrt(const float x)
{
  const float xhalf = 0.5f * x;
 
  union // get bits for floating value
  {
    float x;
    int i;
  } u;

  u.x = x;
  u.i = SQRT_MAGIC_F - (u.i >> 1); // gives initial guess y0

  return x * u.x * (1.5f - xhalf * u.x * u.x); // Newton step, repeating increases accuracy 
}

//==============================================================================

typedef struct Vec3
{
  float x, y, z;
} Vec3;

#define Vec3(x, y, z) ((Vec3){(x), (y), (z)})

void vec3_plus(Vec3* out_v, const Vec3* v1, const Vec3* v2)
{
  out_v->x = v1->x + v2->x;
  out_v->y = v1->y + v2->y;
  out_v->z = v1->z + v2->z;
}

void vec3_minus(Vec3* out_v, const Vec3* v1, const Vec3* v2)
{
  out_v->x = v1->x - v2->x;
  out_v->y = v1->y - v2->y;
  out_v->z = v1->z - v2->z;
}

void vec3_multiply(Vec3* out_v, const Vec3* v1, float multiplier)
{
  out_v->x = v1->x * multiplier;
  out_v->y = v1->y * multiplier;
  out_v->z = v1->z * multiplier;
}

float vec3_length(const Vec3* v)
{
  return q_sqrt(v->x * v->x + v->y * v->y + v->z * v->z);
}

float vec3_normalize(Vec3* v)
{
  float length = vec3_length(v);
    
  // Will also work for zero-sized vectors, but will change nothing
  if (length > 1e-06)
  {
    float inv_length = 1.0f / length;
    v->x *= inv_length;
    v->y *= inv_length;
    v->z *= inv_length;
  }
  
  return length;
}

void vec3_cross_product(Vec3* out_v, const Vec3* v1, const Vec3* v2)
{
  out_v->x = v1->y * v2->z - v1->z * v2->y;
  out_v->y = v1->z * v2->x - v1->x * v2->z;
  out_v->z = v1->x * v2->y - v1->y * v2->x;
}

//==============================================================================

typedef struct Mat4
{
  float m[16];
} Mat4;

enum Mat4RowCol  // column-major
{
  _00 = 0,
  _10,
  _20,
  _30,
  
  _01,
  _11,
  _21,
  _31,
  
  _02,
  _12,
  _22,
  _32,
  
  _03,
  _13,
  _23,
  _33
};

void mat4_set(Mat4* m,
  float m00, float m01, float m02, float m03,
  float m10, float m11, float m12, float m13,
  float m20, float m21, float m22, float m23,
  float m30, float m31, float m32, float m33)
{
  m->m[_00] = m00; m->m[_01] = m01; m->m[_02] = m02; m->m[_03] = m03;
  m->m[_10] = m10; m->m[_11] = m11; m->m[_12] = m12; m->m[_13] = m13;
  m->m[_20] = m20; m->m[_21] = m21; m->m[_22] = m22; m->m[_23] = m23;
  m->m[_30] = m30; m->m[_31] = m31; m->m[_32] = m32; m->m[_33] = m33;
}

void mat4_multiply(Mat4* out_m, const Mat4* m1, const Mat4* m2)
{
  out_m->m[_00] = m1->m[_00] * m2->m[_00] + m1->m[_01] * m2->m[_10] + m1->m[_02] * m2->m[_20] + m1->m[_03] * m2->m[_30];
  out_m->m[_01] = m1->m[_00] * m2->m[_01] + m1->m[_01] * m2->m[_11] + m1->m[_02] * m2->m[_21] + m1->m[_03] * m2->m[_31];
  out_m->m[_02] = m1->m[_00] * m2->m[_02] + m1->m[_01] * m2->m[_12] + m1->m[_02] * m2->m[_22] + m1->m[_03] * m2->m[_32];
  out_m->m[_03] = m1->m[_00] * m2->m[_03] + m1->m[_01] * m2->m[_13] + m1->m[_02] * m2->m[_23] + m1->m[_03] * m2->m[_33];
  
  out_m->m[_10] = m1->m[_10] * m2->m[_00] + m1->m[_11] * m2->m[_10] + m1->m[_12] * m2->m[_20] + m1->m[_13] * m2->m[_30];
  out_m->m[_11] = m1->m[_10] * m2->m[_01] + m1->m[_11] * m2->m[_11] + m1->m[_12] * m2->m[_21] + m1->m[_13] * m2->m[_31];
  out_m->m[_12] = m1->m[_10] * m2->m[_02] + m1->m[_11] * m2->m[_12] + m1->m[_12] * m2->m[_22] + m1->m[_13] * m2->m[_32];
  out_m->m[_13] = m1->m[_10] * m2->m[_03] + m1->m[_11] * m2->m[_13] + m1->m[_12] * m2->m[_23] + m1->m[_13] * m2->m[_33];
  
  out_m->m[_20] = m1->m[_20] * m2->m[_00] + m1->m[_21] * m2->m[_10] + m1->m[_22] * m2->m[_20] + m1->m[_23] * m2->m[_30];
  out_m->m[_21] = m1->m[_20] * m2->m[_01] + m1->m[_21] * m2->m[_11] + m1->m[_22] * m2->m[_21] + m1->m[_23] * m2->m[_31];
  out_m->m[_22] = m1->m[_20] * m2->m[_02] + m1->m[_21] * m2->m[_12] + m1->m[_22] * m2->m[_22] + m1->m[_23] * m2->m[_32];
  out_m->m[_23] = m1->m[_20] * m2->m[_03] + m1->m[_21] * m2->m[_13] + m1->m[_22] * m2->m[_23] + m1->m[_23] * m2->m[_33];
  
  out_m->m[_30] = m1->m[_30] * m2->m[_00] + m1->m[_31] * m2->m[_10] + m1->m[_32] * m2->m[_20] + m1->m[_33] * m2->m[_30];
  out_m->m[_31] = m1->m[_30] * m2->m[_01] + m1->m[_31] * m2->m[_11] + m1->m[_32] * m2->m[_21] + m1->m[_33] * m2->m[_31];
  out_m->m[_32] = m1->m[_30] * m2->m[_02] + m1->m[_31] * m2->m[_12] + m1->m[_32] * m2->m[_22] + m1->m[_33] * m2->m[_32];
  out_m->m[_33] = m1->m[_30] * m2->m[_03] + m1->m[_31] * m2->m[_13] + m1->m[_32] * m2->m[_23] + m1->m[_33] * m2->m[_33];
}

void mat4_multiply_vec3(Vec3* out_v, const Mat4* m, const Vec3* v)
{
  float inv_w = 1.0f / (m->m[_30] * v->x + m->m[_31] * v->y + m->m[_32] * v->z + m->m[_33]);
  
  out_v->x = (m->m[_00] * v->x + m->m[_01] * v->y + m->m[_02] * v->z + m->m[_03]) * inv_w;
  out_v->y = (m->m[_10] * v->x + m->m[_11] * v->y + m->m[_12] * v->z + m->m[_13]) * inv_w;
  out_v->z = (m->m[_20] * v->x + m->m[_21] * v->y + m->m[_22] * v->z + m->m[_23]) * inv_w;
}

void mat4_translate(Mat4* m, const Vec3* translate)
{
  m->m[_00] = 1.0f; m->m[_01] = 0.0f; m->m[_02] = 0.0f; m->m[_03] = translate->x;
  m->m[_10] = 0.0f; m->m[_11] = 1.0f; m->m[_12] = 0.0f; m->m[_13] = translate->y;
  m->m[_20] = 0.0f; m->m[_21] = 0.0f; m->m[_22] = 1.0f; m->m[_23] = translate->z;
  m->m[_30] = 0.0f; m->m[_31] = 0.0f; m->m[_32] = 0.0f; m->m[_33] = 1.0f;
}

void MatrixLookAtRH(Mat4* out_m, const Vec3* eye, const Vec3* at, const Vec3* up)
{
  Vec3 f, up_actual, s, u;

  vec3_minus(&f, at, eye);
  vec3_normalize(&f);

  up_actual = *up;
  vec3_normalize(&up_actual);

  vec3_cross_product(&s, &f, &up_actual);
  vec3_cross_product(&u, &s, &f);

  Mat4 t1;
  mat4_set(&t1,
     s.x,  s.y,  s.z,    0,
     u.x,  u.y,  u.z,    0,
    -f.x, -f.y, -f.z,    0,
       0,    0,    0,    1);

  Mat4 t2;
  Vec3 neg_eye;
  vec3_multiply(&neg_eye, eye, -1);
  mat4_translate(&t2, &neg_eye);

  mat4_multiply(out_m, &t1, &t2);
}
