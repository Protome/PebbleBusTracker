#include <pebble.h>

#define KEY_BACKGROUND_COLOUR 0
#define TOP_COLOUR GColorCyan
#define BOTTOM_COLOUR GColorOrange
#define LEFT_COLOUR GColorPurple
#define RIGHT_COLOUR GColorRed
#define DISCONNECTED_COLOUR GColorLightGray

typedef struct {
  GColor8 colour;
} ColourData;

static Window *s_main_window;
static TextLayer *s_time_layer;
static Layer *s_top_layer, *s_right_layer, *s_left_layer, *s_bottom_layer;

static GColor8 background_colour;
static GFont s_time_font;
static bool isConnected = false;

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
    
  if (background_color_t) {
    int colour = background_color_t->value->int32;
    APP_LOG(APP_LOG_LEVEL_DEBUG, "background colour got!");
    persist_write_int(KEY_BACKGROUND_COLOUR, colour);
    background_colour = GColorFromHEX(colour);
    window_set_background_color(s_main_window, background_colour);
  }
}

static void bluetooth_callback(bool connected) {
  isConnected = connected;
  vibes_double_pulse();

  //Marking one layer as dirty seems to redraw all of them.
  //That probably means I messed something up somewhere.
  layer_mark_dirty(s_top_layer);
}

static void update_bar_proc(Layer *barLayer, GContext *ctx) {
  GRect bounds = layer_get_bounds(barLayer);
  
  if (isConnected) {
  ColourData *colour_ctx = (ColourData*)layer_get_data(barLayer);
  GColor8 background_colour = colour_ctx -> colour;
  graphics_context_set_fill_color(ctx, background_colour);
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
  text_layer_set_text_color(s_time_layer, GColorWhite);
  text_layer_set_text(s_time_layer, "00:00");
  
  s_time_font = fonts_get_system_font(FONT_KEY_LECO_32_BOLD_NUMBERS);
  
  text_layer_set_font(s_time_layer, s_time_font);
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);

  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
  
  //Border rectangles creation
  int16_t  horizontalSize = bounds.size.w/6;
  int16_t  verticalSize = bounds.size.h/6;
  
  s_top_layer = layer_create_with_data(GRect(0,0, bounds.size.w, verticalSize), sizeof(ColourData));
  ColourData *top_colour = (ColourData*)layer_get_data(s_top_layer);
  top_colour ->colour = TOP_COLOUR;
  layer_set_update_proc(s_top_layer, update_bar_proc);
    
  s_right_layer = layer_create_with_data(GRect(bounds.size.w - horizontalSize, 0, horizontalSize, bounds.size.h), sizeof(ColourData));
  ColourData *right_colour = (ColourData*)layer_get_data(s_right_layer);
  right_colour ->colour = RIGHT_COLOUR;
  layer_set_update_proc(s_right_layer, update_bar_proc);
  
  s_left_layer = layer_create_with_data(GRect(0,0, horizontalSize, bounds.size.h), sizeof(ColourData));
  ColourData *left_colour = (ColourData*)layer_get_data(s_left_layer);
  left_colour ->colour = LEFT_COLOUR;
  layer_set_update_proc(s_left_layer, update_bar_proc);
  
  s_bottom_layer = layer_create_with_data(GRect(horizontalSize, bounds.size.h - verticalSize, bounds.size.w - horizontalSize, verticalSize), sizeof(ColourData));
  ColourData *bottom_colour = (ColourData*)layer_get_data(s_bottom_layer);
  bottom_colour ->colour = BOTTOM_COLOUR;
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
