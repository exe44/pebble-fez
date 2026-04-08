#include "camera_controller.h"

struct CameraControllerState
{
  Mat4 view_matrix;
  Vec3 eye;
  Vec3 at;
  Vec3 up;
  Vec3 eye_from;
  int eye_to_idx;
  bool slow_mode;
  AnimationImplementation anim_impl;
  Animation *anim;
  CameraInvalidateHandler invalidate_handler;
  void *invalidate_context;
};

static const Vec3 EYE_WAYPOINTS[] = {
  { 1, 1, 1 },
  { 1, -1, 1 },
  { -1, -1, 1 },
  { -1, 1, 1 }
};

static void invalidate(CameraController *controller)
{
  if (controller->state->invalidate_handler != NULL)
  {
    controller->state->invalidate_handler(controller->state->invalidate_context);
  }
}

static void anim_stopped(struct Animation* animation, bool finished, void *context);

static bool create_animation(CameraController *controller)
{
  controller->state->anim = animation_create();
  if (controller->state->anim == NULL)
  {
    return false;
  }

  animation_set_delay(controller->state->anim, controller->state->slow_mode ? 1000 : 500);
  animation_set_duration(controller->state->anim, controller->state->slow_mode ? 3000 : 500);
  animation_set_implementation(controller->state->anim, &controller->state->anim_impl);
  animation_set_handlers(controller->state->anim, (AnimationHandlers) {
    .stopped = anim_stopped,
  }, controller);

  return true;
}

static void anim_update(struct Animation* animation, const AnimationProgress time_normalized)
{
  CameraController *controller = animation_get_context(animation);
  float ratio = (float)time_normalized / ANIMATION_NORMALIZED_MAX;

  controller->state->eye.x = controller->state->eye_from.x * (1 - ratio) + EYE_WAYPOINTS[controller->state->eye_to_idx].x * ratio;
  controller->state->eye.y = controller->state->eye_from.y * (1 - ratio) + EYE_WAYPOINTS[controller->state->eye_to_idx].y * ratio;
  mat4_look_at_rh(&controller->state->view_matrix, &controller->state->eye, &controller->state->at, &controller->state->up);
  invalidate(controller);
}

static void anim_stopped(struct Animation* animation, bool finished, void *context)
{
  CameraController *controller = context;

  if (!finished)
  {
    return;
  }

  controller->state->eye = EYE_WAYPOINTS[controller->state->eye_to_idx];
  mat4_look_at_rh(&controller->state->view_matrix, &controller->state->eye, &controller->state->at, &controller->state->up);
  invalidate(controller);

  animation_destroy(animation);
  if (controller->state->anim == animation)
  {
    controller->state->anim = NULL;
  }
}

bool camera_controller_init(CameraController *controller, bool slow_mode,
  CameraInvalidateHandler invalidate_handler, void *invalidate_context)
{
  controller->state = NULL;
  controller->state = malloc(sizeof(CameraControllerState));
  if (controller->state == NULL)
  {
    return false;
  }

  controller->state->invalidate_handler = invalidate_handler;
  controller->state->invalidate_context = invalidate_context;
  controller->state->slow_mode = slow_mode;
  controller->state->eye = EYE_WAYPOINTS[0];
  controller->state->at = Vec3(0, 0, 0);
  controller->state->up = Vec3(0, 1, 0);
  controller->state->eye_from = controller->state->eye;
  controller->state->eye_to_idx = 0;
  controller->state->anim = NULL;
  controller->state->anim_impl.setup = NULL;
  controller->state->anim_impl.update = anim_update;
  controller->state->anim_impl.teardown = NULL;
  mat4_look_at_rh(&controller->state->view_matrix, &controller->state->eye,
    &controller->state->at, &controller->state->up);

  return true;
}

void camera_controller_deinit(CameraController *controller)
{
  if (controller->state == NULL)
  {
    return;
  }

  if (controller->state->anim != NULL)
  {
    animation_unschedule(controller->state->anim);
    animation_destroy(controller->state->anim);
    controller->state->anim = NULL;
  }

  free(controller->state);
  controller->state = NULL;
}

void camera_controller_set_slow_mode(CameraController *controller, bool slow_mode)
{
  if (controller->state == NULL)
  {
    return;
  }

  controller->state->slow_mode = slow_mode;
}

void camera_controller_start_transition(CameraController *controller)
{
  if (controller->state == NULL)
  {
    return;
  }

  if (controller->state->anim != NULL && animation_is_scheduled(controller->state->anim))
  {
    animation_unschedule(controller->state->anim);
  }

  if (controller->state->anim == NULL)
  {
    if (!create_animation(controller))
    {
      return;
    }
  }

  controller->state->eye_from = controller->state->eye;
  controller->state->eye_to_idx = (controller->state->eye_to_idx + 1) % ARRAY_LENGTH(EYE_WAYPOINTS);
  if (controller->state->anim != NULL)
  {
    animation_schedule(controller->state->anim);
  }
}

const Mat4 *camera_controller_get_view_matrix(const CameraController *controller)
{
  if (controller->state == NULL)
  {
    return NULL;
  }

  return &controller->state->view_matrix;
}

bool camera_controller_is_ready(const CameraController *controller)
{
  return controller->state != NULL;
}
