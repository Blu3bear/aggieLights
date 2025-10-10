#include <WiFi.h>
#include <ESPAsyncWebServer.h>

// WiFi credentials
const char* ssid = "ESP32_AccessPoint";
const char* password = "12345678";

// Global variables for custom RGB values
volatile uint32_t CUSTOM_RGB[3] = {0, 0, 0}; // Default to black

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// Pin definitions
#define R_PIN 12
#define G_PIN 14
#define B_PIN 13
#define BTN_PIN 27

// State definitions
#define SOLID_WHITE 0
#define SOLID_BLUE 1
#define ALTERNATING 2
#define RUNNING 3
#define CUSTOM 4

uint8_t BLUE[3] = {40, 141, 194};
uint8_t WHITE[3] = {255, 255, 255};

uint8_t state = 0;
uint8_t debounce = 0;
uint16_t running_cnt = 0;

float r_step;
float g_step;
float b_step;

// Function to handle RGB changes
void setCustomRGB(uint32_t r, uint32_t g, uint32_t b) {
  CUSTOM_RGB[0] = r;
  CUSTOM_RGB[1] = g;
  CUSTOM_RGB[2] = b;
}

void setup() {
  // Initialize pins
  pinMode(BTN_PIN, INPUT);
  pinMode(R_PIN, OUTPUT);
  pinMode(G_PIN, OUTPUT);
  pinMode(B_PIN, OUTPUT);

  // Initialize WiFi in access point mode
  WiFi.softAP(ssid, password);
  Serial.begin(115200);
  Serial.println("Access point started");
  Serial.print("IP Address: ");
  Serial.println(WiFi.softAPIP());

  // Serve the webpage for RGB input
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/html", R"rawliteral(
      <!DOCTYPE html>
      <html>
      <head>
      <meta name="viewport" content="width=device-width,initial-scale=1">
      <title>RGB Control</title>
      <style>
      body{font-family:Arial;max-width:400px;margin:50px auto;padding:20px;background:#222;color:#fff}
      h1{text-align:center;font-size:24px;margin:0 0 20px}
      .mode{text-align:center;margin-bottom:20px}
      .mode label{display:inline;margin:0 10px;font-weight:normal;cursor:pointer}
      .slider{width:100%;margin:15px 0}
      input[type=range]{width:100%;height:30px;-webkit-appearance:none;background:transparent}
      input[type=range]::-webkit-slider-track{height:8px;background:#555;border-radius:4px}
      input[type=range]::-webkit-slider-thumb{-webkit-appearance:none;width:24px;height:24px;border-radius:50%;cursor:pointer}
      input[type=range]::-moz-range-track{height:8px;background:#555;border-radius:4px}
      input[type=range]::-moz-range-thumb{width:24px;height:24px;border-radius:50%;border:0;cursor:pointer}
      #r::-webkit-slider-thumb,#r2::-webkit-slider-thumb{background:#f00}
      #r::-moz-range-thumb,#r2::-moz-range-thumb{background:#f00}
      #g::-webkit-slider-thumb,#g2::-webkit-slider-thumb{background:#0f0}
      #g::-moz-range-thumb,#g2::-moz-range-thumb{background:#0f0}
      #b::-webkit-slider-thumb,#b2::-webkit-slider-thumb{background:#00f}
      #b::-moz-range-thumb,#b2::-moz-range-thumb{background:#00f}
      label{display:flex;justify-content:space-between;margin-bottom:5px;font-weight:bold}
      .val{background:#333;border:1px solid #555;border-radius:4px;padding:2px 6px;color:#fff;width:35px;text-align:center}
      input[type=number]::-webkit-inner-spin-button,input[type=number]::-webkit-outer-spin-button{-webkit-appearance:none;margin:0}
      input[type=number]{-moz-appearance:textfield}
      #preview{height:60px;border-radius:8px;margin:20px 0;transition:background .1s}
      #preview2{height:60px;border-radius:8px;margin:20px 0;transition:background .1s;display:none}
      #c2{display:none}
      button{width:100%;padding:12px;font-size:16px;background:#0066cc;color:#fff;border:0;border-radius:6px;cursor:pointer;margin-top:10px}
      button:active{background:#0052a3}
      </style>
      </head>
      <body>
      <h1>RGB Light Control</h1>
      <div class="mode">
      <label><input type="radio" name="m" value="0" checked onchange="m()"> Single</label>
      <label><input type="radio" name="m" value="1" onchange="m()"> Transition</label>
      </div>
      <div id="preview"></div>
      <div id="c1">
      <div class="slider">
      <label>R: <input type="number" class="val" id="rv" min="0" max="255" value="0" onchange="v('r',this.value)"></label>
      <input type="range" id="r" min="0" max="255" value="0" oninput="u()">
      </div>
      <div class="slider">
      <label>G: <input type="number" class="val" id="gv" min="0" max="255" value="0" onchange="v('g',this.value)"></label>
      <input type="range" id="g" min="0" max="255" value="0" oninput="u()">
      </div>
      <div class="slider">
      <label>B: <input type="number" class="val" id="bv" min="0" max="255" value="0" onchange="v('b',this.value)"></label>
      <input type="range" id="b" min="0" max="255" value="0" oninput="u()">
      </div>
      </div>
      <div id="c2">
      <div id="preview2"></div>
      <div class="slider">
      <label>R2: <input type="number" class="val" id="r2v" min="0" max="255" value="0" onchange="v('r2',this.value)"></label>
      <input type="range" id="r2" min="0" max="255" value="0" oninput="u2()">
      </div>
      <div class="slider">
      <label>G2: <input type="number" class="val" id="g2v" min="0" max="255" value="0" onchange="v('g2',this.value)"></label>
      <input type="range" id="g2" min="0" max="255" value="0" oninput="u2()">
      </div>
      <div class="slider">
      <label>B2: <input type="number" class="val" id="b2v" min="0" max="255" value="0" onchange="v('b2',this.value)"></label>
      <input type="range" id="b2" min="0" max="255" value="0" oninput="u2()">
      </div>
      </div>
      <button onclick="s()">Set Color</button>
      <script>
      function u(){
      let r=document.getElementById('r').value;
      let g=document.getElementById('g').value;
      let b=document.getElementById('b').value;
      document.getElementById('rv').value=r;
      document.getElementById('gv').value=g;
      document.getElementById('bv').value=b;
      document.getElementById('preview').style.background='rgb('+r+','+g+','+b+')';
      }
      function u2(){
      let r2=document.getElementById('r2').value;
      let g2=document.getElementById('g2').value;
      let b2=document.getElementById('b2').value;
      document.getElementById('r2v').value=r2;
      document.getElementById('g2v').value=g2;
      document.getElementById('b2v').value=b2;
      document.getElementById('preview2').style.background='rgb('+r2+','+g2+','+b2+')';
      }
      function v(id,val){
      document.getElementById(id).value=val;
      if(id.includes('2'))u2();else u();
      }
      function m(){
      let mode=document.querySelector('input[name="m"]:checked').value;
      document.getElementById('c2').style.display=mode=='1'?'block':'none';
      document.getElementById('preview2').style.display=mode=='1'?'block':'none';
      }
      function s(){
      let mode=document.querySelector('input[name="m"]:checked').value;
      let r=document.getElementById('r').value;
      let g=document.getElementById('g').value;
      let b=document.getElementById('b').value;
      let body='r='+r+'&g='+g+'&b='+b+'&m='+mode;
      if(mode=='1'){
      let r2=document.getElementById('r2').value;
      let g2=document.getElementById('g2').value;
      let b2=document.getElementById('b2').value;
      body+='&r2='+r2+'&g2='+g2+'&b2='+b2;
      }
      fetch('/setRGB',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:body});
      }
      u();
      </script>
      </body>
      </html>
    )rawliteral");
  });

  // Handle RGB POST request
  server.on("/setRGB", HTTP_POST, [](AsyncWebServerRequest *request){
    if (request->hasParam("r", true) && request->hasParam("g", true) && request->hasParam("b", true)) {
      uint32_t r = request->getParam("r", true)->value().toInt();
      uint32_t g = request->getParam("g", true)->value().toInt();
      uint32_t b = request->getParam("b", true)->value().toInt();
      setCustomRGB(r, g, b);
      request->send(200, "text/plain", "RGB values updated");
    } else {
      request->send(400, "text/plain", "Invalid RGB values");
    }
  });

  // Start server
  server.begin();
}

void loop() {
  // Check the state, then run the appropriate
  switch (state) {
    case SOLID_WHITE:
      lightWhite();
      break;
    case SOLID_BLUE:
      lightBlue();
      break;
    case ALTERNATING:
      if (running_cnt < 500) {
        lightWhite();
        delay(1);
        running_cnt++;
      } else {
        lightBlue();
        delay(1);
        running_cnt = (running_cnt + 1) % 1000;
      }
      break;
    case RUNNING:
      // calculate step size
      r_step = lin_interp(WHITE[0],BLUE[0],1000);
      g_step = lin_interp(WHITE[1],BLUE[1],1000);
      b_step = lin_interp(WHITE[2],BLUE[2],1000);
      if (running_cnt < 1000) {
        writeLed((uint8_t)(WHITE[0]+r_step*running_cnt),(uint8_t)(WHITE[1]+g_step*running_cnt),(uint8_t)(WHITE[2]+b_step*running_cnt));
        delay(1);
        running_cnt++;
      } else {
        writeLed((uint8_t)(BLUE[0]-r_step*(running_cnt-1000)),(uint8_t)(BLUE[1]-g_step*(running_cnt-1000)),(uint8_t)(BLUE[2]-b_step*(running_cnt-1000)));
        delay(1);
        running_cnt = (running_cnt + 1) % 2000;
      }
      break;
    case CUSTOM:
      analogWrite(R_PIN, CUSTOM_RGB[0]);
      analogWrite(G_PIN, CUSTOM_RGB[1]);
      analogWrite(B_PIN, CUSTOM_RGB[2]);
      break;
    default:
      analogWrite(R_PIN, 0);
      analogWrite(G_PIN, 0);
      analogWrite(B_PIN, 0);
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

void writeLed(uint8_t r, uint8_t g, uint8_t b){
  analogWrite(R_PIN, r);
  analogWrite(G_PIN, g);
  analogWrite(B_PIN, b);
}

void lightWhite() {
  analogWrite(R_PIN, WHITE[0]);
  analogWrite(G_PIN, WHITE[1]);
  analogWrite(B_PIN, WHITE[2]);
}

void lightBlue() {
  analogWrite(R_PIN, BLUE[0]);
  analogWrite(G_PIN, BLUE[1]);
  analogWrite(B_PIN, BLUE[2]);
}

float lin_interp(float x, float y, float steps){
// This function takes two values and a number of steps and returns the step size
return (y-x)/steps;
}