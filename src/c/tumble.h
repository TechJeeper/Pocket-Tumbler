#pragma once
#include <pebble.h>

typedef struct {
  int16_t energy;      // decaying tumble intensity 0..1000
  int16_t angle;       // animation angle (degrees-ish, wraps)
  int16_t offset_x;    // rock bob in barrel
  int16_t offset_y;
  int16_t last_ax;
  int16_t last_ay;
  int16_t last_az;
  bool have_sample;
} Tumble;

void tumble_init(Tumble *t);
void tumble_on_accel(Tumble *t, int16_t ax, int16_t ay, int16_t az);
// Advance animation; returns polish credit (0 if none).
uint16_t tumble_tick(Tumble *t);
bool tumble_is_active(const Tumble *t);
