#include <pebble.h>
#include "math_helper.h"
#include "poly_data.h"

#define FEZ_SLOW_VERSION 0
#define POLY_SCALE 1.4f
#define NUM_DIGITS 4

Window* window;
Mat4 view_matrix;

//==============================================================================

#define HALF_SCREEN_WIDTH 72
#define HALF_SCREEN_HEIGHT 84

void GetScreenCoordPos(GPoint* out_p, Vec3* v)
{
  out_p->x = v->x + HALF_SCREEN_WIDTH;
  out_p->y = HALF_SCREEN_HEIGHT - v->y;
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

void poly_init(Poly* poly)
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
typedef Layer PolyLayer;

void poly_layer_update_proc(PolyLayer *poly_layer, GContext* ctx)
{
  PolyLayerData *data = layer_get_data(poly_layer);
  Poly* poly = data->poly_ref;

  if (NULL == poly)
    return;

  static GPoint screen_poss[32];

  // get current layer pos (frame center) in screen coordinate

  Vec3 center_view_pos;
  mat4_multiply_vec3(&center_view_pos, &view_matrix, &data->pos);
  GPoint center_screen_pos;
  GetScreenCoordPos(&center_screen_pos, &center_view_pos);

  // update frame

  GRect frame = layer_get_frame(poly_layer);
  frame.origin.x = center_screen_pos.x - frame.size.w / 2;
  frame.origin.y = center_screen_pos.y - frame.size.h / 2;
  layer_set_frame(poly_layer, frame);

  // get poly's vertex pos in frame coordinate

  Vec3 model_pos, scale_pos, world_pos, view_pos;
  for (int i = 0; i < poly->vertex_num; ++i)
  {
    vec3_multiply(&scale_pos, &poly->vertexs[i], POLY_SCALE);
    vec3_minus(&model_pos, &scale_pos, &poly->center);
    vec3_plus(&world_pos, &data->pos, &model_pos);
    mat4_multiply_vec3(&view_pos, &view_matrix, &world_pos);

    GetScreenCoordPos(&screen_poss[i], &view_pos);
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

PolyLayer *poly_layer_create(GSize size, Vec3 pos)
{
  
  PolyLayer *layer;
  PolyLayerData *data;
  
  // pos is frame center
  GPoint screen_pos;
  GetScreenCoordPos(&screen_pos, &pos);
  layer = layer_create_with_data(GRect(screen_pos.x - size.w / 2, screen_pos.y - size.h / 2, size.w, size.h), sizeof(PolyLayerData));
  data = layer_get_data(layer);
  data->poly_ref = NULL;
  data->pos = pos;

  layer_set_update_proc(layer, poly_layer_update_proc);
  
  return layer;
}

void poly_layer_set_poly_ref(PolyLayer *poly_layer, Poly* poly)
{
  
  ((PolyLayerData*)layer_get_data(poly_layer))->poly_ref = poly;
  layer_mark_dirty(poly_layer);
}

//==============================================================================

Poly number_polys[10];
PolyLayer *digits[NUM_DIGITS];

#define MAKE_NUM_POLY(NUM) \
  poly_init(&number_polys[NUM]); \
  number_polys[NUM].center = Vec3(15 * POLY_SCALE, 20 * POLY_SCALE, 6 * POLY_SCALE); \
  number_polys[NUM].vertexs = vertexs_##NUM; \
  number_polys[NUM].vertex_num = sizeof(vertexs_##NUM) / sizeof(vertexs_##NUM[0]); \
  number_polys[NUM].idxs = idxs_##NUM; \
  number_polys[NUM].idx_num = sizeof(idxs_##NUM) / sizeof(idxs_##NUM[0]);

//==============================================================================
// camera

Vec3 eye, at, up;

Vec3 eye_waypoints[] = {
  { 1, 1, 1 },
  { 1, -1, 1 },
  { -1, -1, 1 },
  { -1, 1, 1 }
};

Vec3 eye_from;
int eye_to_idx;

//==============================================================================

AnimationImplementation anim_impl;
Animation *anim;

void anim_update(struct Animation *animation, const uint32_t time_normalized)
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

// void anim_teardown(struct Animation *animation)
void anim_stopped(struct Animation *animation, bool finished, void *context)
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

int current_hr = -1;
int current_min = -1;

int calculate_12_format(int hr)
{
  if (hr == 0) hr += 12;
  if (hr > 12) hr -= 12;
  return hr;
}

// Called once per minute
void handle_minute_tick(struct tm *time, TimeUnits units_changed)
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

void handle_init()
{
  window = window_create();
  window_stack_push(window, true);
  window_set_background_color(window, GColorBlack);

  //

  tick_timer_service_subscribe(MINUTE_UNIT, handle_minute_tick);

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

  // Ensures time is displayed immediately (will break if NULL tick event accessed).
  // (This is why it's a good idea to have a separate routine to do the update itself.)

  eye = eye_waypoints[0];
  at = Vec3(0, 0, 0);
  up = Vec3(0, 1, 0);
  MatrixLookAtRH(&view_matrix, &eye, &at, &up);
  eye_to_idx = 0;

  time_t timestamp = time(NULL);
  struct tm *time = localtime(&timestamp);
  handle_minute_tick(time, MINUTE_UNIT);
}

void handle_deinit(void) {
  animation_destroy(anim);
  for (int i = 0; i < NUM_DIGITS; i++) {
    layer_destroy(digits[i]);
  }
  for (int i = 0; i < 10; i++) {

  }
  window_destroy(window);
}

int main(void) {
  handle_init();
  app_event_loop();
  handle_deinit();
}
