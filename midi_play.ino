#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>

/*
 * Pins used:
 *  D3  - output - ADC DIN
 *  D2  - output - ADC LRCK
 *  D15 - output - ADC BCK
 *  D13 - input  - software serial: MIDI
 *  (so we can't use hardware Serial - both normal and alternative ports are taken)
 *  (Serial1 output is taken too)
 *  D1  - ouput  - software serial: console
 */

SoftwareSerial swSer;

void setup() {
  // disable WiFi
  WiFi.mode(WIFI_OFF);

  // INIT serial communication
  Serial.begin(115200);
  Serial.println('\n');

  pinMode(13, INPUT); // soft serial
  swSer.begin(31250, SWSERIAL_8N1, 13);
}

void loop() {
  while (swSer.available() > 0) {
    char c = swSer.read();
    if (c < 0xf) {
      Serial.print('0');
    }
    Serial.print(swSer.read(), HEX);
    yield();
  }
}
