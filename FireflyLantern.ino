#include <FastLED.h>
//#include "firefly.c"

#define DATA_PIN   11 // SPI MOSI pin
#define CLOCK_PIN  13 //13 //SPI  SCK

//#define COLOR_ORDER BGR  // most of the 10mm black APA102
#define COLOR_ORDER GBR //mike's short test strip, 19 pixels

#define CHIPSET     APA102
#define FPS 100
#define NUM_LEDS    19

CRGBArray<NUM_LEDS> leds;

CRGBPalette16 currentPalette;

int brightness = 255;

#define NFF 10  // number of fireflies
float RATE = 1;  // average number events per second

// ******************************************************************
// create Firefly objects

#define INIT_DURATION 400
#define INIT_HUE 55 // of 256, yellowish
#define INIT_SATURATION 255  // of 256
#define INIT_BRIGHT 255  // of 256, mid level brightness

typedef struct Firefly {
  int index;
  uint8_t hue;
  uint8_t sat;
  uint8_t bright;
  uint8_t fullbright;
  long int starttime; // in ms
  int interval;  // poisson interval betweeen events, random process
  int elapsedt;  // time since starttime
  int duration; // time to decay to black, in ms
} Firefly;

Firefly ff[NFF];
// Firefly ff = malloc(sizeof(Firefly*) * NFF)

void initFireflies(Firefly *f) {
  for(int i=0; i < NFF; i++) {
    f[i].index = random16(NUM_LEDS); // choose which pixel to start at
    f[i].fullbright = INIT_BRIGHT;
    f[i].bright = 0;
    f[i].hue = INIT_HUE;
    f[i].sat = INIT_SATURATION;
    f[i].starttime = millis();
    f[i].interval = poissonInterval(RATE) * 1000;
    f[i].elapsedt = 0;
    f[i].duration = INIT_DURATION;
  }
}

void updateFireflies() {
  for(int i=0; i<NFF; i++) {
    // light the pixel if we've reached the interval
    if (ff[i].elapsedt > ff[i].interval) {
      printFFState(ff[i]);

      // set initial state on trigger 
      ff[i].hue = random16(10, 80);
      leds[ff[i].index].setHSV(ff[i].hue, ff[i].sat, ff[i].fullbright);

      // reset values for the next start
      ff[i].bright = ff[i].fullbright;
      ff[i].interval = poissonInterval(RATE) * 1000;
      ff[i].elapsedt = 0;
      ff[i].starttime = millis();
    }
    else if ( ff[i].elapsedt > ff[i].duration) {
      ff[i].index = random16(NUM_LEDS);
      ff[i].interval = poissonInterval(RATE) * 1000;
      ff[i].elapsedt = 0;
      ff[i].starttime = millis();
    }
    else {
      ff[i].elapsedt = millis() - ff[i].starttime;

      leds[ff[i].index].fadeToBlackBy(256 * (ff[i].elapsedt / ff[i].duration) );
    }


  }
/*
// to update interval (from Dan Garcia)
// you’d need to keep the “full” value of the pixel stored somewhere (so that you aren’t
//  unscaling to try to get the scaling right for any given frame) - and for a given pixel, 
//  record a start time (when it was full) and end time (when it should be black) - then you
//  can get the total time and elapsed time for a given pixel - (256 * elapsed/total) gives
//   you a value to use for fadeToBlackBy against the full value of the pixel.
*/
}

void printFFState(Firefly f) {
  Serial.print("i: ");
  Serial.print(f.index);
  Serial.print(" intvl: ");
  Serial.print(f.interval);
  Serial.print(" dt: ");
  Serial.print(f.elapsedt);
  Serial.print("\n");

}

// ******************************************************************
// create poisson interval

// return float between 0 and 1, innclusive
float randf() {
  return (float)random(RAND_MAX) / (float)RAND_MAX;
  // return (float) random() / (RAND_MAX + 1); //causes overflow??
}

// poisson random interval
// returns float with next interval with mean rate of rateParameter per second
// 
// from this method: http://preshing.com/20111007/how-to-generate-random-timings-for-a-poisson-process/
// finds inverse poisson given uniform distribution between 0 and 1
float poissonInterval(float rateParameter) {
  float p = -log(1.0f - randf()) / rateParameter;
  return p;
}

// ******************************************************************
// ******************************************************************

void setup() {
  //pinMode(buttonPin, INPUT); // where will I get the button pin values from? probaly the hardware.h
  delay(2000); // sanity delay
  randomSeed(analogRead(0));

  FastLED.addLeds<CHIPSET, DATA_PIN, CLOCK_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalSMD5050 );
  FastLED.setBrightness( brightness );

  initFireflies(ff); //initialize all firefly objects

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


void loop() {

  updateFireflies();

  // REMOVE THIS LINE and get the decay rate accurate!!!
  leds.fadeToBlackBy(50); // reduce by percent of prior value

  // updatePixels();
  
  FastLED.show();
  delayToSyncFrameRate(FPS);

}

static float eventInterval = poissonInterval(RATE);
//uint8_t values[NUM_LEDS] = {255}; // use this to apply a palette, which we aren't doing yet

void updatePixels() {
  static long int lastt = millis();
  int dt = millis() - lastt;
  if (dt > eventInterval * 1000) {
    lastt = millis();
    // leds[random8(NUM_LEDS)] = CRGB::Yellow;
    leds[random8(NUM_LEDS)].setHSV(random8(10, 80), 255, 255);
    eventInterval = poissonInterval(RATE);
  }
}








/*
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

  void selectFirefly() {
  int i = random8(NUM_LEDS);
  firefly[0].index = i;
  firefly[0].color = CRGB::Yellow;

  }

*/

