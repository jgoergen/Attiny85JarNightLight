// Don't forget to add the ATTINY85 board to your ide. You'll need to add this to your preferences -> additional board manager urls
// https://raw.githubusercontent.com/damellis/attiny/ide-1.6.x-boards-manager/package_damellis_attiny_index.json
// More info on working with the ATTINY85 here: http://highlowtech.org/?p=1695

#include <tinySPI.h> // https://github.com/JChristensen/tinySPI

// PINS ////////////////////////////////////////////////////////////////////

#define LATCH_PIN               3 // ATTINY85 pin 2 / 74HC595 pin 12 (ST_CP)
#define DATA_PIN                1 // ATTINY85 pin 6 / 74HC595 pin 14 (DS)
#define CLOCK_PIN               2 // ATTINY85 pin 7 / 74HC595 pin 11 (SH_CP)
#define OUTPUT_ENABLE_PIN       0 // ATTINY85 pin 5 / 74HC595 pin 13 (OE)
#define LIGHT_DETECTOR_PIN      A2 // light detecting resistor to positive pin on attiny85 ( neesd a resistor from ground to A2 as well. )

// SETTINGS ////////////////////////////////////////////////////////////////

#define MAX_LOOP_DELAY_SPEED    40 // the maximum speed ( picked at random between min and ma ) to fade the leds
#define MIN_LOOP_DELAY_SPEED    4 // the minimum speed ( picked at random between min and max ) to fade the leds
#define MAX_BRIGHTNESS_PWM      255 // the minimum brightness of the leds when faded all the way out ( inverted, so 255 - this value )
#define MIN_BRIGHTNESS_PWM      100 // the maximum brightness of the leds when faded all the way in ( inverted, so 255 - this value )
#define MAX_WAIT_BETWEEN_LEDS   6000 // the maximum time to wait between lighting leds
#define MAX_LIGHTS_AT_A_TIME    1 // the maximum amount of leds that will be lit up at a time
#define LIGHT_COUNT             8 // the amouunt of lights connected
#define USE_LIGHT_SENSOR        0 // 1 to use a photoressitor to turn it off when some amount of ambient light is detected
#define MIN_LIGHT_BEFORE_DISABLE 40 // out of 1024, beyond this point the light turns off

// VARIABLES ////////////////////////////////////////////////////////////////

int pwmFreq = 123;
int pwmFreqChangeSpeed = 1;
uint8_t currentLEDS = 0b0001000;
int loopDelay = MIN_LOOP_DELAY_SPEED;
int lightCount = 1;

void setup() {
  
  randomSeed(analogRead(4));
  pinMode(OUTPUT_ENABLE_PIN, OUTPUT);  
  SPI.begin();
  pinMode(LATCH_PIN, OUTPUT); 
  digitalWrite(LATCH_PIN, HIGH);

  if (USE_LIGHT_SENSOR == 1)
    pinMode(LIGHT_DETECTOR_PIN, INPUT);
  
  changeLED();
}

void loop() {

  // if there's too much light, stop it
  if (USE_LIGHT_SENSOR == 1 && analogRead(LIGHT_DETECTOR_PIN) > MIN_LIGHT_BEFORE_DISABLE) {

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
      (float)MIN_BRIGHTNESS_PWM, // begInnIng value
      (float)(MAX_BRIGHTNESS_PWM - MIN_BRIGHTNESS_PWM), // change In value
      (float)MAX_BRIGHTNESS_PWM); // duration
      
  analogWrite(OUTPUT_ENABLE_PIN, easedPWM);

  if (pwmFreq >= MAX_BRIGHTNESS_PWM) {

    pwmFreqChangeSpeed *= -1;    
    changeLED();   
    
  } else if (pwmFreq <= MIN_BRIGHTNESS_PWM) {

    pwmFreqChangeSpeed *= -1; 
  }  
}

void changeLED() {
  
  lightCount = random(MAX_LIGHTS_AT_A_TIME) + 1;
  currentLEDS = 0b11111111; //0b00000000;
  
  for (int i = 0; i < lightCount; i++)
    currentLEDS = currentLEDS | 1 << random(LIGHT_COUNT);
  
  loopDelay = MIN_LOOP_DELAY_SPEED + random(MAX_LOOP_DELAY_SPEED);
  delay(random(MAX_WAIT_BETWEEN_LEDS));
}

// t: current time, b: begInnIng value, c: change In value, d: duration
float easeInOutQuad (float t, float b, float c, float d) {

  if ((t /= d / 2) < 1) 
    return c / 2 * t * t + b;
    
  return -c / 2 * ((--t) * (t - 2) - 1) + b;
}

