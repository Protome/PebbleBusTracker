#include <pebble.h>

#define KEY_BACKGROUND_COLOUR 0
#define KEY_TEXT_COLOUR 1
#define KEY_TOP_COLOUR 2
#define KEY_LEFT_COLOUR 3
#define KEY_RIGHT_COLOUR 4
#define KEY_BOTTOM_COLOUR 5
#define KEY_DISCONNECT_ALERT 6

#define DEFAULT_BACKGROUND_COLOUR GColorBlack
#define DEFAULT_TEXT_COLOUR GColorWhite
#define DEFAULT_TOP_COLOUR GColorCyan
#define DEFAULT_BOTTOM_COLOUR GColorOrange
#define DEFAULT_LEFT_COLOUR GColorPurple
#define DEFAULT_RIGHT_COLOUR GColorRed
#define DISCONNECTED_COLOUR GColorLightGray

typedef enum  {
  TOP,
  BOTTOM,
  RIGHT,
  LEFT  
} BAR_POSITION;

typedef struct {
  BAR_POSITION position;
} ColourData;

static Window *s_main_window;
static TextLayer *s_time_layer;
static Layer *s_top_layer, *s_right_layer, *s_left_layer, *s_bottom_layer;

static GColor8 background_colour;
static GColor8 text_colour;


static GFont s_time_font;
static bool isConnected = true;

static void update_time() {
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);
  static char buffer[] = "00:00";

  if(clock_is_24h_style() == true) {
    strftime(buffer, sizeof("00:00"), "%H:%M", tick_time);
  } else {
    strftime(buffer, sizeof("00:00"), "%I:%M", tick_time);
  }

  text_layer_set_text(s_time_layer, buffer);
}

static void inbox_received_handler(DictionaryIterator *iter, void *context) {
  Tuple *background_color_t = dict_find(iter, KEY_BACKGROUND_COLOUR);
  Tuple *top_color_t = dict_find(iter, KEY_TOP_COLOUR);
  Tuple *bottom_color_t = dict_find(iter, KEY_BOTTOM_COLOUR);
  Tuple *right_color_t = dict_find(iter, KEY_RIGHT_COLOUR);
  Tuple *left_color_t = dict_find(iter, KEY_LEFT_COLOUR);
  Tuple *text_colour_t = dict_find(iter, KEY_TEXT_COLOUR);
  Tuple *disconnect_alert_t = dict_find(iter, KEY_DISCONNECT_ALERT);
  
  if (background_color_t) {
    int colour = background_color_t->value->int32;
    persist_write_int(KEY_BACKGROUND_COLOUR, colour);
    background_colour = GColorFromHEX(colour);
    window_set_background_color(s_main_window, background_colour);
  }
  
  if (top_color_t) {
    int colour = top_color_t->value->int32;
    //Hacky fix but persist memory stores int values as 0 by default. So when the code is 0 (black) the value gets ignored when reading later.
    if (colour == 0) {
      colour = 1;
    }
    persist_write_int(KEY_TOP_COLOUR, colour);
    layer_mark_dirty(s_top_layer);
  }
  
  if (bottom_color_t) {
    int colour = bottom_color_t->value->int32;
    if (colour == 0) {
      colour = 1;
    }
    persist_write_int(KEY_BOTTOM_COLOUR, colour);
    layer_mark_dirty(s_bottom_layer);
  }
  
  if (right_color_t) {
    int colour = right_color_t->value->int32;
    if (colour == 0) {
      colour = 1;
    }
    persist_write_int(KEY_RIGHT_COLOUR, colour);
    layer_mark_dirty(s_right_layer);
  }
  
  if (left_color_t) {
    int colour = left_color_t->value->int32;
    if (colour == 0) {
      colour = 1;
    }
    persist_write_int(KEY_LEFT_COLOUR, colour);
    layer_mark_dirty(s_left_layer);
  }
  
  if(text_colour_t) {
    int colour = text_colour_t->value->int32;
    if (colour == 0) {
      colour = 1;
    }
    persist_write_int(KEY_TEXT_COLOUR, colour);
    text_colour = GColorFromHEX(colour);
    text_layer_set_text_color(s_time_layer, text_colour);
  }
  
  if (disconnect_alert_t) {
    if (disconnect_alert_t->value->int32 > 0) {
      persist_write_bool(KEY_DISCONNECT_ALERT, true);
    }
    else {
      persist_write_bool(KEY_DISCONNECT_ALERT, false);
    }
  }
}

static void bluetooth_callback(bool connected) {
  if (persist_read_bool(KEY_DISCONNECT_ALERT)) {
  if (isConnected != connected) {
    isConnected = connected;
    vibes_double_pulse();
  }

  //Marking one layer as dirty seems to redraw all of them.
  //That probably means I messed something up somewhere.
  layer_mark_dirty(s_top_layer);
  }
}

static GColor8 get_section_colour(BAR_POSITION position) {
  switch(position) {
    case TOP: 
      if(persist_read_int(KEY_TOP_COLOUR)) {
        int color_hex = persist_read_int(KEY_TOP_COLOUR);
        APP_LOG(APP_LOG_LEVEL_DEBUG, "The top colour is: %i", color_hex);
        return GColorFromHEX(color_hex);
      }
      else {
        APP_LOG(APP_LOG_LEVEL_DEBUG, "Using default colour");
        return DEFAULT_TOP_COLOUR;
      }
    case BOTTOM:
      if(persist_read_int(KEY_BOTTOM_COLOUR)) {
        int color_hex = persist_read_int(KEY_BOTTOM_COLOUR);
        return GColorFromHEX(color_hex);
      }
      else {
        return DEFAULT_BOTTOM_COLOUR;
      }
    case RIGHT:
      if(persist_read_int(KEY_RIGHT_COLOUR)) {
        int color_hex = persist_read_int(KEY_RIGHT_COLOUR);
        return GColorFromHEX(color_hex);
      }
      else {
        return DEFAULT_RIGHT_COLOUR;
      }
    case LEFT:
      if(persist_read_int(KEY_LEFT_COLOUR)) {
        int color_hex = persist_read_int(KEY_LEFT_COLOUR);
        return GColorFromHEX(color_hex);
      }
      else {
        return DEFAULT_LEFT_COLOUR;
      }
  }
  
  return DISCONNECTED_COLOUR;
}

static void update_bar_proc(Layer *barLayer, GContext *ctx) {
  GRect bounds = layer_get_bounds(barLayer);
  
  if (isConnected) {
  ColourData *colour_ctx = (ColourData*)layer_get_data(barLayer);
  BAR_POSITION barPosition = colour_ctx -> position;
  GColor8 colour = get_section_colour(barPosition);
    
  graphics_context_set_fill_color(ctx, colour);
  }
  else {
    graphics_context_set_fill_color(ctx, DISCONNECTED_COLOUR);
  }
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);
}

static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  
  if (persist_read_int(KEY_BACKGROUND_COLOUR)) {
   int color_hex = persist_read_int(KEY_BACKGROUND_COLOUR);
    background_colour = GColorFromHEX(color_hex);
  }
  else {
    background_colour = GColorBlack;
  }
  window_set_background_color(window, background_colour);
  
  //Time Text Creation
  s_time_layer = text_layer_create(GRect(0, 58, bounds.size.w, 50));
  text_layer_set_background_color(s_time_layer, GColorClear);
  
  if (persist_read_int(KEY_TEXT_COLOUR)) {
    int color_hex = persist_read_int(KEY_TEXT_COLOUR);
    text_colour = GColorFromHEX(color_hex);
  }
  else {
    text_colour = GColorWhite;
  }
  
  text_layer_set_text_color(s_time_layer, text_colour);
  text_layer_set_text(s_time_layer, "00:00");
  
  s_time_font = fonts_get_system_font(FONT_KEY_LECO_32_BOLD_NUMBERS);
  
  text_layer_set_font(s_time_layer, s_time_font);
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);

  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
  
  //Border rectangles creation
  int16_t  horizontalSize = bounds.size.w/6;
  int16_t  verticalSize = bounds.size.h/6;
  
  s_top_layer = layer_create_with_data(GRect(0,0, bounds.size.w, verticalSize), sizeof(ColourData));
  ColourData *top_colour_data = (ColourData*)layer_get_data(s_top_layer);
  top_colour_data -> position = TOP;
  layer_set_update_proc(s_top_layer, update_bar_proc);
    
  s_right_layer = layer_create_with_data(GRect(bounds.size.w - horizontalSize, 0, horizontalSize, bounds.size.h), sizeof(ColourData));
  ColourData *right_colour_data = (ColourData*)layer_get_data(s_right_layer);
  right_colour_data -> position = RIGHT;
  layer_set_update_proc(s_right_layer, update_bar_proc);
  
  s_left_layer = layer_create_with_data(GRect(0,0, horizontalSize, bounds.size.h), sizeof(ColourData));
  ColourData *left_colour = (ColourData*)layer_get_data(s_left_layer);
  left_colour -> position = LEFT;
  layer_set_update_proc(s_left_layer, update_bar_proc);
  
  s_bottom_layer = layer_create_with_data(GRect(horizontalSize, bounds.size.h - verticalSize, bounds.size.w - horizontalSize, verticalSize), sizeof(ColourData));
  ColourData *bottom_colour = (ColourData*)layer_get_data(s_bottom_layer);
  bottom_colour -> position = BOTTOM;
  layer_set_update_proc(s_bottom_layer, update_bar_proc);  
  
  layer_add_child(window_layer, s_left_layer);
  layer_add_child(window_layer, s_top_layer);
  layer_add_child(window_layer, s_right_layer);
  layer_add_child(window_layer, s_bottom_layer); 
  
  update_time();
  bluetooth_callback(connection_service_peek_pebble_app_connection());
}

static void main_window_unload(Window *window) {
  text_layer_destroy(s_time_layer);
  
  layer_destroy(s_top_layer);
  layer_destroy(s_bottom_layer);
  layer_destroy(s_right_layer);
  layer_destroy(s_left_layer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
}

static void init() {
  // Create main Window element and assign to pointer
  s_main_window = window_create();

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);
  
  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  
  //Register with app messages
  app_message_register_inbox_received(inbox_received_handler);
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
  
  //Register with bluetooth connection updates
  connection_service_subscribe((ConnectionHandlers) {
    .pebble_app_connection_handler = bluetooth_callback
  });
}

static void deinit() {
  // Destroy Window
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
