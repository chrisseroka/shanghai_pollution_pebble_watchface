#include <pebble.h>

static Window *window;
static TextLayer *pm25_value_layer;
static TextLayer *pm25_label_layer;
static TextLayer *pm2524h_value_layer;
static TextLayer *pm2524h_label_layer;
static TextLayer *aqi_value_layer;
static TextLayer *aqi_label_layer;
static TextLayer *time_layer;

static AppSync sync;
static uint8_t sync_buffer[64];

enum {
  PM25 = 0x0,
  PM2524H = 0x1,
  AQI = 0x2
};

void sync_tuple_changed_callback(const uint32_t key,
        const Tuple* new_tuple, const Tuple* old_tuple, void* context) {

  APP_LOG(APP_LOG_LEVEL_DEBUG, "PM25 changed:: %s", new_tuple->value->cstring);
  switch (key) {
    case PM25:
      text_layer_set_text(pm25_value_layer, new_tuple->value->cstring);
      break;
    case PM2524H:
      text_layer_set_text(pm2524h_value_layer, new_tuple->value->cstring);
      break;
    case AQI:
      text_layer_set_text(aqi_value_layer, new_tuple->value->cstring);
      break;
  }
}

// http://stackoverflow.com/questions/21150193/logging-enums-on-the-pebble-watch/21172222#21172222
char *translate_error(AppMessageResult result) {
  switch (result) {
    case APP_MSG_OK: return "APP_MSG_OK";
    case APP_MSG_SEND_TIMEOUT: return "APP_MSG_SEND_TIMEOUT";
    case APP_MSG_SEND_REJECTED: return "APP_MSG_SEND_REJECTED";
    case APP_MSG_NOT_CONNECTED: return "APP_MSG_NOT_CONNECTED";
    case APP_MSG_APP_NOT_RUNNING: return "APP_MSG_APP_NOT_RUNNING";
    case APP_MSG_INVALID_ARGS: return "APP_MSG_INVALID_ARGS";
    case APP_MSG_BUSY: return "APP_MSG_BUSY";
    case APP_MSG_BUFFER_OVERFLOW: return "APP_MSG_BUFFER_OVERFLOW";
    case APP_MSG_ALREADY_RELEASED: return "APP_MSG_ALREADY_RELEASED";
    case APP_MSG_CALLBACK_ALREADY_REGISTERED: return "APP_MSG_CALLBACK_ALREADY_REGISTERED";
    case APP_MSG_CALLBACK_NOT_REGISTERED: return "APP_MSG_CALLBACK_NOT_REGISTERED";
    case APP_MSG_OUT_OF_MEMORY: return "APP_MSG_OUT_OF_MEMORY";
    case APP_MSG_CLOSED: return "APP_MSG_CLOSED";
    case APP_MSG_INTERNAL_ERROR: return "APP_MSG_INTERNAL_ERROR";
    default: return "UNKNOWN ERROR";
  }
}

void sync_error_callback(DictionaryResult dict_error, AppMessageResult app_message_error, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "... Sync Error: %s", translate_error(app_message_error));
}

static void ask_for_pollution_update(void) {
  Tuplet value = TupletInteger(1, 1);

  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);

  if (iter == NULL) {
    return;
  }

  dict_write_tuplet(iter, &value);
  dict_write_end(iter);

  app_message_outbox_send();
}

static void handle_second_tick(struct tm* tick_time, TimeUnits units_changed) {
  static char time_text[] = "00:00";
  static char minutes_text[] = "00";
  static char seconds_text[] = "00";
  strftime(time_text, sizeof(time_text), "%l:%M", tick_time);
  text_layer_set_text(time_layer, time_text);
  strftime(minutes_text, sizeof(minutes_text), "%M", tick_time);
  strftime(seconds_text, sizeof(seconds_text), "%S", tick_time);
  if ((strcmp(minutes_text, "00") == 0) && (strcmp(seconds_text, "00") == 0)){
    APP_LOG(APP_LOG_LEVEL_DEBUG,"Time changed:%s asking for update", seconds_text);
    ask_for_pollution_update();
  }
}

static void init_clock(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  time_layer = text_layer_create(GRect(0, 10, bounds.size.w, bounds.size.h-100));
  text_layer_set_text_alignment(time_layer, GTextAlignmentCenter);
  text_layer_set_text_color(time_layer, GColorWhite);
  text_layer_set_background_color(time_layer, GColorClear);
  text_layer_set_font(time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));

  time_t now = time(NULL);
  struct tm *current_time = localtime(&now);
  handle_second_tick(current_time, SECOND_UNIT);
  tick_timer_service_subscribe(SECOND_UNIT, &handle_second_tick);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(time_layer));
}

static void init_pollution_update(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  aqi_label_layer = text_layer_create((GRect) { .origin = { 0, 70 }, .size = { 90, 100 } });
  text_layer_set_text(aqi_label_layer, "Air Quality");
  text_layer_set_text_color(aqi_label_layer, GColorWhite);
  text_layer_set_text_alignment(aqi_label_layer, GTextAlignmentLeft);
  text_layer_set_background_color(aqi_label_layer, GColorClear);
  text_layer_set_font(aqi_label_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
  layer_add_child(window_layer, text_layer_get_layer(aqi_label_layer));

  pm25_label_layer = text_layer_create((GRect) { .origin = { 0, 100 }, .size = { 90, 100 } });
  text_layer_set_text(pm25_label_layer, "PM2.5(1h)");
  text_layer_set_text_color(pm25_label_layer, GColorWhite);
  text_layer_set_text_alignment(pm25_label_layer, GTextAlignmentLeft);
  text_layer_set_background_color(pm25_label_layer, GColorClear);
  text_layer_set_font(pm25_label_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
  layer_add_child(window_layer, text_layer_get_layer(pm25_label_layer));

  pm2524h_label_layer = text_layer_create((GRect) { .origin = { 0, 130 }, .size = { 90, 100 } });
  text_layer_set_text(pm2524h_label_layer, "PM2.5(24h)");
  text_layer_set_text_color(pm2524h_label_layer, GColorWhite);
  text_layer_set_text_alignment(pm2524h_label_layer, GTextAlignmentLeft);
  text_layer_set_background_color(pm2524h_label_layer, GColorClear);
  text_layer_set_font(pm2524h_label_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
  layer_add_child(window_layer, text_layer_get_layer(pm2524h_label_layer));

  aqi_value_layer = text_layer_create((GRect) { .origin = { 25, 60 }, .size = { bounds.size.w, bounds.size.h } });
  text_layer_set_text(aqi_value_layer, "");
  text_layer_set_text_color(aqi_value_layer, GColorWhite);
  text_layer_set_text_alignment(aqi_value_layer, GTextAlignmentCenter);
  text_layer_set_background_color(aqi_value_layer, GColorClear);
  text_layer_set_overflow_mode(aqi_value_layer, GTextOverflowModeFill);
  text_layer_set_font(aqi_value_layer, fonts_get_system_font(FONT_KEY_BITHAM_34_MEDIUM_NUMBERS));
  layer_add_child(window_layer, text_layer_get_layer(aqi_value_layer));

  pm25_value_layer = text_layer_create((GRect) { .origin = { 25, 90 }, .size = { bounds.size.w, bounds.size.h } });
  text_layer_set_text(pm25_value_layer, "");
  text_layer_set_text_color(pm25_value_layer, GColorWhite);
  text_layer_set_text_alignment(pm25_value_layer, GTextAlignmentCenter);
  text_layer_set_background_color(pm25_value_layer, GColorClear);
  text_layer_set_overflow_mode(pm25_value_layer, GTextOverflowModeFill);
  text_layer_set_font(pm25_value_layer, fonts_get_system_font(FONT_KEY_BITHAM_34_MEDIUM_NUMBERS));
  layer_add_child(window_layer, text_layer_get_layer(pm25_value_layer));

  pm2524h_value_layer = text_layer_create((GRect) { .origin = { 25, 120 }, .size = { bounds.size.w, bounds.size.h } });
  text_layer_set_text(pm2524h_value_layer, "");
  text_layer_set_text_color(pm2524h_value_layer, GColorWhite);
  text_layer_set_text_alignment(pm2524h_value_layer, GTextAlignmentCenter);
  text_layer_set_background_color(pm2524h_value_layer, GColorClear);
  text_layer_set_overflow_mode(pm2524h_value_layer, GTextOverflowModeFill);
  text_layer_set_font(pm2524h_value_layer, fonts_get_system_font(FONT_KEY_BITHAM_34_MEDIUM_NUMBERS));
  layer_add_child(window_layer, text_layer_get_layer(pm2524h_value_layer));

  Tuplet initial_values[] = {
     TupletCString(PM25, ""),
     TupletCString(PM2524H, ""),
     TupletCString(AQI, ""),
  };

  app_sync_init(&sync, sync_buffer, sizeof(sync_buffer), initial_values, ARRAY_LENGTH(initial_values),
      sync_tuple_changed_callback, sync_error_callback, NULL);
}


static void window_load(Window *window) {
  init_pollution_update(window);
  init_clock(window);
}

static void window_unload(Window *window) {
  text_layer_destroy(pm25_value_layer);
  text_layer_destroy(pm25_label_layer);
  text_layer_destroy(pm2524h_value_layer);
  text_layer_destroy(pm2524h_label_layer);
  text_layer_destroy(aqi_value_layer);
  text_layer_destroy(aqi_label_layer);
  text_layer_destroy(time_layer);
}

static void init(void) {
  window = window_create();
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });

  app_message_open(64, 64);

  const bool animated = true;
  window_stack_push(window, animated);
  window_set_background_color(window, GColorBlack);
}

static void deinit(void) {
  window_destroy(window);
}

int main(void) {
  init();

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", window);

  app_event_loop();
  deinit();
}
