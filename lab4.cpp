#include <avr/io.h> 
#include <avr/interrupt.h>

volatile uint8_t countdown = 0;

ISR(INT0_vect) { 
    PORTB |= (1 << 5); 
    countdown = 10;  
    TCNT1 = 0;                
    TIFR1 |= (1 << OCF1A);  
    TIMSK1 |= (1 << OCIE1A);   
}
ISR(TIMER1_COMPA_vect) { 
    if (countdown > 0) {
        countdown--;
        if (countdown == 0) {
            PORTB &= ~(1 << 5); 
            TIMSK1 &= ~(1 << OCIE1A);
        }
        }
    }

void setup() {

    DDRB |= (1 << 5);   
    PORTB &= ~(1 << 5);   

    DDRD &= ~(1 << 2);  
    PORTD |= (1 << 2); 

    EICRA &= ~((1 << ISC00));
    EICRA |= (1 << ISC01);
    EIMSK |= (1 << INT0);

    TCCR1A = 0;                         // Clear control register A
    TCCR1B = 0;                         // Clear control register B
    TCNT1  = 0;                         // Reset counter
    
    TCCR1B |= (1 << WGM12);
    TCCR1B |= (1 << CS10) | (0 << CS11); // Prescaler = 64

    OCR1A = 249; // 1 ms interval
    TIMSK1 |= (1 << OCIE1A);

    sei();
}

void loop() {
    
}

/*so basicly D2 is button input which is watched by INT0 for falling edge 
because pull-up makes it HIGH in not-press, and pressing makes it LOW.
when button is pressed, INT0 runs that turn on D13 and sets countdown=10
Timer1 runs every 1ms, each 1 ms decreases countdown
when countdown=0 it turns off D13 
so D13 satys on about 10 ms

INT0 - external interrupt - happens because of change on outside pin
Timer1- timer interrupt - happens because of internal timer - timer count clock ticks by itself, when certain time is reached creates interrupt
vect - vector shows address/location in memory where cpu should jump when specific interrupt happens

f(timer) = f(cpu)/prescaler = 16MHz/64 = 250 000Hz
T(tick) = 1/f(timer) = 1/250000= 4us
T(tick) = prescaler/f(cpu) = 64/16000000 = 4us
counts = ORC1A + 1 = 249 + 1 = 250
T = counts * T(tick) = (ORC1A + 1) * T(tick) = 250 * 4us = 1ms
T(pulse) = countdown * T = 10 * 1ms = 10ms

button pressed = LOW
button not pressed = HIGH
HIGH => LOW - falling edge

00 → low level
01 → any logical change
10 → falling edge = ISC01=1, ISC0=0
11 → rising edge

TCCR1B |= (1 << WGM12); make Timer1 count up until it matches ORC1A, then reset in CTC mode
TIMSK1 |= (1 << OCIE1A) when Timer1 matches with ORC1A allow interrupt to happen

sei(); enable global interrupts without it even if INT0 and Timer1 are enabled doesnt work

TIFR1 |= (1 << OCF1A); clear old timer1 compare match event = contains flags that show events already happened
*/
