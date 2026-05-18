// MASTER CODE
// This code configures the Arduino/AVR board as an SPI Master.
// The master sends values one by one to the SPI Slave.
// The values sent are: 85, 170, and 255.
// After sending all three values, it starts again from the first value.

#include <avr/io.h>        // Gives access to AVR registers like DDRB, PORTB, SPCR, SPSR, SPDR
#include <util/delay.h>    // Used for _delay_ms()

// This array contains the data that the master will send to the slave.
// 85  = 01010101 in binary
// 170 = 10101010 in binary
// 255 = 11111111 in binary
uint8_t dataSequence[3] = {85, 170, 255};

// This variable keeps track of which value from the array should be sent next.
uint8_t indexTx = 0;

void SPI_MasterInit() {
  // Set SPI pins for the master.
  // PB2 = SS   -> Slave Select
  // PB3 = MOSI -> Master Out Slave In
  // PB5 = SCK  -> SPI Clock
  //
  // These pins must be outputs for the master.
  DDRB |= (1 << PB2) | (1 << PB3) | (1 << PB5);

  // PB4 = MISO -> Master In Slave Out
  // This pin is input because the master receives data from the slave through MISO.
  DDRB &= ~(1 << PB4);

  // Keep SS HIGH when the slave is not selected.
  // In SPI, SS is usually active LOW.
  // HIGH means the slave is not active yet.
  PORTB |= (1 << PB2);

  // Configure the SPI Control Register.
  //
  // SPE  = SPI Enable. This turns on SPI.
  // MSTR = Master mode. This makes this device the SPI master.
  // SPR1 and SPR0 = clock rate selection bits.
  //
  // With SPR1 = 1 and SPR0 = 1, SPI clock = fosc / 128.
  // If the CPU clock is 16 MHz:
  // 16 MHz / 128 = 125 kHz.
  SPCR = (1 << SPE) | (1 << MSTR) | (1 << SPR1) | (1 << SPR0);
}

uint8_t SPI_MasterTransmit(uint8_t data) {
  // Select the slave before sending data.
  // SS LOW means the slave becomes active and listens to the master.
  PORTB &= ~(1 << PB2);

  // Put the data byte into the SPI Data Register.
  // Once data is written into SPDR, SPI transmission starts automatically.
  SPDR = data;

  // Wait until the transmission is complete.
  // SPIF is the SPI Interrupt Flag.
  // It becomes 1 when the SPI transfer is finished.
  while (!(SPSR & (1 << SPIF))) {
    // Do nothing here, just wait.
  }

  // Read SPDR after transmission.
  // In SPI, sending and receiving happen at the same time.
  // Even if we only care about sending, reading SPDR properly clears the SPIF flag.
  uint8_t received = SPDR;

  // Deselect the slave after the transfer is finished.
  // SS HIGH means the slave is no longer selected.
  PORTB |= (1 << PB2);

  // Return the byte received from the slave.
  // In this code, we do not use it in loop(), but the function still returns it.
  return received;
}

void setup() {
  // Initialize SPI in master mode.
  SPI_MasterInit();
}

void loop() {
  // Send the current value from the dataSequence array to the slave.
  SPI_MasterTransmit(dataSequence[indexTx]);

  // Move to the next value in the array.
  indexTx++;

  // If all 3 values were sent, start again from the first value.
  if (indexTx >= 3) {
    indexTx = 0;
  }

  // Wait 1 second before sending the next value.
  _delay_ms(1000);
}
