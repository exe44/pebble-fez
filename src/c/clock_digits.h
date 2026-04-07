#pragma once

#include <pebble.h>

#define CLOCK_DIGIT_COUNT 4

typedef struct ClockDigits
{
  int value[CLOCK_DIGIT_COUNT];
  bool hidden[CLOCK_DIGIT_COUNT];
} ClockDigits;

typedef struct ClockDigitsDiff
{
  bool changed[CLOCK_DIGIT_COUNT];
  bool hour_changed;
  bool minute_changed;
} ClockDigitsDiff;

void clock_digits_from_time(const struct tm *time_value, bool is_24h_style, ClockDigits *out_digits);
void clock_digits_diff(const ClockDigits *previous, const ClockDigits *current, ClockDigitsDiff *out_diff);
