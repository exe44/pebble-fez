#pragma once

#include <pebble.h>
#include "app_settings.h"
#include "math_helper.h"

typedef struct DigitRendererState DigitRendererState;

typedef struct DigitRenderer
{
  DigitRendererState *state;
} DigitRenderer;

bool digit_renderer_init(DigitRenderer *renderer, Layer *root_layer,
  const AppSettings *settings, const Mat4 *view_matrix);
void digit_renderer_deinit(DigitRenderer *renderer);
void digit_renderer_set_digit(DigitRenderer *renderer, int index, int value, bool hidden);
void digit_renderer_mark_all_dirty(DigitRenderer *renderer);
bool digit_renderer_is_ready(const DigitRenderer *renderer);
