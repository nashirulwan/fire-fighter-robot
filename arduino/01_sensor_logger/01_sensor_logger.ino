/*
  Data logger untuk 5 channel flame sensor.
  Upload ke Arduino, buka Serial Monitor 9600 baud.
  Posisikan robot di setiap kondisi api, copy output ke CSV,
  lalu tambahkan kolom label (NO_FIRE / FIRE_LEFT / FIRE_CENTER / FIRE_RIGHT) secara manual.
*/

const int flamePins[5] = { A0, A1, A2, A3, A4 };
const unsigned long sampleDelayMs = 300;

void setup() {
  Serial.begin(9600);
  delay(1000);
  Serial.println("s1,s2,s3,s4,s5");
}

void loop() {
  int s1 = analogRead(flamePins[0]);
  int s2 = analogRead(flamePins[1]);
  int s3 = analogRead(flamePins[2]);
  int s4 = analogRead(flamePins[3]);
  int s5 = analogRead(flamePins[4]);

  Serial.print(s1);   Serial.print(",");
  Serial.print(s2);   Serial.print(",");
  Serial.print(s3);   Serial.print(",");
  Serial.print(s4);   Serial.print(",");
  Serial.println(s5);

  delay(sampleDelayMs);
}
