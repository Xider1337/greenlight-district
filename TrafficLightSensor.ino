#include <SPI.h>
#include <WiFi101.h>
#include <PubSubClient.h>
#include <Adafruit_NeoPixel.h>

#define TOPIC_OUT "gl-sensor-out"
#define TOPIC_IN "gl-sensor-in"
#define MESSAGE_GREEN "Green"
#define MESSAGE_RED "Red"

#define PIN 6
#define NUMPIXELS 24
#define SIGNAL 0,25,128
#define SIGNAL_RED 100,0,0
#define FULL_DURATION 12000

char ssid[] = "IXDS WORKSHOP";   // your network SSID (name)
char pass[] = "ThisIsAPassword"; // your network password (use for WPA, or use as key for WEP)

void mqttCallback(char* topic, byte* payload, unsigned int length);

WiFiClient wificlient;
IPAddress server(192, 168, 179, 89);
PubSubClient mqttclient(server, 1883, mqttCallback, wificlient);
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

int status = WL_IDLE_STATUS;

bool isRed = false;

long lastRed = 0;
long lastGreen = 0;
long durationGreen = 0;
long durationRed = 0;

#define block 4
#define gap 1


char binary_str[33];
int threshold = 70;

#define THRES_LED 10

void setRemainingMillis(long remaining, bool green) {
  int ledsOn = min(NUMPIXELS, NUMPIXELS * ((float)remaining) / FULL_DURATION)+1;

  for(int i=0; i< NUMPIXELS; i++) {
    if(i < ledsOn){
      if(green) {
        pixels.setPixelColor(i, pixels.Color(SIGNAL));
      } else {
        pixels.setPixelColor(i, pixels.Color(SIGNAL_RED));
      }
      
    } else {
      pixels.setPixelColor(i, pixels.Color(0,0,0));
    }
  }
    pixels.show();
}

void setup() {

  pixels.begin();
  WiFi.setPins(8,7,4,2);

  pinMode(THRES_LED, OUTPUT);
  
  // attempt to connect to WiFi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection:
    bool connected = false;
    for(int i=0; i<5000; i+=100) {      
      if(status == WL_CONNECTED) {
        connected = true;
        break;
      }

      if(connected) break;
      delay(100);
    }    
  }

  Serial.println("Connected to wifi");
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  if(strcmp(topic,TOPIC_IN) == 0) int i=0;
}

//      lg      lr             lg  dg  lr
//       _________     dr       ________
// ______|       |______________|      |______
//

void loop() {

  int sensorValue = analogRead(A0);
  delay(1);

  if(sensorValue > threshold && isRed) { // === Switched to GREEN ===     
    isRed = false;

    lastGreen = millis();
    durationRed = millis() - lastRed;
    
    itoa(durationRed, binary_str, 10);
  } 

  if(sensorValue <= threshold && !isRed) { // === Switched to RED ===
    isRed = true;

    lastRed = millis();
    durationGreen =  millis() - lastGreen;

    itoa(durationGreen, binary_str, 10);
  }

  if(!isRed) {
    long timeSinceRed = millis() - lastGreen;
    setRemainingMillis(durationGreen - timeSinceRed, true);
  }

  if(isRed) {
    long timeSinceGreen = millis() - lastRed;
    setRemainingMillis(durationRed - timeSinceGreen, false);
  }

  digitalWrite(THRES_LED, !isRed);

  threshold = analogRead(A1);
}
