#include <ESP8266WiFi.h>
#include "SoftwareSerial.h"
#include <i2s.h>
#include <i2s_reg.h>

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

//int16_t sine[256] = {
//0x0000, 0x0324, 0x0647, 0x096a, 0x0c8b, 0x0fab, 0x12c8, 0x15e2,
//0x18f8, 0x1c0b, 0x1f19, 0x2223, 0x2528, 0x2826, 0x2b1f, 0x2e11,
//0x30fb, 0x33de, 0x36ba, 0x398c, 0x3c56, 0x3f17, 0x41ce, 0x447a,
//0x471c, 0x49b4, 0x4c3f, 0x4ebf, 0x5133, 0x539b, 0x55f5, 0x5842,
//0x5a82, 0x5cb4, 0x5ed7, 0x60ec, 0x62f2, 0x64e8, 0x66cf, 0x68a6,
//0x6a6d, 0x6c24, 0x6dca, 0x6f5f, 0x70e2, 0x7255, 0x73b5, 0x7504,
//0x7641, 0x776c, 0x7884, 0x798a, 0x7a7d, 0x7b5d, 0x7c29, 0x7ce3,
//0x7d8a, 0x7e1d, 0x7e9d, 0x7f09, 0x7f62, 0x7fa7, 0x7fd8, 0x7ff6,
//0x7fff, 0x7ff6, 0x7fd8, 0x7fa7, 0x7f62, 0x7f09, 0x7e9d, 0x7e1d,
//0x7d8a, 0x7ce3, 0x7c29, 0x7b5d, 0x7a7d, 0x798a, 0x7884, 0x776c,
//0x7641, 0x7504, 0x73b5, 0x7255, 0x70e2, 0x6f5f, 0x6dca, 0x6c24,
//0x6a6d, 0x68a6, 0x66cf, 0x64e8, 0x62f2, 0x60ec, 0x5ed7, 0x5cb4,
//0x5a82, 0x5842, 0x55f5, 0x539b, 0x5133, 0x4ebf, 0x4c3f, 0x49b4,
//0x471c, 0x447a, 0x41ce, 0x3f17, 0x3c56, 0x398c, 0x36ba, 0x33de,
//0x30fb, 0x2e11, 0x2b1f, 0x2826, 0x2528, 0x2223, 0x1f19, 0x1c0b,
//0x18f8, 0x15e2, 0x12c8, 0x0fab, 0x0c8b, 0x096a, 0x0647, 0x0324,
//0x0000, 0xfcdc, 0xf9b9, 0xf696, 0xf375, 0xf055, 0xed38, 0xea1e,
//0xe708, 0xe3f5, 0xe0e7, 0xdddd, 0xdad8, 0xd7da, 0xd4e1, 0xd1ef,
//0xcf05, 0xcc22, 0xc946, 0xc674, 0xc3aa, 0xc0e9, 0xbe32, 0xbb86,
//0xb8e4, 0xb64c, 0xb3c1, 0xb141, 0xaecd, 0xac65, 0xaa0b, 0xa7be,
//0xa57e, 0xa34c, 0xa129, 0x9f14, 0x9d0e, 0x9b18, 0x9931, 0x975a,
//0x9593, 0x93dc, 0x9236, 0x90a1, 0x8f1e, 0x8dab, 0x8c4b, 0x8afc,
//0x89bf, 0x8894, 0x877c, 0x8676, 0x8583, 0x84a3, 0x83d7, 0x831d,
//0x8276, 0x81e3, 0x8163, 0x80f7, 0x809e, 0x8059, 0x8028, 0x800a,
//0x8000, 0x800a, 0x8028, 0x8059, 0x809e, 0x80f7, 0x8163, 0x81e3,
//0x8276, 0x831d, 0x83d7, 0x84a3, 0x8583, 0x8676, 0x877c, 0x8894,
//0x89bf, 0x8afc, 0x8c4b, 0x8dab, 0x8f1e, 0x90a1, 0x9236, 0x93dc,
//0x9593, 0x975a, 0x9931, 0x9b18, 0x9d0e, 0x9f14, 0xa129, 0xa34c,
//0xa57e, 0xa7be, 0xaa0b, 0xac65, 0xaecd, 0xb141, 0xb3c1, 0xb64c,
//0xb8e4, 0xbb86, 0xbe32, 0xc0e9, 0xc3aa, 0xc674, 0xc946, 0xcc22,
//0xcf05, 0xd1ef, 0xd4e1, 0xd7da, 0xdad8, 0xdddd, 0xe0e7, 0xe3f5,
//0xe708, 0xea1e, 0xed38, 0xf055, 0xf375, 0xf696, 0xf9b9, 0xfcdc
//};
uint8_t phase=0; //Sine phase counter

void ICACHE_RAM_ATTR onTimerISR(){ //Code needs to be in IRAM because its a ISR
  bool flag;
  do {
    uint16_t val; // = sine[phase];
    if (phase%100 < 50) {
      val = 0x3ffc; // about 1/2 -- a lot of ones to see on the scope, <1 when shifted, symmetric (MSB/LSB first doesn't matter)
    } else {
//      val = 0;
      val = 0xc003; // about -1/2
    }
    uint32_t val2 = ((uint32_t)val << 16) | val;
    flag = i2s_write_sample_nb(val2);

//    if (++phase > 255) {
//      phase = 0;
//    }
    ++phase;
  } while (flag);
  timer1_write(2000);//Next in 2mS
}

/*
 * Configurations tried:
 * (
 *  I2SMR  - I2S_MSB_RIGHT
 *  I2STMS - I2S_TRANS_MSB_SHIFT
 * )
 * 48kHz + 4% = 49,92
 * - 80MHz, 44100, I2SMR | I2STMS (no output, measured LCK 46kHz)
 * - 160MHz, 44100, I2SMR | I2STMS (no output, measured LCK 46kHz)
 * - 160MHz, 48000, I2SMR | I2STMS (no output, measured LCK 50,76kHz)
 * - 160MHz, 48000,         I2STMS (no output, measured LCK 50,76kHz)
 */

void setup() {
  // disable WiFi
  WiFi.mode(WIFI_OFF);

//  system_update_cpu_freq(160);

  pinMode(1, OUTPUT); // soft serial output
  pinMode(13, INPUT); // soft serial input
  swSer.begin(31250, SWSERIAL_8N1, 13, -1);
  swSer1.begin(9600, SWSERIAL_8N1, -1, 1);

  pinMode(2, OUTPUT); // LRCK
  pinMode(3, OUTPUT); // DIN
  pinMode(15,OUTPUT); // BCK
//  i2s_begin();
//  i2s_set_rate(48000);
  timer1_attachInterrupt(onTimerISR); //Attach our sampling ISR
  timer1_enable(TIM_DIV16, TIM_EDGE, TIM_SINGLE);
  timer1_write(2000); //Service at 2mS intervall

  last_flush = millis();
}

uint32_t f[] = {
 25000,
 25252,
 25510,
 25641,
 26041,
 26315,
 26455,
 26595,
 26737,
 26881,
 27027,
 27173,
 27322,
 27472,
 27777,
 28248,
 28409,
 28571,
 28735,
 29069,
 29239,
 29411,
 29585,
 29761,
 30303,
 30487,
 30864,
 31055,
 31250,
 31446,
 32051,
 32258,
 32467,
 32679,
 32894,
 33333,
 33783,
 34013,
 34482,
 34722,
 34965,
 35460,
 35714,
 36231,
 36764,
 37037,
 37593,
 37878,
 38461,
 38759,
 39062,
 39682,
 40000,
 40322,
 40650,
 40983,
 41322,
 41666,
 42016,
 42372,
 42735,
 43103,
 43478,
 43859,
 44642,
 45045,
 45454,
 46296,
 47169,
 47619,
 48076,
 49019,
 50000,
 50505,
 51020,
 52083,
 52631,
 53191,
 53763,
 54347,
 54945,
 55555,
 56818,
 57471,
 58139,
 58823,
 59523,
 60975,
 61728,
 62500,
 64102,
 64935,
 65789,
 66666,
 67567,
 69444,
 71428,
 72463,
 73529,
 75757,
 76923,
 78125,
 79365,
 80645,
 81967,
 83333,
 84745,
 86206,
 87719,
 89285,
 90909,
 92592,
 94339,
 96153,
 98039,
 100000,
};
uint8_t div1[] = {
 4,
 9,
 4,
 5,
 4,
 5,
 9,
 4,
 11,
 3,
 5,
 4,
 3,
 7,
 9,
 3,
 4,
 5,
 3,
 4,
 9,
 5,
 13,
 3,
 11,
 4,
 9,
 7,
 4,
 3,
 4,
 5,
 11,
 9,
 4,
 3,
 4,
 3,
 5,
 4,
 11,
 3,
 4,
 3,
 4,
 3,
 7,
 3,
 5,
 3,
 4,
 2,
 5,
 2,
 3,
 2,
 11,
 2,
 7,
 2,
 3,
 2,
 5,
 2,
 2,
 3,
 2,
 9,
 2,
 3,
 2,
 2,
 2,
 9,
 2,
 2,
 5,
 2,
 3,
 2,
 7,
 9,
 2,
 3,
 2,
 5,
 2,
 2,
 9,
 2,
 2,
 7,
 2,
 3,
 2,
 2,
 2,
 3,
 2,
 2,
 5,
 2,
 1,
 1,
 1,
 1,
 1,
 1,
 1,
 1,
 1,
 1,
 1,
 1,
 1,
 1,
};
uint8_t div2[] = {
 50,
 22,
 49,
 39,
 48,
 38,
 21,
 47,
 17,
 62,
 37,
 46,
 61,
 26,
 20,
 59,
 44,
 35,
 58,
 43,
 19,
 34,
 13,
 56,
 15,
 41,
 18,
 23,
 40,
 53,
 39,
 31,
 14,
 17,
 38,
 50,
 37,
 49,
 29,
 36,
 13,
 47,
 35,
 46,
 34,
 45,
 19,
 44,
 26,
 43,
 32,
 63,
 25,
 62,
 41,
 61,
 11,
 60,
 17,
 59,
 39,
 58,
 23,
 57,
 56,
 37,
 55,
 12,
 53,
 35,
 52,
 51,
 50,
 11,
 49,
 48,
 19,
 47,
 31,
 46,
 13,
 10,
 44,
 29,
 43,
 17,
 42,
 41,
 9,
 40,
 39,
 11,
 38,
 25,
 37,
 36,
 35,
 23,
 34,
 33,
 13,
 32,
 63,
 62,
 61,
 60,
 59,
 58,
 57,
 56,
 55,
 54,
 53,
 52,
 51,
 50,
};
const int CNT = 116;

void loop() {
  for (int i = 0; i < CNT; ++i) {
    swSer1.print("\n\nf: ");
    swSer1.print(f[i]);
    swSer1.print(" i: ");
    swSer1.print(div1[i]);
    swSer1.print(" j: ");
    swSer1.print(div2[i]);
    int max = 0;
    i2s_begin();
    i2s_set_dividers(div1[i], div2[i]);
    delay(1000);
    i2s_end();
    delay(2000); // give it time to power down
//    swSer1.print("\nmax = ");
//    swSer1.print(max);
  }
  
//  while (swSer.available() > 0) {
//    if (millis() - last_flush > 100) {
//      swSer1.print('\n');
//      last_flush = millis();
//    }
//    char c = swSer.read();
//    if (c < 0xf) {
//      swSer1.print('0');
//    }
//    swSer1.print(c, HEX);
//    yield();
//  }

}
