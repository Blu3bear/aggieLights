#include <Adafruit_NeoPixel.h>

#define PIN_NEO_PIXEL 16  // The ESP32 pin GPIO16 connected to NeoPixel
#define NUM_PIXELS 30     // The number of LEDs (pixels) on NeoPixel LED strip
#define SOLID_WHITE 0     // The state value corresponding to solid white
#define SOLID_BLUE 1      // The state value corresponding to solid blue
#define ALTERNATING 2     // The state value corresponding to alternating blue/white
#define RUNNING 3         // The state value corresponding to blue running up
#define CUSTOM 4          // The state value corresponding to a custom color map(WIP) 

Adafruit_NeoPixel NeoPixel(NUM_PIXELS, PIN_NEO_PIXEL, NEO_WRGB + NEO_KHZ800);

uint32_t BLUE = NeoPixel.Color( 40, 141, 194);
uint32_t WHITE = NeoPixel.Color(0,0,0,255);

uint8_t state = 0;
uint32_t map[NUM_PIXELS] = {}

void setup() {
  NeoPixel.begin();  // initialize NeoPixel strip
  state = 0;
}

void loop() {
  // Check the state, then run the appropriate
  switch (state) {
  case SOLID_WHITE:
    // code block
    break;
  case SOLID_BLUE:
    // code block
    break;
  default:
    // code block
}
}
