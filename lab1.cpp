// Method 1

void setup() { 
  pinMode(13, OUTPUT);   // Configure pin 13 as an output pin
}

void loop() {
  digitalWrite(13, HIGH); // Set pin 13 HIGH → turn LED ON
  delay(500);             // Wait for 500 milliseconds (0.5 seconds)

  digitalWrite(13, LOW);  // Set pin 13 LOW → turn LED OFF
  delay(500);             // Wait for 500 milliseconds (0.5 seconds)
}

// Method 2

void setup() {
  DDRB |= (1 << DDB5);        // Set PB5 (Arduino pin 13) as OUTPUT
                              // DDRB = Data Direction Register for Port B
                              // DDB5 corresponds to bit 5 (pin 13)
}

void loop() {
  PORTB |= (1 << PORTB5);     // Set PB5 HIGH → turn LED ON
                              // PORTB controls the output values

  delay(500);                 // Wait for 500 milliseconds

  PORTB &= ~(1 << PORTB5);    // Clear PB5 → turn LED OFF
                              // ~ (NOT) ensures only bit 5 is cleared

  delay(500);                 // Wait for 500 milliseconds
}
