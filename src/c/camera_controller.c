#include "camera_controller.h"

static const Vec3 EYE_WAYPOINTS[] = {
  { 1, 1, 1 },
  { 1, -1, 1 },
  { -1, -1, 1 },
  { -1, 1, 1 }
};

static void invalidate(CameraController *controller)
{
  if (controller->invalidate_handler != NULL)
  {
    controller->invalidate_handler(controller->invalidate_context);
  }
}

static void anim_stopped(struct Animation* animation, bool finished, void *context);

static void create_animation(CameraController *controller)
{
  controller->anim = animation_create();
  animation_set_delay(controller->anim, controller->slow_mode ? 1000 : 500);
  animation_set_duration(controller->anim, controller->slow_mode ? 3000 : 500);
  animation_set_implementation(controller->anim, &controller->anim_impl);
  animation_set_handlers(controller->anim, (AnimationHandlers) {
    .stopped = anim_stopped,
  }, controller);
}

static void anim_update(struct Animation* animation, const AnimationProgress time_normalized)
{
  CameraController *controller = animation_get_context(animation);
  float ratio = (float)time_normalized / ANIMATION_NORMALIZED_MAX;

  controller->eye.x = controller->eye_from.x * (1 - ratio) + EYE_WAYPOINTS[controller->eye_to_idx].x * ratio;
  controller->eye.y = controller->eye_from.y * (1 - ratio) + EYE_WAYPOINTS[controller->eye_to_idx].y * ratio;
  mat4_look_at_rh(&controller->view_matrix, &controller->eye, &controller->at, &controller->up);
  invalidate(controller);
}

static void anim_stopped(struct Animation* animation, bool finished, void *context)
{
  CameraController *controller = context;

  if (!finished)
  {
    return;
  }

  controller->eye = EYE_WAYPOINTS[controller->eye_to_idx];
  mat4_look_at_rh(&controller->view_matrix, &controller->eye, &controller->at, &controller->up);
  invalidate(controller);

  animation_destroy(animation);
  if (controller->anim == animation)
  {
    controller->anim = NULL;
  }
}

void camera_controller_init(CameraController *controller, bool slow_mode,
  CameraInvalidateHandler invalidate_handler, void *invalidate_context)
{
  controller->invalidate_handler = invalidate_handler;
  controller->invalidate_context = invalidate_context;
  controller->slow_mode = slow_mode;
  controller->eye = EYE_WAYPOINTS[0];
  controller->at = Vec3(0, 0, 0);
  controller->up = Vec3(0, 1, 0);
  controller->eye_from = controller->eye;
  controller->eye_to_idx = 0;
  controller->anim = NULL;
  controller->anim_impl.setup = NULL;
  controller->anim_impl.update = anim_update;
  controller->anim_impl.teardown = NULL;
  mat4_look_at_rh(&controller->view_matrix, &controller->eye, &controller->at, &controller->up);
}

void camera_controller_deinit(CameraController *controller)
{
  if (controller->anim != NULL)
  {
    animation_unschedule(controller->anim);
    animation_destroy(controller->anim);
    controller->anim = NULL;
  }
}

void camera_controller_set_slow_mode(CameraController *controller, bool slow_mode)
{
  controller->slow_mode = slow_mode;
}

void camera_controller_start_transition(CameraController *controller)
{
  if (controller->anim != NULL && animation_is_scheduled(controller->anim))
  {
    animation_unschedule(controller->anim);
  }

  if (controller->anim == NULL)
  {
    create_animation(controller);
  }

  controller->eye_from = controller->eye;
  controller->eye_to_idx = (controller->eye_to_idx + 1) % ARRAY_LENGTH(EYE_WAYPOINTS);
  animation_schedule(controller->anim);
}

const Mat4 *camera_controller_get_view_matrix(const CameraController *controller)
{
  return &controller->view_matrix;
}
