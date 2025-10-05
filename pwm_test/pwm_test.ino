/*
This program is a simple way to test some ideas using pwm based light strip while addressable leds are unavailable.

It seeks to implement the same base functionality but with modified states for the light driving.
*/

#define R_PIN 14
#define G_PIN 12
#define B_PIN 13
#define BTN_PIN 27
#define SOLID_WHITE 0     // The state value corresponding to solid white
#define SOLID_BLUE 1      // The state value corresponding to solid blue
#define ALTERNATING 2     // The state value corresponding to alternating blue/white
#define RUNNING 3         // The state value corresponding to blue running up
#define CUSTOM 4          // The state value corresponding to a custom color map(WIP) 


// The lights get activated by driving the pins low, but the driver I have for power requirements inverts the signal
uint32_t BLUE[3] = {40, 141, 194};
uint32_t WHITE[3] ={255,255,255};

// state variable, and counter initialization
uint8_t state = 0;
uint8_t debounce = 0;
uint16_t running_cnt = 0;

void setup() {
  // Initialize the state variable
  state = 0;
  debounce = 0;
  running_cnt = 0;
  pinMode(BTN_PIN, INPUT);
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
    // Alternate white and blue
    // for the strip im using the best I can do is light blue and every other one will activate ):
    analogWrite(R_PIN, 0);
    analogWrite(G_PIN, 0);
    analogWrite(B_PIN, BLUE[2]);
  break;
  case RUNNING:
    // running light effect
    if(running_cnt < 200){
    lightWhite();
    delay(1);
    running_cnt++;
    }
    else{
    lightBlue();
    delay(1);
    running_cnt = (running_cnt+1)%400;
    }
  break;
  default:
    // default to lights off
    analogWrite(R_PIN, 0);
    analogWrite(G_PIN, 0);
    analogWrite(B_PIN, 0);
  }

  // Debounce button press and increment the state
  int btn_val = digitalRead(BTN_PIN);

  if(btn_val == HIGH) {
    // Increment our counter while the button is held(saturates at 255)
    if(debounce <255){
      debounce++;
    }
  }
  else if(btn_val == LOW && debounce > 1){
    // When the button is released increment the state
    debounce = 0;
    state = (state+1)%5;
  }
  else{
    // If the button isn't being held or wasn't held long enough then it must have been a bounce
    debounce = 0;
  }
}

void lightWhite(){
  analogWrite(R_PIN, WHITE[0]);
  analogWrite(G_PIN, WHITE[1]);
  analogWrite(B_PIN, WHITE[2]);
  }

void lightBlue(){
  analogWrite(R_PIN, BLUE[0]);
  analogWrite(G_PIN, BLUE[1]);
  analogWrite(B_PIN, BLUE[2]);
}
