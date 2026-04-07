#pragma once

#include <pebble.h>
#include "app_settings.h"
#include "math_helper.h"
#include "poly_data.h"

#define DIGIT_RENDERER_DIGIT_COUNT 4

typedef struct Poly
{
  Vec3 center;
  const DigitPolyData *poly_data;
} Poly;

typedef struct DigitRenderer
{
  Layer *digits[DIGIT_RENDERER_DIGIT_COUNT];
  Poly number_polys[10];
  GPoint screen_center;
  float poly_scale;
  GSize digit_layer_size;
  Vec3 digit_positions[DIGIT_RENDERER_DIGIT_COUNT];
  const AppSettings *settings;
  const Mat4 *view_matrix;
  bool is_ready;
} DigitRenderer;

void digit_renderer_init(DigitRenderer *renderer, Layer *root_layer,
  const AppSettings *settings, const Mat4 *view_matrix);
void digit_renderer_deinit(DigitRenderer *renderer);
void digit_renderer_set_digit(DigitRenderer *renderer, int index, int value, bool hidden);
void digit_renderer_mark_all_dirty(DigitRenderer *renderer);
bool digit_renderer_is_ready(const DigitRenderer *renderer);
