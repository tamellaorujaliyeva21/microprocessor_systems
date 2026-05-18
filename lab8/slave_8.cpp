#define F_CPU 16000000UL   // CPU frequency = 16 MHz

#include <avr/io.h>

#define SLAVE_ADDR 0x08    // I2C slave address

// Variable that stores data to send back to master
// 'volatile' because it can change during hardware events
volatile uint8_t state = 0x00;

/* ===================== SETUP FUNCTION ===================== */

void setup() {

  // Set own slave address (shifted left because LSB is reserved for general call)
  TWAR = (SLAVE_ADDR << 1);

  // Enable TWI (I2C), enable ACK, and clear interrupt flag to be ready
  TWCR = (1 << TWEN) | (1 << TWEA) | (1 << TWINT);

  // Enable pull-up resistors on SDA (PC4) and SCL (PC5)
  PORTC |= (1 << PC4) | (1 << PC5);

  // Configure PB5 as output (LED)
  DDRB |= (1 << PB5);

  // Configure PD2 as input (button)
  DDRD &= ~(1 << PD2);

  // Enable pull-up resistor on button pin
  PORTD |= (1 << PD2);
}

/* ===================== MAIN LOOP ===================== */

void loop() {

    // Check if local button is pressed (active LOW)
    if (!(PIND & (1 << PD2))) {
        state = 0x02; // prepare response for master ("button pressed")
    }

    // Check if a TWI event has occurred (TWINT flag set)
    if (TWCR & (1 << TWINT)) {

        // Read TWI status (mask lower 3 bits → keep only status code)
        uint8_t status = TWSR & 0b11111000;

        /* ===================== CASE 1: MASTER WRITES TO SLAVE ===================== */
        if (status == 0x60) { 
            // Own SLA+W received (slave addressed for writing)

            // Prepare to receive data, send ACK
            TWCR = (1 << TWEN) | (1 << TWEA) | (1 << TWINT);
        }

        /* ===================== CASE 2: DATA RECEIVED FROM MASTER ===================== */
        else if (status == 0x80) { 
            // Data received after SLA+W

            uint8_t received = TWDR; // Read received byte

            // Check protocol command from master
            if (received == 0x01) {
                PORTB |= (1 << PB5);  // Turn LED ON
            } else {
                PORTB &= ~(1 << PB5); // Turn LED OFF
            }

            // Acknowledge and continue communication
            TWCR = (1 << TWEN) | (1 << TWEA) | (1 << TWINT);
        }

        /* ===================== CASE 3: MASTER READS FROM SLAVE ===================== */
        else if (status == 0xA8) { 
            // Own SLA+R received (master requests data)

            TWDR = state; // Load data to send to master

            // Send data and ACK
            TWCR = (1 << TWEN) | (1 << TWEA) | (1 << TWINT);

            state = 0x00; // Reset state after sending (one-time message)
        }

        /* ===================== DEFAULT CASE ===================== */
        else {
            // Any other state (ignore but continue communication)

            TWCR = (1 << TWEN) | (1 << TWEA) | (1 << TWINT);
        }
    }
}
