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
SoftwareSerial swSer1;
long last_flush;

void setup() {
  // disable WiFi
  WiFi.mode(WIFI_OFF);

  pinMode(1, OUTPUT); // soft serial output
  pinMode(13, INPUT); // soft serial input
  swSer.begin(31250, SWSERIAL_8N1, 13, -1);
  swSer1.begin(115200, SWSERIAL_8N1, -1, 1);

  pinMode(2, OUTPUT); // LRCK
  pinMode(3, OUTPUT); // DIN
  pinMode(15,OUTPUT); // BCK

  last_flush = millis();
}

void loop() {
  while (swSer.available() > 0) {
    if (millis() - last_flush > 100) {
      swSer1.print('\n');
      last_flush = millis();
    }
    char c = swSer.read();
    if (c < 0xf) {
      swSer1.print('0');
    }
    swSer1.print(c, HEX);
    yield();
  }

}
