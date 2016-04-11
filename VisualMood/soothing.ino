void doublePressure(){
  sensorValue = getSensorValue(SENSOR_1, 100);
  sensorValue1 = getSensorValue(SENSOR_2, 100);
  sensorValue2 = getSensorValue(SENSOR_3, 100);

  float red = putInRange(sensorValue, 100, currentDiff.getHighLevel());
  float green = putInRange(sensorValue1, 100, currentDiff.getHighLevel());
  float blue = putInRange(sensorValue2, 100, currentDiff.getHighLevel());
  
  int jump = 15;
  uint32_t currentColor = strip.getPixelColor(129);
  transitionAllLights(strip.Color(red, green, blue), currentColor, jump);
}

// Moves between colors by mixing them as pressure changes 
void colorWithPressure(){
  sensorValue = getSensorValue_All(100);
  
  uint32_t currentColor = strip.getPixelColor(129);
  int jump = 15; //Smaller = faster transistions
  if(sensorValue > currentDiff.getLowLevel() && sensorValue <= currentDiff.getMediumLevel()){
    float green = putInRange(sensorValue, currentDiff.getLowLevel(), currentDiff.getMediumLevel());
    transitionAllLights(strip.Color(0, green, 0), currentColor, jump);
  }else if(currentDiff.getMediumLevel() < sensorValue && sensorValue <= currentDiff.getHighLevel()){
    float blue = putInRange(sensorValue, currentDiff.getMediumLevel(), currentDiff.getHighLevel());
    float green = 255 - blue;
    transitionAllLights(strip.Color(0, green, blue), currentColor, jump);
  }else if(currentDiff.getHighLevel() < sensorValue){
    float red = putInRange(sensorValue, currentDiff.getHighLevel(), 1023);
    float blue = 255 - red;
    transitionAllLights(strip.Color(red, 0, blue), currentColor, jump);
  }
  else {
    transitionAllLights(strip.Color(0, 0, 0), currentColor, jump);
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
    sensorValue = getSensorValue_All(100);
    uint8_t wait = getDelayFromPressure(sensorValue);

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
      rainbowStyle = 1;
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
    sensorValue = getSensorValue_All(100);
    uint8_t wait = getDelayFromPressure(sensorValue);

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
      rainbowStyle = 0;
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
      sensorValue = getSensorValue_All(100);
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
    red = putInRange(sensorValue, 100, currentDiff.getHighLevel());
  }if(sensorValue1 > 0){
    green = putInRange(sensorValue1, 100, currentDiff.getHighLevel());
  }if(sensorValue2 > 0){
    blue = putInRange(sensorValue2, 100, currentDiff.getHighLevel());
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
