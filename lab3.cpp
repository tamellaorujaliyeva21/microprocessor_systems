#include <Arduino.h>
#include <avr/io.h>

// Global counter stored in SRAM (volatile because it may change unexpectedly)
volatile uint8_t counter = 200;   

void setup() {

  Serial.begin(9600); // Start serial communication

  // Wait until any previous EEPROM write is finished
  while (EECR & (1 << EEPE));   

  // Set EEPROM address (0x00)
  EEAR = 0x00;                 

  // Trigger EEPROM read
  EECR |= (1 << EERE);          

  // Load stored value into counter
  counter = EEDR;               
}

void loop() {

  // Inline assembly: increment counter using CPU registers
  asm volatile(
    "lds r24, counter  \n\t"   // Load counter into register r24
    "inc r24           \n\t"   // Increment value (r24 = r24 + 1)
    "sts counter, r24  \n\t"   // Store result back to counter
  );

  // Print current counter value
  Serial.print("Counter = ");
  Serial.println(counter);

  // Check if user sent a command via Serial
  if (Serial.available()) {

    char command = Serial.read(); // Read input character

    // If user presses 'S' → store counter in EEPROM
    if (command == 'S') {

      // Wait for any ongoing EEPROM write
      while (EECR & (1 << EEPE));   

      EEAR = 0x00;          // Select EEPROM address
      EEDR = counter;       // Load data to be written

      EECR |= (1 << EEMPE); // Enable master write
      EECR |= (1 << EEPE);  // Start EEPROM write

      Serial.println("Stored in EEPROM.");
    }

    // If user presses 'R' → reset counter and EEPROM
    if (command == 'R') {

      counter = 0; // Reset counter in RAM

      // Wait for EEPROM to be ready
      while (EECR & (1 << EEPE));

      EEAR = 0x00;   // EEPROM address
      EEDR = 0;      // Value to store (reset)

      EECR |= (1 << EEMPE); // Enable write
      EECR |= (1 << EEPE);  // Start write

      Serial.println("Reset to 0.");
    }
  }

  // Wait 1 second before next increment
  delay(1000); 
}
