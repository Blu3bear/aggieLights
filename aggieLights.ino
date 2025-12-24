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
#define RAINBOW 5        // Rainbow state
#define CUSTOM 6         // The state value corresponding to a custom color map(WIP)

// Library objects
Adafruit_NeoPixel NeoPixel(NUM_PIXELS, PIN_NEO_PIXEL, NEO_GRBW + NEO_KHZ800);
Preferences preferences;
AsyncWebServer server(80);

// WiFi credentials
const char *ssid = "Aggie_Light";

uint32_t BLUE = NeoPixel.Color(1, 173, 216, 0);
uint32_t WHITE = NeoPixel.Color(0, 0, 0, 255);

uint8_t state = 0;
uint8_t debounce = 0;
uint16_t running_cnt = 0;
uint32_t base_color;
uint32_t sec_color;
uint8_t brightness;

// Button timing variables
unsigned long press_start = 0;
bool button_handled = false;
bool web_server_enabled = false;
const unsigned long LONG_PRESS_MS = 3000;
const uint8_t DEBOUNCE_THRESHOLD = 50;

// Function prototypes
uint32_t rgbw_lin_interp(uint32_t c1, uint32_t c2, uint32_t step, uint32_t num_steps = 255);
void setupServerRoutes();
void startServer();
void stopServer();

// HTML page for RGBW color control
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>Aggie Light Control</title>
<style>
body{font-family:Arial;max-width:400px;margin:50px auto;padding:20px;background:#0F2439;color:#fff}
h1{text-align:center;font-size:24px;margin:0 0 20px;color:#fff}
h2{font-size:18px;margin:20px 0 10px;color:#289BD8}
.slider{width:100%;margin:10px 0}
input[type=range]{width:100%;height:30px;-webkit-appearance:none;background:transparent}
input[type=range]::-webkit-slider-track{height:8px;background:#555;border-radius:4px}
input[type=range]::-webkit-slider-thumb{-webkit-appearance:none;width:24px;height:24px;border-radius:50%;cursor:pointer}
input[type=range]::-moz-range-track{height:8px;background:#555;border-radius:4px}
input[type=range]::-moz-range-thumb{width:24px;height:24px;border-radius:50%;border:0;cursor:pointer}
.r::-webkit-slider-thumb{background:#f00}
.r::-moz-range-thumb{background:#f00}
.g::-webkit-slider-thumb{background:#0f0}
.g::-moz-range-thumb{background:#0f0}
.b::-webkit-slider-thumb{background:#00f}
.b::-moz-range-thumb{background:#00f}
.w::-webkit-slider-thumb{background:#fff}
.w::-moz-range-thumb{background:#fff}
label{display:flex;justify-content:space-between;margin-bottom:5px;font-weight:bold}
.val{background:#333;border:1px solid #555;border-radius:4px;padding:2px 6px;color:#fff;width:35px;text-align:center}
input[type=number]::-webkit-inner-spin-button,input[type=number]::-webkit-outer-spin-button{-webkit-appearance:none;margin:0}
input[type=number]{-moz-appearance:textfield}
.preview{height:50px;border-radius:8px;margin:15px 0;border:2px solid #555}
button{width:100%;padding:12px;font-size:16px;background:#289BD8;color:#fff;border:0;border-radius:6px;cursor:pointer;margin-top:20px}
button:active{background:#1a7ab0}
.section{background:#1a3a5c;padding:15px;border-radius:8px;margin-bottom:15px}
</style>
</head>
<body>
<h1>Aggie Light Control</h1>

<div class="section">
<h2>Base Color</h2>
<div class="preview" id="preview1"></div>
<div class="slider">
<label>R: <input type="number" class="val" id="r1v" min="0" max="255" value="0" onchange="v('r1',this.value)"></label>
<input type="range" class="r" id="r1" min="0" max="255" value="0" oninput="u(1)">
</div>
<div class="slider">
<label>G: <input type="number" class="val" id="g1v" min="0" max="255" value="0" onchange="v('g1',this.value)"></label>
<input type="range" class="g" id="g1" min="0" max="255" value="0" oninput="u(1)">
</div>
<div class="slider">
<label>B: <input type="number" class="val" id="b1v" min="0" max="255" value="0" onchange="v('b1',this.value)"></label>
<input type="range" class="b" id="b1" min="0" max="255" value="0" oninput="u(1)">
</div>
<div class="slider">
<label>W: <input type="number" class="val" id="w1v" min="0" max="255" value="255" onchange="v('w1',this.value)"></label>
<input type="range" class="w" id="w1" min="0" max="255" value="255" oninput="u(1)">
</div>
</div>

<div class="section">
<h2>Secondary Color</h2>
<div class="preview" id="preview2"></div>
<div class="slider">
<label>R: <input type="number" class="val" id="r2v" min="0" max="255" value="1" onchange="v('r2',this.value)"></label>
<input type="range" class="r" id="r2" min="0" max="255" value="1" oninput="u(2)">
</div>
<div class="slider">
<label>G: <input type="number" class="val" id="g2v" min="0" max="255" value="173" onchange="v('g2',this.value)"></label>
<input type="range" class="g" id="g2" min="0" max="255" value="173" oninput="u(2)">
</div>
<div class="slider">
<label>B: <input type="number" class="val" id="b2v" min="0" max="255" value="216" onchange="v('b2',this.value)"></label>
<input type="range" class="b" id="b2" min="0" max="255" value="216" oninput="u(2)">
</div>
<div class="slider">
<label>W: <input type="number" class="val" id="w2v" min="0" max="255" value="0" onchange="v('w2',this.value)"></label>
<input type="range" class="w" id="w2" min="0" max="255" value="0" oninput="u(2)">
</div>
</div>

<div class="section">
<h2>Brightness</h2>
<div class="slider">
<label>Level: <input type="number" class="val" id="brv" min="0" max="255" value="255" onchange="v('br',this.value)"></label>
<input type="range" class="w" id="br" min="0" max="255" value="255" oninput="document.getElementById('brv').value=this.value">
</div>
</div>

<button onclick="s()">Save Settings</button>

<script>
function u(n){
let r=document.getElementById('r'+n).value;
let g=document.getElementById('g'+n).value;
let b=document.getElementById('b'+n).value;
let w=document.getElementById('w'+n).value;
document.getElementById('r'+n+'v').value=r;
document.getElementById('g'+n+'v').value=g;
document.getElementById('b'+n+'v').value=b;
document.getElementById('w'+n+'v').value=w;
let wr=Math.min(255,parseInt(r)+parseInt(w));
let wg=Math.min(255,parseInt(g)+parseInt(w));
let wb=Math.min(255,parseInt(b)+parseInt(w));
document.getElementById('preview'+n).style.background='rgb('+wr+','+wg+','+wb+')';
}
function v(id,val){
document.getElementById(id).value=val;
u(id.charAt(1));
}
function s(){
let r1=document.getElementById('r1').value;
let g1=document.getElementById('g1').value;
let b1=document.getElementById('b1').value;
let w1=document.getElementById('w1').value;
let r2=document.getElementById('r2').value;
let g2=document.getElementById('g2').value;
let b2=document.getElementById('b2').value;
let w2=document.getElementById('w2').value;
let br=document.getElementById('br').value;
let body='r1='+r1+'&g1='+g1+'&b1='+b1+'&w1='+w1+'&r2='+r2+'&g2='+g2+'&b2='+b2+'&w2='+w2+'&br='+br;
fetch('/setColors',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:body})
.then(r=>r.text()).then(t=>alert(t));
}
u(1);u(2);
</script>
</body>
</html>
)rawliteral";

void setup()
{
  pinMode(BTN_PIN, INPUT);
  NeoPixel.begin();                   // initialize NeoPixel strip
  preferences.begin("colors", false); // initialize preferences

  // Load saved colors from preferences
  base_color = preferences.getUInt("base", WHITE);
  sec_color = preferences.getUInt("sec", BLUE);
  brightness = preferences.getUChar("bright", 255);
  NeoPixel.setBrightness(brightness);

  // Setup server routes (but don't start yet)
  setupServerRoutes();

  state = 0;
}

void setupServerRoutes()
{
  // Serve the webpage for RGBW input
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send_P(200, "text/html", index_html); });

  // Handle color POST request
  server.on("/setColors", HTTP_POST, [](AsyncWebServerRequest *request)
            {
    if (request->hasParam("r1", true) && request->hasParam("g1", true) && 
        request->hasParam("b1", true) && request->hasParam("w1", true) &&
        request->hasParam("r2", true) && request->hasParam("g2", true) && 
        request->hasParam("b2", true) && request->hasParam("w2", true) &&
        request->hasParam("br", true)) {
      
      // Get base color values
      uint8_t r1 = request->getParam("r1", true)->value().toInt();
      uint8_t g1 = request->getParam("g1", true)->value().toInt();
      uint8_t b1 = request->getParam("b1", true)->value().toInt();
      uint8_t w1 = request->getParam("w1", true)->value().toInt();
      
      // Get secondary color values
      uint8_t r2 = request->getParam("r2", true)->value().toInt();
      uint8_t g2 = request->getParam("g2", true)->value().toInt();
      uint8_t b2 = request->getParam("b2", true)->value().toInt();
      uint8_t w2 = request->getParam("w2", true)->value().toInt();
      
      // Get brightness
      brightness = request->getParam("br", true)->value().toInt();
      
      // Update colors using NeoPixel.Color() - note: Color(r,g,b,w)
      base_color = NeoPixel.Color(r1, g1, b1, w1);
      sec_color = NeoPixel.Color(r2, g2, b2, w2);
      NeoPixel.setBrightness(brightness);
      
      // Save to preferences for persistence
      preferences.putUInt("base", base_color);
      preferences.putUInt("sec", sec_color);
      preferences.putUChar("bright", brightness);
      
      request->send(200, "text/plain", "Colors saved successfully!");
    } else {
      request->send(400, "text/plain", "Invalid color values");
    } });
}

void startServer()
{
  // Start open AP (no password)
  WiFi.softAP(ssid);

  // Start web server
  server.begin();
}

void stopServer()
{
  // Stop web server
  server.end();

  // Stop AP
  WiFi.softAPdisconnect(true);
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
    color = rgbw_lin_interp(base_color, sec_color, steps);
    NeoPixel.fill(color);
    break;
  case RUNNING:
    // Fill the whole strip white
    NeoPixel.fill(base_color);
    // Set a single blue pixel to blue to create a running effect.
    NeoPixel.setPixelColor((running_cnt / 127) % NUM_PIXELS, sec_color);
    break;
  case RAINBOW:
    // Fill with rainbow
    NeoPixel.rainbow(running_cnt);
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

  if (btn_val == LOW)
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

      if (web_server_enabled)
      {
        startServer();
        // Flash green to indicate server started
        for (int i = 0; i < 3; i++)
        {
          NeoPixel.fill(NeoPixel.Color(0, 255, 0, 0)); // Green
          NeoPixel.show();
          delay(200);
          NeoPixel.clear();
          NeoPixel.show();
          delay(200);
        }
      }
      else
      {
        stopServer();
        // Flash red to indicate server stopped
        for (int i = 0; i < 3; i++)
        {
          NeoPixel.fill(NeoPixel.Color(255, 0, 0, 0)); // Red
          NeoPixel.show();
          delay(200);
          NeoPixel.clear();
          NeoPixel.show();
          delay(200);
        }
      }
    }
  }
  else if (btn_val == HIGH && press_start > 0)
  {
    // Button released
    if (!button_handled && debounce >= DEBOUNCE_THRESHOLD)
    {
      // Short press - advance state
      running_cnt = 0;
      state = (state + 1) % 6;
    }
    // Reset for next press
    debounce = 0;
    press_start = 0;
    button_handled = false;
  }
}

uint32_t rgbw_lin_interp(uint32_t c1, uint32_t c2, uint32_t step, uint32_t num_steps)
{
  // linear interpolation for rgbw values
  // arguments:
  //    c1 - a packed color code in the form 0xgrbw
  //    c2 - a second packed color code
  //    step - the current step between c1 and c2
  //    steps - the number of steps between c1 and c2(default 255)
  // returns:
  //    uint32_t representing the current value that is %step% steps from c1 to c2

  int16_t dif_r, dif_g, dif_b, dif_w;
  uint8_t new_r, new_g, new_b, new_w;
  // most significant byte is white
  dif_w = ((c2 & 0xff000000) >> 24) - ((c1 & 0xff000000) >> 24);

  // then red
  dif_r = ((c2 & 0xff0000) >> 16) - ((c1 & 0xff0000) >> 16);

  // then green
  dif_g = ((c2 & 0xff00) >> 8) - ((c1 & 0xff00) >> 8);

  // then blue
  dif_b = (c2 & 0xff) - (c1 & 0xff);

  // get each new color value
  new_w = (uint16_t)((c1 & 0xff000000) >> 24) + (dif_w * step)/num_steps;
  new_r = (uint16_t)((c1 & 0xff0000) >> 16) + (dif_r * step)/num_steps;
  new_g = (uint16_t)((c1 & 0xff00) >> 8) + (dif_g * step)/num_steps;
  new_b = (uint16_t)(c1 & 0xff) + (dif_b * step)/num_steps;

  // repack to new color with gamma correction for perceptually smooth transitions
  uint32_t new_color = ((uint32_t)NeoPixel.gamma8(new_w) << 24) |
                       ((uint32_t)NeoPixel.gamma8(new_r) << 16) |
                       ((uint32_t)NeoPixel.gamma8(new_g) << 8) |
                       NeoPixel.gamma8(new_b);

  return new_color;
}