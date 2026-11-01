#include <pebble.h>
#include "game.h"
#include "draw.h"
#include "tumble.h"
#include "settings.h"
#include "comm.h"

#define ACCEL_SAMPLES 3
#define TICK_MS 100

static Window *s_window;
static Game s_game;
static Tumble s_tumble;
static AppTimer *s_tick_timer;

static void request_redraw(void) {
  layer_mark_dirty(window_get_root_layer(s_window));
}

void app_on_settings_changed(void) {
  request_redraw();
}

static void refresh_health(void) {
#if defined(PBL_HEALTH)
  game_tick_calendar(&s_game);
  time_t now = time(NULL);
  time_t start = (time_t)s_game.week_start;
  if (now <= start) {
    return;
  }
  HealthServiceAccessibilityMask mask =
      health_service_metric_accessible(HealthMetricStepCount, start, now);
  if (mask & HealthServiceAccessibilityMaskAvailable) {
    HealthValue steps = health_service_sum(HealthMetricStepCount, start, now);
    if (steps >= 0) {
      game_sync_week_steps(&s_game, (uint32_t)steps);
    }
  } else {
    s_game.health_ok = 0;
  }
#else
  s_game.health_ok = 0;
#endif
}

#if defined(PBL_HEALTH)
static void health_handler(HealthEventType event, void *context) {
  (void)context;
  if (event == HealthEventMovementUpdate ||
      event == HealthEventSignificantUpdate) {
    refresh_health();
    request_redraw();
  }
}
#endif

static void accel_handler(AccelData *data, uint32_t num_samples) {
  int32_t sx = 0, sy = 0, sz = 0;
  uint32_t used = 0;
  for (uint32_t i = 0; i < num_samples; i++) {
    if (data[i].did_vibrate) {
      continue;
    }
    sx += data[i].x;
    sy += data[i].y;
    sz += data[i].z;
    used++;
  }
  if (used == 0) {
    return;
  }
  tumble_on_accel(&s_tumble,
                  (int16_t)(sx / (int32_t)used),
                  (int16_t)(sy / (int32_t)used),
                  (int16_t)(sz / (int32_t)used));
}

static void tick_cb(void *context) {
  (void)context;
  s_tick_timer = app_timer_register(TICK_MS, tick_cb, NULL);

  uint16_t credit = tumble_tick(&s_tumble);
  if (credit) {
    game_add_shake_polish(&s_game, credit);
  }

  static uint8_t health_div;
  if (++health_div >= 20) { // ~2s
    health_div = 0;
    refresh_health();
  }

  if (s_game.dirty || tumble_is_active(&s_tumble) || credit) {
    s_game.dirty = false;
    request_redraw();
  }
}

static void update_layer(Layer *layer, GContext *ctx) {
  draw_scene(ctx, layer_get_bounds(layer), &s_game, &s_tumble);
}

static void select_click(ClickRecognizerRef recognizer, void *context) {
  (void)recognizer;
  (void)context;
  if (game_is_complete(&s_game)) {
    game_claim_gem(&s_game);
    refresh_health();
    request_redraw();
  }
}

static void up_click(ClickRecognizerRef recognizer, void *context) {
  (void)recognizer;
  (void)context;
  refresh_health();
  request_redraw();
}

static void down_click(ClickRecognizerRef recognizer, void *context) {
  (void)recognizer;
  (void)context;
  // Long-press alternative: force a tumble visual pulse for feedback.
  s_tumble.energy = (int16_t)(s_tumble.energy + 400);
  if (s_tumble.energy > 1000) {
    s_tumble.energy = 1000;
  }
  request_redraw();
}

static void click_config(void *context) {
  (void)context;
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click);
  window_single_click_subscribe(BUTTON_ID_UP, up_click);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click);
}

static void window_load(Window *window) {
  Layer *root = window_get_root_layer(window);
  layer_set_update_proc(root, update_layer);
  window_set_click_config_provider(window, click_config);
}

static void window_unload(Window *window) {
  (void)window;
}

static void init(void) {
  settings_init();
  game_init(&s_game);
  tumble_init(&s_tumble);

  s_window = window_create();
  window_set_window_handlers(s_window, (WindowHandlers){
    .load = window_load,
    .unload = window_unload,
  });
  window_stack_push(s_window, true);

  comm_init();

  accel_data_service_subscribe(ACCEL_SAMPLES, accel_handler);
  accel_service_set_sampling_rate(ACCEL_SAMPLING_25HZ);

#if defined(PBL_HEALTH)
  health_service_events_subscribe(health_handler, NULL);
#endif
  refresh_health();

  s_tick_timer = app_timer_register(TICK_MS, tick_cb, NULL);
}

static void deinit(void) {
  if (s_tick_timer) {
    app_timer_cancel(s_tick_timer);
    s_tick_timer = NULL;
  }
  accel_data_service_unsubscribe();
#if defined(PBL_HEALTH)
  health_service_events_unsubscribe();
#endif
  game_deinit(&s_game);
  settings_save();
  window_destroy(s_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
