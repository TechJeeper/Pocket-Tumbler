#pragma once
#include <pebble.h>
#include "game.h"

#define ROCK_MAX_VERTS 12

typedef struct {
  GPoint verts[ROCK_MAX_VERTS];
  int n;
  GColor fill;
  GColor edge;
  GColor highlight;
  GColor facet_a;
  GColor facet_b;
  bool is_gem;
} RockShape;

// Build a procedural rock/gem outline centered at (cx,cy) with radius r.
// angle_deg rotates the silhouette for tumble animation.
void rock_build(RockShape *out, uint8_t stage, uint8_t gem_type,
                uint32_t seed, int cx, int cy, int r, int angle_deg);
void rock_draw(GContext *ctx, const RockShape *rock);
