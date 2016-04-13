#include <Time.h>
#include <TimeLib.h>

#include <Adafruit_NeoPixel.h>
#include <LiquidCrystal.h>

#define PIN 6
#define METER_PIN 13
//#define TRAINSENSOR A3

#define SENSOR_1 A0 // Sensor_1 is used for all single controlled Modes
// #define SENSOR_2 A1
#define SENSOR_3 A2 // SENSOR_3/A2 will be TRAINSENSOR now!

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
Adafruit_NeoPixel meter = Adafruit_NeoPixel(10, METER_PIN, NEO_GRB + NEO_KHZ800);


//float voltage = 0.0;
//Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V):
//voltage = sensorValue * (5.0 / 1023.0);
int sensorValue = 0; // Weak Sensor
int sensorValue1 = 0; // Strong Sensor
int sensorValue2 = 0; // Weak Sensor

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

enum DifficultyLevel{
  Low,
  Medium,
  High
};

class Difficulty{
private:
  DifficultyLevel currentDiff;
  float high_High = 1020;
  float high_Med = 980;
  float high_Low = 750;
  
  float med_High = 980;
  float med_Med = 800;
  float med_Low = 650;

  float low_High = 900;
  float low_Med = 725;
  float low_Low = 600;
  
public:
  Difficulty(){
    currentDiff = Low;
  } 
  void cycleDiff(){
    if(currentDiff == Low){
      currentDiff = Medium;
    }else if(currentDiff == Medium){
      currentDiff = High;
    }else if(currentDiff == High){
      currentDiff = Low;
    }
  }
  String getDifficulty(){
    if(currentDiff == Low){
      return "Low";
    }else if(currentDiff == Medium){
      return "Medium";
    }else if(currentDiff == High){
      return "High";
    }
  }
  float getHighLevel(){
    if(currentDiff == Low){
      return low_High;
    }else if(currentDiff == Medium){
      return med_High;
    }else if(currentDiff == High){
      return high_High;
    }
  }
  float getMediumLevel(){
    if(currentDiff == Low){
      return low_Med;
    }else if(currentDiff == Medium){
      return med_Med;
    }else if(currentDiff == High){
      return high_Med;
    }
  }
  float getLowLevel(){
    if(currentDiff == Low){
      return low_Low;
    }else if(currentDiff == Medium){
      return med_Low;
    }else if(currentDiff == High){
      return high_Low;
    }
  }
};

namespace smoothOP
{ 
  int curTime = 0;
  int lastTime = second(now());
  int lastMarker = second(now());
  int speedInterval = 0; // seconds of time requirement for transition
 
  uint32_t curColor = 0;
  int red = 0;
  int blue = 0;
  int green = 0;
}


enum LightMode {
  off, 
  simple,
  smoothMove,
  maxOut,
  pressure2x,
  painting, 
  colorWave,
  ripple,
  rainbow
}; 

Difficulty currentDiff;
LightMode currentMode;

// declaration of functions and classes from other project files, code not able to compile otherwise
// utility.ino functions
int getSensorValue(uint8_t pin, int minValue);
int getSensorValue(uint8_t pin);
int getColorFromPressure(int sensorValue, int currentColor);
uint32_t Wheel(byte WheelPos);
uint32_t toRGB(uint32_t inVal);
float putInRange(float value, float oldMin, float oldMax);
void setAllLights(uint32_t c);
void transitionAllLights(uint32_t newColor, uint32_t currentColor, int jumpVal);
uint8_t splitColor ( uint32_t c, char value );
String make16Chars(String input);

// soothing.ino functions
void doublePressure();
void colorWithPressure();
void rainbowWithPressure();
void rainbowCycle();
void rippleEffect();
void colorMixWave();
void colorMixPaint();
void colorMixDrawer(uint32_t lineColor);
void colorMixFader();

// training.ino functions
void smoothOperator();
void goingUP();
void goingDOWN();
uint32_t increaseColor();
uint32_t decreaseColor();
void maxOutTraining();

void setup() {
  Serial.begin(9600); //bits per second
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  meter.begin();
  meter.show();
  pinMode(modeButtonPin, INPUT);
  pinMode(optionButtonPin, INPUT);
  currentMode = off;
  Serial.println(currentMode);
  lcd.begin(16, 2); // set up the LCD's number of columns and rows:
  lcd.setCursor(0, 1);
  lcd.print(make16Chars("Diff: " + currentDiff.getDifficulty()));
}

void loop() {
  optionButtonState = digitalRead(optionButtonPin);
  if (!optionButtonPushed && optionButtonState == HIGH) {
    optionButtonPushed = true;
    currentDiff.cycleDiff();
    lcd.setCursor(0, 1);
    lcd.print(make16Chars("Diff: " + currentDiff.getDifficulty()));
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
  lcd.setCursor(0, 0);
  switch (currentMode){
    case (off):
      lcd.print(make16Chars("LED OFF"));
      setAllLights(strip.Color(0, 0, 0));
      break;
    case (simple):
      lcd.print(make16Chars("Simple"));
      colorWithPressure();
      break;
    case (smoothMove):
      lcd.print(make16Chars("Training"));
      smoothOperator();
      break;
    case (maxOut):
      lcd.print(make16Chars("Max Out"));
      maxOutTraining();
      break;
    case (pressure2x):
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

