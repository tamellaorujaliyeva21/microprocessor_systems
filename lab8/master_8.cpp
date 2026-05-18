#define F_CPU 16000000UL   // CPU frequency = 16 MHz (used for delay calculations)

#define SLAVE_ADDR 0x08    // I2C address of the slave device

/* PROTOCOL VALUES:
    0x00 - do nothing
    0x01 - send command (turn ON LED on slave)
    0x02 - receive signal (slave button pressed) */

#include <avr/io.h>
#include <util/delay.h>
#include <stdbool.h>

/* ===================== TWI (I2C) LOW LEVEL FUNCTIONS ===================== */

// Generate START condition on I2C bus
void TWI_start() {
    TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN); // TWSTA = START, TWEN = enable TWI
    while (!(TWCR & (1 << TWINT))); // wait until START is transmitted
}

// Generate STOP condition on I2C bus
void TWI_stop() {
    TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN); // TWSTO = STOP
}

// Write a byte to I2C bus
void TWI_write(uint8_t data) {
    TWDR = data; // load data into TWI Data Register
    TWCR = (1 << TWINT) | (1 << TWEN); // start transmission
    while (!(TWCR & (1 << TWINT))); // wait until transmission completes
}

// Read a byte from I2C bus without sending ACK (used for last byte)
uint8_t TWI_read_nack() {
    TWCR = (1 << TWINT) | (1 << TWEN); // read without ACK
    while (!(TWCR & (1 << TWINT))); // wait until data is received
    return TWDR; // return received byte
}

/* ===================== MASTER TRANSMISSION FUNCTIONS ===================== */

// Send one byte of data to the slave
void send_to_slave(uint8_t data) {
    TWI_start(); // start communication

    // Send slave address + write bit (0)
    TWI_write((SLAVE_ADDR << 1) | 0);

    TWI_write(data); // send actual data

    TWI_stop(); // end communication
}

// Request one byte of data from slave
uint8_t request_from_slave() {
    uint8_t data;

    TWI_start(); // start communication

    // Send slave address + read bit (1)
    TWI_write((SLAVE_ADDR << 1) | 1);

    data = TWI_read_nack(); // read data from slave (no ACK since single byte)

    TWI_stop(); // end communication

    return data; // return received data
}

/* ===================== SETUP FUNCTION ===================== */

void setup() {
    TWSR = 0x00; // prescaler = 1
    TWBR = 72;   // bit rate register → sets SCL ≈ 100 kHz
    TWCR = (1 << TWEN); // enable TWI (I2C hardware)

    // Enable internal pull-up resistors for SDA (PC4) and SCL (PC5)
    PORTC |= (1 << PC4) | (1 << PC5);

    DDRB |= (1 << PB5); // set PB5 as output (onboard LED)

    // Configure PD2 as input (button)
    DDRD &= ~(1 << PD2);

    // Enable pull-up resistor on PD2
    PORTD |= (1 << PD2);
}

/* ===================== MAIN LOOP ===================== */

void loop() {

    // Check if button is pressed (active LOW)
    if (!(PIND & (1 << PD2))) {
        send_to_slave(0x01); // send command to slave → turn LED ON
    } else {
        send_to_slave(0x00); // send command → do nothing / turn OFF
    }

    // Request data from slave
    uint8_t received = request_from_slave();

    // If slave reports its button is pressed
    if (received == 0x02) {
        PORTB |= (1 << PB5); // turn ON master LED
    } else {
        PORTB &= ~(1 << PB5); // turn OFF master LED
    }

    _delay_ms(10); // small delay for stability
}
