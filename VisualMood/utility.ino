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
int getSensorValue_All(int minValue){
  int sensorVal = analogRead(SENSOR_1);
  //int sensorVal2 = analogRead(SENSOR_2);
  int sensorVal2 = 0;
  int sensorVal3 = analogRead(SENSOR_3);
  int max1 = max(sensorVal, sensorVal2);
  int finalMax = max(max1, sensorVal3);
  if (finalMax < minValue){
    return 0.0;
  }else{
    return finalMax;
  }
}

// higher pressure --> lower delay
int getDelayFromPressure(int sensorValue) {
  int wait;
  if (sensorValue <= currentDiff.getLowLevel()){
      wait = 80;
   }else if (currentDiff.getLowLevel() < sensorValue && sensorValue <= currentDiff.getMediumLevel()){
      wait = 40;
   }else if (currentDiff.getMediumLevel() < sensorValue && sensorValue <= currentDiff.getHighLevel()){
      wait = 20;
   }else if (currentDiff.getHighLevel() < sensorValue) {
      wait = 5;
   }
   return wait;
}

// higher pressure --> ligther color
int getColorFromPressure(int sensorValue, int currentColor) {
  int nextColor = currentColor;
  if(sensorValue <= currentDiff.getLowLevel()){
    nextColor += 1;
  }else if(currentDiff.getLowLevel() < sensorValue && sensorValue <= currentDiff.getMediumLevel()){
    nextColor += 3;
  }else if(currentDiff.getMediumLevel() < sensorValue && sensorValue <= currentDiff.getHighLevel()){
    nextColor += 5;
  }else if(currentDiff.getHighLevel() < sensorValue){
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
