// Smooth Loop for pressure transitions:
void smoothOperator() {
  uint32_t sensorValue = getSensorValue(SENSOR_1);
  // This below is a timer that resets if below the lower level, and increases to 10 seconds. 
  if (sensorValue < currentDiff.getLowLevel()) {
    smoothOP::curTime = 0;
  } else {
    if (smoothOP::lastTime != smoothOP::lastMarker) {
      if (smoothOP::curTime < 10) {
        smoothOP::curTime++;
      }
      smoothOP::lastMarker = second(now());
      smoothOP::lastTime = second(now());
    } else {
      smoothOP::lastTime = second(now());
    }
  }
  
  uint32_t curColor = strip.getPixelColor(129);
  uint32_t targetColor = smoothOP::curTime * MAX24 / 10;

  if (targetColor > (255 | (255 << 8))) {
    targetColor = targetColor | 255 | (255 << 8);
  }
  
  if (targetColor > 255) {
    targetColor = targetColor | 255;
  }
  Serial.println("TargetColor: ");
  Serial.println(targetColor);
  transitionAllLights(targetColor, curColor, 15);
  
}



void maxOutTraining(){
  sensorValue = getSensorValue_All(100);
  float meterHeight = putInRange(sensorValue, 100, currentDiff.getHighLevel(), 0, 130);
  for (int i = 0; i < meterHeight; i++){
    if(i <= 40){
      strip.setPixelColor(i, strip.Color(0,255,0));
    }else if (i > 40 && i <= 100){
      strip.setPixelColor(i, strip.Color(255,255,0));
    }else if (i > 100){
      strip.setPixelColor(i, strip.Color(255,0,0));
    }
    strip.show();

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
      currentMode = pressure2x;
      Serial.println("Pressed!");
      Serial.println(currentMode);
      break;
    } else if (modeButtonState == LOW) {
      modeButtonPushed = false;
    }
  }
  for (int i = strip.numPixels() - 1; i >= meterHeight; i--){
    strip.setPixelColor(i, strip.Color(0,0,0));
    strip.show();

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
      currentMode = pressure2x;
      Serial.println("Pressed!");
      Serial.println(currentMode);
      break;
    } else if (modeButtonState == LOW) {
      modeButtonPushed = false;
    }
  }
}
