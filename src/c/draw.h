#pragma once
#include <pebble.h>
#include "game.h"
#include "tumble.h"

void draw_scene(GContext *ctx, GRect bounds, const Game *game, const Tumble *tumble);
