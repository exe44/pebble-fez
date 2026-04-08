#include "app_settings.h"

static ColorMode sanitize_color_mode(int32_t value, ColorMode fallback)
{
  if (value < COLOR_MODE_FOREGROUND || value > COLOR_MODE_MIXED)
  {
    return fallback;
  }

  return (ColorMode)value;
}

static void sanitize_settings(AppSettings *settings)
{
  if (settings->fg_color == 0 || settings->fg_color == 1)
  {
    settings->fg_color = settings->fg_color == 0 ? 0x000000 : 0xFFFFFF;
  }

  if (settings->bg_color == 0 || settings->bg_color == 1)
  {
    settings->bg_color = settings->bg_color == 0 ? 0x000000 : 0xFFFFFF;
  }

  if (settings->accent_color == 0 || settings->accent_color == 1)
  {
    settings->accent_color = settings->accent_color == 0 ? 0x000000 : 0xFFFFFF;
  }

  settings->fg_color &= 0xFFFFFF;
  settings->bg_color &= 0xFFFFFF;
  settings->accent_color &= 0xFFFFFF;

  settings->line_color_mode = sanitize_color_mode(settings->line_color_mode, COLOR_MODE_FOREGROUND);
  settings->face_color_mode = sanitize_color_mode(settings->face_color_mode, COLOR_MODE_MIXED);
}

static GColor get_mixed_color(const AppSettings *settings)
{
  uint32_t fg = settings->fg_color;
  uint32_t bg = settings->bg_color;

  uint8_t r = (((fg >> 16) & 0xFF) * 3 + ((bg >> 16) & 0xFF)) / 4;
  uint8_t g = (((fg >> 8) & 0xFF) * 3 + ((bg >> 8) & 0xFF)) / 4;
  uint8_t b = (((fg >> 0) & 0xFF) * 3 + ((bg >> 0) & 0xFF)) / 4;

  return GColorFromHEX((r << 16) | (g << 8) | b);
}

static GColor get_color_for_mode(const AppSettings *settings, ColorMode mode)
{
  switch (mode)
  {
    case COLOR_MODE_BACKGROUND:
      return app_settings_get_background_color(settings);
    case COLOR_MODE_ACCENT:
      return app_settings_get_accent_color(settings);
    case COLOR_MODE_MIXED:
      return get_mixed_color(settings);
    case COLOR_MODE_FOREGROUND:
    default:
      return app_settings_get_foreground_color(settings);
  }
}

void app_settings_load(AppSettings *settings)
{
  settings->slow_version = persist_exists(PERSIST_KEY_SLOW_VERSION) ? persist_read_bool(PERSIST_KEY_SLOW_VERSION) : false;
  settings->fg_color = persist_exists(PERSIST_KEY_FG_COLOR) ? persist_read_int(PERSIST_KEY_FG_COLOR) : 0xFFFFFF;
  settings->bg_color = persist_exists(PERSIST_KEY_BG_COLOR) ? persist_read_int(PERSIST_KEY_BG_COLOR) : 0x000000;
  settings->accent_color = persist_exists(PERSIST_KEY_ACCENT_COLOR) ? persist_read_int(PERSIST_KEY_ACCENT_COLOR) : 0xFFAA00;
  settings->line_color_mode = persist_exists(PERSIST_KEY_LINE_COLOR_MODE)
    ? (ColorMode)persist_read_int(PERSIST_KEY_LINE_COLOR_MODE)
    : COLOR_MODE_FOREGROUND;
  settings->face_color_mode = persist_exists(PERSIST_KEY_FACE_COLOR_MODE)
    ? (ColorMode)persist_read_int(PERSIST_KEY_FACE_COLOR_MODE)
    : COLOR_MODE_ACCENT;
  sanitize_settings(settings);
}

void app_settings_save(const AppSettings *settings)
{
  persist_write_bool(PERSIST_KEY_SLOW_VERSION, settings->slow_version);
  persist_write_int(PERSIST_KEY_FG_COLOR, settings->fg_color);
  persist_write_int(PERSIST_KEY_BG_COLOR, settings->bg_color);
  persist_write_int(PERSIST_KEY_ACCENT_COLOR, settings->accent_color);
  persist_write_int(PERSIST_KEY_LINE_COLOR_MODE, (int32_t)settings->line_color_mode);
  persist_write_int(PERSIST_KEY_FACE_COLOR_MODE, (int32_t)settings->face_color_mode);
}

bool app_settings_apply_message(AppSettings *settings, DictionaryIterator *iterator)
{
  bool changed = false;
  Tuple *slow_tuple = dict_find(iterator, MESSAGE_KEY_SETTING_SLOW_VERSION);
  Tuple *fg_tuple = dict_find(iterator, MESSAGE_KEY_SETTING_FG_COLOR);
  Tuple *bg_tuple = dict_find(iterator, MESSAGE_KEY_SETTING_BG_COLOR);
  Tuple *accent_tuple = dict_find(iterator, MESSAGE_KEY_SETTING_ACCENT_COLOR);
  Tuple *line_color_mode_tuple = dict_find(iterator, MESSAGE_KEY_SETTING_LINE_COLOR_MODE);
  Tuple *face_color_mode_tuple = dict_find(iterator, MESSAGE_KEY_SETTING_FACE_COLOR_MODE);

  if (slow_tuple != NULL)
  {
    settings->slow_version = slow_tuple->value->int32 != 0;
    changed = true;
  }

  if (fg_tuple != NULL)
  {
    settings->fg_color = fg_tuple->value->int32;
    changed = true;
  }

  if (bg_tuple != NULL)
  {
    settings->bg_color = bg_tuple->value->int32;
    changed = true;
  }

  if (accent_tuple != NULL)
  {
    settings->accent_color = accent_tuple->value->int32;
    changed = true;
  }

  if (line_color_mode_tuple != NULL)
  {
    settings->line_color_mode = (ColorMode)line_color_mode_tuple->value->int32;
    changed = true;
  }

  if (face_color_mode_tuple != NULL)
  {
    settings->face_color_mode = (ColorMode)face_color_mode_tuple->value->int32;
    changed = true;
  }

  if (changed)
  {
    sanitize_settings(settings);
  }

  return changed;
}

GColor app_settings_get_foreground_color(const AppSettings *settings)
{
  return GColorFromHEX(settings->fg_color);
}

GColor app_settings_get_background_color(const AppSettings *settings)
{
  return GColorFromHEX(settings->bg_color);
}

GColor app_settings_get_accent_color(const AppSettings *settings)
{
  return GColorFromHEX(settings->accent_color);
}

GColor app_settings_get_line_color(const AppSettings *settings)
{
  return get_color_for_mode(settings, settings->line_color_mode);
}

GColor app_settings_get_face_color(const AppSettings *settings)
{
  return get_color_for_mode(settings, settings->face_color_mode);
}
