#include "digit_renderer.h"
#include "poly_data.h"

#define DIGIT_RENDERER_DIGIT_COUNT 4
#define DIGIT_SHARED_POINT_COUNT ((int)ARRAY_LENGTH(digit_poly_points))

typedef struct Poly
{
  Vec3 center;
  const DigitPolyData *poly_data;
} Poly;

struct DigitRendererState
{
  Layer *digits[DIGIT_RENDERER_DIGIT_COUNT];
  Poly number_polys[10];
  GPoint screen_center;
  float poly_scale;
  GSize digit_layer_size;
  Vec3 digit_positions[DIGIT_RENDERER_DIGIT_COUNT];
  const AppSettings *settings;
  const Mat4 *view_matrix;
};

typedef struct PolyLayerData
{
  DigitRenderer *renderer;
  Poly *poly_ref;
  Vec3 pos;
} PolyLayerData;

typedef struct ContourInfo
{
  uint8_t start;
  uint8_t length;
} ContourInfo;

static int round_to_int(float value)
{
  return (int)(value + (value >= 0 ? 0.5f : -0.5f));
}

static void configure_layout(DigitRenderer *renderer, GRect bounds)
{
  DigitRendererState *state = renderer->state;
  const float width_scale = (float)bounds.size.w / 144.0f;
  const float height_scale = (float)bounds.size.h / 168.0f;
  float layout_scale = width_scale < height_scale ? width_scale : height_scale;
#ifdef PBL_ROUND
  layout_scale *= 0.8f;
#endif

  state->poly_scale = 1.4f * layout_scale;
  state->screen_center = grect_center_point(&bounds);
  state->digit_layer_size = GSize(round_to_int(40.0f * state->poly_scale),
    round_to_int(50.0f * state->poly_scale));

  {
    const float digit_offset_x = 40.0f * layout_scale;
    const float digit_offset_y = 45.0f * layout_scale;
    state->digit_positions[0] = Vec3(-digit_offset_x, digit_offset_y, 0);
    state->digit_positions[1] = Vec3(digit_offset_x, digit_offset_y, 0);
    state->digit_positions[2] = Vec3(-digit_offset_x, -digit_offset_y, 0);
    state->digit_positions[3] = Vec3(digit_offset_x, -digit_offset_y, 0);
  }
}

static void view_to_screen_pos(GPoint* out_screen_pos, const DigitRenderer *renderer, const Vec3 *view_pos)
{
  const DigitRendererState *state = renderer->state;

  out_screen_pos->x = state->screen_center.x + round_to_int(view_pos->x);
  out_screen_pos->y = state->screen_center.y - round_to_int(view_pos->y);
}

static void world_to_screen_pos(GPoint* out_screen_pos, const DigitRenderer *renderer, const Vec3 *world_pos)
{
  Vec3 view_pos;
  mat4_multiply_vec3(&view_pos, renderer->state->view_matrix, world_pos);
  view_to_screen_pos(out_screen_pos, renderer, &view_pos);
}

static void poly_init(Poly* poly)
{
  poly->center = Vec3(0, 0, 0);
  poly->poly_data = NULL;
}

static void init_number_poly(DigitRenderer *renderer, Poly *poly, int number)
{
  poly_init(poly);
  poly->center = Vec3(15 * renderer->state->poly_scale, 20 * renderer->state->poly_scale, 6 * renderer->state->poly_scale);
  poly->poly_data = &digit_poly_data[number];
}

static void project_model_point(GPoint *out_screen_pos,
  const DigitRenderer *renderer, const Poly *poly, const PolyLayerData *data, float x, float y, float z,
  GPoint center_screen_pos, GSize frame_size)
{
  Vec3 local_pos = Vec3(x, y, z);
  Vec3 model_pos, scale_pos, world_pos, view_pos;

  vec3_multiply(&scale_pos, &local_pos, renderer->state->poly_scale);
  vec3_minus(&model_pos, &scale_pos, &poly->center);
  vec3_plus(&world_pos, &data->pos, &model_pos);
  mat4_multiply_vec3(&view_pos, renderer->state->view_matrix, &world_pos);

  view_to_screen_pos(out_screen_pos, renderer, &view_pos);
  out_screen_pos->x = out_screen_pos->x - center_screen_pos.x + frame_size.w / 2;
  out_screen_pos->y = out_screen_pos->y - center_screen_pos.y + frame_size.h / 2;
}

static void draw_filled_path(GContext *ctx, GPoint *points, int point_num, GColor color)
{
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

  graphics_context_set_fill_color(ctx, color);
  gpath_draw_filled(ctx, &path);
}

static void draw_solid_poly(GContext *ctx, const DigitRenderer *renderer, const Poly *poly,
  const PolyLayerData *data, GPoint center_screen_pos, GSize frame_size,
  const PolyPath *solid_poly, float z, GColor color)
{
  GPoint points[16];

  for (int i = 0; i < solid_poly->point_count; ++i)
  {
    GPoint point = digit_poly_points[solid_poly->point_idxs[i]];
    project_model_point(&points[i], renderer, poly, data, point.x, point.y, z,
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

static void draw_poly_fill(GContext *ctx, const DigitRenderer *renderer, Poly *poly,
  const PolyLayerData *data, GPoint center_screen_pos, GSize frame_size, GPoint *screen_poss)
{
  ContourInfo contours[4];
  const DigitPolyData *poly_data = poly->poly_data;
  int contour_num = parse_front_contours(poly_data, contours);
  int back_offset = DIGIT_SHARED_POINT_COUNT;
  GColor fill_color = app_settings_get_face_color(renderer->state->settings);

  for (int i = 0; i < poly_data->solid_poly_count; ++i)
  {
    const PolyPath *solid_poly = &poly_data->solid_polys[i];

    draw_solid_poly(ctx, renderer, poly, data, center_screen_pos, frame_size,
      solid_poly, 0.0f, fill_color);
    draw_solid_poly(ctx, renderer, poly, data, center_screen_pos, frame_size,
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
  DigitRenderer *renderer = data->renderer;
  Poly* poly = data->poly_ref;

  if (poly == NULL)
  {
    return;
  }

  static GPoint screen_poss[DIGIT_SHARED_POINT_COUNT * 2];
  GPoint center_screen_pos;
  GRect frame = layer_get_frame(layer);

  world_to_screen_pos(&center_screen_pos, renderer, &data->pos);

  frame.origin.x = center_screen_pos.x - frame.size.w / 2;
  frame.origin.y = center_screen_pos.y - frame.size.h / 2;
  layer_set_frame(layer, frame);

  for (int i = 0; i < DIGIT_SHARED_POINT_COUNT; ++i)
  {
    GPoint point = digit_poly_points[i];
    project_model_point(&screen_poss[i], renderer, poly, data, point.x, point.y, 0.0f,
      center_screen_pos, frame.size);
    project_model_point(&screen_poss[i + DIGIT_SHARED_POINT_COUNT], renderer, poly, data, point.x, point.y, 10.0f,
      center_screen_pos, frame.size);
  }

  draw_poly_fill(ctx, renderer, poly, data, center_screen_pos, frame.size, screen_poss);

  graphics_context_set_stroke_color(ctx, app_settings_get_line_color(renderer->state->settings));
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

static Layer* poly_layer_create(DigitRenderer *renderer, GSize size, Vec3 pos)
{
  Layer *layer;
  PolyLayerData *data;
  GPoint screen_pos;

  world_to_screen_pos(&screen_pos, renderer, &pos);
  layer = layer_create_with_data(GRect(screen_pos.x - size.w / 2, screen_pos.y - size.h / 2, size.w, size.h),
    sizeof(PolyLayerData));
  data = layer_get_data(layer);
  data->renderer = renderer;
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

bool digit_renderer_init(DigitRenderer *renderer, Layer *root_layer,
  const AppSettings *settings, const Mat4 *view_matrix)
{
  renderer->state = NULL;
  renderer->state = malloc(sizeof(DigitRendererState));
  if (renderer->state == NULL)
  {
    return false;
  }

  GRect bounds = layer_get_bounds(root_layer);

  renderer->state->settings = settings;
  renderer->state->view_matrix = view_matrix;
  configure_layout(renderer, bounds);

  for (int i = 0; i < (int)ARRAY_LENGTH(renderer->state->number_polys); ++i)
  {
    init_number_poly(renderer, &renderer->state->number_polys[i], i);
  }

  for (int i = 0; i < DIGIT_RENDERER_DIGIT_COUNT; ++i)
  {
    renderer->state->digits[i] = poly_layer_create(renderer, renderer->state->digit_layer_size, renderer->state->digit_positions[i]);
    layer_add_child(root_layer, renderer->state->digits[i]);
  }

  return true;
}

void digit_renderer_deinit(DigitRenderer *renderer)
{
  if (renderer->state == NULL)
  {
    return;
  }

  for (int i = 0; i < DIGIT_RENDERER_DIGIT_COUNT; ++i)
  {
    layer_destroy(renderer->state->digits[i]);
    renderer->state->digits[i] = NULL;
  }

  free(renderer->state);
  renderer->state = NULL;
}

void digit_renderer_set_digit(DigitRenderer *renderer, int index, int value, bool hidden)
{
  if (renderer->state == NULL)
  {
    return;
  }

  Layer *layer = renderer->state->digits[index];

  layer_set_hidden(layer, hidden);
  if (!hidden)
  {
    poly_layer_set_poly_ref(layer, &renderer->state->number_polys[value]);
  }
}

void digit_renderer_mark_all_dirty(DigitRenderer *renderer)
{
  if (renderer->state == NULL)
  {
    return;
  }

  for (int i = 0; i < DIGIT_RENDERER_DIGIT_COUNT; ++i)
  {
    layer_mark_dirty(renderer->state->digits[i]);
  }
}

bool digit_renderer_is_ready(const DigitRenderer *renderer)
{
  return renderer->state != NULL;
}
