#pragma once

//==============================================================================
// sqrt implement from Quake 3

#define SQRT_MAGIC_F 0x5f3759df

float q_sqrt(const float x);

//==============================================================================

typedef struct Vec3
{
  float x, y, z;
} Vec3;

#define Vec3(x, y, z) ((Vec3){(x), (y), (z)})

void vec3_plus(Vec3* out_v, const Vec3* v1, const Vec3* v2);
void vec3_minus(Vec3* out_v, const Vec3* v1, const Vec3* v2);
void vec3_multiply(Vec3* out_v, const Vec3* v1, float multiplier);
float vec3_length(const Vec3* v);
float vec3_normalize(Vec3* v);
void vec3_cross_product(Vec3* out_v, const Vec3* v1, const Vec3* v2);

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
  float m30, float m31, float m32, float m33);
void mat4_multiply(Mat4* out_m, const Mat4* m1, const Mat4* m2);
void mat4_multiply_vec3(Vec3* out_v, const Mat4* m, const Vec3* v);
void mat4_translate(Mat4* m, const Vec3* translate);
void mat4_look_at_rh(Mat4* out_m, const Vec3* eye, const Vec3* at, const Vec3* up);
