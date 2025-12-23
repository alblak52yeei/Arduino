#define ROW_COUNT 3
#define COL_COUNT 3
#define TOTAL_BUTTONS (ROW_COUNT * COL_COUNT)

volatile bool button_state[TOTAL_BUTTONS] = {false};
volatile bool state_changed = false;
volatile uint8_t active_row = 0;
unsigned long press_time[TOTAL_BUTTONS] = {0};
bool last_state[TOTAL_BUTTONS] = {0};

void setup() {
  Serial.begin(9600);

  // Настройка портов: строки как выходы, столбцы как входы с подтяжкой
  DDRD |= (1 << DDD2) | (1 << DDD3) | (1 << DDD4);
  DDRD &= ~((1 << DDD5) | (1 << DDD6) | (1 << DDD7));
  PORTD |= (1 << PORTD2) | (1 << PORTD3) | (1 << PORTD4);
  PORTD |= (1 << PORTD5) | (1 << PORTD6) | (1 << PORTD7);

  // Конфигурация таймера Timer1
  cli();
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1  = 0;

  OCR1A = 999;

  TCCR1B |= (1 << WGM12);
  TCCR1B |= (1 << CS11) | (1 << CS10);
  TIMSK1 |= (1 << OCIE1A);

  sei();

  Serial.println("Матрица кнопок инициализирована. Начато сканирование.");
}

void loop() {
  bool current_state[TOTAL_BUTTONS];
  bool has_changes = false;
  
  for(int i = 0; i < TOTAL_BUTTONS; i++) {
    current_state[i] = button_state[i];
    if (current_state[i] != last_state[i]) {
      has_changes = true;
    }
  }

  if (has_changes) {
    unsigned long current_millis = millis();

    for(int i = 0; i < TOTAL_BUTTONS; i++) {
      if (!last_state[i] && current_state[i]) {
        press_time[i] = current_millis;
        Serial.print("Кнопка ");
        Serial.print(i + 1);
        Serial.print(" нажата в: ");
        Serial.print(current_millis);
        Serial.println(" мс");
      }
      if (last_state[i] && !current_state[i]) {
        unsigned long hold_time = current_millis - press_time[i];
        Serial.print("Кнопка ");
        Serial.print(i + 1);
        Serial.print(" отпущена. Удержание: ");
        Serial.print(hold_time);
        Serial.print(" мс. Начало: ");
        Serial.print(press_time[i]);
        Serial.println(" мс");
      }
    }

    Serial.print("Активные кнопки: ");
    bool found_any = false;
    for(int i = 0; i < TOTAL_BUTTONS; i++) {
      if (current_state[i]) {
        if (found_any) Serial.print(", ");
        Serial.print(i + 1);
        found_any = true;
      }
    }
    if (!found_any) Serial.print("Нет");
    Serial.println();

    for(int i = 0; i < TOTAL_BUTTONS; i++) {
      last_state[i] = current_state[i];
    }
  }
  delay(10);
}

ISR(TIMER1_COMPA_vect) {
  // Установка всех строк в HIGH
  PORTD |= (1 << PORTD2) | (1 << PORTD3) | (1 << PORTD4);

  // Активация текущей строки (установка в LOW)
  int active_pin = active_row + 2;
  PORTD &= ~(1 << active_pin);

  // Чтение состояния столбцов
  uint8_t port_state = PIND;

  bool col0_pressed = !((port_state >> 5) & 1);
  button_state[active_row * 3 + 0] = col0_pressed;

  bool col1_pressed = !((port_state >> 6) & 1);
  button_state[active_row * 3 + 1] = col1_pressed;

  bool col2_pressed = !((port_state >> 7) & 1);
  button_state[active_row * 3 + 2] = col2_pressed;

  active_row++;
  if (active_row >= ROW_COUNT) {
    active_row = 0;
  }
}