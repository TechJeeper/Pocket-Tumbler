#include "settings.h"

#define KEY_SETTINGS 1

static Settings s_settings = {
  .weekly_goal = GOAL_50K,
  .weeks_to_gem = WEEKS_5,
  .shake_bonus = 1,
  .vibe = 1,
};

static uint8_t clamp_u8(uint8_t v, uint8_t max) {
  return v > max ? max : v;
}

void settings_init(void) {
  if (persist_exists(KEY_SETTINGS)) {
    persist_read_data(KEY_SETTINGS, &s_settings, sizeof(s_settings));
    s_settings.weekly_goal = clamp_u8(s_settings.weekly_goal, GOAL_70K);
    s_settings.weeks_to_gem = clamp_u8(s_settings.weeks_to_gem, WEEKS_8);
    s_settings.shake_bonus = s_settings.shake_bonus ? 1 : 0;
    s_settings.vibe = s_settings.vibe ? 1 : 0;
  }
}

void settings_save(void) {
  persist_write_data(KEY_SETTINGS, &s_settings, sizeof(s_settings));
}

Settings *settings_get(void) {
  return &s_settings;
}

void settings_apply(uint8_t weekly_goal, uint8_t weeks_to_gem,
                    uint8_t shake_bonus, uint8_t vibe) {
  s_settings.weekly_goal = clamp_u8(weekly_goal, GOAL_70K);
  s_settings.weeks_to_gem = clamp_u8(weeks_to_gem, WEEKS_8);
  s_settings.shake_bonus = shake_bonus ? 1 : 0;
  s_settings.vibe = vibe ? 1 : 0;
  settings_save();
}

uint32_t settings_weekly_goal_steps(void) {
  switch (s_settings.weekly_goal) {
    case GOAL_20K: return 20000;
    case GOAL_35K: return 35000;
    case GOAL_70K: return 70000;
    case GOAL_50K:
    default: return 50000;
  }
}

uint32_t settings_polish_goal(void) {
  uint32_t weeks;
  switch (s_settings.weeks_to_gem) {
    case WEEKS_3: weeks = 3; break;
    case WEEKS_8: weeks = 8; break;
    case WEEKS_5:
    default: weeks = 5; break;
  }
  return settings_weekly_goal_steps() * weeks;
}
