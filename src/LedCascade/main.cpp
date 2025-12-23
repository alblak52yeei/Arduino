#include <Arduino.h>

#define NUM_LEDS 5

const uint8_t blink_periods[NUM_LEDS] = {1, 2, 3, 4, 5};
volatile uint8_t period_counters[NUM_LEDS] = {0};

void setup() {
    cli();

    // Настройка порта B на вывод
    DDRB = DDRB | ((1 << DDB0) | (1 << DDB1) | (1 << DDB2) | (1 << DDB3) | (1 << DDB4));
    // Начальное состояние - все светодиоды включены
    PORTB = PORTB | ((1 << PORTB0) | (1 << PORTB1) | (1 << PORTB2) | (1 << PORTB3) | (1 << PORTB4));

    // Настройка Timer1 в режиме CTC
    TCCR1A = 0;
    TCCR1B = 0;
    TCNT1 = 0;

    TCCR1B = TCCR1B | (1 << WGM12);
    TCCR1B = TCCR1B | ((1 << CS11) | (1 << CS10));

    OCR1A = 2499;

    TIMSK1 = TIMSK1 | (1 << OCIE1A);

    sei();
}

ISR(TIMER1_COMPA_vect) {
    for (uint8_t i = 0; i < NUM_LEDS; i++) {
        period_counters[i]++;

        if (period_counters[i] >= blink_periods[i]) {
            PORTB = PORTB ^ (1 << i);
            period_counters[i] = 0;
        }
    }
}

void loop() {
}