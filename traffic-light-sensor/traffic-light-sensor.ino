#include <Adafruit_NeoPixel.h>

#define PIN_DATA_RGB_PIXELS 6     // GPIO for data connection to LED Strip
#define PIN_GREEN_SENSOR A0       // GPIO for reading the brightness of the photoresistor
#define PIN_GREEN_THRESHOLD A1    // GPIO to read the green brightness threshold value from
#define PIN_THRES_LED 10          // GPIO for LED, displays when green is detected

#define NUM_RGB_PIXELS 24         // Amount of Pixels of the Neopixel strip to control
#define SIGNAL_GREEN 0  , 25, 128 // RGB Color when approaching a green light 
#define SIGNAL_RED   100,  0,   0 // RGB Color when approaching a red light
#define SIGNAL_OFF   0  ,  0,   0 // RGB Color for the LEDs that are off

#define MILLIS_FULL_STRIP 12000 // duration of the full countdown

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUM_RGB_PIXELS, PIN_DATA_RGB_PIXELS, NEO_GRB + NEO_KHZ800);

bool isGreen = false; // represents the currently sensed state of the traffic light

long millisLastRed    = 0; // point in time when switched red 
long millisLastGreen  = 0; // point in time when switched green
long durationGreen    = 0; // currently measured duration of the green phase
long durationRed      = 0; // currently measured duration of the green phase

int brightnessThreshold = 1024; // brightness value to exceed to recognise green 

//sets a pixel of the LEDs strip
inline void setColor(int pixelId, int r, int g, int b) {
  pixels.setPixelColor(pixelId, pixels.Color(r, g, b));
}

//sets the led strip to show the remaining time (red & green)
void displayRemainingTime(long remainingMillis, bool green) {
  int numberActiveLeds = min(NUM_RGB_PIXELS, NUM_RGB_PIXELS * ((float)remainingMillis) / MILLIS_FULL_STRIP)+1;

  for(int i=0; i< NUM_RGB_PIXELS; i++) {
    if(i < numberActiveLeds) {
      if(green) { setColor(i, SIGNAL_GREEN);  } 
      else      { setColor(i, SIGNAL_RED);    }
    } else {      setColor(i, SIGNAL_OFF);    }
  }
    pixels.show();
}

void setup() {
  pixels.begin();
  pinMode(PIN_THRES_LED, OUTPUT);
}

// Phases & Moments: lg=millisLastGreen, lr=millisLastRed, dr=duration red, ...
//
//      lg      lr             lg  dg  lr
//       _________     dr       ________
// ______|       |______________|      |______

void loop() {
  delay(10); // Delay between loops to keep calculations at a reasonable Level

  brightnessThreshold = analogRead(PIN_GREEN_THRESHOLD); //sets brightness threshold for the green light
  int brightnessGreen = analogRead(PIN_GREEN_SENSOR);    //check photo-resistor brightness
  digitalWrite(PIN_THRES_LED, isGreen);                  //Threshold feedback (green when brightness bigger than the threshold) 

  if(brightnessGreen > brightnessThreshold && !isGreen) { // === Switched to GREEN ===     
    isGreen = true;

    millisLastGreen = millis();
    durationRed = millisLastGreen - millisLastRed;
  } 

  if(brightnessGreen <= brightnessThreshold && isGreen) { // === Switched to RED ===
    isGreen = false;

    millisLastRed = millis();
    durationGreen =  millisLastRed - millisLastGreen;
  }

  if(isGreen) {
    long timeSinceRed = millis() - millisLastGreen;
    displayRemainingTime(durationGreen - timeSinceRed, true);

  } else {
    long timeSinceGreen = millis() - millisLastRed;
    displayRemainingTime(durationRed - timeSinceGreen, false);
  }  
}