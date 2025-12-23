const int photoPin = A0;
bool streamEnabled = false;
unsigned long lastReadTime = 0;
const int readInterval = 1000;

void setup() {
  Serial.begin(9600);
  pinMode(photoPin, INPUT);
}

void loop() {
  if (Serial.available() > 0) {
    char cmd = Serial.read();

    while(Serial.available() > 0 && (Serial.peek() == '\n' || Serial.peek() == '\r')) {
      Serial.read();
    }

    if (cmd == 'p') {
      int reading = analogRead(photoPin);
      Serial.print("PHOTO_VALUE:");
      Serial.println(reading);
    }
    else if (cmd == 's') {
      streamEnabled = true;
      Serial.println("STREAM_ACTIVE");
    }
    else if (cmd == 'q') {
      streamEnabled = false;
      Serial.println("STREAM_INACTIVE");
    }
  }

  if (streamEnabled) {
    if (millis() - lastReadTime >= readInterval) {
      int reading = analogRead(photoPin);
      Serial.print("PHOTO_VALUE:");
      Serial.println(reading);
      lastReadTime = millis();
    }
  }
}