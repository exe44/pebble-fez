#include <pebble.h>
#include "app_settings.h"
#include "camera_controller.h"
#include "clock_digits.h"
#include "digit_renderer.h"

//==============================================================================
// app state

static Window* s_window;
static AppSettings s_settings;
static CameraController s_camera_controller;
static DigitRenderer s_digit_renderer;

static void invalidate_digit_layers(void *context)
{
  digit_renderer_mark_all_dirty(&s_digit_renderer);
}

//==============================================================================
// clock state

static bool s_has_current_digits;
static ClockDigits s_current_digits;

//==============================================================================
// settings / redraw

static void apply_visual_settings(void)
{
  if (s_window == NULL)
  {
    return;
  }

  window_set_background_color(s_window, app_settings_get_background_color(&s_settings));

  if (!digit_renderer_is_ready(&s_digit_renderer))
  {
    return;
  }

  digit_renderer_mark_all_dirty(&s_digit_renderer);
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context)
{
  if (app_settings_apply_message(&s_settings, iterator))
  {
    app_settings_save(&s_settings);
    camera_controller_set_slow_mode(&s_camera_controller, s_settings.slow_version);
    apply_visual_settings();
  }
}

//==============================================================================
// tick handling

// Called once per minute
static void handle_minute_tick(struct tm* time, TimeUnits units_changed)
{
  ClockDigits next_digits;
  ClockDigitsDiff diff;

  clock_digits_from_time(time, clock_is_24h_style(), &next_digits);

  if (!s_has_current_digits)
  {
    for (int i = 0; i < CLOCK_DIGIT_COUNT; ++i)
    {
      diff.changed[i] = true;
    }
    diff.hour_changed = true;
    diff.minute_changed = true;
  }
  else
  {
    clock_digits_diff(&s_current_digits, &next_digits, &diff);
  }

  s_current_digits = next_digits;
  s_has_current_digits = true;

  if (!digit_renderer_is_ready(&s_digit_renderer))
  {
    return;
  }

  for (int i = 0; i < CLOCK_DIGIT_COUNT; ++i)
  {
    if (!diff.changed[i])
    {
      continue;
    }

    digit_renderer_set_digit(&s_digit_renderer, i, next_digits.value[i], next_digits.hidden[i]);
  }

  if (!diff.minute_changed)
  {
    return;
  }

  camera_controller_start_transition(&s_camera_controller);
}

//==============================================================================
// window lifecycle

static void window_load(Window* window)
{
  Layer *root_layer = window_get_root_layer(window);

  // view_matrix should be ready before poly layer creation
  camera_controller_init(&s_camera_controller, s_settings.slow_version, invalidate_digit_layers, NULL);
  if (!digit_renderer_init(&s_digit_renderer, root_layer, &s_settings,
    camera_controller_get_view_matrix(&s_camera_controller)))
  {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Failed to initialize digit renderer");
    return;
  }

  s_has_current_digits = false;

  // Ensures time is displayed immediately

  time_t timestamp = time(NULL);
  struct tm *time = localtime(&timestamp);
  handle_minute_tick(time, MINUTE_UNIT);
}

static void window_unload(Window *window)
{
  digit_renderer_deinit(&s_digit_renderer);
  camera_controller_deinit(&s_camera_controller);
}

//==============================================================================
// app lifecycle

static void handle_init()
{
  app_settings_load(&s_settings);

  s_window = window_create();
  window_set_background_color(s_window, app_settings_get_background_color(&s_settings));
  window_set_window_handlers(s_window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  window_stack_push(s_window, true);

  app_message_register_inbox_received(inbox_received_callback);
  app_message_open(128, 128);
  tick_timer_service_subscribe(MINUTE_UNIT, handle_minute_tick);
}

static void handle_deinit(void)
{
  window_destroy(s_window);
}

int main(void)
{
  handle_init();
  app_event_loop();
  handle_deinit();
}
