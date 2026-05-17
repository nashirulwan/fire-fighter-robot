/*
  Data logger flame sensor 5 channel.
  Upload file ini dulu untuk mengambil dataset asli dari robot.
*/

const int flamePins[5] = { A0, A1, A2, A3, A4 };
const unsigned long sampleDelayMs = 300;

void setup() {
  Serial.begin(9600);
  delay(1000);
  Serial.println("s1,s2,s3,s4,s5");
}

void loop() {
  for (int i = 0; i < 5; i++) {
    int value = analogRead(flamePins[i]);
    Serial.print(value);
    if (i < 4) {
      Serial.print(",");
    }
  }

  Serial.println();
  delay(sampleDelayMs);
}
