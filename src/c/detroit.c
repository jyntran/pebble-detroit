#include <pebble.h>
#include <ctype.h>

static Window *s_window;
static TextLayer  *s_time_layer,
                  *s_ampm_layer,
                  *s_date_layer;
static GFont s_bold_font,
             s_light_font;

static void to_uppercase(char *str) {
  int i = 0;
  while (str[i]) {
    str[i] = toupper((unsigned char) str[i]);
    i++;
  }
}

static void update_time() {
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
  static char s_time_buffer[8];
  static char s_ampm_buffer[4];
  strftime(s_time_buffer, sizeof(s_time_buffer), "%I:%M", tick_time);
  strftime(s_ampm_buffer, sizeof(s_ampm_buffer), "%p", tick_time);
  text_layer_set_text(s_time_layer, s_time_buffer);
  text_layer_set_text(s_ampm_layer, s_ampm_buffer);
}

static char *get_ordinal(int num) {
  switch (num % 100) {
    case 11:
    case 12:
    case 13:
      return "th";
  }

  switch (num % 10) {
    case 1:
      return "st";
    case 2:
      return "nd";
    case 3:
      return "rd";
    default:
      return "th";
  }
}

static void update_date() {
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
  static char *s_date_format;
  s_date_format = strcat("%b\n%e", get_ordinal(tick_time->tm_mday));
  static char s_date_buffer[16];
  strftime(s_date_buffer, sizeof(s_date_buffer), s_date_format, tick_time);
  to_uppercase(s_date_buffer);
  text_layer_set_text(s_date_layer, s_date_buffer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
  if (units_changed & DAY_UNIT) {
    update_date();
  }
}

static void prv_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  window_set_background_color(s_window, GColorBlack);

  int v_padding = 7;
  int h_padding = 4;
  int time_height = 42;
  int time_width = bounds.size.w - h_padding;

  s_bold_font = fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD);
  s_light_font = fonts_get_system_font(FONT_KEY_BITHAM_42_LIGHT);

  s_ampm_layer = text_layer_create(
    GRect(
      h_padding, bounds.size.h-(time_height*2)-v_padding,
      time_width, time_height
    )
  );
  text_layer_set_font(s_ampm_layer, s_light_font);
  text_layer_set_text_alignment(s_ampm_layer, GTextAlignmentLeft);
  text_layer_set_background_color(s_ampm_layer, GColorClear);
  text_layer_set_text_color(s_ampm_layer, GColorWhite);
  layer_add_child(window_layer, text_layer_get_layer(s_ampm_layer));

  s_time_layer = text_layer_create(
    GRect(
      h_padding, bounds.size.h-(time_height*1)-v_padding,
      time_width, time_height
    )
  );
  text_layer_set_font(s_time_layer, s_bold_font);
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentLeft);
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorWhite);
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));

  s_date_layer = text_layer_create(
    GRect(
      h_padding, bounds.size.h-(time_height*4)-v_padding,
      bounds.size.w, time_height*2
    )
  );
  text_layer_set_font(s_date_layer, s_bold_font);
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentLeft);
  text_layer_set_background_color(s_date_layer, GColorClear);
  text_layer_set_text_color(s_date_layer, GColorPictonBlue);
  layer_add_child(window_layer, text_layer_get_layer(s_date_layer));

  update_time();
  update_date();
}

static void prv_window_unload(Window *window) {
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_ampm_layer);
  text_layer_destroy(s_date_layer);
}

static void prv_init(void) {
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);

  s_window = window_create();
  window_set_window_handlers(s_window, (WindowHandlers) {
    .load = prv_window_load,
    .unload = prv_window_unload,
  });
  const bool animated = true;
  window_stack_push(s_window, animated);
}

static void prv_deinit(void) {
  tick_timer_service_unsubscribe();
  window_destroy(s_window);
}

int main(void) {
  prv_init();
  app_event_loop();
  prv_deinit();
}
