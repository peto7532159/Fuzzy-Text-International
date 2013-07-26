#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"

#include "num2words-en.h"

#define DEBUG 0

#define NUM_LINES 3
#define LINE_LENGTH 7
#define BUFFER_SIZE 44
//(LINE_LENGTH + 1)
#define ROW_HEIGHT 37
#define TOP_MARGIN 10

#define ANIMATION_DURATION 400
#define ANIMATION_STAGGER_TIME 150

// We can add a new word to a line if there are at least this many characters free after
#define LINE_ADD_LIMIT (LINE_LENGTH - 2)

#if DEBUG
	#define WATCH_TITLE "SE Fuzzy Text Dbg" 
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
	char lineStr1[BUFFER_SIZE];
	char lineStr2[BUFFER_SIZE];
	PropertyAnimation animation1;
	PropertyAnimation animation2;
	TextLayer *currentLayer;
	TextLayer *nextLayer;
} Line;

Line lines[NUM_LINES];

PblTm t;

int currentMinutes;
int currentNLines;

// Animation handler
void animationStoppedHandler(struct Animation *animation, bool finished, void *context)
{
	TextLayer *current = (TextLayer *)context;
	GRect rect = layer_get_frame(&current->layer);
	rect.origin.x = 144;
	layer_set_frame(&current->layer, rect);
}

// Animate line
void makeAnimationsForLayer(Line *line)
{
	TextLayer *current = line->currentLayer;
	TextLayer *next = line->nextLayer;

	// Configure animation for current layer to move out
	GRect rect = layer_get_frame(&current->layer);
	rect.origin.x =  -144;	
	property_animation_init_layer_frame(&line->animation1, &current->layer, NULL, &rect);
	animation_set_duration(&line->animation1.animation, ANIMATION_DURATION);
	animation_set_delay(&line->animation1.animation, 0);
	animation_set_curve(&line->animation1.animation, AnimationCurveEaseIn); // Accelerate

	// Configure animation for current layer to move in
	GRect rect2 = layer_get_frame(&next->layer);
	rect2.origin.x = 0;
	property_animation_init_layer_frame(&line->animation2, &next->layer, NULL, &rect2);
	animation_set_duration(&line->animation2.animation, ANIMATION_DURATION);
	animation_set_delay(&line->animation2.animation, 100);
	animation_set_curve(&line->animation2.animation, AnimationCurveEaseOut); // Deaccelerate

	// Set a handler to rearrange layers after animation is finished
	animation_set_handlers(&line->animation2.animation, (AnimationHandlers) {
		.stopped = (AnimationStoppedHandler)animationStoppedHandler
	}, current);

	// Start the animations
	animation_schedule(&line->animation1.animation);
	animation_schedule(&line->animation2.animation);	
}

void updateLayerText(TextLayer* layer, char* text)
{
	const char* layerText = text_layer_get_text(layer);
	strcpy((char*)layerText, text);
	// To mark layer dirty
	text_layer_set_text(layer, layerText);
    //layer_mark_dirty(&layer->layer);
}

// Update line
void updateLineTo(Line *line, char *value)
{
	updateLayerText(line->nextLayer, value);
	makeAnimationsForLayer(line);

	// Swap current/next layers
	TextLayer *tmp = line->nextLayer;
	line->nextLayer = line->currentLayer;
	line->currentLayer = tmp;
}

// Check to see if the current line needs to be updated
bool needToUpdateLine(Line *line, char *nextValue)
{
	const char *currentStr = text_layer_get_text(line->currentLayer);

	if (strcmp(currentStr, nextValue) != 0) {
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

// Configure the layers for the given text
int configureLayersForText(char text[NUM_LINES][BUFFER_SIZE])
{
	int numLines = 0;

	// Set bold layer. TODO make dynamic
	if (strlen(text[2]) > 0) {
		numLines = 3;
		configureLightLayer(lines[0].nextLayer);
		configureLightLayer(lines[1].nextLayer);
		configureBoldLayer(lines[2].nextLayer);
	}
	else if (strlen(text[1]) > 0) {
		numLines = 2;
		configureLightLayer(lines[0].nextLayer);
		configureBoldLayer(lines[1].nextLayer);
	} else
	{
		numLines = 1;
		configureBoldLayer(lines[0].nextLayer);
	}

	// Calculate y position of top Line
	int ypos = (168 - numLines * ROW_HEIGHT) / 2 - TOP_MARGIN;

	// Set y positions for the lines
	for (int i = 0; i < numLines; i++)
	{
		layer_set_frame(&lines[i].nextLayer->layer, GRect(144, ypos, 144, 50));
		ypos += ROW_HEIGHT;
	}

	return numLines;
}

void time_to_lines(int hours, int minutes, char lines[NUM_LINES][BUFFER_SIZE])
{
	int length = NUM_LINES * BUFFER_SIZE + 1;
	char timeStr[length];
	time_to_words(hours, minutes, timeStr, length);
	
	// Empty all lines
	for (int i = 0; i < NUM_LINES; i++)
	{
		lines[i][0] = '\0';
	}

	char *start = timeStr;
	char *end = strstr(start, " ");
	int w = 0;
	while (end != NULL && w < NUM_LINES) {
		// Can we add a word to the first line?
		if (w == 0 && end - start < LINE_ADD_LIMIT - 1)
		{
			char *try = strstr(end + 1, " ");
			if (try != NULL && try - start <= LINE_ADD_LIMIT)
			{
				end = try;
			}
		}

		*end = '\0';
		strcpy(lines[w++], start);
		start = end + 1;
		end = strstr(start, " ");
	}
	
}

// Update screen based on new time
void display_time(PblTm *t)
{
	// The current time text will be stored in the following strings
	char textLine[NUM_LINES][BUFFER_SIZE];
	
	time_to_lines(t->tm_hour, t->tm_min, textLine);
	
	currentNLines = configureLayersForText(textLine);

	if (needToUpdateLine(&lines[0], textLine[0])) {
		updateLineTo(&lines[0], textLine[0]);	
	}
	psleep(ANIMATION_STAGGER_TIME);
	if (needToUpdateLine(&lines[1], textLine[1])) {
		updateLineTo(&lines[1], textLine[1]);	
	}
	psleep(ANIMATION_STAGGER_TIME);
	if (needToUpdateLine(&lines[2], textLine[2])) {
		updateLineTo(&lines[2], textLine[2]);	
	}
}

void initLineForStart(Line* line)
{
	// Switch current and next layer
	TextLayer* tmp  = line->currentLayer;
	line->currentLayer = line->nextLayer;
	line->nextLayer = tmp;

	// Move current layer to screen;
	GRect rect = layer_get_frame(&line->currentLayer->layer);
	rect.origin.x = 0;
	layer_set_frame(&line->currentLayer->layer, rect);
}

// Update screen without animation first time we start the watchface
void display_initial_time(PblTm *t)
{
	// The current time text will be stored in the following strings
	char textLine[NUM_LINES][BUFFER_SIZE];

	time_to_lines(t->tm_hour, t->tm_min, textLine);

	// This configures the nextLayer for each line
	currentNLines = configureLayersForText(textLine);

	// Set the text and configure layers to the start position
	for (int i = 0; i < currentNLines; i++)
	{
		updateLayerText(lines[i].nextLayer, textLine[i]);
		// This call switches current- and nextLayer
		initLineForStart(&lines[i]);
	}	
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

void init_line(Line* line)
{
	// Set current- and nextLayer pointers 
	line->currentLayer = &line->layer1;
	line->nextLayer = &line->layer2;

	// Init to dummy position to the right of the screen
	text_layer_init(line->currentLayer, GRect(144, 0, 144, 50));
	text_layer_init(line->nextLayer, GRect(144, 0, 144, 50));

	// Configure a style
	configureLightLayer(line->currentLayer);
	configureLightLayer(line->nextLayer);

	// Set the text buffers
	line->lineStr1[0] = '\0';
	line->lineStr2[0] = '\0';
	text_layer_set_text(&line->layer1, line->lineStr1);
	text_layer_set_text(&line->layer2, line->lineStr2);
}

void handle_init(AppContextRef ctx) {
  	(void)ctx;

	window_init(&window, "TextWatch");
	window_stack_push(&window, true);
	window_set_background_color(&window, GColorBlack);

	// Init resources
	resource_init_current_app(&APP_RESOURCES);
	
	// Init and load lines
	for (int i = 0; i < NUM_LINES; i++)
	{
		init_line(&lines[i]);
	  	layer_add_child(&window.layer, &lines[i].layer1.layer);
		layer_add_child(&window.layer, &lines[i].layer2.layer);
	}

	// Configure time on init
	get_time(&t);
	display_initial_time(&t);


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
    .init_handler = &handle_init
#if !DEBUG
	,
	.tick_info = {
		      .tick_handler = &handle_minute_tick,
		      .tick_units = MINUTE_UNIT
		    }
#endif
  };
  app_event_loop(params, &handlers);
}
