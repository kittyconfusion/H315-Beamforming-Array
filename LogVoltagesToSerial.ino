// Currently expects the datasheet of a UM FeatherS2 
// Board Manager -> esp32 by Espressif Systems
// Library Manager -> arduinoFFT by Enrique Condes
// https://github.com/kosme/arduinoFFT/wiki
// current code adapted from built-in examples of arduinoFFT

#include "Arduino.h"

#define NumberOfMics 2

const unsigned int samples = 1024; 
const double maxFrequency = 3072; // Maxmimum frequency microphone can pick up 
const double samplingRate = 2*maxFrequency; //limited at 20k samples / # microphones

const unsigned int sampling_period_us = round(1000000*(1.0/samplingRate)); // How long between each sample
const unsigned int sampling_period_per_mic = round(1000000*(1.0/samplingRate) / NumberOfMics);
unsigned long microseconds;

struct Microphone {
  double *vReal;
};

Microphone mics[NumberOfMics];
const unsigned int micPins[NumberOfMics] = {6, 5}; // separate from mics for faster dereferencing 

unsigned int c = 0;

void setup()
{
  Serial.begin(115200);
  while(!Serial);
  Serial.println("Initialized!!!!");

  Serial.print("Initial PSRAM: ");
  Serial.println(ESP.getFreePsram());

  for(int i = 0; i < NumberOfMics; i++) {
    mics[i].vReal = (double *)ps_malloc(samples * sizeof(double));
  }
  
  Serial.print("Free PSRAM after array allocations: ");
  Serial.println(ESP.getFreePsram());

  Serial.println();
  Serial.print("Sampling Rate: ");
  Serial.println(samplingRate);
  //Serial.println("5cm separation");
  Serial.println();
  //LogFrequencies();
  Serial.println();
}

void loop()
{
  microseconds = micros();
  for(int i = 0; i < samples; i++) {
    for(int j = 0; j < NumberOfMics; j++) {
      // pauses execution for ~45-48us for 1-8 microphones respectively storing in psram
      mics[j].vReal[i] = analogRead(micPins[j]);  // 12-bit resolution

    // Evenly space out microphone samples
    if(j < NumberOfMics - 1) {
      while(micros() - microseconds < sampling_period_per_mic) {
        //empty loop
        }
      }
      microseconds += sampling_period_per_mic;
    }

  // at 2kHz sampling rate waits for 500us to have passed
  while(micros() - microseconds < sampling_period_us) {
    //empty loop
    }
  }
  
  for(int k = 0; k < NumberOfMics; k++) {
    Serial.print("Microphone ");
    Serial.println(k + 1);

    LogVoltages(mics[k].vReal);

    Serial.println();
  }
  c++;
  if (c == 5) {
    sleep(100000);
  }
  sleep(2);
}

void LogMagnitudes (double* mag) {
  Serial.println("Magnitudes:");
  for(int i = 0; i < samples/2 - 1; i++) {
    Serial.print(mag[i]);
    Serial.print(", ");
  }
  Serial.println(mag[samples/2]);
}

void LogFrequencies() {
  Serial.println("Frequencies:");
  for(int i = 0; i < samples/2 - 1; i++) {
    Serial.print(1.0 * i * samplingRate / samples);
    Serial.print(", ");
  }
  Serial.println(samplingRate / 2);
}

void LogVoltages(double* vReal) {
  Serial.println("Voltages:");
  for(int i = 0; i < samples/2 - 1; i++) {
    Serial.print(vReal[i]);
    Serial.print(", ");
  }
  Serial.println(vReal[samples/2]);
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