// Currently expects the datasheet of a UM FeatherS2 
// Board Manager -> esp32 by Espressif Systems
// Library Manager -> arduinoFFT by Enrique Condes
// https://github.com/kosme/arduinoFFT/wiki
// current code adapted from built-in examples of arduinoFFT

#include "arduinoFFT.h"

#define NumberOfMics 2

const uint16_t samples = 128; // Samples before performing FFT, this value MUST ALWAYS be a power of 2
const double maxFrequency = 1000; // Maxmimum frequency microphone will pick up
const double samplingRate = 2*maxFrequency;

unsigned int sampling_period_us; // How long between each sample
unsigned long microseconds;

struct Microphone {
  double xPos; // to account for the physical difference in each microphone location
  double vReal[samples];
  double vImag[samples];
};

Microphone mics[NumberOfMics];
const unsigned int micPins[NumberOfMics] = {A0, A1}; // separate from mics for faster dereferencing 

/*
These are the input and output vectors
Input vectors receive computed results from FFT
*/
double *vReal;
double *vImag;

/* Create FFT object */
ArduinoFFT<double> FFT = ArduinoFFT<double>(vReal, vImag, samples, samplingRate);

void setup()
{
  sampling_period_us = round(1000000*(1.0/samplingRate));
  Serial.begin(115200);
  while(!Serial);
  Serial.println("Initialized");

  // each microphone in mics should be initialized here
}

void loop()
{
  microseconds = micros();
  for(int i=0; i<samples; i++) {
    for(int j=0; j<NumberOfMics; j++) {
      // pauses execution for ~100us (10kHz sampling rate of a single microphone)
      // this may need to be accounted for because it adds up very fast
      mics[j].vReal[i] = analogRead(micPins[j]); 
      mics[j].vImag[i] = 0; 
    }

    // at 2kHz sampling rate waits for 500us to have passed
    while(micros() - microseconds < sampling_period_us) {
      //empty loop
    }
    microseconds += sampling_period_us;
  }

    for(int j=0; j<NumberOfMics; j++) {
      vReal = mics[j].vReal;
      vImag = mics[j].vImag;
    
      FFT.windowing(FFTWindow::Hamming, FFTDirection::Forward);	// a bunch of different modes, this seems to be standard
      FFT.compute(FFTDirection::Forward); // compute the fourier transform

      // at this point the frequencies we have complex fourier values for are arranged like:
      // frequency = index*sampling_frequency/n_samples
      // for example, if sf=2000 n=1000, then vReal[100] is real component of 100*2000/1000 = 200Hz

      FFT.complexToMagnitude(); //computes up to the nyquist frequency (first half of vReal becomes magnitudes)


  }
}

//       /\_/\
//      ( o.o )
//       > ^ <