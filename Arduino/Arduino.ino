
#include <tinySPI.h> // https://github.com/JChristensen/tinySPI

#define LATCH_PIN  3 // 74HC595 - ST_CP / 12
#define DATA_PIN   1 // 74HC595 - DS / 14
#define CLOCK_PIN  2 // 74HC595 - SH_CP / 11
#define OUTPUT_ENABLE_PIN     0 // 74HC595 - OE / 13
#define LIGHT_DETECTOR_PIN    A2 // light detecting resistor to positive pin on attiny85

// SETTINGS ////////
int maxLoopDelay = 40;
int minLoopDelay = 4;
int maxPWMFreq = 130;
int minPWMFreq = 0;
int maxWaitBetweenLEDS = 6000;
int maxLightsAtATime = 1;
int minLightBeforeShutoff = 300; // out of 1024, point at which the light turns off
//////////////////////

int pwmFreq = 123;
int pwmFreqChangeSpeed = 1;
uint8_t currentLEDS = 0b0001000;
int loopDelay = minLoopDelay;
int lightCount = 1;

void setup() {
  
  randomSeed(analogRead(4));
  pinMode(OUTPUT_ENABLE_PIN, OUTPUT);  
  pinMode(LIGHT_DETECTOR_PIN, INPUT);
  SPI.begin();
  pinMode(LATCH_PIN, OUTPUT); 
  digitalWrite(LATCH_PIN, HIGH);
  changeLED();
}

void loop() {

  // if there's too much light, stop it
  if (analogRead(LIGHT_DETECTOR_PIN) > minLightBeforeShutoff) {

    digitalWrite(LATCH_PIN, LOW);
    SPI.transfer(0b00000000);
    digitalWrite(LATCH_PIN, HIGH);
    analogWrite(OUTPUT_ENABLE_PIN, 0);
    delay(1000);
    return;
  }

  digitalWrite(LATCH_PIN, LOW);
  SPI.transfer(currentLEDS);
  digitalWrite(LATCH_PIN, HIGH);
  updatePWM();
  delay(loopDelay);
}

void updatePWM() {

  pwmFreq += pwmFreqChangeSpeed;
  
  int easedPWM = 
    (int)easeInOutQuad(
      (float)pwmFreq, // current time
      (float)minPWMFreq, // begInnIng value
      (float)(maxPWMFreq - minPWMFreq), // change In value
      (float)maxPWMFreq); // duration
      
  analogWrite(OUTPUT_ENABLE_PIN, easedPWM);

  if (pwmFreq >= maxPWMFreq) {

    pwmFreqChangeSpeed *= -1;    
    changeLED();   
    
  } else if (pwmFreq <= minPWMFreq) {

    pwmFreqChangeSpeed *= -1; 
  }  
}

void changeLED() {
  
  lightCount = random(maxLightsAtATime) + 1;
  currentLEDS = 0b00000000;
  
  for (int i = 0; i < lightCount; i++)
    currentLEDS = currentLEDS | 1 << random(8);
  
  loopDelay = minLoopDelay + random(maxLoopDelay);
  delay(random(maxWaitBetweenLEDS));
}

// t: current time, b: begInnIng value, c: change In value, d: duration
float easeInOutQuad (float t, float b, float c, float d) {

  if ((t /= d / 2) < 1) 
    return c / 2 * t * t + b;
    
  return -c / 2 * ((--t) * (t - 2) - 1) + b;
}

