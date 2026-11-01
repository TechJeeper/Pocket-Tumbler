#include "comm.h"
#include "settings.h"

extern void app_on_settings_changed(void);

static int32_t tuple_int(Tuple *t, int32_t fallback) {
  if (!t) {
    return fallback;
  }
  if (t->type == TUPLE_INT) {
    switch (t->length) {
      case 1: return t->value->int8;
      case 2: return t->value->int16;
      default: return t->value->int32;
    }
  }
  if (t->type == TUPLE_UINT) {
    switch (t->length) {
      case 1: return t->value->uint8;
      case 2: return t->value->uint16;
      default: return (int32_t)t->value->uint32;
    }
  }
  if (t->type == TUPLE_CSTRING) {
    return atoi(t->value->cstring);
  }
  return fallback;
}

static void inbox_received(DictionaryIterator *it, void *context) {
  (void)context;
  Settings *s = settings_get();

  // Clay sends messageKey fields directly (no Cmd wrapper).
  uint8_t weekly = (uint8_t)tuple_int(dict_find(it, MESSAGE_KEY_WeeklyGoal), s->weekly_goal);
  uint8_t weeks = (uint8_t)tuple_int(dict_find(it, MESSAGE_KEY_WeeksToGem), s->weeks_to_gem);
  uint8_t shake = (uint8_t)tuple_int(dict_find(it, MESSAGE_KEY_ShakeBonus), s->shake_bonus);
  uint8_t vibe = (uint8_t)tuple_int(dict_find(it, MESSAGE_KEY_Vibe), s->vibe);

  settings_apply(weekly, weeks, shake, vibe);
  app_on_settings_changed();
}

void comm_init(void) {
  app_message_register_inbox_received(inbox_received);
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
}
