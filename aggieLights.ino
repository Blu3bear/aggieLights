#include <Adafruit_NeoPixel.h>
#include <math.h>
#include <Preferences.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>

#define BTN_PIN 27
#define PIN_NEO_PIXEL 14 // The ESP32 pin GPIO16 connected to NeoPixel
#define NUM_PIXELS 48    // The number of LEDs (pixels) on NeoPixel LED strip
#define DARK 0           // The state where the leds are off
#define SOLID_WHITE 1    // The state value corresponding to solid white
#define SOLID_BLUE 2     // The state value corresponding to solid blue
#define ALTERNATING 3    // The state value corresponding to alternating blue/white
#define RUNNING 4        // The state value corresponding to blue running up
#define CUSTOM 5         // The state value corresponding to a custom color map(WIP)

// Library objects
Adafruit_NeoPixel NeoPixel(NUM_PIXELS, PIN_NEO_PIXEL, NEO_GRBW + NEO_KHZ800);
Preferences preferences;
AsyncWebServer server(80);

// WiFi credentials
const char* ssid = "Aggie_Light";
const char* password = "usu1888";

uint32_t BLUE = NeoPixel.Color(1, 173, 216, 0);
uint32_t WHITE = NeoPixel.Color(0, 0, 0, 255);

uint8_t state = 0;
uint8_t debounce = 0;
uint16_t running_cnt = 0;
uint32_t base_color;
uint32_t sec_color;

// Button timing variables
unsigned long press_start = 0;
bool button_handled = false;
bool web_server_enabled = false;
const unsigned long LONG_PRESS_MS = 3000;
const uint8_t DEBOUNCE_THRESHOLD = 50;

// Function prototype
uint32_t rgbw_lin_interp(uint32_t c1, uint32_t c2, uint32_t steps = 255);

void setup()
{
  pinMode(BTN_PIN, INPUT);
  NeoPixel.begin();                   // initialize NeoPixel strip
  preferences.begin("colors", false); // initialize preferences
    // Initialize WiFi in access point mode
  WiFi.softAP(ssid, password);
  Serial.begin(115200);
  
  base_color = preferences.getUInt("base", WHITE);
  sec_color = preferences.getUInt("sec", BLUE);
  state = 0;
}

void loop()
{
  // Declare variables used in switch cases
  uint8_t steps;
  uint32_t color;

  // Check the state, then run the appropriate
  switch (state)
  {
  case DARK:
    NeoPixel.clear();
    break;
  case SOLID_WHITE:
    // Use the neopixel fill function to set all white(color is in the form 0xggrrbbww)
    NeoPixel.fill(base_color);
    break;
  case SOLID_BLUE:
    // Fill with electric blue(pulled from the usu brand toolkit)
    NeoPixel.fill(sec_color);
    break;
  case ALTERNATING:
    // Use the sine8 function to give a smooth transition between two colors
    steps = NeoPixel.sine8(running_cnt % 256);
    if (base_color < sec_color)
    {
      color = base_color + rgbw_lin_interp(base_color, sec_color) * steps;
    }
    else
    {
      color = sec_color + rgbw_lin_interp(sec_color, base_color) * steps;
    }
    NeoPixel.fill(color);
    break;
  case RUNNING:
    // Fill the whole strip white
    NeoPixel.fill(base_color);
    // Set a single blue pixel to blue to create a running effect.
    NeoPixel.setPixelColor((running_cnt / 127) % NUM_PIXELS, sec_color);
    break;
  default:
    // fill with red if something went wrong.
    NeoPixel.fill(0xff000000);
    break;
  }
  NeoPixel.show();

  running_cnt++;
  // don't let running_cnt overflow
  if (running_cnt == 0xffff)
  {
    running_cnt = 0;
  }

  // Debounce button press and increment the state
  int btn_val = digitalRead(BTN_PIN);

  if (btn_val == HIGH)
  {
    if (debounce < DEBOUNCE_THRESHOLD)
    {
      debounce++;
    }
    else if (press_start == 0)
    {
      // Button just became stable HIGH, record start time
      press_start = millis();
      button_handled = false;
    }
    else if (!button_handled && (millis() - press_start >= LONG_PRESS_MS))
    {
      // Long press detected while still holding
      button_handled = true;
      web_server_enabled = !web_server_enabled;

      // Flash yellow to indicate toggle
      for (int i = 0; i < 3; i++)
      {
        NeoPixel.fill(NeoPixel.Color(255, 255, 0, 0)); // Yellow
        NeoPixel.show();
        delay(200);
        NeoPixel.clear();
        NeoPixel.show();
        delay(200);
      }
    }
  }
  else if (btn_val == LOW && press_start > 0)
  {
    // Button released
    if (!button_handled && debounce >= DEBOUNCE_THRESHOLD)
    {
      // Short press - advance state
      running_cnt = 0;
      state = (state + 1) % 5;
    }
    // Reset for next press
    debounce = 0;
    press_start = 0;
    button_handled = false;
  }
}

uint32_t rgbw_lin_interp(uint32_t c1, uint32_t c2, uint32_t steps)
{
  // linear interpolation for rgbw values
  // arguments:
  //    c1 - a packed color code in the form 0xgrbw
  //    c2 - a second packed color code
  //    steps - the number of steps between c1 and c2(default 255)
  // returns:
  //    uint32_t representing the offset to go from the higher of the two to the other in steps
  uint32_t dif = 0;
  uint32_t begin, end;
  if (c1 < c2)
  {
    begin = c1;
    end = c2;
  }
  else
  {
    begin = c2;
    end = c1;
  }

  // dif_1 = begin;

  // TODO: extract from difference in w from c1 and c2
  //        extract the difference in r from c1 and c2
  //        extract the difference in g from c1 and c2
  //        extract the difference in b from c1 and c2

  // TODO: repack the differnces as a single value

  return dif;
}