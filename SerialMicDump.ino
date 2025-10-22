// Currently expects the datasheet of a UM FeatherS2 
// Board Manager -> esp32 by Espressif Systems
// Library Manager -> arduinoFFT by Enrique Condes
// https://github.com/kosme/arduinoFFT/wiki
// current code adapted from built-in examples of arduinoFFT

#include "arduinoFFT.h"
#include "Arduino.h"

#define NumberOfMics 2

const unsigned int samples = 1; 
const double maxFrequency = 3072; // Maxmimum frequency microphone can pick up 
const double samplingRate = 2*maxFrequency; //limited at 20k samples / # microphones

const unsigned int sampling_period_us = round(1000000*(1.0/samplingRate)); // How long between each sample
const unsigned int sampling_period_per_mic = round(1000000*(1.0/samplingRate) / NumberOfMics);
unsigned long microseconds;

struct Microphone {
  uint16_t *vReal;
};

Microphone mics[NumberOfMics];
const unsigned int micPins[NumberOfMics] = {6, 5}; // separate from mics for faster dereferencing 

void setup()
{
  Serial.begin(115200);
  while(!Serial);
  Serial.println("Initialized!!!!");

  for(int i = 0; i < NumberOfMics; i++) {
    mics[i].vReal = (uint16_t *)ps_malloc(samples * sizeof(uint16_t));
 }
}

void loop()
{
  microseconds = micros();
  for(int i = 0; i < samples; i++) {
    for(int j = 0; j < NumberOfMics; j++) {
      // pauses execution for ~45-48us for 1-8 microphones respectively storing in psram
      mics[j].vReal[i] = analogRead(micPins[j]);  // 12-bit resolution
      Serial.write((byte*)&(mics[j].vReal[i]), 2);

    // Evenly space out microphone samples
    if(j < NumberOfMics - 1) {
      while(micros() - microseconds < sampling_period_per_mic) {
        //empty loop
        }
      }
    }
  Serial.write('\n');
  // at 2kHz sampling rate waits for 500us to have passed
  while(micros() - microseconds < sampling_period_us) {
    //empty loop
    }
  }
}








void LogVoltages(uint16_t* vReal) {
  for(int i = 0; i < samples; i++) {
    Serial.write((byte*)&vReal[i], 2);
  }
}
//       /\_/\
//      ( o.o )
//       > ^ <

//      (
//     `-`-.
//     '( @ >
//      _) (
//     /    )
//    /_,'  / 
//      \  / 
//   ===m""m===