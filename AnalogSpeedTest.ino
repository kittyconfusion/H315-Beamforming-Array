// Determines the speed of the analogRead function for a variable number of microphones and the maximum polling rate
#define MaximumNumberOfMics 1

const uint16_t samples = 200000; // Samples taken each time
unsigned long microseconds;

struct Microphone {
  double *vReal;
  double *vImag;
};

Microphone mics[MaximumNumberOfMics];
const unsigned int micPins[MaximumNumberOfMics] = {6}; // first 4 on adc1 second 4 on adc2
//const unsigned int micPins[MaximumNumberOfMics] = {1,12,3,14,5,17,6,18}; // alternating adc1 and adc2


double *vReal;

void setup()
{
  Serial.begin(115200);
  while(!Serial);
  Serial.println("Initialized");

  Serial.print("Initial PSRAM: ");
  Serial.println(ESP.getFreePsram());

  for(int i = 0; i < MaximumNumberOfMics; i++) {
    mics[i].vReal = (double *)ps_malloc(samples * sizeof(double));
    mics[i].vImag = (double *)ps_malloc(samples * sizeof(double));
  }

  Serial.print("Free PSRAM after array allocations: ");
  Serial.println(ESP.getFreePsram());

  microseconds = micros();
  for(int m = 0; m < MaximumNumberOfMics; m++) {
    m++;

    for(int i = 0; i < samples; i++) {
      for(int j = 0; j < (m-1); j++) {
        mics[j].vReal[i] = analogRead(micPins[j]); 
        mics[j].vImag[i] = 0;
      }
    }

    microseconds = micros() - microseconds;
    double usPerSample = (microseconds * 1.0) / (samples * m);
    double maxSampleRate = 1000000 / (usPerSample * m);
    
    Serial.println();
    Serial.print(m);
    Serial.println(" microphone(s)");
    Serial.print("us per sample: ");
    Serial.println(usPerSample);
    Serial.print("max sampling rate: ");
    Serial.println(maxSampleRate);

    m--;
  }
}

void loop()
{
  sleep(1);
}

//       /\_/\
//      ( o.o )
//       > ^ <