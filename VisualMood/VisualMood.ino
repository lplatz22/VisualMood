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
}

// Fill the dots one after the other with a color
void setAllLights(uint32_t c) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, c);
  }
  strip.show();
}


