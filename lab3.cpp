#include <Arduino.h>
#include <avr/io.h>

volatile uint8_t counter = 200;   

void setup() {

  Serial.begin(9600);

  while (EECR & (1 << EEPE));   

  EEAR = 0x00;                 
  EECR |= (1 << EERE);          
  counter = EEDR;               
}

void loop() {

  asm volatile(
    "lds r24, counter  \n\t"   
    "inc r24           \n\t"   
    "sts counter, r24  \n\t"   
  );

  Serial.print("Counter = ");
  Serial.println(counter);

  if (Serial.available()) {

    char command = Serial.read();

    if (command == 'S') {

      while (EECR & (1 << EEPE));   

      EEAR = 0x00;                  
      EEDR = counter;               
      EECR |= (1 << EEMPE);         
      EECR |= (1 << EEPE);          

      Serial.println("Stored in EEPROM.");
    }

    if (command == 'R') {

      counter = 0;

      while (EECR & (1 << EEPE));

      EEAR = 0x00;
      EEDR = 0;
      EECR |= (1 << EEMPE);
      EECR |= (1 << EEPE);

      Serial.println("Reset to 0.");
    }
  }

  delay(1000); 
}
