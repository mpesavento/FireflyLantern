#include <FastLED.h>
#include "firefly.c"

#define DATA_PIN   11 // SPI MOSI pin
#define CLOCK_PIN  13 //13 //SPI  SCK

//#define COLOR_ORDER BGR  // most of the 10mm black APA102
#define COLOR_ORDER GBR //mike's short test strip, 19 pixels

#define CHIPSET     APA102
#define NUM_LEDS    19

#define FPS 100

// CRGB leds[NUM_LEDS];
CRGBArray<NUM_LEDS> leds;

CRGBPalette16 gPal;

int brightness = 255;

#define NFF 5  // number of fireflies
float RATE = 1.5;  // average interval between events per second

//Firefly firefly[NFF];

int randInterval(int rate) {
  return random8(rate - 10, rate + 10);
}

// return float between 0 and 1, innclusive
float randf() {
  return (float)random(RAND_MAX) / (float)RAND_MAX;
}

// poisson random interval
// the interval at which the next poisson event occurs
//
// rate: number of events per unit time.
// it will need to be scaled as appropriate for whatever time unit s you are using
// eg can be scaled as 100/4, or 100 events every quarter second.
//
// taken from Donald Kknuth
// http:#en.wikipedia.org/wiki/Poisson_distribution#Generating_Poisson-distributed_random_variables
// this method is good for short rate values (< 30)
int poissonInterval(float rate) {
  double L = exp(-rate); // lambda, rate in exp
  Serial.print("rate: ");
  Serial.println(rate);
  Serial.print("exp rate: ");
  Serial.println(L);
  double p = 1;
int k = 0;
  while (p > L) {
    k++;
    float f = randf();
    Serial.print(f);
    Serial.print(" ");
    p *= f; // scale 1 by [0,1] until it is higher than threshold lambda
    Serial.println(p);
  }
  Serial.println(k-1);
  return k-1; // give the integer of how long it took to cross threshold
}



void setup() {
  //pinMode(buttonPin, INPUT); // where will I get the button pin values from? probaly the hardware.h
  delay(2000); // sanity delay
  randomSeed(analogRead(0));

  FastLED.addLeds<CHIPSET, DATA_PIN, CLOCK_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalSMD5050 );
  FastLED.setBrightness( brightness );

  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  Serial.println("loaded");
  Serial.println(RAND_MAX);
}



void loop() {
  // static int blinkRate = 0.5 * 1000;  // blinks per second
   static int eventInterval = 0;
   eventInterval = poissonInterval(RATE) * 1000; // get delay in millis

  // how can i change to reduce by time?
  leds.fadeToBlackBy(50); // reduce by percent of prior value

  static long int lastt = millis();
  int dt = millis() - lastt;
  if (dt > eventInterval) {
    lastt = millis();
    leds[random8(NUM_LEDS)] = CRGB::Yellow;
    // leds[random8(NUM_LEDS)].setHSV(random8(), 255, 255);
  }
  
  FastLED.show();
  Serial.println(".");
  delayToSyncFrameRate(FPS);



}

/*
void loop() {

  // firefly objects decay by given rate
  // leds.fadeToBlackBy(40);

  // leds[3] = CRGB::Blue;
  leds[3].setHSV( random8(), 255, 255);

  // for (int i=0; i<NUM_LEDS; i++)
  //   leds[i] = CRGB::Blue;

  // updateFireflies(); // update the frame

  static int eventInterval = poissonInterval(RATE) * 1e3; // get delay in millis
  static uint64_t lastt = millis();
  Serial.println((int)lastt);

  FastLED.show();
  delayToSyncFrameRate(FPS);

}
*/


/*
void updateFireflies() {
  static int eventInterval = poissonInterval(RATE) * 1e3; // get delay in millis
  static uint64_t lastt = millis();
  Serial.print((int)lastt);
  Serial.print(" ");
  Serial.print(eventInterval);
  if (millis() - lastt >= eventInterval) {
    Serial.print(".");

    //Serial.println(millis() - lastt);
    // turn on randomly selected LED
    int pos = random8(NUM_LEDS);
    leds[pos] = CRGB::Yellow;

    // set new interval
    eventInterval = poissonInterval(RATE) * 1e3; // rate is given in Hz, scale by dt
    //Serial.println(eventInterval);
    lastt = millis();
  }

}
*/

// delayToSyncFrameRate - delay how many milliseconds are needed
//   to maintain a stable frame rate.
static void delayToSyncFrameRate( uint8_t framesPerSecond) {
  static uint32_t msprev = 0;
  uint32_t mscur = millis();
  uint16_t msdelta = mscur - msprev;
  uint16_t mstargetdelta = 1000 / framesPerSecond;
  if ( msdelta < mstargetdelta) {
    FastLED.delay( mstargetdelta - msdelta);
  }
  msprev = mscur;
}


/*
  void selectFirefly() {
  int i = random8(NUM_LEDS);
  firefly[0].index = i;
  firefly[0].color = CRGB::Yellow;

  }


  #define INIT_RISETIME 200  // in milliseconds
  #define INIT_FALLTIME 1000
  #define INIT_HUE 43 // of 256, yellowish
  #define INIT_BRIGHT 128  // of 256, mid level brightness
  #define INIT_SATURATION 200  // of 256

  typedef struct Firefly {
    int index;
    int risetime;
    int falltime;
    int starttime;
    uint32_t color;
  } Firefly;

  void initFirefly(Firefly* f) {
    f->index = 0;
    f->risetime = INIT_RISETIME;
    f->falltime = INIT_FALLTIME;
    f->color = CRGB::Yellow;
    return;
  }

  void setFireflyParams(Firefly* f, int index, int risetime, int falltime, uint8_t maxBrightness) {
    f->index = index;
    f->risetime = risetime;
    f->falltime = falltime;
    f->maxBrightness = maxBrightness;
    return;
  }
*/

