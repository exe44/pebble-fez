#include <pebble.h>
#include "math_helper.h"
#include "poly_data.h"

#define FEZ_SLOW_VERSION 0
#define POLY_SCALE 1.4f
#define NUM_DIGITS 4

static Window* window;
static Mat4 view_matrix;

//==============================================================================

#define HALF_SCREEN_WIDTH 72
#define HALF_SCREEN_HEIGHT 84

static void ViewToScreenPos(GPoint* out_screen_pos, Vec3* view_pos)
{
  out_screen_pos->x = view_pos->x + HALF_SCREEN_WIDTH;
  out_screen_pos->y = HALF_SCREEN_HEIGHT - view_pos->y;
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
    vec3_multiply(&scale_pos, &poly->vertexs[i], POLY_SCALE);
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
  graphics_context_set_stroke_color(ctx, GColorWhite);

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
  number_polys[NUM].center = Vec3(15 * POLY_SCALE, 20 * POLY_SCALE, 6 * POLY_SCALE); \
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

static void anim_update(struct Animation* animation, const uint32_t time_normalized)
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
}

//==============================================================================

static int current_hr = -1;
static int current_min = -1;

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

    if (animation_is_scheduled(anim))
      animation_unschedule(anim);

    eye_from = eye;
    eye_to_idx = (eye_to_idx + 1) % ARRAY_LENGTH(eye_waypoints);
    animation_schedule(anim);
  }
}

//==============================================================================

static void window_load(Window* window)
{
  // GRect bounds = layer_get_bounds(window_layer);

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

  Layer *root_layer = window_get_root_layer(window);
  digits[0] = poly_layer_create(GSize(40 * POLY_SCALE, 50 * POLY_SCALE), Vec3(-40, 45, 0));
  layer_add_child(root_layer, digits[0]);
  digits[1] = poly_layer_create(GSize(40 * POLY_SCALE, 50 * POLY_SCALE), Vec3(40, 45, 0));
  layer_add_child(root_layer, digits[1]);
  digits[2] = poly_layer_create(GSize(40 * POLY_SCALE, 50 * POLY_SCALE), Vec3(-40, -45, 0));
  layer_add_child(root_layer, digits[2]);
  digits[3] = poly_layer_create(GSize(40 * POLY_SCALE, 50 * POLY_SCALE), Vec3(40, -45, 0));
  layer_add_child(root_layer, digits[3]);

  //

  anim_impl.setup = NULL;
  anim_impl.update = anim_update;
  // anim_impl.teardown = anim_teardown;
  anim_impl.teardown = NULL;
  anim = animation_create();
  animation_set_delay(anim, (FEZ_SLOW_VERSION? 1000 : 500));
  animation_set_duration(anim, (FEZ_SLOW_VERSION? 3000 : 500));
  animation_set_implementation(anim, &anim_impl);

  animation_set_handlers(anim, (AnimationHandlers) {
    .stopped = (AnimationStoppedHandler)anim_stopped,
  }, NULL);
}

static void window_unload(Window *window)
{
  animation_destroy(anim);

  for (int i = 0; i < NUM_DIGITS; i++)
  {
    layer_destroy(digits[i]);
  }
}

//==============================================================================

static void handle_init()
{
  window = window_create();
  window_set_background_color(window, GColorBlack);
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  window_stack_push(window, true);

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
