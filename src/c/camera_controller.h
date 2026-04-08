#pragma once

#include <pebble.h>
#include "math_helper.h"

typedef void (*CameraInvalidateHandler)(void *context);

typedef struct CameraControllerState CameraControllerState;

typedef struct CameraController
{
  CameraControllerState *state;
} CameraController;

bool camera_controller_init(CameraController *controller, bool slow_mode,
  CameraInvalidateHandler invalidate_handler, void *invalidate_context);
void camera_controller_deinit(CameraController *controller);
void camera_controller_set_slow_mode(CameraController *controller, bool slow_mode);
void camera_controller_start_transition(CameraController *controller);
const Mat4 *camera_controller_get_view_matrix(const CameraController *controller);
bool camera_controller_is_ready(const CameraController *controller);
