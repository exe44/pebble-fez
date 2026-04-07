#include "clock_digits.h"

static int calculate_12_hour(int hour)
{
  if (hour == 0)
  {
    return 12;
  }

  if (hour > 12)
  {
    return hour - 12;
  }

  return hour;
}

void clock_digits_from_time(const struct tm *time_value, bool is_24h_style, ClockDigits *out_digits)
{
  int hour = is_24h_style ? time_value->tm_hour : calculate_12_hour(time_value->tm_hour);

  out_digits->value[0] = hour / 10;
  out_digits->value[1] = hour % 10;
  out_digits->value[2] = time_value->tm_min / 10;
  out_digits->value[3] = time_value->tm_min % 10;

  out_digits->hidden[0] = !is_24h_style && out_digits->value[0] == 0;
  out_digits->hidden[1] = false;
  out_digits->hidden[2] = false;
  out_digits->hidden[3] = false;
}

void clock_digits_diff(const ClockDigits *previous, const ClockDigits *current, ClockDigitsDiff *out_diff)
{
  out_diff->hour_changed = false;
  out_diff->minute_changed = false;

  for (int i = 0; i < CLOCK_DIGIT_COUNT; ++i)
  {
    out_diff->changed[i] = previous->value[i] != current->value[i] ||
      previous->hidden[i] != current->hidden[i];
  }

  out_diff->hour_changed = out_diff->changed[0] || out_diff->changed[1];
  out_diff->minute_changed = out_diff->changed[2] || out_diff->changed[3];
}
