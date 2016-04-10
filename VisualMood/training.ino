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

void maxOutTraining(){
  sensorValue = getSensorValue_All(100);
  float meterHeight = putInRange(sensorValue, 100, currentDiff.getHighLevel(), 0, 130);

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
}
