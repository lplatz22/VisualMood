#include <Adafruit_NeoPixel.h>
#include <LiquidCrystal.h>

#define PIN 6

#define SENSOR_1 A0 // Sensor_1 is used for all single controlled Modes
#define SENSOR_2 A1
#define SENSOR_3 A2

const uint32_t MAX32 = 4294967295;
const uint32_t MAX24 = 16777216;
const double MAXSensor = 1023.0;
// Parameter 1 = number of pixels in strip
// Parameter 2 = pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(130, PIN, NEO_GRB + NEO_KHZ800);

//float voltage = 0.0;
//Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V):
//voltage = sensorValue * (5.0 / 1023.0);
int sensorValue = 0; // Weak Sensor
int sensorValue1 = 0; // Strong Sensor
int sensorValue2 = 0; // Weak Sensor

// start with increasing intensity in the color
// once breatheTimer reaches 425, we start to decrease color intensity
int breatheTimer = 0;
float breatheMax = 425.0;
bool increaseIntensity = true;

const int modeButtonPin = 7; // button to switch modes
int modeButtonState = 0;     // variable for reading the pushbutton status
bool modeButtonPushed = false;

const int optionButtonPin = 8; // button to switch modes
int optionButtonState = 0;     // variable for reading the pushbutton status
bool optionButtonPushed = false;

int rainbowStyle = 0;

// initialize the library with the numbers of the interface pins
// for the LED screen
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

class User{
private:
  String name;
  float highPressure;
  int highDuration;
  
public:
  User(String incomingName){
    name = incomingName;
    highPressure = 0;
    highDuration = 0;
  }
  float getHighPressure(){
    return highPressure;
  }
  int getHighDuration(){
    return highDuration;
  }
  String getName(){
    return name;
  }

  void setHighTime(int seconds){
    highDuration = seconds;
  }
  void setHighPressure(float highPressureIn){
    highPressure = highPressureIn;
  }
  void setName(String NameIn){
    name = NameIn;
  }
};

int currentUser = 0;
const int MAXUSERS = 100;
User* users[MAXUSERS];
int totalUsers = 0;

namespace smoothOP
{ 
  uint32_t curColor = 0;
  int red = 0;
  int blue = 0;
  int green = 0;
}

enum LightMode {
  off, 
  simple, 
  breathe,
  smoothMove,
  maxOut,
  pressure2x,
  painting, 
  colorWave,
  ripple,
  rainbow
}; 

LightMode currentMode;

void setup() {
  Serial.begin(9600); //bits per second
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  pinMode(modeButtonPin, INPUT);
  pinMode(optionButtonPin, INPUT);
  currentMode = off;
  Serial.println(currentMode);
  lcd.begin(16, 2); // set up the LCD's number of columns and rows:
  lcd.setCursor(0, 0); //column 0, row 0
  lcd.print(make16Chars("Visual Mood!")); // Print a message to the LCD.

  //Set Up Users
  User* luke = new User("Luke");
  User* victor = new User("Victor");
  User* rohan = new User("Rohan");
  User* sam = new User("Sam");
  //Add users to users[]
  users[0] = luke;
  totalUsers++;
  users[1] = victor;
  totalUsers++;
  users[2] = rohan;
  totalUsers++;
  users[3] = sam;
  totalUsers++;
}

void loop() {
  optionButtonState = digitalRead(optionButtonPin);
  if (!optionButtonPushed && optionButtonState == HIGH) {
    optionButtonPushed = true;
    if (currentMode == smoothMove || currentMode == maxOut){
      currentUser++;
      currentUser = currentUser % totalUsers;
    }else if (currentMode == rainbow){
      Serial.println(rainbowStyle);
      rainbowStyle++;
      rainbowStyle = rainbowStyle % 1;
    }
  } else if (optionButtonState == LOW){
    optionButtonPushed = false;
  } 
  
  modeButtonState = digitalRead(modeButtonPin);
  if (!modeButtonPushed && modeButtonState == HIGH) {
    modeButtonPushed = true;
    Serial.println("Mode Changed");
    if(currentMode == off){
      currentMode = simple;
    }else if(currentMode == simple){
      currentMode = breathe;
    }else if (currentMode == breathe){
      currentMode = smoothMove;
    }else if (currentMode == smoothMove) {
      currentMode = maxOut;
    }else if (currentMode == maxOut) {
      currentMode = pressure2x;
    }else if (currentMode == pressure2x) {
      currentMode = painting;
    }else if (currentMode == painting) {
      currentMode = colorWave;
    }else if (currentMode == colorWave) {
      currentMode = ripple;
    }else if (currentMode == ripple) {
      currentMode = rainbow;
    }else if (currentMode == rainbow){
      currentMode = off;
    }
  } else if (modeButtonState == LOW){
    modeButtonPushed = false;
  } 

  // set the cursor to column 0, line 1
  // (note: line 1 is the second row, since counting begins with 0):
  lcd.setCursor(0, 1);
  switch (currentMode){
    case (off):
      lcd.print(make16Chars("LED OFF"));
      setAllLights(strip.Color(0, 0, 0));
      break;
    case (simple):
      lcd.print(make16Chars("Simple"));
      colorWithPressure();
      break;
    case (breathe):
      lcd.print(make16Chars("Breathe"));
      breatheEffectLoop();
      break;
    case (smoothMove):
      lcd.print(make16Chars("Training: " + users[currentUser]->getName()));
      lcd.setCursor(0, 0);
      lcd.print(make16Chars("High Score: " + (String)users[currentUser]->getHighPressure()));
      lcd.setCursor(0, 1);
      smoothOperator();
      break;
    case (maxOut):
      lcd.print(make16Chars("Max Out: " + users[currentUser]->getName()));
      lcd.setCursor(0, 0);
      lcd.print(make16Chars("High Score: " + (String)users[currentUser]->getHighPressure()));
      lcd.setCursor(0, 1);
      maxOutTraining();
      break;
    case (pressure2x):
      lcd.setCursor(0, 0);
      lcd.print(make16Chars("Visual Mood!"));
      lcd.setCursor(0, 1);
      lcd.print(make16Chars("Color Mixing"));
      doublePressure();
      break;
    case (painting):
      lcd.print(make16Chars("Color Painting"));
      colorMixPaint();
      break;
    case (ripple):
      lcd.print(make16Chars("Ripple"));
      rippleEffect();
      break;
    case (colorWave):
      lcd.print(make16Chars("Color Wave"));
      colorMixWave();
      break;
    case (rainbow):
      if(rainbowStyle == 0){
        lcd.print(make16Chars("Rainbow - Reg"));
        rainbowWithPressure();
      }else{
        lcd.print(make16Chars("Rainbow - Cycle"));
        rainbowCycle();
      }
      break;
    default:
      lcd.print(make16Chars("ERROR! - off/on"));
      colorWithPressure();
  }
}

int getSensorValue(uint8_t pin, int minValue){
  int sensorVal = analogRead(pin);
  if (sensorVal < minValue){
    return 0.0;
  }else{
    return sensorVal;
  }
}

int getSensorValue(uint8_t pin){
  return analogRead(pin);
}

void doublePressure(){
  sensorValue = getSensorValue(SENSOR_1, 100);
  sensorValue1 = getSensorValue(SENSOR_2, 100);
  sensorValue2 = getSensorValue(SENSOR_3, 100);

  float red = putInRange(sensorValue, 100, 1023);
  float green = putInRange(sensorValue1, 100, 1023);
  float blue = putInRange(sensorValue2, 100, 1023);
  
  int jump = 15;
  uint32_t currentColor = strip.getPixelColor(129);
  transitionAllLights(strip.Color(red, green, blue), currentColor, jump);
}

// Moves between colors by mixing them as pressure changes 
void colorWithPressure(){
  sensorValue = getSensorValue(SENSOR_1, 400);
  uint32_t currentColor = strip.getPixelColor(129);
  int jump = 15; //Smaller = faster transistions
  if(sensorValue > 850 && sensorValue <= 945){
    float green = putInRange(sensorValue, 850, 945);
    transitionAllLights(strip.Color(0, green, 0), currentColor, jump);
  }else if(945 < sensorValue && sensorValue <= 995){
    float blue = putInRange(sensorValue, 945, 995);
    float green = 255 - blue;
    transitionAllLights(strip.Color(0, green, blue), currentColor, jump);
  }else if(995 < sensorValue){
    float red = putInRange(sensorValue, 995, 1020);
    float blue = 255 - red;
    transitionAllLights(strip.Color(red, 0, blue), currentColor, jump);
  }
  else {
    transitionAllLights(strip.Color(0, 0, 0), currentColor, jump);
  }
}


//breathe effect, with smooth transitions with pressure
void breatheEffectLoop() {
  sensorValue = getSensorValue(SENSOR_1, 100);
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
  uint32_t currentColor = strip.getPixelColor(129);
  int jump = 15; //Smaller = faster transistions
  if(sensorValue < 800){ // Green Base
    float green = putInRange(sensorValue, 0, 800);
    green *= percent;
    transitionAllLights(strip.Color(0, green, 0), currentColor, jump);
  }else if(800 <= sensorValue && sensorValue < 980){ // Blue Base
    float blue = putInRange(sensorValue, 800, 980);
    float green = 255 - blue;
    blue *= percent;
    green *= percent;
    transitionAllLights(strip.Color(0, green, blue), currentColor, jump);
  }else if(980 <= sensorValue){ // Red base
    float red = putInRange(sensorValue, 980, 1023);
    float blue = 255 - red;
    red *= percent;
    blue *= percent;
    transitionAllLights(strip.Color(red, 0, blue), currentColor, jump);
  }
}

//runs continuous rainbow effect, speeding up with higher pressure
void rainbowWithPressure() {
  uint16_t i, j;
  for(j=0; j<256; j++) {
    for(i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel((i+j) & 255));
    }
    strip.show();
    sensorValue = getSensorValue(SENSOR_1);
    uint8_t wait = getDelayFromPressure(sensorValue);

    optionButtonState = digitalRead(optionButtonPin);
    if (!optionButtonPushed && optionButtonState == HIGH) {
      optionButtonPushed = true;
      Serial.println(rainbowStyle);
      rainbowStyle = 1;
      lcd.print(make16Chars("Rainbow - Cycle"));
      break;
    } else if (optionButtonState == LOW){
      optionButtonPushed = false;
    } 

    modeButtonState = digitalRead(modeButtonPin);
    if (!modeButtonPushed && modeButtonState == HIGH) {
      modeButtonPushed = true;
      currentMode = off; // BUG: Workaround, will work as long as simple is first, last is rainbow
      Serial.println("Pressed!");
      Serial.println(currentMode);
      break;
    } else if (modeButtonState == LOW) {
      modeButtonPushed = false;
    }
    delay(wait);
  }
}

// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle() {
  uint16_t i, j;
  for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel
    for(i=0; i< strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
    sensorValue = getSensorValue(SENSOR_1);
    uint8_t wait = getDelayFromPressure(sensorValue);

    optionButtonState = digitalRead(optionButtonPin);
    if (!optionButtonPushed && optionButtonState == HIGH) {
      optionButtonPushed = true;
      Serial.println(rainbowStyle);
      rainbowStyle = 0;
      break;
    } else if (optionButtonState == LOW){
      optionButtonPushed = false;
    } 
    
    modeButtonState = digitalRead(modeButtonPin);
    if (!modeButtonPushed && modeButtonState == HIGH) {
      modeButtonPushed = true;
      currentMode = off; // BUG: Workaround, will work as long as simple is first, last is rainbow
      Serial.println("Pressed!");
      Serial.println(currentMode);
      break;
    } else if (modeButtonState == LOW) {
      modeButtonPushed = false;
    }
    delay(wait / 2);
  }
}

// Smooth Loop for pressure transitions:
void smoothOperator() {
  uint32_t curColor = strip.Color(smoothOP::red, smoothOP::green, smoothOP::blue);
  uint32_t targetColor = getSensorValue(SENSOR_1) * MAX24 / MAXSensor;
  if (smoothOP::red) {
    curColor = curColor || 255 || (255 << 8);
  }
  
  if (smoothOP::green) {
    curColor = curColor || 255;
  }
  
  if (smoothOP::curColor < targetColor) {
    goingUP();
  } else {
    goingDOWN();
  } 
}

void goingUP() {
//  Serial.println("GoingUP");
  if(smoothOP::red < 255) {
    setAllLights(increaseColor());
  }
  return;
}

void goingDOWN() {
//  Serial.println("GoingDown");
  if(smoothOP::blue || smoothOP::red || smoothOP::green) {
    setAllLights(decreaseColor());
  }
  return;
}

void maxOutTraining(){
  sensorValue = getSensorValue(SENSOR_1, 100);
  float meterHeight = putInRange(sensorValue, 100, 1023, 0, 130);

  for (int i = 0; i < meterHeight; i++){
    if(i <= 20){
      strip.setPixelColor(i, strip.Color(0,255,0));
    }else if (i > 20 && i <= 80){
      strip.setPixelColor(i, strip.Color(255,255,0));
    }else if (i > 80){
      strip.setPixelColor(i, strip.Color(255,0,0));
    }
    strip.show();
  }
  for (int i = strip.numPixels() - 1; i >= meterHeight; i--){
    strip.setPixelColor(i, strip.Color(0,0,0));
    strip.show();
  }

  float calibrateVal = -12.0;
  if(users[currentUser]->getHighPressure() < (sensorValue/1023.0 * 22.0 + calibrateVal)){
      users[currentUser]->setHighPressure((sensorValue/1023.0 * 22.0 + calibrateVal));
  }
}

uint32_t increaseColor()
{
   if (smoothOP::green == 255 || smoothOP::red > 0) {
//     Serial.println("RED");
     return strip.Color(++smoothOP::red, --smoothOP::green, smoothOP::blue);
   } else if (smoothOP::blue == 255 || smoothOP::green > 0) {
//     Serial.println("GREEN");
     return strip.Color(smoothOP::red, ++smoothOP::green, --smoothOP::blue);
   } else {
//     Serial.println("BLUE");
     return strip.Color(smoothOP::red, smoothOP::green, ++smoothOP::blue);
   } 
}

uint32_t decreaseColor()
{
  if (smoothOP::red > 0) {
//    Serial.println("RED");
    return strip.Color(--smoothOP::red, ++smoothOP::green, smoothOP::blue);
  } else if (smoothOP::green > 0) {
//    Serial.println("GREEN");
    return strip.Color(smoothOP::red, --smoothOP::green, ++smoothOP::blue);
  } else {
//    Serial.println("BLUE");
    return strip.Color(smoothOP::red, smoothOP::green, --smoothOP::blue);
  }
}

// ripple based off the Wheel function.
// the higher the pressure applied, the faster the ripple, and thus the faster
// it goes through the wheel. 
// At each new pixel, the wheel value is increased by 1.
int currentPixel = 0;
void rippleEffect() {
  // modeButtonPushed = true, need to wait for it to go LOW,
  // then we know its ready to be pushed again
  int color = 0;
  int numPixels = strip.numPixels();
  boolean doneWithRipple = false;
  while (1) {
    for(int curPixel=0; curPixel<numPixels; curPixel++) {
      
      // get delay and color from pressure reading
      sensorValue = getSensorValue(SENSOR_1);
      int wait = getDelayFromPressure(sensorValue);
      for (int i = 0; i <= curPixel; i++) {
        strip.setPixelColor(curPixel, Wheel(color));
      }
      strip.show();

      // only change color every 4 pixels
      if (curPixel % 4 == 0) {
        if (color == 255) {
          color = 0;
        } else {
          color++;
        }
      }

      modeButtonState = digitalRead(modeButtonPin);
    
      if (!modeButtonPushed && modeButtonState == HIGH) {
        modeButtonPushed = true;
        currentMode = rainbow;
        doneWithRipple = true;
        break;
      } else if (modeButtonState == LOW){
        modeButtonPushed = false;
      } 

      delay(wait);
    }
    if (doneWithRipple) {
      break;
    }
  }
}

//squeeze red/blue/green: send that color from 0->130, or until it stops recieving an input.
//on stoppage of input, keep wave length, send until its consumed by the 130th pixel
//basically, send a pixel of each color every loop that something is inputted,
//constantly be moving every pixel to 0->130
void colorMixWave(){
  //Rotate every pixel, every frame (maybe start doing this every 2nd,3rd)
  for(int i = strip.numPixels() - 1; i >= 0; i--){
    uint32_t currColor = strip.getPixelColor(i);
    strip.setPixelColor(i, strip.Color(0,0,0));
    if(i+1 < strip.numPixels() - 1){
      strip.setPixelColor(i + 1, currColor);
    }
  }

  sensorValue = getSensorValue(SENSOR_1, 200);
  sensorValue1 = getSensorValue(SENSOR_2, 100);
  sensorValue2 = getSensorValue(SENSOR_3, 100);
  uint8_t red = 0;
  uint8_t blue = 0;
  uint8_t green = 0;
  if(sensorValue > 0){
    red = putInRange(sensorValue, 100, 1023);
  }if(sensorValue1 > 0){
    green = putInRange(sensorValue1, 100, 1023);
  }if(sensorValue2 > 0){
    blue = putInRange(sensorValue2, 100, 1023);
  }
  strip.setPixelColor(0, strip.Color(red, green, blue));
  strip.show();
}


//when a color is squeezed, pick a random pixel and direction (back if within 10 or so of the end)
//those pixels are then in an array with 5 second timers set on all of them
//those timers decrease at the same rate
//if a pixel gets drawn on it again, its timer goes back to 5 seconds
//mix colors if anything overlaps, otherwise its straight red/blue/green all at 255

int pixelTimers[130] = { 0 };
int fullTime = 255;
int getNewColor = true;

void colorMixPaint(){

  bool redChosen = false;
  bool greenChosen = false;
  bool blueChosen = false;
  
  sensorValue = getSensorValue(SENSOR_1, 300);
  sensorValue1 = getSensorValue(SENSOR_2, 100);
  sensorValue2 = getSensorValue(SENSOR_3, 100);

  if(sensorValue > 0){
    redChosen = true;
  }else if(sensorValue1 > 0){
    greenChosen = true;
  }else if(sensorValue2 > 0){
    blueChosen = true;
  }

  if(redChosen){
    colorMixDrawer(strip.Color(255, 0, 0)); // Red
    Serial.println("Red...");
  }else if(greenChosen){
    colorMixDrawer(strip.Color(0, 255, 0)); // Green
    Serial.println("Green...");
  }else if(blueChosen){
    colorMixDrawer(strip.Color(0, 0, 255)); // Blue
    Serial.println("Blue...");
  }else{
    Serial.println("No color Drawn");
  }

  colorMixFader();
}

void colorMixDrawer(uint32_t lineColor){
  int pixel = random(0, strip.numPixels()); //0 up to 129
  int orientation = rand() % 2; // 0: 129->0 or 1: 0->129;
  
  if (pixel < 26){ orientation = 1; }
  if (pixel > 104){ orientation = 0; }
  
  int targetPixel;
  
  if(orientation == 1){ 
    targetPixel = random(pixel + 10, strip.numPixels());
  }
  if(orientation == 0){
    targetPixel = random(0, pixel - 10);
  }

  //  mix existing colors:
  //  get existing color of that pixel, separate out those colors, add in new color, set pixel color
  if(orientation == 1){
    for(int i = pixel; i < targetPixel; i++){
      strip.setPixelColor(i, lineColor);
      pixelTimers[i] = fullTime;
      strip.show();
      delay(10);
    }
  }else{
    for(int i = pixel; i > targetPixel; i--){
      strip.setPixelColor(i, lineColor);
      pixelTimers[i] = fullTime;
      strip.show();
      delay(10);
    }
  }
}

void colorMixFader(){
  for(int i = 0; i < strip.numPixels(); i++){
    pixelTimers[i]--;
    if(pixelTimers[i] <= 0){
      pixelTimers[i] = 0;
    }
    if(splitColor(strip.getPixelColor(i), 'r') > 0){
      strip.setPixelColor(i, strip.Color(pixelTimers[i], 0, 0));
    }else if(splitColor(strip.getPixelColor(i), 'g') > 0){
      strip.setPixelColor(i, strip.Color(0, pixelTimers[i], 0));
    }else if(splitColor(strip.getPixelColor(i), 'b') > 0){
      strip.setPixelColor(i, strip.Color(0, 0, pixelTimers[i]));
    }
  }
  strip.show();
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
      wait = 5;
   }
   return wait;
}

// higher pressure --> ligther color
int getColorFromPressure(int sensorValue, int currentColor) {
  int nextColor = currentColor;
  if(sensorValue <= 800){
    nextColor += 1;
  }else if(800 < sensorValue && sensorValue <= 890){
    nextColor += 3;
  }else if(890 < sensorValue && sensorValue <= 980){
    nextColor += 5;
  }else if(980 < sensorValue){
    nextColor += 7;
  }
  return nextColor;
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
  if (value > oldMax) {
    return 255;
  }
  if (value < oldMin) {
    return 0;
  }
  float oldRange = oldMax - oldMin; 
  float newRange = 255;
  return (((value - oldMin) * newRange) / oldRange);
}

float putInRange(float value, float oldMin, float oldMax, float newMin, float newMax){
  if (value > oldMax) {
    return newMax;
  }
  if (value < oldMin) {
    return newMin;
  }
  float oldRange = oldMax - oldMin; 
  float newRange = newMax - newMin;
  return (((value - oldMin) * newRange) / oldRange) + newMin;
}

// Sets all lights on the strip to color c,
// Then shows the strip
void setAllLights(uint32_t c) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, c);
  }
  strip.show();
}

//Transitions to new color newColor at speed jumpVal 
//  jumpVal = how many color values to move through on its way
//  Take in new color, jump value (lower = Faster transition)
void transitionAllLights(uint32_t newColor, uint32_t currentColor, int jumpVal) {
  int newRed = splitColor(newColor, 'r');
  int newBlue = splitColor(newColor, 'b');
  int newGreen = splitColor(newColor, 'g');
  Serial.println("Goal r:" + (String)newRed + " g:" + (String)newGreen + " b:" + (String)newBlue);
  
  int currRed = splitColor(currentColor, 'r');
  int currBlue = splitColor(currentColor, 'b');
  int currGreen = splitColor(currentColor, 'g');

  int redDiff = abs(currRed - newRed);
  int blueDiff = abs(currBlue - newBlue);
  int greenDiff = abs(currGreen - newGreen);

  int redJump = ((redDiff / jumpVal) == 0) ? 1 : (redDiff / jumpVal);
  int blueJump = ((blueDiff / jumpVal) == 0) ? 1 : (blueDiff / jumpVal);
  int greenJump = ((greenDiff / jumpVal) == 0) ? 1 : (greenDiff / jumpVal);

  bool one = false;
  bool two = false;
  bool three = false;
  while(1){
    if(currRed < newRed){
      currRed += redJump;
      if(currRed > newRed){
        currRed = newRed;
      }
    }else if(currRed > newRed){
      currRed -= redJump;
      if(currRed < newRed){
        currRed = newRed;
      }
    }else{
      one = true;
    }
    
    if(currBlue < newBlue){
      currBlue += blueJump;
      if(currBlue > newBlue){
        currBlue = newBlue;
      }
    }else if(currBlue > newBlue){
      currBlue -= blueJump;
      if(currBlue < newBlue){
        currBlue = newBlue;
      }
    }else{
      two = true;
    }
    
    if(currGreen < newGreen){
      currGreen += greenJump;
      if(currGreen > newGreen){
        currGreen = newGreen;
      }
    }else if (currGreen > newGreen){
      currGreen -= greenJump;
      if(currGreen < newGreen){
        currGreen = newGreen;
      }
    }else{
      three = true;
    }

    if(one && two && three){
      break;
    }
    setAllLights(strip.Color(currRed, currGreen, currBlue));
    Serial.println("Current r:" + (String)currRed + " g:" + (String)currGreen + " b:" + (String)currBlue);
  }
}

//splits color into r g or b value
uint8_t splitColor ( uint32_t c, char value ){
  switch ( value ) {
    case 'r': return (uint8_t)(c >> 16);
    case 'g': return (uint8_t)(c >>  8);
    case 'b': return (uint8_t)(c >>  0);
    default:  return 0;
  }
}

String make16Chars(String input){
  for(int i = input.length(); i < 16; i++){
      input.concat(" ");
  }
  return input;
}

