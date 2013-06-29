#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"

#include "num2words-en.h"

#define DEBUG 1
#define BUFFER_SIZE 44

#if DEBUG
	#define WATCH_TITLE "SE Fuzzy Text Db" 
#else
	#define WATCH_TITLE "SE Fuzzy Text " 
#endif

#define MY_UUID { 0x46, 0x6E, 0x04, 0xAD, 0x13, 0x2A, 0x48, 0xAB, 0xB1, 0x65, 0x7F, 0xF4, 0xA9, 0x98, 0x72, 0xD2 }
PBL_APP_INFO(MY_UUID,
             WATCH_TITLE, "Wip Interactive",
             1, 0,
             DEFAULT_MENU_ICON,
#if DEBUG
             APP_INFO_STANDARD_APP
#else
			 APP_INFO_WATCH_FACE
#endif
);

Window window;

typedef struct {
	TextLayer layer1;
	TextLayer layer2;	
	PropertyAnimation animation1;
	PropertyAnimation animation2;
	TextLayer *currentLayer;
	TextLayer *nextLayer;
} Line;

Line line1;
Line line2;
Line line3;

PblTm t;

static char line1Str[2][BUFFER_SIZE];
static char line2Str[2][BUFFER_SIZE];
static char line3Str[2][BUFFER_SIZE];

static bool textInitialized = false;

// Animation handler
void animationStoppedHandler(struct Animation *animation, bool finished, void *context)
{
	TextLayer *current = (TextLayer *)context;
	GRect rect = layer_get_frame(&current->layer);
	rect.origin.x = 144;
	layer_set_frame(&current->layer, rect);
}

// Animate line
void makeAnimationsForLayer(Line *line, TextLayer *current, TextLayer *next)
{
	GRect rect = layer_get_frame(&next->layer);
	rect.origin.x -= 144;
	
	property_animation_init_layer_frame(&line->animation2, &next->layer, NULL, &rect);
	animation_set_duration(&line->animation2.animation, 400);
	animation_set_curve(&line->animation2.animation, AnimationCurveEaseOut);
	animation_schedule(&line->animation2.animation);
	
	GRect rect2 = layer_get_frame(&current->layer);
	rect2.origin.x -= 144;
	
	property_animation_init_layer_frame(&line->animation1, &current->layer, NULL, &rect2);
	animation_set_duration(&line->animation1.animation, 400);
	animation_set_curve(&line->animation1.animation, AnimationCurveEaseOut);
	
	animation_set_handlers(&line->animation1.animation, (AnimationHandlers) {
		.stopped = (AnimationStoppedHandler)animationStoppedHandler
	}, current);
	
	animation_schedule(&line->animation1.animation);
}

// Update line
void updateLineTo(Line *line, char lineStr[2][BUFFER_SIZE], char *value)
{
	TextLayer *next, *current;
	
	GRect rect = layer_get_frame(&line->currentLayer->layer);
	current = (rect.origin.x == 0) ? line->currentLayer : line->nextLayer;
	next = (current == line->currentLayer) ? line->nextLayer : line->currentLayer;
	
	// Update correct text only
	if (current == line->currentLayer) {
		memset(lineStr[1], 0, BUFFER_SIZE);
		memcpy(lineStr[1], value, strlen(value));
		text_layer_set_text(next, lineStr[1]);
	} else {
		memset(lineStr[0], 0, BUFFER_SIZE);
		memcpy(lineStr[0], value, strlen(value));
		text_layer_set_text(next, lineStr[0]);
	}
	
	makeAnimationsForLayer(line, current, next);
}

// Check to see if the current line needs to be updated
bool needToUpdateLine(Line *line, char lineStr[2][BUFFER_SIZE], char *nextValue)
{
	char *currentStr;
	GRect rect = layer_get_frame(&line->currentLayer->layer);
	currentStr = (rect.origin.x == 0) ? lineStr[0] : lineStr[1];

	if (memcmp(currentStr, nextValue, strlen(nextValue)) != 0 ||
		(strlen(nextValue) == 0 && strlen(currentStr) != 0)) {
		return true;
	}
	return false;
}

// Configure bold line of text
void configureBoldLayer(TextLayer *textlayer)
{
	text_layer_set_font(textlayer, fonts_get_system_font(FONT_KEY_GOTHAM_42_BOLD));
	text_layer_set_text_color(textlayer, GColorWhite);
	text_layer_set_background_color(textlayer, GColorClear);
	text_layer_set_text_alignment(textlayer, GTextAlignmentCenter);
}

// Configure light line of text
void configureLightLayer(TextLayer *textlayer)
{
	text_layer_set_font(textlayer, fonts_get_system_font(FONT_KEY_GOTHAM_42_LIGHT));
	text_layer_set_text_color(textlayer, GColorWhite);
	text_layer_set_background_color(textlayer, GColorClear);
	text_layer_set_text_alignment(textlayer, GTextAlignmentCenter);
}

// Update screen based on new time
void display_time(PblTm *t)
{
	// The current time text will be stored in the following 3 strings
	char textLine1[BUFFER_SIZE];
	char textLine2[BUFFER_SIZE];
	char textLine3[BUFFER_SIZE];
	
	time_to_3words(t->tm_hour, t->tm_min, textLine1, textLine2, textLine3, BUFFER_SIZE);
	
	// Set bold layer.
	if (strlen(textLine3) > 0) {
		configureLightLayer(line1.nextLayer);
		configureLightLayer(line2.nextLayer);
		configureBoldLayer(line3.nextLayer);
	}
	else if (strlen(textLine2) > 0) {
		configureLightLayer(line1.nextLayer);
		configureBoldLayer(line2.nextLayer);
	} else
	{
		configureBoldLayer(line1.nextLayer);
	}

	if (needToUpdateLine(&line1, line1Str, textLine1)) {
		updateLineTo(&line1, line1Str, textLine1);	
	}
	if (needToUpdateLine(&line2, line2Str, textLine2)) {
		updateLineTo(&line2, line2Str, textLine2);	
	}
	if (needToUpdateLine(&line3, line3Str, textLine3)) {
		updateLineTo(&line3, line3Str, textLine3);	
	}
}

// Update screen without animation first time we start the watchface
void display_initial_time(PblTm *t)
{
	time_to_3words(t->tm_hour, t->tm_min, line1Str[0], line2Str[0], line3Str[0], BUFFER_SIZE);
	
	text_layer_set_text(line1.currentLayer, line1Str[0]);
	text_layer_set_text(line2.currentLayer, line2Str[0]);
	text_layer_set_text(line3.currentLayer, line3Str[0]);
}

/** 
 * Debug methods. For quickly debugging enable debug macro on top to transform the watchface into
 * a standard app and you will be able to change the time with the up and down buttons
 */ 
#if DEBUG

void up_single_click_handler(ClickRecognizerRef recognizer, Window *window) {
	(void)recognizer;
	(void)window;
	
	t.tm_min += 5;
	if (t.tm_min >= 60) {
		t.tm_min = 0;
		t.tm_hour += 1;
		
		if (t.tm_hour >= 24) {
			t.tm_hour = 0;
		}
	}
	display_time(&t);
}


void down_single_click_handler(ClickRecognizerRef recognizer, Window *window) {
	(void)recognizer;
	(void)window;
	
	t.tm_min -= 5;
	if (t.tm_min < 0) {
		t.tm_min = 55;
		t.tm_hour -= 1;
		
		if (t.tm_hour < 0) {
			t.tm_hour = 23;
		}
	}
	display_time(&t);
}

void click_config_provider(ClickConfig **config, Window *window) {
  (void)window;

  config[BUTTON_ID_UP]->click.handler = (ClickHandler) up_single_click_handler;
  config[BUTTON_ID_UP]->click.repeat_interval_ms = 100;

  config[BUTTON_ID_DOWN]->click.handler = (ClickHandler) down_single_click_handler;
  config[BUTTON_ID_DOWN]->click.repeat_interval_ms = 100;
}

#endif

void handle_init(AppContextRef ctx) {
  	(void)ctx;

	window_init(&window, "TextWatch");
	window_stack_push(&window, true);
	window_set_background_color(&window, GColorBlack);

	// Init resources
	resource_init_current_app(&APP_RESOURCES);
	
	// 1st line layers
	text_layer_init(&line1.layer1, GRect(0, 18, 144, 50));
	text_layer_init(&line1.layer2, GRect(144, 18, 144, 50));
	line1.currentLayer = &line1.layer1;
	line1.nextLayer = &line1.layer2;
	configureLightLayer(&line1.layer1);
	configureLightLayer(&line1.layer2);

	// 2nd layers
	text_layer_init(&line2.layer1, GRect(0, 55, 144, 50));
	text_layer_init(&line2.layer2, GRect(144, 55, 144, 50));
	line2.currentLayer = &line2.layer1;
	line2.nextLayer = &line2.layer2;
	configureLightLayer(&line2.layer1);
	configureLightLayer(&line2.layer2);

	// 3rd layers
	text_layer_init(&line3.layer1, GRect(0, 92, 144, 50));
	text_layer_init(&line3.layer2, GRect(144, 92, 144, 50));
	line3.currentLayer = &line3.layer1;
	line3.nextLayer = &line3.layer2;	configureBoldLayer(&line3.layer1);
	configureBoldLayer(&line3.layer2);

	// Configure time on init
	get_time(&t);
	display_initial_time(&t);
	
	// Load layers
  	layer_add_child(&window.layer, &line1.layer1.layer);
	layer_add_child(&window.layer, &line1.layer2.layer);
	layer_add_child(&window.layer, &line2.layer1.layer);
	layer_add_child(&window.layer, &line2.layer2.layer);
	layer_add_child(&window.layer, &line3.layer1.layer);
	layer_add_child(&window.layer, &line3.layer2.layer);
	
#if DEBUG
	// Button functionality
	window_set_click_config_provider(&window, (ClickConfigProvider) click_config_provider);
#endif
}

// Time handler called every minute by the system
void handle_minute_tick(AppContextRef ctx, PebbleTickEvent *t) {
  (void)ctx;

  display_time(t->tick_time);
}

void pbl_main(void *params) {
  PebbleAppHandlers handlers = {
    .init_handler = &handle_init,
	.tick_info = {
		      .tick_handler = &handle_minute_tick,
		      .tick_units = MINUTE_UNIT
		    }
  };
  app_event_loop(params, &handlers);
}
