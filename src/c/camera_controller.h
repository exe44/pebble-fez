#pragma once

#include <pebble.h>
#include "math_helper.h"

typedef void (*CameraInvalidateHandler)(void *context);

typedef struct CameraController
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
} CameraController;

void camera_controller_init(CameraController *controller, bool slow_mode,
  CameraInvalidateHandler invalidate_handler, void *invalidate_context);
void camera_controller_deinit(CameraController *controller);
void camera_controller_set_slow_mode(CameraController *controller, bool slow_mode);
void camera_controller_start_transition(CameraController *controller);
const Mat4 *camera_controller_get_view_matrix(const CameraController *controller);
