#include "game.h"
#include "settings.h"

#define KEY_GAME 2

static uint32_t start_of_week_monday(void) {
  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  // tm_wday: 0=Sun … 6=Sat → days since Monday
  int since_mon = (t->tm_wday + 6) % 7;
  t->tm_hour = 0;
  t->tm_min = 0;
  t->tm_sec = 0;
  time_t midnight = mktime(t);
  return (uint32_t)(midnight - (time_t)since_mon * 24 * 60 * 60);
}

static uint32_t day_stamp(void) {
  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  return (uint32_t)(t->tm_year * 1000 + t->tm_yday);
}

static void reseal_rock(Game *g) {
  g->polish_xp = 0;
  g->credited_week_steps = 0;
  g->week_bonus_claimed = 0;
  g->gem_claimed = 0;
  g->shake_today = 0;
  g->shake_day = day_stamp();
  g->rock_seed = (uint32_t)time(NULL) ^ ((uint32_t)g->gems_collected * 2654435761u);
  if (g->rock_seed == 0) {
    g->rock_seed = 0xA5A5u;
  }
}

void game_init(Game *g) {
  memset(g, 0, sizeof(*g));
  if (persist_exists(KEY_GAME)) {
    persist_read_data(KEY_GAME, g, sizeof(*g));
  } else {
    reseal_rock(g);
    g->week_start = start_of_week_monday();
  }
  g->health_ok = 0;
  g->dirty = true;
  game_tick_calendar(g);
}

void game_deinit(Game *g) {
  game_save(g);
}

void game_save(Game *g) {
  persist_write_data(KEY_GAME, g, sizeof(*g));
}

void game_tick_calendar(Game *g) {
  uint32_t week = start_of_week_monday();
  if (week != g->week_start) {
    g->week_start = week;
    g->credited_week_steps = 0;
    g->week_bonus_claimed = 0;
    g->dirty = true;
    game_save(g);
  }
  uint32_t day = day_stamp();
  if (day != g->shake_day) {
    g->shake_day = day;
    g->shake_today = 0;
    game_save(g);
  }
}

void game_sync_week_steps(Game *g, uint32_t week_steps) {
  game_tick_calendar(g);
  g->health_ok = 1;

  if (week_steps > g->credited_week_steps) {
    uint32_t delta = week_steps - g->credited_week_steps;
    g->credited_week_steps = week_steps;
    g->polish_xp += delta;
    g->dirty = true;

    uint32_t goal = settings_weekly_goal_steps();
    if (!g->week_bonus_claimed && week_steps >= goal) {
      g->week_bonus_claimed = 1;
      g->polish_xp += goal / 10; // 10% bonus for hitting the weekly goal
      if (settings_get()->vibe) {
        vibes_short_pulse();
      }
    }
    game_save(g);
  }

  if (game_is_complete(g) && !g->gem_claimed && settings_get()->vibe) {
    // Soft celebrate the first time we cross the finish line.
    vibes_double_pulse();
    g->gem_claimed = 1;
    game_save(g);
  }
}

bool game_add_shake_polish(Game *g, uint16_t amount) {
  if (!settings_get()->shake_bonus || amount == 0) {
    return false;
  }
  game_tick_calendar(g);
  const uint16_t daily_cap = 800;
  if (g->shake_today >= daily_cap) {
    return false;
  }
  uint16_t room = (uint16_t)(daily_cap - g->shake_today);
  if (amount > room) {
    amount = room;
  }
  g->shake_today = (uint16_t)(g->shake_today + amount);
  g->polish_xp += amount;
  g->dirty = true;
  game_save(g);
  return true;
}

uint32_t game_polish_goal(void) {
  return settings_polish_goal();
}

uint8_t game_stage(const Game *g) {
  uint32_t goal = game_polish_goal();
  if (goal == 0) {
    return STAGE_ROUGH;
  }
  if (g->polish_xp >= goal) {
    return STAGE_GEM;
  }
  uint32_t pct = (g->polish_xp * 100) / goal;
  if (pct < 14) return STAGE_ROUGH;
  if (pct < 28) return STAGE_ANGULAR;
  if (pct < 42) return STAGE_SOFTENED;
  if (pct < 56) return STAGE_SMOOTH;
  if (pct < 72) return STAGE_POLISHED;
  if (pct < 90) return STAGE_GLEAMING;
  return STAGE_GEM;
}

uint8_t game_polish_percent(const Game *g) {
  uint32_t goal = game_polish_goal();
  if (goal == 0) {
    return 0;
  }
  uint32_t pct = (g->polish_xp * 100) / goal;
  return pct > 100 ? 100 : (uint8_t)pct;
}

uint8_t game_gem_type(const Game *g) {
  return (uint8_t)(g->rock_seed % GEM_TYPE_COUNT);
}

const char *game_stage_name(uint8_t stage) {
  switch (stage) {
    case STAGE_ANGULAR: return "Angular";
    case STAGE_SOFTENED: return "Softened";
    case STAGE_SMOOTH: return "Smooth";
    case STAGE_POLISHED: return "Polished";
    case STAGE_GLEAMING: return "Gleaming";
    case STAGE_GEM: return "Gem";
    case STAGE_ROUGH:
    default: return "Rough";
  }
}

const char *game_gem_name(uint8_t type) {
  switch (type) {
    case GEM_SAPPHIRE: return "Sapphire";
    case GEM_EMERALD: return "Emerald";
    case GEM_AMETHYST: return "Amethyst";
    case GEM_TOPAZ: return "Topaz";
    case GEM_CITRINE: return "Citrine";
    case GEM_AQUA: return "Aquamarine";
    case GEM_GARNET: return "Garnet";
    case GEM_RUBY:
    default: return "Ruby";
  }
}

bool game_is_complete(const Game *g) {
  return g->polish_xp >= game_polish_goal();
}

void game_claim_gem(Game *g) {
  if (game_is_complete(g)) {
    g->gems_collected++;
  }
  reseal_rock(g);
  g->week_start = start_of_week_monday();
  g->dirty = true;
  game_save(g);
  if (settings_get()->vibe) {
    vibes_short_pulse();
  }
}
