#include <FastLED.h>
//#include "firefly.c"

#define DATA_PIN   11 // SPI MOSI pin
#define CLOCK_PIN  13 //13 //SPI  SCK

#define COLOR_ORDER BGR  // most of the 10mm black APA102 / Ray Wu
// #define COLOR_ORDER GBR //mike's short test strip, 19 pixels

#define CHIPSET     APA102
#define FPS 100
#define NUM_LEDS    60

CRGBArray<NUM_LEDS> leds;

CRGBPalette16 currentPalette;

int brightness = 255;

#define NFF 15  // number of fireflies
float POISSON_RATE = 0.3;  // average number events per second
// float POISSON_RATE = 0.5;  // average number events per second
int HUE_START = 10;
int HUE_END = 65;


// ******************************************************************
// create Firefly objects

#define INIT_RISETIME 100  // left as parameters to eventually add noise to them
#define INIT_FALLTIME 1000
#define INIT_HUE 55 // of 256, yellowish
#define INIT_SATURATION 255  // of 256
#define MAX_BRIGHT 255  // of 256, mid level brightness

typedef struct Firefly {
  int index;
  uint8_t state; // 0= off/waiting, 1=rise, 2=fall
  uint8_t hue;
  uint8_t sat;
  uint8_t bright;
  long int starttime; // in ms, what time we start the process of an "event" following an interval
  int interval;  // poisson interval betweeen events, random process
  int elapsedt;  // time since starttime, or the transition from 
  int risetime;
  int falltime; // time to decay to black, in ms
} Firefly;

Firefly ff[NFF];
// Firefly ff = malloc(sizeof(Firefly*) * NFF)

// initialize array of Firefly objects
void initFireflies(Firefly *ff) {
  for(int i=0; i < NFF; i++) {
    ff[i].index = random16(NUM_LEDS); // choose which pixel to start at
    ff[i].state = 0;
    ff[i].hue = INIT_HUE;
    ff[i].sat = INIT_SATURATION;
    ff[i].bright = 0;
    ff[i].starttime = millis();  // record the time that we trigger the start of an interval
    ff[i].interval = poissonInterval(POISSON_RATE) * 1000;  // how long to wait until the next event
    ff[i].elapsedt = 0;
    ff[i].risetime = INIT_RISETIME;
    ff[i].falltime = INIT_FALLTIME;
  }
}

void updateFireflies() {
  unsigned long currentMillis = millis();
  for(int i=0; i<NFF; i++) {
    ff[i].elapsedt = currentMillis - ff[i].starttime;
    printFFState(ff[i]);
    if (ff[i].state == 0) {
      if (ff[i].elapsedt > ff[i].interval) {
        // turn on the pixel
        ff[i].state = 1; // switch state to rise
        ff[i].hue = random8(HUE_START, HUE_END);
        ff[i].sat = 255; // random8(200, 255);
        ff[i].bright = 0;
        ff[i].elapsedt = 0;
        ff[i].starttime = currentMillis;

      }
      else {
        // do nothing, cause already off and waiting for event at interval        
      }
    }
    else if (ff[i].state == 1) {
      if (ff[i].elapsedt > ff[i].risetime) {
        ff[i].state = 2;  // switch state to decay
        ff[i].bright = MAX_BRIGHT;
        // Serial.print("max: ");
        // Serial.println(ff[i].bright);

      }
      else {
        //add slow rise to peak
        ff[i].bright = (uint8_t)( ((long int)MAX_BRIGHT * (long int)ff[i].elapsedt) / (long int)ff[i].risetime);
        Serial.print("u: ");
        Serial.println(ff[i].bright);
      }
    }
    else if (ff[i].state == 2) {
      if (ff[i].elapsedt > ff[i].risetime + ff[i].falltime) {
        //force the pixel off
        leds[ff[i].index].setHSV(ff[i].hue, ff[i].sat, 0);
        ff[i].state = 0; // change state to off
        ff[i].index = random16(NUM_LEDS);
        ff[i].elapsedt = 0;
        ff[i].starttime = currentMillis;
        ff[i].bright = 0;
        //set the new interval here, so we go from peak to start of the next
        ff[i].interval = poissonInterval(POISSON_RATE) * 1000;  // how long to wait until the next event
      }
      else {
        uint8_t m = (uint8_t)( ((long int)MAX_BRIGHT * (long int)ff[i].elapsedt) / ((long int)ff[i].falltime + (long int)ff[i].risetime));
        ff[i].bright = MAX_BRIGHT - m;
        Serial.print("d: ");
        Serial.println(ff[i].bright);

        ff[i].bright = scale8(ff[i].bright,ff[i].bright); // scale by self for more of a gamma curve
      }
    }
    leds[ff[i].index].setHSV(ff[i].hue, ff[i].sat, ff[i].bright);

  } // end for ff
}

void printFFState(Firefly f) {
  Serial.print("i: ");
  Serial.print(f.index);
  Serial.print(" s: ");
  Serial.print(f.state);
  Serial.print(" intvl: ");
  Serial.print(f.interval);
  Serial.print(" dt: ");
  Serial.print(f.elapsedt);
  Serial.print(" v: ");
  Serial.print(f.bright);
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

  // Serial.begin(9600);
  // while (!Serial) {
  //   ; // wait for serial port to connect. Needed for native USB port only
  // }
  // Serial.println("loaded");
  // Serial.println("------");
}

// delayToSyncFrameRate - delay how many milliseconds are needed
//   to maintain a stable frame rate.
static void delayToSyncFrameRate( uint8_t framesPerSecond) {
  static uint32_t msprev = 0;
  uint32_t mscur = millis();
  uint16_t msdelta = mscur - msprev;
  uint16_t mstargetdelta = 1000 / framesPerSecond;
  // Serial.print("frame dt: ");
  // Serial.println(msdelta);
  if ( msdelta < mstargetdelta) {
    FastLED.delay( mstargetdelta - msdelta);
  }
  msprev = mscur;
}


void loop() {

  updateFireflies();
  FastLED.show();
  // delayToSyncFrameRate(FPS);

}
