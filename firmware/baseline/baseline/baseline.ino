#include <DHT.h>

#define TRIG_PIN 9
#define ECHO_PIN 10
#define DHT_PIN  2
#define DHT_TYPE DHT11

DHT dht(DHT_PIN, DHT_TYPE);

void setup() {
  Serial.begin(9600);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  dht.begin();

  Serial.println("Time_us,Raw_Dist_cm,Temp_C");
}

void loop() {
  // Send a 10 us ultrasonic pulse
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // Measure round-trip echo time (microseconds) with 30 ms timeout
  unsigned long duration = pulseIn(ECHO_PIN, HIGH, 30000);

  // Uncalibrated distance assuming constant speed of sound (0.0343 cm/us)
  float rawDistanceCm = (duration * 0.0343) / 2.0;

  // Read ambient temperature in Celsius
  float tempC = dht.readTemperature();

  // Print CSV format for easy logging later
  Serial.print(duration);
  Serial.print(",");
  Serial.print(rawDistanceCm, 2);
  Serial.print(",");
  if (isnan(tempC)) {
    Serial.println("ERR");
  } else {
    Serial.println(tempC, 1);
  }

  delay(2000); // DHT11 requires ~2 seconds between reads
}