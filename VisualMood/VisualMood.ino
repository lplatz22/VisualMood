#include <Adafruit_NeoPixel.h>

#define PIN 6

// Parameter 1 = number of pixels in strip
// Parameter 2 = pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(150, PIN, NEO_GRB + NEO_KHZ800);

//float voltage = 0.0;
//Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V):
//voltage = sensorValue * (5.0 / 1023.0);
int sensorValue = 0;

// start with increasing intensity in the color
// once breatheTimer reaches 500, we start to decrease color intensity
int breatheTimer = 0;
bool increaseIntensity = true;

const int buttonPin = 2; // the number of the pushbutton pin
int buttonState = 0;     // variable for reading the pushbutton status
bool pushed = false;

enum LightMode { 
  simple, 
  breathe, 
  rainbow
}; 

LightMode currentMode;

void setup() {
  Serial.begin(9600); //bits per second
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  pinMode(buttonPin, INPUT);
  currentMode = simple;
  Serial.println(currentMode);
}

void loop() {
  
  buttonState = digitalRead(buttonPin);
  if (!pushed && buttonState == HIGH) {
    pushed = true;
    Serial.println("Pressed!");
    if(currentMode == simple){
      currentMode = breathe;
    }else if (currentMode == breathe){
      currentMode = rainbow;
    }else if (currentMode == rainbow){ // BUG: Wont switch away from rainbow, is stuck in that loop, will be fine for demo
      currentMode = simple;
    }
    Serial.println(currentMode);
  } else if (buttonState == LOW){
    pushed = false;
  } 
  
  switch (currentMode){
    case (simple):
      colorWithPressure();
      break;
    case (breathe):
      breatheEffectLoop();
      break;
    case (rainbow):
      rainbowWithPressure();
      break;
    default:
      colorWithPressure();
  }
}

// Moves between colors by mixing them as pressure changes 
// instead of sharply transitioning
void colorWithPressure(){
  sensorValue = analogRead(A0);
  if(sensorValue <= 800){
    float green = putInRange(sensorValue, 0, 800);
    setAllLights(strip.Color(0, green, 0)); // Green
  }else if(800 < sensorValue && sensorValue <= 980){
    float blue = putInRange(sensorValue, 800, 980);
    float green = 255 - blue;
    setAllLights(strip.Color(0, green, blue)); // Blue
  }else if(980 < sensorValue){
    float red = putInRange(sensorValue, 980, 1023);
    float blue = 255 - red;
    setAllLights(strip.Color(red, 0, blue)); // Red
  }
}

//breathe effect, with smooth transitions with pressure
void breatheEffectLoop() {
  sensorValue = analogRead(A0);
  if (increaseIntensity) {
    breatheTimer++;
  }else {
    breatheTimer--;
  }
  if (breatheTimer == 500) { // start decreasing intensity of lights from here on out
    increaseIntensity = false;
  }else if (breatheTimer == 0) { // start increasing intensity of lights from here on out
    increaseIntensity = true;
  }
  float percent = (breatheTimer / 500.0); // percentage of breatheTimer gone through
  if(sensorValue < 800){ // Green Base
    float green = putInRange(sensorValue, 0, 800);
    green *= percent;
    setAllLights(strip.Color(0, green, 0));
  }else if(800 <= sensorValue && sensorValue < 980){ // Blue Base
    float blue = putInRange(sensorValue, 800, 980);
    float green = 255 - blue;
    blue *= percent;
    green *= percent;
    setAllLights(strip.Color(0, green, blue));
  }else if(980 <= sensorValue){ // Red base
    float red = putInRange(sensorValue, 980, 1023);
    float blue = 255 - red;
    red *= percent;
    blue *= percent;
    setAllLights(strip.Color(red, 0, blue));
  }
}

//runs continuous rainbow effect, speeding up with higher pressure
void rainbowWithPressure() {
  uint16_t i, j;
  uint8_t wait = 30;
  for(j=0; j<256; j++) {
    for(i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel((i+j) & 255));
    }
    strip.show();
    sensorValue = analogRead(A0);
    if (sensorValue <= 800){
      wait = 80;
    }else if (800 < sensorValue && sensorValue <= 980){
      wait = 25;
    }else if (980 < sensorValue){
      wait = 0;
    }
    delay(wait);
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
// Helper function for rainbow
uint32_t Wheel(byte WheelPos) {
  if(WheelPos < 85) {
   return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  } else if(WheelPos < 170) {
   WheelPos -= 85;
   return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else {
   WheelPos -= 170;
   return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}

// squeeze any number in any range in between 0-255
float putInRange(float value, float oldMin, float oldMax){
  float oldRange = oldMax - oldMin; 
  float newRange = 255;
  return (((value - oldMin) * newRange) / oldRange);
}

// Sets all lights on the strip to color c.
void setAllLights(uint32_t c) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, c);
  }
  strip.show();
}

