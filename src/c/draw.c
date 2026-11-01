#include "draw.h"
#include "rock.h"
#include "settings.h"

static void fill_rect(GContext *ctx, GRect r, GColor c) {
  graphics_context_set_fill_color(ctx, c);
  graphics_fill_rect(ctx, r, 0, GCornerNone);
}

static void draw_header(GContext *ctx, GRect bounds, const Game *game) {
#ifdef PBL_COLOR
  fill_rect(ctx, GRect(0, 0, bounds.size.w, 22), GColorOxfordBlue);
  graphics_context_set_text_color(ctx, GColorWhite);
#else
  fill_rect(ctx, GRect(0, 0, bounds.size.w, 22), GColorBlack);
  graphics_context_set_text_color(ctx, GColorWhite);
#endif
  graphics_draw_text(ctx, "Pocket Tumbler",
                     fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD),
                     GRect(4, 2, bounds.size.w - 8, 18),
                     GTextOverflowModeTrailingEllipsis,
                     GTextAlignmentLeft, NULL);

  char right[24];
  snprintf(right, sizeof(right), "%u gem%s",
           (unsigned)game->gems_collected,
           game->gems_collected == 1 ? "" : "s");
  graphics_draw_text(ctx, right,
                     fonts_get_system_font(FONT_KEY_GOTHIC_14),
                     GRect(4, 2, bounds.size.w - 8, 18),
                     GTextOverflowModeTrailingEllipsis,
                     GTextAlignmentRight, NULL);
}

static void draw_barrel(GContext *ctx, GRect barrel, bool spinning) {
#ifdef PBL_COLOR
  fill_rect(ctx, barrel, GColorDarkGreen);
  graphics_context_set_stroke_color(ctx, GColorBrass);
  graphics_context_set_fill_color(ctx, GColorArmyGreen);
#else
  fill_rect(ctx, barrel, GColorWhite);
  graphics_context_set_stroke_color(ctx, GColorBlack);
  graphics_context_set_fill_color(ctx, GColorWhite);
#endif

  // Drum body
  GRect drum = GRect(barrel.origin.x + 8, barrel.origin.y + 6,
                     barrel.size.w - 16, barrel.size.h - 14);
  graphics_fill_rect(ctx, drum, 8, GCornersAll);
  graphics_draw_round_rect(ctx, drum, 8);

  // Rib lines suggesting a rotating tumbler
  int mid = drum.origin.y + drum.size.h / 2;
  int shift = spinning ? 3 : 0;
  for (int i = -2; i <= 2; i++) {
    int y = mid + i * 7 + shift;
    if (y > drum.origin.y + 4 && y < drum.origin.y + drum.size.h - 4) {
      graphics_draw_line(ctx,
                         GPoint(drum.origin.x + 4, y),
                         GPoint(drum.origin.x + drum.size.w - 4, y));
    }
  }

  // End caps
#ifdef PBL_COLOR
  graphics_context_set_fill_color(ctx, GColorBrass);
#else
  graphics_context_set_fill_color(ctx, GColorBlack);
#endif
  graphics_fill_circle(ctx, GPoint(drum.origin.x, mid), 5);
  graphics_fill_circle(ctx, GPoint(drum.origin.x + drum.size.w, mid), 5);
}

static void draw_polish_bar(GContext *ctx, GRect bar, uint8_t pct, uint8_t stage) {
#ifdef PBL_COLOR
  fill_rect(ctx, bar, GColorDarkGray);
  graphics_context_set_fill_color(ctx, stage >= STAGE_GEM ? GColorChromeYellow : GColorPictonBlue);
#else
  fill_rect(ctx, bar, GColorWhite);
  graphics_context_set_stroke_color(ctx, GColorBlack);
  graphics_draw_rect(ctx, bar);
  graphics_context_set_fill_color(ctx, GColorBlack);
#endif
  int w = (bar.size.w - 2) * pct / 100;
  if (w > 0) {
    graphics_fill_rect(ctx, GRect(bar.origin.x + 1, bar.origin.y + 1, w, bar.size.h - 2),
                       0, GCornerNone);
  }
}

void draw_scene(GContext *ctx, GRect bounds, const Game *game, const Tumble *tumble) {
#ifdef PBL_COLOR
  fill_rect(ctx, bounds, GColorMidnightGreen);
#else
  fill_rect(ctx, bounds, GColorWhite);
#endif

  draw_header(ctx, bounds, game);

  GRect barrel = GRect(10, 28, bounds.size.w - 20, bounds.size.h - 78);
  bool spinning = tumble_is_active(tumble);
  draw_barrel(ctx, barrel, spinning);

  uint8_t stage = game_stage(game);
  uint8_t gem = game_gem_type(game);
  int cx = barrel.origin.x + barrel.size.w / 2 + tumble->offset_x;
  int cy = barrel.origin.y + barrel.size.h / 2 + tumble->offset_y;
  int radius = (barrel.size.w < barrel.size.h ? barrel.size.w : barrel.size.h) / 5;
  if (radius < 14) radius = 14;
  if (radius > 28) radius = 28;

  RockShape rock;
  rock_build(&rock, stage, gem, game->rock_seed, cx, cy, radius, tumble->angle);
  rock_draw(ctx, &rock);

  // Status lines
  char line1[48];
  char line2[48];
  uint8_t pct = game_polish_percent(game);

  if (game_is_complete(game)) {
    snprintf(line1, sizeof(line1), "%s ready!", game_gem_name(gem));
    snprintf(line2, sizeof(line2), "Select: new rock");
  } else {
    snprintf(line1, sizeof(line1), "%s  %u%%", game_stage_name(stage), (unsigned)pct);
    uint32_t goal = settings_weekly_goal_steps();
    uint32_t week = game->credited_week_steps;
    if (week > goal) week = goal;
    if (game->health_ok) {
      snprintf(line2, sizeof(line2), "Week %u/%uk steps",
               (unsigned)(week / 1000), (unsigned)(goal / 1000));
    } else {
      snprintf(line2, sizeof(line2), "Enable Pebble Health");
    }
  }

#ifdef PBL_COLOR
  graphics_context_set_text_color(ctx, GColorWhite);
#else
  graphics_context_set_text_color(ctx, GColorBlack);
#endif
  graphics_draw_text(ctx, line1,
                     fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD),
                     GRect(4, bounds.size.h - 48, bounds.size.w - 8, 22),
                     GTextOverflowModeTrailingEllipsis,
                     GTextAlignmentCenter, NULL);
  graphics_draw_text(ctx, line2,
                     fonts_get_system_font(FONT_KEY_GOTHIC_14),
                     GRect(4, bounds.size.h - 28, bounds.size.w - 8, 18),
                     GTextOverflowModeTrailingEllipsis,
                     GTextAlignmentCenter, NULL);

  draw_polish_bar(ctx,
                  GRect(14, bounds.size.h - 8, bounds.size.w - 28, 5),
                  pct, stage);
}
