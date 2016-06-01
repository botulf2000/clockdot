#include <pebble.h>
#include <math.h>

static Window *s_main_window;
static TextLayer *s_time_layer;

static BitmapLayer *s_background_layer;
static GBitmap *s_background_bitmap;

static GFont s_time_font;
static GFont s_date_font;

static Layer *s_canvas_layer;
static int minutes, hours;
static struct tm ts;

static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);
  ts=*localtime(&temp);
  minutes=ts.tm_min;
  //minutes=(minutes+15)%60;
  hours=ts.tm_hour;
  
  
  // Write the current hours and minutes into a buffer
  static char s_buffer[3];
  snprintf(s_buffer,3, "%d", hours);
  // Display this time on the TextLayer
  text_layer_set_text(s_time_layer, s_buffer);
  
   
  layer_mark_dirty(s_canvas_layer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
}


static void canvas_update_proc(Layer *layer, GContext *ctx) {
  // Custom drawing happens here!
  
  Layer *window_layer = window_get_root_layer(s_main_window);
  GRect bounds = layer_get_bounds(window_layer);
  
  int x,y;
  int midpoint=bounds.size.w/2;
  int adjusted_minutes=90-((minutes+2)/5)*5*6;
  //printf("Mid: %d Minutes: %d", midpoint, adjusted_minutes);
  x=midpoint+cos(3.14*adjusted_minutes/180)*(3*midpoint/4);
  y=midpoint-sin(3.14*adjusted_minutes/180)*(3*midpoint/4);
  printf("minutes: %d x: %d y:%d",adjusted_minutes, x,y);
  GPoint center = GPoint(x, y);
  uint16_t radius = midpoint/4;

  // Draw the outline of a circle
  graphics_context_set_stroke_color(ctx, GColorDukeBlue);
  graphics_draw_circle(ctx, center, radius);

  // Fill a circle
  graphics_context_set_fill_color(ctx, GColorCadetBlue);
  graphics_fill_circle(ctx, center, radius);

}


static void main_window_load(Window *window) {
  // Get information about the Window
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  // Create GBitmap
//  s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BILD);

  // Create BitmapLayer to display the GBitmap
  //s_background_layer = bitmap_layer_create(bounds);

  // Set the bitmap onto the layer and add to the window
  //bitmap_layer_set_bitmap(s_background_layer, s_background_bitmap);
  //layer_add_child(window_layer, bitmap_layer_get_layer(s_background_layer));
  
  //Canvas for drawing
//  GRect bounds = layer_get_bounds(window_get_root_layer(window));

  // Create canvas layer
  s_canvas_layer = layer_create(bounds);
  
  
  // Assign the custom drawing procedure
  layer_set_update_proc(s_canvas_layer, canvas_update_proc);

  // Add to Window
  layer_add_child(window_get_root_layer(window), s_canvas_layer);

  // Create the TextLayer with specific bounds
  s_time_layer = text_layer_create(
      GRect(0, PBL_IF_ROUND_ELSE(58, 52), bounds.size.w, 50));

  // Improve the layout to be more like a watchface
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorBlack);
  text_layer_set_text(s_time_layer, "00:00");
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);

  // Create GFont
  s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_VIPER_SOLID_40));

  // Apply to TextLayer
  text_layer_set_font(s_time_layer, s_time_font);

  // Add it as a child layer to the Window's root layer
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
  
  // Create the TextLayer with specific bounds
}

static void main_window_unload(Window *window) {
  // Destroy TextLayer
  text_layer_destroy(s_time_layer);

  // Unload GFont
  fonts_unload_custom_font(s_time_font);

  // Destroy GBitmap
  gbitmap_destroy(s_background_bitmap);

  // Destroy BitmapLayer
  bitmap_layer_destroy(s_background_layer);
}


static void init() {
  // Create main Window element and assign to pointer
  s_main_window = window_create();

  // Set the background color
  window_set_background_color(s_main_window, GColorWhite);

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);

  // Make sure the time is displayed from the start
  update_time();

  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  minutes=0;
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