#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>

/*
 * Pins used:
 *  D3  - output - ADC DIN
 *  D2  - output - ADC LRCK
 *  D15 - output - ADC BCK
 *  D13 - input  - software serial: MIDI
 *  (so we can't use hardware Serial - both normal and alternative ports are taken)
 *  D1  - ouput  - software serial: console
 */

String inputString = "";
bool stringComplete = false;

SoftwareSerial swSer;

void setup() {
  // disable WiFi
  WiFi.mode(WIFI_OFF);

  // INIT serial communication
  Serial.begin(115200);
  Serial.println('\n');
  inputString.reserve(100);

  pinMode(13, INPUT); // soft serial
  swSer.begin(31250, SWSERIAL_8N1, 13);
}

void loop() {
  // check serial port events
  if (Serial.available() > 0) {
    serialEvent();
  }

  // serial port commands handling
  if (stringComplete) {
    Serial.println(inputString);
    inputString = "";
    stringComplete = false;
  }

  while (swSer.available() > 0) {
    char c = swSer.read();
    if (c < 0xf) {
      Serial.print('0');
    }
    Serial.print(swSer.read(), HEX);
    yield();
  }
}

void serialEvent() {
  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read();
    // if the incoming character is a newline, set a flag so the main loop can
    // do something about it:
    if (inChar == '\n') {
      stringComplete = true;
    } else {
      // add it to the inputString:
      inputString += inChar;
    }
  }
}
