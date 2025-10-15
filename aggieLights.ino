// on the offbrand leds i got some of the colors are mixed up from the neopixel library
// r is red still
// g is blue on our strip
// b is white on this strip
// w is green on this strip

#include <Adafruit_NeoPixel.h>

#define BTN_PIN 27
#define PIN_NEO_PIXEL 14  // The ESP32 pin GPIO16 connected to NeoPixel
#define NUM_PIXELS 60     // The number of LEDs (pixels) on NeoPixel LED strip
#define SOLID_WHITE 0     // The state value corresponding to solid white
#define SOLID_BLUE 1      // The state value corresponding to solid blue
#define ALTERNATING 2     // The state value corresponding to alternating blue/white
#define RUNNING 3         // The state value corresponding to blue running up
#define CUSTOM 4          // The state value corresponding to a custom color map(WIP) 

Adafruit_NeoPixel NeoPixel(NUM_PIXELS, PIN_NEO_PIXEL, NEO_WRGB + NEO_KHZ800);

uint32_t BLUE = 0xAD01D800;
uint32_t WHITE = NeoPixel.Color(0,0,255,0);

uint8_t state = 0;
uint8_t debounce = 0;
uint16_t running_cnt = 0;

void setup() {
  pinMode(BTN_PIN, INPUT);
  NeoPixel.begin();  // initialize NeoPixel strip
  state = 0;
}

void loop() {
  // Check the state, then run the appropriate
  switch (state) {
  case SOLID_WHITE:
    //Use the neopixel fill function to set all white(color is in the form 0xwwrrggbb)
    NeoPixel.fill(WHITE);
    break;
  case SOLID_BLUE:
    //Fill with electric blue(pulled from the usu brand toolkit)
    NeoPixel.fill(BLUE);
    break;
  case ALTERNATING:
    // Use the sine8 function to give a smooth transition between two colors
    uint8_t steps = NeoPixel.sine8(running_cnt%256);
    uint32_t color = WHITE + rgbw_lin_interp(WHITE,BLUE)*steps;
    NeoPixel.fill(color);
  break;
  case RUNNING:
    //Fill the whole strip white
    NeoPixel.fill(0x000000ff);
    //Set a single blue pixel to blue to create a running effect.
    NeoPixel.setPixelColor((running_cnt/127)%NUM_PIXELS,0xAD01D800);
    running_cnt++;
  break;
  default:
    //fill with red if something went wrong.
    NeoPixel.fill(0x00ff0000);
    break;
  }
  NeoPixel.show();


  // don't let running_cnt overflow
  if(running_cnt == 0xffff){
      running_cnt = 0;
  }
  
  // Debounce button press and increment the state
  int btn_val = digitalRead(BTN_PIN);

  if (btn_val == HIGH) {
    if (debounce < 255) {
      debounce++;
    }
  } else if (btn_val == LOW && debounce > 1) {
    debounce = 0;
    running_cnt = 0;
    state = (state + 1) % 5;
  } else {
    debounce = 0;
  }
}

uint32_t rgbw_lin_interp(uint32_t c1, uint32_t c2, uint32_t steps = 255){
  // linear interpolation for rgbw values
  // arguments:
  //    c1 - a packed color code in the form 0xwwrrggbb
  //    c2 - a second packed color code
  //    steps - the number of steps between c1 and c2(default 255)
  // returns:
  //    uint32_t representing the offset to go from c1 to c2 in steps
  int32_t dif = 0;
  // TODO: extract from difference in w from c1 and c2
  //        extract the difference in r from c1 and c2
  //        extract the difference in g from c1 and c2
  //        extract the difference in b from c1 and c2
  
  // TODO: repack the differnces as a single value

  return dif;
}