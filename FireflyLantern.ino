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
float RATE = 5;  // average number events per second; cant go below 1!

//Firefly firefly[NFF];

// return float between 0 and 1, innclusive
float randf() {
  return (float)random(RAND_MAX) / (float)RAND_MAX;
  // return (float) random() / (RAND_MAX + 1); //causes overflow??
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
// int poissonInterval(float rate) {
//   double L = exp(-rate); // lambda, rate in exp
//   // Serial.print("rate: ");
//   // Serial.println(rate);
//   // Serial.print("exp rate: ");
//   // Serial.println(L);
//   double p = 1;
//   int k = 0;
//   while (p > L) {
//     k++;
//     float f = randf();
//     p *= f; // scale 1 by [0,1] until it is higher than threshold lambda
//     // Serial.print(f);
//     // Serial.print(" ");
//     // Serial.println(p);
//   }  
//   Serial.print("interval: ");
//   Serial.println(k-1);
//   return k-1; // give the integer of how long it took to cross threshold
// }


// poisson random interval
// returns float with next interval with mean rate of rateParameter per second
// 
// from this method: http://preshing.com/20111007/how-to-generate-random-timings-for-a-poisson-process/
// finds inverse poisson given uniform distribution between 0 and 1
float poissonInterval(float rateParameter) {
  float p = -log(1.0f - randf()) / rateParameter;
  // Serial.print(" interval: ");
  // Serial.println(p);
    return p;
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
  Serial.println("------");
}

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

static float eventInterval = poissonInterval(RATE);

void loop() {

  // how can i change to reduce by time?
  leds.fadeToBlackBy(50); // reduce by percent of prior value

  static long int lastt = millis();
  int dt = millis() - lastt;
  // Serial.print("frame time: ");
  // Serial.print(dt);
  // Serial.print(" interval: ");
  // Serial.println(eventInterval);

  if (dt > eventInterval * 1000) {
    lastt = millis();
    leds[random8(NUM_LEDS)] = CRGB::Yellow;
    // leds[random8(NUM_LEDS)].setHSV(random8(), 255, 255);
    eventInterval = poissonInterval(RATE);
    // Serial.print("interval: ");
    // Serial.println(eventInterval);
  }
  
  FastLED.show();
  delayToSyncFrameRate(FPS);



}


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

/*
// to update interval
// you’d need to keep the “full” value of the pixel stored somewhere (so that you aren’t
//  unscaling to try to get the scaling right for any given frame) - and for a given pixel, 
//  record a start time (when it was full) and end time (when it should be black) - then you
//  can get the total time and elapsed time for a given pixel - (256 * elapsed/total) gives
//   you a value to use for fadeToBlackBy against the full value of the pixel.
*/