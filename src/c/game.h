#pragma once
#include <pebble.h>

enum {
  STAGE_ROUGH = 0,
  STAGE_ANGULAR = 1,
  STAGE_SOFTENED = 2,
  STAGE_SMOOTH = 3,
  STAGE_POLISHED = 4,
  STAGE_GLEAMING = 5,
  STAGE_GEM = 6,
  STAGE_COUNT = 7,
};

enum {
  GEM_RUBY = 0,
  GEM_SAPPHIRE = 1,
  GEM_EMERALD = 2,
  GEM_AMETHYST = 3,
  GEM_TOPAZ = 4,
  GEM_CITRINE = 5,
  GEM_AQUA = 6,
  GEM_GARNET = 7,
  GEM_TYPE_COUNT = 8,
};

typedef struct {
  uint32_t polish_xp;
  uint32_t credited_week_steps;
  uint32_t week_start;       // monday 00:00 local
  uint32_t rock_seed;
  uint16_t gems_collected;
  uint16_t shake_today;      // polish credited from shake today
  uint32_t shake_day;        // day-of-year stamp for shake_today reset
  uint8_t week_bonus_claimed;
  uint8_t gem_claimed;       // already celebrated this gem
  uint8_t health_ok;
  uint8_t dirty;
} Game;

void game_init(Game *g);
void game_deinit(Game *g);
void game_save(Game *g);

// Sync week boundary + credit new pedometer steps into polish_xp.
void game_tick_calendar(Game *g);
void game_sync_week_steps(Game *g, uint32_t week_steps);

// Small polish from IMU tumbling (capped per day). Returns true if XP changed.
bool game_add_shake_polish(Game *g, uint16_t amount);

uint8_t game_stage(const Game *g);
uint32_t game_polish_goal(void);
uint8_t game_polish_percent(const Game *g);
uint8_t game_gem_type(const Game *g);
const char *game_stage_name(uint8_t stage);
const char *game_gem_name(uint8_t type);

// Claim finished gem and start a fresh rough rock.
void game_claim_gem(Game *g);

bool game_is_complete(const Game *g);
