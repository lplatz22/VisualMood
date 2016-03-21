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
    currentUser++;
    currentUser = currentUser % totalUsers;
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
    case (ripple):
      lcd.print(make16Chars("Ripple"));
      rippleEffect();
      break;
    case (rainbow):
      lcd.print(make16Chars("Rainbow"));
      rainbowWithPressure();
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
  setAllLights(strip.Color(red, green, blue));
}

// Moves between colors by mixing them as pressure changes 
// instead of sharply transitioning
void colorWithPressure(){
  sensorValue = getSensorValue(SENSOR_1, 400);
  
  if(sensorValue > 850 && sensorValue <= 945){
    float green = putInRange(sensorValue, 850, 945);
    setAllLights(strip.Color(0, green, 0)); // Green
  }else if(945 < sensorValue && sensorValue <= 995){
    float blue = putInRange(sensorValue, 945, 995);
    float green = 255 - blue;
    setAllLights(strip.Color(0, green, blue)); // Blue
  }else if(995 < sensorValue){
    float red = putInRange(sensorValue, 995, 1020);
    float blue = 255 - red;
    setAllLights(strip.Color(red, 0, blue)); // Red
  }
  else {
    setAllLights(strip.Color(0, 0, 0));
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
  for(j=0; j<256; j++) {
    for(i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel((i+j) & 255));
    }
    strip.show();
    sensorValue = getSensorValue(SENSOR_1);
    uint8_t wait = getDelayFromPressure(sensorValue);
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
    modeButtonState = digitalRead(modeButtonPin);
    if (!modeButtonPushed && modeButtonState == HIGH) {
      modeButtonPushed = true;
      currentMode = rainbow; // BUG: Workaround, will work as long as simple is first, last is rainbow
      Serial.println("Pressed!");
      Serial.println(currentMode);
      break;
    } else if (modeButtonState == LOW) {
      modeButtonPushed = false;
    }
    delay(wait);
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
  
  if(users[currentUser]->getHighPressure() < (sensorValue/1023.0 * 22.0)){
      users[currentUser]->setHighPressure((sensorValue/1023.0 * 22.0));
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

// Sets all lights on the strip to color c.
void setAllLights(uint32_t c) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, c);
  }
  strip.show();
}

String make16Chars(String input){
  for(int i = input.length(); i < 16; i++){
      input.concat(" ");
  }
  return input;
}

