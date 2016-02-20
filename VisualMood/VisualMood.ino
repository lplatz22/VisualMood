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

int currentColor = 0;
int sensorValue = 0;

// start with increasing intensity in the color
// once breatheTimer reaches 500, we start to decrease color intensity
int breatheTimer = 0;
bool increaseIntensity = true;

void setup() {
  Serial.begin(9600); //bits per second
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
}

void loop() {
    //----Pressure Sensor----
    // Read the input on analog pin 0:
    sensorValue = analogRead(A0);
    //341 is middle point of all color transitions
    //0-341 = green 0-255
    //341-682 = green 255-0, blue 0-255
    //682-1023 = blue 255-0, red 0-255
    
    if(sensorValue <= 341){
      float green = ((sensorValue % 341) * (765 / 1023));
      setAllLights(strip.Color(0, green, 0)); // Green
    }else if(341 < sensorValue && sensorValue <= 682){
      float blue = (sensorValue % 341) * (765.0 / 1023.0);
      float green = 255 - blue;
      setAllLights(strip.Color(0, green, blue)); // Blue
    }else if(682 < sensorValue){
      float red = (sensorValue % 341) * (765.0 / 1023.0);
      float blue = 255 - red;
      setAllLights(strip.Color(red, 0, blue)); // Red
    }



//    if (sensorValue < 800 && currentColor != 0) {
//      setAllLights(strip.Color(0, 0, 0)); // Off
//      currentColor = 0;
//    }else if(800 <= sensorValue && sensorValue < 900 && currentColor != 1){
//      setAllLights(strip.Color(0, 255, 0)); // Green
//      currentColor = 1;
//    }else if(900 <= sensorValue && sensorValue < 990 && currentColor != 2){
//      setAllLights(strip.Color(0, 0, 255)); // Blue
//      currentColor = 2;
//    }else if(sensorValue >= 990 && currentColor != 3){
//      setAllLights(strip.Color(255, 0, 0)); // Red
//      currentColor = 3;
//    }
}

void breatheEffectLoop() {
    //----Pressure Sensor----
    // Read the input on analog pin 0:
    sensorValue = analogRead(A0);

    // voltage range is 0 - 5
    int voltage = sensorValue * (5.0 / 1023.0);

    if (increaseIntensity) {
      breatheTimer++;
    }
    else {
      breatheTimer--;
    }

    // start decreasing intensity of lights from here on out
    if (breatheTimer == 500) {
      increaseIntensity = false;
    }
    // start increasing intensity of lights from here on out
    else if (breatheTimer == 0) {
      increaseIntensity = true;
    }

    // max brightness (255) * percentage of breatheTimer we've gone through
    float colorIntensity = 255 * (breatheTimer / 500.0);

    // green
    if(voltage < 4.5){
      setAllLights(strip.Color(0, colorIntensity, 0));
    }
    // blue
    else if(voltage < 4.9){
      setAllLights(strip.Color(0, 0, colorIntensity));
    }
    // red
    else {
      setAllLights(strip.Color(colorIntensity, 0, 0));
    }
}

// Fill the dots one after the other with a color
void setAllLights(uint32_t c) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, c);
  }
  strip.show();
}


