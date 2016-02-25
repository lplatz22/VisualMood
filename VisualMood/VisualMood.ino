#include <Adafruit_NeoPixel.h>

#define PIN 6
const uint32_t MAX32 = 4294967295;
const double MAXSensor = 1023.0;
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
int sensorValue1 = 0;

// start with increasing intensity in the color
// once breatheTimer reaches 425, we start to decrease color intensity
int breatheTimer = 0;
float breatheMax = 425.0;
bool increaseIntensity = true;

const int buttonPin = 2; // the number of the pushbutton pin
int buttonState = 0;     // variable for reading the pushbutton status
bool pushed = false;

namespace smoothOP
{
  uint32_t curColor = 0;
  int red = 0;
  int blue = 0;
  int green = 0;
}

enum LightMode { 
  simple, 
  breathe,
  smoothMove,
  pressure2x, 
  ripple,
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
//  sensorValue = analogRead(A0);
//  sensorValue1 = analogRead(A2);
//  Serial.println("A0: " + sensorValue);
//  Serial.println(sensorValue1);

  buttonState = digitalRead(buttonPin);
  if (!pushed && buttonState == HIGH) {
    pushed = true;
    Serial.println("Pressed!");
    if(currentMode == simple){
      currentMode = breathe;
    }else if (currentMode == breathe){
      currentMode = smoothMove;
    }else if (currentMode == smoothMove) {
      currentMode = pressure2x;
    }else if (currentMode == pressure2x) {
      currentMode = ripple;
    }else if (currentMode == ripple) {
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
    case (smoothMove):
      smoothOperator();
      break;
    case (pressure2x):
      doublePressure();
      break;
    case (ripple):
      rippleEffect();
      break;
    case (rainbow):
      rainbowWithPressure();
      break;
    default:
      colorWithPressure();
  }
}

void doublePressure(){
  sensorValue = analogRead(A0);
  sensorValue1 = analogRead(A2);

  float red = putInRange(sensorValue, 0, 1023);
  float green = putInRange(sensorValue1, 0, 1000);
  setAllLights(strip.Color(red, green, 0));
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
  if (breatheTimer == breatheMax) { // start decreasing intensity of lights from here on out
    increaseIntensity = false;
  }else if (breatheTimer == 0) { // start increasing intensity of lights from here on out
    increaseIntensity = true;
  }
  float percent = (breatheTimer / breatheMax); // percentage of breatheTimer gone through
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

//runs continuous rainbow effect, speeding up with higher pressure & breathing effect added
void rainbowWithPressureAndBreathe() {
  uint16_t i, j;
  uint8_t wait = 30;

  if (increaseIntensity) {
    breatheTimer++;
  }
  else {
    breatheTimer--;
  }
  
  if (breatheTimer == breatheMax) { // start decreasing intensity of lights from here on out
    increaseIntensity = false;
  }
  else if (breatheTimer == 0) { // start increasing intensity of lights from here on out
    increaseIntensity = true;
  }

  float percent = (breatheTimer / breatheMax); // percentage of breatheTimer gone through
  
  for(j=0; j<256; j++) {
    for(i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, WheelWithBreathe((i+j) & 255, percent));
    }
    
    strip.show();
    sensorValue = analogRead(A0);
    
    if (sensorValue <= 800){
      wait = 80;
    }
    else if (800 < sensorValue && sensorValue <= 980){
      wait = 25;
    }
    else if (980 < sensorValue){
      wait = 0;
    }
    
    buttonState = digitalRead(buttonPin);
    
    if (!pushed && buttonState == HIGH) {
      pushed = true;
      currentMode = simple; // BUG: Workaround, will work as long as simple is first, last is rainbow
      Serial.println("Pressed!");
      Serial.println(currentMode);
      break;
    }
    
    delay(wait);
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
    buttonState = digitalRead(buttonPin);
    if (!pushed && buttonState == HIGH) {
      pushed = true;
      currentMode = simple; // BUG: Workaround, will work as long as simple is first, last is rainbow
      Serial.println("Pressed!");
      Serial.println(currentMode);
      break;
    }
    delay(wait);
  }
}

// Smooth Loop for pressure transitions:
void smoothOperator() {
  sensorValue = analogRead(A2);
  if(sensorValue >= 765){
    red = 255;
    blue = 255;
    green = 255;
  }else if (sensorValue >= 510 && sensorValue < 765){
    red = sensorValue - 510;
    blue = 255;
    green = 255;
  }else if (sensorValue >= 255 && sensorValue < 510){
    red = 0;
    blue = sensorValue - 255;
    green = 255;
  }else if (sensorValue >= 0 && sensorValue < 255){
    red = 0;
    blue = 0;
    green = sensorValue;
  }
  uint32_t targetColor = strip.Color(red, green, blue);
  if (smoothOP::curColor < targetColor) {
    goingUP();
  } else {
    goingDOWN();
  } 
}

void goingUP() {
  delay(20);
  if(smoothOP::curColor < MAX32) {
    setAllLights(toRGB(++smoothOP::curColor));
  }
  return;
}

void goingDOWN() {
  delay(8);
  if(smoothOP::curColor > 0) {
    setAllLights(toRGB(--smoothOP::curColor));
  }
  return;
}

// all lights will be set to one color.
// there will be a ripple of another color travelling around the LEDs
// the higher the pressure applied, the faster the ripple and the lighter the color 
int currentPixel = 0;
void rippleEffect() {
  int blue = strip.Color(0, 0, 255);
  setAllLights(blue);

  int numPixels = strip.numPixels();
  int numLoopsOverAllPixels = 5;
  int curPixel, prevPixel;
  for (int j = 0; j < numLoopsOverAllPixels; j++) {
    for(curPixel=0; curPixel<numPixels; curPixel++) {
      
      // get delay and color from pressure reading
      sensorValue = analogRead(A0);
      int wait = getDelayFromPressure(sensorValue);
      int color = getColorFromPressure(sensorValue);
      
      int rippleWidth = 5;
      for (int k = 0; k < rippleWidth; k++) {
        strip.setPixelColor((curPixel+k) % (numPixels-1), color);
      }
      
      // set previous pixel back to blue
      if (curPixel == 0) {
        prevPixel = numPixels - 1;
      } else {
        prevPixel = curPixel - 1;
      }
      strip.setPixelColor(prevPixel, blue);
      strip.show();
      
      delay(wait);
    }
  }
  currentMode = rainbow;
}

// higher pressure --> lower delay
int getDelayFromPressure(int sensorValue) {
  int wait;
  if (sensorValue <= 800){
      wait = 80;
   }else if (800 < sensorValue && sensorValue <= 890){
      wait = 40;
   }else if (890 < sensorValue && sensorValue <= 980){
      wait = 20;
   }else if (980 < sensorValue) {
      wait = 10;
   }
   return wait;
}

// higher pressure --> ligther color
int getColorFromPressure(int sensorValue) {
  if(sensorValue <= 800){
    int darkGreen = strip.Color(0, 255, 0);
    return darkGreen;
  }else if(800 < sensorValue && sensorValue <= 890){
    int midGreen = strip.Color(0, 160, 0);
    return midGreen;
  }else if(890 < sensorValue && sensorValue <= 980){
    int lightGreen = strip.Color(0, 80, 0);
    return lightGreen;
  }else if(980 < sensorValue){
    int white = strip.Color(0, 0, 0);
    return white;
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
// Helper function for rainbow and Smooth Operator
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

uint32_t WheelWithBreathe(byte WheelPos, float percent) {
  if(WheelPos < 85) {
   return strip.Color(WheelPos * 3 * percent, (255 - WheelPos * 3) * percent, 0);
  } else if(WheelPos < 170) {
   WheelPos -= 85;
   return strip.Color((255 - WheelPos * 3) * percent, 0, WheelPos * 3 * percent);
  } else {
   WheelPos -= 170;
   return strip.Color(0, WheelPos * 3 * percent, (255 - WheelPos * 3) * percent);
  }
}

uint32_t toRGB(uint32_t inVal) {
  if (inVal > 255) {
    inVal | 255;
  }
  
  if (inVal > 65535) {
    inVal | 65535;
  }
 
  if (inVal > 16777215) {
   inVal | 16777215;
  }
  
  return inVal;
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

