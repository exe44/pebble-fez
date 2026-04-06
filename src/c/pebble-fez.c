#include <pebble.h>
#include "math_helper.h"
#include "poly_data.h"

#define NUM_DIGITS 4
#define PERSIST_KEY_SLOW_VERSION 1
#define PERSIST_KEY_FG_COLOR 2
#define PERSIST_KEY_BG_COLOR 3

enum
{
  SETTING_COLOR_BLACK = 0,
  SETTING_COLOR_WHITE = 1
};

typedef struct AppSettings
{
  bool slow_version;
  uint8_t fg_color;
  uint8_t bg_color;
} AppSettings;

static Window* window;
static Mat4 view_matrix;
static GPoint screen_center;
static float poly_scale;
static GSize digit_layer_size;
static Vec3 digit_positions[NUM_DIGITS];
static AppSettings settings;

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
  Vec3* vertexs;
  int vertex_num;
  int* idxs;
  int idx_num;
} Poly;

static void poly_init(Poly* poly)
{
  poly->center = Vec3(0, 0, 0);
  poly->vertex_num = 0;
  poly->idx_num = 0;
}

//==============================================================================

typedef struct PolyLayerData
{
  Poly* poly_ref;
  Vec3 pos;
} PolyLayerData;

static void poly_layer_update_proc(Layer *layer, GContext* ctx)
{
  PolyLayerData *data = layer_get_data(layer);
  Poly* poly = data->poly_ref;

  if (NULL == poly)
    return;

  static GPoint screen_poss[32];

  // get current layer pos (frame center) in screen coordinate

  GPoint center_screen_pos;
  WorldToScreenPos(&center_screen_pos, &data->pos);

  // update frame

  GRect frame = layer_get_frame(layer);
  frame.origin.x = center_screen_pos.x - frame.size.w / 2;
  frame.origin.y = center_screen_pos.y - frame.size.h / 2;
  layer_set_frame(layer, frame);

  // get poly's vertex pos in frame coordinate

  Vec3 model_pos, scale_pos, world_pos, view_pos;
  for (int i = 0; i < poly->vertex_num; ++i)
  {
    vec3_multiply(&scale_pos, &poly->vertexs[i], poly_scale);
    vec3_minus(&model_pos, &scale_pos, &poly->center);
    vec3_plus(&world_pos, &data->pos, &model_pos);
    mat4_multiply_vec3(&view_pos, &view_matrix, &world_pos);

    ViewToScreenPos(&screen_poss[i], &view_pos);
    screen_poss[i].x = screen_poss[i].x - center_screen_pos.x + frame.size.w / 2;
    screen_poss[i].y = screen_poss[i].y - center_screen_pos.y + frame.size.h / 2;
  }

  // draw line according to vertex idx

  int prev_vertex_idx = -1;
  int vertex_idx = -1;
  graphics_context_set_stroke_color(ctx, settings.fg_color == SETTING_COLOR_BLACK ? GColorBlack : GColorWhite);

  for (int i = 0; i < poly->idx_num; ++i)
  {
    vertex_idx = poly->idxs[i];

    if (prev_vertex_idx != -1)
    {
      if (vertex_idx == prev_vertex_idx) // same idx, end line
      {
        prev_vertex_idx = -1;
      }
      else
      {
        graphics_draw_line(ctx, screen_poss[prev_vertex_idx], screen_poss[vertex_idx]);

        prev_vertex_idx = vertex_idx;
      }
    }
    else
    {
      prev_vertex_idx = vertex_idx;
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

#define MAKE_NUM_POLY(NUM) \
  poly_init(&number_polys[NUM]); \
  number_polys[NUM].center = Vec3(15 * poly_scale, 20 * poly_scale, 6 * poly_scale); \
  number_polys[NUM].vertexs = vertexs_##NUM; \
  number_polys[NUM].vertex_num = sizeof(vertexs_##NUM) / sizeof(vertexs_##NUM[0]); \
  number_polys[NUM].idxs = idxs_##NUM; \
  number_polys[NUM].idx_num = sizeof(idxs_##NUM) / sizeof(idxs_##NUM[0]);

//==============================================================================
// camera

static Vec3 eye, at, up;

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
extern uint32_t MESSAGE_KEY_SETTING_SLOW_VERSION;
extern uint32_t MESSAGE_KEY_SETTING_FG_COLOR;
extern uint32_t MESSAGE_KEY_SETTING_BG_COLOR;

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

static void sanitize_settings(void)
{
  if (settings.fg_color > SETTING_COLOR_WHITE)
  {
    settings.fg_color = SETTING_COLOR_WHITE;
  }

  if (settings.bg_color > SETTING_COLOR_WHITE)
  {
    settings.bg_color = SETTING_COLOR_BLACK;
  }

  if (settings.fg_color == settings.bg_color)
  {
    settings.bg_color = settings.fg_color == SETTING_COLOR_BLACK ? SETTING_COLOR_WHITE : SETTING_COLOR_BLACK;
  }
}

static GColor get_background_color(void)
{
  return settings.bg_color == SETTING_COLOR_WHITE ? GColorWhite : GColorBlack;
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
  settings.fg_color = persist_exists(PERSIST_KEY_FG_COLOR) ? (uint8_t)persist_read_int(PERSIST_KEY_FG_COLOR) : SETTING_COLOR_WHITE;
  settings.bg_color = persist_exists(PERSIST_KEY_BG_COLOR) ? (uint8_t)persist_read_int(PERSIST_KEY_BG_COLOR) : SETTING_COLOR_BLACK;
  sanitize_settings();
}

static void save_settings(void)
{
  persist_write_bool(PERSIST_KEY_SLOW_VERSION, settings.slow_version);
  persist_write_int(PERSIST_KEY_FG_COLOR, settings.fg_color);
  persist_write_int(PERSIST_KEY_BG_COLOR, settings.bg_color);
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context)
{
  Tuple *slow_tuple = dict_find(iterator, MESSAGE_KEY_SETTING_SLOW_VERSION);
  Tuple *fg_tuple = dict_find(iterator, MESSAGE_KEY_SETTING_FG_COLOR);
  Tuple *bg_tuple = dict_find(iterator, MESSAGE_KEY_SETTING_BG_COLOR);

  if (slow_tuple != NULL)
  {
    settings.slow_version = slow_tuple->value->int32 != 0;
  }

  if (fg_tuple != NULL)
  {
    settings.fg_color = (uint8_t)fg_tuple->value->uint8;
  }

  if (bg_tuple != NULL)
  {
    settings.bg_color = (uint8_t)bg_tuple->value->uint8;
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

  MAKE_NUM_POLY(0)
  MAKE_NUM_POLY(1)
  MAKE_NUM_POLY(2)
  MAKE_NUM_POLY(3)
  MAKE_NUM_POLY(4)
  MAKE_NUM_POLY(5)
  MAKE_NUM_POLY(6)
  MAKE_NUM_POLY(7)
  MAKE_NUM_POLY(8)
  MAKE_NUM_POLY(9)

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
