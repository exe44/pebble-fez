#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"

#include "math_helper.h"
#include "poly_data.h"

#define MY_UUID { 0xCE, 0x86, 0x3A, 0xBC, 0xD9, 0x90, 0x41, 0x2B, 0xB1, 0x77, 0x63, 0x2B, 0xB7, 0x95, 0x1C, 0x12 }
PBL_APP_INFO(MY_UUID,
             "FEZ", "exe",
             1, 0, /* App version */
             DEFAULT_MENU_ICON,
             APP_INFO_WATCH_FACE);

Window window;
Mat4 view_matrix;

#define POLY_SCALE 1.4f

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

typedef struct PolyLayer
{
  Layer layer;
  Poly* poly_ref;
  Vec3 pos;
} PolyLayer;

void poly_layer_update_proc(PolyLayer *poly_layer, GContext* ctx)
{
  Poly* poly = poly_layer->poly_ref;

  if (NULL == poly)
    return;

  static GPoint screen_poss[32];

  // get current layer pos (frame center) in world coordinate

  Vec3 center_view_pos;
  mat4_multiply_vec3(&center_view_pos, &view_matrix, &poly_layer->pos);
  GPoint center_screen_pos;
  GetScreenCoordPos(&center_screen_pos, &center_view_pos);

  // update frame

  GRect frame = poly_layer->layer.frame;
  frame.origin.x = center_screen_pos.x - frame.size.w / 2;
  frame.origin.y = center_screen_pos.y - frame.size.h / 2;
  layer_set_frame(&poly_layer->layer, frame);

  // get poly's vertex pos in frame coordinate

  Vec3 model_pos, scale_pos, world_pos, view_pos;
  for (int i = 0; i < poly->vertex_num; ++i)
  {
    vec3_minus(&model_pos, &poly->vertexs[i], &poly->center);
    vec3_multiply(&scale_pos, &poly->vertexs[i], POLY_SCALE);
    vec3_minus(&model_pos, &scale_pos, &poly->center);
    vec3_plus(&world_pos, &poly_layer->pos, &model_pos);
    mat4_multiply_vec3(&view_pos, &view_matrix, &world_pos);

    GetScreenCoordPos(&screen_poss[i], &view_pos);
    screen_poss[i].x = screen_poss[i].x - center_screen_pos.x + poly_layer->layer.frame.size.w / 2;
    screen_poss[i].y = screen_poss[i].y - center_screen_pos.y + poly_layer->layer.frame.size.h / 2;
  }

  // draw line according to vertex idx

  int prev_vertex_idx = -1;
  int vertex_idx = -1;

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

void poly_layer_init(PolyLayer *poly_layer, GSize size, Vec3 pos)
{
  // pos is frame center

  GPoint screen_pos;
  GetScreenCoordPos(&screen_pos, &pos);
  layer_init(&poly_layer->layer, GRect(screen_pos.x - size.w / 2, screen_pos.y - size.h / 2, size.w, size.h));

  poly_layer->layer.update_proc = (LayerUpdateProc)poly_layer_update_proc;

  poly_layer->poly_ref = NULL;
  poly_layer->pos = pos;
}

void poly_layer_set_poly_ref(PolyLayer *poly_layer, Poly* poly)
{
  poly_layer->poly_ref = poly;
  layer_mark_dirty(&poly_layer->layer); 
}

//==============================================================================

Poly number_polys[10];
PolyLayer digits[4];

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
int eye_num;

//==============================================================================

AnimationImplementation anim_impl;
Animation anim;

void anim_update(struct Animation *animation, const uint32_t time_normalized)
{
  float ratio = (float)time_normalized / ANIMATION_NORMALIZED_MAX;
  eye.x = eye_from.x * (1 - ratio) + eye_waypoints[eye_to_idx].x * ratio;
  eye.y = eye_from.y * (1 - ratio) + eye_waypoints[eye_to_idx].y * ratio;
  MatrixLookAtRH(&view_matrix, &eye, &at, &up);

  for (int i = 0; i < 4; ++i)
  {
    layer_mark_dirty(&digits[i].layer);
  }
}

void anim_teardown(struct Animation *animation)
// void anim_stopped(struct Animation *animation, bool finished, void *context)
{
  eye = eye_waypoints[eye_to_idx];
  MatrixLookAtRH(&view_matrix, &eye, &at, &up);

  for (int i = 0; i < 4; ++i)
  {
    layer_mark_dirty(&digits[i].layer);
  }
}

//==============================================================================

int current_hr = -1;
int current_min = -1;

// Called once per minute
void handle_minute_tick(AppContextRef ctx, PebbleTickEvent *t)
{
  PblTm currentTime;
  get_time(&currentTime);

  if (current_hr != currentTime.tm_hour)
  {
    current_hr = currentTime.tm_hour;

    poly_layer_set_poly_ref(&digits[0], &number_polys[(int)(current_hr / 10)]);
    poly_layer_set_poly_ref(&digits[1], &number_polys[current_hr % 10]);
  }

  if (current_min != currentTime.tm_min)
  {
    current_min = currentTime.tm_min;

    poly_layer_set_poly_ref(&digits[2], &number_polys[(int)(current_min / 10)]);
    poly_layer_set_poly_ref(&digits[3], &number_polys[current_min % 10]);

    // start camera move animation

    if (animation_is_scheduled(&anim))
      animation_unschedule(&anim);

    eye_from = eye;
    eye_to_idx++;
    if (eye_to_idx >= eye_num)
      eye_to_idx = 0;

    animation_schedule(&anim);
  }
}

void handle_init(AppContextRef ctx)
{
  (void)ctx;

  window_init(&window, "Window Name");
  window_stack_push(&window, true /* Animated */);
  window_set_background_color(&window, GColorBlack);

  //

  eye = eye_waypoints[0];
  at = Vec3(0, 0, 0);
  up = Vec3(0, 1, 0);
  MatrixLookAtRH(&view_matrix, &eye, &at, &up);

  eye_to_idx = 0;
  eye_num = sizeof(eye_waypoints) / sizeof(eye_waypoints[0]);

  //

  anim_impl.setup = NULL;
  anim_impl.update = anim_update;
  anim_impl.teardown = anim_teardown;
  animation_init(&anim);
  animation_set_delay(&anim, 500);
  animation_set_duration(&anim, 500);
  animation_set_implementation(&anim, &anim_impl);
  // animation_set_handlers(&anim, (AnimationHandlers){
  //       .stopped = (AnimationStoppedHandler)anim_stopped,
  //     }, NULL);

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

  poly_layer_init(&digits[0], GSize(40 * POLY_SCALE, 50 * POLY_SCALE), Vec3(-40, 45, 0));
  layer_add_child(&window.layer, &digits[0].layer);
  poly_layer_init(&digits[1], GSize(40 * POLY_SCALE, 50 * POLY_SCALE), Vec3(40, 45, 0));
  layer_add_child(&window.layer, &digits[1].layer);
  poly_layer_init(&digits[2], GSize(40 * POLY_SCALE, 50 * POLY_SCALE), Vec3(-40, -45, 0));
  layer_add_child(&window.layer, &digits[2].layer);
  poly_layer_init(&digits[3], GSize(40 * POLY_SCALE, 50 * POLY_SCALE), Vec3(40, -45, 0));
  layer_add_child(&window.layer, &digits[3].layer);

  // Ensures time is displayed immediately (will break if NULL tick event accessed).
  // (This is why it's a good idea to have a separate routine to do the update itself.)
  handle_minute_tick(ctx, NULL);
}

void pbl_main(void *params)
{
  PebbleAppHandlers handlers = {
    .init_handler = &handle_init,

    // Handle time updates
    .tick_info = {
      .tick_handler = &handle_minute_tick,
      .tick_units = MINUTE_UNIT
    }

  };
  app_event_loop(params, &handlers);
}
