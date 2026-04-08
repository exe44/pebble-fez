#pragma once

#include <pebble.h>

typedef enum ColorMode
{
  COLOR_MODE_FOREGROUND = 0,
  COLOR_MODE_BACKGROUND = 1,
  COLOR_MODE_ACCENT = 2,
  COLOR_MODE_MIXED = 3,
} ColorMode;

typedef struct AppSettings
{
  bool slow_version;
  int32_t fg_color;
  int32_t bg_color;
  int32_t accent_color;
  ColorMode line_color_mode;
  ColorMode face_color_mode;
} AppSettings;

enum
{
  PERSIST_KEY_SLOW_VERSION = 1,
  PERSIST_KEY_FG_COLOR = 2,
  PERSIST_KEY_BG_COLOR = 3,
  PERSIST_KEY_LINE_COLOR_MODE = 4,
  PERSIST_KEY_FACE_COLOR_MODE = 5,
  PERSIST_KEY_ACCENT_COLOR = 6,
};

void app_settings_load(AppSettings *settings);
void app_settings_save(const AppSettings *settings);
bool app_settings_apply_message(AppSettings *settings, DictionaryIterator *iterator);
GColor app_settings_get_foreground_color(const AppSettings *settings);
GColor app_settings_get_background_color(const AppSettings *settings);
GColor app_settings_get_accent_color(const AppSettings *settings);
GColor app_settings_get_line_color(const AppSettings *settings);
GColor app_settings_get_face_color(const AppSettings *settings);
