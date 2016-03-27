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

// declaration of functions and classes from other project files, code not able to compile otherwise
// utility.ino functions
int getSensorValue(uint8_t pin, int minValue);
int getSensorValue(uint8_t pin);
int getColorFromPressure(int sensorValue, int currentColor);
uint32_t Wheel(byte WheelPos);
uint32_t WheelWithBreathe(byte WheelPos, float percent);
uint32_t toRGB(uint32_t inVal);
float putInRange(float value, float oldMin, float oldMax);
void setAllLights(uint32_t c);
void transitionAllLights(uint32_t newColor, uint32_t currentColor, int jumpVal);
uint8_t splitColor ( uint32_t c, char value );
String make16Chars(String input);

// soothing.ino functions
void doublePressure();
void colorWithPressure();
void breatheEffectLoop();
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

