
const int ledOutput = 13;
bool blinkMode = false;
unsigned long previousBlink = 0;
bool currentLedState = LOW;

void setup() {
  Serial.begin(9600);
  pinMode(ledOutput, OUTPUT);
}

void loop() {
  if (Serial.available() > 0) {
    char command = Serial.read();

    while(Serial.available() > 0 && (Serial.peek() == '\n' || Serial.peek() == '\r')) {
      Serial.read();
    }

    if (command == 'u') {
      blinkMode = false;
      digitalWrite(ledOutput, HIGH);
      Serial.println("LED_ON");
    }
    else if (command == 'd') {
      blinkMode = false;
      digitalWrite(ledOutput, LOW);
      Serial.println("LED_OFF");
    }
    else if (command == 'b') {
      blinkMode = true;
      Serial.println("LED_BLINK_MODE");
    }
  }

  if (blinkMode) {
    if (millis() - previousBlink >= 500) {
      currentLedState = !currentLedState;
      digitalWrite(ledOutput, currentLedState);
      previousBlink = millis();
    }
  }
}