#include <DHT.h>

#define TRIG_PIN 9
#define ECHO_PIN 10
#define DHT_PIN  2
#define DHT_TYPE DHT11

// Unit 4 Least Squares calibration constants (from baseline data)
const float CAL_SLOPE  = 1.0070;
const float CAL_OFFSET = 0.4421;

DHT dht(DHT_PIN, DHT_TYPE);
float lastValidTemp = 21.4; // Fallback if a DHT11 read drops

// Take 5 ultrasonic pings and return the median to reduce timing jitter (Delta t)
unsigned long readMedianEchoUs() {
  unsigned long samples[5];
  int validCount = 0;

  for (int i = 0; i < 5; i++) {
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);

    unsigned long d = pulseIn(ECHO_PIN, HIGH, 30000);
    if (d > 0) {
      samples[validCount++] = d;
    }
    delay(30); // Short pause so echoes don't overlap
  }

  if (validCount == 0) return 0;

  // Insertion sort valid readings to grab the median
  for (int i = 1; i < validCount; i++) {
    unsigned long key = samples[i];
    int j = i - 1;
    while (j >= 0 && samples[j] > key) {
      samples[j + 1] = samples[j];
      j--;
    }
    samples[j + 1] = key;
  }

  return samples[validCount / 2];
}

void setup() {
  Serial.begin(9600);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  dht.begin();

  Serial.println("Time_us,Raw_Dist_cm,Cal_Dist_cm,Temp_C");
}

void loop() {
  // Read live temperature from DHT11
  float tempC = dht.readTemperature();
  if (!isnan(tempC)) {
    lastValidTemp = tempC;
  }

  // Median echo duration (microseconds)
  unsigned long duration = readMedianEchoUs();
  if (duration == 0) {
    delay(500);
    return; // Skip missed echoes
  }

  // 1. Old uncalibrated formula (fixed 343 m/s) for side-by-side comparison
  float rawDistCm = (duration * 0.0343) / 2.0;

  // 2. Multivariable temp-compensated distance D(t, T)
  float tempCompCm = (duration * (331.3 + 0.606 * lastValidTemp)) / 20000.0;

  // 3. Least-squares calibrated distance
  float calDistCm = (CAL_SLOPE * tempCompCm) + CAL_OFFSET;

  // CSV output: Time_us, Raw_Dist_cm, Cal_Dist_cm, Temp_C
  Serial.print(duration);
  Serial.print(",");
  Serial.print(rawDistCm, 2);
  Serial.print(",");
  Serial.print(calDistCm, 2);
  Serial.print(",");
  Serial.println(lastValidTemp, 1);

  delay(1850); // ~2 seconds total per cycle for DHT11
}