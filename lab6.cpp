// This line tells the compiler that the microcontroller clock speed is 16 MHz.
// It is important for timing calculations, especially when using timers.
#define F_CPU 16000000UL

// This library gives access to AVR input/output registers like DDRB, PORTB, etc.
#include <avr/io.h>

// This library allows us to use interrupts.
#include <avr/interrupt.h>

// This variable stores the current digit shown on the 7-segment display.
// volatile is used because this value changes inside an interrupt.
volatile uint8_t digit = 0;

// This variable tells whether counting is paused or not.
// 0 means not paused, 1 means paused.
volatile uint8_t paused = 0;

// This array contains the binary patterns for digits 0 to 9.
// Each value tells which segments of the 7-segment display should turn on.
// The segments are usually named a, b, c, d, e, f, g.
const uint8_t seg[10] = {
    0x3F, // 0
    0x06, // 1
    0x5B, // 2
    0x4F, // 3
    0x66, // 4
    0x6D, // 5
    0x7D, // 6
    0x07, // 7
    0x7F, // 8
    0x6F  // 9
};

// This function displays one digit on the 7-segment display.
static inline void show_digit(uint8_t n) {
    // Get the segment pattern for the digit n.
    uint8_t x = seg[n];

    // Send segments a-f to pins PB0, PB1, PB2, PB3, PB4, PB5.
    // First, (PORTB & ~0x3F) clears only PB0..PB5.
    // Then, (x & 0x3F) puts the needed segment values into those pins.
    // Other PORTB pins are not changed.
    PORTB = (PORTB & ~0x3F) | (x & 0x3F);

    // Send segment g to PC0.
    // Segment g is bit 6 in the segment pattern, so we shift it right by 6.
    // Then we place it into PC0 without changing other PORTC pins.
    PORTC = (PORTC & ~0x01) | ((x >> 6) & 1);
}

// This interrupt runs automatically every 0.5 seconds.
// It is triggered by Timer1 compare match.
ISR(TIMER1_COMPA_vect) {
    // Only count if the program is not paused.
    if (!paused) {
        // Go to the next digit.
        // After 9, it goes back to 0 because of % 10.
        digit = (digit + 1) % 10;

        // Show the updated digit on the display.
        show_digit(digit);
    }
}

// This interrupt runs when the button connected to INT0 is pressed.
// On Arduino Uno, INT0 is usually PD2.
ISR(INT0_vect) {
    // Toggle pause/resume.
    // If paused was 0, it becomes 1.
    // If paused was 1, it becomes 0.
    paused ^= 1;
}

int main(void) {
    // Set PB0..PB5 as output pins.
    // These pins control segments a-f of the 7-segment display.
    DDRB |= 0x3F;

    // Set PC0 as output.
    // This pin controls segment g.
    DDRC |= (1 << PC0);

    // Set PD2 as input.
    // The button is connected to this pin.
    DDRD &= ~(1 << PD2);

    // Enable the internal pull-up resistor on PD2.
    // This means the button pin is HIGH normally,
    // and becomes LOW when the button is pressed.
    PORTD |= (1 << PD2);

    // Show the starting digit, which is 0.
    show_digit(digit);

    // ---------------- Timer1 setup ----------------

    // Clear Timer1 control register A.
    // We do not need special waveform output pins here.
    TCCR1A = 0;

    // Configure Timer1:
    // WGM12 = 1 means CTC mode.
    // In CTC mode, the timer resets when it reaches OCR1A.
    // CS12 = 1 means prescaler 256.
    TCCR1B = (1 << WGM12) | (1 << CS12);

    // Set the compare value for 0.5 seconds.
    //
    // Clock frequency = 16,000,000 Hz
    // Prescaler = 256
    // Timer frequency = 16,000,000 / 256 = 62,500 ticks per second
    //
    // For 0.5 seconds:
    // 62,500 * 0.5 = 31,250 ticks
    //
    // Since counting starts from 0, we use 31,250 - 1 = 31,249.
    OCR1A = 31249;

    // Enable Timer1 compare match A interrupt.
    // This makes ISR(TIMER1_COMPA_vect) run every 0.5 seconds.
    TIMSK1 = (1 << OCIE1A);

    // ---------------- External interrupt setup ----------------

    // Configure INT0 to trigger on falling edge.
    // Falling edge means the signal changes from HIGH to LOW.
    // This happens when the button is pressed because of INPUT_PULLUP.
    EICRA = (1 << ISC01);

    // Enable external interrupt INT0.
    EIMSK = (1 << INT0);

    // Enable global interrupts.
    // Without this, Timer1 interrupt and INT0 interrupt will not work.
    sei();

    // Infinite loop.
    // The main program does nothing here because everything is handled by interrupts.
    while (1) {
    }
}
