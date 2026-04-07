#include <pebble.h>
#include "app_settings.h"
#include "camera_controller.h"
#include "clock_digits.h"
#include "digit_renderer.h"

static Window* window;
static AppSettings settings;
static CameraController camera_controller;
static DigitRenderer digit_renderer;

static void invalidate_digit_layers(void *context)
{
  digit_renderer_mark_all_dirty(&digit_renderer);
}

//==============================================================================

static bool has_current_digits;
static ClockDigits current_digits;

static void apply_visual_settings(void)
{
  if (window == NULL)
  {
    return;
  }

  window_set_background_color(window, app_settings_get_background_color(&settings));

  if (!digit_renderer_is_ready(&digit_renderer))
  {
    return;
  }

  digit_renderer_mark_all_dirty(&digit_renderer);
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context)
{
  if (app_settings_apply_message(&settings, iterator))
  {
    app_settings_save(&settings);
    camera_controller_set_slow_mode(&camera_controller, settings.slow_version);
    apply_visual_settings();
  }
}

// Called once per minute
static void handle_minute_tick(struct tm* time, TimeUnits units_changed)
{
  ClockDigits next_digits;
  ClockDigitsDiff diff;

  clock_digits_from_time(time, clock_is_24h_style(), &next_digits);

  if (!has_current_digits)
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
    clock_digits_diff(&current_digits, &next_digits, &diff);
  }

  for (int i = 0; i < CLOCK_DIGIT_COUNT; ++i)
  {
    if (!diff.changed[i])
    {
      continue;
    }

    digit_renderer_set_digit(&digit_renderer, i, next_digits.value[i], next_digits.hidden[i]);
  }

  current_digits = next_digits;
  has_current_digits = true;

  if (!diff.minute_changed)
  {
    return;
  }

  camera_controller_start_transition(&camera_controller);
}

//==============================================================================

static void window_load(Window* window)
{
  Layer *root_layer = window_get_root_layer(window);

  // view_matrix should be ready before poly layer creation
  camera_controller_init(&camera_controller, settings.slow_version, invalidate_digit_layers, NULL);
  digit_renderer_init(&digit_renderer, root_layer, &settings, camera_controller_get_view_matrix(&camera_controller));

  // Ensures time is displayed immediately

  time_t timestamp = time(NULL);
  struct tm *time = localtime(&timestamp);
  handle_minute_tick(time, MINUTE_UNIT);
}

static void window_unload(Window *window)
{
  digit_renderer_deinit(&digit_renderer);
  camera_controller_deinit(&camera_controller);
}

//==============================================================================

static void handle_init()
{
  app_settings_load(&settings);

  window = window_create();
  window_set_background_color(window, app_settings_get_background_color(&settings));
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
