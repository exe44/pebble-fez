#pragma once

#include <pebble.h>

typedef struct AppSettings
{
  bool slow_version;
  int32_t bg_color;
  int32_t face_color;
  bool face_mix_with_background;
  int32_t line_color;
  bool line_mix_with_background;
  bool split_line_colors;
  int32_t back_line_color;
  int32_t side_line_color;
} AppSettings;

enum
{
  PERSIST_KEY_SLOW_VERSION = 1,
  PERSIST_KEY_BG_COLOR = 2,
  PERSIST_KEY_FACE_COLOR = 3,
  PERSIST_KEY_FACE_MIX_WITH_BACKGROUND = 4,
  PERSIST_KEY_LINE_COLOR = 5,
  PERSIST_KEY_LINE_MIX_WITH_BACKGROUND = 6,
  PERSIST_KEY_SPLIT_LINE_COLORS = 7,
  PERSIST_KEY_BACK_LINE_COLOR = 8,
  PERSIST_KEY_SIDE_LINE_COLOR = 9,
};

void app_settings_load(AppSettings *settings);
void app_settings_save(const AppSettings *settings);
bool app_settings_apply_message(AppSettings *settings, DictionaryIterator *iterator);
GColor app_settings_get_background_color(const AppSettings *settings);
GColor app_settings_get_line_color(const AppSettings *settings);
GColor app_settings_get_back_line_color(const AppSettings *settings);
GColor app_settings_get_side_line_color(const AppSettings *settings);
GColor app_settings_get_face_color(const AppSettings *settings);
