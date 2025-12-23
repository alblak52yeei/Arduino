#include <avr/io.h>
#include <avr/interrupt.h>

int latchPin = 5;
int clockPin = 3;
int dataPin = 7;

bool segment_patterns[10][8] = {
  {1,1,0,1,1,1,0,1},
  {0,1,0,1,0,0,0,0},
  {1,1,0,0,1,1,1,0},
  {1,1,0,1,1,0,1,0},
  {0,1,0,1,0,0,1,1},
  {1,0,0,1,1,0,1,1},
  {1,0,1,1,1,1,1,1},
  {1,1,0,1,0,0,0,0},
  {1,1,0,1,1,1,1,1},
  {1,1,1,1,1,0,1,1}
};

volatile int timer_counter = 0;
volatile int next_value = -1;
volatile bool is_active = false;

volatile uint16_t data_reg = 0;
volatile int current_bit = 15;
volatile int tick_counter = 0;

enum FsmState { IDLE, SENDING, UPDATING };
volatile FsmState machine_state = IDLE;

void setup() {
  pinMode(latchPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
  pinMode(clockPin, OUTPUT);

  digitalWrite(clockPin, LOW);
  digitalWrite(latchPin, HIGH);

  Serial.begin(9600);
  Serial.println(F("Введите начальное значение"));

  cli();
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1  = 0;

  OCR1A = 249;
  TCCR1B |= (1 << WGM12);
  TCCR1B |= (1 << CS11) | (1 << CS10);
  TIMSK1 |= (1 << OCIE1A);
  sei();
}

void loop() {
  static char input_line[3];
  static byte input_pos = 0;

  if (Serial.available()) {
    char ch = Serial.read();

    if (ch >= '0' && ch <= '9') {
      if (input_pos < 2) input_line[input_pos++] = ch;
    }

    if (input_pos == 2 || ch == '\n') {
      input_line[input_pos] = 0;
      int num = atoi(input_line);

      if (num >= 0 && num <= 99) {
        cli();

        if (!is_active) {
          timer_counter = num;
          is_active = true;
          tick_counter = 999;
          Serial.print(F("ЗАПУСК со значением: ")); Serial.println(num);
        } else {
          next_value = num;
          Serial.print(F("ПЕРЕОПРЕДЕЛЕНИЕ на следующем тике: ")); Serial.println(num);
        }

        sei();
      }
      input_pos = 0;
    }
  }
}

uint16_t build_segments(int number) {
  int tens_digit = number / 10;
  int ones_digit = number % 10;

  uint16_t result = 0;

  for(int i = 7; i >= 0; i--) {
     if(segment_patterns[tens_digit][i]) result |= (1 << (8 + i));
  }

  for(int i = 7; i >= 0; i--) {
     if(segment_patterns[ones_digit][i]) result |= (1 << i);
  }

  return result;
}

ISR(TIMER1_COMPA_vect) {
  tick_counter++;

  if (tick_counter >= 1000) {
    tick_counter = 0;

    if (is_active) {
      int display_num;

      if (next_value != -1) {
        display_num = next_value;
        next_value = -1;
        timer_counter++;
      }
      else {
        display_num = timer_counter;
        timer_counter++;
      }

      if (timer_counter > 99) timer_counter = 0;

      data_reg = build_segments(display_num);
      machine_state = SENDING;
      current_bit = 15;
    }
  }
  switch (machine_state) {
    case SENDING:
      if ((data_reg >> current_bit) & 1) {
        PORTD |= (1 << 7);
      } else {
        PORTD &= ~(1 << 7);
      }
      PORTD |= (1 << 3);
      PORTD &= ~(1 << 3);

      current_bit--;
      if (current_bit < 0) {
        machine_state = UPDATING;
      }
      break;

    case UPDATING:
      PORTD |= (1 << 5);
      PORTD &= ~(1 << 5);

      machine_state = IDLE;
      break;

    case IDLE:
      break;
  }
}