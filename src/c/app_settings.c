#include "app_settings.h"

static int32_t sanitize_color_value(int32_t value, int32_t fallback)
{
  if (value == 0 || value == 1)
  {
    return value == 0 ? 0x000000 : 0xFFFFFF;
  }

  if (value < 0)
  {
    return fallback;
  }

  return value & 0xFFFFFF;
}

static GColor mix_with_background(const AppSettings *settings, int32_t color)
{
  uint32_t fg = (uint32_t)color & 0xFFFFFF;
  uint32_t bg = (uint32_t)settings->bg_color & 0xFFFFFF;
  uint8_t r = (uint8_t)((((fg >> 16) & 0xFF) * 3 + ((bg >> 16) & 0xFF)) / 4);
  uint8_t g = (uint8_t)((((fg >> 8) & 0xFF) * 3 + ((bg >> 8) & 0xFF)) / 4);
  uint8_t b = (uint8_t)((((fg >> 0) & 0xFF) * 3 + ((bg >> 0) & 0xFF)) / 4);

  return GColorFromHEX((r << 16) | (g << 8) | b);
}

static void sanitize_settings(AppSettings *settings)
{
  settings->bg_color = sanitize_color_value(settings->bg_color, 0x000000);
  settings->face_color = sanitize_color_value(settings->face_color, 0xFFAA00);
  settings->face_mix_with_background = settings->face_mix_with_background ? true : false;
  settings->line_color = sanitize_color_value(settings->line_color, 0xFFFFFF);
  settings->line_mix_with_background = settings->line_mix_with_background ? true : false;
  settings->back_line_color = sanitize_color_value(settings->back_line_color, settings->line_color);
  settings->side_line_color = sanitize_color_value(settings->side_line_color, settings->line_color);
  settings->split_line_colors = settings->split_line_colors ? true : false;
}

void app_settings_load(AppSettings *settings)
{
  settings->slow_version = persist_exists(PERSIST_KEY_SLOW_VERSION) ? persist_read_bool(PERSIST_KEY_SLOW_VERSION) : false;
  settings->bg_color = persist_exists(PERSIST_KEY_BG_COLOR) ? persist_read_int(PERSIST_KEY_BG_COLOR) : 0x000000;
  settings->face_color = persist_exists(PERSIST_KEY_FACE_COLOR) ? persist_read_int(PERSIST_KEY_FACE_COLOR) : 0xFFAA00;
  settings->face_mix_with_background = persist_exists(PERSIST_KEY_FACE_MIX_WITH_BACKGROUND)
    ? persist_read_bool(PERSIST_KEY_FACE_MIX_WITH_BACKGROUND)
    : false;
  settings->line_color = persist_exists(PERSIST_KEY_LINE_COLOR)
    ? persist_read_int(PERSIST_KEY_LINE_COLOR)
    : 0xFFFFFF;
  settings->line_mix_with_background = persist_exists(PERSIST_KEY_LINE_MIX_WITH_BACKGROUND)
    ? persist_read_bool(PERSIST_KEY_LINE_MIX_WITH_BACKGROUND)
    : false;
  settings->split_line_colors = persist_exists(PERSIST_KEY_SPLIT_LINE_COLORS)
    ? persist_read_bool(PERSIST_KEY_SPLIT_LINE_COLORS)
    : false;
  settings->back_line_color = persist_exists(PERSIST_KEY_BACK_LINE_COLOR)
    ? persist_read_int(PERSIST_KEY_BACK_LINE_COLOR)
    : settings->line_color;
  settings->side_line_color = persist_exists(PERSIST_KEY_SIDE_LINE_COLOR)
    ? persist_read_int(PERSIST_KEY_SIDE_LINE_COLOR)
    : settings->line_color;

  sanitize_settings(settings);
}

void app_settings_save(const AppSettings *settings)
{
  persist_write_bool(PERSIST_KEY_SLOW_VERSION, settings->slow_version);
  persist_write_int(PERSIST_KEY_BG_COLOR, settings->bg_color);
  persist_write_int(PERSIST_KEY_FACE_COLOR, settings->face_color);
  persist_write_bool(PERSIST_KEY_FACE_MIX_WITH_BACKGROUND, settings->face_mix_with_background);
  persist_write_int(PERSIST_KEY_LINE_COLOR, settings->line_color);
  persist_write_bool(PERSIST_KEY_LINE_MIX_WITH_BACKGROUND, settings->line_mix_with_background);
  persist_write_bool(PERSIST_KEY_SPLIT_LINE_COLORS, settings->split_line_colors);
  persist_write_int(PERSIST_KEY_BACK_LINE_COLOR, settings->back_line_color);
  persist_write_int(PERSIST_KEY_SIDE_LINE_COLOR, settings->side_line_color);
}

bool app_settings_apply_message(AppSettings *settings, DictionaryIterator *iterator)
{
  bool changed = false;
  Tuple *slow_tuple = dict_find(iterator, MESSAGE_KEY_SETTING_SLOW_VERSION);
  Tuple *bg_tuple = dict_find(iterator, MESSAGE_KEY_SETTING_BG_COLOR);
  Tuple *face_color_tuple = dict_find(iterator, MESSAGE_KEY_SETTING_FACE_COLOR);
  Tuple *face_mix_with_background_tuple = dict_find(iterator, MESSAGE_KEY_SETTING_FACE_MIX_WITH_BACKGROUND);
  Tuple *line_color_tuple = dict_find(iterator, MESSAGE_KEY_SETTING_LINE_COLOR);
  Tuple *line_mix_with_background_tuple = dict_find(iterator, MESSAGE_KEY_SETTING_LINE_MIX_WITH_BACKGROUND);
  Tuple *split_line_colors_tuple = dict_find(iterator, MESSAGE_KEY_SETTING_SPLIT_LINE_COLORS);
  Tuple *back_line_color_tuple = dict_find(iterator, MESSAGE_KEY_SETTING_BACK_LINE_COLOR);
  Tuple *side_line_color_tuple = dict_find(iterator, MESSAGE_KEY_SETTING_SIDE_LINE_COLOR);

  if (slow_tuple != NULL)
  {
    settings->slow_version = slow_tuple->value->int32 != 0;
    changed = true;
  }

  if (bg_tuple != NULL)
  {
    settings->bg_color = bg_tuple->value->int32;
    changed = true;
  }

  if (face_color_tuple != NULL)
  {
    settings->face_color = face_color_tuple->value->int32;
    changed = true;
  }

  if (face_mix_with_background_tuple != NULL)
  {
    settings->face_mix_with_background = face_mix_with_background_tuple->value->int32 != 0;
    changed = true;
  }

  if (line_color_tuple != NULL)
  {
    settings->line_color = line_color_tuple->value->int32;
    changed = true;
  }

  if (line_mix_with_background_tuple != NULL)
  {
    settings->line_mix_with_background = line_mix_with_background_tuple->value->int32 != 0;
    changed = true;
  }

  if (split_line_colors_tuple != NULL)
  {
    settings->split_line_colors = split_line_colors_tuple->value->int32 != 0;
    changed = true;
  }

  if (back_line_color_tuple != NULL)
  {
    settings->back_line_color = back_line_color_tuple->value->int32;
    changed = true;
  }

  if (side_line_color_tuple != NULL)
  {
    settings->side_line_color = side_line_color_tuple->value->int32;
    changed = true;
  }

  if (changed)
  {
    sanitize_settings(settings);
  }

  return changed;
}

GColor app_settings_get_background_color(const AppSettings *settings)
{
  return GColorFromHEX(settings->bg_color);
}

GColor app_settings_get_line_color(const AppSettings *settings)
{
  return settings->line_mix_with_background
    ? mix_with_background(settings, settings->line_color)
    : GColorFromHEX(settings->line_color);
}

GColor app_settings_get_back_line_color(const AppSettings *settings)
{
  int32_t color = settings->split_line_colors ? settings->back_line_color : settings->line_color;

  return settings->line_mix_with_background
    ? mix_with_background(settings, color)
    : GColorFromHEX(color);
}

GColor app_settings_get_side_line_color(const AppSettings *settings)
{
  int32_t color = settings->split_line_colors ? settings->side_line_color : settings->line_color;

  return settings->line_mix_with_background
    ? mix_with_background(settings, color)
    : GColorFromHEX(color);
}

GColor app_settings_get_face_color(const AppSettings *settings)
{
  return settings->face_mix_with_background
    ? mix_with_background(settings, settings->face_color)
    : GColorFromHEX(settings->face_color);
}
