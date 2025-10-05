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
        <title>RGB Light Control</title>
      </head>
      <body>
        <h1>Set Custom RGB Values</h1>
        <form action="/setRGB" method="POST">
          R: <input type="number" name="r" min="0" max="255"><br>
          G: <input type="number" name="g" min="0" max="255"><br>
          B: <input type="number" name="b" min="0" max="255"><br>
          <input type="submit" value="Set">
        </form>
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