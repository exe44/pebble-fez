#include <pebble.h>
#include "math_helper.h"
#include "poly_data.h"

#define NUM_DIGITS 4
#define PERSIST_KEY_SLOW_VERSION 1
#define PERSIST_KEY_FG_COLOR 2
#define PERSIST_KEY_BG_COLOR 3
#define PERSIST_KEY_LINE_COLOR_MODE 4
#define PERSIST_KEY_FACE_COLOR_MODE 5
#define PERSIST_KEY_ACCENT_COLOR 6
#define MESSAGE_KEY_SETTING_SLOW_VERSION 0
#define MESSAGE_KEY_SETTING_FG_COLOR 1
#define MESSAGE_KEY_SETTING_BG_COLOR 2
#define MESSAGE_KEY_SETTING_LINE_COLOR_MODE 3
#define MESSAGE_KEY_SETTING_FACE_COLOR_MODE 4
#define MESSAGE_KEY_SETTING_ACCENT_COLOR 5
#define DIGIT_SHARED_POINT_COUNT ((int)ARRAY_LENGTH(digit_poly_points))

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
  int32_t line_color_mode;
  int32_t face_color_mode;
} AppSettings;

static Window* window;
static Mat4 view_matrix;
static GPoint screen_center;
static float poly_scale;
static GSize digit_layer_size;
static Vec3 digit_positions[NUM_DIGITS];
static AppSettings settings;
static Vec3 eye, at, up;

static GColor get_foreground_color(void);
static GColor get_background_color(void);
static GColor get_accent_color(void);
static GColor get_mixed_color(void);
static GColor get_color_for_mode(ColorMode mode);
static GColor get_line_color(void);
static GColor get_face_color(void);

//==============================================================================

static int round_to_int(float value)
{
  return (int)(value + (value >= 0 ? 0.5f : -0.5f));
}

static void configure_layout(GRect bounds)
{
  const float width_scale = (float)bounds.size.w / 144.0f;
  const float height_scale = (float)bounds.size.h / 168.0f;
  float layout_scale = width_scale < height_scale ? width_scale : height_scale;
#ifdef PBL_ROUND
  layout_scale *= 0.8f;
#endif

  poly_scale = 1.4f * layout_scale;
  screen_center = grect_center_point(&bounds);

  digit_layer_size = GSize(round_to_int(40.0f * poly_scale), round_to_int(50.0f * poly_scale));

  const float digit_offset_x = 40.0f * layout_scale;
  const float digit_offset_y = 45.0f * layout_scale;
  digit_positions[0] = Vec3(-digit_offset_x, digit_offset_y, 0);
  digit_positions[1] = Vec3(digit_offset_x, digit_offset_y, 0);
  digit_positions[2] = Vec3(-digit_offset_x, -digit_offset_y, 0);
  digit_positions[3] = Vec3(digit_offset_x, -digit_offset_y, 0);
}

static void ViewToScreenPos(GPoint* out_screen_pos, Vec3* view_pos)
{
  out_screen_pos->x = screen_center.x + round_to_int(view_pos->x);
  out_screen_pos->y = screen_center.y - round_to_int(view_pos->y);
}

static void WorldToScreenPos(GPoint* out_screen_pos, Vec3* world_pos)
{
  Vec3 view_pos;
  mat4_multiply_vec3(&view_pos, &view_matrix, world_pos);
  ViewToScreenPos(out_screen_pos, &view_pos);
}

//==============================================================================

typedef struct Poly
{
  Vec3 center;
  const DigitPolyData *poly_data;
} Poly;

static void poly_init(Poly* poly)
{
  poly->center = Vec3(0, 0, 0);
  poly->poly_data = NULL;
}

//==============================================================================

typedef struct PolyLayerData
{
  Poly* poly_ref;
  Vec3 pos;
} PolyLayerData;

typedef struct ContourInfo
{
  uint8_t start;
  uint8_t length;
} ContourInfo;

static GColor get_mixed_color(void)
{
  uint32_t fg = settings.fg_color;
  uint32_t bg = settings.bg_color;

  uint8_t r = (((fg >> 16) & 0xFF) * 3 + ((bg >> 16) & 0xFF)) / 4;
  uint8_t g = (((fg >> 8) & 0xFF) * 3 + ((bg >> 8) & 0xFF)) / 4;
  uint8_t b = (((fg >> 0) & 0xFF) * 3 + ((bg >> 0) & 0xFF)) / 4;

  return GColorFromHEX((r << 16) | (g << 8) | b);
}

static GColor get_accent_color(void)
{
  return GColorFromHEX(settings.accent_color);
}

static GColor get_color_for_mode(ColorMode mode)
{
  switch (mode)
  {
    case COLOR_MODE_BACKGROUND:
      return get_background_color();
    case COLOR_MODE_ACCENT:
      return get_accent_color();
    case COLOR_MODE_MIXED:
      return get_mixed_color();
    case COLOR_MODE_FOREGROUND:
    default:
      return get_foreground_color();
  }
}

static GColor get_line_color(void)
{
  return get_color_for_mode((ColorMode)settings.line_color_mode);
}

static GColor get_face_color(void)
{
  return get_color_for_mode((ColorMode)settings.face_color_mode);
}

static void project_model_point(GPoint *out_screen_pos, Vec3 *out_world_pos,
  const Poly *poly, const PolyLayerData *data, float x, float y, float z,
  GPoint center_screen_pos, GSize frame_size)
{
  Vec3 local_pos = Vec3(x, y, z);
  Vec3 model_pos, scale_pos, world_pos, view_pos;

  vec3_multiply(&scale_pos, &local_pos, poly_scale);
  vec3_minus(&model_pos, &scale_pos, &poly->center);
  vec3_plus(&world_pos, &data->pos, &model_pos);
  mat4_multiply_vec3(&view_pos, &view_matrix, &world_pos);

  ViewToScreenPos(out_screen_pos, &view_pos);
  out_screen_pos->x = out_screen_pos->x - center_screen_pos.x + frame_size.w / 2;
  out_screen_pos->y = out_screen_pos->y - center_screen_pos.y + frame_size.h / 2;
  *out_world_pos = world_pos;
}

static void draw_filled_path(GContext *ctx, GPoint *points, int point_num, GColor color)
{
  graphics_context_set_fill_color(ctx, color);

  GPathInfo path_info = {
    .num_points = point_num,
    .points = points,
  };
  GPath path = {
    .num_points = path_info.num_points,
    .points = path_info.points,
    .rotation = 0,
    .offset = GPointZero,
  };

  gpath_draw_filled(ctx, &path);
}

static void draw_solid_poly(GContext *ctx, const Poly *poly,
  const PolyLayerData *data, GPoint center_screen_pos, GSize frame_size,
  const PolyPath *solid_poly, float z, GColor color)
{
  GPoint points[16];
  Vec3 world_pos;

  for (int i = 0; i < solid_poly->point_count; ++i)
  {
    GPoint point = digit_poly_points[solid_poly->point_idxs[i]];
    project_model_point(&points[i], &world_pos, poly, data, point.x, point.y, z,
      center_screen_pos, frame_size);
  }

  draw_filled_path(ctx, points, solid_poly->point_count, color);
}

static void draw_side_face(GContext *ctx, GPoint *screen_poss,
  int front_a, int front_b, int back_offset, GColor color)
{
  GPoint points[4];
  int back_a = front_a + back_offset;
  int back_b = front_b + back_offset;

  points[0] = screen_poss[front_a];
  points[1] = screen_poss[front_b];
  points[2] = screen_poss[back_b];
  points[3] = screen_poss[back_a];

  draw_filled_path(ctx, points, 4, color);
}

static int parse_front_contours(const DigitPolyData *poly_data, ContourInfo *contours)
{
  for (int i = 0; i < poly_data->contour_count; ++i)
  {
    contours[i].start = i;
    contours[i].length = poly_data->contours[i].point_count;
  }

  return poly_data->contour_count;
}

static void draw_poly_fill(GContext *ctx, Poly *poly, const PolyLayerData *data,
  GPoint center_screen_pos, GSize frame_size, GPoint *screen_poss, Vec3 *world_poss)
{
  ContourInfo contours[4];
  const DigitPolyData *poly_data = poly->poly_data;
  int contour_num = parse_front_contours(poly_data, contours);
  int back_offset = DIGIT_SHARED_POINT_COUNT;
  GColor fill_color = get_face_color();

  for (int i = 0; i < poly_data->solid_poly_count; ++i)
  {
    const PolyPath *solid_poly = &poly_data->solid_polys[i];

    draw_solid_poly(ctx, poly, data, center_screen_pos, frame_size,
      solid_poly, 0.0f, fill_color);
    draw_solid_poly(ctx, poly, data, center_screen_pos, frame_size,
      solid_poly, 10.0f, fill_color);
  }

  for (int i = 0; i < contour_num; ++i)
  {
    for (int j = 0; j < contours[i].length; ++j)
    {
      const PolyPath *contour = &poly_data->contours[contours[i].start];
      int front_a = contour->point_idxs[j];
      int front_b = contour->point_idxs[(j + 1) % contours[i].length];
      draw_side_face(ctx, screen_poss, front_a, front_b, back_offset, fill_color);
    }
  }
}

static void poly_layer_update_proc(Layer *layer, GContext* ctx)
{
  PolyLayerData *data = layer_get_data(layer);
  Poly* poly = data->poly_ref;

  if (NULL == poly)
    return;

  static GPoint screen_poss[DIGIT_SHARED_POINT_COUNT * 2];
  static Vec3 world_poss[DIGIT_SHARED_POINT_COUNT * 2];

  // get current layer pos (frame center) in screen coordinate

  GPoint center_screen_pos;
  WorldToScreenPos(&center_screen_pos, &data->pos);

  // update frame

  GRect frame = layer_get_frame(layer);
  frame.origin.x = center_screen_pos.x - frame.size.w / 2;
  frame.origin.y = center_screen_pos.y - frame.size.h / 2;
  layer_set_frame(layer, frame);

  // project shared front/back points for fill, wireframe, and side faces
  for (int i = 0; i < DIGIT_SHARED_POINT_COUNT; ++i)
  {
    GPoint point = digit_poly_points[i];
    project_model_point(&screen_poss[i], &world_poss[i], poly, data, point.x, point.y, 0.0f,
      center_screen_pos, frame.size);
    project_model_point(&screen_poss[i + DIGIT_SHARED_POINT_COUNT],
      &world_poss[i + DIGIT_SHARED_POINT_COUNT], poly, data, point.x, point.y, 10.0f,
      center_screen_pos, frame.size);
  }

  draw_poly_fill(ctx, poly, data, center_screen_pos, frame.size, screen_poss, world_poss);

  // draw wireframe from front/back contours plus connecting edges
  graphics_context_set_stroke_color(ctx, get_line_color());
  for (int i = 0; i < poly->poly_data->contour_count; ++i)
  {
    const PolyPath *contour = &poly->poly_data->contours[i];
    for (int j = 0; j < contour->point_count; ++j)
    {
      int front_a = contour->point_idxs[j];
      int front_b = contour->point_idxs[(j + 1) % contour->point_count];
      int back_a = front_a + DIGIT_SHARED_POINT_COUNT;
      int back_b = front_b + DIGIT_SHARED_POINT_COUNT;

      graphics_draw_line(ctx, screen_poss[front_a], screen_poss[front_b]);
      graphics_draw_line(ctx, screen_poss[back_a], screen_poss[back_b]);
      graphics_draw_line(ctx, screen_poss[front_a], screen_poss[back_a]);
    }
  }
}

static Layer* poly_layer_create(GSize size, Vec3 pos)
{
  Layer *layer;
  PolyLayerData *data;

  // pos is frame center
  GPoint screen_pos;
  WorldToScreenPos(&screen_pos, &pos);

  layer = layer_create_with_data(GRect(screen_pos.x - size.w / 2, screen_pos.y - size.h / 2, size.w, size.h), sizeof(PolyLayerData));
  data = layer_get_data(layer);
  data->poly_ref = NULL;
  data->pos = pos;

  layer_set_update_proc(layer, poly_layer_update_proc);

  return layer;
}

static void poly_layer_set_poly_ref(Layer *layer, Poly* poly)
{
  ((PolyLayerData*)layer_get_data(layer))->poly_ref = poly;
  layer_mark_dirty(layer);
}

//==============================================================================

static Poly number_polys[10];
static Layer* digits[NUM_DIGITS];
#define NUM_NUMBER_POLYS ((int)ARRAY_LENGTH(number_polys))

static void init_number_poly(Poly *poly, int number)
{
  poly_init(poly);
  poly->center = Vec3(15 * poly_scale, 20 * poly_scale, 6 * poly_scale);
  poly->poly_data = &digit_poly_data[number];
}

//==============================================================================
// camera

static Vec3 eye_waypoints[] = {
  { 1, 1, 1 },
  { 1, -1, 1 },
  { -1, -1, 1 },
  { -1, 1, 1 }
};

static Vec3 eye_from;
static int eye_to_idx;

//==============================================================================

static AnimationImplementation anim_impl;
static Animation* anim;

static void anim_stopped(struct Animation* animation, bool finished, void *context);

static void create_animation(void)
{
  anim = animation_create();
  animation_set_delay(anim, (settings.slow_version ? 1000 : 500));
  animation_set_duration(anim, (settings.slow_version ? 3000 : 500));
  animation_set_implementation(anim, &anim_impl);
  animation_set_handlers(anim, (AnimationHandlers) {
    .stopped = (AnimationStoppedHandler)anim_stopped,
  }, NULL);
}

static void anim_update(struct Animation* animation, const AnimationProgress time_normalized)
{
  float ratio = (float)time_normalized / ANIMATION_NORMALIZED_MAX;
  eye.x = eye_from.x * (1 - ratio) + eye_waypoints[eye_to_idx].x * ratio;
  eye.y = eye_from.y * (1 - ratio) + eye_waypoints[eye_to_idx].y * ratio;
  MatrixLookAtRH(&view_matrix, &eye, &at, &up);

  for (int i = 0; i < NUM_DIGITS; ++i)
  {
    layer_mark_dirty(digits[i]);
  }
}

// void anim_teardown(struct Animation* animation)
static void anim_stopped(struct Animation* animation, bool finished, void *context)
{
  if (!finished)
    return;

  eye = eye_waypoints[eye_to_idx];
  MatrixLookAtRH(&view_matrix, &eye, &at, &up);

  for (int i = 0; i < NUM_DIGITS; ++i)
  {
    layer_mark_dirty(digits[i]);
  }

  animation_destroy(animation);
  if (anim == animation)
  {
    anim = NULL;
  }
}

//==============================================================================

static int current_hr = -1;
static int current_min = -1;

static GColor get_foreground_color(void)
{
  return GColorFromHEX(settings.fg_color);
}

static void sanitize_settings(void)
{
  if (settings.fg_color == 0 || settings.fg_color == 1)
  {
    settings.fg_color = settings.fg_color == 0 ? 0x000000 : 0xFFFFFF;
  }

  if (settings.bg_color == 0 || settings.bg_color == 1)
  {
    settings.bg_color = settings.bg_color == 0 ? 0x000000 : 0xFFFFFF;
  }

  if (settings.accent_color == 0 || settings.accent_color == 1)
  {
    settings.accent_color = settings.accent_color == 0 ? 0x000000 : 0xFFFFFF;
  }

  settings.fg_color &= 0xFFFFFF;
  settings.bg_color &= 0xFFFFFF;
  settings.accent_color &= 0xFFFFFF;

  if (settings.line_color_mode < COLOR_MODE_FOREGROUND || settings.line_color_mode > COLOR_MODE_MIXED)
  {
    settings.line_color_mode = COLOR_MODE_FOREGROUND;
  }

  if (settings.face_color_mode < COLOR_MODE_FOREGROUND || settings.face_color_mode > COLOR_MODE_MIXED)
  {
    settings.face_color_mode = COLOR_MODE_MIXED;
  }
}

static GColor get_background_color(void)
{
  return GColorFromHEX(settings.bg_color);
}

static void apply_visual_settings(void)
{
  if (window == NULL)
  {
    return;
  }

  window_set_background_color(window, get_background_color());

  if (digits[0] == NULL)
  {
    return;
  }

  for (int i = 0; i < NUM_DIGITS; ++i)
  {
    layer_mark_dirty(digits[i]);
  }
}

static void load_settings(void)
{
  settings.slow_version = persist_exists(PERSIST_KEY_SLOW_VERSION) ? persist_read_bool(PERSIST_KEY_SLOW_VERSION) : false;
  settings.fg_color = persist_exists(PERSIST_KEY_FG_COLOR) ? persist_read_int(PERSIST_KEY_FG_COLOR) : 0xFFFFFF;
  settings.bg_color = persist_exists(PERSIST_KEY_BG_COLOR) ? persist_read_int(PERSIST_KEY_BG_COLOR) : 0x000000;
  settings.accent_color = persist_exists(PERSIST_KEY_ACCENT_COLOR) ? persist_read_int(PERSIST_KEY_ACCENT_COLOR) : 0xFFAA00;
  settings.line_color_mode = persist_exists(PERSIST_KEY_LINE_COLOR_MODE) ? persist_read_int(PERSIST_KEY_LINE_COLOR_MODE) : COLOR_MODE_FOREGROUND;
  settings.face_color_mode = persist_exists(PERSIST_KEY_FACE_COLOR_MODE) ? persist_read_int(PERSIST_KEY_FACE_COLOR_MODE) : COLOR_MODE_ACCENT;
  sanitize_settings();
}

static void save_settings(void)
{
  persist_write_bool(PERSIST_KEY_SLOW_VERSION, settings.slow_version);
  persist_write_int(PERSIST_KEY_FG_COLOR, settings.fg_color);
  persist_write_int(PERSIST_KEY_BG_COLOR, settings.bg_color);
  persist_write_int(PERSIST_KEY_ACCENT_COLOR, settings.accent_color);
  persist_write_int(PERSIST_KEY_LINE_COLOR_MODE, settings.line_color_mode);
  persist_write_int(PERSIST_KEY_FACE_COLOR_MODE, settings.face_color_mode);
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context)
{
  Tuple *slow_tuple = dict_find(iterator, MESSAGE_KEY_SETTING_SLOW_VERSION);
  Tuple *fg_tuple = dict_find(iterator, MESSAGE_KEY_SETTING_FG_COLOR);
  Tuple *bg_tuple = dict_find(iterator, MESSAGE_KEY_SETTING_BG_COLOR);
  Tuple *accent_tuple = dict_find(iterator, MESSAGE_KEY_SETTING_ACCENT_COLOR);
  Tuple *line_color_mode_tuple = dict_find(iterator, MESSAGE_KEY_SETTING_LINE_COLOR_MODE);
  Tuple *face_color_mode_tuple = dict_find(iterator, MESSAGE_KEY_SETTING_FACE_COLOR_MODE);

  if (slow_tuple != NULL)
  {
    settings.slow_version = slow_tuple->value->int32 != 0;
  }

  if (fg_tuple != NULL)
  {
    settings.fg_color = fg_tuple->value->int32;
  }

  if (bg_tuple != NULL)
  {
    settings.bg_color = bg_tuple->value->int32;
  }

  if (accent_tuple != NULL)
  {
    settings.accent_color = accent_tuple->value->int32;
  }

  if (line_color_mode_tuple != NULL)
  {
    settings.line_color_mode = line_color_mode_tuple->value->int32;
  }

  if (face_color_mode_tuple != NULL)
  {
    settings.face_color_mode = face_color_mode_tuple->value->int32;
  }

  sanitize_settings();
  save_settings();
  apply_visual_settings();
}

static int calculate_12_format(int hr)
{
  if (hr == 0) hr += 12;
  if (hr > 12) hr -= 12;
  return hr;
}

// Called once per minute
static void handle_minute_tick(struct tm* time, TimeUnits units_changed)
{
  int hr = clock_is_24h_style() ? time->tm_hour : calculate_12_format(time->tm_hour);

  if (current_hr != hr)
  {
    int digit_0 = (int)(hr / 10);
    int digit_1 = hr % 10;

    if (current_hr == -1 || (int)(current_hr / 10) != digit_0)
    {
      if (digit_0 == 0)
      {
        layer_set_hidden(digits[0], true);
      }
      else
      {
        layer_set_hidden(digits[0], false);
        poly_layer_set_poly_ref(digits[0], &number_polys[digit_0]);
      }
    }

    if (current_hr == -1 || (current_hr % 10) != digit_1)
    {
      poly_layer_set_poly_ref(digits[1], &number_polys[digit_1]);
    }

    current_hr = hr;
  }

  if (current_min != time->tm_min)
  {
    int digit_2 = (int)(time->tm_min / 10);
    int digit_3 = time->tm_min % 10;

    if (current_min == -1 || (int)(current_min / 10) != digit_2)
    {
      poly_layer_set_poly_ref(digits[2], &number_polys[digit_2]);
    }

    if (current_min == -1 || (current_min % 10) != digit_3)
    {
      poly_layer_set_poly_ref(digits[3], &number_polys[digit_3]);
    }

    current_min = time->tm_min;

    // start camera move animation

    if (anim != NULL && animation_is_scheduled(anim))
    {
      animation_unschedule(anim);
    }

    if (anim == NULL)
    {
      create_animation();
    }

    eye_from = eye;
    eye_to_idx = (eye_to_idx + 1) % ARRAY_LENGTH(eye_waypoints);
    animation_schedule(anim);
  }
}

//==============================================================================

static void window_load(Window* window)
{
  Layer *root_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(root_layer);
  configure_layout(bounds);

  // view_matrix should be ready before poly layer creation

  eye = eye_waypoints[0];
  at = Vec3(0, 0, 0);
  up = Vec3(0, 1, 0);
  MatrixLookAtRH(&view_matrix, &eye, &at, &up);
  eye_to_idx = 0;

  //

  for (int i = 0; i < NUM_NUMBER_POLYS; ++i)
  {
    init_number_poly(&number_polys[i], i);
  }

  //

  digits[0] = poly_layer_create(digit_layer_size, digit_positions[0]);
  layer_add_child(root_layer, digits[0]);
  digits[1] = poly_layer_create(digit_layer_size, digit_positions[1]);
  layer_add_child(root_layer, digits[1]);
  digits[2] = poly_layer_create(digit_layer_size, digit_positions[2]);
  layer_add_child(root_layer, digits[2]);
  digits[3] = poly_layer_create(digit_layer_size, digit_positions[3]);
  layer_add_child(root_layer, digits[3]);

  //

  anim_impl.setup = NULL;
  anim_impl.update = anim_update;
  // anim_impl.teardown = anim_teardown;
  anim_impl.teardown = NULL;
  anim = NULL;

  // Ensures time is displayed immediately

  time_t timestamp = time(NULL);
  struct tm *time = localtime(&timestamp);
  handle_minute_tick(time, MINUTE_UNIT);
}

static void window_unload(Window *window)
{
  if (anim != NULL)
  {
    animation_unschedule(anim);
    animation_destroy(anim);
    anim = NULL;
  }

  for (int i = 0; i < NUM_DIGITS; i++)
  {
    layer_destroy(digits[i]);
  }
}

//==============================================================================

static void handle_init()
{
  load_settings();

  window = window_create();
  window_set_background_color(window, get_background_color());
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  window_stack_push(window, true);

  app_message_register_inbox_received(inbox_received_callback);
  app_message_open(128, 128);
  tick_timer_service_subscribe(MINUTE_UNIT, handle_minute_tick);
}

static void handle_deinit(void)
{
  window_destroy(window);
}

int main(void)
{
  handle_init();
  app_event_loop();
  handle_deinit();
}
