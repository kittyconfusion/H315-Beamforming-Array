// Currently expects the datasheet of a UM FeatherS2 
// Board Manager -> esp32 by Espressif Systems
// Library Manager -> arduinoFFT by Enrique Condes
// https://github.com/kosme/arduinoFFT/wiki
// current code adapted from built-in examples of arduinoFFT

#include "arduinoFFT.h"

#define NumberOfMics 1

const unsigned int samples = 1024; // Samples before performing FFT, this value MUST ALWAYS be a power of 2
const double maxFrequency = 2000; // Maxmimum frequency microphone can pick up 
const double samplingRate = 2*maxFrequency; //limited at 20k samples / # microphones

const unsigned int sampling_period_us = round(1000000*(1.0/samplingRate)); // How long between each sample
unsigned long microseconds;

struct Microphone {
  double xPos; // to account for the physical difference in each microphone location
  // These are the input and output for the FFT object
  double *vReal;
  double *vImag;
  ArduinoFFT<double> FFT;
};

Microphone mics[NumberOfMics];
const unsigned int micPins[NumberOfMics] = {6}; // separate from mics for faster dereferencing 

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
    mics[i].vImag = (double *)ps_malloc(samples * sizeof(double));
    mics[i].FFT = ArduinoFFT<double>(mics[i].vReal, mics[i].vImag, samples, samplingRate);
  }
  Serial.print("Free PSRAM after array allocations: ");
  Serial.println(ESP.getFreePsram());
}

void loop()
{
  microseconds = micros();
  for(int i = 0; i < samples; i++) {
    for(int j = 0; j < NumberOfMics; j++) {
      // pauses execution for ~45-48us for 1-8 microphones respectively storing in psram
      mics[j].vReal[i] = analogRead(micPins[j]);  // 12-bit resolution
      mics[j].vImag[i] = 0; 
    }

  // at 2kHz sampling rate waits for 500us to have passed
  while(micros() - microseconds < sampling_period_us) {
    //empty loop
    }
  microseconds += sampling_period_us;
  }
  usleep(100000);
  //Serial.println(++c);
  for(int k = 0; k < NumberOfMics; k++) {
    ArduinoFFT<double> currentFFT = mics[k].FFT;

    currentFFT.windowing(FFTWindow::Hamming, FFTDirection::Forward);	// a bunch of different modes, this seems to be standard
    currentFFT.compute(FFTDirection::Forward); // compute the fourier transform
    // at this point the frequencies we have complex fourier values for are arranged like:
    // frequency = index*sampling_rate/n_samples
    // index = (frequency*n_samples)/sampling_rate
    // for example, if sr=2000 n=1000, then vReal[100] is real component of 100*2000/1000 = 200Hz

    currentFFT.complexToMagnitude(); //computes up to the nyquist frequency (first half of vReal becomes magnitudes)

    Serial.println(mics[k].vReal[153]);
      
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