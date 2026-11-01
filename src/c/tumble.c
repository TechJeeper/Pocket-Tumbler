#include "tumble.h"

void tumble_init(Tumble *t) {
  memset(t, 0, sizeof(*t));
}

void tumble_on_accel(Tumble *t, int16_t ax, int16_t ay, int16_t az) {
  if (!t->have_sample) {
    t->last_ax = ax;
    t->last_ay = ay;
    t->last_az = az;
    t->have_sample = true;
    return;
  }

  int32_t dx = (int32_t)ax - t->last_ax;
  int32_t dy = (int32_t)ay - t->last_ay;
  int32_t dz = (int32_t)az - t->last_az;
  t->last_ax = ax;
  t->last_ay = ay;
  t->last_az = az;

  // Magnitude of jerk — walking / shaking the wrist.
  int32_t jerk = dx * dx + dy * dy + dz * dz;
  if (jerk < 2500) {
    return;
  }
  int32_t bump = jerk / 800;
  if (bump > 180) {
    bump = 180;
  }
  t->energy = (int16_t)(t->energy + bump);
  if (t->energy > 1000) {
    t->energy = 1000;
  }
}

uint16_t tumble_tick(Tumble *t) {
  if (t->energy <= 0) {
    t->offset_x = 0;
    t->offset_y = 0;
    return 0;
  }

  t->angle = (int16_t)((t->angle + 8 + t->energy / 80) % 360);

  // Cheap sin/cos via quarter-wave table-ish using angle buckets.
  int a = t->angle;
  int sx = (a < 90) ? (a) : (a < 180) ? (180 - a) : (a < 270) ? (a - 180) : (360 - a);
  int sy = (a < 90) ? (90 - a) : (a < 180) ? (a - 90) : (a < 270) ? (270 - a) : (a - 270);
  if (a >= 180) sx = -sx;
  if (a < 90 || a >= 270) sy = sy; else sy = -sy;

  int amp = 2 + t->energy / 200;
  t->offset_x = (int16_t)((sx * amp) / 90);
  t->offset_y = (int16_t)((sy * amp) / 90);

  uint16_t credit = 0;
  if (t->energy > 120) {
    credit = (uint16_t)(1 + t->energy / 400);
  }

  t->energy = (int16_t)(t->energy - 18);
  if (t->energy < 0) {
    t->energy = 0;
  }
  return credit;
}

bool tumble_is_active(const Tumble *t) {
  return t->energy > 40;
}
