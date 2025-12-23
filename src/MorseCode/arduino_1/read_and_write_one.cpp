#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define BOARD_ID 1

LiquidCrystal_I2C display(0x27, 16, 2);

const int INPUT_PIN = 7;
const int OUTPUT_PIN = 8;
const int STATUS_LED = 9;
const int BUTTON_PIN = 4;

// Временные параметры протокола
const int baseUnit = 300;
const int dotMaxTime = baseUnit * 1.5;
const int symbolPause = baseUnit * 3;

// Состояние приемника
String incomingCode = "";
unsigned long pulseBeginTime = 0;
unsigned long pulseEndTime = 0;
bool signalActive = false;
String displayText = "";

// Состояние кнопки для ручной передачи
String manualCode = "";
unsigned long buttonDownTime = 0;
bool buttonActive = false;
unsigned long buttonUpTime = 0;

// Преобразование кода Морзе в символ
char morseToChar(String code) {
  // Буквы
  if (code == ".-") return 'A';
  if (code == "-...") return 'B';
  if (code == "-.-.") return 'C';
  if (code == "-..") return 'D';
  if (code == ".") return 'E';
  if (code == "..-.") return 'F';
  if (code == "--.") return 'G';
  if (code == "....") return 'H';
  if (code == "..") return 'I';
  if (code == ".---") return 'J';
  if (code == "-.-") return 'K';
  if (code == ".-..") return 'L';
  if (code == "--") return 'M';
  if (code == "-.") return 'N';
  if (code == "---") return 'O';
  if (code == ".--.") return 'P';
  if (code == "--.-") return 'Q';
  if (code == ".-.") return 'R';
  if (code == "...") return 'S';
  if (code == "-") return 'T';
  if (code == "..-") return 'U';
  if (code == "...-") return 'V';
  if (code == ".--") return 'W';
  if (code == "-..-") return 'X';
  if (code == "-.--") return 'Y';
  if (code == "--..") return 'Z';
  // Цифры
  if (code == ".----") return '1';
  if (code == "..---") return '2';
  if (code == "...--") return '3';
  if (code == "....-") return '4';
  if (code == ".....") return '5';
  if (code == "-....") return '6';
  if (code == "--...") return '7';
  if (code == "---..") return '8';
  if (code == "----.") return '9';
  if (code == "-----") return '0';
  return '?';
}

const char* morseLetters[] = {
  ".-", "-...", "-.-.", "-..", ".", "..-.", "--.", "....", "..", ".---",
  "-.-", ".-..", "--", "-.", "---", ".--.", "--.-", ".-.", "...", "-",
  "..-", "...-", ".--", "-..-", "-.--", "--.."
};

const char* morseDigits[] = {
  "-----", ".----", "..---", "...--", "....-", ".....", "-....", "--...", "---..", "----."
};

void initializeDisplay() {
  if (BOARD_ID == 1) {
    display.init();
    display.backlight();
    display.setCursor(0, 0);
    display.print("Node 1: Active");
    display.setCursor(0, 1);
    display.print("Node 2: Standby");
    delay(1000);
    display.clear();
  } else {
    Wire.begin();
    display.setBacklight(HIGH);
    delay(1500);
  }
}

void updateDisplay(char symbol) {
  int row = (BOARD_ID == 1) ? 1 : 0;
  
  displayText += symbol;
  
  // Защита от конфликтов на I2C шине
  delay(random(5, 15));
  
  display.setCursor(0, row);
  if (displayText.length() > 16) {
    display.print(displayText.substring(displayText.length() - 16));
  } else {
    display.print(displayText);
  }
}

// Обработка входящих сигналов
void processIncomingSignal() {
  int pinState = digitalRead(INPUT_PIN);
  
  if (pinState == HIGH && !signalActive) {
    pulseBeginTime = millis();
    signalActive = true;
    digitalWrite(STATUS_LED, HIGH);
  }

  if (pinState == LOW && signalActive) {
    signalActive = false;
    digitalWrite(STATUS_LED, LOW);
    unsigned long pulseDuration = millis() - pulseBeginTime;
    pulseEndTime = millis();

    if (pulseDuration > 30) {
      if (pulseDuration < dotMaxTime) {
        incomingCode += ".";
      } else {
        incomingCode += "-";
      }
    }
  }

  if (!signalActive && incomingCode.length() > 0) {
    if (millis() - pulseEndTime > symbolPause) {
      char decodedChar = morseToChar(incomingCode);
      
      Serial.print("-> Принят символ: ");
      Serial.print(decodedChar);
      Serial.print(" (Морзе: ");
      Serial.print(incomingCode);
      Serial.println(")");
      
      updateDisplay(decodedChar);
      incomingCode = "";
    }
  }
}

// Неблокирующая пауза с мониторингом входа
void nonBlockingDelay(unsigned long waitMs) {
  unsigned long startTime = millis();
  while (millis() - startTime < waitMs) {
    processIncomingSignal();
  }
}

// Передача символа через последовательный порт
void transmitMorseCode(const char* morsePattern) {
  int idx = 0;
  while (morsePattern[idx] != '\0') {
    digitalWrite(OUTPUT_PIN, HIGH);
    if (morsePattern[idx] == '.') {
      nonBlockingDelay(baseUnit);
    } else {
      nonBlockingDelay(baseUnit * 3);
    }
    digitalWrite(OUTPUT_PIN, LOW);
    nonBlockingDelay(baseUnit);
    idx++;
  }
  nonBlockingDelay(baseUnit * 3);
}

// Обработка ручного ввода через кнопку
void processManualInput() {
  int buttonState = digitalRead(BUTTON_PIN);

  // Обработка нажатия
  if (buttonState == LOW && !buttonActive) {
    buttonActive = true;
    buttonDownTime = millis();
    digitalWrite(OUTPUT_PIN, HIGH);
  }

  // Обработка отпускания
  if (buttonState == HIGH && buttonActive) {
    buttonActive = false;
    unsigned long holdDuration = millis() - buttonDownTime;
    digitalWrite(OUTPUT_PIN, LOW);
    buttonUpTime = millis();

    if (holdDuration > 30) {
      if (holdDuration < dotMaxTime) {
        manualCode += ".";
      } else {
        manualCode += "-";
      }
    }
  }

  // Определение переданного символа для отладки
  if (!buttonActive && manualCode.length() > 0) {
    if (millis() - buttonUpTime > symbolPause) {
      char sentChar = morseToChar(manualCode);
      
      Serial.print("<- Передан символ: ");
      Serial.println(sentChar);
      
      manualCode = "";
    }
  }
}

void setup() {
  pinMode(INPUT_PIN, INPUT);
  pinMode(OUTPUT_PIN, OUTPUT);
  pinMode(STATUS_LED, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  Serial.begin(9600);

  Serial.print("Инициализация системы | Устройство #");
  Serial.println(BOARD_ID);

  initializeDisplay();
}

void loop() {
  // Мониторинг входящих данных
  processIncomingSignal();

  // Обработка кнопки
  processManualInput();

  // Обработка данных из Serial Monitor
  if (Serial.available()) {
    char inputChar = Serial.read();
    inputChar = toupper(inputChar);
    
    if (inputChar >= 'A' && inputChar <= 'Z') {
      Serial.print("<- Передача: ");
      Serial.println(inputChar);
      transmitMorseCode(morseLetters[inputChar - 'A']);
    } 
    else if (inputChar >= '0' && inputChar <= '9') {
      Serial.print("<- Передача: ");
      Serial.println(inputChar);
      transmitMorseCode(morseDigits[inputChar - '0']);
    } 
    else if (inputChar == ' ') {
      Serial.println("<- Передача: ПРОБЕЛ");
      nonBlockingDelay(baseUnit * 7);
    }
  }
}