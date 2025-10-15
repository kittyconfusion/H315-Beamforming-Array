// Currently expects the datasheet of a UM FeatherS2 
// Board Manager -> esp32 by Espressif Systems
// Library Manager -> arduinoFFT by Enrique Condes
// https://github.com/kosme/arduinoFFT/wiki
// current code adapted from built-in examples of arduinoFFT

#include "arduinoFFT.h"
#include "Arduino.h"

#define NumberOfMics 2
#define SpeedOfSound 34300 // cm/s

// Samples before performing FFT, this value MUST ALWAYS be a power of 2
// Do not use 2048 or 4096 specifically due to a bug in ArduinoFFT
const unsigned int samples = 1024; 
const double maxFrequency = 3072; // Maxmimum frequency microphone can pick up 
const double samplingRate = 2*maxFrequency; //limited at 20k samples / # microphones

const double expectedTone = 360; //Hz
const unsigned int expectedToneIndex = round((expectedTone * samples) / samplingRate); // closest index to expected frequency

const unsigned int sampling_period_us = round(1000000*(1.0/samplingRate)); // How long between each sample
const unsigned int sampling_period_per_mic = round(1000000*(1.0/samplingRate) / NumberOfMics);
unsigned long microseconds;

struct Microphone {
  double xPos; // cm
  double expectedDelay; //us, measurement delay
  double expectedPhaseDiff;
  double calculatedPhase;
  double *vReal;
  double *vImag;
  ArduinoFFT<double> FFT;
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
    mics[i].vImag = (double *)ps_malloc(samples * sizeof(double));
    mics[i].FFT = ArduinoFFT<double>(mics[i].vReal, mics[i].vImag, samples, samplingRate);
    mics[i].expectedDelay = i * sampling_period_per_mic;
    mics[i].xPos = 5 * i;
    double phaseSampleDiff = ((2.0*M_PI*expectedTone) / 1000000) * sampling_period_per_mic * i;
    double phaseLocationDiff = (2.0*M_PI*expectedTone) * mics[i].xPos / SpeedOfSound;
    mics[i].expectedPhaseDiff = phaseSampleDiff + phaseLocationDiff;
  }
  
  Serial.print("Free PSRAM after array allocations: ");
  Serial.println(ESP.getFreePsram());

  Serial.println();
  Serial.print("Sampling Rate: ");
  Serial.println(samplingRate);
  Serial.println("5cm separation");
  Serial.println();
  LogFrequencies();
  Serial.println();
}

void loop()
{
  c++;
  microseconds = micros();
  for(int i = 0; i < samples; i++) {
    for(int j = 0; j < NumberOfMics; j++) {
      // pauses execution for ~45-48us for 1-8 microphones respectively storing in psram
      mics[j].vReal[i] = analogRead(micPins[j]);  // 12-bit resolution
      mics[j].vImag[i] = 0; 

    // Evenly space out microphone samples
    while(micros() - microseconds < sampling_period_per_mic) {
      //empty loop
      }
    }

  // at 2kHz sampling rate waits for 500us to have passed
  while(micros() - microseconds < sampling_period_us) {
    //empty loop
    }
  microseconds += sampling_period_us;
  }
  
  for(int k = 0; k < NumberOfMics; k++) {
    //Serial.print("Microphone ");
    //Serial.println(k + 1);

    //LogVoltages(mics[k].vReal);

    ArduinoFFT<double> currentFFT = mics[k].FFT;

    currentFFT.windowing(FFTWindow::Hann, FFTDirection::Forward);	// https://en.wikipedia.org/wiki/Window_function
    currentFFT.compute(FFTDirection::Forward); // compute the fourier transform
    // at this point the frequencies we have complex fourier values for are arranged like:
    // frequency = index*sampling_rate/n_samples
    // index = (frequency*n_samples)/sampling_rate
    // for example, if sr=2000 n=1000, then vReal[100] is real component of 100*2000/1000 = 200Hz

    //currentFFT.complexToMagnitude(); //computes up to the nyquist frequency (first half of vReal becomes magnitudes)

    mics[k].calculatedPhase = atan2(mics[k].vImag[expectedToneIndex], mics[k].vReal[expectedToneIndex]) - mics[k].expectedPhaseDiff;
    //LogComponents(mics[k].vReal, mics[k].vImag);
    //Serial.println();
    //Serial.println(mics[k].vReal[153]);
      
    //LogMagnitudes(mics[k].vReal);

  }
  double phaseDiff = mics[1].calculatedPhase - mics[0].calculatedPhase;

  Serial.println(phaseDiff);
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
void LogComponents(double* vReal, double* vImag) {
  Serial.println("Real:");
  for(int i = 0; i < samples/2 - 1; i++) {
    Serial.print(vReal[i]);
    Serial.print(", ");
  }
  Serial.println(vReal[samples/2]);

  Serial.println("Imag:");
  for(int i = 0; i < samples/2 - 1; i++) {
    Serial.print(vImag[i]);
    Serial.print(", ");
  }
  Serial.println(vImag[samples/2]);
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