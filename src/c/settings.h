#pragma once
#include <pebble.h>

enum {
  GOAL_20K = 0,
  GOAL_35K = 1,
  GOAL_50K = 2,
  GOAL_70K = 3,
};

enum {
  WEEKS_3 = 0,
  WEEKS_5 = 1,
  WEEKS_8 = 2,
};

typedef struct {
  uint8_t weekly_goal;  // GOAL_*
  uint8_t weeks_to_gem; // WEEKS_*
  uint8_t shake_bonus;  // 1 = IMU can add a little polish
  uint8_t vibe;         // 1 = buzz on milestones
} Settings;

void settings_init(void);
void settings_save(void);
Settings *settings_get(void);

void settings_apply(uint8_t weekly_goal, uint8_t weeks_to_gem,
                    uint8_t shake_bonus, uint8_t vibe);

uint32_t settings_weekly_goal_steps(void);
uint32_t settings_polish_goal(void);
